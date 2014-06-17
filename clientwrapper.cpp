#include "clientwrapper.hpp"

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
   _bitshares_thread("bitshares")
{
}
ClientWrapper::~ClientWrapper()
{
   try {
      _init_complete.wait();
      _bitshares_thread.async( [this](){ _client->stop(); _client.reset(); } ).wait();
   } catch ( ... ) 
   {
      elog( "uncaught exception" );
   }
}

void ClientWrapper::initialize()
{
    QSettings settings;
    bool      upnp    = settings.value( "network/p2p/use_upnp", true ).toBool();
    uint32_t  p2pport = settings.value( "network/p2p/port", BTS_NETWORK_DEFAULT_P2P_PORT ).toInt();
    Q_UNUSED(p2pport);

    _cfg.rpc.rpc_user     = "randomuser";
    _cfg.rpc.rpc_password = fc::variant(fc::ecc::private_key::generate()).as_string();
    _cfg.rpc.httpd_endpoint = fc::ip::endpoint::from_string( "127.0.0.1:9999" );
    _cfg.rpc.httpd_endpoint.set_port(0);
    ilog( "config: ${d}", ("d", fc::json::to_pretty_string(_cfg) ) );

    auto data_dir = fc::app_path() / BTS_BLOCKCHAIN_NAME;


    fc::thread& main_thread = fc::thread::current();
    Q_UNUSED(main_thread);

    _init_complete = _bitshares_thread.async( [this,data_dir,upnp,p2pport](){

      _client = std::make_shared<bts::client::client>();
      _client->open( data_dir );

      // setup  RPC / HTTP services
      _client->get_rpc_server()->set_http_file_callback( get_htdocs_file );
      _client->get_rpc_server()->configure( _cfg.rpc );
      _actual_httpd_endpoint = _client->get_rpc_server()->get_httpd_endpoint();

      // load config for p2p node.. creates cli
      _client->configure( data_dir );
      _client->init_cli();

      _client->listen_on_port(0);
      fc::ip::endpoint actual_p2p_endpoint = _client->get_p2p_listening_endpoint();

      _client->connect_to_p2p_network();

      for (std::string default_peer : _cfg.default_peers)
        _client->connect_to_peer(default_peer);

      _client->set_daemon_mode(true);
      _client->start();
      _client->run_delegate();
      if( !_actual_httpd_endpoint )
      { // I presume Qt will do the proper thread proxy here
          Q_EMIT error( tr("Unable to start HTTP server...")); 
      }

      if( upnp )
      {
         auto upnp_service = new bts::net::upnp_service();
         upnp_service->map_port( actual_p2p_endpoint.port() );
      }

      try {
      _client->wallet_open( "default" );
      } catch ( ... ) {}

      // EMIT COMPLETE...
    });

    _init_complete.wait();
    wlog( "init complete" );

    //std::cout << "http rpc url: http://" << std::string( *actual_httpd_endpoint ) << std::endl;
}

QUrl ClientWrapper::http_url()
{
    QUrl url = QString::fromStdString("http://" + std::string( *_actual_httpd_endpoint ) );
    url.setUserName(_cfg.rpc.rpc_user.c_str() );
    url.setPassword(_cfg.rpc.rpc_password.c_str() );
    return url;
}

QVariant ClientWrapper::get_info()
{
    fc::variant_object result = _bitshares_thread.async( [this](){ return _client->get_info(); }).wait();
    std::string sresult = fc::json::to_string( result );
    return QJsonDocument::fromJson( QByteArray( sresult.c_str(), sresult.length() ) ).toVariant();
}


