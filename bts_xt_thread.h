#ifndef BTSXTTHREAD_H
#define BTSXTTHREAD_H

#include <memory>
#include <QThread>

namespace boost {
    namespace program_options { class variables_map; }
}

namespace bts {
    namespace client { class client; }
}

class BtsXtThread : public QThread
{
    Q_OBJECT
    int _argc;
    char** _argv;
    
public: 
    BtsXtThread(int argc, char** argv)
    : QThread(0), _argc(argc), _argv(argv)
    {
    }
    
    ~BtsXtThread() {}
    
    void run() Q_DECL_OVERRIDE ;
    void stop();

signals:
    void resultReady(const QString &s);
};


#endif
