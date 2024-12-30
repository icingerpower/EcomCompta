#ifndef ABSTRACTENTRYPARSER_H
#define ABSTRACTENTRYPARSER_H

#include "model/bookkeeping/entries/AccountingEntrySet.h"

class AbstractEntryParser
{
public:
    enum Type{Bank, Purchase, Sale, SaleMarketplace, VariousOperations}; //TODO BankAuto for things like mutuel? Or automatique detection?
    AbstractEntryParser();
    virtual ~AbstractEntryParser();
    virtual AccountingEntries entries(QList<int> years) const;
    virtual AccountingEntries entries(int year) const = 0;
    virtual Type typeOfEntries() const = 0;
    virtual QString name() const = 0;
    virtual QString journal() const = 0;
};

#endif // ABSTRACTENTRYPARSER_H
