#pragma once

#include <QObject>
#include <QSettings>
#include <QVariant>

#include <bts/rpc/rpc_server.hpp>
#include <bts/client/client.hpp>
#include <bts/net/upnp.hpp>

class ClientWrapper : public QObject 
{
    Q_OBJECT

  public:
    /// Helper interface to send back notifications about startup progressing.
    class INotifier
    {
    public:
      virtual void on_config_loaded(const bts::client::config& config) = 0;

    protected:
      virtual ~INotifier() {}
    };

    ClientWrapper(QObject *parent = nullptr);
    virtual ~ClientWrapper();

    ///Not done in constructor to allow caller to connect to error()
    void initialize(INotifier* notifier);

    QUrl http_url() const;
    void set_web_package(std::unordered_map<std::string, std::vector<char>>&& web_package);
    bool has_web_package() {
      return !_web_package.empty();
    }

    QString get_data_dir();

    Q_INVOKABLE QVariant get_info();
    Q_INVOKABLE QString get_http_auth_token();
    std::shared_ptr<bts::client::client> get_client() { return _client; }

    void handle_crash();

public Q_SLOTS:
    void set_data_dir(QString data_dir);
    void confirm_and_set_approval(QString delegate_name, bool approve);

  Q_SIGNALS:
    void initialized();
    void status_update(QString statusString);
    void error(QString errorString);

  private:
    bts::client::config                  _cfg;
    std::shared_ptr<bts::client::client> _client;
    fc::thread                           _bitshares_thread;
    fc::future<void>                     _init_complete;
    fc::optional<fc::ip::endpoint>       _actual_httpd_endpoint;
    QSettings                            _settings;

    std::shared_ptr<bts::net::upnp_service> _upnp_service;

    std::unordered_map<std::string, std::vector<char>> _web_package;

    void get_htdocs_file(const fc::path& filename, const fc::http::server::response& r);
    void close();
};
