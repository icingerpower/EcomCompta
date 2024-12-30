#include <QSettings>

#include "model/bookkeeping/entries/EntrySaverIbiza.h"
#include "model/SettingManager.h"

#include "SettingBookKeeping.h"

//----------------------------------------------------------
SettingBookKeeping::SettingBookKeeping()
{
}
//----------------------------------------------------------
SettingBookKeeping *SettingBookKeeping::instance()
{
    static SettingBookKeeping instance;
    return &instance;
}
//----------------------------------------------------------
SettingBookKeeping::~SettingBookKeeping()
{
}
//----------------------------------------------------------
void SettingBookKeeping::onCustomerSelectedChanged(const QString &)
{
}
//----------------------------------------------------------
QString SettingBookKeeping::uniqueId() const
{
    return "SettingBookKeeping";
}
//----------------------------------------------------------
QString SettingBookKeeping::saverName() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    return settings.value(
                settingKeySaver(),
                EntrySaverIbiza().name()).toString();
}
//----------------------------------------------------------
void SettingBookKeeping::setSaverName(const QString &saver)
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    settings.setValue(settingKeySaver(), saver);
}
//----------------------------------------------------------
QString SettingBookKeeping::dirPath() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QString path =  settings.value(
                settingKeyBookKeepingDir(),
                QFileInfo(settings.fileName()).path())
            .toString();
    return path; //TODO check if it exists otherwise return default
}
//----------------------------------------------------------
void SettingBookKeeping::setDirPath(const QString &dirPath)
{
    QSettings settings;
    settings.setValue(settingKeyBookKeepingDir(), dirPath);
}
//----------------------------------------------------------
QString SettingBookKeeping::internWireAccount() const
{
    return "580000";
}
//----------------------------------------------------------
QString SettingBookKeeping::settingKeySaver() const
{
    return settingKey() + "-saver";
}
//----------------------------------------------------------
QString SettingBookKeeping::settingKeyBookKeepingDir() const
{
    return settingKey() + "-bookkeepingdir";
}
//----------------------------------------------------------
