#pragma once

#include <QMainWindow>
#include <QSettings>

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QSettings settings;
    
public:
    MainWindow();
    
private:
    void readSettings();
    void closeEvent( QCloseEvent* );
};
