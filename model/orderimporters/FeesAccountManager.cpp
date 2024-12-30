#include <QtCore/qsettings.h>

#include "model/SettingManager.h"

#include "FeesAccountManager.h"

//----------------------------------------------------------
FeesAccountManager::FeesAccountManager(QObject *parent)
    : QAbstractTableModel(parent)
{
    loadFromSettings();
}
//----------------------------------------------------------
FeesAccountManager *FeesAccountManager::instance()
{
    static FeesAccountManager instance;
    return &instance;
}
//----------------------------------------------------------
void FeesAccountManager::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QString key = "feesAccountManager";
    if (m_accounts.size() > 0) {
        QStringList elements;
        for (auto it = m_accounts.begin();
             it != m_accounts.end();
             ++it) {
            elements << it.key() + ":::" + it.value();
        }
        QString stringCompressed = elements.join(";;;");
        settings.setValue(key, stringCompressed);
    } else if (settings.contains(key)) {
        settings.remove(key);
    }
}
//----------------------------------------------------------
void FeesAccountManager::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QString key = "feesAccountManager";
    if (m_accountList.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_accountList.size()-1);
        m_accounts.clear();
        m_accountList.clear();
        endRemoveRows();
    }
    if (settings.contains(key)) {
        QString stringCompressed = settings.value(key).toString();
        QStringList elements = stringCompressed.split(";;;");
        for (auto element : elements) {
            QStringList values = element.split(":::");
            m_accounts[values[0]] = values[1];
        }
        beginInsertRows(QModelIndex(), 0, m_accounts.size()-1);
        m_accountList = m_accounts.keys();
        endInsertRows();
    }
}
//----------------------------------------------------------
void FeesAccountManager::addAccount(const QString &number, const QString &label)
{
    m_accounts[number] = label;
    int index = m_accounts.keys().indexOf(number);
    beginInsertRows(QModelIndex(), index, index);
    m_accountList.insert(index, number);
    endInsertRows();
    saveInSettings();
}
//----------------------------------------------------------
void FeesAccountManager::removeAccount(int position)
{
    beginRemoveRows(QModelIndex(), position, position);
    QString account = m_accountList[position];
    m_accountList.removeAt(position);
    m_accounts.remove(account);
    endRemoveRows();
    saveInSettings();
}
//----------------------------------------------------------
QStringList FeesAccountManager::toList() const
{
    QStringList list;
    for (auto account : m_accountList) {
        QString value = account + " - " + m_accounts[account];
        list << value;
    }
    return list;
}
//----------------------------------------------------------
QVariant FeesAccountManager::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    QVariant value;
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList header = {"Compte", "Label"};
        value = header[section];
    }
    return value;
}
//----------------------------------------------------------
int FeesAccountManager::rowCount(const QModelIndex &) const
{
    return m_accounts.size();
}
//----------------------------------------------------------
int FeesAccountManager::columnCount(const QModelIndex &) const
{
    return 2;
}
//----------------------------------------------------------
QVariant FeesAccountManager::data(const QModelIndex &index, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        QString account = m_accountList[index.row()];
        if (index.column() == 0) {
            value = account;
        } else {
            value = m_accounts[account];
        }
    }
    return value;
}
//----------------------------------------------------------
Qt::ItemFlags FeesAccountManager::flags(const QModelIndex &) const
{
    Qt::ItemFlags flags
            = Qt::ItemIsEnabled
            | Qt::ItemIsEditable
            | Qt::ItemIsSelectable;
    return flags;
}
//----------------------------------------------------------
bool FeesAccountManager::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    bool edited = false;
    if (role == Qt::EditRole) {
        if (index.column() == 0) {
            QString previousKey = m_accountList[index.row()];
            QString newKey = value.toString();
            if (previousKey != newKey) {
                QString label = m_accounts[previousKey];
                m_accounts.remove(previousKey);
                m_accounts[newKey] = label;
                m_accountList = m_accounts.keys();
                edited = true;
                emit dataChanged(this->index(0, 0),
                                 this->index(m_accounts.size(), 1),
                {Qt::DisplayRole});
                emit accountUpdated(previousKey, newKey);
            }
        } else {
            m_accounts[m_accountList[index.row()]] = value.toString();
            edited = true;
        }
    }
    if (edited) {
        saveInSettings();
    }
    return edited;
}
//----------------------------------------------------------
