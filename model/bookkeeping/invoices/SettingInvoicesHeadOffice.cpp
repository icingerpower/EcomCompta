#include <QSettings>
#include <QVariant>
#include <QDate>

#include "model/SettingManager.h"
#include "SettingInvoicesHeadOffice.h"

//----------------------------------------------------------
QStringList SettingInvoicesHeadOffice::colNames = {
    QObject::tr("Date to")
    , QObject::tr("Company address")
    , QObject::tr("Legal bottom notes")
    , QObject::tr("Law bottom notes")
};
//----------------------------------------------------------
SettingInvoicesHeadOffice::SettingInvoicesHeadOffice(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_listOfVariantList << QList<QVariant>(
                               {QVariant("Aujourd’hui"),
                                QVariant(QString()),
                                QVariant(QString()),
                                QVariant(QString())});
}
//----------------------------------------------------------
SettingInvoicesHeadOffice *SettingInvoicesHeadOffice::instance()
{
    static SettingInvoicesHeadOffice instance;
    static bool first = true;
    if (first) {
        instance.loadFromSettings();
        first = false;
    }
    return &instance;
}
//----------------------------------------------------------
QStringList SettingInvoicesHeadOffice::addressFrom(
        const QDate &date) const
{
    int row = _getRowOfDate(date);
    return m_listOfVariantList[row][1].toString().split("\n");
}
//----------------------------------------------------------
QStringList SettingInvoicesHeadOffice::textBottomLegal(
        const QDate &date) const
{
    int row = _getRowOfDate(date);
    return m_listOfVariantList[row][2].toString().split("\n");
}
//----------------------------------------------------------
QStringList SettingInvoicesHeadOffice::textBottomLaw(
        const QDate &date) const
{
    int row = _getRowOfDate(date);
    return m_listOfVariantList[row][3].toString().split("\n");
}
//----------------------------------------------------------
void SettingInvoicesHeadOffice::addDate(const QDate &date)
{
    int index = 1;
    for (index = 1; index<m_listOfVariantList.size(); ++index) {
        if (date > m_listOfVariantList[index][0].toDate()) {
            break;
        }
    }
    beginInsertRows(QModelIndex(), index, index);
    m_listOfVariantList.insert(
                index, QList<QVariant>(
                    {date,
                     QVariant(QString()),
                     QVariant(QString()),
                     QVariant(QString())}));
    endInsertRows();
}
//----------------------------------------------------------
void SettingInvoicesHeadOffice::removeDate(const QModelIndex &index)
{
    if (index.row() > 0) {
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        m_listOfVariantList.removeAt(index.row());
        endRemoveRows();
    }
}
//----------------------------------------------------------
void SettingInvoicesHeadOffice::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QStringList lines;
    for (auto it=m_listOfVariantList.begin();
         it!=m_listOfVariantList.end(); ++it) {
        QStringList elements = {it->first().toDate().toString("yyyy-MM-dd")};
        for (auto it2 = it->begin()+1;
             it2 != it->end(); ++it2) {
            elements << it2->toString();
        }
        lines << elements.join(SettingManager::SEP_COL);
    }
    settings.setValue(settingKey(), lines.join(
                          SettingManager::SEP_LINES));

}
//----------------------------------------------------------
void SettingInvoicesHeadOffice::loadFromSettings()
{
    beginRemoveRows(QModelIndex(), m_listOfVariantList.size()-1, m_listOfVariantList.size()-1);
    m_listOfVariantList.clear();
    endRemoveRows();
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(settingKey())) {
        QString text = settings.value(settingKey()).toString();
        QStringList lines = text.split(SettingManager::SEP_LINES);
        if (lines.size() > 0) {
            beginInsertRows(QModelIndex(), 0, lines.size()-1);
            for (auto line = lines.begin(); line != lines.end(); ++line) {
                QStringList elements = line->split(SettingManager::SEP_COL);
                QList<QVariant> variants = {QDate::fromString(elements.takeFirst(), "yyyy-MM-dd")};
                for (auto itEl = elements.begin();
                     itEl != elements.end(); ++itEl){
                    variants << *itEl;
                }
                /*
            if (m_listOfVariantList.contains(elements[0])) {
                int typeId = m_listOfVariantList[elements[0]].userType();
                m_paramValues[elements[0]] = elements[1];
                m_paramValues[elements[0]].convert(typeId);
            }
            //*/
                m_listOfVariantList << variants;
            }
            endInsertRows();
        }
    }
}
//----------------------------------------------------------
void SettingInvoicesHeadOffice::onCustomerSelectedChanged(
        const QString &)
{
    beginRemoveRows(QModelIndex(), 0, m_listOfVariantList.size()-1);
    m_listOfVariantList.clear();
    endRemoveRows();
    loadFromSettings();
}
//----------------------------------------------------------
QString SettingInvoicesHeadOffice::uniqueId() const
{
    return "SettingInvoicesHeadOffice";
}
//----------------------------------------------------------
QVariant SettingInvoicesHeadOffice::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return colNames[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int SettingInvoicesHeadOffice::rowCount(
        const QModelIndex &) const
{
    return m_listOfVariantList.size();
}
//----------------------------------------------------------
int SettingInvoicesHeadOffice::columnCount(
        const QModelIndex &) const
{
    return colNames.size();
}
//----------------------------------------------------------
QVariant SettingInvoicesHeadOffice::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_listOfVariantList[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
bool SettingInvoicesHeadOffice::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        m_listOfVariantList[index.row()][index.column()] = value;
        saveInSettings();
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}
//----------------------------------------------------------
Qt::ItemFlags SettingInvoicesHeadOffice::flags(
        const QModelIndex &index) const
{
    if (index.row() == 0 && index.column() == 0) {
        return QAbstractItemModel::flags(index);
    }
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}
//----------------------------------------------------------
int SettingInvoicesHeadOffice::_getRowOfDate(const QDate &date) const
{
    int row = m_listOfVariantList.size() - 1;
    while (m_listOfVariantList[row][0].toDate() < date && row > 0) {
        --row;
    }
    return row;
}
//----------------------------------------------------------
