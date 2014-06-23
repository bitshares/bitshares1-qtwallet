#pragma once

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
    
private:
    void readSettings();
    virtual void closeEvent( QCloseEvent* );
    void initMenu();
};
