#ifndef ENTRYSAVERIBIZA_H
#define ENTRYSAVERIBIZA_H


#include "AbstractEntrySaver.h"

class EntrySaverIbiza : public AbstractEntrySaver
{
public:
    EntrySaverIbiza();
    QString name() const override;
    void save(const AccountingEntries &entries,
                      const QDir &dir) override;
};

#endif // ENTRYSAVERIBIZA_H
