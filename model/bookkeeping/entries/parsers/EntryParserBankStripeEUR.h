#ifndef ENTRYPARSERBANKSTRIPEEUR_H
#define ENTRYPARSERBANKSTRIPEEUR_H

#include "EntryParserBankStripe.h"

class EntryParserBankStripeEUR : public EntryParserBankStripe
{
public:
    EntryParserBankStripeEUR();
    ~EntryParserBankStripeEUR() override;

    QString currency() const override;
    QString account() const override;
    QString accountFees() const override;
};

#endif // ENTRYPARSERBANKSTRIPEEUR_H
