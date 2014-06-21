#pragma once

#include <QObject>
#include <QVariant>

#include <bts/rpc/rpc_server.hpp>
#include <bts/client/client.hpp>

class ClientWrapper : public QObject 
{
   Q_OBJECT

   public:
       ClientWrapper(QObject *parent = nullptr);
       ~ClientWrapper();

       ///Not done in constructor to allow caller to connect to error()
       void initialize();

       QUrl http_url() const;

       Q_INVOKABLE QVariant get_info();
       std::shared_ptr<bts::client::client> get_client() { return _client; }

   signals:
       void initialized();
       void error(QString errorString);

   private:
       bts::client::config                  _cfg;
       std::shared_ptr<bts::client::client> _client;
       fc::thread                           _bitshares_thread;
       fc::future<void>                     _init_complete;
       fc::optional<fc::ip::endpoint>       _actual_httpd_endpoint;
};
