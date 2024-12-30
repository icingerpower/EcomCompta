#ifndef MANAGERACCOUNTSSTOCKDEPORTED_H
#define MANAGERACCOUNTSSTOCKDEPORTED_H

#include <QAbstractTableModel>

#include "model/UpdateToCustomer.h"

class ManagerAccountsStockDeported : public QAbstractTableModel, public UpdateToCustomer

{
    Q_OBJECT

public:
    static QStringList COL_NAMES;
    static int IND_COL_ACCOUNT_6_IMPORTED;
    static int IND_COL_ACCOUNT_7_EXPORTED;
    static QString KEY_ACCOUNT_4;
    static QString KEY_ACCOUNT_4_VAT_TO_PAY;
    static QString KEY_ACCOUNT_4_VAT_DEDUCTIBLE;
    static QString KEY_ACCOUNT_4_OUTSIDE_COUNTRY;
    static ManagerAccountsStockDeported *instance();

    QString getAccountImportedFromUe(const QString &countryCode) const;
    QString getAccountExportedToUe(const QString &countryCode) const;
    QString getAccount4();
    QString getAccount4VatToPay();
    QString getAccount4VatDeductible();
    QString getAccount4OutsideCountry();

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

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

public slots:
    void setAccount4(const QString &account);
    void setAccount4VatToPay(const QString &account);
    void setAccount4VatDeductible(const QString &account);
    void setAccount4OutsideCountry(const QString &account);

private:
    explicit ManagerAccountsStockDeported(
            QObject *parent = nullptr);
    QList<QStringList> m_listOfStringList;
    QHash<QString, int> m_countryCodeToIndex;
    void _clear();
    void saveInSettings() const;
    void loadFromSettings();
};

#endif // MANAGERACCOUNTSSTOCKDEPORTED_H
