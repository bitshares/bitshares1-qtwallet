#include "ClientWrapper.hpp"

#include <bts/net/upnp.hpp>

#include <QApplication>
#include <QResource>
#include <QSettings>
#include <QJsonDocument>
#include <QUrl>

#include <iostream>

void get_htdocs_file( const fc::path& filename, const fc::http::server::response& r )
{
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
    QSettings settings("BitShares", BTS_BLOCKCHAIN_NAME);
    bool      upnp    = settings.value( "network/p2p/use_upnp", true ).toBool();
    uint32_t  p2pport = settings.value( "network/p2p/port", BTS_NETWORK_DEFAULT_P2P_PORT ).toInt();
    std::string default_wallet_name = settings.value("client/default_wallet_name", "default").toString().toStdString();
    Q_UNUSED(p2pport);

    _cfg.rpc.rpc_user     = "randomuser";
    _cfg.rpc.rpc_password = fc::variant(fc::ecc::private_key::generate()).as_string();
    _cfg.rpc.httpd_endpoint = fc::ip::endpoint::from_string( "127.0.0.1:9999" );
    _cfg.rpc.httpd_endpoint.set_port(0);
    ilog( "config: ${d}", ("d", fc::json::to_pretty_string(_cfg) ) );

    auto data_dir = fc::app_path() / BTS_BLOCKCHAIN_NAME;

    fc::thread* main_thread = &fc::thread::current();

    _init_complete = _bitshares_thread.async( [this,main_thread,data_dir,upnp,p2pport,default_wallet_name](){
      try
      {
        main_thread->async( [&](){ Q_EMIT status_update(tr("Starting %1 client").arg(qApp->applicationName())); });
        _client = std::make_shared<bts::client::client>();
        _client->open( data_dir );

        // setup  RPC / HTTP services
        main_thread->async( [&](){ Q_EMIT status_update(tr("Loading interface")); });
        _client->get_rpc_server()->set_http_file_callback( get_htdocs_file );
        _client->get_rpc_server()->configure_http( _cfg.rpc );
        _actual_httpd_endpoint = _client->get_rpc_server()->get_httpd_endpoint();

        // load config for p2p node.. creates cli
        _client->configure( data_dir );
        _client->init_cli();

        main_thread->async( [&](){ Q_EMIT status_update(tr("Connecting to %1 network").arg(qApp->applicationName())); });
        _client->listen_on_port(0, false /*don't wait if not available*/);
        fc::ip::endpoint actual_p2p_endpoint = _client->get_p2p_listening_endpoint();

        _client->connect_to_p2p_network();

        for (std::string default_peer : _cfg.default_peers)
          _client->connect_to_peer(default_peer);

        _client->set_daemon_mode(true);
        _client->start();
        _client->start_delegate_loop();
        if( !_actual_httpd_endpoint )
        {
            main_thread->async( [&](){ Q_EMIT error( tr("Unable to start HTTP server...")); });
        }

        main_thread->async( [&](){ Q_EMIT status_update(tr("Forwarding port")); });
        if( upnp )
        {
           auto upnp_service = new bts::net::upnp_service();
           upnp_service->map_port( actual_p2p_endpoint.port() );
        }

        _client->wallet_open(default_wallet_name);

        main_thread->async( [&](){ Q_EMIT initialized(); });
      }
      catch (const bts::db::db_in_use_exception&)
      {
        main_thread->async( [&](){ Q_EMIT error( tr("An instance of %1 is already running! Please close it and try again.").arg(qApp->applicationName())); });
      }
      catch (const fc::exception &e)
      {
        ilog("Failure when attempting to initialize client: ${error}", ("error", e.to_detail_string()));
        main_thread->async( [&](){ Q_EMIT error( tr("An error occurred while trying to start: %1").arg(e.what())); });
      }
    });
}

QUrl ClientWrapper::http_url() const
{
    QUrl url = QString::fromStdString("http://" + std::string( *_actual_httpd_endpoint ) );
    url.setUserName(_cfg.rpc.rpc_user.c_str() );
    url.setPassword(_cfg.rpc.rpc_password.c_str() );
    return url;
}

QVariant ClientWrapper::get_info(  )
{
    fc::variant_object result = _bitshares_thread.async( [this](){ return _client->get_info(); }).wait();
    std::string sresult = fc::json::to_string( result );
    return QJsonDocument::fromJson( QByteArray( sresult.c_str(), sresult.length() ) ).toVariant();
}

QString ClientWrapper::get_http_auth_token()
{
    QByteArray result = _cfg.rpc.rpc_user.c_str();
    result += ":";
    result += _cfg.rpc.rpc_password.c_str();
    return result.toBase64( QByteArray::Base64Encoding | QByteArray::KeepTrailingEquals );
}


