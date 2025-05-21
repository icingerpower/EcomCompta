#include <qdebug.h>

#include "../common/currencies/CurrencyRateManager.h"
#include "../common/countries/CountryManager.h"

#include "model/vat/VatNumbersModel.h"
#include "model/vat/VatRateManager.h"
#include "model/vat/ManagerCompanyVatParams.h"
#include "model/CustomerManager.h"
#include "model/SettingManager.h"
#include "model/orderimporters/AbstractOrderImporter.h"
#include "model/orderimporters/OrderImporterServiceSales.h"
#include "model/orderimporters/OrderImporterAmazonUE.h"
#include "model/bookkeeping/invoices/SettingInvoices.h"
#include "model/bookkeeping/ManagerSaleTypes.h"

#include "Shipment.h"
#include "Order.h"
#include "Refund.h"

//#define AMAZON_RIGHT true

#include "model/bookkeeping/ManagerAccountsSales.h"

QString Shipment::VAT_REGIME_NONE = QObject::tr("Hors UE", "Pas de TVA en UE");
//QString Shipment::VAT_REGIME_UE_DDP = QObject::tr("UE DDP", "UE DDP de la part d'un autre exporteur");
QString Shipment::VAT_REGIME_NORMAL = QObject::tr("Normal");
QString Shipment::VAT_REGIME_NORMAL_EXPORT = QObject::tr("Normal export");
QString Shipment::VAT_REGIME_OSS = QObject::tr("OSS", "Régime de TVA normal");
QString Shipment::VAT_REGIME_IOSS = QObject::tr("IOSS");
QString Shipment::VAT_RESPONSIBLE_SELLER = QObject::tr("Vendeur");
QString Shipment::VAT_RESPONSIBLE_MARKETPLACE = QObject::tr("Marketplace");
QHash<QString, QHash<QString, int>> Shipment::m_countriesFromByCustomerId
= QHash<QString, QHash<QString, int>>();
//==========================================================
QStringList *Shipment::allRegimes()
{
    static QStringList allRegimes
            = QStringList({VAT_REGIME_NONE,
                           VAT_REGIME_NORMAL,
                           VAT_REGIME_NORMAL_EXPORT,
                           VAT_REGIME_OSS,
                           VAT_REGIME_IOSS});
    return &allRegimes;
}

QString Shipment::amazonCountryDeclToCode(const QString &countryNameVatReport)
{
    static QHash<QString, QString> mapping
            = {{"FRANCE", "FR"}
               ,{"UNITED KINGDOM", "GB"}
               ,{"GERMANY", "DE"}
               ,{"SPAIN", "ES"}
               ,{"ITALY", "IT"}
               ,{"POLAND", "PL"}
               ,{"CZECH REPUBLIC", "CZ"}
               ,{"NETHERLANDS", "NL"}
               ,{"SWEDEN", "SE"}
               ,{"BELGIUM", "BE"}
               ,{"BULGARIA", "BG"}
               ,{"DENMARK", "DK"}
               ,{"ESTONIA", "EE"}
               ,{"IRELAND", "IE"}
               ,{"CROATIA", "HR"}
               ,{"CYPRUS", "CY"}
               ,{"LETTONIA", "LV"}
               ,{"LITUANIA", "LT"}
               ,{"LITHUANIA", "LT"}
               ,{"GREECE", "GR"}
               ,{"FINLAND", "FI"}
               ,{"LUXEMBOURG", "LU"}
               ,{"HUNGARY", "HU"}
               ,{"MALTA", "MT"}
               ,{"AUSTRIA", "AT"}
               ,{"PORTUGAL", "PT"}
               ,{"ROMANIA", "RO"}
               ,{"SLOVENIA", "SI"}
               ,{"SLOVAKIA", "SK"}
               ,{"LATVIA", "LV"}
               ,{"", ""}
               };
    const QString &upperCountry = countryNameVatReport.toUpper();
    Q_ASSERT(mapping.contains(upperCountry));
    return mapping[upperCountry];
}
//==========================================================
Shipment::Shipment(const QString &id,
                   const QHash<QString, QSharedPointer<ArticleSold>> &articlesShipped,
                   const Shipping &shipping,
                   const QDateTime &dateTime,
                   const Address &addressFrom,
                   const QString &currency,
                   double vatForRoundCorrection)
{
    m_order = nullptr;
    m_fromAmazonVatReports = false;
    m_id = id;
    m_currency = currency;
    m_vatCollectResponsible = "";
    Q_ASSERT(m_currency.size() == 3);
    m_shipping = shipping;
    setArticlesShipped(articlesShipped);
    m_dateTime = dateTime;
    m_addressFrom = addressFrom;
    m_vatForRoundCorrection = vatForRoundCorrection;
    m_totalPriceTaxedAmazon = 0.;
    m_totalPriceTaxesAmazon = 0.;
    _recordAddressFromCountry();
}
//==========================================================
void Shipment::setAmazonVatInformations(
        const QString &countryVatAmazon,
        const QString &countrySaleDeclarationAmazon,
        const QString &regimeVatAmazon,
        double totalPriceTaxedAmazon,
        double totalPriceTaxesAmazon)
{
    m_countrySaleDeclarationAmazon = countrySaleDeclarationAmazon;
    m_countryVatAmazon = countryVatAmazon;
    if (regimeVatAmazon == "REGULAR")
    {
        m_regimeVatAmazon = VAT_REGIME_NORMAL;
    }
    else if (regimeVatAmazon == "UNION-OSS")
    {
        m_regimeVatAmazon = VAT_REGIME_OSS;
    }
    else if (regimeVatAmazon == "UK_VOEC-DOMESTIC")
    {
        m_regimeVatAmazon = VAT_REGIME_NONE;
    }
    else if (regimeVatAmazon == "UK_VOEC-IMPORT")
    {
        m_regimeVatAmazon = VAT_REGIME_NORMAL_EXPORT;
    }
    else if (regimeVatAmazon == "CH_VOEC")
    {
        m_regimeVatAmazon = VAT_REGIME_NORMAL_EXPORT;
    }
    else if (regimeVatAmazon == "NO_VOEC")
    {
        m_regimeVatAmazon = VAT_REGIME_NORMAL_EXPORT;
    }
    else if (regimeVatAmazon.contains("IOSS"))
    {
        m_regimeVatAmazon = VAT_REGIME_IOSS;
    }
    else
    {
        Q_ASSERT(false);
    }
    m_regimeVatAmazon = regimeVatAmazon;
    m_totalPriceTaxedAmazon = totalPriceTaxedAmazon;
    m_totalPriceTaxesAmazon = totalPriceTaxesAmazon;
}
//==========================================================
QString Shipment::getAmazonVatRate() const
{
    double untaxed = getTotalPriceUntaxedAmazon();
    if (qAbs(untaxed) > 0.00001)
    {
        double rate = getTotalPriceTaxesAmazon() / untaxed;
        return QString::number(rate, 'f', 2);
    }
    return "0.00";
}
//==========================================================
bool Shipment::hasAmazonVatInformations() const
{
    return !m_countryVatAmazon.isEmpty();
}
//==========================================================
Shipment::~Shipment()
{

}
//==========================================================
void Shipment::_recordAddressFromCountry()
{
    if (!m_addressFrom.isEmpty()) {
        QString customerId = CustomerManager::instance()->getSelectedCustomerId();
        if (!customerId.isEmpty()) {
            if (!m_countriesFromByCustomerId.contains(customerId)) {
                m_countriesFromByCustomerId[customerId] = QHash<QString, int>();
            }
            if (!m_countriesFromByCustomerId[customerId].contains(m_addressFrom.countryCode())) {
                m_countriesFromByCustomerId[customerId][m_addressFrom.countryCode()] = 1;
            } else {
                ++m_countriesFromByCustomerId[customerId][m_addressFrom.countryCode()];
            }
        }
    }
}
//==========================================================
QHash<QString, QDate> Shipment::getReportsFrom() const
{
    return m_reportsFrom;
}
//==========================================================
QSet<QString> Shipment::getSaleTypes() const
{
    QSet<QString> saleTypes;
    for (auto itArt = m_articlesShipped.begin();
         itArt != m_articlesShipped.end(); ++itArt) {
        saleTypes << (*itArt)->getSaleType();
    }
    return saleTypes;
}

