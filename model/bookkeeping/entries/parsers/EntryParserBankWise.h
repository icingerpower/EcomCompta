#ifndef ENTRYPARSERBANKWISE_H
#define ENTRYPARSERBANKWISE_H

#include "EntryParserBank.h"

class EntryParserBankWise
        : public EntryParserBank
{
public:
    EntryParserBankWise();
    ~EntryParserBankWise() override;
    QList<QVariantList> loadValues(
            const QString &absFileName) const override;
    virtual QString currency() const = 0;
    bool invoicesForFees() const override;
    QString name() const override;
    QString journal() const override;
};

#endif // ENTRYPARSERBANKWISE_H
