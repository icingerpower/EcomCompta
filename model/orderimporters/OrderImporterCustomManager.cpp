#include <QtCore/qsettings.h>
#include <QtCore/qdatetime.h>

#include "model/SettingManager.h"
#include "OrderImporterCustomManager.h"
#include "OrderImporterCustom.h"

//----------------------------------------------------------
OrderImporterCustomManager::OrderImporterCustomManager(QObject *parent)
    : QAbstractListModel(parent)
{
    loadFromSettings();
}
//----------------------------------------------------------
OrderImporterCustomManager *OrderImporterCustomManager::instance()
{
    static OrderImporterCustomManager instance;
    return &instance;
}
//----------------------------------------------------------
OrderImporterCustomManager::~OrderImporterCustomManager()
{
    for (auto customParams : m_customParamsById) {
        delete customParams;
    }
    for (auto importer : m_importerById) {
        delete importer;
    }
}
//----------------------------------------------------------
void OrderImporterCustomManager::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (m_names.size() > 0) {
        QStringList lines;
        for (auto elements : m_names) {
            lines << elements.join(SettingManager::SEP_COL);
        }
        settings.setValue(_settingKey(), lines.join(
                              SettingManager::SEP_LINES));
    } else if (settings.contains(_settingKey())) {
        settings.remove(_settingKey());
    }
}
//----------------------------------------------------------
void OrderImporterCustomManager::loadFromSettings()
{
    //m_names.clear();
    //m_customParamsById.clear();
    //m_importerById.clear();
    Q_ASSERT(m_names.size() == 0);
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(_settingKey())) {
        QString text = settings.value(_settingKey()).toString();
        QStringList lines = text.split(SettingManager::SEP_LINES);
        for (auto line : lines) {
            QStringList elements = line.split(SettingManager::SEP_COL);
            m_names << elements;
            QString id = elements[1];
            m_customParamsById[id] = new OrderImporterCustomParams(id);
            m_importerById[id] = new OrderImporterCustom(id);
        }
        beginInsertRows(QModelIndex(), 0, m_names.size()-1);
        endInsertRows();
    }
}
//----------------------------------------------------------
QList<AbstractOrderImporter *> OrderImporterCustomManager::allImporters()
{
    QList<AbstractOrderImporter *> importers;
    for (auto importer : m_importerById) {
        importers << importer;
    }
    return importers;
}
//----------------------------------------------------------
OrderImporterCustom *OrderImporterCustomManager::importer(
        const QString &id) const
{
    return m_importerById[id];
}
//----------------------------------------------------------
void OrderImporterCustomManager::addCustomParams(const QString &name)
{
    QString id = name + QDateTime::currentDateTime().toString("yyyyMMddhhss");
    beginInsertRows(QModelIndex(), 0, 0);
    m_names.insert(0, {name, id});
    m_customParamsById[id] = new OrderImporterCustomParams(id);
    m_importerById[id] = new OrderImporterCustom(id);
    saveInSettings();
    endInsertRows();
}
//----------------------------------------------------------
void OrderImporterCustomManager::removeCustomParams(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    QString id = m_names[index][1];
    auto modelParams = m_customParamsById.take(id);
    modelParams->deleteLater();
    m_names.removeAt(index);
    saveInSettings();
    endRemoveRows();
}
//----------------------------------------------------------
OrderImporterCustomParams *OrderImporterCustomManager::customParams(int index)
{
    QString id = m_names[index][1];
    return m_customParamsById[id];
}
//----------------------------------------------------------
OrderImporterCustomParams *OrderImporterCustomManager::customParams(
        const QString &id)
{
    return m_customParamsById[id];
}
//----------------------------------------------------------
QString OrderImporterCustomManager::customParamsId(int index) const
{
    return m_names[index][1];
}
//----------------------------------------------------------
QString OrderImporterCustomManager::customParamsName(const QString &id) const
{
    for (auto elements : m_names) {
        if (elements[1] == id) {
            return elements[0];
        }
    }
    return "";
}
//----------------------------------------------------------
QString OrderImporterCustomManager::customParamsName(int index) const
{

    return m_names[index][0];
}
//----------------------------------------------------------
QVariant OrderImporterCustomManager::headerData(
        int, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return "Nom";
    }
    return QVariant();
}
//----------------------------------------------------------
int OrderImporterCustomManager::rowCount(const QModelIndex &) const
{
    return m_names.size();
}
//----------------------------------------------------------
int OrderImporterCustomManager::columnCount(const QModelIndex &) const
{
    return 1;
}
//----------------------------------------------------------
QVariant OrderImporterCustomManager::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_names[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
Qt::ItemFlags OrderImporterCustomManager::flags(
        const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}
//----------------------------------------------------------
bool OrderImporterCustomManager::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index) != value && role == Qt::EditRole) {
        m_names[index.row()][index.column()] = value.toString();
        saveInSettings();
        return true;
    }
    return false;
}
//----------------------------------------------------------
QString OrderImporterCustomManager::_settingKey() const
{
    return "OrderImporterCustomManager_settingKey";
}
//----------------------------------------------------------

