#ifndef CHANGENOTIFIER_H
#define CHANGENOTIFIER_H

#include <QtCore/QObject>

class ChangeNotifier : public QObject
{
    Q_OBJECT
public:
    static ChangeNotifier *instance();
    void emitDataForVatChanged();

signals:
    void dataForVatChanged();
private:
    ChangeNotifier(QObject *parent = nullptr);
};

#endif // CHANGENOTIFIER_H
