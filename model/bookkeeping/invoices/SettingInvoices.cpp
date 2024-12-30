#include <QSettings>

#include "model/SettingManager.h"
#include "model/orderimporters/AbstractOrderImporter.h"

#include "SettingInvoices.h"

QString SettingInvoices::PAR_INVOICE_FIRST_NUMER = QObject::tr("Premier numéro de facture");
QString SettingInvoices::PAR_INVOICE_PREFIX = QObject::tr("Prefix facture") + " - ";
//----------------------------------------------------------
SettingInvoices::SettingInvoices(QObject *parent)
    : QAbstractTableModel(parent), UpdateToCustomer()
{
    _initDefaultValues();
}
//----------------------------------------------------------
void SettingInvoices::_initDefaultValues()
{
    m_paramNames << PAR_INVOICE_FIRST_NUMER;
    m_paramValues[PAR_INVOICE_FIRST_NUMER] = 1;
    for (auto importer : AbstractOrderImporter::allImporters()) {
        QString importerName = importer->name();
        QString paramName = _paramChannel(importerName);
        m_paramNames << paramName;
        m_paramValues[paramName] = importer->invoicePrefix();
    }
}
//----------------------------------------------------------
QString SettingInvoices::_paramChannel(const QString &channelName) const
{
    return PAR_INVOICE_PREFIX + channelName;
}
//----------------------------------------------------------
QString SettingInvoices::_settingKeyAddress() const
{
    return settingKey() + "-address";
}
//----------------------------------------------------------
QString SettingInvoices::_settingKeyBottomLegal() const
{
    return settingKey() + "-bottom-legal";
}
//----------------------------------------------------------
QString SettingInvoices::_settingKeyBottomLaw() const
{
    return settingKey() + "-bottom-law";
}
//----------------------------------------------------------
SettingInvoices *SettingInvoices::instance()
{
    static SettingInvoices instance;
    static bool first = true;
    if (first) {
        instance.loadFromSettings();
        first = false;
    }
    return &instance;
}
//----------------------------------------------------------
void SettingInvoices::onCustomerSelectedChanged(
        const QString &)
{
    beginRemoveRows(QModelIndex(), 0, m_paramNames.size()-1);
    m_paramNames.clear();
    m_paramValues.clear();
    endRemoveRows();
    _initDefaultValues();
    beginInsertRows(QModelIndex(), 0, m_paramNames.size()-1);
    loadFromSettings();
    endInsertRows();
}
//----------------------------------------------------------
QString SettingInvoices::uniqueId() const
{
    return "SettingInvoices";
}
//----------------------------------------------------------
int SettingInvoices::invoiceFirstNumber() const
{
    return m_paramValues[PAR_INVOICE_FIRST_NUMER].toInt();
}
//----------------------------------------------------------
QString SettingInvoices::invoicePrefix(const QString &channel) const
{
    return m_paramValues[_paramChannel(channel)].toString();
}
//----------------------------------------------------------
void SettingInvoices::setAddressFrom(const QStringList &lines)
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    settings.setValue(
                _settingKeyAddress(),
                lines.join("\n"));
}
//----------------------------------------------------------
QStringList SettingInvoices::addressFrom() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    return settings.value(
                _settingKeyAddress(), "").toString().split("\n");
}
//----------------------------------------------------------
void SettingInvoices::setTextBottomLegal(const QStringList &lines)
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    settings.setValue(
                _settingKeyBottomLegal(),
                lines.join("\n"));
}
//----------------------------------------------------------
QStringList SettingInvoices::textBottomLegal() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    return settings.value(
                _settingKeyBottomLegal(), "").toString().split("\n");
}
//----------------------------------------------------------
void SettingInvoices::setTextBottomLaw(const QStringList &lines)
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    settings.setValue(
                _settingKeyBottomLaw(),
                lines.join("\n"));
}
//----------------------------------------------------------
QStringList SettingInvoices::textBottomLaw() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    return settings.value(
                _settingKeyBottomLaw(), "").toString().split("\n");
}
//----------------------------------------------------------
void SettingInvoices::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QStringList lines;
    for (auto it=m_paramValues.begin();
         it!=m_paramValues.end(); ++it) {
        QString line = it.key() + SettingManager::SEP_COL + it.value().toString();
        lines << line;
    }
    settings.setValue(settingKey(), lines.join(
                          SettingManager::SEP_LINES));
}
//----------------------------------------------------------
void SettingInvoices::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(settingKey())) {
        QString text = settings.value(settingKey()).toString();
        QStringList lines = text.split(SettingManager::SEP_LINES);
        for (auto line = lines.begin(); line != lines.end(); ++line) {
            QStringList elements = line->split(SettingManager::SEP_COL);
            if (m_paramValues.contains(elements[0])) {
                int typeId = m_paramValues[elements[0]].userType();
                m_paramValues[elements[0]] = elements[1];
                m_paramValues[elements[0]].convert(typeId);
            }
        }
    }
}
//----------------------------------------------------------
QVariant SettingInvoices::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList values = {tr("Paramètre"), tr("Valeur")};
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int SettingInvoices::rowCount(const QModelIndex &) const
{
    return m_paramNames.size();
}
//----------------------------------------------------------
int SettingInvoices::columnCount(const QModelIndex &) const
{
    return 2;
}
//----------------------------------------------------------
QVariant SettingInvoices::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() == 0) {
            return m_paramNames[index.row()];
        } else {
            return m_paramValues[m_paramNames[index.row()]];
        }
    }
    return QVariant();
}
//----------------------------------------------------------
bool SettingInvoices::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() > 0 && data(index, role) != value) {
        m_paramValues[m_paramNames[index.row()]] = value;
        saveInSettings();
        return true;
    }
    return false;
}
//----------------------------------------------------------
Qt::ItemFlags SettingInvoices::flags(const QModelIndex &index) const
{
    if (index.column() == 0) {
        return Qt::ItemIsEnabled;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsEditable;
}
//----------------------------------------------------------
