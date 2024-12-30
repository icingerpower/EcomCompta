#include <QtCore/qsettings.h>
#include <QtCore/qstringlist.h>

#include "model/SettingManager.h"

#include "SelectedSkusListModel.h"


//----------------------------------------------------------
SelectedSkusListModel::SelectedSkusListModel(
        const QString &settingKey, QObject *parent)
    : QAbstractListModel(parent)
{
    m_settingKey = settingKey;
}
//----------------------------------------------------------
SelectedSkusListModel::~SelectedSkusListModel()
{

}
//----------------------------------------------------------
bool SelectedSkusListModel::addSku(const QString &sku)
{
    bool success = false;
    int initialSize = m_skus.size();
    m_skus << sku;
    if (m_skus.size() > initialSize) {
        beginInsertRows(QModelIndex(), initialSize, initialSize);
        m_skusList << sku;
        success = true;
        saveInSettings();
        endInsertRows();
    }
    return success;
}
//----------------------------------------------------------
void SelectedSkusListModel::removeSku(const QString &sku)
{
    int initialSize = m_skus.size();
    m_skus.remove(sku);
    if (m_skus.size() < initialSize) {
        int index = m_skusList.indexOf(sku);
        m_skusList.removeAt(index);
        beginRemoveRows(QModelIndex(), index, index);
        saveInSettings();
        endRemoveRows();
    }
}
//----------------------------------------------------------
bool SelectedSkusListModel::contains(const QString &sku) const
{
    return m_skus.contains(sku);
}
//----------------------------------------------------------
void SelectedSkusListModel::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(m_settingKey)) {
        QString string = settings.value(m_settingKey).toString();
        m_skus = string.split(";;").toSet();
        m_skusList = m_skus.toList();
        if (m_skus.size() == 1 && m_skusList[0].isEmpty()) {
            m_skusList.clear();
            m_skus.clear();
        } else {
            beginInsertRows(QModelIndex(), 0, m_skus.size()-1);
            endInsertRows();

        }
    }
}
//----------------------------------------------------------
void SelectedSkusListModel::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QStringList elements = m_skus.toList();
    if (elements.size() > 0) {
        settings.setValue(m_settingKey, elements.join(";;"));
    } else if (settings.contains(m_settingKey)) {
        settings.remove(m_settingKey);
    }
}
//----------------------------------------------------------
void SelectedSkusListModel::clear()
{
    if (m_skus.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_skus.size()-1);
        m_skus.clear();
        m_skusList.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------
int SelectedSkusListModel::rowCount(const QModelIndex &) const
{
    return m_skus.size();
}
//----------------------------------------------------------
int SelectedSkusListModel::columnCount(const QModelIndex &) const
{
    return 1;
}
//----------------------------------------------------------
QVariant SelectedSkusListModel::headerData(
        int section,
        Qt::Orientation orientation,
        int role) const
{
    return QAbstractListModel::headerData(section, orientation, role);
}
//----------------------------------------------------------
QVariant SelectedSkusListModel::data(
        const QModelIndex &index, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole) {
        value = m_skusList[index.row()];
    }
    return value;
}
//----------------------------------------------------------
