#include <QSharedPointer>
#include <QDateTime>
#include <QSettings>

#include "model/SettingManager.h"

#include "SkusGroupProfitModel.h"

//----------------------------------------------------------
SkusGroupProfitModel *SkusGroupProfitModel::instance()
{
    static QSharedPointer<SkusGroupProfitModel> instance
            = []() -> QSharedPointer<SkusGroupProfitModel>{
            QSharedPointer<SkusGroupProfitModel> _instance(
                new SkusGroupProfitModel);
            _instance->init();
            return _instance;
}();
    return instance.data();
}
//----------------------------------------------------------
QString SkusGroupProfitModel::addGroup(const QString &name)
{
    QString id = _genId(name);
    beginInsertRows(QModelIndex(),
                    m_listOfStringList.size()-1,
                    m_listOfStringList.size()-1);
    m_listOfStringList << QStringList({name, id});
    endInsertRows();
    return id;
}
//----------------------------------------------------------
void SkusGroupProfitModel::insertGroup(
        int index, const QString &name)
{
    beginInsertRows(QModelIndex(), index, index);
    m_listOfStringList.insert(
                0,
                QStringList({name, _genId(name)}));
    endInsertRows();
}
//----------------------------------------------------------
void SkusGroupProfitModel::removeGroup(
        const QModelIndex &index)
{
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    m_listOfStringList.removeAt(index.row());
    endRemoveRows();
}
//----------------------------------------------------------
QString SkusGroupProfitModel::groupId(
        const QModelIndex &index) const
{
    return m_listOfStringList[index.row()].last();
}
//----------------------------------------------------------
QString SkusGroupProfitModel::getTextOfGroup(
        const QModelIndex &index) const
{
    QString groupId = m_listOfStringList[index.row()].last();
    QString groupSettingKey = _settingIdGroup(groupId);
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    return settings.value(groupSettingKey).toString();
}
//----------------------------------------------------------
void SkusGroupProfitModel::setTextOfGroup(
        const QModelIndex &index,
        const QString &text)
{
    QString groupId = m_listOfStringList[index.row()].last();
    setTextOfGroup(groupId, text);
}
//----------------------------------------------------------
void SkusGroupProfitModel::setTextOfGroup(
        const QString &groupId,
        const QString &text)
{
    QString groupSettingKey = _settingIdGroup(groupId);
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    return settings.setValue(groupSettingKey, text);
}
//----------------------------------------------------------
void SkusGroupProfitModel::onCustomerSelectedChanged(
        const QString &)
{
    _loadFromSettings();
}
//----------------------------------------------------------
QString SkusGroupProfitModel::uniqueId() const
{
    return "SkusGroupProfitModel";
}
//----------------------------------------------------------
SkusGroupProfitModel::SkusGroupProfitModel(QObject *parent)
    : QAbstractListModel(parent), UpdateToCustomer()
{
    _loadFromSettings();
}
//----------------------------------------------------------
void SkusGroupProfitModel::_clear()
{
    if (m_listOfStringList.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        m_listOfStringList.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------
void SkusGroupProfitModel::_saveInSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settingKey().isEmpty()) {
        QStringList lines;
        for (auto itLine = m_listOfStringList.begin();
             itLine != m_listOfStringList.end(); ++itLine) {
            QStringList lineElements;
            for (auto itEl = itLine->begin();
                 itEl != itLine->end(); ++itEl) {
                lineElements << *itEl;
            }
            lines << lineElements.join(";;;");
        }
        settings.setValue(
                    settingKey(),
                    lines.join(":::"));
    }
}
//----------------------------------------------------------
void SkusGroupProfitModel::_loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settingKey().isEmpty()) {
        if (settings.contains(settingKey())) {
            _clear();
            QList<QStringList> loadedValues;
            auto loadedString = settings.value(settingKey()).toString();
            QStringList lines = loadedString.split(":::");
            for (auto itLine = lines.begin();
                 itLine != lines.end(); ++itLine) {
                loadedValues << itLine->split(";;;");
            }
            beginInsertRows(QModelIndex(), 0, loadedValues.size()-1);
            m_listOfStringList = loadedValues;
            endInsertRows();
        }
    }
}
//----------------------------------------------------------
QString SkusGroupProfitModel::_genId(const QString &name) const
{
    QString id = name;
    id += QDateTime::currentDateTime().toString(
                "yyyy-MM-dd__mm-ss-zzz");
    return id;
}
//----------------------------------------------------------
QString SkusGroupProfitModel::_settingIdGroup(
        const QString &groupId) const
{
    return settingKey() + groupId;
}
//----------------------------------------------------------
QVariant SkusGroupProfitModel::headerData(
        int, Qt::Orientation, int) const
{
    return QVariant();
}
//----------------------------------------------------------
int SkusGroupProfitModel::rowCount(
        const QModelIndex &) const
{
    return m_listOfStringList.size();
}
//----------------------------------------------------------
QVariant SkusGroupProfitModel::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_listOfStringList[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
bool SkusGroupProfitModel::setData(
        const QModelIndex &index,
        const QVariant &value,
        int role)
{
    if (data(index, role) != value) {
        m_listOfStringList[index.row()][index.column()]
                = value.toString();
        _saveInSettings();
        return true;
    }
    return false;
}
//----------------------------------------------------------
Qt::ItemFlags SkusGroupProfitModel::flags(
        const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable; // FIXME: Implement me!
}
//----------------------------------------------------------
