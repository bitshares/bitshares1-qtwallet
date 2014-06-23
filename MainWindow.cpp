#include <QString>
#include <bts/blockchain/config.hpp>
#include "MainWindow.hpp"

MainWindow::MainWindow()
: settings("BitShares", BTS_BLOCKCHAIN_NAME)

{
    readSettings();
}

void MainWindow::readSettings()
{
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::closeEvent( QCloseEvent* event )
{
    settings.setValue("test","bla-bla-bla");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}
