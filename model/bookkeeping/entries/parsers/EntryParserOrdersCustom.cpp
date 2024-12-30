#include "../common/countries/CountryManager.h"

#include "model/CustomerManager.h"
#include "model/orderimporters/Shipment.h"
#include "model/bookkeeping/ManagerAccountsSales.h"
#include "model/orderimporters/Order.h"
#include "model/orderimporters/Refund.h"
#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/bookkeeping/invoices/InvoiceGenerator.h"

#include "EntryParserOrdersCustom.h"

//----------------------------------------------------------
EntryParserOrdersCustom::EntryParserOrdersCustom()
    : EntryParserOrders()
{
}
//----------------------------------------------------------
void EntryParserOrdersCustom::clearTransactions()
{
    m_shipmentAndRefunds.clear();
}
//----------------------------------------------------------
void EntryParserOrdersCustom::recordTransactions(
        const Shipment *shipmentOrRefund)
{
    /// In fact this class generates for all simple orders, not just custom so it doesn't have the right name
    if (!shipmentOrRefund->getOrder()->isFromMarketplace()) {
        QDateTime dateTime = shipmentOrRefund->getDateTime();
        int year = dateTime.date().year();
        if(!m_shipmentAndRefunds.contains(year)) {
            m_shipmentAndRefunds[year] = QMap<QDateTime, const Shipment *>();
        }
        m_shipmentAndRefunds[year].insert(dateTime, shipmentOrRefund);
    }
}
//----------------------------------------------------------
AccountingEntries EntryParserOrdersCustom::entries(int year) const
{
    AccountingEntries allEntries;
    allEntries[year] = QMap<QString, QMap<QString, QMultiMap<QString, QSharedPointer<AccountingEntrySet>>>>();
    QString journal = ManagerEntryTables::instance()->journalName(
                this->name());
    allEntries[year][journal] = QMap<QString, QMultiMap<QString, QSharedPointer<AccountingEntrySet>>>();
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
        /*
        QString countryCodeFrom = orderOrRefund->getAddressFrom().countryCode();
        if (orderOrRefund->isRefund()) {
            Refund *refund = static_cast<Refund *>(orderOrRefund);
            refund->getOrder()->getAdd
            static_cast<Refund *>(orderOrRefund)->
        }
        //*/
        if (shipmentOrRefund->getId() == "SCTGRATIV-refund") {
            int TEMP=10;++TEMP;
        }
        QString label = shipmentOrRefund->getId()
                +  " " + shipmentOrRefund->getRegimeVat()
                +  " " + countryVat
                +  " (" + shipmentOrRefund->getAddressFrom().countryCode()
                +  " => " + shipmentOrRefund->getOrder()->getAddressTo().countryCode()
                + ")";
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
        //TODO from settings
        QString accountClient;
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
        entryDebit.setAccount(accountClient);
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

        //entryDebitUnkown.setAccount();
        /*
        double totalTaxedConv
                = orderOrRefund->getTotalPriceTaxedConverted();
        QString totalTaxedConvStr
                = QString::number(totalTaxedConv, 'f', 2);
                //*/
        double totalTaxed
                = shipmentOrRefund->getTotalPriceTaxed();
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
                entryCredit.setAccount(account.saleAccount);
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
                    entryCreditVat.setAccount(account.vatAccount);
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
            entrySet->roundCreditDebit();
            if (entrySet->id().contains("SCTGRATIV-refund")) {
                QString saleType = itSaleType.key();
                int TEMP=10;++TEMP;
            }
            addEntryStatic(allEntries, entrySet);
        }
    }
    return allEntries;
}
//----------------------------------------------------------
AbstractEntryParser::Type EntryParserOrdersCustom::typeOfEntries() const
{
    return Sale;
}
//----------------------------------------------------------
QString EntryParserOrdersCustom::name() const
{
    return QObject::tr("Ventes simples");
}
//----------------------------------------------------------
QString EntryParserOrdersCustom::journal() const
{
    return QObject::tr("VT NN AMAZON", "As sales not amazon");
}
//----------------------------------------------------------
