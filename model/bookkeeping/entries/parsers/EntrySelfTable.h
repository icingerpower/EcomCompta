#ifndef ENTRYSELFTABLE_H
#define ENTRYSELFTABLE_H


#include <QtCore/qabstractitemmodel.h>

#include "model/UpdateToCustomer.h"

struct EntrySelfInfo{
    QString id;
    QString title;
    QString account;
};

class EntrySelfTable : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT
public:
    static EntrySelfTable *instance();
    ~EntrySelfTable() override;

    QString uniqueId() const override;
    EntrySelfInfo account(int pos) const;
    EntrySelfInfo account(const QString &id) const;
    void addAccount(const QString &title, const QString &account);
    void removeAccount(int index);

    void loadFromSettings();
    void saveInSettings() const;

    void onCustomerSelectedChanged(const QString &customerId) override;
    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

protected:
    EntrySelfTable(QObject *object = nullptr);
    QList<QStringList> m_values;
    QHash<QString, int> m_idToIndex;
    void _clear();
};

#endif // ENTRYSELFTABLE_H
