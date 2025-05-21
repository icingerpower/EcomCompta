#include "../common/countries/CountryManager.h"

#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/bookkeeping/ManagerAccountsSales.h"
#include "model/bookkeeping/ManagerAccountsVatPayments.h"
#include "model/orderimporters/Shipment.h"
#include "model/orderimporters/Order.h"
#include "model/reports/ReportMonthlyAmazon.h"
#include "model/LogVat.h"

#include "EntryParserSaleVatOssIoss.h"

//----------------------------------------------------------
EntryParserSaleVatOssIoss::EntryParserSaleVatOssIoss()
    : EntryParserOrders()
{
}
//----------------------------------------------------------
AccountingEntries EntryParserSaleVatOssIoss::entries(
        int year) const
{
    AccountingEntries allEntries;
    QString journal = ManagerEntryTables::instance()->journalName(
                this->name());
    QString currency = CustomerManager::instance()->getSelectedCustomerCurrency();

    //    regimeVAT     dateReturn   countryCodeFrom countryCodeTo    saleType      vatRate
    QHash<QString, QHash<QDate, QHash<QString, QHash<QString, QHash<QString, QHash<QString, double>>>>>> regimeDateCountryTypesaleRateVat;
    QHash<QString, QHash<QDate, QHash<QString, QHash<QString, QHash<QString, QMultiHash<QString, const Shipment *>>>>>> regimeDateCountryTypesaleRateVatShipments;
    for (auto itShipment = m_shipmentAndRefunds[year].begin();
         itShipment != m_shipmentAndRefunds[year].end();
         ++itShipment) {
        auto date = itShipment.key();
        auto shipmentOrRefund = itShipment.value();
        QString countryCodeVat = shipmentOrRefund->getCountryCodeVat();
        QString countryCodeVatFrom;
        if (shipmentOrRefund->isRefund())
        {
            auto shipments = shipmentOrRefund->getOrder()->getShipments();
            for (auto shipment : shipments)
            {
                countryCodeVatFrom = shipment->countryCodeFrom();
                if (countryCodeVatFrom != countryCodeVat)
                {
                    break; // TODO Won't work for refund with 3 countries due to 3 shipments
                }
            }
        }
        else
        {
            countryCodeVatFrom = shipmentOrRefund->countryCodeFrom();
        }
        Q_ASSERT(!countryCodeVatFrom.isEmpty());
        QString regimeVat = shipmentOrRefund->getRegimeVat();
        Q_ASSERT(regimeVat != Shipment::VAT_REGIME_OSS
                 || countryCodeVat != countryCodeVatFrom);
        auto vatByRateConverted = shipmentOrRefund->getTotalPriceTaxesByVatRateConverted();
        bool isIoss = regimeVat == Shipment::VAT_REGIME_IOSS;
        QDate dateVatReturn;
        if (isIoss) {
            dateVatReturn = QDate(year, date.date().month(), 1)
                    .addMonths(1).addDays(-1);
        } else {
            int trimester = (date.date().month()-1) / 3;
            int lastMonthTrimester = (trimester + 1) * 3;
            dateVatReturn = QDate(year, lastMonthTrimester, 1)
                    .addMonths(1).addDays(-1);
            int month = date.date().month();
            if (month == 7 || month == 8 || month == 9) {
                Q_ASSERT(lastMonthTrimester == 9);
            } else {
                Q_ASSERT(lastMonthTrimester != 9);
            }
        }
        LogVat::instance()->recordAmount("OD-2", shipmentOrRefund);
        for (auto itSaleType = vatByRateConverted.begin();
             itSaleType != vatByRateConverted.end(); ++itSaleType) {
            auto saleType = itSaleType.key();
            for (auto itRateTaxe = itSaleType.value().begin();
                 itRateTaxe != itSaleType.value().end(); ++itRateTaxe) {
                auto vatRate = itRateTaxe.key();
                Q_ASSERT(regimeVat != Shipment::VAT_REGIME_OSS
                         || countryCodeVat != countryCodeVatFrom);
                if (!regimeDateCountryTypesaleRateVat[regimeVat][dateVatReturn][countryCodeVat][countryCodeVatFrom][saleType].contains(vatRate)) {
                    regimeDateCountryTypesaleRateVat[regimeVat][dateVatReturn][countryCodeVat][countryCodeVatFrom][saleType][vatRate] = 0.;
                }
                regimeDateCountryTypesaleRateVat[regimeVat][dateVatReturn][countryCodeVat][countryCodeVatFrom][saleType][vatRate]
                        += itRateTaxe.value().taxes;
                regimeDateCountryTypesaleRateVatShipments
                        [regimeVat][dateVatReturn][countryCodeVat][countryCodeVatFrom][saleType].insert(
                            vatRate, shipmentOrRefund);
            }
        }
    }
    LogVat::instance()->saveLog("OD-2", "OD-2.log", 2023, 7, 9);
    for (auto itRegimeVat = regimeDateCountryTypesaleRateVat.begin();
         itRegimeVat != regimeDateCountryTypesaleRateVat.end(); ++itRegimeVat) {
        for (auto itDate = itRegimeVat.value().begin();
             itDate != itRegimeVat.value().end(); ++itDate) {
            ReportMonthlyAmazon reportGenerator;
            QList<QStringList> csvData;
            auto date = itDate.key();
            QString dateString = date.toString(QObject::tr("MM/yyyy", "date format"));
            auto regimeVat = itRegimeVat.key();
            QString id = QString::number(year);
            id += "-";
            id += regimeVat;
            id += "-";
            id += dateString;
            QString labelBase(QObject::tr("TVA"));
            labelBase += " ";
            labelBase += regimeVat;
            labelBase += " ";
            labelBase += dateString;
            for (auto itCountryCode = itDate.value().begin();
                 itCountryCode != itDate.value().end(); ++itCountryCode) {
                double totalAmount = 0.;
                QList<QSharedPointer<AccountingEntrySet>> entrySetsOfDateReturns;
                QString countryName = CountryManager::instance()->countryName(
                    itCountryCode.key());
                QSharedPointer<AccountingEntrySet> entrySet(
                    new AccountingEntrySet(
                        AccountingEntrySet::Sale));
                entrySet->setCurrencyOrig(currency);
                entrySet->setId(id);
                for (auto itCountryCodeFrom = itCountryCode.value().begin();
                     itCountryCodeFrom != itCountryCode.value().end(); ++itCountryCodeFrom) {
                    const QString &countryCodeFrom = itCountryCodeFrom.key();
                    QString countryNameFrom = CountryManager::instance()->countryName(
                        countryCodeFrom);
                    Q_ASSERT(!countryCodeFrom.isEmpty());
                    for (auto itSaleType = itCountryCodeFrom.value().begin();
                         itSaleType != itCountryCodeFrom.value().end(); ++itSaleType) {
                        for (auto itVatRate = itSaleType.value().begin();
                             itVatRate != itSaleType.value().end(); ++itVatRate) {
                            double amount = itVatRate.value();
                            if (qAbs(amount) > 0.005) {
                                auto vatRate = itVatRate.key();
                                double vatRatePercent = vatRate.toDouble() * 100.;
                                if (regimeVat == Shipment::VAT_REGIME_OSS
                                    && itCountryCodeFrom.key() == itCountryCode.key())
                                {
                                    int TEMP=10;++TEMP;
                                }
                                auto shipmentRefunds = regimeDateCountryTypesaleRateVatShipments
                                                           [regimeVat][itDate.key()]
                                                           [itCountryCode.key()]
                                                           [itCountryCodeFrom.key()]
                                                               .constFind(itSaleType.key());
                                double totalUntaxed = 0.;
                                double totaltaxes = 0.;
                                auto account = ManagerAccountsSales::instance()
                                                   ->getAccounts(regimeVat,
                                                                 itCountryCode.key(),
                                                                 itSaleType.key(),
                                                                 vatRate);
                                for (auto itShipment = shipmentRefunds->begin();
                                     itShipment != shipmentRefunds->end(); ++itShipment) {
                                    auto shipmentOrRefund = itShipment.value();
                                    auto vatByRateConverted = shipmentOrRefund->getTotalPriceTaxesByVatRateConverted();
                                    auto price = vatByRateConverted[itSaleType.key()][itVatRate.key()];
                                    csvData << reportGenerator.addTableRow(
                                        shipmentOrRefund, regimeVat, account.saleAccount, account.vatAccount, price.untaxed, price.taxes);
                                    totalUntaxed += price.untaxed;
                                    totaltaxes += price.taxes;
                                }
                                QString label(labelBase);
                                label += " ";
                                label += dateString;
                                label += " ";
                                label += countryNameFrom;
                                label += " => ";
                                label += countryName;
                                label += " ";
                                label += QString::number(totalUntaxed, 'f', 2);
                                label += " ";
                                label += currency;
                                label += " ";
                                label += QString::number(vatRatePercent, 'f', 2);
                                label += "%";
                                reportGenerator.addTitle(label);
                                csvData << QStringList({label});
                                csvData << reportGenerator.addTable();
                                csvData << reportGenerator.addTableTotal(totalUntaxed, totaltaxes);
                                AccountingEntry entryDebit;
                                entryDebit.setJournal(journal);
                                entryDebit.setDate(date);
                                entryDebit.setCurrency(currency);
                                entryDebit.setLabel(label);
                                entryDebit.setAccount(account.vatAccount);
                                if (amount > 0) {
                                    entryDebit.setDebitOrig(amount);
                                } else {
                                    entryDebit.setCreditOrig(-amount);
                                }
                                entrySet->addEntry(entryDebit);
                                totalAmount += amount;
                                entrySetsOfDateReturns << entrySet;
                            }
                        }
                    }
                }
                AccountingEntry entryCredit;
                entryCredit.setJournal(journal);
                entryCredit.setDate(date);
                entryCredit.setCurrency(currency);
                if (regimeVat == Shipment::VAT_REGIME_OSS)
                {
                    entryCredit.setLabel(labelBase + " " + countryName);
                }
                else
                {
                    entryCredit.setLabel(labelBase);
                }
                QString accountVat = ManagerAccountsVatPayments::instance()
                                         ->getAccount(regimeVat);
                entryCredit.setAccount(accountVat);
                if (totalAmount > 0) {
                    entryCredit.setCreditOrig(totalAmount);
                } else {
                    entryCredit.setDebitOrig(-totalAmount);
                }
                entrySet->insertEntry(0, entryCredit);
                entrySet->setAmountOrig(totalAmount);
                entrySet->roundCreditDebit();
                addEntryStatic(allEntries, entrySet);

                QString entrySetName = journal + "-vat-" + itDate.key().toString("yyyy-MM");
                for (auto entrySet : entrySetsOfDateReturns) {
                    entrySet->setHtmlDocument(reportGenerator.html(), entrySetName);
                    entrySet->setCsvData(csvData, entrySetName);
                }
            }
        }
    }

    LogVat::instance()->saveLog("OD", "OD.log", 2023, 7, 9);
    return allEntries;
}
//----------------------------------------------------------
AbstractEntryParser::Type EntryParserSaleVatOssIoss::typeOfEntries() const
{
    return Sale;
}
//----------------------------------------------------------
QString EntryParserSaleVatOssIoss::name() const
{
    return QObject::tr("Divers TVA OSS et IOSS");
}
//----------------------------------------------------------
QString EntryParserSaleVatOssIoss::journal() const
{
    return QObject::tr("OD", "Various operation bookkeeping journal name");
}
//----------------------------------------------------------
void EntryParserSaleVatOssIoss::recordTransactions(
        const Shipment *shipmentOrRefund)
{
    //TODOCEDRIC
    // save in a file each order of country / regime with diffence
    auto vatRegime = shipmentOrRefund->getRegimeVat();
    if (vatRegime == Shipment::VAT_REGIME_OSS
            || vatRegime == Shipment::VAT_REGIME_IOSS) {
        QDateTime dateTime = shipmentOrRefund->getDateTime();
        int year = dateTime.date().year();
        m_shipmentAndRefunds[year].insert(dateTime, shipmentOrRefund);
        LogVat::instance()->recordAmount("OD", shipmentOrRefund);
    }
}
//----------------------------------------------------------
void EntryParserSaleVatOssIoss::clearTransactions()
{
    m_shipmentAndRefunds.clear();
}
//----------------------------------------------------------
