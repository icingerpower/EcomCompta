#ifndef MANAGERACCOUNTSSALES_H
#define MANAGERACCOUNTSSALES_H

#include <QtCore/qabstractitemmodel.h>

#include "model/UpdateToCustomer.h"

class ManagerAccountsSales : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT
public:
    struct Accounts{
        QString saleAccount;
        QString vatAccount;
        QString titleBase;
    };
    static QString SALE_PRODUCTS;
    static QString SALE_SERVICES;
    static QString SALE_PAYMENT_FASCILITOR;
    static ManagerAccountsSales *instance();
    ~ManagerAccountsSales() override;
    void onCustomerSelectedChanged(const QString &customerId) override;
    QString uniqueId() const override;
    void addRow(const QString &regime,
                const QString &country,
                const QString &saleType,
                const QString &vatRate,
                const QString &accountSale,
                const QString &accountVat);
    void removeRow(int index);
    void saveInSettings() const;
    void loadFromSettings();
    Accounts getAccounts(const QString &regime,
                         const QString &countryCode,
                         const QString &saleType,
                         const QString &vatRate) const;

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
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;


private:
    explicit ManagerAccountsSales(QObject *parent = nullptr);
    QList<QStringList> m_values;
    QHash<QString, QHash<QString, QHash<QString, QHash<QString, int>>>> m_mapping;
    void _clear();
    void _generateBasicAccounts();
    void _generateMapping();
};
#endif // MANAGERACCOUNTSSALES_H
