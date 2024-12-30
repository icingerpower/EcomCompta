#ifndef MANAGERACCOUNTSSALESRARES_H
#define MANAGERACCOUNTSSALESRARES_H

#include <QtCore/qabstractitemmodel.h>

#include "model/UpdateToCustomer.h"

class ManagerAccountsSalesRares : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT
public:
    struct AccountsRare{
        QString number;
        QString title;
    };
    static ManagerAccountsSalesRares *instance();
    ~ManagerAccountsSalesRares() override;
    void onCustomerSelectedChanged(const QString &customerId) override;
    QString uniqueId() const override;
    void addRow(const QString &subChannel,
                const QString &sku,
                const QString &number,
                const QString &title);
    void removeRow(int index);
    void saveInSettings() const;
    void loadFromSettings();
    AccountsRare getAccounts(
            const QString &subChannel,
            const QString &sku,
            const QString &defaultNumber,
            const QString &defaultTitle) const;

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
    explicit ManagerAccountsSalesRares(QObject *parent = nullptr);
    QList<QStringList> m_values;
    /// Canal-vente     sku            account   title
    QHash<QString, QHash<QString, QPair<QString, QString>>> m_mapping;
    void _clear();
    void _generateBasicAccounts();
    void _generateMapping();
};

#endif // MANAGERACCOUNTSSALESRARES_H
