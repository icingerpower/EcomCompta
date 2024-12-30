#ifndef ENTRYPARSERIMPORTATIONSTABLE_H
#define ENTRYPARSERIMPORTATIONSTABLE_H

#include "EntryParserImportations.h"

#include "AbstractEntryParserTable.h"

class EntryParserImportations;

class EntryParserImportationsTable : public AbstractEntryParserTable
{
    Q_OBJECT
public:
    static QString COL_VAT_CONV;
    static QString COL_VAT_ORIG;
    static QString COL_VAT_COUNTRY;
    static QString COL_VAT_CURRENCY;
    static QString COL_COUNTRY_CODE_FROM;
    static QString COL_COUNTRY_CODE_TO;
    EntryParserImportationsTable(QObject *object = nullptr);
    ~EntryParserImportationsTable() override;
    QString name() const override;
    QString journal() const override;
    void fillEntries(AccountingEntries &entries, int year) override;

    void addInvoice(ImportInvoiceInfo &infos);
    void removeInvoices(const QModelIndexList &indexes);

    void removeEntry(int row) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

protected:
    virtual QList<ColInfo> *colInfos() const override;
    QStringList *_editableColNames() const;


private:
    EntryParserImportations *m_entryParserPurchases;
};

#endif // ENTRYPARSERIMPORTATIONSTABLE_H
