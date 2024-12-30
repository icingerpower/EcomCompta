#include <QtCore/qsettings.h>

#include "ManagerAccountsSalesRares.h"
#include "model/orderimporters/Shipment.h"

//----------------------------------------------------------
ManagerAccountsSalesRares *ManagerAccountsSalesRares::instance()
{
    static ManagerAccountsSalesRares instance;
    return &instance;
}
//----------------------------------------------------------
ManagerAccountsSalesRares::~ManagerAccountsSalesRares()
{
}
//----------------------------------------------------------
ManagerAccountsSalesRares::ManagerAccountsSalesRares(QObject *parent)
    : QAbstractTableModel(parent), UpdateToCustomer()
{
    _generateBasicAccounts();
    init();
}
//----------------------------------------------------------
void ManagerAccountsSalesRares::onCustomerSelectedChanged(
        const QString &customerId)
{
    if (customerId.isEmpty()) {
        _clear();
    } else {
        loadFromSettings();
    }
}
//----------------------------------------------------------
QString ManagerAccountsSalesRares::uniqueId() const
{
    return "ManagerAccountsSalesRares";
}
//----------------------------------------------------------
void ManagerAccountsSalesRares::_clear()
{
    beginRemoveRows(QModelIndex(), 0, m_values.size()-1);
    m_values.clear();
    endRemoveRows();
}
//----------------------------------------------------------
void ManagerAccountsSalesRares::_generateBasicAccounts()
{
    m_values << QStringList({"cdiscount.fr", "INTERETBCA",
                             "708500", "Frais accessoires sur ventes 0%"});
    _generateMapping();
}
//----------------------------------------------------------
void ManagerAccountsSalesRares::_generateMapping()
{
    m_mapping.clear();
    for (int pos=0; pos < m_values.size(); ++pos) {
        QString subchannel = m_values[pos][0];
        QString sku = m_values[pos][1];
        QString account = m_values[pos][2];
        QString title = m_values[pos][3];
        if (!m_mapping.contains(subchannel)) {
            m_mapping[subchannel] = QHash<QString, QPair<QString, QString>>();
        }
        if (!m_mapping[subchannel].contains(sku)) {
            m_mapping[subchannel][sku] = QPair<QString, QString>();
        }
        m_mapping[subchannel][sku].first = account;
        m_mapping[subchannel][sku].second = title;
    }
}
//----------------------------------------------------------
void ManagerAccountsSalesRares::saveInSettings() const
{
    //return;
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settingKey().isEmpty()) {
        QStringList lines;
        for (auto line : m_values) {
            lines << line.join(";;;");
        }
        settings.setValue(settingKey(), lines.join(":::"));
    }
}
//----------------------------------------------------------
void ManagerAccountsSalesRares::loadFromSettings()
{
    //return; // TODO
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settingKey().isEmpty()) {
        if (settings.contains(settingKey())) {
            _clear();
            QString text = settings.value(settingKey()).toString();
            QStringList lines = text.split(":::");
            for (auto line:lines) {
                QStringList elements = line.split(";;;");
                m_values << elements;
            }
        }
    }
    if (m_values.size() > 0) {
        beginInsertRows(QModelIndex(), 0, m_values.size()-1);
        _generateMapping();
        endInsertRows();
    }
}
//----------------------------------------------------------
ManagerAccountsSalesRares::AccountsRare ManagerAccountsSalesRares::getAccounts(
        const QString &subChannel,
        const QString &sku,
        const QString &defaultNumber,
        const QString &defaultTitle) const
{
    AccountsRare accounts;
    accounts.title = defaultTitle;
    accounts.number = defaultNumber;
    if (m_mapping.contains(subChannel)
            && m_mapping[subChannel].contains(sku)) {
        auto pair = m_mapping[subChannel][sku];
        accounts.title = pair.first;
        accounts.number = pair.second;
    }
    return accounts;
}
//----------------------------------------------------------
void ManagerAccountsSalesRares::addRow(const QString &subChannel,
        const QString &sku,
        const QString &number,
        const QString &title)
{
    beginInsertRows(QModelIndex(), 0, 0);
    m_values.insert(0, QStringList({subChannel, sku, number, title}));
    saveInSettings();
    _generateMapping();
    endInsertRows();
}
//----------------------------------------------------------
void ManagerAccountsSalesRares::removeRow(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    m_values.removeAt(index);
    _generateMapping();
    saveInSettings();
    endRemoveRows();
}
//----------------------------------------------------------
QVariant ManagerAccountsSalesRares::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList values
                = {tr("Canal de vente"), tr("SKU"), tr("Compte"),
                   tr("Titre")};
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int ManagerAccountsSalesRares::rowCount(const QModelIndex &) const
{
    return m_values.size();
}
//----------------------------------------------------------
int ManagerAccountsSalesRares::columnCount(const QModelIndex &) const
{
    return 4;
}
//----------------------------------------------------------
QVariant ManagerAccountsSalesRares::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_values[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
Qt::ItemFlags ManagerAccountsSalesRares::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}
//----------------------------------------------------------
bool ManagerAccountsSalesRares::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        m_values[index.row()][index.column()] = value.toString();
        _generateMapping();
        saveInSettings();
        return true;
    }
    return false;
}
//----------------------------------------------------------
void ManagerAccountsSalesRares::sort(int column, Qt::SortOrder order)
{
    if (order == Qt::AscendingOrder) {
        std::sort(m_values.begin(), m_values.end(),
                  [column](const QStringList &v1, const QStringList &v2){
            return v1[column] < v2[column];
        });
    } else {
        std::sort(m_values.begin(), m_values.end(),
                  [column](const QStringList &v1, const QStringList &v2){
            return v1[column] > v2[column];
        });
    }
    emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1),
                     QVector<int>() << Qt::DisplayRole);
}
//----------------------------------------------------------
