#pragma once

#include <QObject>
#include <QVariant>

#include <bts/rpc/rpc_server.hpp>
#include <bts/client/client.hpp>

class ClientWrapper : public QObject {
    Q_OBJECT

public:
    ClientWrapper(QObject *parent = nullptr);
    ~ClientWrapper()
    {
        bitshares_thread.async( [&](){ client->stop(); client.reset(); } ).wait();
    }

    ///Not done in constructor to allow caller to connect to error()
    void initialize();

    QUrl http_url();

    Q_INVOKABLE QVariant get_info();

signals:
    void error(QString errorString);

private:
    bts::client::config cfg;
    std::shared_ptr<bts::client::client> client;
    fc::thread bitshares_thread;
};
