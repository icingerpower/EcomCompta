#ifndef MANAGERACCOUNTSVATPAYMENTS_H
#define MANAGERACCOUNTSVATPAYMENTS_H

#include <QAbstractTableModel>

#include "model/UpdateToCustomer.h"

class ManagerAccountsVatPayments : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT

public:
    static QStringList COL_NAMES;
    static int IND_COL_ACCOUNT;
    static int IND_COL_VAT_REGIME;
    static ManagerAccountsVatPayments *instance();

    QString getAccount(const QString &vatRegime);

    // UpdateToCustomer
    void onCustomerSelectedChanged(const QString &customerId) override;
    QString uniqueId() const override;

    // Header:
    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    explicit ManagerAccountsVatPayments(
            QObject *parent = nullptr);
    QList<QStringList> m_listOfStringList;
    void _clear();
    void saveInSettings() const;
    void loadFromSettings();
};

#endif // MANAGERACCOUNTSVATPAYMENTS_H
