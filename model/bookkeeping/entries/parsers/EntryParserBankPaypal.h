#ifndef ENTRYPARSERBANKPAYPAL_H
#define ENTRYPARSERBANKPAYPAL_H

#include "EntryParserBank.h"

class EntryParserBankPaypal
        : public EntryParserBank
{
public:
    EntryParserBankPaypal();
    ~EntryParserBankPaypal() override;
    QList<QVariantList> loadValues(
            const QString &absFileName) const override;
    virtual QString currency() const = 0;
    bool invoicesForFees() const override;
    QString name() const override;
    QString journal() const override;

};

#endif // ENTRYPARSERBANKPAYPAL_H
