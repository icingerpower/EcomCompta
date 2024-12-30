#include "ChangeNotifier.h"

//----------------------------------------------------------
ChangeNotifier *ChangeNotifier::instance()
{
    static ChangeNotifier instance;
    return &instance;
}
//----------------------------------------------------------
void ChangeNotifier::emitDataForVatChanged()
{
    emit dataForVatChanged();
}
//----------------------------------------------------------
ChangeNotifier::ChangeNotifier(QObject *parent)
    : QObject(parent)
{

}
//----------------------------------------------------------
