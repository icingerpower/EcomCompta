#include <QtCore/qtimer.h>
#include "UpdateToCustomer.h"
#include "ChangeNotifier.h"

//----------------------------------------------------------
UpdateToCustomer::UpdateToCustomer()
{
    /*
    QString customerId = CustomerManager::instance()->getSelectedCustomerId();
    if (!customerId.isEmpty()) {
        m_settingKey = typeid(this).name() + CustomerManager::instance()->getSelectedCustomerId();
    }
    //*/
    m_connection = CustomerManager::instance()->connect(CustomerManager::instance(),
                                         &CustomerManager::selectedCustomerChanged,
                                         [this](const QString &customerId){
        onCustomerSelectedChanged(customerId);
    });
}
//----------------------------------------------------------
UpdateToCustomer::~UpdateToCustomer()
{
    QObject::disconnect(m_connection); //TODO disconnect if CustomerManager was not deleted yet
}
//----------------------------------------------------------
void UpdateToCustomer::callOnCustomerSelectedChanged()
{
    QString customerId = CustomerManager::instance()->getSelectedCustomerId();
    onCustomerSelectedChanged(customerId);
}
//----------------------------------------------------------
QString UpdateToCustomer::settingKey() const
{
    return uniqueId() + CustomerManager::instance()->getSelectedCustomerId();
}
//----------------------------------------------------------
void UpdateToCustomer::init()
{
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChanged(selectedCustomerId);
    }
}
//----------------------------------------------------------

