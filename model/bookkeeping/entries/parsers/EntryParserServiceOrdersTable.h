#ifndef ENTRYPARSERSERVICEORDERSTABLE_H
#define ENTRYPARSERSERVICEORDERSTABLE_H

#include "EntryParserOrdersTable.h"

class EntryParserServiceOrders;

class EntryParserServiceOrdersTable
        : public EntryParserOrdersTable
{
    Q_OBJECT
public:
    EntryParserServiceOrdersTable(QObject *object = nullptr);
    ~EntryParserServiceOrdersTable() override;
    QString journal() const override;
    void fillEntries(AccountingEntries &entries, int year) override;

    EntryParserOrders *saleParser() const override;

private:
    EntryParserServiceOrders *m_entryParser;
};

#endif // ENTRYPARSERSERVICEORDERSTABLE_H
