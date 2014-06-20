#include "html5viewer/html5viewer.h"
#include "ClientWrapper.hpp"
#include "Utilities.hpp"

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

int main( int argc, char** argv )
{
   QCoreApplication::setOrganizationName( "BitShares" );
   QCoreApplication::setOrganizationDomain( "bitshares.org" );
   QCoreApplication::setApplicationName( BTS_BLOCKCHAIN_NAME );
   QApplication app(argc, argv);
   app.setWindowIcon(QIcon(":/images/qtapp.ico"));

   auto menuBar = new QMenuBar(nullptr);
   auto fileMenu = menuBar->addMenu("File");
   fileMenu->addAction("&Import Wallet");
   fileMenu->addAction("&Export Wallet");
   fileMenu->addAction("&Change Password");
   fileMenu->addAction("&Quit");
   menuBar->show();

   QTimer fc_tasks;
   fc_tasks.connect( &fc_tasks, &QTimer::timeout, [](){ fc::usleep( fc::microseconds( 1000 ) ); } );
   fc_tasks.start(33);

   QPixmap pixmap(":/images/splash_screen.jpg");
   QSplashScreen splash(pixmap);
   splash.showMessage(QObject::tr("Loading configuration..."),
                      Qt::AlignCenter | Qt::AlignBottom, Qt::white);
   splash.show();

   QWebSettings::globalSettings()->setAttribute( QWebSettings::PluginsEnabled, false );

   QMainWindow mainWindow;
   auto viewer = new Html5Viewer;
   ClientWrapper client;

   viewer->webView()->page()->settings()->setAttribute( QWebSettings::PluginsEnabled, false );
   viewer->setOrientation(Html5Viewer::ScreenOrientationAuto);
   viewer->webView()->setAcceptHoverEvents(true);
   viewer->webView()->page()->mainFrame()->addToJavaScriptWindowObject("bitshares", &client);
   viewer->webView()->page()->mainFrame()->addToJavaScriptWindowObject("utilities", new Utilities, QWebFrame::ScriptOwnership);

   mainWindow.resize(1200,800);
   mainWindow.setCentralWidget(viewer);

   QObject::connect(viewer->webView()->page()->networkAccessManager(), &QNetworkAccessManager::authenticationRequired,
                    [&client](QNetworkReply*, QAuthenticator *auth) {
      auth->setUser(client.http_url().userName());
      auth->setPassword(client.http_url().password());
   });
   client.connect(&client, &ClientWrapper::initialized, [&viewer,&client]() {
      ilog( "Client initialized; loading web interface from ${url}", ("url", client.http_url().toString().toStdString()) );
      viewer->webView()->load(client.http_url());
   });
   viewer->connect(viewer->webView(), &QGraphicsWebView::loadFinished, [&mainWindow,&splash](bool ok) {
      ilog( "Webview loaded: ${status}", ("status", ok) );
      mainWindow.show();
      splash.finish(&mainWindow);
   });
   client.connect(&client, &ClientWrapper::error, [&](QString errorString) {
      splash.showMessage(errorString, Qt::AlignCenter | Qt::AlignBottom, Qt::white);
      fc::usleep( fc::seconds(3) );
   });

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
