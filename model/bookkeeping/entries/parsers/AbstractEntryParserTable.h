#ifndef ABSTRACTENTRYPARSERTABLE_H
#define ABSTRACTENTRYPARSERTABLE_H


#include <QtCore/qdir.h>
#include <QtCore/qmap.h>
#include <QtCore/qset.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtCore/QModelIndexList>

#include "model/UpdateToCustomer.h"
#include "model/bookkeeping/entries/AccountingEntrySet.h"

class AccountingEntrySet;
//TODO one table for sales, one for purchase, one for banks, one for no connection entry of banks

class AbstractEntryParserTable : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT
public:
    static QString COL_ID;
    static QString COL_DATE;
    static QString COL_JOURNAL;
    static QString COL_ACCOUNT_1;
    static QString COL_ACCOUNT_2;
    static QString COL_AMOUNT_CONV;
    static QString COL_AMOUNT_ORIG;
    static QString COL_CURRENCY_ORIG;
    static QString COL_LABEL;
    static QString COL_FILE_NAME_REL;
    AbstractEntryParserTable(QObject *object);
    ~AbstractEntryParserTable() override;
    void onCustomerSelectedChanged(
            const QString &customerId) override;

    QString uniqueId() const override;
    virtual QString name() const = 0;
    virtual QString journal() const = 0;
    virtual void fillEntries(AccountingEntries &entries, int year) = 0;
    virtual bool displays() const;

    QSharedPointer<AccountingEntrySet> entrySet(int index) const;
    QSharedPointer<AccountingEntrySet> entrySet(
            const QModelIndex & index) const;
    void addEntrySet(QSharedPointer<AccountingEntrySet> entrySet);
    void removeEntry(const QModelIndexList &indexes);
    virtual void removeEntry(int row);
    void load(int year);
    const AccountingEntries *entries() const;
    void clear();
    /*
    QString associate(QModelIndexList &selectedRows,
                   QList<QSharedPointer<AccountingEntrySet> > *entrySet);
    void dissociate(QSharedPointer<AccountingEntrySet> entrySet);
    void dissociate(const QModelIndex &index);
    //*/
    double sumEntry(QSet<QString> ids) const;

    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;


protected:
    struct ColInfo {
        QString name;
        QVariant (*getValue)(const AbstractEntryParserTable *table, const AccountingEntrySet *entrySet);
    };
    virtual QList<ColInfo> *colInfos() const;
    const QList<QSharedPointer<AccountingEntrySet>> *_entrySets() const;
    void _setEntrySet(int index, QSharedPointer<AccountingEntrySet> entrySet);
    AccountingEntries m_entries;
    //QList<QStringList> m_values;
    QHash<QString, QSharedPointer<AccountingEntrySet>> m_entryById;
    QHash<AccountingEntrySet *, int> m_entrySetPositions;
    void _emitChangeRowsOfEntrySetIds(const QString &id);
    void _emitChangeRowsOfEntrySetIds(const QSet<QString> &ids);
    void _addAssociatedIds(const QSet<QString> &ids);
    void _removeAssociatedIds(const QSet<QString> &ids);
    //void _saveAssociations();
    //void _loadAssociations();
    void _generateHash();
    //QStringList _entrySetToList(AccountingEntrySet *entrySet);

private:
    QList<QSharedPointer<AccountingEntrySet>> m_entrySets;
    QList<QMetaObject::Connection> m_connections;
};

#endif // ABSTRACTTABLEENTRYDISPLAY_H
