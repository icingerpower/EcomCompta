#ifndef ENTRYPARSERBANKPAYPALEUR_H
#define ENTRYPARSERBANKPAYPALEUR_H

#include "EntryParserBankPaypal.h"

class EntryParserBankPaypalEUR : public EntryParserBankPaypal
{
public:
    EntryParserBankPaypalEUR();
    ~EntryParserBankPaypalEUR() override;

    QString currency() const override;
    QString account() const override;
    QString accountFees() const override;
};

#endif // ENTRYPARSERBANKPAYPALEUR_H
