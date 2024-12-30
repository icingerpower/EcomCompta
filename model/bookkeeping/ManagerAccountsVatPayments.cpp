#include <QSharedPointer>
#include <QSettings>

#include "model/orderimporters/Shipment.h"
#include "model/SettingManager.h"

#include "ManagerAccountsVatPayments.h"

//----------------------------------------------------------
QStringList ManagerAccountsVatPayments::COL_NAMES
= {QObject::tr("Nom"), QObject::tr("Compte")};
int ManagerAccountsVatPayments::IND_COL_ACCOUNT = 1;
int ManagerAccountsVatPayments::IND_COL_VAT_REGIME
= ManagerAccountsVatPayments::COL_NAMES.size();
//----------------------------------------------------------
ManagerAccountsVatPayments::ManagerAccountsVatPayments(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_listOfStringList << QStringList({"TVA à payer OSS", "445520", Shipment::VAT_REGIME_OSS, "id-tva-oss-to-pay"});
    m_listOfStringList << QStringList({"TVA à payer IOSS", "445530", Shipment::VAT_REGIME_IOSS, "id-tva-ioss-to-pay"});
}
//----------------------------------------------------------
void ManagerAccountsVatPayments::_clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    m_listOfStringList.clear();
    endRemoveRows();
}
//----------------------------------------------------------
void ManagerAccountsVatPayments::saveInSettings() const
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
void ManagerAccountsVatPayments::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settingKey().isEmpty()) {
        if (settings.contains(settingKey())) {
            //_clear();
            QList<QStringList> loadedValues;
            auto loadedString = settings.value(settingKey()).toString();
            QStringList lines = loadedString.split(":::");
            for (auto itLine = lines.begin();
                 itLine != lines.end(); ++itLine) {
                loadedValues << itLine->split(";;;");
            }
            for (auto itLoaded = loadedValues.begin();
                 itLoaded != loadedValues.end(); ++itLoaded) {
                for (auto itVal = m_listOfStringList.begin();
                     itVal != m_listOfStringList.end(); ++itVal) {
                    if (itLoaded->last() == itVal->last()) {
                        *itVal = *itLoaded;
                    }
                }
            }
            emit dataChanged(index(0, 0),
                             index(rowCount()-1, columnCount()-1));
        }
    }
}
//----------------------------------------------------------
ManagerAccountsVatPayments *ManagerAccountsVatPayments::instance()
{
    static QSharedPointer<ManagerAccountsVatPayments> instance
            = []() -> QSharedPointer<ManagerAccountsVatPayments>{
            QSharedPointer<ManagerAccountsVatPayments> _instance(
                new ManagerAccountsVatPayments);
            _instance->init();
            return _instance;
}();
    return instance.data();
}
//----------------------------------------------------------
QString ManagerAccountsVatPayments::getAccount(
        const QString &vatRegime)
{
    for (auto itList = m_listOfStringList.begin();
         itList != m_listOfStringList.end(); ++itList) {
        if (itList->value(IND_COL_VAT_REGIME) == vatRegime) {
            return itList->value(IND_COL_ACCOUNT);
        }
    }
    return QString();
}
//----------------------------------------------------------
void ManagerAccountsVatPayments::onCustomerSelectedChanged(
        const QString &customerId)
{
    //UpdateToCustomer::onCustomerSelectedChanged(customerId);
    loadFromSettings();
}
//----------------------------------------------------------
QString ManagerAccountsVatPayments::uniqueId() const
{
    return "ManagerAccountsVatPayments";
}
//----------------------------------------------------------
QVariant ManagerAccountsVatPayments::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return COL_NAMES[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int ManagerAccountsVatPayments::rowCount(
        const QModelIndex &) const
{
    return m_listOfStringList.size();
}
//----------------------------------------------------------
int ManagerAccountsVatPayments::columnCount(
        const QModelIndex &) const
{
    return COL_NAMES.size();
}
//----------------------------------------------------------
QVariant ManagerAccountsVatPayments::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_listOfStringList[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
bool ManagerAccountsVatPayments::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        m_listOfStringList[index.row()][index.column()] = value.toString();
        saveInSettings();
        return true;
    }
    return false;
}
//----------------------------------------------------------
Qt::ItemFlags ManagerAccountsVatPayments::flags(
        const QModelIndex &index) const
{
    if (index.column() > 0) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//----------------------------------------------------------
