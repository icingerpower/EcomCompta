#include <QtCore/qlocale.h>
#include <qapplication.h>

#include "ManagerCompanyVatParams.h"
#include "../common/currencies/CurrencyManager.h"
#include "../common/countries/CountryManager.h"


QString ManagerCompanyVatParams::VAL_CURRENCY = QObject::tr("Monaie");
QString ManagerCompanyVatParams::VAL_COUNTRY = QObject::tr("Pays du siège social");
//----------------------------------------------------------
ManagerCompanyVatParams::ManagerCompanyVatParams(QObject *parent)
    : QAbstractTableModel(parent)
{
    QLocale locale = QLocale::system();
    QString countryCode = locale.name().split("_").last();
    m_values[VAL_COUNTRY] = CountryManager::instance()->countryName(countryCode);
    m_values[VAL_CURRENCY] = locale.currencySymbol(QLocale::CurrencyIsoCode);;
    // TODO save in setting for each customer
}
//----------------------------------------------------------
ManagerCompanyVatParams *ManagerCompanyVatParams::instance()
{
    static ManagerCompanyVatParams instance;
    return &instance;
}
//----------------------------------------------------------
QString ManagerCompanyVatParams::currency() const
{
    return m_values[VAL_CURRENCY].toString();
}
//----------------------------------------------------------
QString ManagerCompanyVatParams::countryNameCompany() const
{
    return m_values[VAL_COUNTRY].toString();
}
//----------------------------------------------------------
QString ManagerCompanyVatParams::countryCodeCompany() const
{
    return CountryManager::instance()->countryCode(
                m_values[VAL_COUNTRY].toString());
}
//----------------------------------------------------------
QVariant ManagerCompanyVatParams::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        static QStringList values = {tr("Paramètre"), tr("Valeur")};
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int ManagerCompanyVatParams::rowCount(const QModelIndex &) const
{
    return 2;
}
//----------------------------------------------------------
int ManagerCompanyVatParams::columnCount(const QModelIndex &) const
{
    return 2;
}
//----------------------------------------------------------
QVariant ManagerCompanyVatParams::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() == 0) {
            return *m_values.keyIt(index.row());
        } else if (index.column() == 1) {
            return m_values.valueByIndex(index.row());
        }
    }
    return QVariant();
}
//----------------------------------------------------------
Qt::ItemFlags ManagerCompanyVatParams::flags(
        const QModelIndex &index) const
{
    if (index.column() == 1 && index.row() == 0) {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }
    return Qt::ItemIsEnabled;
}
//----------------------------------------------------------
bool ManagerCompanyVatParams::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.row() == 0) {
        m_values.setValue(index.row(), value);
        auto currencyIndex = this->index(1, 1, QModelIndex());
        QString currencyCountry
                = CurrencyManager::instance()->currency(
                    countryCodeCompany());
        m_values.setValue(1, currencyCountry);
        emit dataChanged(currencyIndex, currencyIndex, QVector<int>({Qt::DisplayRole}));
        return true;
    }
    return false;
}
//----------------------------------------------------------

