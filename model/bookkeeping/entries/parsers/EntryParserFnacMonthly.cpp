#include "../common/utils/CsvReader.h"

#include "EntryParserFnacMonthly.h"
#include "model/orderimporters/Shipment.h"
#include "model/orderimporters/ImportedFileReportManager.h"
#include "model/orderimporters/OrderImporterFnac.h"
#include "model/bookkeeping/ManagerAccountsAmazon.h"

//----------------------------------------------------------
QString EntryParserFnacMonthly::NAME
= QObject::tr("Ventes Fnac (mensuel)");
QString EntryParserFnacMonthly::JOURNAL
= "FNAC";
//----------------------------------------------------------
EntryParserFnacMonthly::EntryParserFnacMonthly()
    : EntryParserMarketplaceMonthly()
{
}
//----------------------------------------------------------
EntryParserFnacMonthly::~EntryParserFnacMonthly()
{
}
//----------------------------------------------------------
QString EntryParserFnacMonthly::name() const
{
    return NAME;
}
//----------------------------------------------------------
QString EntryParserFnacMonthly::journal() const
{
    return JOURNAL;
}
//----------------------------------------------------------
QString EntryParserFnacMonthly::nameSupplierCustomer() const
{
    return "Fnac";
}
//----------------------------------------------------------
void EntryParserFnacMonthly::recordTransactions(
        const Shipment *shipmentOrRefund)
{
     EntryParserMarketplaceMonthly::recordTransactions(shipmentOrRefund);
    if (acceptShipmentOrRefund()(shipmentOrRefund)) {
        auto dateTime = shipmentOrRefund->getDateTime();
        auto date = dateTime.date();
        if (!m_totalConvertedByYearMonth.contains(date.year())) {
            m_totalConvertedByYearMonth[date.year()] = QMap<int, double>();
        }
        if (!m_totalConvertedByYearMonth[date.year()].contains(date.month())) {
            m_totalConvertedByYearMonth[date.year()][date.month()] = 0.;
        }
        double total = shipmentOrRefund->getTotalPriceTaxedConverted();
        m_totalConvertedByYearMonth[date.year()][date.month()] += total;
    }

}
//----------------------------------------------------------
void EntryParserFnacMonthly::clearTransactions()
{
    EntryParserMarketplaceMonthly::clearTransactions();
    m_totalConvertedByYearMonth.clear();
}
//----------------------------------------------------------
std::function<bool (const Shipment *)>
EntryParserFnacMonthly::acceptShipmentOrRefund() const
{
    static auto fAccept = [](const Shipment *shipmentOrRefund) -> bool {
        return shipmentOrRefund->channel().toLower().contains("fnac");
    };
    return fAccept;
}
//----------------------------------------------------------
QMap<QString, QMultiMap<QString, EntryParserMarketplaceMonthly::BalanceAmount> > EntryParserFnacMonthly::balanceAmounts(int year, int month) const
{
    QMap<QString, QMultiMap<QString, BalanceAmount> > amounts;
    auto accounts
            = ManagerAccountsAmazon::instance()->amazonAccount(
                "fnac.com"); //TODO settings
    QString supplierVatAccount = "445660"; //TODO settings
    double amountSalesConvMonth = m_totalConvertedByYearMonth[year][month];
    amounts[accounts.client] = QMap<QString, BalanceAmount>();
    BalanceAmount amountClient;
    amountClient.amount = amountSalesConvMonth;
    amounts[accounts.supplier] = QMap<QString, BalanceAmount>();
    //amounts[supplierVatAccount] = QMap<QString, BalanceAmount>();
    //amounts[accounts.reserve] = QMap<QString, BalanceAmount>();
    QStringList filePaths
            = ImportedFileReportManager::instance()->filePaths(
                OrderImporterFnac::NAME,
                OrderImporterFnac::REPORT_PAYMENTS,
                year,
                month,
                true);
    for (auto itFilePath = filePaths.begin();
         itFilePath != filePaths.end(); ++itFilePath) {
        QString reportFileName = QFileInfo(*itFilePath).fileName();
        CsvReader reader(*itFilePath, ";", "\"", true, "\r\n", 0, "ISO 8859-1");
        reader.readAll();
        const DataFromCsv *dataRode = reader.dataRode();
        int indName = dataRode->header.pos("Produit");
        int indOrderId = dataRode->header.pos("N° commande logistique");
        int indDateTime = dataRode->header.pos("Acceptée le");
        int indAmount = dataRode->header.pos("Montant");
        int indPaymentState = dataRode->header.pos("Statut paiement");
        int indPaymentInvoice = dataRode->header.pos("N° facture");
        int idLine = 1;
        for (auto elements : dataRode->lines) {
            QString amountStr = elements[indAmount].replace(",", ".");
            if (!amountStr.isEmpty()) {
                QDate date = QDate::fromString(elements[indDateTime], "dd/MM/yyyy");
                if (date.year() == year && date.month() == month) {
                    Q_ASSERT(elements[indPaymentState].isEmpty()
                             || elements[indPaymentState] == "Payée");
                    double amount = amountStr.toDouble();
                    amountClient.amount += amount;
                    QString name = elements[indName];
                    QString paymentInvoiceId = elements[indPaymentInvoice];
                    QString orderId = elements[indOrderId];
                    if (!name.startsWith("\"")
                            && !name.startsWith("Remboursement\\")
                            && !name.startsWith("Frais de port")) { //TODO I should white list all correct value and make it crash in not correct
                        /// It means it is a fees
                        Q_ASSERT(name.contains("TVA")
                                 || name.contains("Frais")
                                 || name.contains("Abonnement"));
                        BalanceAmount amountFees;
                        amountFees.reportFileName = reportFileName;
                        amountFees.reportId = paymentInvoiceId;
                        amountFees.orderId = orderId;
                        amountFees.row = QString::number(idLine);
                        amountFees.date = date;
                        amountFees.title = name;
                        amountFees.amount = -amount;
                        /*
                        if (name.contains("TVA")) {
                            amounts[supplierVatAccount].insert(name, amountFees);
                        } else {
                            amounts[accounts.supplier].insert(name, amountFees);
                        }
                        //*/
                        amounts[accounts.supplier].insert(name, amountFees);
                    }
                }
            }
            ++idLine;
        }
    }
    amounts[accounts.client].insert(accounts.client, amountClient);
    return amounts;
}
//----------------------------------------------------------
