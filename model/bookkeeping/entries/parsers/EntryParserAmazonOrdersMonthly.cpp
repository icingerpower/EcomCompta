#include "../common/countries/CountryManager.h"

#include "EntryParserAmazonOrdersMonthly.h"
#include "model/orderimporters/Shipment.h"
#include "model/orderimporters/OrderImporterAmazonUE.h"
#include "model/orderimporters/OrderImporterAmazon.h"
#include "model/bookkeeping/ManagerAccountsSales.h"
#include "model/bookkeeping/ManagerAccountsAmazon.h"
#include "model/bookkeeping/entries/AccountingEntry.h"
#include "model/bookkeeping/entries/AccountingEntrySet.h"
#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/reports/ReportMonthlyAmazon.h"

QString EntryParserAmazonOrdersMonthly::JOURNAL
= QObject::tr("VT", "Vente / sale") + " AMAZON";
QString EntryParserAmazonOrdersMonthly::NAME
= QObject::tr("Commandes amazon (mensuel)");
//----------------------------------------------------------
EntryParserAmazonOrdersMonthly::EntryParserAmazonOrdersMonthly()
    : EntryParserOrders()
{
}
//----------------------------------------------------------
EntryParserAmazonOrdersMonthly::~EntryParserAmazonOrdersMonthly()
{
}
//----------------------------------------------------------
void EntryParserAmazonOrdersMonthly::recordTransactions(
        const Shipment *shipmentOrRefund)
{
    if (shipmentOrRefund->orderId() == "112-0768008-5776232") {
        int TEMP=10;++TEMP;
    }
    if (shipmentOrRefund->channel() == OrderImporterAmazonUE::NAME
            || shipmentOrRefund->channel() == OrderImporterAmazon::NAME) {
        auto dateTime = shipmentOrRefund->getDateTime();
        int year = dateTime.date().year();
        int month = dateTime.date().month();
        if (!m_shipments.contains(year)) {
            m_shipments[year] = QMap<int, QMap<QString, QMap<QString, QMap<QString, QMap<QString, QMultiMap<QString, PriceInfos>>>>>>();
        }
        if (!m_shipments[year].contains(month)) {
            m_shipments[year][month]
                    = QMap<QString, QMap<QString, QMap<QString, QMap<QString, QMultiMap<QString, PriceInfos>>>>>();
        }
        QString amazon = shipmentOrRefund->subchannel();
        auto amazonAccounts = ManagerAccountsAmazon::instance()->amazonAccount(amazon);
        QString accountSalesUnkown = amazonAccounts.salesUnknown;
        if (!m_shipments[year][month].contains(accountSalesUnkown)) {
            m_shipments[year][month][accountSalesUnkown]
                    = QMap<QString, QMap<QString, QMap<QString, QMultiMap<QString, PriceInfos>>>>();
        }
        QString regime = shipmentOrRefund->getRegimeVat();
        if (!m_shipments[year][month][accountSalesUnkown].contains(regime)) {
            m_shipments[year][month][accountSalesUnkown][regime]
                    = QMap<QString, QMap<QString, QMultiMap<QString, PriceInfos>>>();
        }
        QString country = shipmentOrRefund->getCountryNameVat();
        auto vatRates = shipmentOrRefund->getTotalPriceTaxesByVatRateConverted().keys();
        if (country == "Royaume-Uni de Grande-Bretagne et d'Irlande du Nord" && regime == Shipment::VAT_REGIME_NONE
            && vatRates.contains("0.20")) {
            int TEMP=10;++TEMP;
        }
        auto pricesConverted
                = shipmentOrRefund->getTotalPriceTaxesByVatRateConverted();
        for (auto itSaleType=pricesConverted.begin();
             itSaleType!=pricesConverted.end(); ++itSaleType) {
            for (auto it=itSaleType.value().begin();
                 it!=itSaleType.value().end(); ++it) {
                PriceInfos infos;
                infos.untaxed = it.value().untaxed;
                infos.shipment = shipmentOrRefund;
                infos.taxes = it.value().taxes;
                if (regime == Shipment::VAT_REGIME_NONE && it.key() == "0.00") {
                    QString countryToUse;
                    if (CountryManager::instance()->countriesCodeUE(year)->contains(
                                shipmentOrRefund->getCountryCodeVat())) {
                        countryToUse = CountryManager::instance()->EU;
                    //} else {
                        //countryToUse = country;
                    }
                    //Q_ASSERT(!countryToUse.isEmpty());
                    /*
                    if (!m_shipments[year][month][accountSalesUnkown][regime].contains(countryToUse)) { //TODO before it was "" why?
                        m_shipments[year][month][accountSalesUnkown][regime][countryToUse]
                                = QMap<QString, QMultiMap<QString, PriceInfos>>();
                    }
                    if (!m_shipments[year][month][accountSalesUnkown][regime][countryToUse].contains(itSaleType.key())) {
                        m_shipments[year][month][accountSalesUnkown][regime][countryToUse][itSaleType.key()]
                                = QMultiMap<QString, PriceInfos>();
                    }
                    //*/
                    if (regime == Shipment::VAT_REGIME_OSS
                            || regime == Shipment::VAT_REGIME_IOSS) {
                        Q_ASSERT(it.key().toDouble() > 0.01 && !countryToUse.isEmpty());
                    }
                    m_shipments[year][month][accountSalesUnkown][regime][countryToUse][itSaleType.key()]
                            .insert(it.key(), infos);
                } else {
                    if (regime == Shipment::VAT_REGIME_NORMAL_EXPORT && it.key() == "0.00") {
                        country = shipmentOrRefund->getCountrySaleDeclarationName();
                    }
                    /*
                    if (!m_shipments[year][month][accountSalesUnkown][regime].contains(country)) {
                        m_shipments[year][month][accountSalesUnkown][regime][country]
                                = QMap<QString, QMultiMap<QString, PriceInfos>>();
                    }
                    if (!m_shipments[year][month][accountSalesUnkown][regime][country].contains(itSaleType.key())) {
                        m_shipments[year][month][accountSalesUnkown][regime][country][itSaleType.key()]
                                = QMultiMap<QString, PriceInfos>();
                    }
                    //*/
                    if (regime == Shipment::VAT_REGIME_OSS
                            || regime == Shipment::VAT_REGIME_IOSS) {
                        if (qAbs(infos.untaxed) < 0.001) {
                            continue;
                        }
                        auto key = it.key();
                        Q_ASSERT(it.key().toDouble() > 0.01 && !country.isEmpty());
                    }
                    m_shipments[year][month][accountSalesUnkown][regime][country][itSaleType.key()]
                            .insert(it.key(), infos);
                }
            }
        }
    }
    /*
    if (m_shipments[year].contains(6)
            && m_shipments[year][6][accountSalesUnkown].contains(Shipment::VAT_REGIME_NORMAL_EXPORT)
            && m_shipments[year][6][accountSalesUnkown][Shipment::VAT_REGIME_NORMAL_EXPORT].contains("")){
        int TEMP=10;++TEMP;
    }
    //*/
}
//----------------------------------------------------------
void EntryParserAmazonOrdersMonthly::clearTransactions()
{
    m_shipments.clear();
}
//----------------------------------------------------------
AccountingEntries EntryParserAmazonOrdersMonthly::entries(int year) const
{
    AccountingEntries allEntries;
    allEntries[year] = QMap<QString, QMap<QString, QMultiMap<QString, QSharedPointer<AccountingEntrySet>>>>();
    QString journal = ManagerEntryTables::instance()->journalName(
                this->name());
    allEntries[year][journal] = QMap<QString, QMultiMap<QString, QSharedPointer<AccountingEntrySet>>>();

    //// regime       / country      / vatRate      / month
    QHash<QString, QMap<QString, QMap<QString, QMap<QString, TablePriceTotal>>>> pricesByMonth;
    QMap<QString, TablePriceTotal> pricesByMonthOssTotal;
    QMap<QString, TablePriceTotal> pricesByMonthIossTotal;
    /*
    auto begin = m_shipments[year].begin();
    auto end = m_shipments[year].end();
    auto begin2 = begin+1;
    auto beginKey = begin2.key();
    auto beginVal = begin2.value();
    //*/
    // TODO debut here for 2021
    for (auto itMonth = m_shipments[year].begin();
         itMonth != m_shipments[year].end();
         ++itMonth) {
        int month = itMonth.key();
        ReportMonthlyAmazon reportGenerator;
        int numEntrySet = 1;
        QString monthStr = QString::number(month).rightJustified(2, '0');
        if (!pricesByMonthOssTotal.contains(monthStr)) {
             pricesByMonthOssTotal[monthStr] = TablePriceTotal();
             pricesByMonthIossTotal[monthStr] = TablePriceTotal();
        }
        allEntries[year][journal][monthStr] = QMultiMap<QString, QSharedPointer<AccountingEntrySet>>();
        QDate endOfMonth = QDate(year, month, 1).addMonths(1).addDays(-1);
        QList<QSharedPointer<AccountingEntrySet>> entrySetsOfMonth;
        for (auto itSalesUnknownAccount = itMonth.value().begin();
             itSalesUnknownAccount != itMonth.value().end();
             ++itSalesUnknownAccount) {
            QString salesUnknownAccount = itSalesUnknownAccount.key();
            for (auto itRegime = itSalesUnknownAccount.value().begin();
                 itRegime != itSalesUnknownAccount.value().end();
                 ++itRegime) {
                QString regime = itRegime.key();
                if (!pricesByMonth.contains(regime)) { /// Monthly totals
                    pricesByMonth[regime] = QMap<QString, QMap<QString, QMap<QString, TablePriceTotal>>>();
                }
                for (auto itCountry = itRegime.value().begin();
                     itCountry != itRegime.value().end();
                     ++itCountry) {
                    QString countryVatName = itCountry.key();
                    if (!pricesByMonth[regime].contains(countryVatName)) { /// Monthly totals
                        pricesByMonth[regime][countryVatName]
                                = QMap<QString, QMap<QString, TablePriceTotal>>();
                    }
                    for (auto itSaleType = itCountry.value().begin();
                         itSaleType != itCountry.value().end(); ++itSaleType) {
                        auto vatRates = itSaleType.value().uniqueKeys();
                        for (auto vatRate : vatRates) {
                            QString label
                                    = QObject::tr("Ventes ")
                                    + regime + " "
                                    + countryVatName + " "
                                    + vatRate;
                            if (regime == Shipment::VAT_REGIME_NORMAL && vatRate == "0.00") {
                                label += " (auto-liquidation ventes intra-UE)";
                            }
                            QString countryVatCode = CountryManager::instance()->countryCode(
                                        countryVatName);
                            auto accounts = ManagerAccountsSales::instance()->getAccounts(
                                        regime,
                                        countryVatCode,
                                        itSaleType.key(),
                                        vatRate);
                            QString labelReport = label;
                            labelReport += " (" + accounts.saleAccount;
                            if (!accounts.vatAccount.isEmpty()) {
                                labelReport += " / " + accounts.vatAccount;
                            } //SUPER
                            labelReport += ")";
                            reportGenerator.addTitle(labelReport);
                            reportGenerator.addTable();
                            PriceInfos priceTotal;
                            priceTotal.taxes = 0.;
                            priceTotal.untaxed = 0.;
                            auto valuesToReverseOrder = itCountry.value()[itSaleType.key()].values(vatRate);
                            for (auto itValue = valuesToReverseOrder.rbegin();
                                 itValue != valuesToReverseOrder.rend(); ++itValue) {
                                priceTotal.taxes += itValue->taxes;
                                priceTotal.untaxed += itValue->untaxed;
                                reportGenerator.addTableRow(
                                            itValue->shipment, regime, accounts.saleAccount, accounts.vatAccount, itValue->untaxed, itValue->taxes);
                            }
                            /*
                        //for (auto value : itCountry.value()[ManagerAccountsSales::SALE_PRODUCTS].values(vatRate)) {
                        for (auto itValue = itCountry.value()[ManagerAccountsSales::SALE_PRODUCTS].begin();
                             itValue != itCountry.value()[ManagerAccountsSales::SALE_PRODUCTS].end();
                             ++itValue) {
                            if (itValue.key() == vatRate) {
                                priceTotal.taxes += itValue.value().taxes;
                                priceTotal.untaxed += itValue.value().untaxed;
                                reportGenerator.addTableRow(
                                            itValue.value().shipment, itValue.value().untaxed, itValue.value().taxes);
                            }
                        }
                        //*/

                            reportGenerator.addTableTotal(priceTotal.untaxed, priceTotal.taxes);

                            if (!pricesByMonth[regime][countryVatName].contains(vatRate)) {
                                pricesByMonth[regime][countryVatName][vatRate] = QMap<QString, TablePriceTotal>();
                            }
                            TablePriceTotal tablePriceTotal;
                            tablePriceTotal.taxes = priceTotal.taxes;
                            tablePriceTotal.untaxed = priceTotal.untaxed;
                            pricesByMonth[regime][countryVatName][vatRate][monthStr] = tablePriceTotal;
                            reportGenerator.addTableMonthlyTotal(month, pricesByMonth[regime][countryVatName][vatRate]);

                            if (regime == Shipment::VAT_REGIME_OSS) {
                                pricesByMonthOssTotal[monthStr].taxes += tablePriceTotal.taxes;
                                pricesByMonthOssTotal[monthStr].untaxed += tablePriceTotal.untaxed;
                            } else if (regime == Shipment::VAT_REGIME_IOSS) {
                                pricesByMonthIossTotal[monthStr].taxes += tablePriceTotal.taxes;
                                pricesByMonthIossTotal[monthStr].untaxed += tablePriceTotal.untaxed;
                            }

                            //QString pdfFilePath;
                            //reportGenerator.save(pdfFilePath);
                            double totalTaxed = priceTotal.taxes + priceTotal.untaxed;
                            if (qAbs(totalTaxed) > 0.009) { /// One order refunded can lead to 0
                                QSharedPointer<AccountingEntrySet> entrySet(
                                            new AccountingEntrySet(AccountingEntrySet::Sale)); //AccountingEntrySet::NoConnection));
                                QString id = "amazon-" + countryVatCode + "-" + QString::number(year) + "-" + monthStr + "-" + QString::number(numEntrySet);
                                entrySet->setId(id);
                                entrySet->setAmountOrig(totalTaxed);
                                entrySetsOfMonth << entrySet;
                                AccountingEntry entryDebitUnkown;
                                entryDebitUnkown.setJournal(journal);
                                entryDebitUnkown.setLabel(label);
                                if (totalTaxed > 0.) {
                                    entryDebitUnkown.setDebitOrig(totalTaxed);
                                } else {
                                    entryDebitUnkown.setCreditOrig(-totalTaxed);
                                }
                                entryDebitUnkown.setDate(endOfMonth);
                                entryDebitUnkown.setAccount(salesUnknownAccount);
                                auto accounts = ManagerAccountsSales::instance()
                                        ->getAccounts(regime, countryVatCode,
                                                      itSaleType.key(), vatRate); //SUPER
                                entrySet->addEntry(entryDebitUnkown);
                                if (regime != Shipment::VAT_REGIME_NONE && accounts.saleAccount == "707030") {
                                    int TEMP=10;++TEMP;
                                }

                                AccountingEntry entryCreditSales;
                                entryCreditSales.setLabel(label);
                                entryCreditSales.setJournal(journal);
                                if (priceTotal.untaxed > 0.) {
                                    entryCreditSales.setCreditOrig(priceTotal.untaxed);
                                } else {
                                    entryCreditSales.setDebitOrig(-priceTotal.untaxed);
                                }
                                entryCreditSales.setDate(endOfMonth);
                                entryCreditSales.setAccount(accounts.saleAccount);
                                entrySet->addEntry(entryCreditSales);

                                if (qAbs(priceTotal.taxes) > 0.005) {
                                    AccountingEntry entryCreditVat;
                                    entryCreditVat.setJournal(journal);
                                    entryCreditVat.setLabel(label);
                                    if (priceTotal.taxes > 0.) {
                                        entryCreditVat.setCreditOrig(priceTotal.taxes);
                                    } else {
                                        entryCreditVat.setDebitOrig(-priceTotal.taxes);
                                    }
                                    entryCreditVat.setDate(endOfMonth);
                                    entryCreditVat.setAccount(accounts.vatAccount);
                                    Q_ASSERT(!accounts.vatAccount.isEmpty());
                                    entrySet->addEntry(entryCreditVat);
                                }
                                entrySet->roundCreditDebit();
                                addEntryStatic(allEntries, entrySet);
                                //allEntries[year][journal][monthStr].insert(label, entrySet);
                            }
                        }
                    }
                }
            }
        }
        reportGenerator.addTitle(QObject::tr("Total OSS"));
        reportGenerator.addTableMonthlyTotal(12, pricesByMonthOssTotal);
        reportGenerator.addTitle(QObject::tr("Total IOSS"));
        reportGenerator.addTableMonthlyTotal(12, pricesByMonthIossTotal);
        reportGenerator.endHtml();
        const auto &csvData = reportGenerator.getCsvData();
        const QString &csvBaseName = journal + "-details-" + endOfMonth.toString("yyyy-MM");
        for (auto entrySet : entrySetsOfMonth) {
            entrySet->setHtmlDocument(reportGenerator.html());
            entrySet->setCsvData(csvData, csvBaseName);
        }
    }
    return allEntries;
}
//----------------------------------------------------------
AbstractEntryParser::Type EntryParserAmazonOrdersMonthly::typeOfEntries() const
{
    return Sale;
}
//----------------------------------------------------------
QString EntryParserAmazonOrdersMonthly::name() const
{
    return EntryParserAmazonOrdersMonthly::NAME;
}
//----------------------------------------------------------
QString EntryParserAmazonOrdersMonthly::journal() const
{
    return EntryParserAmazonOrdersMonthly::JOURNAL;
}
//----------------------------------------------------------
