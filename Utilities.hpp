#pragma once

#include <QObject>
#include <QUrl>

class Utilities : public QObject {
    Q_OBJECT

public:
    Utilities(QObject *parent = nullptr) : QObject(parent) {}
    ~Utilities() {}

    Q_INVOKABLE static void copy_to_clipboard(const QString& string);
    Q_INVOKABLE static void open_in_external_browser(const QString& url);
    Q_INVOKABLE static void open_in_external_browser(const QUrl& url);
    Q_INVOKABLE static QString prompt_user_to_open_file(const QString& dialogCaption);
    Q_INVOKABLE static void log_message(const QString& message);
};
