#ifndef ENTRYPARSERCDISCOUNTPAYMENT_H
#define ENTRYPARSERCDISCOUNTPAYMENT_H

#include "EntryParserOrders.h"

class EntryParserCdiscountPayment
        : public EntryParserOrders
{
public:
    static QString NAME;
    static QString JOURNAL;
    EntryParserCdiscountPayment();
    ~EntryParserCdiscountPayment() override;
    QString name() const override;
    QString journal() const override;
    void clearTransactions() override;
    void recordTransactions(const Shipment *shipmentOrRefund) override;
    AccountingEntries entries(int year) const override;
    Type typeOfEntries() const override;

    static double cdiscountToDouble(QString &string);
private:
    QString _settingKey();
};

#endif // ENTRYPARSERCDISCOUNTPAYMENT_H