QString Shipment::vatScheme() const
{
    return m_vatScheme;
}

void Shipment::setVatScheme(const QString &newVatScheme)
{
    m_vatScheme = newVatScheme;
}
//==========================================================
bool Shipment::getFromAmazonVatReports() const
{
    return m_fromAmazonVatReports;
}
//==========================================================
void Shipment::setFromAmazonVatReports(bool fromAmazonVatReports)
{
    m_fromAmazonVatReports = fromAmazonVatReports;
}
//==========================================================
void Shipment::addReportFrom(
        const QDate &date, const QString &reportName)
{
    m_reportsFrom[reportName] = date;
}
//==========================================================
QString Shipment::getCountrySaleDeclaration() const
{
    return m_countrySaleDeclaration;
}
//==========================================================
QString Shipment::getCountrySaleDeclarationName() const
{
    return CountryManager::instance()->countryName(m_countrySaleDeclaration);
}
//==========================================================
QString Shipment::getInvoiceName() const
{
    if (m_invoiceNumberMarketplace.isEmpty()) {
        return m_invoiceNumber;
    }
    return m_invoiceNumberMarketplace;
}
//==========================================================
void Shipment::setInvoiceName(const QString &invoiceNumber)
{
    m_invoiceNumber = invoiceNumber;
    m_invoiceNumber.replace(" ", "-");
    m_invoiceNumber.replace("\t", "-");
}
//==========================================================
QString Shipment::getVatCollectResponsible() const
{
    if (m_vatCollectResponsible.isEmpty()) {
        return VAT_RESPONSIBLE_SELLER;
    }
    return m_vatCollectResponsible;
}
//==========================================================
void Shipment::setVatCollectResponsible(const QString &vatCollectResponsible)
{
    m_vatCollectResponsible = vatCollectResponsible;
}
//==========================================================
QString Shipment::getInvoiceNameMarketPlace() const
{
    return m_invoiceNumberMarketplace;
}
//==========================================================
void Shipment::setInvoiceNameMarketplace(
        const QString &invoiceNumber)
{
    m_invoiceNumberMarketplace = invoiceNumber;
}
//==========================================================
QString Shipment::getCurrency() const
{
    return m_currency;
}
//==========================================================
void Shipment::merge(const Shipment &shipment)
{
    if (m_dateTime.isNull()) {
        m_dateTime = shipment.m_dateTime;
    } else if (!shipment.m_dateTime.isNull()) {
        if (m_dateTime.time() == QTime(23,59)) {
            m_dateTime = shipment.m_dateTime;
        }
    }
    if (shipment.m_dateTime.daysTo(m_dateTime) > 1) {
        m_dateTime = shipment.m_dateTime; // Changed on february 22 2022
    }
    bool addedArticles = false;
    //*
    bool mergeArticlesFromLast = false;
    if (m_reportsFrom.contains(OrderImporterAmazonUE::REPORT_ORDERS_INVOICING_SHORT)
            && shipment.m_reportsFrom.contains(OrderImporterAmazonUE::REPORT_ORDERS_INVOICING_SHORT)
            && (m_reportsFrom[OrderImporterAmazonUE::REPORT_ORDERS_INVOICING_SHORT]
                != shipment.m_reportsFrom[OrderImporterAmazonUE::REPORT_ORDERS_INVOICING_SHORT])) {
        mergeArticlesFromLast = true; // TODO WORKAROUND for order like 305-7159603-7269163 that first miss ID article shipment, and then not, leading to same article shipped with different ids
    }

    if (mergeArticlesFromLast) {
        if (shipment.m_reportsFrom[OrderImporterAmazonUE::REPORT_ORDERS_INVOICING_SHORT]
                > m_reportsFrom[OrderImporterAmazonUE::REPORT_ORDERS_INVOICING_SHORT]) {
            m_articlesShipped = shipment.m_articlesShipped;
            for (auto itArticle = m_articlesShipped.begin();
                 itArticle != m_articlesShipped.end();
                 ++itArticle) {
                m_articlesShipped[itArticle.key()]->init(this);
            }
        }
    } else {
        for (auto it=shipment.m_articlesShipped.begin();
             it != shipment.m_articlesShipped.end();
             ++it) {
            if (m_articlesShipped.contains(it.key())) {
                m_articlesShipped[it.key()]->merge(*it.value().data());
            } else {
                m_articlesShipped[it.key()] = it.value();
                m_articlesShipped[it.key()]->init(this);
                addedArticles = true;
            }
        }
    }
        //*
    for (auto itReportFrom = shipment.m_reportsFrom.begin();
         itReportFrom != shipment.m_reportsFrom.end(); ++itReportFrom) {
        if (!m_reportsFrom.contains(itReportFrom.key())
                || m_reportsFrom[itReportFrom.key()] < itReportFrom.value()) {
            m_reportsFrom[itReportFrom.key()] = itReportFrom.value();
        }
    }
    //*/
    //m_reportsFrom.unite(shipment.m_reportsFrom);
    if (addedArticles) {
        m_vatForRoundCorrection += shipment.m_vatForRoundCorrection;
    } else if (m_vatForRoundCorrection == 0. && shipment.m_vatForRoundCorrection > 0.001) { /// This shouldn't happen I think except if both 0
        m_vatForRoundCorrection = shipment.m_vatForRoundCorrection;
    } else if (qAbs(m_vatForRoundCorrection - shipment.m_vatForRoundCorrection) < 0.03
               && shipment.getFromAmazonVatReports()) { /// This shouldn't happen I think except if both 0
        m_vatForRoundCorrection = shipment.m_vatForRoundCorrection;
    }
    if (m_shipping.isNull()) {
        m_shipping = shipment.m_shipping;
    }
    if (m_paymentId.isEmpty()) {
        m_paymentId = shipment.m_paymentId;
    }
    if (m_invoiceNumberMarketplace.isEmpty()) {
        m_invoiceNumberMarketplace = shipment.m_invoiceNumberMarketplace;
    }
    if (m_vatScheme.isEmpty()) {
        m_vatScheme = shipment.m_vatScheme;
    }
    if (m_vatCollectResponsible.isEmpty()) {
        m_vatCollectResponsible = shipment.m_vatCollectResponsible;
    }
    m_addressFrom.merge(shipment.m_addressFrom);
    if (m_order == nullptr) {
        init(m_order);
    }
    for (auto itSku = shipment.m_chargedFeesBySKU.begin();
         itSku != shipment.m_chargedFeesBySKU.end();
         ++itSku) {
        for (auto itFee = itSku.value().begin();
             itFee != itSku.value().end();
             ++itFee) {
            for (int i=0; i<itFee.value().amounts.size(); ++i){
                QString id = itFee.value().ids[i];
                double amount = itFee.value().amounts[i];
                addChargedFee(id, itFee.key(), itSku.key(), amount);
            }
        }
    }
}
//==========================================================
QString Shipment::countryVat() const
{
#ifdef AMAZON_RIGHT
    if (!m_countryVatAmazon.isEmpty())
    {
        return m_countryVatAmazon;
    }
#endif
    return m_countryVat;
}
//==========================================================
QString Shipment::countryCodeTo() const
{
    return m_order->getAddressTo().countryCode();
}
//==========================================================
QString Shipment::countryCodeFrom() const
{
    return m_addressFrom.countryCode();
}
//==========================================================
QString Shipment::countryNameTo() const
{
    QString _countryNameTo = CountryManager::instance()
            ->countryName(countryCodeTo());
    return _countryNameTo;
}
//==========================================================
QString Shipment::countryNameFrom() const
{
    QString _countryNameFrom = CountryManager::instance()
            ->countryName(countryCodeFrom());
    return _countryNameFrom;
}
//==========================================================
QSet<QString> Shipment::skusSet() const
{
    QSet<QString> set;
    for (auto article : m_articlesShipped) {
        set << article->getSku();
    }
    return set;
}
//==========================================================
void Shipment::computeVatRegime(double &totalSaleCountryOss,
                                bool alsoRecomputeVat)
{
    static QHash<QString, int> lastInvoiceName;
    bool wasVatChared = qAbs(m_vatForRoundCorrection) > 0.001;
    //if (m_order->getId() == "403-7978774-7968342") { // VA 406-2471898-8862728
    if (m_order->getId() == "406-2471898-8862728") { // VAtican
        bool complete = isComplete();
        bool completeLoaded = isCompletelyLoaded();
        int TEMP=10;++TEMP;
    }
    QString countryFrom = m_addressFrom.countryCode();
    QString countryTo = m_order->getAddressTo().countryCode();
    double vatBefore = getTotalPriceTaxes();
    double thresholdCountryTo = VatNumbersModel::instance()->iossThreshold(); //VatThresholdModel::instance()->threshold(countryTo);
    auto countriesFromContain = [](const QString &countryFrom) -> bool {
        QString customerId = CustomerManager::instance()->getSelectedCustomerId();
        return m_countriesFromByCustomerId[customerId].contains(countryFrom);
    };
    int yearShipment = m_dateTime.date().year();
    //QString invoiceNumberMarketplace = getInvoiceNameMarketPlace();
    bool isAfterOss = m_dateTime >= QDateTime(QDate(2021,7,1)); /// This is te law
    //if (!invoiceNumberMarketplace.isEmpty() && isAfterOss)
    //if (!m_vatScheme.isEmpty() && isAfterOss)
    //{
        //isAfterOss = m_vatScheme.contains("UNION-OSS"); //|| invoiceNumberMarketplace.contains("UOSS");
    //}
    int yearOrder = m_order->getDateTime().date().year();
    /*
    QString yearOrderString = QString::number(yearOrder);
    QString invoicePrefix = getOrder()->getChannel()
            + "-" + getOrder()->getSubchannel()
            + "-" + QString::number(yearOrder) + "-";
    if (!lastInvoiceName.contains(invoicePrefix)) {
        lastInvoiceName[invoicePrefix] = 0;
    }
    ++lastInvoiceName[invoicePrefix];
    QString invoiceNumberGenerated
            = invoicePrefix + QString::number(lastInvoiceName[invoicePrefix]);
    setInvoiceName(invoiceNumberGenerated);
    //*/
    bool isBusinessCustomer = m_order->isBusinessCustomer();
    bool isVatNumberValid = m_order->isVatNumberValid();
    bool recomputeOrderShipping =
            alsoRecomputeVat
            && isFirstShipment()
            && !m_order->getShipping().isNull();
    Shipping orderShipping = m_order->getShipping();
    if ((!SettingManager::instance()->countriesUE(yearOrder)->contains(countryFrom)
            && !SettingManager::instance()->countriesUE(yearOrder)->contains(countryTo))
            || (!SettingManager::instance()->countriesUE(yearOrder)->contains(countryFrom)
                && !isAfterOss /// For instance order CN > Irelande in marche 2021
                && SettingManager::instance()->countriesUE(yearShipment)->contains(countryTo))) {
        /// No regime as not by UE
        m_regimeVat = Shipment::VAT_REGIME_NONE;
        m_countryVat = countryTo;
        m_countrySaleDeclaration = countryFrom; // Not sure here
        if (countryFrom == "UK") {
            m_countrySaleDeclaration = "GB"; // Not sure here
        }
        //m_countrySaleDeclaration = ManagerCompanyVatParams::instance()->countryCompany();
        if (alsoRecomputeVat) {
            if (isBusinessCustomer
                    && isVatNumberValid
                    && countryFrom == "UK"
                    && countryTo == "GB") { /// NOT UE but europe with vat to declare
                for (auto itArticle = m_articlesShipped.begin();
                     itArticle != m_articlesShipped.end();
                     ++itArticle) {
                    QString sku = itArticle.value()->getSku();
                    double vatRateTo = VatRateManager::instance()->vatRate(
                                countryTo, m_dateTime.date(), sku);
                    itArticle.value()->recomputeVat(vatRateTo);
                }
                double vatRate = VatRateManager::instance()->vatRateDefault(
                                            countryTo, m_dateTime.date());
                m_shipping.recomputeVat(vatRate);
                if (recomputeOrderShipping) {
                    orderShipping.recomputeVat(vatRate);
                    m_order->setShipping(orderShipping);
                }
            } else {
                if (!isMarketplaceResponsibleVat()) {
                    //Q_ASSERT(qAbs(getTotalPriceTaxes()) < 0.001); //TODO find out why I put this. Now we are there with taxes for sale US > US
                    ///It can mean export like UK > Portugal or US > US
                    for (auto article : m_articlesShipped) {
                        article->setTotalPriceTaxes(0.);
                        auto shipping = article->getShipping();
                        shipping.setTotalTaxes(0.);
                        article->setShipping(shipping);
                    }
                    m_vatForRoundCorrection = 0.;
                    m_shipping.setTotalTaxes(0.);
                    if (recomputeOrderShipping) {
                        orderShipping.setTotalTaxes(0.);
                        m_order->setShipping(orderShipping);
                    }
                }
            }
        }
    } else if (SettingManager::instance()->countriesUE(yearOrder)->contains(countryFrom)
               && !SettingManager::instance()->countriesUE(yearOrder)->contains(countryTo)) {
        /// Regime normal export
        m_regimeVat = Shipment::VAT_REGIME_NORMAL_EXPORT;
        m_countryVat = countryFrom; // TODO why did I put countryTo ????
        m_countrySaleDeclaration = countryFrom;
        if (alsoRecomputeVat) { /// This is for instance sales on amazon.com shipped from France. taxes should be removed
            if (!isMarketplaceResponsibleVat()) {
                for (auto article : m_articlesShipped) {
                    article->setTotalPriceTaxes(0.);
                    auto shipping = article->getShipping();
                    shipping.setTotalTaxes(0.);
                    article->setShipping(shipping);
                }
                m_vatForRoundCorrection = 0.;
                m_shipping.setTotalTaxes(0.);
                if (recomputeOrderShipping) {
                    orderShipping.setTotalTaxes(0.);
                    m_order->setShipping(orderShipping);
                }
            }
        }
        /// Sometimes, we see taxes here when amazon is responsibile to return vat
        /*
        m_vatForRoundCorrection = 0.;
        m_shipping.setTotalTaxes(0.);
        for (auto article : m_articlesShipped) {
            article->setTotalPriceTaxes(0.);
            auto shipping = article->getShipping();
            shipping.setTotalTaxes(0.);
            article->setShipping(shipping);
        }
        //*/
    } else if (SettingManager::instance()->countriesUE(yearOrder)->contains(countryFrom)
               && countryTo == "MC") {
        m_countryVat = "FR";
        m_countrySaleDeclaration = "FR";
        if (countryFrom == "FR" || !isAfterOss) {
            m_regimeVat = Shipment::VAT_REGIME_NORMAL;
        } else {
            m_regimeVat = Shipment::VAT_REGIME_OSS;
        }
        if (alsoRecomputeVat) {
            for (auto itArticle = m_articlesShipped.begin();
                 itArticle != m_articlesShipped.end();
                 ++itArticle) {
                QString sku = itArticle.value()->getSku();
                double vatRateFrom = VatRateManager::instance()->vatRate(
                            "FR", m_dateTime.date(), sku);
                itArticle.value()->recomputeVat(vatRateFrom);
            }
            double vatRate = VatRateManager::instance()->vatRateDefault(
                        "FR", m_dateTime.date());
            m_shipping.recomputeVat(vatRate);
            if (recomputeOrderShipping) {
                orderShipping.recomputeVat(vatRate);
                m_order->setShipping(orderShipping);
            }
        }
    } else if (SettingManager::instance()->countriesUE(yearOrder)->contains(countryFrom)
               && SettingManager::instance()->countriesUE(yearOrder)->contains(countryTo)
               && countryFrom != countryTo
               && isAfterOss) {
        /// OSS or NORMAL if under threshold
        double vatRate = VatRateManager::instance()->vatRateDefault(
                    countryFrom, m_dateTime.date());
        double taxesCountryFrom
                = m_shipping.totalPriceTaxed() * vatRate;
        if (recomputeOrderShipping) {
            taxesCountryFrom += orderShipping.totalPriceTaxed() * vatRate;
        }
        for (auto itArticle = m_articlesShipped.begin();
             itArticle != m_articlesShipped.end();
             ++itArticle) {
            QString sku = itArticle.value()->getSku();
            double vatRateFrom = VatRateManager::instance()->vatRate(
                        countryFrom, m_dateTime.date(), sku);
            taxesCountryFrom += itArticle.value()->getTotalPriceTaxed()*(1 - 1/(1+vatRateFrom));
        }
        double shipmentTotalTaxed = getTotalPriceTaxed();
        double totalTemp = totalSaleCountryOss + shipmentTotalTaxed - taxesCountryFrom;
        if (!isBusinessCustomer && totalTemp > thresholdCountryTo) {
            /// OSS Pay vat in countryTo
            m_regimeVat = Shipment::VAT_REGIME_OSS;
            m_countryVat = countryTo;
            m_countrySaleDeclaration = countryTo;
            if (alsoRecomputeVat) {
                for (auto itArticle = m_articlesShipped.begin();
                     itArticle != m_articlesShipped.end();
                     ++itArticle) {
                    QString sku = itArticle.value()->getSku();
                    double vatRateTo = VatRateManager::instance()->vatRate(
                                countryTo, m_dateTime.date(), sku);
                    itArticle.value()->recomputeVat(vatRateTo);
                }
                double vatRate = VatRateManager::instance()->vatRateDefault(
                                            countryTo, m_dateTime.date());
                m_shipping.recomputeVat(vatRate);
                if (recomputeOrderShipping) {
                    orderShipping.recomputeVat(vatRate);
                    m_order->setShipping(orderShipping);
                }
            }
        } else {
            /// Normal regime becaues under threshold or business transaction
            m_regimeVat = Shipment::VAT_REGIME_NORMAL;
            m_countryVat = countryTo;
            m_countrySaleDeclaration = countryFrom;
            /// Pay vat in countryFrom with NORMAL regime and regularisation needed if not registred
            // if (!VatNumbersModel::instance()->containsCountry(countryFrom)) {
            if (alsoRecomputeVat) {
                for (auto itArticle = m_articlesShipped.begin();
                     itArticle != m_articlesShipped.end();
                     ++itArticle) {
                    QString sku = itArticle.value()->getSku();
                    //double vatRateFrom = VatRateManager::instance()->vatRate(
                                //countryFrom, m_dateTime.date(), sku);
                    //itArticle.value()->recomputeVat(vatRateFrom);
                    // Change on june 20 2023
                    itArticle.value()->recomputeVat(0.);
                }
                //double vatRate = VatRateManager::instance()->vatRateDefault(
                                            //countryFrom, m_dateTime.date());
                double vatRate = 0.;
                m_shipping.recomputeVat(vatRate);
                if (recomputeOrderShipping) {
                    orderShipping.recomputeVat(vatRate);
                    m_order->setShipping(orderShipping);
                }
            }
        }
    } else if (SettingManager::instance()->countriesUE(yearOrder)->contains(countryFrom)
               && SettingManager::instance()->countriesUE(yearOrder)->contains(countryTo)
               && (countryTo == countryFrom || !isAfterOss || isBusinessCustomer)) {
        /// Normal VAT
        m_regimeVat = Shipment::VAT_REGIME_NORMAL;
        if ((isAfterOss || countriesFromContain(countryTo))) {
            if (isBusinessCustomer) {
                m_countrySaleDeclaration = countryFrom;
                m_countryVat = countryFrom;
            } else {
                m_countrySaleDeclaration = countryTo;
                m_countryVat = countryTo;
            }
        } else {
            m_countryVat = countryFrom; //This create 200 difference with amazon…because amazon say vat country is countryTo but use vat rate of country from
            m_countrySaleDeclaration = countryFrom;
        }
        //if (!isAfterOss) {
            //m_countrySaleDeclaration = countryFrom;
        //}
        if (alsoRecomputeVat) {
            for (auto itArticle = m_articlesShipped.begin();
                 itArticle != m_articlesShipped.end();
                 ++itArticle) {
                QString sku = itArticle.value()->getSku();
                double vatRate = VatRateManager::instance()->vatRate(
                            m_countryVat, m_order->getDateTime().date(), sku);
                itArticle.value()->recomputeVat(vatRate);
            }
            double vatRate = VatRateManager::instance()->vatRateDefault(
                        m_countryVat, m_order->getDateTime().date());
            m_shipping.recomputeVat(vatRate);
            if (recomputeOrderShipping) {
                orderShipping.recomputeVat(vatRate);
                m_order->setShipping(orderShipping);
            }
            double newVat = getTotalPriceTaxes(false);
            if (qAbs(m_vatForRoundCorrection - newVat) > 0.029) {
                m_vatForRoundCorrection = newVat;
            }
        }
    } else if (isAfterOss && !SettingManager::instance()->countriesUE(yearOrder)->contains(countryFrom)
               && SettingManager::instance()->countriesUE(yearOrder)->contains(countryTo)
               && m_order->getShippedBySeller()) {
        /// IOSS
        m_regimeVat = Shipment::VAT_REGIME_IOSS;
        m_countryVat = countryTo;
        m_countrySaleDeclaration = countryTo;
        if (alsoRecomputeVat) {
            for (auto itArticle = m_articlesShipped.begin();
                 itArticle != m_articlesShipped.end();
                 ++itArticle) {
                QString sku = itArticle.value()->getSku();
                double vatRate = VatRateManager::instance()->vatRate(
                            countryTo, m_dateTime.date(), sku);
                itArticle.value()->recomputeVat(vatRate);
            }
            double vatRate = VatRateManager::instance()->vatRateDefault(
                                        countryTo, m_dateTime.date());
            m_shipping.recomputeVat(vatRate);
            if (recomputeOrderShipping) {
                orderShipping.recomputeVat(vatRate);
                m_order->setShipping(orderShipping);
            }
            double newVat = getTotalPriceTaxes(false);
            if (qAbs(m_vatForRoundCorrection - newVat) > 0.029) {
                m_vatForRoundCorrection = newVat;
            }
        }
    } else if (!SettingManager::instance()->countriesUE(yearOrder)->contains(countryFrom)
               && SettingManager::instance()->countriesUE(yearOrder)->contains(countryTo)
               && !m_order->getShippedBySeller()) {
        /// For instance amazon.com USA shipped to France or germany in DDP
        // TODO we should go there also before IOSS
        m_regimeVat = Shipment::VAT_REGIME_NONE;
        m_countryVat = countryTo;
        m_countrySaleDeclaration = countryFrom;
        /*
        ManagerAccountsSales::instance()->getAccounts(
                    m_regimeVat, m_countryVat,
                    ManagerAccountsSales::SALE_PRODUCTS,
                    "0.00");
                    //*/
        if (alsoRecomputeVat) {
            for (auto itArticle = m_articlesShipped.begin();
                 itArticle != m_articlesShipped.end();
                 ++itArticle) {
                itArticle.value()->recomputeVat(0.);
            }
            m_vatForRoundCorrection = 0.;
            m_shipping.recomputeVat(0.);
            if (recomputeOrderShipping) {
                orderShipping.recomputeVat(0.);
                m_order->setShipping(orderShipping);
            }
        }

        //} else if (!isAfterOss && SettingManager::instance()->countriesUE(year)->contains(countryTo)) {
    } else {
        m_regimeVat = Shipment::VAT_REGIME_NORMAL;
        if (isAfterOss || countriesFromContain(countryTo)) {
            m_countryVat = countryTo;
            m_countrySaleDeclaration = countryTo;
        } else {
            m_countryVat = countryFrom;
            m_countrySaleDeclaration = countryFrom;
        }
        Q_ASSERT(SettingManager::instance()->countriesUE(yearShipment)->contains(countryTo));
        /// Pay vat in countryFrom with NORMAL regime and regularisation needed if not registred
        // if (!VatNumbersModel::instance()->containsCountry(countryFrom)) {
        if (alsoRecomputeVat) {
            for (auto itArticle = m_articlesShipped.begin();
                 itArticle != m_articlesShipped.end();
                 ++itArticle) {
                QString sku = itArticle.value()->getSku();
                double vatRateFrom = VatRateManager::instance()->vatRate(
                            m_countryVat, m_dateTime.date(), sku);
                itArticle.value()->recomputeVat(vatRateFrom);
            }
            double vatRate = VatRateManager::instance()->vatRateDefault(
                        m_countryVat, m_dateTime.date());
            m_shipping.recomputeVat(vatRate);
            if (recomputeOrderShipping) {
                orderShipping.recomputeVat(vatRate);
                m_order->setShipping(orderShipping);
            }
            double newVat = getTotalPriceTaxes(false);
            if (qAbs(m_vatForRoundCorrection - newVat) > 0.029) {
                m_vatForRoundCorrection = newVat;
            }
        }
    }
    if (isBusinessCustomer && isVatNumberValid && alsoRecomputeVat
            && !(countryFrom == "UK" && countryTo == "GB")
            && !wasVatChared
            && (countryFrom != countryTo
                || countryFrom == "ES"|| countryFrom == "IT")) { /// Don't know why amazon do this
                //|| countryTo != ManagerCompanyVatParams::instance()->countryCodeCompany())) { /// Auto-liquidation of VAT
        for (auto itArticle = m_articlesShipped.begin();
             itArticle != m_articlesShipped.end();
             ++itArticle) {
            itArticle.value()->recomputeVat(0.);
        }
        m_shipping.recomputeVat(0.);
        if (recomputeOrderShipping) {
            orderShipping.recomputeVat(0.);
            m_order->setShipping(orderShipping);
        }
        m_vatForRoundCorrection = 0.;
    }
    if (isMarketplaceResponsibleVat()) {
        for (auto itArticle = m_articlesShipped.begin();
             itArticle != m_articlesShipped.end();
             ++itArticle) {
            itArticle.value()->setTotalPriceTaxed(itArticle.value()->getTotalPriceUntaxed());
            itArticle.value()->setTotalPriceTaxes(0.);
            auto shipping = itArticle.value()->getShipping();
            shipping.setTotalPriceTaxed(shipping.totalPriceUntaxed());
            shipping.setTotalTaxes(0.);
            itArticle.value()->setShipping(shipping);
        }
        m_shipping.setTotalPriceTaxed(m_shipping.totalPriceUntaxed());
        m_shipping.setTotalTaxes(0.);
        if (recomputeOrderShipping) {
            orderShipping.setTotalPriceTaxed(orderShipping.totalPriceUntaxed());
            orderShipping.recomputeVat(0.);
            m_order->setShipping(orderShipping);
        }
        m_vatForRoundCorrection = 0.;
    }
    if (getCountryCodeVat() == "CH"
            && getRegimeVat().contains("xport")) {
        int TEMP=10;++TEMP;
    }
    double vatAfter = getTotalPriceTaxes();
    if (alsoRecomputeVat) {
        if (m_regimeVat == VAT_REGIME_OSS && qAbs(getTotalPriceTaxes()) < 0.01) {
            int TEMP=10;++TEMP;
        }
        bool recentOrderMaybeImcomplete = m_dateTime.date().daysTo(QDate::currentDate()) < 33;
        double totalPriceTaxes = getTotalPriceTaxes();
        auto totalPriceTaxesByVatRateConverted = getTotalPriceTaxesByVatRateConverted();
        //Q_ASSERT(qAbs(totalPriceTaxes - m_vatForRoundCorrection) < 0.05
                 //|| recentOrderMaybeImcomplete || !m_order->getChannel().contains("mazon")); /// It means we may not have vat transaction report and not know that the customer is business customer
        //Q_ASSERT(qAbs(totalPriceTaxes) < 0.01
                 //|| !totalPriceTaxesByVatRateConverted.contains("0.00")
                 //|| totalPriceTaxesByVatRateConverted.size() > 1);
        Q_ASSERT(!m_countryVat.isEmpty() || recentOrderMaybeImcomplete);
        if (qAbs(vatAfter - vatBefore) > 0.009) {
            qWarning() << "VAT was " << vatBefore << " and now is " << vatAfter;
        }
    }
}
/*
//==========================================================
QString Shipment::findCountryVat(const QHash<QString, double> &totalSaleDestCountry)
{
    QString countryFrom = m_addressFrom.countryCode();
    QString countryTo = m_order->getAddressTo().countryCode();
    if (!SettingManager::instance()->countriesUE()->contains(countryTo)) {
        m_countryVat = "";
    } else if (!SettingManager::instance()->countriesUE()->contains(countryFrom)) {
        m_countryVat = "IM";
    } else if (VatNumbersModel::instance()->containsCountry(countryTo)) {
        m_countryVat = countryTo;
    } else {
        double totalUntaxed = totalSaleDestCountry[countryTo] + getTotalPriceUntaxed();
        if (totalUntaxed < VatThresholdModel::instance()->threshold(countryTo)) {
            m_countryVat = countryFrom;
        } else {
            m_countryVat = countryTo;
        }
    }
    return m_countryVat;
}
//*/
//==========================================================
void Shipment::init(Order *order)
{
    m_order = order;
    for (auto article : m_articlesShipped) {
        article->init(this);
    }
    m_shipping.init(order);
}
//==========================================================
bool Shipment::isInitialized() const
{
    bool is = m_order != nullptr;
    return is;
}
//==========================================================
bool Shipment::isComplete() const
{
    bool complete = !m_dateTime.isNull()
            && !m_order->getDateTime().isNull()
            && !m_addressFrom.countryCode().isEmpty();
    return complete;
}
//==========================================================
bool Shipment::isCompletelyLoaded() const
{
    //if (m_order != nullptr && m_order->getId() == "114-9464722-9224229")
    //{
        //int TEMP=10;++TEMP;
    //}
    QHash<QString, QList<QSet<QString>>> possibleCombinations
            = [this]() -> QHash<QString, QList<QSet<QString>>> {
                    QHash<QString, QList<QSet<QString>>> valuesByChannel;
                        for (auto importer : AbstractOrderImporter::allImporters()) {
                            QList<QSet<QString>> values;
                            for (auto list : importer->reportForOrderComplete(getOrder())) {
                                 values << list.toSet();
                            }
                            valuesByChannel[importer->name()] = values;
                        }
                        OrderImporterServiceSales importer;
                        QList<QSet<QString>> values;
                        for (auto list : importer.reportForOrderComplete(getOrder())) {
                            values << list.toSet();
                        }
                        valuesByChannel[importer.name()] = values;
                        return valuesByChannel;
    }();

    //if (m_order->getId() == "111-7562514-2177011") { // 111-7562514-2177011 is not in orderd from FBM
    for (auto possible : possibleCombinations[channel()]) {
        bool isPossible = true;
        for (auto itPossible = possible.begin();
             itPossible != possible.end(); ++itPossible) {
            if (!m_reportsFrom.contains(*itPossible)) {
                isPossible = false;
                break;
            }
        }
        if (isPossible) {
            return true;
        }
    }
    return false;
}
//==========================================================
bool Shipment::isRefund() const
{
    return dynamic_cast<const Refund *>(this) != nullptr;
}
//==========================================================
bool Shipment::containsSku(const QString &sku) const
{
    for (auto article : m_articlesShipped) {
        if (article->getSku().contains(sku)) {
            return true;
        }
    }
    return false;
}
//==========================================================
QString Shipment::channel() const
{
    return m_order->getChannel();
}
//==========================================================
QString Shipment::subchannel() const
{
    return m_order->getSubchannel();
}
//==========================================================
bool Shipment::isMarketplaceResponsibleVat() const
{
    return m_vatCollectResponsible == VAT_RESPONSIBLE_MARKETPLACE;
}
//==========================================================
bool Shipment::operator<(const Shipment &other) const
{
    return m_dateTime < other.m_dateTime;
}
//==========================================================
bool Shipment::operator>(const Shipment &other) const
{
    return m_dateTime > other.m_dateTime;
}
//==========================================================
double Shipment::getTotalPriceTaxed() const
{
#ifdef AMAZON_RIGHT
    if (hasAmazonVatInformations())
    {
        return getTotalPriceTaxedAmazon();
    }
#endif
    double price = m_shipping.totalPriceTaxed();
    if (isFirstShipment()) {
        price += m_order->getShipping().totalPriceTaxed();
    }
    for (auto article : m_articlesShipped) {
        price += article->getTotalPriceTaxed();
        price += article->getShipping().totalPriceTaxed();
    }
    return price;
}
//==========================================================
double Shipment::getTotalPriceTaxes(bool round) const
{
#ifdef AMAZON_RIGHT
    if (hasAmazonVatInformations())
    {
        return getTotalPriceTaxesAmazon();
    }
#endif
    double price = m_vatForRoundCorrection;
    if (qAbs(price) < 0.001 || !round) {
        price = m_shipping.totalTaxes();
        if (isFirstShipment()) {
            price += m_order->getShipping().totalTaxes();
        }
        for (auto article : m_articlesShipped) {
            price += article->getTotalPriceTaxes();
            price += article->getShipping().totalTaxes();
        }
    }
    return price;
}
//==========================================================
double Shipment::getTotalPriceUntaxed() const
{
#ifdef AMAZON_RIGHT
    if (hasAmazonVatInformations())
    {
        return getTotalPriceUntaxedAmazon();
    }
#endif
    double value = getTotalPriceTaxed() - getTotalPriceTaxes();
    return value;
}
//==========================================================
double Shipment::getTotalPriceTaxedConverted() const
{
    if (m_currency != CustomerManager::instance()->getSelectedCustomerCurrency()
            && m_order != nullptr && m_order->getDateTime().isValid()) {
        return CurrencyRateManager::instance()->convert(
                    getTotalPriceTaxed(),
                    m_currency,
                    CustomerManager::instance()->getSelectedCustomerCurrency(),
                    m_order->getDateTime().date());
    }
    return getTotalPriceTaxed();
}
//==========================================================
double Shipment::getTotalPriceTaxesConverted() const
{
    if (m_currency != CustomerManager::instance()->getSelectedCustomerCurrency()
            && m_order != nullptr && m_order->getDateTime().isValid()) {
        return CurrencyRateManager::instance()->convert(
                    getTotalPriceTaxes(),
                    m_currency,
                    CustomerManager::instance()->getSelectedCustomerCurrency(),
                    m_order->getDateTime().date());
    }
    return getTotalPriceTaxes();
}
//==========================================================
double Shipment::getTotalPriceUntaxedConverted() const
{
    if (m_currency != CustomerManager::instance()->getSelectedCustomerCurrency()
            && m_order != nullptr && m_order->getDateTime().isValid()) {
        return CurrencyRateManager::instance()->convert(
                    getTotalPriceUntaxed(),
                    m_currency,
                    CustomerManager::instance()->getSelectedCustomerCurrency(),
                    m_order->getDateTime().date());
    }
    return getTotalPriceUntaxed();
}
//==========================================================
QMap<QString, QMap<QString, Price> > Shipment::getTotalPriceTaxesByVatRate() const
{
    QMap<QString, QMap<QString, Price>> taxesByRates;
    //taxesByRates[ManagerSaleTypes::SALE_PRODUCTS] = QMap<QString, Price>();
    for (auto article : m_articlesShipped) {
        QString saleType = article->getSaleType();
        if (!taxesByRates.contains(saleType)) {
            taxesByRates[saleType] = QMap<QString, Price>();
        }
        taxesByRates[saleType][article->vatRateString()] = Price();
    }
    if (!m_shipping.isNull()) {
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][m_shipping.vatRateString()] = Price();
    } else if (isFirstShipment() && !m_order->getShipping().isNull()) {
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][m_order->getShipping().vatRateString()] = Price();
    }
    for (auto article : m_articlesShipped) {
        auto vatRate = article->vatRateString();
        QString saleType = article->getSaleType();
        taxesByRates[saleType][vatRate].taxed += article->getTotalPriceTaxed();
        taxesByRates[saleType][vatRate].untaxed += article->getTotalPriceUntaxed();
        taxesByRates[saleType][vatRate].taxes += article->getTotalPriceTaxes();
        auto shipping = article->getShipping();
        if (!shipping.isNull()){
            taxesByRates[saleType][vatRate].taxed += shipping.totalPriceTaxed();
            taxesByRates[saleType][vatRate].untaxed += shipping.totalPriceUntaxed();
            taxesByRates[saleType][vatRate].taxes += shipping.totalTaxes();
        }
    }
    if (!m_shipping.isNull()) {
        auto vatRate = m_shipping.vatRateString();
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][vatRate].taxed
                += m_shipping.totalPriceTaxed();
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][vatRate].untaxed
                += m_shipping.totalPriceUntaxed();
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][vatRate].taxes
                += m_shipping.totalTaxes();
    }
    if (isFirstShipment() && !m_order->getShipping().isNull()) {
        Shipping orderShipping = m_order->getShipping();
        auto vatRate = orderShipping.vatRateString();
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][vatRate].taxed
                += orderShipping.totalPriceTaxed();
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][vatRate].untaxed
                += orderShipping.totalPriceUntaxed();
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][vatRate].taxes
                += orderShipping.totalTaxes();
    }
