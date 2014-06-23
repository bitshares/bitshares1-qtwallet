#include <QString>
#include <QMenuBar>
#include <bts/blockchain/config.hpp>
#include "MainWindow.hpp"

MainWindow::MainWindow()
: settings("BitShares", BTS_BLOCKCHAIN_NAME)

{
    readSettings();
    initMenu();
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
    _accountMenu = menuBar->addMenu("&Accounts");
    setMenuBar(menuBar);
}
