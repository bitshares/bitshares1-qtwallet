#include <boost/program_options.hpp>

#include <bts/client/client.hpp>
#include <bts/net/upnp.hpp>
#include <bts/blockchain/chain_database.hpp>
#include <bts/rpc/rpc_server.hpp>
#include <bts/cli/cli.hpp>
#include <bts/utilities/git_revision.hpp>
#include <fc/filesystem.hpp>
#include <fc/thread/thread.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/log/logger_config.hpp>
#include <fc/io/json.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/git_revision.hpp>
#include <fc/io/json.hpp>

#include <iostream>
#include <iomanip>

#include "bts_xt_thread.h"

struct config
{
   config() : default_peers{{"107.170.30.182:8764"}}, ignore_console(false) {}
   bts::rpc::rpc_server::config rpc;
   std::vector<std::string>     default_peers;
   bool                         ignore_console;
};

FC_REFLECT( config, (rpc)(default_peers)(ignore_console) )

using namespace boost;

void try_to_open_wallet(bts::rpc::rpc_server_ptr);
void print_banner();
void configure_logging(const fc::path&);
fc::path get_data_dir(const program_options::variables_map& option_variables);
config   load_config( const fc::path& datadir );
void  load_and_configure_chain_database(const fc::path& datadir,
                                        const program_options::variables_map& option_variables);

config cfg;

bool BtsXtThread::init(int argc, char** argv)
{

// parse command-line options
   program_options::options_description option_config("Allowed options");
   option_config.add_options()("data-dir", program_options::value<std::string>(), "configuration data directory")
                              ("help", "display this help message")
                              ("p2p-port", program_options::value<uint16_t>(), "set port to listen on")
                              ("maximum-number-of-connections", program_options::value<uint16_t>(), "set the maximum number of peers this node will accept at any one time")
                              ("upnp", program_options::value<bool>()->default_value(true), "Enable UPNP")
                              ("connect-to", program_options::value<std::vector<std::string> >(), "set remote host to connect to")
                              ("server", "enable JSON-RPC server")
                              ("daemon", "run in daemon mode with no CLI console, starts JSON-RPC server")
                              ("rpcuser", "username for JSON-RPC") // default arguments are in config.json
                              ("rpcpassword", "password for JSON-RPC")
                              ("rpcport", "port to listen for JSON-RPC connections")
                              ("httpport", "port to listen for HTTP JSON-RPC connections")
                              ("genesis-config", program_options::value<std::string>()->default_value("genesis.dat"), 
                               "generate a genesis state with the given json file (only accepted when the blockchain is empty)")
                              ("clear-peer-database", "erase all information in the peer database")
                              ("resync-blockchain", "delete our copy of the blockchain at startup, and download a fresh copy of the entire blockchain from the network")
                              ("version", "print the version information for bts_xt_client")
                              ("rpc-only", "start rpc server in console, no gui");

    
    
    boost::program_options::positional_options_description positional_config;
    positional_config.add("data-dir", 1);
    
    p_option_variables = new boost::program_options::variables_map;
    boost::program_options::variables_map& option_variables = *p_option_variables;
    try
    {
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).
                                      options(option_config).positional(positional_config).run(), option_variables);
        boost::program_options::notify(option_variables);
    }
    catch (boost::program_options::error&)
    {
        std::cerr << "Error parsing command-line options\n\n";
        std::cerr << option_config << "\n";
        return false;
    }
    
    if (option_variables.count("help"))
    {
        std::cout << option_config << "\n";
        return false;
    }
    
    if (option_variables.count("version"))
    {
        std::cout << "bts_xt_client built on " << __DATE__ << " at " << __TIME__ << "\n";
        std::cout << "  bitshares_toolkit revision: " << bts::utilities::git_revision_sha << "\n";
        std::cout << "                              committed " << fc::get_approximate_relative_time_string(fc::time_point_sec(bts::utilities::git_revision_unix_timestamp)) << "\n";
        std::cout << "                 fc revision: " << fc::git_revision_sha << "\n";
        std::cout << "                              committed " << fc::get_approximate_relative_time_string(fc::time_point_sec(bts::utilities::git_revision_unix_timestamp)) << "\n";
        return false;
    }
    
   rpc_only = option_variables.count("rpc-only");


   try {
      print_banner();
      fc::path datadir = get_data_dir(option_variables);
      ::configure_logging(datadir);

      cfg   = load_config(datadir);
      std::cout << fc::json::to_pretty_string( cfg ) <<"\n";

      load_and_configure_chain_database(datadir, option_variables);

      client = std::make_shared<bts::client::client>();
      client->open( datadir, option_variables["genesis-config"].as<std::string>() );

      client->run_delegate();

      bts::rpc::rpc_server_ptr rpc_server = client->get_rpc_server();


        // the user wants us to launch the RPC server.
        // First, override any config parameters they
        bts::rpc::rpc_server::config rpc_config(cfg.rpc);
        if (option_variables.count("rpcuser"))
          rpc_config.rpc_user = option_variables["rpcuser"].as<std::string>();
        if (option_variables.count("rpcpassword"))
          rpc_config.rpc_password = option_variables["rpcpassword"].as<std::string>();
        // for now, force binding to localhost only
        if (option_variables.count("rpcport"))
          rpc_config.rpc_endpoint = fc::ip::endpoint(fc::ip::address("127.0.0.1"), option_variables["rpcport"].as<uint16_t>());
        else
          rpc_config.rpc_endpoint = fc::ip::endpoint(fc::ip::address("127.0.0.1"), uint16_t(9988));
        if (option_variables.count("httpport"))
            rpc_config.httpd_endpoint = fc::ip::endpoint(fc::ip::address("127.0.0.1"), option_variables["httpport"].as<uint16_t>());
        std::cout<<"Starting json rpc server on "<< std::string( rpc_config.rpc_endpoint ) <<"\n";
        std::cout<<"Starting http json rpc server on "<< std::string( rpc_config.httpd_endpoint ) <<"\n";     
        try_to_open_wallet(rpc_server); // assuming password is blank
        bool rpc_succuss = rpc_server->configure(rpc_config);
        if (!rpc_succuss)
        {
            std::cerr << "Error starting rpc server\n\n";
            return false;
        }

      client->configure( datadir );

      if (option_variables.count("maximum-number-of-connections"))
      {
        fc::mutable_variant_object params;
        params["maximum_number_of_connections"] = option_variables["maximum-number-of-connections"].as<uint16_t>();
        client->network_set_advanced_node_parameters(params);
      }
       
      if (option_variables.count("clear-peer-database"))
      {
        std::cout << "Erasing old peer database\n";
        client->get_node()->clear_peer_database();
      }

       return true;
    } 
   catch ( const fc::exception& e )
   {
      std::cerr << "------------ error --------------\n" 
                << e.to_detail_string() << "\n";
      wlog( "${e}", ("e", e.to_detail_string() ) );
   }
    return false;
}

