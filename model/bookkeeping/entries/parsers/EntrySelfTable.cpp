#include <QSettings>

#include "model/SettingManager.h"

#include "EntrySelfTable.h"

//----------------------------------------------------------
EntrySelfTable::EntrySelfTable(QObject *object)
    : QAbstractTableModel(object), UpdateToCustomer()
{
    init();
    QString key = settingKey();
    int TEMP=10;++TEMP;
    //m_values << QStringList({"2", "437100", "Assurance retraite"});
}
//----------------------------------------------------------
EntrySelfTable *EntrySelfTable::instance()
{
    static EntrySelfTable table;
    return &table;
}
//----------------------------------------------------------
EntrySelfTable::~EntrySelfTable()
{
}
//----------------------------------------------------------
QString EntrySelfTable::uniqueId() const
{
    return "EntrySelfTable";
}
//----------------------------------------------------------
EntrySelfInfo EntrySelfTable::account(const QString &id) const
{
    return account(m_idToIndex[id]);
}
//----------------------------------------------------------
EntrySelfInfo EntrySelfTable::account(int pos) const
{
    EntrySelfInfo info;
    info.id = m_values[pos][0];
    info.title = m_values[pos][1];
    info.account = m_values[pos][2];
    return info;
}
//----------------------------------------------------------
void EntrySelfTable::addAccount(
        const QString &title,
        const QString &account)
{
    int lastId = 1;
    if (m_values.size() > 0) {
        for (auto value : m_values) {
            lastId = qMax(lastId, value[0].toInt() + 1);
        }
    }
    QStringList elements = {QString::number(lastId+1), title, account};
    beginInsertRows(QModelIndex(), 0, 0);
    m_values.insert(0, elements);
    m_idToIndex[elements[0]] = 0;
    saveInSettings();
    endInsertRows();
}
//----------------------------------------------------------
void EntrySelfTable::removeAccount(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    m_idToIndex.remove(m_values[index][0]);
    m_values.removeAt(index);
    saveInSettings();
    endRemoveRows();
}
//----------------------------------------------------------
void EntrySelfTable::_clear()
{
    if (m_values.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_values.size()-1);
        m_values.clear();
        m_idToIndex.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------
void EntrySelfTable::onCustomerSelectedChanged(const QString &)
{
    loadFromSettings();
}
//----------------------------------------------------------
QVariant EntrySelfTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList values = {tr("Nom"), tr("Compte")};
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int EntrySelfTable::rowCount(const QModelIndex &) const
{
    return m_values.size();
}
//----------------------------------------------------------
int EntrySelfTable::columnCount(const QModelIndex &) const
{
    return 2;
}
//----------------------------------------------------------
QVariant EntrySelfTable::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_values[index.row()][index.column()+1];
    }
    return QVariant();
}
//----------------------------------------------------------
Qt::ItemFlags EntrySelfTable::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}
//----------------------------------------------------------
bool EntrySelfTable::setData(
        const QModelIndex &index,
        const QVariant &value,
        int role)
{
    if (role == Qt::EditRole && data(index) != value) {
        m_values[index.row()][index.column()+1] = value.toString();
        saveInSettings();
        return true;
    }
    return false;
}
//----------------------------------------------------------
void EntrySelfTable::loadFromSettings()
{
    //return; //TODO find why it share common data with Mananger Account Purchase
    _clear();
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(settingKey())) {
        QStringList lines
                = settings.value(settingKey()).toString().split(":::");
        int id = 0;
        for (auto line : lines) {
            QStringList elements = line.split(";;;");
            m_values << elements;
            m_idToIndex[elements[0]] = id;
            ++id;
        }
        beginInsertRows(QModelIndex(), 0, m_values.size()-1);
        endInsertRows();
    }
}
//----------------------------------------------------------
void EntrySelfTable::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (m_values.size() > 0) {
        QStringList lines;
        for (auto it = m_values.begin();
             it != m_values.end(); ++it) {
            lines << it->join(";;;");
        }
        settings.setValue(settingKey(), lines.join(":::"));
    } else {
        settings.remove(settingKey());
    }
}
//----------------------------------------------------------
