#include "../common/countries/CountryManager.h"

#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/bookkeeping/ManagerAccountsStockDeported.h"
#include "model/orderimporters/ModelStockDeported.h"
#include "model/orderimporters/VatOrdersModel.h"
#include "model/orderimporters/OrderMapping.h"
#include "model/orderimporters/OrderManager.h"
#include "model/reports/ReportStockDeported.h"
#include "model/vat/VatRateManager.h"

#include "EntryParserStockDeported.h"

//----------------------------------------------------------
EntryParserStockDeported::EntryParserStockDeported()
    : AbstractEntryParser()
{
}
//----------------------------------------------------------
AccountingEntries EntryParserStockDeported::entries(
        int year) const
{
    AccountingEntries allEntries;
    QString journal = ManagerEntryTables::instance()->journalName(
                this->name());
    Q_ASSERT(!journal.isEmpty());
    QString currency = CustomerManager::instance()->getSelectedCustomerCurrency();

      // month      countryCode     sku           units amount
    QHash<int, QHash<QString, QHash<QString, QPair<int, double>>>> inventoryLeavingFrance7SkuUnitsAmount;
    QHash<int, QHash<QString, QHash<QString, QPair<int, double>>>> inventoryGoingInFrance6SkuUnitsAmount;

      // month      countryCode
    QHash<int, QHash<QString, double>> inventoryLeavingFrance7;
    QHash<int, QHash<QString, double>> inventoryGoingInFrance6;
    QHash<int, double> totalInventoryLeavingFrance7;
    QHash<int, double> totalInventoryGoingInFrance6;
    QString countryCodeCompany = CustomerManager::instance()->getSelectedCustomerCountryCode();
    QString countryNameCompany = CountryManager::instance()->countryName(countryCodeCompany);
    auto orderManager = VatOrdersModel::instance()->orderManager();
    for (auto itChannel = orderManager->getOrdersByChannel().begin();
         itChannel != orderManager->getOrdersByChannel().end();
         ++itChannel) {
        for (auto itDate = itChannel.value().inventoryDeported[year].begin();
             itDate != itChannel.value().inventoryDeported[year].end();
             ++itDate) {
            int month = itDate.key().month();
            for (auto itCountryCodeFrom = itDate.value().begin();
                 itCountryCodeFrom != itDate.value().end();
                 ++itCountryCodeFrom) {
                auto countryCodeFrom = itCountryCodeFrom.key();
                for (auto itCountryCodeTo = itCountryCodeFrom.value().begin();
                     itCountryCodeTo != itCountryCodeFrom.value().end();
                     ++itCountryCodeTo) {
                    auto countryCodeTo = itCountryCodeTo.key();
                    for (auto itSku = itCountryCodeTo.value().begin();
                         itSku != itCountryCodeTo.value().end();
                         ++itSku) {
                        auto sku = itSku.key();
                        auto units = itSku.value();
                        double unitPrice
                                = ModelStockDeported::instance()->inventoryValue(sku);
                        double inventoryValue = unitPrice * units;
                        if (countryCodeTo == countryCodeCompany) {
                            /// Stock deported to France which means imported from UE => Compte 6
                            if (!inventoryGoingInFrance6[month].contains(countryCodeFrom)) {
                                inventoryGoingInFrance6[month][countryCodeFrom] = 0.;
                            }
                            inventoryGoingInFrance6[month][countryCodeFrom] += inventoryValue;
                            if (!totalInventoryGoingInFrance6.contains(month)) {
                                totalInventoryGoingInFrance6[month] = 0.;
                            }
                            totalInventoryGoingInFrance6[month] += inventoryValue;
                            if (!inventoryGoingInFrance6SkuUnitsAmount[month][countryCodeFrom].contains(sku)) {
                                inventoryGoingInFrance6SkuUnitsAmount[month][countryCodeFrom][sku] = {0, unitPrice};
                            }
                            inventoryGoingInFrance6SkuUnitsAmount[month][countryCodeFrom][sku].first += units;
                            // TODO check that it is country of company like Customer->getCountryCompany
                        } else if (countryCodeFrom == countryCodeCompany) {
                            /// Stock deported from France To UE which means exported to UE => Compte 7
                            if (!inventoryLeavingFrance7[month].contains(countryCodeTo)) {
                                inventoryLeavingFrance7[month][countryCodeTo] = 0.;
                            }
                            inventoryLeavingFrance7[month][countryCodeTo] += inventoryValue;
                            if (!totalInventoryLeavingFrance7.contains(month)) {
                                totalInventoryLeavingFrance7[month] = 0.;
                            }
                            totalInventoryLeavingFrance7[month] += inventoryValue;
                            if (!inventoryLeavingFrance7SkuUnitsAmount[month][countryCodeTo].contains(sku)) {
                                inventoryLeavingFrance7SkuUnitsAmount[month][countryCodeTo][sku] = {0, unitPrice};
                            }
                            inventoryLeavingFrance7SkuUnitsAmount[month][countryCodeTo][sku].first += units;
                        }
                    }
                }
            }
        }
    }
    QString account4 = ManagerAccountsStockDeported::instance()
            ->getAccount4();
    QString account4vatDeductible = ManagerAccountsStockDeported::instance()
            ->getAccount4VatDeductible();
    QString account4vatToPay = ManagerAccountsStockDeported::instance()
            ->getAccount4VatToPay();
    /*
    for (auto itMonth = inventoryLeavingFrance7.begin();
         itMonth != inventoryLeavingFrance7.end(); ++itMonth) {
        QSharedPointer<AccountingEntrySet> entrySet(
                    new AccountingEntrySet(
                        AccountingEntrySet::Sale));
        entrySet->setCurrencyOrig(currency);
        QDate date(year, itMonth.key(), 1);
        date = date.addMonths(1).addDays(-1);
        QString id(date.toString("yyyy-MM-dd"));
        id += "-stock-deported-leaving";
        entrySet->setId(id);
        QString labelBase(QObject::tr("Stock déporté vers UE (Vente intracom)"));
        double total = totalInventoryLeavingFrance7[itMonth.key()];
        AccountingEntry entryDebit;
        entryDebit.setJournal(journal);
        entryDebit.setDate(date);
        entryDebit.setCurrency(currency);
        entryDebit.setLabel(labelBase);
        entryDebit.setAccount(account4);
        entryDebit.setDebitOrig(total);
        entrySet->addEntry(entryDebit);
        ReportStockDeported reportGenerator;
        QString reportTitle(labelBase);
        reportTitle += " - ";
        reportTitle += date.toString(QObject::tr("dd/MM/yyyy"));
        reportGenerator.addTitle(reportTitle);
        for (auto itCountryCode = itMonth.value().begin();
             itCountryCode != itMonth.value().end(); ++itCountryCode) {
            double amount = itCountryCode.value();
            if (amount > 0.005) {
                auto countryCode = itCountryCode.key();
                QString countryName = CountryManager::instance()
                        ->countryName(countryCode);
                reportGenerator.addTitleH2(countryName);
                reportGenerator.addTable();
                AccountingEntry entryCredit;
                entryCredit.setJournal(journal);
                entryCredit.setDate(date);
                entryCredit.setCurrency(currency);
                QString label(labelBase);
                label += " - ";
                label += CountryManager::instance()->countryName(countryCode);
                entryCredit.setLabel(label);
                QString account = ManagerAccountsStockDeported::instance()
                        ->getAccountExportedToUe(countryCode);
                entryCredit.setAccount(account);
                entryCredit.setCreditOrig(amount);
                entrySet->addEntry(entryCredit);

                auto details = inventoryLeavingFrance7SkuUnitsAmount
                        [itMonth.key()].constFind(itCountryCode.key());
                for (auto itSku = details->begin();
                     itSku != details->end(); ++itSku) {
                    reportGenerator.addTableRow(
                                itSku.key(), itSku.value().first, itSku.value().second);
                }
            }
        }
        reportGenerator.endHtml();
        entrySet->setHtmlDocument(reportGenerator.html());
        entrySet->roundCreditDebit();
        addEntryStatic(allEntries, entrySet);
    }
    //*/

    for (auto itMonth = inventoryGoingInFrance6.begin();
         itMonth != inventoryGoingInFrance6.end(); ++itMonth) {
        QSharedPointer<AccountingEntrySet> entrySet(
                    new AccountingEntrySet(
                        AccountingEntrySet::Sale));
        entrySet->setCurrencyOrig(currency);
        QDate date(year, itMonth.key(), 1);
        date = date.addMonths(1).addDays(-1);
        QString id(date.toString("yyyy-MM-dd"));
        id += "-stock-deported-coming";
        entrySet->setId(id);
        QString labelBase(QObject::tr("Stock déporté depuis l'UE"));
        QString labelBasePurchase6 = labelBase;
        labelBasePurchase6 += " ";
        QString labelBaseSale7 = labelBasePurchase6;
        labelBasePurchase6 += QObject::tr("(Acquisition intracom)");
        labelBaseSale7 += QObject::tr("(Vente intracom)");
        double total = totalInventoryGoingInFrance6[itMonth.key()];
        AccountingEntry entryCredit4;
        entryCredit4.setJournal(journal);
        entryCredit4.setDate(date);
        entryCredit4.setCurrency(currency);
        entryCredit4.setLabel(labelBasePurchase6);
        AccountingEntry entryCreditVatDue = entryCredit4;
        AccountingEntry entryDebitVatDeductible = entryCredit4;
        entryCredit4.setAccount(account4);
        AccountingEntry entryDebit4 = entryCredit4;
        entryCredit4.setCreditOrig(total);
        entryDebit4.setLabel(labelBaseSale7);
        entryDebit4.setDebitOrig(total);
        QList<AccountingEntry> entriesCreditSales;

        double vatRateDefault =
                VatRateManager::instance()->vatRateDefault(
                    countryCodeCompany, date);
        double vat = total * vatRateDefault; // TODO handle different vat rate for different SKU
        entryCreditVatDue.setAccount(account4vatToPay);
        entryCreditVatDue.setCreditOrig(vat);

        entryDebitVatDeductible.setAccount(account4vatDeductible);
        entryDebitVatDeductible.setDebitOrig(vat);

        ReportStockDeported reportGenerator;
        QString reportTitle(labelBase);
        reportTitle += " - ";
        reportTitle += date.toString(QObject::tr("dd/MM/yyyy"));
        reportGenerator.addTitle(reportTitle);
        for (auto itCountryCode = itMonth.value().begin();
             itCountryCode != itMonth.value().end(); ++itCountryCode) {
            double amount = itCountryCode.value();
            if (amount > 0.005) {
                auto countryCode = itCountryCode.key();
                QString countryName = CountryManager::instance()
                        ->countryName(countryCode);
                reportGenerator.addTitleH2(countryName);
                reportGenerator.addTable();
                AccountingEntry entryDebitPurchase;
                entryDebitPurchase.setJournal(journal);
                entryDebitPurchase.setDate(date);
                entryDebitPurchase.setCurrency(currency);
                QString labelPurchase(labelBasePurchase6);
                QString endLabel(" - ");
                endLabel += CountryManager::instance()->countryName(countryCode);
                endLabel += " > ";
                endLabel += countryNameCompany;
                labelPurchase += endLabel;
                QString accountPurchase = ManagerAccountsStockDeported::instance()
                        ->getAccountImportedFromUe(countryCode);
                AccountingEntry entryCreditSale = entryDebitPurchase;
                entryDebitPurchase.setLabel(labelPurchase);
                entryDebitPurchase.setAccount(accountPurchase);
                entryDebitPurchase.setDebitOrig(amount);
                entrySet->addEntry(entryDebitPurchase);
                QString accountSale = ManagerAccountsStockDeported::instance()
                        ->getAccountExportedToUe(countryCode);
                QString labelSale(labelBaseSale7);
                labelSale += endLabel;
                entryCreditSale.setLabel(labelSale);
                entryCreditSale.setAccount(accountSale);
                entryCreditSale.setCreditOrig(amount);
                entriesCreditSales << entryCreditSale;
      // month      countryCode     sku           units amount
                auto details = inventoryGoingInFrance6SkuUnitsAmount
                        [itMonth.key()].constFind(itCountryCode.key());
                for (auto itSku = details->begin();
                     itSku != details->end(); ++itSku) {
                    reportGenerator.addTableRow(
                                itSku.key(), itSku.value().first, itSku.value().second);
                }
                reportGenerator.addTableTotal(amount);
            }
        }
        entrySet->addEntry(entryDebitVatDeductible);
        entrySet->addEntry(entryCreditVatDue);
        entrySet->addEntry(entryCredit4);
        entrySet->addEntry(entryDebit4);
        for (auto itEntryCreditSale = entriesCreditSales.begin();
             itEntryCreditSale != entriesCreditSales.end(); ++itEntryCreditSale) {
            entrySet->addEntry(*itEntryCreditSale);
        }
        reportGenerator.endHtml();
        entrySet->setHtmlDocument(reportGenerator.html());
        entrySet->roundCreditDebit();
        addEntryStatic(allEntries, entrySet);
    }
    return allEntries;
}
//----------------------------------------------------------
AbstractEntryParser::Type EntryParserStockDeported::typeOfEntries() const
{
    return VariousOperations;
}
//----------------------------------------------------------
QString EntryParserStockDeported::name() const
{
    return QObject::tr("Stock déporté");
}
//----------------------------------------------------------
QString EntryParserStockDeported::journal() const
{
    return QObject::tr("OD", "Various operation bookkeeping journal name");
}
//----------------------------------------------------------
