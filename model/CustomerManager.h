#ifndef CUSTOMERMANAGER_H
#define CUSTOMERMANAGER_H

#include <QtCore/qhash.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qabstractitemmodel.h>
#include "Customer.h"

class CustomerManager : public QAbstractListModel
{
    Q_OBJECT
public:
    static CustomerManager *instance();
    void loadCustomers();
    void selectCustomer(const QString &customerName);
    QString getSelectedCustomerId() const;
    QString getSelectedCustomerCurrency() const;
    QString getSelectedCustomerCountryCode() const;
    QString getSelectedCustomer() const;
    bool isCustomerAvailable(const QString &name) const;
    void addCustomerAvailable(const Customer &customer);
    void editCustomer(const QString &currentName, const Customer &customer);
    void removeCustomer(const QString &name);
    void removeCustomer(int index);
    QHash<QString, Customer> getCustomers();
    Customer getCustomer(int index);
    Customer getCustomer(const QString &name);
    QStringList getCustomerNames();
    int nCustomers() const;

    int	rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role) const override;

signals:
    void selectedCustomerChanged(const QString &customerId);

private:
    const char* SET_CUSTOMERS = "customers";
    const char* SET_CUSTOMER_SELECTED = "customerSelected";
    CustomerManager(QObject *object = nullptr);
    QHash<QString, Customer> m_customers;
    QStringList m_customerNames;
    QString m_selectedCustomer;
    void saveCustomers();
};

#endif // CUSTOMERMANAGER_H
