#include <QSettings>
#include <QDateTime>

#include "model/SettingManager.h"

#include "SaleTemplateManager.h"

SaleTemplateManager::SaleTemplateManager(QObject *parent)
    : QAbstractListModel(parent), UpdateToCustomer()
{
}

void SaleTemplateManager::_clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    m_listOfStringList.clear();
    endRemoveRows();
}

void SaleTemplateManager::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (m_listOfStringList.size() > 0)
    {
        QStringList stringList;
        for (const auto &curStringList : m_listOfStringList)
        {
            QStringList subStringList;
            for (const auto & string : curStringList)
            {
                subStringList << string;
            }
            stringList << subStringList.join(SettingManager::SEP_COL);
        }
        settings.setValue(settingKey(), stringList);
    }
    else if (settings.contains(settingKey()))
    {
        settings.remove(settingKey());
    }
}

void SaleTemplateManager::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QStringList tempStringList
            = settings.value(settingKey()).value<QStringList>();
    _clear();
    beginInsertRows(QModelIndex{}, 0, tempStringList.size()-1);
    m_listOfStringList.clear();
    for (const auto &stringListCompressed : qAsConst(tempStringList))
    {
        const auto &stringList = stringListCompressed.split(SettingManager::SEP_COL);
        m_listOfStringList << stringList;
    }
    endInsertRows();
}

SaleTemplateManager *SaleTemplateManager::instance()
{
    static SaleTemplateManager instance;
    static bool first = true;
    if (first)
    {
        QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
        instance.onCustomerSelectedChanged(selectedCustomerId);
        first = false;
    }
    return &instance;
}

const QString &SaleTemplateManager::getId(const QModelIndex &index) const
{
    return getId(index.row());
}

const QString &SaleTemplateManager::getId(int rowIndex) const
{
    return m_listOfStringList[rowIndex].last();
}

void SaleTemplateManager::add(const QString &name)
{
    beginInsertRows(QModelIndex{}, rowCount(), rowCount());
    m_listOfStringList << QStringList{
                           name,
                           name + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")};
    saveInSettings();
    endInsertRows();
}

void SaleTemplateManager::remove(const QModelIndex &index)
{
    beginRemoveRows(QModelIndex{}, index.row(), index.row());
    m_listOfStringList.removeAt(index.row());
    saveInSettings();
    endRemoveRows();
}

QString SaleTemplateManager::uniqueId() const
{
    return "SaleTemplateManager";
}

void SaleTemplateManager::onCustomerSelectedChanged(const QString &customerId)
{
    if (customerId.isEmpty())
    {
        _clear();
    }
    else
    {
        loadFromSettings();
    }
}

int SaleTemplateManager::rowCount(const QModelIndex &) const
{
    return m_listOfStringList.size();
}

QVariant SaleTemplateManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return m_listOfStringList[index.row()][index.column()];
    }
    return QVariant();
}

bool SaleTemplateManager::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value)
    {
        m_listOfStringList[index.row()][index.column()] = value.toString();
        saveInSettings();
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

Qt::ItemFlags SaleTemplateManager::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

