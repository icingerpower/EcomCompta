#ifndef ENTRYPARSERAMAZONPAYMENTSTABLE_H
#define ENTRYPARSERAMAZONPAYMENTSTABLE_H

#include <qstyleditemdelegate.h>

#include "model/bookkeeping/entries/parsers/EntryParserOrdersTable.h"

class EntryParserAmazonPayments;

class EntryParserAmazonPaymentsTable
        : public EntryParserOrdersTable
{
    Q_OBJECT
public:
    static QString COL_AMOUNT_PAID_CONV;
    static QString COL_AMOUNT_PAID_CONV_CURRENCY;
    static QString COL_AMOUNT_SALES_ORIG;
    static QString COL_AMOUNT_FEES_ORIG;
    static QString COL_AMOUNT_RESERVE_PREVIOUS_ORIG;
    static QString COL_AMOUNT_RESERVE_CURRENT_ORIG;
    EntryParserAmazonPaymentsTable(QObject *parent = nullptr);
    ~EntryParserAmazonPaymentsTable() override;
    void onCustomerSelectedChanged(
            const QString &customerId) override;
    QString journal() const override;
    void fillEntries(AccountingEntries &entries, int year) override;

    EntryParserOrders *saleParser() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

protected:
    virtual QList<ColInfo> *colInfos() const override;

private:
    EntryParserAmazonPayments * m_entryParser;
    QStringList *_editableColNames() const;

};

class EntryParserAmazonPaymentsDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit EntryParserAmazonPaymentsDelegate(
            const EntryParserAmazonPaymentsTable *model,
            QObject *parent = nullptr);
    QWidget *createEditor(
            QWidget *parent,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const override;
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;

private:
    const EntryParserAmazonPaymentsTable *m_model;
};

#endif // ENTRYPARSERAMAZONPAYMENTSTABLE_H
