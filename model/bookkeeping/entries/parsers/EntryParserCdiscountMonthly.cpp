#include "../common/utils/CsvReader.h"

#include "EntryParserCdiscountMonthly.h"

#include "model/orderimporters/Shipment.h"
#include "model/orderimporters/ImportedFileReportManager.h"
#include "model/orderimporters/OrderImporterCdiscount.h"
#include "model/bookkeeping/ManagerAccountsAmazon.h"

QString EntryParserCdiscountMonthly::NAME
= QObject::tr("Ventes CDiscount (mensuel)");
QString EntryParserCdiscountMonthly::JOURNAL
= "CDISCOUNT";
//----------------------------------------------------------
EntryParserCdiscountMonthly::EntryParserCdiscountMonthly()
    : EntryParserMarketplaceMonthly()
{
}
//----------------------------------------------------------
EntryParserCdiscountMonthly::~EntryParserCdiscountMonthly()
{

}
//----------------------------------------------------------
QString EntryParserCdiscountMonthly::name() const
{
    return NAME;
}
//----------------------------------------------------------
QString EntryParserCdiscountMonthly::journal() const
{
    return JOURNAL;
}
//----------------------------------------------------------
QString EntryParserCdiscountMonthly::nameSupplierCustomer() const
{
    return "CDiscount";
}
//----------------------------------------------------------
void EntryParserCdiscountMonthly::clearTransactions()
{
    EntryParserMarketplaceMonthly::clearTransactions();
    m_totalConvertedByYearMonth.clear();
}
//----------------------------------------------------------
void EntryParserCdiscountMonthly::recordTransactions(const Shipment *shipmentOrRefund)
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
std::function<bool (const Shipment *)> EntryParserCdiscountMonthly::acceptShipmentOrRefund() const
{
    static auto fAccept = [](const Shipment *shipmentOrRefund) -> bool {
        return shipmentOrRefund->channel().toLower().contains("cdiscount");
    };
    return fAccept;
}
//----------------------------------------------------------
QMap<QString, QMultiMap<QString, EntryParserMarketplaceMonthly::BalanceAmount> >
EntryParserCdiscountMonthly::balanceAmounts(
        int year, int month) const
{
    QMap<QString, QMultiMap<QString, BalanceAmount> > amounts;
    auto accounts
            = ManagerAccountsAmazon::instance()->amazonAccount(
                "cdiscount.fr"); //TODO settings
    double amountSalesConvMonth = m_totalConvertedByYearMonth[year][month];
    amounts[accounts.client] = QMap<QString, BalanceAmount>();
    BalanceAmount amountClient;
    amountClient.amount = amountSalesConvMonth;
    amounts[accounts.supplier] = QMap<QString, BalanceAmount>();
    amounts[accounts.reserve] = QMap<QString, BalanceAmount>();
    QStringList filePaths
            = ImportedFileReportManager::instance()->filePaths(
                OrderImporterCdiscount::NAME,
                OrderImporterCdiscount::REPORT_PAYMENTS,
                year,
                month,
                true);
    for (auto itFilePath = filePaths.begin();
         itFilePath != filePaths.end(); ++itFilePath) {
        QString reportFileName = QFileInfo(*itFilePath).fileName();
        /*
        QSet<QString> orderIdsFileOrderDEB;
        if (reportFileName.contains("01-2022")) {
            QString debFilePath = "/home/cedric/Dropbox/compta-nidal/rapports/Maazoun/cdiscount/rapport-volet-commande/2022/OrderExtract_01-2022.csv";
            CsvReader reader(debFilePath, ";", "", true, "\r\n", 5);
            reader.readAll();
            const DataFromCsv *dataRode = reader.dataRode();
            int indOrderStatusDEB = dataRode->header.pos("Statut commande");
            int indOrderIdDEB = dataRode->header.pos("Référence commande");
            for (auto elements : dataRode->lines) {
                QString orderIdDEB = elements[indOrderIdDEB];
                orderIdsFileOrderDEB << orderIdDEB;
            }
        }
        QSet<QString> orderIdsFilePaymentDEB;
        //*/
        int linesToSkip = 3;
        CsvReader reader(*itFilePath, ";", "\"", true, "\r\n", linesToSkip);
        reader.readAll();
        const DataFromCsv *dataRode = reader.dataRode();
        //int indCdiscountInvoice = dataRode->header.pos("N° facture/avoir");
        QHash<QString, QString> orderIdToInvoiceId;
        int indPaymentInvoice = dataRode->header.pos("N° facture/avoir");
        int indOrderId = dataRode->header.pos("N° commande / Service");
        int indDateTime = dataRode->header.pos("Date opération comptable");
        int indDateTimeAnticipatedPayment
                = dataRode->header.pos("Date de mise en paiement anticipé");
        int indComProduit = dataRode->header.pos("Commission Produit");
        int indComFacilitePayment = dataRode->header.pos("Commission Facilités de paiement");
        int indComPayment4times = dataRode->header.pos("Commission Frais de paiement 4 fois");
        int indComAvoir = dataRode->header.pos("Avoir commission");
        int indTotalPaid = dataRode->header.pos("Total reçu");
        int idLine = linesToSkip + 2;
        double specialFeesDEB = 0.;
        for (auto elements : dataRode->lines) {
            QDate date = QDate::fromString(elements[indDateTime], "dd/MM/yyyy");
            if (date.year() == year && date.month() == month) {
                double fees = 0.;
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
                    if (serviceName.contains("garantie")) {
                        double amount = cdiscountToDouble(elements[indTotalPaid]);
                        if (amount > 0.001 ) {
                            balanceCredit += amount;
                        } else if (amount < 0.001 ) {
                            balanceDebit += amount;
                        }
                    } else {
                        fees = cdiscountToDouble(elements[indTotalPaid]);
                    }
                } else {
                    titleFees = QObject::tr("Commissions") + " " + serviceName;
                    fees = cdiscountToDouble(elements[indComProduit])
                            + cdiscountToDouble(elements[indComPayment4times])
                            + cdiscountToDouble(elements[indComAvoir]);
                    if (!elements[indComProduit].isEmpty()) {
                        fees += cdiscountToDouble(elements[indComFacilitePayment]);
                        specialFeesDEB += cdiscountToDouble(elements[indComFacilitePayment]);
                    }
                    orderId = serviceName;
                }
                /*
                int indTotalRefundedDEB
                        = dataRode->header.pos("Remboursement TTC hors frais de port");
                bool isRefundDEB = !elements[indTotalRefundedDEB].isEmpty() || !elements[indComAvoir].isEmpty();
                if (!isRefundDEB) {
                    orderIdsFilePaymentDEB << orderId;
                }
                //*/
                if (qAbs(fees) > 0.001) {
                    /* /// NIDAL removed
                    amountClient.amount += fees;
                    BalanceAmount amountFees;
                    amountFees.reportFileName = reportFileName;
                    amountFees.reportId = paymentInvoiceId;
                    amountFees.orderId = orderId;
                    amountFees.row = QString::number(idLine);
                    amountFees.date = date;
                    amountFees.title = titleFees;
                    amountFees.amount = -fees;
                    //amounts[accounts.supplier].insert(titleFees, amountFees);
                    //*/
                } else if (qAbs(balanceDebit) > 0.001 || qAbs(balanceCredit) > 0.001) {
                    /* /// NIDAL removed
                    double balance = balanceDebit + balanceCredit; /// one is 0
                    QString titleBalance = QObject::tr("Retenue de garanti");
                    if (qAbs(balanceCredit) > 0.001) {
                        titleBalance = QObject::tr("Retenue de garanti (remboursement)");
                    }
                    amountClient.amount += balance;
                    BalanceAmount amountBalance;
                    amountBalance.reportFileName = reportFileName;
                    amountBalance.reportId = paymentInvoiceId;
                    amountBalance.orderId = orderId;
                    amountBalance.row = QString::number(idLine);
                    amountBalance.date = date;
                    amountBalance.title = titleBalance;
                    amountBalance.amount = -balance;
                    amounts[accounts.reserve].insert(titleBalance, amountBalance);
                    //*/

                    /*
                    if (!amounts[accounts.reserve].contains(titleBalance)) {
                        amounts[accounts.reserve][titleBalance] = 0.;
                    }
                    amounts[accounts.reserve][titleBalance] -= balance;
                    //*/
                }
            }
            ++idLine;
        }
        /*
        if (reportFileName.contains("01-2022")) {
            QSet<QString> inPayButNotOrder = orderIdsFilePaymentDEB;
            inPayButNotOrder.subtract(orderIdsFileOrderDEB);
            QSet<QString> inOrderBytNotPay = orderIdsFileOrderDEB;
            inOrderBytNotPay.subtract(orderIdsFilePaymentDEB);
            int TEMP=10;++TEMP;
        }
        //*/
    }
    // NIDAL changed amounts[accounts.client].insert(accounts.client, amountClient);
    amounts[accounts.salesUnknown].insert(
                QObject::tr("Ventes mensuelles CDiscount"),
                amountClient);
    return amounts;
}
//----------------------------------------------------------
double EntryParserCdiscountMonthly::cdiscountToDouble(QString &string)
{
    return string.replace(" ", "").replace(",", ".").toDouble();
}
//----------------------------------------------------------
