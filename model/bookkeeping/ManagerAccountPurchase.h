#ifndef MANAGERACCOUNTPURCHASE_H
#define MANAGERACCOUNTPURCHASE_H

#include <QtCore/qabstractitemmodel.h>

#include "model/UpdateToCustomer.h"

class ManagerAccountPurchase : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT
public:
    static ManagerAccountPurchase *instance();

    void onCustomerSelectedChanged(const QString &customerId) override;
    QString uniqueId() const override;

    QString accountVat(const QString &countryName) const;
    void addAccount(const QString &country, const QString &account);
    void removeAccount(const QModelIndex &index);
    void saveInSettings() const;
    void loadFromSettings();

    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;
    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

protected:
    QList<QStringList> m_accounts;
    QHash<QString, QString> m_accountsByCountry;
    ManagerAccountPurchase(QObject *parent = nullptr);
    void _clear();
};

#endif // MANAGERACCOUNTPURCHASE_H
