#ifndef ENTRYPARSERCDISCOUNTPAYMENTTABLE_H
#define ENTRYPARSERCDISCOUNTPAYMENTTABLE_H

#include "model/bookkeeping/entries/parsers/EntryParserOrdersTable.h"

class EntryParserCdiscountPayment;

class EntryParserCdiscountPaymentTable
        : public EntryParserOrdersTable
{
    Q_OBJECT
public:
    EntryParserCdiscountPaymentTable(QObject *object = nullptr);
    ~EntryParserCdiscountPaymentTable() override;
    QString journal() const override;
    void fillEntries(AccountingEntries &entries, int year) override;

    EntryParserOrders *saleParser() const override;
    EntryParserCdiscountPayment *entryParserCdiscountPayment() const;

private:
    EntryParserCdiscountPayment *m_entryParser;
};

#endif // ENTRYPARSERCDISCOUNTPAYMENTTABLE_H
