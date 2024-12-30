#include "../common/utils/CsvReader.h"

#include "model/orderimporters/ImportedFileReportManager.h"
#include "model/orderimporters/OrderImporterCdiscount.h"
#include "model/bookkeeping/ManagerAccountsAmazon.h"
#include "model/CustomerManager.h"
#include "model/reports/ReportMonthlyAmazon.h"
#include "EntryParserCdiscountMonthly.h"
#include "EntryParserMarketplaceMonthly.h"
#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"

#include "EntryParserCdiscountPayment.h"

//----------------------------------------------------------
QString EntryParserCdiscountPayment::NAME
= QObject::tr("Paiement CDiscount");
QString EntryParserCdiscountPayment::JOURNAL
= "VTCDISCOUNT";
//----------------------------------------------------------
EntryParserCdiscountPayment::EntryParserCdiscountPayment()
    : EntryParserOrders()
{
}
//----------------------------------------------------------
EntryParserCdiscountPayment::~EntryParserCdiscountPayment()
{
}
//----------------------------------------------------------
QString EntryParserCdiscountPayment::name() const
{
    return NAME;
}
//----------------------------------------------------------
QString EntryParserCdiscountPayment::journal() const
{
    return JOURNAL;
}
//----------------------------------------------------------
void EntryParserCdiscountPayment::clearTransactions()
{
}
//----------------------------------------------------------
void EntryParserCdiscountPayment::recordTransactions(
        const Shipment *shipmentOrRefund)
{

}
//----------------------------------------------------------
AccountingEntries EntryParserCdiscountPayment::entries(
        int year) const
{
    AccountingEntries allEntries;
    QStringList filePaths
            = ImportedFileReportManager::instance()->filePaths(
                OrderImporterCdiscount::NAME,
                OrderImporterCdiscount::REPORT_PAYMENTS,
                year,
                true);
    auto accounts
            = ManagerAccountsAmazon::instance()->amazonAccount(
                "cdiscount.fr"); //TODO settings
    QString journal =
                ManagerEntryTables::instance()->journalName(
                this->name());
    for (auto filePath : filePaths) {
        QMap<QString, QMultiMap<QString,
                EntryParserMarketplaceMonthly::BalanceAmount>> balanceAmounts;
        EntryParserMarketplaceMonthly::BalanceAmount amountClient;
        QString reportFileName = QFileInfo(filePath).fileName();
        int linesToSkip = 3;
        CsvReader reader(filePath, ";", "\"", true, "\r\n", linesToSkip);
        reader.readAll();
        const DataFromCsv *dataRode = reader.dataRode();
        QHash<QString, QString> orderIdToInvoiceId;
        int indPaymentInvoice = dataRode->header.pos("N° facture/avoir");
        int indOrderId = dataRode->header.pos("N° commande / Service");
        int indDateTime = dataRode->header.pos("Date opération comptable");
        int indDateTimeAnticipatedPayment
                = dataRode->header.pos("Date de mise en paiement anticipé");
        int indDatePayment = dataRode->header.pos({"Date\nvirement", "Date\r\nvirement"});
        int indSalesTotal = dataRode->header.pos("Vente TTC hors frais de port");
        int indSalesShippingTotal = dataRode->header.pos("Frais de port TTC");
        int indSalesRefund = dataRode->header.pos("Remboursement TTC");

        int indComProduit = dataRode->header.pos("Commission Produit");
        int indComFacilitePayment = dataRode->header.pos("Commission Facilités de paiement");
        int indComPayment4times = dataRode->header.pos("Commission Frais de paiement 4 fois");
        int indComAvoir = dataRode->header.pos("Avoir commission");
        int indTotalPaid = dataRode->header.pos("Total reçu");
        int idLine = linesToSkip + 2;
        QDate dateStartPayment;
        QDate datePayment;
        QString lastSkiped = dataRode->skipedLines.last().trimmed();
        lastSkiped.replace(";", "");
        double chargedAmount = cdiscountToDouble(lastSkiped);
        QString chargedAmountStr = QString::number(chargedAmount, 'f', 2);
        if (dataRode->lines.size() > 0) {
            datePayment = QDate::fromString(dataRode->lines[0][indDatePayment], "dd/MM/yyyy");
            dateStartPayment = datePayment;
        }
        double salesTotal = 0;
        double feesTotal = 0;
        double balanceDebitTotal = 0.;
        double balanceCreditTotal = 0.;
        for (auto elements : dataRode->lines) {
            QDate dateAccounting = QDate::fromString(elements[indDateTime], "dd/MM/yyyy");
            if (dateAccounting < dateStartPayment) {
                dateStartPayment = dateAccounting;
            }
            //if (date.year() == year && date.month() == month) {
            double fees = 0.;
            double sales = 0.;
            double balanceDebit = 0.;
            double balanceCredit = 0.;
            QString paymentInvoiceId = elements[indPaymentInvoice];
            QString serviceName = elements[indOrderId];
            QString orderId;
            if (paymentInvoiceId.isEmpty()) {
                paymentInvoiceId = orderIdToInvoiceId.value(serviceName, "");
            } else {
                orderIdToInvoiceId[serviceName] = paymentInvoiceId;
            }
            //QString invoice = elements[indCdiscountInvoice];
            //QString titleFees = invoice + " - " + serviceName;
            QString titleFees = serviceName;
            if (elements[indDateTimeAnticipatedPayment].isEmpty()) { /// Means special fees
                Q_ASSERT(!elements[indTotalPaid].contains("("));
                if (serviceName.contains("garantie")) {
                    double amount = cdiscountToDouble(elements[indTotalPaid]);
                    if (amount > 0.001 ) {
                        balanceCredit += amount;
                    } else if (amount < 0.001 ) {
                        balanceDebit += amount;
                    }
                    double balance = balanceDebit + balanceCredit; /// one is 0
                    QString titleBalance = QObject::tr("Retenue de garanti");
                    if (qAbs(balanceCreditTotal) > 0.001) {
                        titleBalance = QObject::tr("Retenue de garanti (remboursement)");
                    }
                    EntryParserMarketplaceMonthly::BalanceAmount amountBalance;
                    amountBalance.reportFileName = reportFileName;
                    amountBalance.reportId = paymentInvoiceId;
                    amountBalance.orderId = orderId;
                    amountBalance.row = QString::number(idLine);
                    amountBalance.date = dateAccounting;
                    amountBalance.title = titleBalance;
                    amountBalance.amount = -balance;
                    if (!balanceAmounts.contains(accounts.reserve)) {
                        balanceAmounts[accounts.reserve]
                                = QMultiMap<QString,
                                EntryParserMarketplaceMonthly::BalanceAmount>();
                    }
                    balanceAmounts[accounts.reserve].insert(titleBalance, amountBalance);
                } else {
                    fees = cdiscountToDouble(elements[indTotalPaid]);
                }
            } else {
                titleFees = QObject::tr("Commissions") + " " + serviceName;
                fees = cdiscountToDouble(elements[indComProduit])
                        + cdiscountToDouble(elements[indComPayment4times])
                        + cdiscountToDouble(elements[indComAvoir]);
                sales = cdiscountToDouble(elements[indSalesTotal])
                        + cdiscountToDouble(elements[indSalesShippingTotal])
                        + cdiscountToDouble(elements[indSalesRefund]);
                if (!elements[indComProduit].isEmpty()) {
                    fees += cdiscountToDouble(elements[indComFacilitePayment]);
                } else {
                    sales += cdiscountToDouble(elements[indComFacilitePayment]);
                }
                orderId = serviceName;
            }
            if (qAbs(sales) > 0.001) {
                EntryParserMarketplaceMonthly::BalanceAmount amountSales;
                amountSales.reportFileName = reportFileName;
                amountSales.reportId = paymentInvoiceId;
                amountSales.orderId = orderId;
                amountSales.row = QString::number(idLine);
                amountSales.date = dateAccounting;
                amountSales.title = QObject::tr("Vente ") + orderId;
                amountSales.amount = sales;
                if (!balanceAmounts.contains(accounts.salesUnknown)) {
                    balanceAmounts[accounts.salesUnknown]
                            = QMultiMap<QString,
                            EntryParserMarketplaceMonthly::BalanceAmount>();
                }
                balanceAmounts[accounts.salesUnknown].insert(
                            amountSales.title, amountSales);
            }
            if (qAbs(fees) > 0.001) {
                //amountClient.amount += fees;
                EntryParserMarketplaceMonthly::BalanceAmount amountFees;
                amountFees.reportFileName = reportFileName;
                amountFees.reportId = paymentInvoiceId;
                amountFees.orderId = orderId;
                amountFees.row = QString::number(idLine);
                amountFees.date = dateAccounting;
                amountFees.title = titleFees;
                amountFees.amount = -fees;
                if (!balanceAmounts.contains(accounts.supplier)) {
                    balanceAmounts[accounts.supplier]
                            = QMultiMap<QString,
                            EntryParserMarketplaceMonthly::BalanceAmount>();
                }
                balanceAmounts[accounts.supplier].insert(titleFees, amountFees);
                //if (!amounts[accounts.supplier].contains(titleFees)) {
                //amounts[accounts.supplier][titleFees] = 0.;
                //}
                //amounts[accounts.supplier][titleFees] -= fees;
                /*
            } else if (qAbs(balanceDebitTotal) > 0.001 || qAbs(balanceCreditTotal) > 0.001) {
                double balance = balanceDebitTotal + balanceCreditTotal; /// one is 0
                QString titleBalance = QObject::tr("Retenue de garanti");
                if (qAbs(balanceCreditTotal) > 0.001) {
                    titleBalance = QObject::tr("Retenue de garanti (remboursement)");
                }
                //amountClient.amount += balance;
                EntryParserMarketplaceMonthly::BalanceAmount amountBalance;
                amountBalance.reportFileName = reportFileName;
                amountBalance.reportId = paymentInvoiceId;
                amountBalance.orderId = orderId;
                amountBalance.row = QString::number(idLine);
                amountBalance.date = dateAccounting;
                amountBalance.title = titleBalance;
                amountBalance.amount = -balance;
                if (!balanceAmounts.contains(accounts.reserve)) {
                    balanceAmounts[accounts.reserve]
                            = QMultiMap<QString,
                            EntryParserMarketplaceMonthly::BalanceAmount>();
                }
                balanceAmounts[accounts.reserve].insert(titleBalance, amountBalance);

                /*
                    if (!amounts[accounts.reserve].contains(titleBalance)) {
                        amounts[accounts.reserve][titleBalance] = 0.;
                    }
                    amounts[accounts.reserve][titleBalance] -= balance;
                    //*/
            }
            //*/
            //}
            balanceCreditTotal += balanceCredit;
            balanceDebitTotal += balanceDebit;
            salesTotal += sales;
            feesTotal += fees;
            ++idLine;
        }
        //balanceAmounts[accounts.client].insert(accounts.client, amountClient);
        QSharedPointer<AccountingEntrySet> entrySet(
                    new AccountingEntrySet(AccountingEntrySet::SaleMarketplace));
        QString id = "cdiscount-" + datePayment.toString("-yyyy-MM-dd");
        ReportMonthlyAmazon reportGenerator;
        QString reportTitle = QObject::tr("Paiement") + " CDiscount - "
                + datePayment.toString("-yyyy-MM-dd") + " - "
                + dateStartPayment.toString("yyyy-MM-dd")
                + " => " + datePayment.toString("yyyy-MM-dd")
                + " (" + reportFileName + ")";
        reportGenerator.addTitle(reportTitle);
                entrySet->setId(id);
        entrySet->setAmountOrig(-chargedAmount);
        entrySet->setCurrencyOrig("EUR");
        QString label = QObject::tr("Paiement")
              + " Cdiscount " + chargedAmountStr + " EUR";
        QString titleDetail = QObject::tr("Détails du compte") + " ";
        AccountingEntry entryDebitCustomer;
        entryDebitCustomer.setJournal(journal);
        entryDebitCustomer.setLabel(label);
        entryDebitCustomer.setDate(datePayment);
        entryDebitCustomer.setCurrency("EUR");
        AccountingEntry entryCreditSales = entryDebitCustomer;
        AccountingEntry entryDebitCharges = entryDebitCustomer;
        //AccountingEntry entryCreditBalancePrevious = entryDebitCustomer;
        //AccountingEntry entryDebitBalanceCurrent = entryDebitCustomer;
        AccountingEntry entryBalance = entryDebitCustomer;
        entryDebitCustomer.setAccount(accounts.client);
        if (chargedAmount > 0) {
            entryDebitCustomer.setDebitOrig(chargedAmount);
        } else {
            entryDebitCustomer.setCreditOrig(-chargedAmount);
        }
        if (qAbs(chargedAmount) >= 0.01) {
            entrySet->addEntry(entryDebitCustomer);
        }


        /// SALES
        if (balanceAmounts.contains(accounts.salesUnknown)) {
            entryCreditSales.setAccount(accounts.salesUnknown);
            if (salesTotal > 0) {
                entryCreditSales.setCreditOrig(salesTotal);
            } else {
                entryCreditSales.setDebitOrig(-salesTotal);
            }
            if (qAbs(salesTotal) >= 0.01) {
                entrySet->addEntry(entryCreditSales);
                QString titleSales = titleDetail + entryCreditSales.account();
                titleSales += " (" + entryCreditSales.amountOrig()
                        + " EUR)";
                reportGenerator.addTitle(titleSales);
                reportGenerator.addTableNonSale();

                for (auto itSales = balanceAmounts[accounts.salesUnknown].begin();
                     itSales != balanceAmounts[accounts.salesUnknown].end(); ++itSales) {
                    //salesTotal += itSales->amount;
                    reportGenerator.addTableNonSaleRow(
                                itSales->reportFileName,
                                itSales->reportId,
                                itSales->title,
                                itSales->orderId,
                                itSales->date,
                                itSales->row,
                                itSales->amount);
                }
                reportGenerator.addTableNonSaleTotal(salesTotal);
            }
        }

        /// FEES
        if (balanceAmounts.contains(accounts.supplier)) {
            bool gatherFees = false;
            entryDebitCharges.setAccount(accounts.supplier);
            if (feesTotal > 0) {
                entryDebitCharges.setCreditOrig(feesTotal);
            } else {
                entryDebitCharges.setDebitOrig(-feesTotal);
            }
            QString titleFees = titleDetail + entryDebitCharges.account()
                    + " (" + entryDebitCharges.amountOrig()
                    + " EUR)";
            reportGenerator.addTitle(titleFees);
            reportGenerator.addTableNonSale();
            if (gatherFees) {
                if (qAbs(feesTotal) >= 0.01) {
                    entrySet->addEntry(entryDebitCharges);
                    for (auto itFees = balanceAmounts[accounts.supplier].begin();
                         itFees != balanceAmounts[accounts.supplier].end(); ++itFees) {
                        reportGenerator.addTableNonSaleRow(
                                    itFees->reportFileName,
                                    itFees->reportId,
                                    itFees->title,
                                    itFees->orderId,
                                    itFees->date,
                                    itFees->row,
                                    itFees->amount);
                    }
                }
            } else {
                QMap<QString, QMultiMap<QString,
                        EntryParserMarketplaceMonthly::BalanceAmount>> balanceAmountsByInvoiceIds;
                for (auto itFees = balanceAmounts[accounts.supplier].begin();
                     itFees != balanceAmounts[accounts.supplier].end(); ++itFees) {
                    if (!balanceAmountsByInvoiceIds.contains(itFees->reportId)) {
                        balanceAmountsByInvoiceIds[itFees->reportId]
                                = QMultiMap<QString,
                                EntryParserMarketplaceMonthly::BalanceAmount>();
                    }
                    balanceAmountsByInvoiceIds[itFees->reportId].insert(
                                itFees.key(), itFees.value());
                }
                for (auto itFeesInvoice = balanceAmountsByInvoiceIds.begin();
                     itFeesInvoice != balanceAmountsByInvoiceIds.end(); ++itFeesInvoice) {
                    auto entryDebitChargesInvoice = entryDebitCharges;
                    double feesTotalInvoice = 0.;
                    for (auto itFees = itFeesInvoice.value().begin();
                         itFees != itFeesInvoice.value().end(); ++itFees) {
                        feesTotalInvoice += itFees->amount;
                        reportGenerator.addTableNonSaleRow(
                                    itFees->reportFileName,
                                    itFees->reportId,
                                    itFees->title,
                                    itFees->orderId,
                                    itFees->date,
                                    itFees->row,
                                    itFees->amount);
                    }
                    if (feesTotalInvoice > 0) {
                        entryDebitChargesInvoice.setCreditOrig(QString());
                        entryDebitChargesInvoice.setDebitOrig(feesTotalInvoice);
                    } else {
                        entryDebitChargesInvoice.setDebitOrig(QString());
                        entryDebitChargesInvoice.setCreditOrig(-feesTotalInvoice);
                    }
                    QString label = entryDebitChargesInvoice.label();
                    label += " ";
                    label += QObject::tr("Facture");
                    label += " ";
                    label += itFeesInvoice.key();
                    entryDebitChargesInvoice.setLabelReplaced(label);
                    entrySet->addEntry(entryDebitChargesInvoice);
                }
            }
            reportGenerator.addTableNonSaleTotal(feesTotal);
        }


        /// RESERVE
        if (accounts.reserve != accounts.supplier && balanceAmounts.contains(accounts.reserve)) {
            entryBalance.setAccount(accounts.reserve);
            //entryCreditBalancePrevious.setAccount(accounts.reserve);
            //entryDebitBalanceCurrent.setAccount(accounts.reserve);
            double balanceTotal = balanceCreditTotal + balanceDebitTotal;
            if (balanceTotal > 0) {
                entryBalance.setCreditOrig(balanceTotal);
            } else {
                entryBalance.setDebitOrig(-balanceTotal);
            }
            if (qAbs(balanceTotal) >= 0.01) {
                entrySet->addEntry(entryBalance);
                QString labelBalanceDebit = QObject::tr("Retenu de garantie");
                QString titleBalance = titleDetail + entryBalance.account()
                        + " (" + QObject::tr("Retenu de garantie")
                        + ": " + QString::number(balanceCreditTotal, 'f', 2)
                        + " / " + QString::number(balanceDebitTotal, 'f', 2)
                        + " EUR)";
                reportGenerator.addTitle(titleBalance);

                reportGenerator.addTableNonSale();
                for (auto itReserve = balanceAmounts[accounts.reserve].begin();
                     itReserve != balanceAmounts[accounts.reserve].end(); ++itReserve) {
                    reportGenerator.addTableNonSaleRow(
                                itReserve->reportFileName,
                                itReserve->reportId,
                                itReserve->title,
                                itReserve->orderId,
                                itReserve->date,
                                itReserve->row,
                                itReserve->amount);
                }
                reportGenerator.addTableNonSaleTotal(balanceTotal);
            }
        }
        reportGenerator.endHtml();
        QString baseFileName = id + "__"
                + QString::number(chargedAmount, 'f', 2) + " EUR";
        entrySet->setHtmlDocument(reportGenerator.html(),
                                  baseFileName);
        if (entrySet->entries().size() > 0) {
            double diff = entrySet->creditTotalConv()
                    - entrySet->debitTotalConv();
            if (qAbs(diff) > 0.009) {
                entrySet->setState(AccountingEntrySet::Error);
            }
            //entrySet->roundCreditDebit();
            addEntryStatic(allEntries, entrySet);
        }
    }
    return allEntries;
}
//----------------------------------------------------------
AbstractEntryParser::Type EntryParserCdiscountPayment::typeOfEntries() const
{
    return SaleMarketplace;
}
//----------------------------------------------------------
double EntryParserCdiscountPayment::cdiscountToDouble(QString &string)
{
    return EntryParserCdiscountMonthly::cdiscountToDouble(string);
}
//----------------------------------------------------------
QString EntryParserCdiscountPayment::_settingKey()
{
    return "EntryParserCdiscountPayment-"
            + CustomerManager::instance()->getSelectedCustomerId();

}
//----------------------------------------------------------
