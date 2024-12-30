#ifndef ENTRYPARSERBANKWISEEUR_H
#define ENTRYPARSERBANKWISEEUR_H

#include "EntryParserBankWise.h"

class EntryParserBankWiseEUR : public EntryParserBankWise
{
public:
    EntryParserBankWiseEUR();
    ~EntryParserBankWiseEUR() override;

    QString currency() const override;
    QString account() const override;
    QString accountFees() const override;
};

#endif // ENTRYPARSERBANKWISEEUR_H
