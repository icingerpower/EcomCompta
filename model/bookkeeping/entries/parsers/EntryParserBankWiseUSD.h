#ifndef ENTRYPARSERBANKWISEUSD_H
#define ENTRYPARSERBANKWISEUSD_H

#include "EntryParserBankWise.h"

class EntryParserBankWiseUSD : public EntryParserBankWise
{
public:
    EntryParserBankWiseUSD();
    ~EntryParserBankWiseUSD() override;

    QString currency() const override;
    QString account() const override;
    QString accountFees() const override;
};


#endif // ENTRYPARSERBANKWISEUSD_H
