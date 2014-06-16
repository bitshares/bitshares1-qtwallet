#pragma once

#include <QObject>

class Utilities : public QObject {
    Q_OBJECT

public:
    Utilities(QObject *parent = nullptr) : QObject(parent) {}
    ~Utilities() {}

    Q_INVOKABLE static void copy_to_clipboard(QString string);
};
