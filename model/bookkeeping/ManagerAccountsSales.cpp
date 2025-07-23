#include <QtCore/qsettings.h>

#include "../common/countries/CountryManager.h"

#include "model/orderimporters/Shipment.h"
#include "model/bookkeeping/ManagerSaleTypes.h"
#include "ExceptionAccountSaleMissing.h"

#include "ManagerAccountsSales.h"


//----------------------------------------------------------
ManagerAccountsSales *ManagerAccountsSales::instance()
{
    static ManagerAccountsSales instance;
    return &instance;
}
//----------------------------------------------------------
ManagerAccountsSales::~ManagerAccountsSales()
{
}
//----------------------------------------------------------
ManagerAccountsSales::ManagerAccountsSales(QObject *parent)
    : QAbstractTableModel(parent), UpdateToCustomer()
{
    _generateBasicAccounts();
    init();
}
//----------------------------------------------------------
void ManagerAccountsSales::onCustomerSelectedChanged(
        const QString &customerId)
{
    if (customerId.isEmpty()) {
        _clear();
    } else {
        loadFromSettings();
    }
}
//----------------------------------------------------------
QString ManagerAccountsSales::uniqueId() const
{
    return "ManagerAccountsSales";
}
//----------------------------------------------------------
void ManagerAccountsSales::_clear()
{
    beginRemoveRows(QModelIndex(), 0, m_values.size()-1);
    m_values.clear();
    endRemoveRows();
}
//----------------------------------------------------------
void ManagerAccountsSales::_generateBasicAccounts()
{
    m_values << QStringList({Shipment::VAT_REGIME_NONE, "",
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707901", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NONE, CountryManager::EU,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707200", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NONE, CountryManager::EU,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "708210", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NONE, CountryManager::UK,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707030", "445791"});
    // TODO create REGIME UE DDP
    /*
    m_values << QStringList({Shipment::VAT_REGIME_NONE, CountryManager::IRELAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707200", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NONE, CountryManager::GERMANY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707200", ""});
                             //*/

    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::UK,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707030", "445791"});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::UK,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707600", ""});

    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::FRANCE,
                             ManagerSaleTypes::SALE_PAYMENT_FASCILITOR, "0.00", "708500", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::CZECH,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707040", "445724"});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::GERMANY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.19", "707034", "445718"});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::GERMANY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.16", "707059", "445729"});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::SPAIN,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707032", "445716"});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::FRANCE,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707000", "445710"});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::ITALY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.22", "707036", "445720"});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::POLAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.23", "707038", "445722"});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::NETHERLAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707001", "445726"});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::SWEDEN,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.25", "707002", "445727"});

    //m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::UK,
                             //ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707001", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, "",
                             ManagerSaleTypes::SALE_PAYMENT_FASCILITOR, "0.00", "707XXX", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::CZECH,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707522", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::GERMANY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707501", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::SPAIN,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707505", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::FRANCE,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707001", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::ITALY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707514", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::POLAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707520", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::NETHERLAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707519", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL, CountryManager::SWEDEN,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707526", ""});


    m_values << QStringList({Shipment::VAT_REGIME_NORMAL_EXPORT, CountryManager::FRANCE,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707020", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL_EXPORT, CountryManager::GERMANY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707021", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL_EXPORT, CountryManager::ITALY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707022", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL_EXPORT, CountryManager::SPAIN,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707023", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL_EXPORT, CountryManager::POLAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707024", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL_EXPORT, CountryManager::CZECH,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707025", ""});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL_EXPORT, CountryManager::UK,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707026", ""});
    /*
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL_EXPORT, CountryManager::NETHERLAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707011", "445725"});
    m_values << QStringList({Shipment::VAT_REGIME_NORMAL_EXPORT, CountryManager::SWEDEN,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.00", "707012", "445726"});
                             //*/

    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::GERMANY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.19", "707060", "445730"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::BELGIUM,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707061", "445731"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::AUSTRIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707062", "445732"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::BULGARIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707063", "445733"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::CHYPRE,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.19", "707064", "445734"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::CROATIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.25", "707065", "445735"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::DENMARK,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.25", "707066", "445736"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::SPAIN,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707067", "445737"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::ESTONIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707068", "445738"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::FINLAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.24", "707069", "445739"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::FRANCE,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707070", "445740"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::GREECE,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.24", "707071", "445741"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::HONGRY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.27", "707072", "445742"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::IRELAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.23", "707073", "445743"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::ITALY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.22", "707074", "445744"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::LATVIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707075", "445745"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::LITHUANIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707076", "445746"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::LUXEMBOURG,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.17", "707077", "445747"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::LUXEMBOURG,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.16", "707087", "445757"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::MALTA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.18", "707078", "445748"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::NETHERLAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707079", "445749"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::POLAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.23", "707080", "445750"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::PORTUGAL,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.23", "707081", "445751"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::CZECH,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707082", "445752"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::ROMANIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.19", "707083", "445753"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::SLOVAKIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707084", "445754"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::SLOVENIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.22", "707085", "445755"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::SWEDEN,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.25", "707086", "445756"});
    m_values << QStringList({Shipment::VAT_REGIME_OSS, CountryManager::IRELAND_NORTHEN,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707088", "445758"});

    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::GERMANY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.19", "707100", "445760"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::BELGIUM,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707101", "445761"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::AUSTRIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707102", "445762"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::BULGARIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707103", "445763"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::CHYPRE,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.19", "707104", "445764"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::CROATIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.25", "707105", "445765"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::DENMARK,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.25", "707106", "445766"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::SPAIN,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707107", "445767"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::ESTONIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707108", "445768"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::FINLAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.24", "707109", "445769"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::FRANCE,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707127", "445787"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::GREECE,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.24", "707111", "445771"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::HONGRY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.27", "707112", "445772"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::IRELAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.23", "707113", "445773"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::ITALY,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.22", "707114", "445774"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::LATVIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707115", "445775"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::LITHUANIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707116", "445776"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::LUXEMBOURG,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.17", "707117", "445777"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::LUXEMBOURG,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.16", "707127", "445787"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::MALTA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.18", "707118", "445778"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::NETHERLAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707119", "445779"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::POLAND,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.23", "707120", "445780"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::PORTUGAL,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.23", "707121", "445781"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::CZECH,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.21", "707122", "445782"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::ROMANIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.19", "707123", "445783"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::SLOVAKIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707124", "445784"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::SLOVENIA,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.22", "707125", "445785"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::SWEDEN,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.25", "707126", "445786"});
    m_values << QStringList({Shipment::VAT_REGIME_IOSS, CountryManager::IRELAND_NORTHEN,
                             ManagerSaleTypes::SALE_PRODUCTS, "0.20", "707128", "445788"});

    _generateMapping();
}
//----------------------------------------------------------
void ManagerAccountsSales::_generateMapping()
{
    m_mapping.clear();
    for (int pos=0; pos < m_values.size(); ++pos) {
        QString regime = m_values[pos][0];
        QString country = m_values[pos][1];
        QString saleType = m_values[pos][2];
        QString vatRate = m_values[pos][3];
        if (!m_mapping.contains(regime)) {
            m_mapping[regime] = QHash<QString, QHash<QString, QHash<QString, int>>>();
        }
        if (!m_mapping[regime].contains(country)) {
            m_mapping[regime][country] = QHash<QString, QHash<QString, int>>();
        }
        if (!m_mapping[regime][country].contains(saleType)) {
            m_mapping[regime][country][saleType] = QHash<QString, int>();
        }
        m_mapping[regime][country][saleType][vatRate] = pos;
    }
}
//----------------------------------------------------------
void ManagerAccountsSales::saveInSettings() const
{
    //return;
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    settings.remove(settingKey());
    if (!settingKey().isEmpty()) {
        QStringList lines;
        for (auto line : m_values) {
            lines << line.join(";;;");
        }
        settings.setValue(settingKey(), lines.join(":::"));
    }
}
//----------------------------------------------------------
void ManagerAccountsSales::loadFromSettings()
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
ManagerAccountsSales::Accounts ManagerAccountsSales::getAccounts(
        const QString &regime,
        const QString &countryCode,
        const QString &saleType,
        const QString &vatRate) const
{
    Accounts accounts;
    if (saleType == ManagerSaleTypes::SALE_SERVICES) {
        accounts.titleBase = tr("Vente de service");
    } else if (saleType == ManagerSaleTypes::SALE_PAYMENT_FASCILITOR) {
        accounts.titleBase = tr("Frais accessoire sur vente");
    } else if (saleType == ManagerSaleTypes::SALE_PRODUCTS) {
        accounts.titleBase = tr("Vente de marchandise"); // TODO value from settings
    } else {
        Q_ASSERT(false);
    }
    //Q_ASSERT(countryCode.size() == 2);

    QString countryName;
    if (countryCode.size() > 0) {
        countryName = CountryManager::instance()->countryName(
                countryCode);
    }
    if (regime == Shipment::VAT_REGIME_NONE
        && (countryCode == CountryManager::EU
            || CountryManager::instance()->countriesCodeUE()->contains(countryCode))) {
        if (countryCode != "GB")
        {
            countryName = CountryManager::EU;
        }
    }
    if (!m_mapping[regime].contains(countryName)) {
        if (regime == Shipment::VAT_REGIME_NONE
                || regime == Shipment::VAT_REGIME_NORMAL_EXPORT) {
            countryName = "";
        }
    }
    if (countryName == "France" && vatRate.contains(".2")) {
        int TEMP=10;++TEMP;
    }
    bool ok1 = m_mapping.contains(regime);
    bool ok2 = m_mapping[regime].contains(countryName);
    bool ok3 = m_mapping[regime][countryName].contains(saleType);
    bool ok4 = m_mapping[regime][countryName][saleType].contains(vatRate);
    if (!m_mapping.contains(regime)
            || !m_mapping[regime].contains(countryName)
            || !m_mapping[regime][countryName].contains(saleType)
            || !m_mapping[regime][countryName][saleType].contains(vatRate)) {
        ExceptionAccountSaleMissing exception;
        exception.setAccounts(
                    regime
                    + " - " + countryName
                    + " - " + saleType
                    + " - " + vatRate);
        exception.raise();
    }
    int row = m_mapping[regime][countryName][saleType][vatRate];
    accounts.saleAccount = m_values[row][4];
    accounts.vatAccount = m_values[row][5];
    //TODO exception
    return accounts;
}
//----------------------------------------------------------
void ManagerAccountsSales::addRow(
        const QString &regime,
        const QString &country,
        const QString &saleType,
        const QString &vatRate,
        const QString &accountSale,
        const QString &accountVat)
{
    beginInsertRows(QModelIndex(), 0, 0);
    m_values.insert(0, QStringList({regime, country, saleType, vatRate, accountSale, accountVat}));
    saveInSettings();
    _generateMapping();
    endInsertRows();

}
//----------------------------------------------------------
void ManagerAccountsSales::removeRow(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    m_values.removeAt(index);
    _generateMapping();
    saveInSettings();
    endRemoveRows();
}
//----------------------------------------------------------
QVariant ManagerAccountsSales::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList values
                = {tr("Régime"), tr("Country"), tr("Type de transaction"),
                   tr("Taux TVA"), tr("Comptes vente"), tr("Compte TVA")};
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int ManagerAccountsSales::rowCount(const QModelIndex &) const
{
    return m_values.size();
}
//----------------------------------------------------------
int ManagerAccountsSales::columnCount(const QModelIndex &) const
{
    return 6;
}
//----------------------------------------------------------
QVariant ManagerAccountsSales::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_values[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
Qt::ItemFlags ManagerAccountsSales::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}
//----------------------------------------------------------
bool ManagerAccountsSales::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (index.column() == 3) {
            m_values[index.row()][index.column()] = SettingManager::formatVatRate(value.toDouble());
        } else {
            m_values[index.row()][index.column()] = value.toString();
        }
        _generateMapping();
        saveInSettings();
        return true;
    }
    return false;
}
//----------------------------------------------------------
void ManagerAccountsSales::sort(int column, Qt::SortOrder order)
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
