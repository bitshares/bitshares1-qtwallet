#include "html5viewer/html5viewer.h"

#include <boost/thread.hpp>
#include <fc/filesystem.hpp>
#include <bts/blockchain/config.hpp>
#include <signal.h>

#include <QApplication>
#include <QSettings>
#include <QPixmap>
#include <QErrorMessage>
#include <QSplashScreen>
#include <QDir>
#include <QWebSettings>
#include <QWebPage>
#include <QGraphicsWebView>

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

#ifdef NDEBUG
#include "config_prod.hpp"
#else
#include "config_dev.hpp"
#endif


struct config
{
   config( ) 
   : default_peers{{"107.170.30.182:8764"},{"114.215.104.153:8764"},{"84.238.140.192:8764"}},
    splash_screen_path( (fc::path(package_dir) / "images/splash_screen.png").string() )
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
   fc::string                   splash_screen_path; 
};

FC_REFLECT( config, (default_peers)(logging)(splash_screen_path) )


static config load_config( const fc::path& data_dir )
{
  auto config_file = data_dir / "config.json";
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
  fc::configure_logging( cfg.logging );
  return cfg;
}

int main( int argc, char** argv )
{
    
    QCoreApplication::setOrganizationName( "BitShares" );
    QCoreApplication::setOrganizationDomain( "bitshares.org" );
    QCoreApplication::setApplicationName( BTS_BLOCKCHAIN_NAME );
    QApplication app(argc, argv);
    
   try {
    auto data_dir = fc::app_path() / "BitShares" BTS_ADDRESS_PREFIX;

    config cfg = load_config( data_dir );

    std::shared_ptr<bts::client::client> client;
    bts::rpc::rpc_server_ptr                rpc_server;

    bts::rpc::rpc_server::config rpc;
#   ifdef NDEBUG
    rpc.rpc_user     = "randomuser";
    rpc.rpc_password = fc::variant(fc::ecc::private_key::generate()).as_string();
    rpc.httpd_endpoint = fc::ip::endpoint::from_string( "127.0.0.1:9999" );
    rpc.httpd_endpoint.set_port(0);
#   else
    rpc.rpc_user     = "user";
    rpc.rpc_password = "pass";
    rpc.httpd_endpoint = fc::ip::endpoint::from_string( "0.0.0.0:5680" );
    rpc.httpd_endpoint.set_port(5680);
#   endif
    rpc.htdocs = fc::path( package_dir ) / "htdocs";
    ilog( "htdocs: ${d}", ("d",rpc.htdocs) );
       
    bool rpc_success = false;
    fc::optional<fc::ip::endpoint> actual_httpd_endpoint;
       
    QSettings settings; 

    //uint32_t p2pport = settings.value( "network/p2p/port", BTS_NETWORK_DEFAULT_P2P_PORT ).toInt();
    bool     upnp    = settings.value( "network/p2p/use_upnp", true ).toBool();

    
    QString splash_screen_path = cfg.splash_screen_path.c_str();
        
    // TODO: splash_screen.png's path should be loaded from config
    QPixmap pixmap(splash_screen_path);
    QSplashScreen splash(pixmap);
       splash.showMessage(QObject::tr("Starting RPC Server..."),
                          Qt::AlignCenter | Qt::AlignBottom, Qt::white);   
    
    splash.show();
    

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
      
      if( upnp )
      {
         auto upnp_service = new bts::net::upnp_service();
         upnp_service->map_port( actual_p2p_endpoint.port() );
      }
      client->connect_to_p2p_network();

      for (std::string default_peer : cfg.default_peers)
        client->connect_to_peer(default_peer);
    }).wait();

    if( !actual_httpd_endpoint ) return -1;
       
    ilog( "http rpc url: ${url}", ("url",actual_httpd_endpoint) );
    std::cout << "http rpc url: http://" << std::string( *actual_httpd_endpoint ) << std::endl;
    
    QString initial_url = ("http://" + std::string( *actual_httpd_endpoint )).c_str();
    if( !fc::exists( data_dir / "wallets" / "default" ) )
        initial_url += "/blank.html#/createwallet";
        
    qApp->processEvents();
    
    Html5Viewer viewer;
    QWebSettings::globalSettings()->setAttribute( QWebSettings::PluginsEnabled, false );
    viewer.webView()->page()->settings()->setAttribute( QWebSettings::PluginsEnabled, false );
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
    //bitshares_thread.quit(); - not sure if this is needed, I assume .wait() makes it to wait until thread exits
    
    return 0;
   }
   catch ( const fc::exception& e) 
   {
      elog( "${e}", ("e",e.to_detail_string() ) );
      QErrorMessage::qtHandler()->showMessage( e.to_string().c_str() );
   }
}
