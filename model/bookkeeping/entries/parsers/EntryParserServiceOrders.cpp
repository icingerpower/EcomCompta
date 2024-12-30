#include "../common/countries/CountryManager.h"

#include "model/orderimporters/Order.h"
#include "model/orderimporters/Refund.h"
#include "model/orderimporters/Shipment.h"
#include "model/bookkeeping/ManagerAccountsSales.h"
#include "model/bookkeeping/ManagerSaleTypes.h"
#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/bookkeeping/invoices/InvoiceGenerator.h"
#include "model/orderimporters/ServiceAccounts.h"

#include "EntryParserServiceOrders.h"

//----------------------------------------------------------
const QString EntryParserServiceOrders::JOURNAL
= QObject::tr("VTPRESTATIONS", "Sales service");;
//----------------------------------------------------------
EntryParserServiceOrders::EntryParserServiceOrders()
    : EntryParserOrders()
{
}
//----------------------------------------------------------
AccountingEntries EntryParserServiceOrders::entries(
        int _year) const
{
    AccountingEntries allEntries;
    QString journal = ManagerEntryTables::instance()->journalName(
                this->name());
    Q_ASSERT(!journal.isEmpty());
    QList<int> years{_year-1, _year};
    for (const auto &year : years)
    {
        for (auto itShipment = m_shipmentAndRefunds[year].begin();
             itShipment != m_shipmentAndRefunds[year].end();
             ++itShipment) {
            QSharedPointer<AccountingEntrySet> entrySet(
                        new AccountingEntrySet(
                            AccountingEntrySet::Sale)); //AccountingEntrySet::NoConnection));
            auto shipmentOrRefund = itShipment.value();
            entrySet->setId(shipmentOrRefund->getId());
            entrySet->setAmountOrig(-shipmentOrRefund->getTotalPriceTaxed());
            entrySet->setCurrencyOrig(shipmentOrRefund->getCurrency());
            auto date = shipmentOrRefund->getDateTime().date();
            QString countryVat = shipmentOrRefund->getCountryCodeVat();
            QString vatRegime = shipmentOrRefund->getRegimeVat();
            QString label = QObject::tr("service");
            label += " ";
            label += shipmentOrRefund->getId();
            label += " ";
            label += shipmentOrRefund->getRegimeVat();
            label += " ";
            label += countryVat;
            if (shipmentOrRefund->getCurrency()
                    != CustomerManager::instance()->getSelectedCustomerCurrency()) {
                label = QString::number(
                            shipmentOrRefund->getTotalPriceTaxed(), 'f', 2)
                        + " " + shipmentOrRefund->getCurrency()
                        + " " + label;
            }
            AccountingEntry entryDebit;
            entryDebit.setJournal(journal);
            entryDebit.setLabel(label);
            entryDebit.setDate(date);
            entryDebit.setCurrency(shipmentOrRefund->getCurrency());
            QString accountClient;
            auto addressTo = shipmentOrRefund->getOrder()->getAddressTo();
            ServiceAccounts serviceAccounts(addressTo.internalId()); ///Zill erase the settings accounts
            // TODO get from settings
            if (shipmentOrRefund->getCountryCodeVat() == "FR") { //TODO also FR from settings
                accountClient = "CCLIENTFR";
            } else if (CountryManager::instance()->countriesCodeUE()->contains(
                           countryVat)) {
                accountClient = "CCLIENTUE";
            } else if (vatRegime == Shipment::VAT_REGIME_NORMAL_EXPORT) {
                accountClient = "CCLIENTEXP";
            } else {
                accountClient = "CCLIENTDOM";
            }
            entryDebit.setAccount(serviceAccounts.accountClient(accountClient));
            QSharedPointer<InvoiceGenerator> invoiceGenerator(nullptr);
            QString orderId = shipmentOrRefund->getOrder()->getId();
            if (shipmentOrRefund->isRefund()) {
                invoiceGenerator = InvoiceGenerator::createGeneratorRefund(
                            static_cast<const Refund *>(shipmentOrRefund));
            } else {
                invoiceGenerator = InvoiceGenerator::createGeneratorInvoice(
                            static_cast<const Refund *>(shipmentOrRefund));
            }
            entrySet->setHtmlDocument(
                        invoiceGenerator->html(),
                        shipmentOrRefund->getInvoiceName() + "-" + orderId);
            double totalTaxed = shipmentOrRefund->getTotalPriceTaxed();
            QString totalTaxedStr
                    = QString::number(totalTaxed, 'f', 2);
            if (shipmentOrRefund->isRefund()) {
                entryDebit.setCreditOrig(totalTaxedStr);
            } else {
                entryDebit.setDebitOrig(totalTaxedStr);
            }
            auto totalsByVatRates = shipmentOrRefund
                    ->getTotalPriceTaxesByVatRate();
            for (auto itSaleType = totalsByVatRates.begin();
                 itSaleType != totalsByVatRates.end(); ++itSaleType) {
                for (auto it = itSaleType.value().begin();
                     it != itSaleType.value().end(); ++it) {
                    auto vatRateStr = it.key();
                    double vatRate = vatRateStr.toDouble();
                    auto charged = it.value();
                    auto account = ManagerAccountsSales::instance()->getAccounts(
                                shipmentOrRefund->getRegimeVat(),
                                shipmentOrRefund->getCountryCodeVat(),
                                itSaleType.key(),
                                vatRateStr);
                    AccountingEntry entryCredit = entryDebit;
                    entryCredit.setDebitOrig("");
                    entryCredit.setCreditOrig("");
                    entryCredit.setAccount(
                                serviceAccounts.accountSale(
                                    account.saleAccount));
                    if (shipmentOrRefund->isRefund()) {
                        entryCredit.setDebitOrig(
                                    QString::number(-charged.untaxed, 'f', 2));
                    } else {
                        entryCredit.setCreditOrig(
                                    QString::number(charged.untaxed, 'f', 2));
                    }
                    entrySet->addEntry(entryDebit);
                    entrySet->addEntry(entryCredit);
                    if (vatRate > 0.001) {
                        AccountingEntry entryCreditVat = entryDebit;
                        entryCreditVat.setDebitOrig("");
                        entryCreditVat.setCreditOrig("");
                        entryCreditVat.setAccount(
                                    serviceAccounts.accountVatToDeclare(
                                        account.vatAccount));
                        if (shipmentOrRefund->isRefund()) {
                            entryCreditVat.setDebitOrig(
                                        QString::number(-charged.taxes, 'f', 2));
                        } else {
                            entryCreditVat.setCreditOrig(
                                        QString::number(charged.taxes, 'f', 2));
                        }
                        entrySet->addEntry(entryCreditVat);
                    }
                }
                Q_ASSERT(entrySet->entries().size() > 0);
                entrySet->roundCreditDebit();
                Q_ASSERT(entrySet->entries().size() > 0);
                Q_ASSERT(!entrySet->journal().isEmpty());
                Q_ASSERT(!entrySet->label().isEmpty());
                addEntryStatic(allEntries, entrySet);
            }
        }
    }
    return allEntries;
}
//----------------------------------------------------------
AbstractEntryParser::Type EntryParserServiceOrders::typeOfEntries() const
{
    return Sale;
}
//----------------------------------------------------------
QString EntryParserServiceOrders::name() const
{
    return QObject::tr("Services");
}
//----------------------------------------------------------
QString EntryParserServiceOrders::journal() const
{
    return JOURNAL;
}
//----------------------------------------------------------
void EntryParserServiceOrders::recordTransactions(
        const Shipment *shipmentOrRefund)
{
    if (shipmentOrRefund->getSaleTypes().contains(ManagerSaleTypes::SALE_SERVICES)) {
        QDateTime dateTime = shipmentOrRefund->getDateTime();
        int year = dateTime.date().year();
        m_shipmentAndRefunds[year].insert(dateTime, shipmentOrRefund);
    }
}
//----------------------------------------------------------
void EntryParserServiceOrders::clearTransactions()
{
    m_shipmentAndRefunds.clear();
}
//----------------------------------------------------------
