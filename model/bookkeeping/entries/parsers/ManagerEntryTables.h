#ifndef MANAGERENTRYTABLE_H
#define MANAGERENTRYTABLE_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qmap.h>

#include "model/UpdateToCustomer.h"
#include "AbstractEntryParser.h"

class AbstractEntryParserTable;
class EntryParserBankTable;
class EntryParserPurchasesTable;
class AbstractEntryParserTable;
class EntryParserOrdersTable;
class EntryParserAmazonPayments;
class EntryParserImportationsTable;

class ManagerEntryTables : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT
public:
    static ManagerEntryTables *instance();
    ~ManagerEntryTables() override;
    void onCustomerSelectedChanged(const QString &customerId) override;
    QString uniqueId() const override;

    QStringList namesBank() const;
    QStringList namesSale() const;
    QString namePurchases() const;
    QMap<QString, EntryParserBankTable*> entryDisplayBanks() const;
    QMap<QString, EntryParserOrdersTable*> entryDisplaySale() const;
    EntryParserPurchasesTable *entryDisplayPurchase() const;
    EntryParserImportationsTable *entryDisplayImportations() const;
    const EntryParserAmazonPayments *entryParserAmazonPayments() const;
    QString journalName(const QString &entryTypeName) const;
    void load(int year);
    AccountingEntries entries(int year) const;

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

signals:
    void bankTypeAdded(const QString &bankName);
    void saleTypeAdded(const QString &bankName);

protected:
    ManagerEntryTables(QObject *parent = nullptr);
    void saveFromSettings();
    void loadFromSettings();
    void _clear();
    QMap<QString, EntryParserBankTable*> m_entryDisplayBanks;
    QMap<QString, EntryParserOrdersTable*> m_entryDisplaySales;
    QMap<QString, AbstractEntryParserTable*> m_entryDisplayVarious;
    EntryParserPurchasesTable* m_entryDisplayPurchase;
    EntryParserImportationsTable* m_entryDisplayImportation;
    QList<QStringList> m_journalNames;
    QHash<QString, QString> m_journalNamesHash;
};

#endif // MANAGERENTRYDISPLAY_H
