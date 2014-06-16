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

#include <QResource>
#include <iostream>
#include <iomanip>

#ifdef NDEBUG
#include "config_prod.hpp"
#else
#include "config_dev.hpp"
#endif

void get_htdocs_file( const fc::path& filename, const fc::http::server::response& r )
{
//   elog( "${file}", ("file",filename) );
   std::cout << filename.generic_string() << "\n";
   QResource  file_to_send( ("/htdocs/htdocs/" + filename.generic_string()).c_str() );
   if( !file_to_send.data() )
   {
      std::string not_found = "this is not the file you are looking for: " + filename.generic_string();
      r.set_status( fc::http::reply::NotFound );
      r.set_length( not_found.size() );
      r.write( not_found.c_str(), not_found.size() );
      return;
   }  
   r.set_status( fc::http::reply::OK );
   if( file_to_send.isCompressed() )
   {
      auto data = qUncompress( file_to_send.data(), file_to_send.size() );
      r.set_length( data.size() );
      r.write( (const char*)data.data(), data.size() );
   }
   else
   {
      r.set_length( file_to_send.size() );
      r.write( (const char*)file_to_send.data(), file_to_send.size() );
   }
}


int main( int argc, char** argv )
{
   QCoreApplication::setOrganizationName( "BitShares" );
   QCoreApplication::setOrganizationDomain( "bitshares.org" );
   QCoreApplication::setApplicationName( BTS_BLOCKCHAIN_NAME );
   QApplication app(argc, argv);

   auto data_dir = fc::app_path() / BTS_BLOCKCHAIN_NAME;

   QPixmap pixmap(":/images/splash_screen.png");
   QSplashScreen splash(pixmap);
      splash.showMessage(QObject::tr("Loading configuration..."),
                         Qt::AlignCenter | Qt::AlignBottom, Qt::white);   
   splash.show();

   QSettings settings; 
   bool      upnp    = settings.value( "network/p2p/use_upnp", true ).toBool();
   uint32_t  p2pport = settings.value( "network/p2p/port", BTS_NETWORK_DEFAULT_P2P_PORT ).toInt();
    
   try {
    std::shared_ptr<bts::client::client> client;

    bts::client::config cfg;
    cfg.rpc.rpc_user     = "randomuser";
    cfg.rpc.rpc_password = fc::variant(fc::ecc::private_key::generate()).as_string();
    cfg.rpc.httpd_endpoint = fc::ip::endpoint::from_string( "127.0.0.1:9999" );
    cfg.rpc.httpd_endpoint.set_port(0);
    ilog( "config: ${d}", ("d", fc::json::to_pretty_string(cfg) ) );
       
    fc::optional<fc::ip::endpoint> actual_httpd_endpoint;
    
    fc::thread& main_thread = fc::thread::current();

    fc::thread bitshares_thread( "bitshares" );
    bitshares_thread.async( [&](){

      client = std::make_shared<bts::client::client>();
      client->open( data_dir );

      // setup  RPC / HTTP services
      client->get_rpc_server()->set_http_file_callback( get_htdocs_file );
      client->get_rpc_server()->configure( cfg.rpc );
      actual_httpd_endpoint = client->get_rpc_server()->get_httpd_endpoint();

      // load config for p2p node.. creates cli
      client->configure( data_dir );
      client->init_cli();

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

      client->start();
      client->run_delegate();
    }).wait();

    if( !actual_httpd_endpoint )
    {
       splash.showMessage(QObject::tr("Unable to start HTTP server..."),
                             Qt::AlignCenter | Qt::AlignBottom, Qt::white);
       fc::usleep( fc::seconds(3) );
       return -1;
    }

    std::cout << "http rpc url: http://" << std::string( *actual_httpd_endpoint ) << std::endl;
    
    QString initial_url = ("http://" + std::string( *actual_httpd_endpoint )).c_str();
        
    qApp->processEvents();
    
    Html5Viewer viewer;
    QWebSettings::globalSettings()->setAttribute( QWebSettings::PluginsEnabled, false );
    viewer.webView()->page()->settings()->setAttribute( QWebSettings::PluginsEnabled, false );
    viewer.setOrientation(Html5Viewer::ScreenOrientationAuto);
    viewer.resize(1200,800);
    viewer.webView()->setAcceptHoverEvents(true);
    viewer.show();
    
    
    QUrl url = QUrl(initial_url);
    url.setUserName(cfg.rpc.rpc_user.c_str() );
    url.setPassword(cfg.rpc.rpc_password.c_str() );
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