#ifdef AMAZON_RIGHT
    if (!m_countryVatAmazon.isEmpty())
    {
        if (taxesByRates.size() == 1 && taxesByRates.first().size() == 1)
        {
            QMap<QString, QMap<QString, Price>> amzTaxesByRates;
            amzTaxesByRates[ManagerSaleTypes::SALE_PRODUCTS][getAmazonVatRate()].taxes
                    = getTotalPriceTaxesAmazon();
            amzTaxesByRates[ManagerSaleTypes::SALE_PRODUCTS][getAmazonVatRate()].taxed
                    = getTotalPriceTaxedAmazon();
            return amzTaxesByRates;
        }
    }
#endif

    return taxesByRates;
}
//==========================================================
QMap<QString, QMap<QString, Price> > Shipment::getTotalPriceTaxesByVatRateConverted() const
{
    QMap<QString, QMap<QString, Price>> taxesByRates;
    taxesByRates[ManagerSaleTypes::SALE_PRODUCTS] = QMap<QString, Price>();
    for (auto article : m_articlesShipped) {
        QString saleType = article->getSaleType();
        if (!taxesByRates.contains(saleType)) {
            taxesByRates[saleType] = QMap<QString, Price>();
        }
        taxesByRates[saleType][article->vatRateString()] = Price();
    }
    if (!m_shipping.isNull()) {
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][m_shipping.vatRateString()] = Price();
    } else if (isFirstShipment() && !m_order->getShipping().isNull()) {
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS]
                [m_order->getShipping().vatRateString()] = Price();
    }
    for (auto article : m_articlesShipped) {
        auto vatRate = article->vatRateString();
        QString saleType = article->getSaleType();
        taxesByRates[saleType][vatRate].taxed += article->getTotalPriceTaxedConverted();
        taxesByRates[saleType][vatRate].untaxed += article->getTotalPriceUntaxedConverted();
        taxesByRates[saleType][vatRate].taxes += article->getTotalPriceTaxesConverted();
        auto shipping = article->getShipping();
        if (!shipping.isNull()){
            taxesByRates[saleType][vatRate].taxed += shipping.totalPriceTaxedConverted();
            taxesByRates[saleType][vatRate].untaxed += shipping.totalPriceUntaxedConverted();
            taxesByRates[saleType][vatRate].taxes += shipping.totalTaxesConverted();
        }
    }
    if (!m_shipping.isNull()) {
        auto vatRate = m_shipping.vatRateString();
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][vatRate].taxed
                += m_shipping.totalPriceTaxedConverted();
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][vatRate].untaxed
                += m_shipping.totalPriceUntaxedConverted();
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][vatRate].taxes
                += m_shipping.totalTaxesConverted();
    }
    if (isFirstShipment() && !m_order->getShipping().isNull()) {
        Shipping orderShipping = m_order->getShipping();
        auto vatRate = orderShipping.vatRateString();
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][vatRate].taxed
                += orderShipping.totalPriceTaxedConverted();
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][vatRate].untaxed
                += orderShipping.totalPriceUntaxedConverted();
        taxesByRates[ManagerSaleTypes::SALE_PRODUCTS][vatRate].taxes
                += orderShipping.totalTaxesConverted();
    }
    #ifdef AMAZON_RIGHT
    if (!m_countryVatAmazon.isEmpty())
    {
        if (taxesByRates.size() == 1 && taxesByRates.first().size() == 1)
        {
            QMap<QString, QMap<QString, Price>> amzTaxesByRates;
            amzTaxesByRates[ManagerSaleTypes::SALE_PRODUCTS][getAmazonVatRate()].taxes
                    = getTotalPriceTaxesAmazonConverted();
            amzTaxesByRates[ManagerSaleTypes::SALE_PRODUCTS][getAmazonVatRate()].taxed
                    = getTotalPriceTaxedAmazonConverted();
            return amzTaxesByRates;
        }
    }
