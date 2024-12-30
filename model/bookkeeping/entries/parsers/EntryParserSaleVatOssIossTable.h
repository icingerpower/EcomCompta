#ifndef ENTRYPARSERSALEVATOSSIOSSTABLE_H
#define ENTRYPARSERSALEVATOSSIOSSTABLE_H

#include "EntryParserOrdersTable.h"

class EntryParserSaleVatOssIoss;

class EntryParserSaleVatOssIossTable
        : public EntryParserOrdersTable
{
    Q_OBJECT
public:
    EntryParserSaleVatOssIossTable(QObject *object = nullptr);
    ~EntryParserSaleVatOssIossTable() override;
    QString journal() const override;
    void fillEntries(AccountingEntries &entries, int year) override;
    bool displays() const override;

    EntryParserOrders *saleParser() const override;

private:
    EntryParserSaleVatOssIoss *m_entryParser;

};

#endif // ENTRYPARSERSALEVATOSSIOSSTABLE_H