void BtsXtThread::run()
{
    try {

        
        client->connect_to_p2p_network();
        if (p_option_variables->count("connect-to"))
        {
            std::vector<std::string> hosts = (*p_option_variables)["connect-to"].as<std::vector<std::string>>();
            for( auto peer : hosts )
                client->connect_to_peer( peer );
        }
        else
        {
            for (std::string default_peer : cfg.default_peers)
                client->connect_to_peer(default_peer);
        }
                
        while(!cancel) fc::usleep(fc::microseconds(100000));
    }
    catch ( const fc::exception& e )
    {
        std::cerr << "------------ error --------------\n" 
        << e.to_detail_string() << "\n";
        wlog( "${e}", ("e", e.to_detail_string() ) );
    }
}

BtsXtThread::~BtsXtThread()
{
    delete p_option_variables;
}

void print_banner()
{
    std::cout<<"================================================================\n";
    std::cout<<"=                                                              =\n";
    std::cout<<"=             Welcome to BitShares "<< std::setw(5) << std::left << BTS_ADDRESS_PREFIX<<"                       =\n";
    std::cout<<"=                                                              =\n";
    std::cout<<"=  This software is in alpha testing and is not suitable for   =\n";
    std::cout<<"=  real monetary transactions or trading.  Use at your own     =\n";
    std::cout<<"=  risk.                                                       =\n";
    std::cout<<"=                                                              =\n";
    std::cout<<"=  Type 'help' for usage information.                          =\n";
    std::cout<<"================================================================\n";
}

