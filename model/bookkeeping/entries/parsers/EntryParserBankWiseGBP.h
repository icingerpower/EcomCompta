#ifndef ENTRYPARSERBANKWISEGBP_H
#define ENTRYPARSERBANKWISEGBP_H

#include "EntryParserBankWise.h"

class EntryParserBankWiseGBP : public EntryParserBankWise
{
public:
    EntryParserBankWiseGBP();
    ~EntryParserBankWiseGBP() override;

    QString currency() const override;
    QString account() const override;
    QString accountFees() const override;
};

#endif // ENTRYPARSERBANKWISEGBP_H
