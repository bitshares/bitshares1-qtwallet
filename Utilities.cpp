#include "Utilities.hpp"

#include <fc/log/logger.hpp>

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>

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
