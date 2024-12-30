#include <QtCore/qsettings.h>

#include "../common/utils/CsvReader.h"

#include "model/SettingManager.h"
#include "ManagerBundle.h"

//----------------------------------------------------------
ManagerBundle *ManagerBundle::instance()
{
    static ManagerBundle instance;
    return &instance;
}
//----------------------------------------------------------

ManagerBundle::ManagerBundle(QObject *parent)
    : QAbstractTableModel(parent), UpdateToCustomer ()
{
    loadFromSettings();
}
//----------------------------------------------------------
ManagerBundle::~ManagerBundle()
{
}
//----------------------------------------------------------
QString ManagerBundle::uniqueId() const
{
    return "ManagerBundle";
}
//----------------------------------------------------------
void ManagerBundle::onCustomerSelectedChanged(
        const QString &)
{
    loadFromSettings();
}
//----------------------------------------------------------
void ManagerBundle::loadFromSettings()
{
    _clear();
    for (auto fileInfo : SettingManager::instance()
         ->dirInventoryBundles().entryInfoList(
             QStringList() << "*.csv", QDir::Files)) {
        _addBundleFile(fileInfo.absoluteFilePath());
    }
    /*
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(settingKey())) {
        QStringList lines = settings.value(
                    settingKey()).toString()
                .split(":::");
        int index = 0;
        for (auto line : lines) {
            auto elements = line.split(";;;");
            m_codeByOrders << elements[0];
            if (!m_mainCodeToSeverals.contains(elements[0])) {
                m_mainCodeToSeverals[elements[0]] = QList<QPair<QString, int>>();
                m_mainCodeToFirstIndex[elements[0]] = index;
            }
            m_mainCodeToSeverals[elements[0]]
                    << QPair<QString, int>(
                        elements[1], elements[2].toInt());
            ++index;
        }
        if (m_codeByOrders.size() > 0) {
            beginInsertRows(QModelIndex(), 0, m_codeByOrders.size()-1);
            endInsertRows();
        }
    }
    //*/
}
//----------------------------------------------------------
void ManagerBundle::_clear()
{
    if (m_codeByOrders.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_codeByOrders.size()-1);
        m_codeByOrders.clear();
        m_mainCodeToSeverals.clear();
        m_mainCodeToFirstIndex.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------
void ManagerBundle::_addBundleFile(
        const QString filePath)
{
    CsvReader csvReader(filePath, "\t");
    if (csvReader.readAll()) {
        auto dataRode = csvReader.dataRode();
        QHash<QString, QList<QPair<QString, int>>> mainCodeToSeveralsToAdd;
        for (auto elements : dataRode->lines) {
            if (!mainCodeToSeveralsToAdd.contains(elements[0])) {
                mainCodeToSeveralsToAdd[elements[0]]
                        = QList<QPair<QString, int>>();
            }
            mainCodeToSeveralsToAdd[elements[0]]
                    << QPair<QString, int>(
                        elements[1], elements[2].toInt());
        }
        for (auto it = mainCodeToSeveralsToAdd.begin();
             it != mainCodeToSeveralsToAdd.end(); ++it) {
            Q_ASSERT(!m_mainCodeToSeverals.contains(it.key())); // TODO trigger exception asking to delete / merge file with twice same code
            m_mainCodeToFirstIndex[it.key()] = m_codeByOrders.size();
            m_mainCodeToSeverals[it.key()]
                        = QList<QPair<QString, int>>();
            for (auto it2 = it.value().begin();
                 it2 != it.value().end(); ++it2) {
                m_codeByOrders << it.key();
                if (it.key().contains("fr-747150648330-1")) {
                    int TEMP=10;++TEMP;
                }
                m_mainCodeToSeverals[it.key()] << *it2;
            }
        }
    }
}
//----------------------------------------------------------
void ManagerBundle::saveInSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (m_codeByOrders.size() > 0) {
        QStringList lines;
        for (auto it1 = m_mainCodeToSeverals.begin();
             it1 != m_mainCodeToSeverals.end(); ++it1) {
            for (auto it2 = it1.value().begin();
                 it2 != it1.value().end(); ++it2) {
                QStringList elements({it1.key(),
                                      it2->first,
                                      QString::number(it2->second)});
                lines << elements.join(";;;");
            }
        }
        settings.setValue(settingKey(), lines.join(":::"));
    } else if (settings.contains(settingKey())) {
        settings.remove(settingKey());
    }
}
//----------------------------------------------------------
QList<QPair<QString, int> > ManagerBundle::codesBase(
        const QString &mainCode) const
{
    return m_mainCodeToSeverals[mainCode];
}
//----------------------------------------------------------
bool ManagerBundle::isBundle(const QString &code) const
{
    return m_mainCodeToSeverals.contains(code);
}
//----------------------------------------------------------
void ManagerBundle::addBundleFile(const QString filePath)
{
    QDir dir = SettingManager::instance()->dirInventoryBundles();
    QFileInfo fileInfo(filePath);
    QString newFilePath = dir.filePath(fileInfo.fileName());
    QFile::copy(filePath, newFilePath);
    int startIndex = m_codeByOrders.size();
    _addBundleFile(newFilePath);
    if (m_codeByOrders.size() > startIndex) {
        beginInsertRows(QModelIndex(), startIndex, m_codeByOrders.size()-1);
        endInsertRows();
    }
}
//----------------------------------------------------------
QVariant ManagerBundle::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal
            && role == Qt::DisplayRole) {
        static QStringList titles
                = {tr("Code parent"),
                   tr("Code", "Product code"),
                   tr("Quantité")};
        return titles[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int ManagerBundle::rowCount(const QModelIndex &) const
{
    return m_codeByOrders.size();
}
//----------------------------------------------------------
int ManagerBundle::columnCount(const QModelIndex &) const
{
    return 3;
}
//----------------------------------------------------------
QVariant ManagerBundle::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        static QList<std::function<QVariant(const ManagerBundle *, const QModelIndex &)>> functions
                                                                    = {
                [](const ManagerBundle *manager, const QModelIndex &index) -> QVariant{
            return manager->m_codeByOrders[index.row()];
        }, [](const ManagerBundle *manager, const QModelIndex &index) -> QVariant{
            QString code = manager->m_codeByOrders[index.row()];
            int firstIndex = manager->m_mainCodeToFirstIndex[code];
            int diffIndex = index.row() - firstIndex;
            return manager->m_mainCodeToSeverals[code][diffIndex].first;
        }, [](const ManagerBundle *manager, const QModelIndex &index) -> QVariant{
            QString code = manager->m_codeByOrders[index.row()];
            int firstIndex = manager->m_mainCodeToFirstIndex[code];
            int diffIndex = index.row() - firstIndex;
            return manager->m_mainCodeToSeverals[code][diffIndex].second;
        }
    };
        return functions[index.column()](this, index);
    }
    if (!index.isValid())
        return QVariant();
    return QVariant();
}
//----------------------------------------------------------
