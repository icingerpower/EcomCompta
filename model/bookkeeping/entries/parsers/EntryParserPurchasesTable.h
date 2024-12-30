#ifndef ENTRYPARSERPURCHASESTABLE_H
#define ENTRYPARSERPURCHASESTABLE_H

#include <qstyleditemdelegate.h>

#include "model/bookkeeping/entries/parsers/AbstractEntryParserTable.h"
#include "model/bookkeeping/entries/parsers/EntryParserPurchases.h"

class EntryParserPurchasesTable : public AbstractEntryParserTable
{
    Q_OBJECT
public:
    static QString COL_VAT_CONV;
    static QString COL_VAT_ORIG;
    static QString COL_VAT_COUNTRY;
    static QString COL_VAT_CURRENCY;
    EntryParserPurchasesTable(QObject *object = nullptr);
    ~EntryParserPurchasesTable() override;
    QString name() const override;
    QString journal() const override;
    void fillEntries(AccountingEntries &entries, int year) override;
    void addInvoice(PurchaseInvoiceInfo &infos);
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
    EntryParserPurchases *m_entryParserPurchases;
};

class TableEntryPurchasesDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit TableEntryPurchasesDelegate(
            const EntryParserPurchasesTable *model,
            QObject *parent = nullptr);
    QWidget *createEditor(
            QWidget *parent,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const override;
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;

private:
    const EntryParserPurchasesTable *m_model;
};

#endif // TABLEENTRYPURCHASES_H
