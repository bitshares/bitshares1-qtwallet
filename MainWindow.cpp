#include "MainWindow.hpp"

#include <QApplication>
#include <QString>
#include <QMenuBar>
#include <QFileOpenEvent>

#include <bts/blockchain/config.hpp>

MainWindow::MainWindow()
: settings("BitShares", BTS_BLOCKCHAIN_NAME)

{
    readSettings();
    initMenu();
}

bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
    ilog("Got event, parsing...");
    if ( event->type() == QEvent::FileOpen )
    {
        QFileOpenEvent* urlEvent = static_cast<QFileOpenEvent*>(event);
        ilog("Got URL to open: ${url}", ("url", urlEvent->url().toString().toStdString()));
        return true;
    }
    ilog("Uninteresting event.");
    return false;
}

ClientWrapper *MainWindow::clientWrapper() const
{
    return _clientWrapper;
}

void MainWindow::setClientWrapper(ClientWrapper *clientWrapper)
{
    _clientWrapper = clientWrapper;
}


void MainWindow::readSettings()
{
    if( settings.contains("geometry") )
    {
        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("windowState").toByteArray());
    }
    else {
        resize(1024,768);
    }
}

void MainWindow::closeEvent( QCloseEvent* event )
{
    settings.setValue("test","bla-bla-bla");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}


void MainWindow::initMenu()
{
    auto menuBar = new QMenuBar(nullptr);

    _fileMenu = menuBar->addMenu("&File");
    _fileMenu->addAction("&Import Wallet")->setEnabled(false);
    _fileMenu->addAction("&Export Wallet")->setEnabled(false);
    _fileMenu->addAction("&Change Password")->setEnabled(false);
#ifndef __APPLE__
    //OSX provides its own Quit menu item which works fine; we don't need to add a second one.
    _fileMenu->addAction("&Quit", qApp, SLOT(quit()));
#endif
    _accountMenu = menuBar->addMenu("&Accounts");
    setMenuBar(menuBar);
}
