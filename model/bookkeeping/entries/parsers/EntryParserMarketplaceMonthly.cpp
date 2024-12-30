#include "EntryParserMarketplaceMonthly.h"

#include "../common/countries/CountryManager.h"

#include "model/orderimporters/Shipment.h"
#include "model/orderimporters/OrderImporterAmazonUE.h"
#include "model/orderimporters/OrderImporterAmazon.h"
#include "model/bookkeeping/ManagerAccountsSales.h"
#include "model/bookkeeping/ManagerAccountsSalesRares.h"
#include "model/bookkeeping/ManagerAccountsAmazon.h"
#include "model/bookkeeping/ManagerSaleTypes.h"
#include "model/bookkeeping/entries/AccountingEntry.h"
#include "model/bookkeeping/entries/AccountingEntrySet.h"
#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/reports/ReportMonthlyAmazon.h"

//----------------------------------------------------------
EntryParserMarketplaceMonthly::EntryParserMarketplaceMonthly()
    : EntryParserOrders()
{
}
//----------------------------------------------------------
EntryParserMarketplaceMonthly::~EntryParserMarketplaceMonthly()
{
}
//----------------------------------------------------------
AbstractEntryParser::Type EntryParserMarketplaceMonthly::typeOfEntries() const
{
    return Sale;
}
//----------------------------------------------------------
void EntryParserMarketplaceMonthly::recordTransactions(
        const Shipment *shipmentOrRefund)
{
    if (acceptShipmentOrRefund()(shipmentOrRefund)) {
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
        QString subChannel = shipmentOrRefund->subchannel();
        auto accounts = ManagerAccountsAmazon::instance()->amazonAccount(subChannel);
        QString accountSalesUnkown = accounts.salesUnknown; // TODO remove this level
        if (!m_shipments[year][month].contains(accountSalesUnkown)) {
            m_shipments[year][month][accountSalesUnkown]
                    = QMap<QString, QMap<QString, QMap<QString, QMultiMap<QString, PriceInfos>>>>();
        }
        QString regime = shipmentOrRefund->getRegimeVat();
        Q_ASSERT(!regime.isEmpty());
        if (!m_shipments[year][month][accountSalesUnkown].contains(regime)) {
            m_shipments[year][month][accountSalesUnkown][regime]
                    = QMap<QString, QMap<QString, QMultiMap<QString, PriceInfos>>>();
        }
        QString country = shipmentOrRefund->getCountryNameVat();
        if (country == "" && regime == Shipment::VAT_REGIME_NORMAL_EXPORT) {
            int TEMP=10;++TEMP;
        }
        auto pricesConverted
                = shipmentOrRefund->getTotalPriceTaxesByVatRateConverted();
        auto vatRates = pricesConverted.keys();
        for (auto itSaleType=pricesConverted.begin();
             itSaleType!=pricesConverted.end(); ++ itSaleType) {
            for (auto it = itSaleType.value().begin();
                 it != itSaleType.value().end(); ++it) {
                PriceInfos infos;
                infos.untaxed = it.value().untaxed;
                infos.shipment = shipmentOrRefund;
                infos.taxes = it.value().taxes;
                if (regime == Shipment::VAT_REGIME_NONE && it.key() == "0.00") {
                    QString countryToUse;
                    if (CountryManager::instance()->countriesCodeUE(year)->contains(
                                shipmentOrRefund->getCountryCodeVat())) {
                        countryToUse = CountryManager::instance()->EU;
                    }
                    if (!m_shipments[year][month][accountSalesUnkown][regime].contains(countryToUse)) {
                        m_shipments[year][month][accountSalesUnkown][regime][countryToUse]
                                = QMap<QString, QMultiMap<QString, PriceInfos>>();
                    }
                    if (!m_shipments[year][month][accountSalesUnkown][regime][countryToUse].contains(itSaleType.key())) {
                        m_shipments[year][month][accountSalesUnkown][regime][countryToUse][itSaleType.key()]
                                = QMultiMap<QString, PriceInfos>();
                    }
                    m_shipments[year][month][accountSalesUnkown][regime][countryToUse][itSaleType.key()]
                            .insert(it.key(), infos);
                } else {
                    if (regime == Shipment::VAT_REGIME_NORMAL_EXPORT && it.key() == "0.00") {
                        country = shipmentOrRefund->getCountrySaleDeclarationName();
                    }
                    if (!m_shipments[year][month][accountSalesUnkown][regime].contains(country)) {
                        m_shipments[year][month][accountSalesUnkown][regime][country]
                                = QMap<QString, QMultiMap<QString, PriceInfos>>();
                    }
                    if (!m_shipments[year][month][accountSalesUnkown][regime][country].contains(itSaleType.key())) {
                        m_shipments[year][month][accountSalesUnkown][regime][country][itSaleType.key()]
                                = QMultiMap<QString, PriceInfos>();
                    }
                    m_shipments[year][month][accountSalesUnkown][regime][country][itSaleType.key()]
                            .insert(it.key(), infos);
                }
            }
        }
    }
}
//----------------------------------------------------------
void EntryParserMarketplaceMonthly::clearTransactions()
{
    m_shipments.clear();
}
//----------------------------------------------------------
AccountingEntries EntryParserMarketplaceMonthly::entries(int year) const
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
        //QList<QSharedPointer<AccountingEntrySet>> entrySetsOfMonth;

        QSharedPointer<AccountingEntrySet> entrySet(
                    new AccountingEntrySet(AccountingEntrySet::Sale)); //AccountingEntrySet::NoConnection));
        QString entrySetId = journal + "-" + QString::number(year) + "-" + monthStr;
        entrySet->setId(entrySetId);
        QString entrySetLabel = entrySetId;
        for (auto itSalesUnknownAccount = itMonth.value().begin();
             itSalesUnknownAccount != itMonth.value().end();
             ++itSalesUnknownAccount) {
            //QString salesUnknownAccount = itSalesUnknownAccount.key(); //TODO remove this level
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
                        pricesByMonth[regime][countryVatName] = QMap<QString, QMap<QString, TablePriceTotal>>();
                    }
                    for (auto itSaleType = itCountry.value().begin();
                         itSaleType != itCountry.value().end(); ++itSaleType) {
                        auto vatRates = itSaleType.value().uniqueKeys();
                        for (auto vatRateIt = vatRates.begin(); vatRateIt != vatRates.end(); ++vatRateIt) {
                            auto vatRate = *vatRateIt;
                            QString label
                                    = QObject::tr("Ventes ")
                                    + regime + " "
                                    + countryVatName + " "
                                    + vatRate;
                            QString countryVatCode = CountryManager::instance()->countryCode(
                                        countryVatName);
                            auto accounts = ManagerAccountsSales::instance()->getAccounts(
                                        regime,
                                        countryVatCode,
                                        itSaleType.key(), //SUPER
                                        vatRate);
                            if (itSaleType.key() != ManagerSaleTypes::SALE_PRODUCTS) {
                                label += " (" + accounts.titleBase + ")";
                            }
                            //SUPER
                            QString labelReport = label;
                            labelReport += " (" + accounts.saleAccount;
                            if (!accounts.vatAccount.isEmpty()) {
                                labelReport += " / " + accounts.vatAccount;
                            }
                            labelReport += ")";
                            //SUPER
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
                                            itValue->shipment, itValue->untaxed, itValue->taxes);
                            }

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
                                QString id = journal + "-" + countryVatCode + "-" + QString::number(year) + "-" + monthStr + "-" + QString::number(numEntrySet);
                                //entrySet->setId(id);
                                //entrySet->setAmountOrig(totalTaxed);
                                //entrySetsOfMonth << entrySet;
                                /*
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
                                                  ManagerAccountsSales::SALE_PRODUCTS, vatRate);
                            entrySet->addEntry(entryDebitUnkown);
                            //*/
                                /*
                                auto accounts = ManagerAccountsSales::instance()
                                        ->getAccounts(regime, countryVatCode,
                                                      itSaleType.key(), vatRate);
                                                      //*/
                                //SUPER

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
                                    entrySet->addEntry(entryCreditVat);
                                }
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

        auto amountsForBalance
                = balanceAmounts(year, month);
        auto allBalanceAccounts = ManagerAccountsAmazon::instance()->allBalanceAccounts();
        auto allSupplierAccounts = ManagerAccountsAmazon::instance()->allSupplierAccounts();
        auto allCustomerAccounts = ManagerAccountsAmazon::instance()->allCustomerAccounts();
        double amountSaleTotal = 0; //TODO from accounts or max ?
        QString titleDetail = QObject::tr("Détails du compte") + " ";
        for (auto itAccount = amountsForBalance.begin();
             itAccount != amountsForBalance.end(); ++itAccount) {
            QString accountName = itAccount.key();
            if (allBalanceAccounts.contains(accountName)) {
                QString labelBalanceDebit = QObject::tr("Retenu de garantie");
                QString labelBalanceCredit = QObject::tr("Retenu de garantie (remboursement)");
                double amountTotalDebit = 0;
                double amountTotalCredit = 0;
                // TODO don't gather debit / credit if balance
                reportGenerator.addTitle(titleDetail + accountName);
                if (accountName.endsWith("467008")) {
                    int TEMP=10;++TEMP;
                }
                reportGenerator.addTableNonSale();
                for (auto itVal = itAccount.value().begin();
                     itVal != itAccount.value().end(); ++itVal) {
                    auto info = itVal.value();
                    double amount = info.amount;
                    if (amount > 0) {
                        amountTotalCredit += amount;
                    } else {
                        amountTotalDebit += amount;
                    }
                    reportGenerator.addTableNonSaleRow(
                                info.reportFileName,
                                info.reportId,
                                info.title,
                                info.orderId,
                                info.date,
                                info.row,
                                info.amount);
                }
                double amountTotal = amountTotalDebit + amountTotalCredit;
                reportGenerator.addTableNonSaleTotal(amountTotal);
                amountSaleTotal = qMax(qAbs(amountTotalCredit), amountSaleTotal);
                amountSaleTotal = qMax(qAbs(amountTotalDebit), amountSaleTotal);
                if (qAbs(amountTotalCredit) > 0.001) {
                    AccountingEntry entryDebitUnkownCredit;
                    entryDebitUnkownCredit.setJournal(journal);
                    entryDebitUnkownCredit.setLabel(labelBalanceCredit);
                    if (amountTotalCredit > 0.) {
                        entryDebitUnkownCredit.setDebitOrig(amountTotalCredit);
                    } else {
                        entryDebitUnkownCredit.setCreditOrig(-amountTotalCredit);
                    }
                    entryDebitUnkownCredit.setDate(endOfMonth);
                    entryDebitUnkownCredit.setAccount(accountName);
                    entrySet->addEntry(entryDebitUnkownCredit);
                }
                if (qAbs(amountTotalDebit) > 0.001) {
                    AccountingEntry entryDebitUnkownDebit;
                    entryDebitUnkownDebit.setJournal(journal);
                    entryDebitUnkownDebit.setLabel(labelBalanceDebit);
                    if (amountTotalDebit > 0.) {
                        entryDebitUnkownDebit.setDebitOrig(amountTotalDebit);
                    } else {
                        entryDebitUnkownDebit.setCreditOrig(-amountTotalDebit);
                    }
                    entryDebitUnkownDebit.setDate(endOfMonth);
                    entryDebitUnkownDebit.setAccount(accountName);
                    entrySet->addEntry(entryDebitUnkownDebit);
                }
            } else {
                double amountTotal = 0;
                // TODO don't gather debit / credit if balance
                reportGenerator.addTitle(titleDetail + accountName);
                reportGenerator.addTableNonSale();
                //int nDetails = itAccount.value().size(); //If 1 line I could remove it
                for (auto itVal = itAccount.value().begin();
                     itVal != itAccount.value().end(); ++itVal) {
                    auto info = itVal.value();
                    amountTotal += info.amount;
                    reportGenerator.addTableNonSaleRow(
                                info.reportFileName,
                                info.reportId,
                                info.title,
                                info.orderId,
                                info.date,
                                info.row,
                                info.amount);
                }
                reportGenerator.addTableNonSaleTotal(amountTotal);
                amountSaleTotal = qMax(qAbs(amountTotal), amountSaleTotal);
                AccountingEntry entryDebitUnkown;
                entryDebitUnkown.setJournal(journal);
                QString label = QObject::tr("Ventes à encaisser TTC") + " " + nameSupplierCustomer();
                /*
                if (allSupplierAccounts.contains(accountName)) {
                    label = QObject::tr("Client") + " " + label;
                } else if (allCustomerAccounts.contains(accountName)) {
                    label = QObject::tr("Fournisseur") + " " + label;
                }
                //*/
                entryDebitUnkown.setLabel(label);
                if (amountTotal > 0.) {
                    entryDebitUnkown.setDebitOrig(amountTotal);
                } else {
                    entryDebitUnkown.setCreditOrig(-amountTotal);
                }
                entryDebitUnkown.setDate(endOfMonth);
                entryDebitUnkown.setAccount(accountName);
                entrySet->addEntry(entryDebitUnkown);
            }
        }
        entrySet->setAmountOrig(amountSaleTotal);
        entrySet->roundCreditDebit();
        addEntryStatic(allEntries, entrySet);


        //TODO use more complete rapport generator with details on fees if any
        reportGenerator.endHtml();
        entrySet->setHtmlDocument(reportGenerator.html());
        //for (auto entrySet = entrySetsOfMonth.begin(); entrySet != entrySetsOfMonth.end(); ++entrySet) {
            //(*entrySet)->setHtmlDocument(reportGenerator.html());
        //}
    }
    return allEntries;
}
//----------------------------------------------------------