#endif
    return taxesByRates;
}

double Shipment::getTotalPriceTaxedAmazon() const
{
    return m_totalPriceTaxedAmazon;
}

double Shipment::getTotalPriceTaxedAmazonConverted() const
{
    return CurrencyRateManager::instance()->convert(
                getTotalPriceTaxedAmazon(),
                m_currency,
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                getOrder()->getDateTime().date());
}

double Shipment::getTotalPriceUntaxedAmazon() const
{
    return m_totalPriceTaxedAmazon - m_totalPriceTaxesAmazon;
}

double Shipment::getTotalPriceUntaxedAmazonConverted() const
{
    return CurrencyRateManager::instance()->convert(
                getTotalPriceUntaxedAmazon(),
                m_currency,
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                getOrder()->getDateTime().date());
}

double Shipment::getTotalPriceTaxesAmazon() const
{
    return m_totalPriceTaxesAmazon;
}

double Shipment::getTotalPriceTaxesAmazonConverted() const
{
    return CurrencyRateManager::instance()->convert(
                getTotalPriceTaxesAmazon(),
                m_currency,
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                getOrder()->getDateTime().date());
}
//==========================================================
/*
QMap<QString, double> Shipment::getArticleChargedFees() const
{
    QMap<QString, double> fees;
    for (auto article : m_articlesShipped) {
        auto articleChargedFees = article->getChargedFees();
        for (auto itFees = articleChargedFees.begin();
             itFees != articleChargedFees.end();
             ++itFees) {
            if (fees.contains(itFees.key())) {
                fees[itFees.key()] += itFees.value();
            } else {
                fees[itFees.key()] = itFees.value();
            }
        }
    }
    return fees;
}
//*/
//==========================================================
Shipping Shipment::getShipping() const
{
    return m_shipping;
}
//==========================================================
void Shipment::setShipping(const Shipping &value)
{
    m_shipping = value;
    m_shipping.init(m_order);
}
//==========================================================
bool Shipment::isFirstShipment() const
{
    return !isRefund() && this == m_order->getShipmentFirst();
}
//==========================================================
QString Shipment::getId() const
{
    return m_id;
}
//==========================================================
double Shipment::getVatForRoundCorrection() const
{
    return m_vatForRoundCorrection;
}
//==========================================================
void Shipment::setVatForRoundCorrection(double vatForRoundCorrection)
{
    m_vatForRoundCorrection = vatForRoundCorrection;
}
//==========================================================
void Shipment::computeVatForRoundCorrection()
{
    m_vatForRoundCorrection = getTotalPriceTaxes();
}
//==========================================================
QString Shipment::invoicePrefix(int year) const
{
    QString prefix;
    if (!channel().isEmpty()) {
        prefix += SettingInvoices::instance()->invoicePrefix(channel());
    }
    if (isRefund()) {
        prefix = invoicePrefixRefundKeyword() + "-" + prefix;
    }
    if (isService()) {
        prefix += QObject::tr("Service");
    }
    if (!subchannel().isEmpty()) {
        prefix += "-" + subchannel();
    }
    prefix += "-" + m_regimeVat;
    prefix += "-" + QString::number(year);
    prefix += "-";
    return prefix;
}
//==========================================================
QString Shipment::invoicePrefixRefundKeyword()
{
    return QObject::tr("REMB", "remboursement / refund");

}
//==========================================================
int Shipment::getTotalQuantity() const
{
    int quantity = 0;
    for (auto article : m_articlesShipped) {
        quantity += article->getUnits();
    }
    return quantity;
}
//==========================================================
const QMap<QString, QHash<QString, Fees>>
&Shipment::getChargedFeesBySKU() const
{
    return m_chargedFeesBySKU;
}
//==========================================================
void Shipment::addChargedFee(
        const QString &lineId, const QString &feeName, const QString &sku, double amount)
{
    /*
    if (!m_chargedFeesBySKU.contains(sku)) {
        m_chargedFeesBySKU[sku] = QHash<QString, Fees>();
    }
    if (!m_chargedFeesBySKU[sku].contains(feeName)) {
        m_chargedFeesBySKU[sku][feeName] = Fees();
    }
    //*/
    if (!m_chargedFeesBySKU[sku][feeName].ids.contains(lineId)) {
        m_chargedFeesBySKU[sku][feeName].amountTotal += amount;
        m_chargedFeesBySKU[sku][feeName].ids << lineId;
        m_chargedFeesBySKU[sku][feeName].amounts << amount;
    }
}
//==========================================================
QDateTime Shipment::getDateTime() const
{
    return m_dateTime;
}
//==========================================================
void Shipment::setDateTime(const QDateTime &dateTime)
{
    m_dateTime = dateTime;
}
//==========================================================
Address Shipment::getAddressFrom() const
{
    return m_addressFrom;
}
//==========================================================
void Shipment::setAddressFrom(const Address &addressFrom)
{
    if (!m_addressFrom.isEmpty()) {
        QString customerId = CustomerManager::instance()->getSelectedCustomerId();
        if (!customerId.isEmpty()) {
            --m_countriesFromByCustomerId[customerId][m_addressFrom.countryCode()];
        }
    }
    m_addressFrom = addressFrom;
    _recordAddressFromCountry();
}
//==========================================================
int Shipment::getNumberOfTheYear() const
{
    return m_numberOfTheYear;
}
//==========================================================
void Shipment::setNumberOfTheYear(int numberOfTheYear)
{
    m_numberOfTheYear = numberOfTheYear;
}
//==========================================================
const QString &Shipment::getRegimeVat() const
{
    return m_regimeVat;
}
//==========================================================
QString Shipment::getCountryCodeVat() const
{
    return m_countryVat;
}
//==========================================================
QString Shipment::getCountryNameVat() const
{
    return CountryManager::instance()->countryName(m_countryVat);
}
//==========================================================
QString Shipment::getPaymentId() const
{
    return m_paymentId;
}
//==========================================================
void Shipment::setPaymentId(const QString &paymentId)
{
    m_paymentId = paymentId;
}
//==========================================================
QHash<QString, QSharedPointer<ArticleSold> > Shipment::getArticlesShipped() const
{
    return m_articlesShipped;
}
//==========================================================
QSharedPointer<ArticleSold> Shipment::getArticleShipped(const QString &sku) const
{
    for (auto article : m_articlesShipped) {
        if (article->getSku() == sku) {
            return article;
        }
    }
    return QSharedPointer<ArticleSold>();
}
//==========================================================
int Shipment::getArticleCount() const
{
    return m_articlesShipped.size();
}
//==========================================================
int Shipment::getUnitCounts() const
{
    int count = 0;
    for (auto itArticle = m_articlesShipped.begin();
         itArticle != m_articlesShipped.end(); ++itArticle) {
        count += (*itArticle)->getUnits();
    }
    return count;
}
//==========================================================
QHash<QString, QSharedPointer<ArticleSold> > Shipment::articlesShipped() const
{
    return m_articlesShipped;
}
//==========================================================
void Shipment::setArticlesShipped(
        const QHash<QString, QSharedPointer<ArticleSold> > &articlesShipped)
{
    m_articlesShipped = articlesShipped;
    for (auto article : m_articlesShipped) {
        article->init(this);
    }
}
//==========================================================
/*
void Shipment::addArticleShipped(QSharedPointer<ArticleSold> article)
{
    m_articlesShipped[article->getShipmentItemId()] = article;
    article->init(this);
}
//*/
//==========================================================
void Shipment::addArticleShipped(const QString &id, QSharedPointer<ArticleSold> article)
{
    if (m_articlesShipped.contains(id)) {
        Q_ASSERT(false); //TODO
    } else {
        article->init(this);
        m_articlesShipped[id] = article;
        m_vatForRoundCorrection += article->getTotalPriceTaxes();
    }
}
//==========================================================
const Order *Shipment::getOrder() const
{
    return m_order;
}
//==========================================================
QString Shipment::orderId() const
{
    return m_order == nullptr ? "" : m_order->getId();
}
//==========================================================
bool Shipment::isVatRegimeComputed() const
{
    return !m_regimeVat.isEmpty();
}
//==========================================================
bool Shipment::isService() const
{
    for (auto itArt = m_articlesShipped.begin();
         itArt != m_articlesShipped.end(); ++itArt) {
        if ((*itArt)->getSaleType() == ManagerSaleTypes::SALE_SERVICES) {
            return true;
        }
    }
    return false;
}
//==========================================================
//==========================================================
Fees::Fees()
{
    amountTotal = 0.;
}
//==========================================================
//==========================================================
Price::Price()
{
    taxed = 0;
    taxes = 0;
    untaxed = 0;
}
//==========================================================
//==========================================================
