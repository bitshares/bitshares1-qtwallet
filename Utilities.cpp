#include "Utilities.hpp"

#include <fc/log/logger.hpp>

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <qglobal.h>

QUuid Utilities::app_id;

void Utilities::copy_to_clipboard(const QString& string)
{
    qApp->clipboard()->setText(string);
}

void Utilities::open_in_external_browser(const QString& url)
{
    QDesktopServices::openUrl(QUrl(url));
}

void Utilities::open_in_external_browser(const QUrl& url)
{
    QDesktopServices::openUrl(url);
}

QString Utilities::prompt_user_to_open_file(const QString& dialogCaption)
{
    return QFileDialog::getOpenFileName(nullptr, dialogCaption, QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first());
}


void Utilities::log_message(const QString& message)
{
    wlog("Message from GUI: ${msg}", ("msg",message.toStdString()));
}

QString Utilities::get_app_id()
{
    return app_id.toString().mid(1,36);
}

QString Utilities::get_os_name()
{
  return 
#ifdef Q_OS_LINUX
  "linux"
#elif defined(Q_OS_WIN32)
  "windows"
#elif defined(Q_OS_MAC)
  "mac"
#else
  "unknown"
#endif
  ;
}
