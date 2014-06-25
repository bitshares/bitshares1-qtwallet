#pragma once

#include "ClientWrapper.hpp"

#include <QMainWindow>
#include <QSettings>
#include <QMenu>

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QSettings settings;
    QMenu* _fileMenu;
    QMenu* _accountMenu;
    
public:
    MainWindow();
    QMenu* fileMenu() { return _fileMenu; }
    QMenu* accountMenu() { return _accountMenu; }

    bool eventFilter(QObject* object, QEvent* event);
    
    ClientWrapper *clientWrapper() const;
    void setClientWrapper(ClientWrapper *clientWrapper);

private:
    ClientWrapper* _clientWrapper;

    void readSettings();
    virtual void closeEvent( QCloseEvent* );
    void initMenu();
};
