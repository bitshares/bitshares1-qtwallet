#include "html5viewer/html5viewer.h"
#include "bts_xt_thread.h"

#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <fc/thread/thread.hpp>
#include <fc/filesystem.hpp>
#include <bts/blockchain/config.hpp>
#include <signal.h>

#include <QApplication>

bool exit_signal = false;
std::shared_ptr<BtsXtThread> btsxt;

void handle_signal( int signum )
{
    std::cout<< "Signal " << signum << " caught. exiting.." << std::endl;
    exit_signal = true;
    btsxt->stop();
}

int main( int argc, char** argv )
{
    signal(SIGABRT, &handle_signal);
    signal(SIGTERM, &handle_signal);
    signal(SIGINT, &handle_signal);
    
    btsxt = std::make_shared<BtsXtThread>(argc, argv);
    btsxt->start();
    
    QString initial_url = "http://127.0.0.1:5680";
    //if(!fc::exists( datadir / "default_wallet.dat" ))
    //    initial_url = "http://127.0.0.1:5680/blank.html#/createwallet";
        
        QApplication app(argc, argv);
        Html5Viewer viewer;
        viewer.setOrientation(Html5Viewer::ScreenOrientationAuto);
        viewer.resize(1024,648);
        viewer.show();
        QUrl url = QUrl(initial_url);
        url.setUserName("");
        url.setPassword("");
        viewer.loadUrl(url);
        app.exec();    
  
    btsxt->stop();
    btsxt->wait();
    
    return 0;
}
