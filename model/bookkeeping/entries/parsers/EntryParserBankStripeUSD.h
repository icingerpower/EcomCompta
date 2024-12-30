#ifndef ENTRYPARSERBANKSTRIPEUSD_H
#define ENTRYPARSERBANKSTRIPEUSD_H

#include "EntryParserBankStripe.h"

class EntryParserBankStripeUSD : public EntryParserBankStripe
{
public:
    EntryParserBankStripeUSD();
    ~EntryParserBankStripeUSD() override;

    QString currency() const override;
    QString account() const override;
    QString accountFees() const override;
};

#endif // ENTRYPARSERBANKSTRIPEUSD_H
