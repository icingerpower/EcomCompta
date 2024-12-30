#include <QtPrintSupport/qprinter.h>
#include <qtextdocument.h>
#include <qtextstream.h>
#include <qpainter.h>

#include "ReportMonthlyAmazon.h"
#include "model/orderimporters/Shipment.h"
#include "model/orderimporters/Order.h"

//----------------------------------------------------------
ReportMonthlyAmazon::ReportMonthlyAmazon()
    : AbstractReportGenerator()
{
}
//----------------------------------------------------------
QStringList ReportMonthlyAmazon::addTable()
{
    m_html += "<br>";
    m_html += "<table><tr style=\"background:#afc6e9\"><th style=\"padding:8px;\">";
    //html += tableWithHeader[0].join("</th><th style=\"width:100px\">");
    QStringList colNames;
    for (auto colInfo : colInfos()) {
        colNames << colInfo.name;
    }
    colNames << QObject::tr("Montant HT");
    colNames << QObject::tr("TVA");
    colNames << QObject::tr("Montant TTC");
    m_html += colNames.join("</th><th style=\"padding:8px;\">");
    m_html += "</th></tr>";
    return colNames;
}
//----------------------------------------------------------
QStringList ReportMonthlyAmazon::addTableRow(const Shipment *shipment, double untaxed, double taxes)
{
    QStringList values;
    for (auto colInfo : colInfos()) {
        values << colInfo.getValue(shipment);
    }
    values << QString::number(untaxed, 'f', 2);
    values << QString::number(taxes, 'f', 2);
    values << QString::number(untaxed + taxes, 'f', 2);
    m_html += "<tr style=\"background:#d7f4ee\"><td style=\"padding:8px;\">";
    m_html += values.join("</td><td style=\"padding:8px;\">");
    m_html += "</td></tr>";
    return values;
}
//----------------------------------------------------------
QStringList ReportMonthlyAmazon::addTableTotal(double untaxed, double taxes)
{
    QStringList values;
    for (auto colInfo : colInfos()) {
        values << "";
    }
    values << QString::number(untaxed, 'f', 2);
    values << QString::number(taxes, 'f', 2);
    values << QString::number(untaxed + taxes, 'f', 2);
    m_html += "<tr style=\"background:#d7f4ee\"><b><td style=\"padding:8px;\"><b>";
    m_html += values.join("</b></td><td style=\"padding:8px;\"><b>");
    m_html += "</b></td></tr></table>";
    return values;
}
//----------------------------------------------------------
void ReportMonthlyAmazon::addTableMonthlyTotal(
        int monthMax,
        const QMap<QString, TablePriceTotal> &priceMonthlyOrig)
{
    auto priceMonthly = priceMonthlyOrig;
    TablePriceTotal total0;
    total0.taxes = 0.;
    total0.untaxed = 0.;
    for (int i=1; i<=monthMax; ++i){
        QString monthStr = QString::number(i).rightJustified(2, '0');
        if (!priceMonthly.contains(monthStr)) {
            priceMonthly[monthStr] = total0;
        }
    }
    m_html += "<br>";
    m_html += "<table><tr style=\"background:#afc6e9\"><th style=\"padding:8px;\">";
    QStringList colNames;
    colNames << "";
    QStringList monthlyUntaxed;
    monthlyUntaxed <<"<b>" + QObject::tr("Montant HT", "Total untaxed") + "</b";
    QStringList monthlyTaxes;
    monthlyTaxes << "<b>" + QObject::tr("TVA", "VAT") + "</b";

    TablePriceTotal totalYearly;
    totalYearly.taxes = 0.;
    totalYearly.untaxed = 0.;
    QVector<TablePriceTotal> totalByTrimester(4, totalYearly);
    QVector<TablePriceTotal> totalBySemester(2, totalYearly);

    for (auto it = priceMonthly.begin(); it != priceMonthly.end(); ++it) {
        int monthIndexStart0 = it.key().toInt() - 1;
        totalByTrimester[monthIndexStart0 / 3].taxes += it.value().taxes;
        totalByTrimester[monthIndexStart0 / 3].untaxed += it.value().untaxed;
        totalBySemester[monthIndexStart0 / 6].taxes += it.value().taxes;
        totalBySemester[monthIndexStart0 / 6].untaxed += it.value().untaxed;
        totalYearly.taxes += it.value().taxes;
        totalYearly.untaxed += it.value().untaxed;

        colNames << it.key();
        monthlyUntaxed << QString::number(it.value().untaxed, 'f', 2);
        monthlyTaxes << QString::number(it.value().taxes, 'f', 2);

        if ((monthIndexStart0 + 1) % 3 == 0) {
            int trimester = monthIndexStart0 / 3 + 1;
            QString trimesterTitle = QObject::tr("T", "T as trimester") + QString::number(trimester);
            colNames << trimesterTitle;
            monthlyUntaxed << "<b>" + QString::number(totalByTrimester[monthIndexStart0 / 3].untaxed, 'f', 2) + "</b>";
            monthlyTaxes << "<b>" + QString::number(totalByTrimester[monthIndexStart0 / 3].taxes, 'f', 2) + "</b>";
        }
        if ((monthIndexStart0 + 1) % 6 == 0) {
            int semester = monthIndexStart0 / 6 + 1;
            QString semesterTitle = QObject::tr("S", "S as semester") + QString::number(semester);
            colNames << semesterTitle;
            monthlyUntaxed << "<b>" + QString::number(totalBySemester[monthIndexStart0 / 6].untaxed, 'f', 2) + "</b>";
            monthlyTaxes << "<b>" + QString::number(totalBySemester[monthIndexStart0 / 6].taxes, 'f', 2) + "</b>";
        }
        if (it.key() == "12") {
            colNames << QObject::tr("Total");
            monthlyUntaxed << QString::number(totalYearly.untaxed, 'f', 2);
            monthlyTaxes << QString::number(totalYearly.taxes, 'f', 2);
        }
    }
    m_html += colNames.join("</th><th style=\"padding:8px;\">");
    m_html += "</th></tr>";
    m_html += "<tr style=\"background:#d7f4ee\"><td style=\"padding:8px;\">";
    m_html += monthlyUntaxed.join("</td><td style=\"padding:8px;\">");
    m_html += "</td></tr>";
    m_html += "<tr style=\"background:#d7f4ee\"><td style=\"padding:8px;\">";
    m_html += monthlyTaxes.join("</td><td style=\"padding:8px;\">");
    m_html += "</td></tr>";
    m_html += "</table>";
}
//----------------------------------------------------------
void ReportMonthlyAmazon::addTableNonSale()
{
    m_html += "<br>";
    m_html += "<table><tr style=\"background:#afc6e9\"><th style=\"padding:8px;\">";
    //html += tableWithHeader[0].join("</th><th style=\"width:100px\">");
    QStringList colNames;
    colNames << QObject::tr("Rapport");
    colNames << QObject::tr("ID Rapport / facture");
    colNames << QObject::tr("Titre");
    colNames << QObject::tr("Commande");
    colNames << QObject::tr("Date");
    colNames << QObject::tr("Ligne");
    colNames << QObject::tr("Montant");
    m_html += colNames.join("</th><th style=\"padding:8px;\">");
    m_html += "</th></tr>";
}
//----------------------------------------------------------
void ReportMonthlyAmazon::addTableNonSaleRow(
        const QString &report,
        const QString &reportId,
        const QString &title,
        const QString &orderId,
        const QDate &date,
        const QString &row,
        double amount)
{
    QStringList values;
    values << report;
    values << reportId;
    values << title;
    values << orderId;
    values << date.toString("yyyy-MM-dd");
    values << row;
    values << QString::number(amount, 'f', 2);
    m_html += "<tr style=\"background:#d7f4ee\"><td style=\"padding:8px;\">";
    m_html += values.join("</td><td style=\"padding:8px;\">");
    m_html += "</td></tr>";
}
//----------------------------------------------------------
void ReportMonthlyAmazon::addTableNonSaleTotal(
        double amount)
{
    QStringList values;
    for (int i=0; i<6; ++i) {
        values << "";
    }
    values << QString::number(amount, 'f', 2);
    m_html += "<tr style=\"background:#d7f4ee\"><b><td style=\"padding:8px;\"><b>";
    m_html += values.join("</b></td><td style=\"padding:8px;\"><b>");
    m_html += "</b></td></tr></table>";
}
//----------------------------------------------------------
/*
void ReportMonthlyAmazon::endHtml()
{
    m_html += "</html></body>";
}
//----------------------------------------------------------
void ReportMonthlyAmazon::save(const QString &absPdfFilePath)
{
    QTextDocument document;
    QPrinter printer;
    printer.setPageMargins(10.0,10.0,10.0,10.0,printer.Millimeter);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPrinter::A1);
    auto width = printer.pageRect().width();
    document.setTextWidth(width);
    document.setHtml(m_html);
    printer.setColorMode(QPrinter::Color);
    printer.setOutputFileName(absPdfFilePath);
    document.print(&printer);
}
//----------------------------------------------------------
void ReportMonthlyAmazon::saveHtml(
        const QString &absHtmlFilePath)
{
    QFile file(absHtmlFilePath);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << m_html;
        file.close();
    }
}
//*/
//----------------------------------------------------------
QList<ReportMonthlyAmazon::ColInfo> ReportMonthlyAmazon::colInfos() const
{
    QList<ReportMonthlyAmazon::ColInfo> infos
            = {{QObject::tr("Channel"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->subchannel();
               }}
              ,{QObject::tr("Commande"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->orderId();
               }}
              ,{QObject::tr("Activité"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getId();
               }}
              ,{QObject::tr("Type"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->isRefund()
                    ? QObject::tr("Remboursement"):QObject::tr("Vente");
               }}
              ,{QObject::tr("Facture"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getInvoiceName();
               }}
              ,{QObject::tr("Expédié vendeur"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getOrder()->getShippedBySeller()
                    ? QObject::tr("Oui"):QObject::tr("Non");
               }}
              ,{QObject::tr("Pays expédition"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getAddressFrom().countryName();
               }}
              ,{QObject::tr("Pays destination"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getOrder()->getAddressTo().countryName();
               }}
              ,{QObject::tr("Régime"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getRegimeVat();
               }}
              ,{QObject::tr("Professionnel"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getOrder()->isBusinessCustomer()
                    ? QObject::tr("Oui"):QObject::tr("Non");
               }}
              ,{QObject::tr("Numéro TVA"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getOrder()->getVatNumber();
               }}
              ,{QObject::tr("Date commande"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getOrder()->getDateTime().toString(QObject::tr("dd/MM/yyyy"));
               }}
              ,{QObject::tr("Date activité"), [](const Shipment *shipmentOrRefund) -> QString{
                   return shipmentOrRefund->getDateTime().toString(QObject::tr("dd/MM/yyyy"));
               }}
              };
    return infos;
}
//----------------------------------------------------------
//----------------------------------------------------------
TablePriceTotal::TablePriceTotal()
{
    untaxed = 0.;
    taxes = 0.;
}
//----------------------------------------------------------
//----------------------------------------------------------
