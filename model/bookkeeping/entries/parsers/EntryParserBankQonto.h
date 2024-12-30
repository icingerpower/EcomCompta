#ifndef ENTRYPARSERBANKQONTO_H
#define ENTRYPARSERBANKQONTO_H

#include "EntryParserBank.h"

class EntryParserBankQonto
        : public EntryParserBank
{
public:
    EntryParserBankQonto();
    ~EntryParserBankQonto() override;
    QList<QVariantList> loadValues(
            const QString &absFileName) const override;
    QString account() const override;
    QString accountFees() const override;
    bool invoicesForFees() const override;
    QString name() const override;
    QString journal() const override;
};

#endif // ENTRYPARSERBANKQONTO_H
