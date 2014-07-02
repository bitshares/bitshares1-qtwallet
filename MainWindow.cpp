#include "MainWindow.hpp"

#include <QApplication>
#include <QString>
#include <QMenuBar>
#include <QFileOpenEvent>

#include <bts/blockchain/config.hpp>

MainWindow::MainWindow()
: settings("BitShares", BTS_BLOCKCHAIN_NAME),
  _clientWrapper(nullptr)

{
    readSettings();
    initMenu();
}

#ifdef __APPLE__
bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
    if ( event->type() == QEvent::FileOpen )
    {
        QFileOpenEvent* urlEvent = static_cast<QFileOpenEvent*>(event);
        ilog("Got URL to open: ${url}", ("url", urlEvent->file().toStdString()));
        return true;
    }
    return false;
}
#endif

ClientWrapper *MainWindow::clientWrapper() const
{
    return _clientWrapper;
}

void MainWindow::setClientWrapper(ClientWrapper *clientWrapper)
{
    _clientWrapper = clientWrapper;
}

void MainWindow::goToMyAccounts()
{
    if( !walletIsUnlocked() )
      return;

    getViewer()->loadUrl(_clientWrapper->http_url().toString() + "/#/accounts");
}

void MainWindow::goToCreateAccount()
{
    if( !walletIsUnlocked() )
        return;

    getViewer()->loadUrl(_clientWrapper->http_url().toString() + "/#/create/account");
}

Html5Viewer* MainWindow::getViewer()
{
    return static_cast<Html5Viewer*>(centralWidget());
}

bool MainWindow::walletIsUnlocked()
{
    return _clientWrapper && _clientWrapper->get_client()->get_wallet()->is_unlocked();
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