void configure_logging(const fc::path& data_dir)
{
    fc::logging_config cfg;
    
    fc::file_appender::config ac;
    ac.filename = data_dir / "default.log";
    ac.truncate = false;
    ac.flush    = true;

    std::cout << "Logging to file \"" << ac.filename.generic_string() << "\"\n";
    
    fc::file_appender::config ac_rpc;
    ac_rpc.filename = data_dir / "rpc.log";
    ac_rpc.truncate = false;
    ac_rpc.flush    = true;

    std::cout << "Logging RPC to file \"" << ac_rpc.filename.generic_string() << "\"\n";
    
    cfg.appenders.push_back(fc::appender_config( "default", "file", fc::variant(ac)));
    cfg.appenders.push_back(fc::appender_config( "rpc", "file", fc::variant(ac_rpc)));
    
    fc::logger_config dlc;
    dlc.level = fc::log_level::debug;
    dlc.name = "default";
    dlc.appenders.push_back("default");
    
    fc::logger_config dlc_rpc;
    dlc_rpc.level = fc::log_level::debug;
    dlc_rpc.name = "rpc";
    dlc_rpc.appenders.push_back("rpc");
    
    cfg.loggers.push_back(dlc);
    cfg.loggers.push_back(dlc_rpc);
    
    fc::configure_logging( cfg );
}


fc::path get_data_dir(const program_options::variables_map& option_variables)
{ try {
   fc::path datadir;
   if (option_variables.count("data-dir"))
   {
     datadir = fc::path(option_variables["data-dir"].as<std::string>().c_str());
   }
   else
   {
#ifdef WIN32
     datadir =  fc::app_path() / "BitShares" BTS_ADDRESS_PREFIX;
#elif defined( __APPLE__ )
     datadir =  fc::app_path() / "BitShares" BTS_ADDRESS_PREFIX;
#else
     datadir = fc::app_path() / ".BitShares" BTS_ADDRESS_PREFIX;
#endif
   }
   return datadir;

} FC_RETHROW_EXCEPTIONS( warn, "error loading config" ) }

void load_and_configure_chain_database( const fc::path& datadir,
                                        const program_options::variables_map& option_variables)
{ try {
  if (option_variables.count("resync-blockchain"))
  {
    std::cout << "Deleting old copy of the blockchain in \"" << ( datadir / "chain" ).generic_string() << "\"\n";
    try
    {
      fc::remove_all(datadir / "chain");
    }
    catch (const fc::exception& e)
    {
      std::cout << "Error while deleting old copy of the blockchain: " << e.what() << "\n";
      std::cout << "You may need to manually delete your blockchain and relaunch bitshares_client\n";
    }
  }
  else
  {
    std::cout << "Loading blockchain from \"" << ( datadir / "chain" ).generic_string()  << "\"\n";
  }

  fc::path genesis_file = option_variables["genesis-config"].as<std::string>();
  std::cout << "Using genesis block from file \"" << fc::absolute( genesis_file ).string() << "\"\n";

} FC_RETHROW_EXCEPTIONS( warn, "unable to open blockchain from ${data_dir}", ("data_dir",datadir/"chain") ) }

config load_config( const fc::path& datadir )
{ try {
      auto config_file = datadir/"config.json";
      config cfg;
      if( fc::exists( config_file ) )
      {
         std::cout << "Loading config \"" << config_file.generic_string()  << "\"\n";
         cfg = fc::json::from_file( config_file ).as<config>();
      }
      else
      {
         std::cerr<<"Creating default config file \""<<config_file.generic_string()<<"\"\n";
         fc::json::save_to_file( cfg, config_file );
      }
      return cfg;
} FC_RETHROW_EXCEPTIONS( warn, "unable to load config file ${cfg}", ("cfg",datadir/"config.json")) }

void try_to_open_wallet(bts::rpc::rpc_server_ptr rpc_server) {
    try
    {
        // try to open without a passphrase first
        rpc_server->direct_invoke_method("open_wallet", fc::variants());
        return;
    }
    catch (...)
    {
    }
}
