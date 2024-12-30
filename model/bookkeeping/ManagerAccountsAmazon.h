#ifndef MANAGERACCOUNTSAMAZON_H
#define MANAGERACCOUNTSAMAZON_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qset.h>

#include "model/UpdateToCustomer.h"

struct AmazonAccounts{
    QString client;
    QString supplier;
    QString reserve;
    QString salesUnknown;
    QString fees;
};

class ManagerAccountsAmazon : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT

public:
    static ManagerAccountsAmazon *instance();
    void onCustomerSelectedChanged(const QString &customerId) override;
    AmazonAccounts amazonAccount(const QString &amazon);
    void addAmazon(const QString &amazon);
    void addAmazon(const QString &amazon,
                   const QString &accountCustomer,
                   const QString &accountReserve,
                   const QString &accountUnknownSales,
                   const QString &fsupplier,
                   const QString &accountCharges);
    void remove(const QModelIndex &index);
    void saveInSettings() const;
    void loadFromSettings();
    QString uniqueId() const override;
    QSet<QString> allBalanceAccounts() const;
    QSet<QString> allSupplierAccounts() const;
    QSet<QString> allCustomerAccounts() const;

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


private:
    explicit ManagerAccountsAmazon(QObject *parent = nullptr);
    QMap<QString, QStringList> m_values;
    void _clear();
    void _generateBasicAccounts();
};

#endif // MANAGERACCOUNTSAMAZON_H
