#ifndef UPDATETOCUSTOMER_H
#define UPDATETOCUSTOMER_H

#include "model/CustomerManager.h"

class UpdateToCustomer
{
public:
    UpdateToCustomer();
    virtual ~UpdateToCustomer();
    virtual void onCustomerSelectedChanged(
            const QString &customerId) = 0;
    void callOnCustomerSelectedChanged();
    virtual QString uniqueId() const = 0;
    //QString uniqueId() const override;
    QString settingKey() const;
    void init();

private:
    QString m_settingKey;
    QMetaObject::Connection m_connection;

};

#endif // UPDATETOCUSTOMER_H
