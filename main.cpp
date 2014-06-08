#include "html5viewer/html5viewer.h"

#include <boost/thread.hpp>
#include <fc/filesystem.hpp>
#include <bts/blockchain/config.hpp>
#include <signal.h>

#include <QApplication>
#include <QSettings>
#include <QPixmap>
#include <QSplashScreen>
#include <QDir>

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
#include <fc/log/logger_config.hpp>

#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/stream.hpp>
#include <fstream>

#include <iostream>
#include <iomanip>

bool exit_signal = false;


struct config
{
   config( ) 
   :default_peers{{"107.170.30.182:8764"},{"114.215.104.153:8764"},{"84.238.140.192:8764"}} 
   {}

   void init_default_logger( const fc::path& data_dir )
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
          
          logging = cfg;
   }
   std::vector<std::string>     default_peers;
   fc::logging_config           logging;
};

FC_REFLECT( config, (default_peers)(logging) )



void handle_signal( int signum )
{
   static int count = 0;
   if( count ) exit(1);
   ++count;
   std::cout<< "Signal " << signum << " caught. exiting.." << std::endl;
   exit_signal = true;
}

QString find_splash_screen_png(QString location) {
    QString res = "";
    QDir app_path = location;
    if(app_path.exists("splash_screen.png")) {
        res = app_path.filePath("splash_screen.png");
    } else {
        QDir updir = app_path; updir.cdUp();
        if(updir.exists("splash_screen.png")) res = updir.filePath("splash_screen.png");
        else {
            updir.cdUp();
            if(updir.exists("splash_screen.png")) res = updir.filePath("splash_screen.png");
        }
    }
    if (res == "") printf("WARNING: splash_screen.png not found, splash screen won't be shonw.");
    return res;
}

config load_config( const fc::path& data_dir )
{
      auto config_file = data_dir/"config.json";
      config cfg;
      if( fc::exists( config_file ) )
      {
         std::cout << "Loading config \"" << config_file.generic_string()  << "\"\n";
         cfg = fc::json::from_file( config_file ).as<config>();
      }
      else
      {
         std::cerr<<"Creating default config file \""<<fc::absolute(config_file).generic_string()<<"\"\n";
         cfg.init_default_logger( data_dir );
         fc::create_directories( config_file.parent_path() );
         fc::json::save_to_file( cfg, fc::absolute(config_file) );
         std::cerr << "done saving config\n";	
      }
     // fc::configure_logging( cfg.logging );
      return cfg;
}

int main( int argc, char** argv )
{
    signal(SIGABRT, &handle_signal);
    signal(SIGTERM, &handle_signal);
    signal(SIGINT, &handle_signal);

    QCoreApplication::setOrganizationName( "BitShares" );
    QCoreApplication::setOrganizationDomain( "bitshares.org" );
    QCoreApplication::setApplicationName( BTS_BLOCKCHAIN_NAME );
    
    auto data_dir = fc::app_path() / BTS_BLOCKCHAIN_NAME;

    config cfg = load_config( data_dir );

    std::shared_ptr<bts::client::client> client;
    bts::rpc::rpc_server_ptr                rpc_server;

    bts::rpc::rpc_server::config rpc;
    rpc.rpc_user     = "randomuser";
    rpc.rpc_password = fc::variant(fc::ecc::private_key::generate()).as_string();
    rpc.httpd_endpoint = fc::ip::endpoint::from_string( "127.0.0.1:9999" );
    rpc.httpd_endpoint.set_port(0);
    rpc.htdocs = /*data_dir / */"htdocs";

    bool rpc_success = false;
    fc::optional<fc::ip::endpoint> actual_httpd_endpoint;

    QSettings settings; 

    //uint32_t p2pport = settings.value( "network/p2p/port", BTS_NETWORK_DEFAULT_P2P_PORT ).toInt();
    bool     upnp    = settings.value( "network/p2p/use_upnp", true ).toBool();

    QApplication app(argc, argv);
    
    QString splash_screen_path = find_splash_screen_png(QCoreApplication::applicationFilePath());
        
    // TODO: splash_screen.png's path should be loaded from config
    QPixmap pixmap(splash_screen_path);
    QSplashScreen splash(pixmap);
    
    if(splash_screen_path != "") splash.show();
    
    splash.showMessage(QObject::tr("Starting RPC Server..."),
                       Qt::AlignCenter | Qt::AlignBottom, Qt::white);    


    fc::thread bitshares_thread( "bitshares" );
    bitshares_thread.async( [&](){
      client = std::make_shared<bts::client::client>();
      client->open( data_dir );
      client->run_delegate();

      rpc_success = client->get_rpc_server()->configure( rpc );

      actual_httpd_endpoint = client->get_rpc_server()->get_httpd_endpoint();
      client->configure( data_dir );

      client->listen_on_port(0);
      fc::ip::endpoint actual_p2p_endpoint = client->get_p2p_listening_endpoint();
      
      ilog( "actual p2p port ${p}", ("p",actual_p2p_endpoint) );
      ilog( "actual http port ${p}", ("p",actual_httpd_endpoint) );
      if( upnp )
      {
         auto upnp_service = new bts::net::upnp_service();
         upnp_service->map_port( actual_p2p_endpoint.port() );
      }
      client->connect_to_p2p_network();

      for (std::string default_peer : cfg.default_peers)
        client->connect_to_peer(default_peer);
    }).wait();


    ilog( "url: ${url}", ("url",actual_httpd_endpoint) );
    if( !actual_httpd_endpoint )
       return -1;
    
    QString initial_url = ("http://" + std::string( *actual_httpd_endpoint )).c_str(); //127.0.0.1:5680";//127.0.0.1:5680";
    //if(!fc::exists( dat_adir / "default_wallet.dat" ))
    //    initial_url = "http://127.0.0.1:5680/blank.html#/createwallet";
        
    qApp->processEvents();
    
    /*
    btsxt->wait_until_initialized();
    QThread::sleep(1); // let's give rpc server one more second to bind to port and start listening
    */
    
    Html5Viewer viewer;
    viewer.setOrientation(Html5Viewer::ScreenOrientationAuto);
    viewer.resize(1200,800);
    viewer.show();
    
    
    QUrl url = QUrl(initial_url);
    url.setUserName(rpc.rpc_user.c_str() );
    url.setPassword(rpc.rpc_password.c_str() );
    viewer.loadUrl(url);
    
    splash.finish(&viewer);
    
    app.exec();    
  
    bitshares_thread.async( [&](){ client->stop(); client.reset(); } ).wait();
    bitshares_thread.quit();
    
    return 0;
}
