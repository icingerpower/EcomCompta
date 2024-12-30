#ifndef ENTRYPARSERAMAZONORDERSMONTHLYTABLE_H
#define ENTRYPARSERAMAZONORDERSMONTHLYTABLE_H

#include "model/bookkeeping/entries/parsers/EntryParserOrdersTable.h"

class EntryParserAmazonOrdersMonthly;

class EntryParserAmazonOrdersMonthlyTable
        : public EntryParserOrdersTable
{
    Q_OBJECT
public:
    EntryParserAmazonOrdersMonthlyTable(QObject *object = nullptr);
    ~EntryParserAmazonOrdersMonthlyTable() override;
    QString journal() const override;
    void fillEntries(AccountingEntries &entries, int year) override;

    EntryParserOrders *saleParser() const override;
    EntryParserAmazonOrdersMonthly *entryParserAmazonOrdersMonthly() const;
    bool displays() const override;

private:
    EntryParserAmazonOrdersMonthly *m_entryParserAmazonOrdersMonthly;
};

#endif // ENTRYPARSERAMAZONORDERSMONTHLYTABLE_H
