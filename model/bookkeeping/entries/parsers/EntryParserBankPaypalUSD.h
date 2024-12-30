#ifndef ENTRYPARSERBANKPAYPALUSD_H
#define ENTRYPARSERBANKPAYPALUSD_H

#include "EntryParserBankPaypal.h"

class EntryParserBankPaypalUSD : public EntryParserBankPaypal
{
public:
    EntryParserBankPaypalUSD();
    ~EntryParserBankPaypalUSD() override;

    QString currency() const override;
    QString account() const override;
    QString accountFees() const override;
};

#endif // ENTRYPARSERBANKPAYPALUSD_H
