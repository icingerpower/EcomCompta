#ifndef SHIPPINGADDRESSESMANAGER_H
#define SHIPPINGADDRESSESMANAGER_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qsharedpointer.h>

#include "model/orderimporters/Address.h"

class ShippingAddressesManager : public QAbstractListModel
{
    Q_OBJECT

public:
    static ShippingAddressesManager *instance();
    Address getAddressCompany() const;
    QString companyCountryName() const;
    QString companyCountryCode() const;
    Address getAddress(int index) const;
    bool contains(const QString &internalId) const;
    Address getAddress(const QString &internalId) const;
    void addAddress(const Address &address);
    void updateAddress(int index, const Address &address);
    void removeAddress(int index);
    void loadAddressesInSettings();
    void saveAddressesInSettings() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void onCustomerSelectedChanged(const QString &customerId);
    void onCustomerSelectedChangedForInit(
            const QString &customerId,
            const QString &settingPrefix);

signals:
    void adressAboutToBeRemoved(const QString &id);
    void adressRemoved(const QString &id);

protected:
    explicit ShippingAddressesManager(QObject *parent = nullptr);
    virtual QString settingKeyPrefix() const;
    void init(const QString &settingPrefix);

private:
    QList<Address> m_addresses;
    QHash<QString, Address> m_addressesById;
    QString m_settingKey;
};

#endif // SHIPPINGADDRESSESMANAGER_H
