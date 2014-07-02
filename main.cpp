#include "html5viewer/html5viewer.h"
#include "ClientWrapper.hpp"
#include "Utilities.hpp"
#include "MainWindow.hpp"

#include <boost/thread.hpp>
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
#include <QWebFrame>
#include <QJsonDocument>
#include <QGraphicsWebView>
#include <QTimer>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QResource>
#include <QGraphicsWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QLayout>

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
#include <fc/signals.hpp>

#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/stream.hpp>

#include <fstream>
#include <iostream>
#include <iomanip>

#ifdef NDEBUG
#include "config_prod.hpp"
#else
#include "config_dev.hpp"
#endif

void setupMenus(ClientWrapper* client, MainWindow* mainWindow)
{
    auto accountMenu = mainWindow->accountMenu();

    accountMenu->addAction("&Go to My Accounts", mainWindow, SLOT(goToMyAccounts()));
    accountMenu->addAction("&Create Account", mainWindow, SLOT(goToCreateAccount()));
    accountMenu->addAction("&Import Account")->setEnabled(false);
}

void prepareStartupSequence(ClientWrapper* client, Html5Viewer* viewer, MainWindow* mainWindow, QSplashScreen* splash)
{
    viewer->webView()->page()->mainFrame()->addToJavaScriptWindowObject("bitshares", client);
    viewer->webView()->page()->mainFrame()->addToJavaScriptWindowObject("magic_unicorn", new Utilities, QWebFrame::ScriptOwnership);

    QObject::connect(viewer->webView()->page()->networkAccessManager(), &QNetworkAccessManager::authenticationRequired,
                     [client](QNetworkReply*, QAuthenticator *auth) {
       auth->setUser(client->http_url().userName());
       auth->setPassword(client->http_url().password());
    });
    client->connect(client, &ClientWrapper::initialized, [viewer,client,mainWindow]() {
       ilog( "Client initialized; loading web interface from ${url}", ("url", client->http_url().toString().toStdString()) );
       client->status_update("Calculating last 3 digits of pi");
       viewer->webView()->load(client->http_url());
       //Now we know the URL of the app, so we can create the items in the Accounts menu
       setupMenus(client, mainWindow);
    });
    auto loadFinishedConnection = std::make_shared<QMetaObject::Connection>();
    *loadFinishedConnection = viewer->connect(viewer->webView(), &QGraphicsWebView::loadFinished, [mainWindow,splash,viewer,loadFinishedConnection](bool ok) {
       ilog( "Webview loaded: ${status}", ("status", ok) );
       viewer->disconnect(*loadFinishedConnection);
       mainWindow->show();
       splash->finish(mainWindow);
       mainWindow->processDeferredUrl();
    });
    client->connect(client, &ClientWrapper::error, [=](QString errorString) {
       splash->showMessage(errorString, Qt::AlignCenter | Qt::AlignBottom, Qt::red);
       fc::usleep( fc::seconds(3) );
       qApp->exit(1);
    });
    client->connect(client, &ClientWrapper::status_update, [=](QString messageString) {
       splash->showMessage(messageString, Qt::AlignCenter | Qt::AlignBottom, Qt::white);
    });
}

int main( int argc, char** argv )
{
   QCoreApplication::setOrganizationName( "BitShares" );
   QCoreApplication::setOrganizationDomain( "bitshares.org" );
   QCoreApplication::setApplicationName( BTS_BLOCKCHAIN_NAME );
   QApplication app(argc, argv);
   app.setWindowIcon(QIcon(":/images/qtapp.ico"));

   MainWindow mainWindow;
   auto viewer = new Html5Viewer;
   ClientWrapper client;

#ifdef NDEBUG
   app.connect(&app, &QApplication::aboutToQuit, [&client](){
       client.get_client()->get_wallet()->close();
       client.get_client()->get_chain()->close();
       exit(0);
   });
#endif

   mainWindow.setCentralWidget(viewer);
   mainWindow.setClientWrapper(&client);

#ifdef __APPLE__
   //Install OSX event handler
   ilog("Installing URL open event filter");
   app.installEventFilter(&mainWindow);
#endif

   QTimer fc_tasks;
   fc_tasks.connect( &fc_tasks, &QTimer::timeout, [](){ fc::usleep( fc::microseconds( 1000 ) ); } );
   fc_tasks.start(33);

   QPixmap pixmap(":/images/splash_screen.jpg");
   QSplashScreen splash(pixmap);
   splash.showMessage(QObject::tr("Loading configuration..."),
                      Qt::AlignCenter | Qt::AlignBottom, Qt::white);
   splash.show();

   prepareStartupSequence(&client, viewer, &mainWindow, &splash);

   QWebSettings::globalSettings()->setAttribute( QWebSettings::PluginsEnabled, false );

   try {
    client.initialize();
    return app.exec();
   }
   catch ( const fc::exception& e) 
   {
      elog( "${e}", ("e",e.to_detail_string() ) );
      QErrorMessage::qtHandler()->showMessage( e.to_string().c_str() );
   }
}
