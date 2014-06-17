#include "ClientWrapper.hpp"

#include <bts/net/upnp.hpp>

#include <QResource>
#include <QSettings>
#include <QJsonDocument>
#include <QUrl>

#include <iostream>

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

ClientWrapper::ClientWrapper(QObject *parent)
 : QObject(parent),
   bitshares_thread("bitshares")
{
}

void ClientWrapper::initialize()
{
    QSettings settings;
    bool      upnp    = settings.value( "network/p2p/use_upnp", true ).toBool();
    uint32_t  p2pport = settings.value( "network/p2p/port", BTS_NETWORK_DEFAULT_P2P_PORT ).toInt();
    Q_UNUSED(p2pport);

    cfg.rpc.rpc_user     = "randomuser";
    cfg.rpc.rpc_password = fc::variant(fc::ecc::private_key::generate()).as_string();
    cfg.rpc.httpd_endpoint = fc::ip::endpoint::from_string( "127.0.0.1:9999" );
    cfg.rpc.httpd_endpoint.set_port(0);
    ilog( "config: ${d}", ("d", fc::json::to_pretty_string(cfg) ) );

    auto data_dir = fc::app_path() / BTS_BLOCKCHAIN_NAME;

    fc::optional<fc::ip::endpoint> actual_httpd_endpoint;

    fc::thread& main_thread = fc::thread::current();
    Q_UNUSED(main_thread);

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
        emit error(tr("Unable to start HTTP server..."));

    std::cout << "http rpc url: http://" << std::string( *actual_httpd_endpoint ) << std::endl;
}

QUrl ClientWrapper::http_url()
{
    QUrl url = QString::fromStdString("http://" + std::string( *client->get_rpc_server()->get_httpd_endpoint() ));
    url.setUserName(cfg.rpc.rpc_user.c_str() );
    url.setPassword(cfg.rpc.rpc_password.c_str() );
    return url;
}

QVariant ClientWrapper::get_info()
{
    fc::variant_object result = bitshares_thread.async( [=](){ return client->get_info(); }).wait();
    std::string sresult = fc::json::to_string( result );
    return QJsonDocument::fromJson( QByteArray( sresult.c_str(), sresult.length() ) ).toVariant();
}
