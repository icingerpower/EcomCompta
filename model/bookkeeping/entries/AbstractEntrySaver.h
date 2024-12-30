#ifndef ABSTRACTENTRYSAVER_H
#define ABSTRACTENTRYSAVER_H

#include <QtCore/qdir.h>
#include "model/utils/SortedMap.h" //TODO put in common


#include "AccountingEntrySet.h"

class AbstractEntrySaver
{
public:
    AbstractEntrySaver();
    virtual ~AbstractEntrySaver();
    static SortedMap<QString, AbstractEntrySaver*> entrySavers();
    static QStringList entrySaverNames();
    static AbstractEntrySaver* selected();
    static QString selectedName();
    static void select(const QString &name);
    virtual QString name() const = 0;
    virtual void save(const AccountingEntries &entries,
                      const QDir &dir) = 0;

protected:
};

#endif // ABSTRACTENTRYSAVER_H
