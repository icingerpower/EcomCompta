#ifndef ENTRYPARSERBANKSTRIPE_H
#define ENTRYPARSERBANKSTRIPE_H

#include "EntryParserBank.h"

class EntryParserBankStripe : public EntryParserBank
{
public:
    EntryParserBankStripe();
    ~EntryParserBankStripe() override;
    QList<QVariantList> loadValues(
            const QString &absFileName) const override;
    virtual QString currency() const = 0;
    bool invoicesForFees() const override;
    QString name() const override;
    QString journal() const override;
};

#endif // ENTRYPARSERBANKSTRIPE_H
