#ifndef BTSXTTHREAD_H
#define BTSXTTHREAD_H

#include <memory>
#include <QThread>

namespace boost {
    namespace program_options { class variables_map; }
}

namespace bts {
    namespace wallet { class wallet; }
    namespace rpc { class rpc_server; }
    namespace client { class client; }
}

class BtsXtThread : public QThread
{
    Q_OBJECT
    bool cancel;
    bool rpc_only;
    std::shared_ptr<bts::client::client> c;
    boost::program_options::variables_map* p_option_variables;
    std::shared_ptr<bts::wallet::wallet> wall;
    std::shared_ptr<bts::rpc::rpc_server> rpc_server;
    
public: 
    BtsXtThread()
    : QThread(0), cancel(false), rpc_only(false)
    {
    }
    
    ~BtsXtThread();
    
    bool init(int argc, char** argv);
    void run() Q_DECL_OVERRIDE ;
    void stop(){ cancel = true; }
    bool is_rpc_only() const {return rpc_only; }

signals:
    void resultReady(const QString &s);
};


#endif
