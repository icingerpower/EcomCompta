#include <QtCore/qsettings.h>

#include "AbstractOrderImporter.h"
#include "model/orderimporters/ShippingAddressesManager.h"
#include "model/orderimporters/OrderImporterAmazonUE.h"
#include "model/orderimporters/OrderImporterAmazon.h"
#include "model/orderimporters/OrderImporterCustomManager.h"
#include "model/orderimporters/OrderImporterCdiscount.h"
#include "model/orderimporters/OrderImporterFnac.h"
#include "model/orderimporters/OrderImporterServiceSales.h"

//----------------------------------------------------------
AbstractOrderImporter::AbstractOrderImporter()
{
}
//----------------------------------------------------------
void AbstractOrderImporter::onCustomerSelectedChanged(
        const QString &customerId)
{
}
//----------------------------------------------------------
AbstractOrderImporter::~AbstractOrderImporter()
{
}
//----------------------------------------------------------
QList<AbstractOrderImporter *>
AbstractOrderImporter::allImporters()
{
    static QList<AbstractOrderImporter *> importers;
    if (importers.isEmpty()) {
        OrderImporterAmazonUE amazonUE;
        static OrderImporterAmazonUE amazonImporterUE;
        importers << &amazonImporterUE;
        static OrderImporterAmazon amazonImporter;
        importers << &amazonImporter;
        static OrderImporterCdiscount cdiscountImporter;
        importers << &cdiscountImporter;
        static OrderImporterFnac importerFnac;
        importers << &importerFnac;
    }
    QList<AbstractOrderImporter *> importersWithCustoms = importers;
    importersWithCustoms << OrderImporterCustomManager::instance()->allImporters();
    return importersWithCustoms;
}
//----------------------------------------------------------
AbstractOrderImporter *AbstractOrderImporter::importer(const QString &name)
{
    static QHash<QString, AbstractOrderImporter *>
            hashImporters = []() -> QHash<QString, AbstractOrderImporter *> {
        QHash<QString, AbstractOrderImporter *> importers;
        for (auto importer : AbstractOrderImporter::allImporters()) {
            importers[importer->name()] = importer;
        }
        static OrderImporterServiceSales importerSelfSales;
        importers[importerSelfSales.name()] = &importerSelfSales;
        return importers;
    }();
    return hashImporters[name];
}
//----------------------------------------------------------
QMultiHash<QString, QStringList> AbstractOrderImporter::reportForOrderCompleteMap(
        const Order *order) const
{
    QMultiHash<QString, QStringList> map;
    for (auto list : reportForOrderComplete(order)) {
        for (auto reportName : list) {
            map.insert(reportName, list);
        }
    }
    return map;
}
//----------------------------------------------------------
QString AbstractOrderImporter::nameForSetting() const
{
    QString nameSetting = name().replace(" ", "-");;
    return nameSetting;
}
//----------------------------------------------------------
bool AbstractOrderImporter::isVatToRecompute() const
{
    bool value = true;
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QString key = _settingKeyVatRecompute();
    if (settings.contains(key)) {
        value = settings.value(key).toBool();
    }
    return value;
}
//----------------------------------------------------------
void AbstractOrderImporter::setVatToRecompute(bool value)
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QString key = _settingKeyVatRecompute();
    settings.setValue(key, value);
}
//----------------------------------------------------------
bool AbstractOrderImporter::countryRequireVatDueThreshold(
        const QString &country) const
{
    (void) country;
    return false; // TODO get from other class
}
//----------------------------------------------------------
void AbstractOrderImporter::setDefaultShippingAddress(
        const Address &address)
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QString key = _settingKeyDefaultShippingAddress();
    settings.setValue(key, address.internalId());
}
//----------------------------------------------------------
QString AbstractOrderImporter::getDefaultShippingAddressId() const
{
    QString value = "";
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QString key = _settingKeyDefaultShippingAddress();
    if (settings.contains(key)) {
        value = settings.value(key).toString();
    } else if (ShippingAddressesManager::instance()->rowCount() > 0){
        value = ShippingAddressesManager::instance()->getAddress(0).internalId();
    } else {
        Q_ASSERT(false);
    }
    return value;
}
//----------------------------------------------------------
void AbstractOrderImporter::updateOrderIfMicroCountry(Order *order) const
{
    static QList<QStringList> toUpdate
            = {{"San Marino", "IT", "SM"}
               , {"Livigno", "IT", "SM"}
               , {"Lugano", "IT", "SM"}
               , {"Campione d'italia", "IT", "SM"}
              };
    for (auto it = toUpdate.begin(); it != toUpdate.end(); ++it) {
        if (order->getId() == "408-8663806-3875547") {
            int TEMP=19;++TEMP;
        }
        if (order->getAddressTo().state().toLower() == it->value(0).toLower()
                && order->getAddressTo().countryCode() == it->value(1)) {
            auto addressToUpdated = order->getAddressTo();
            addressToUpdated.setCountryCode(it->value(2));
            order->setAddressTo(addressToUpdated);
            break;
        }
    }
}
//----------------------------------------------------------
QString AbstractOrderImporter::_settingKeyVatRecompute() const
{
    return  settingKey() + "-" + name() + "-VatRecompute";
}
//----------------------------------------------------------
QString AbstractOrderImporter::_settingKeyDefaultShippingAddress() const
{
    return  settingKey() + "-" + name() + "-DefaultShippingAddress";
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
void OrderImporterException::raise() const
{
    throw *this;
}
//----------------------------------------------------------
//----------------------------------------------------------
OrderImporterException *OrderImporterException::clone() const
{
    return new OrderImporterException(*this);
}
//----------------------------------------------------------
//----------------------------------------------------------
QString OrderImporterException::error() const
{
    return m_error;
}
//----------------------------------------------------------
//----------------------------------------------------------
void OrderImporterException::setError(const QString &error)
{
    m_error = error;
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
