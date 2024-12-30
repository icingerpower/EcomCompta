#ifndef TABLEBANKBEGINBALANCES_H
#define TABLEBANKBEGINBALANCES_H

#include <QAbstractTableModel>

#include "model/UpdateToCustomer.h"

class TableBankBeginBalances : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT

public:
    static TableBankBeginBalances *instance();
    double getBeginAmount(const QString &bankName) const;

    void onCustomerSelectedChanged(const QString &customerId) override;
    QString uniqueId() const override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    explicit TableBankBeginBalances(QObject *parent = nullptr);
    QList<double> m_amounts;
    QStringList m_bankNames;
    QHash<QString, int> m_bankNamesToIndex;
    void saveInSettings() const;
    void loadFromSettings();
    void _clear();
    QString settingsKeyBankNames() const;
    QString settingsKeyBalances() const;
};

#endif // TABLEBANKBEGINBALANCES_H
