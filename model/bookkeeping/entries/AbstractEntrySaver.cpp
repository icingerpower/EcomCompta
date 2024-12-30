#include <QtCore/qsettings.h>

#include "model/SettingManager.h"
#include "AbstractEntrySaver.h"

#include "EntrySaverIbiza.h"

//----------------------------------------------------------
AbstractEntrySaver::AbstractEntrySaver()
{
}
//----------------------------------------------------------
AbstractEntrySaver::~AbstractEntrySaver()
{
}
//----------------------------------------------------------
SortedMap<QString, AbstractEntrySaver *> AbstractEntrySaver::entrySavers()
{
    static SortedMap<QString, AbstractEntrySaver *> savers;
    if (savers.isEmpty()) {
        static EntrySaverIbiza ibiza;
        savers[ibiza.name()] = &ibiza;
    }
    return savers;
}
//----------------------------------------------------------
QStringList AbstractEntrySaver::entrySaverNames()
{
    static QStringList names = entrySavers().keys();
    return names;
}
//----------------------------------------------------------
AbstractEntrySaver *AbstractEntrySaver::selected()
{
    return entrySavers()[selectedName()];
}
//----------------------------------------------------------
QString AbstractEntrySaver::selectedName()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QString sel = settings.value(
                "AbstractEntrySaver__selected",
                EntrySaverIbiza().name()).toString();
    return sel;
}
//----------------------------------------------------------
void AbstractEntrySaver::select(const QString &name)
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    settings.setValue("AbstractEntrySaver__selected",
                      name);
}
//----------------------------------------------------------
