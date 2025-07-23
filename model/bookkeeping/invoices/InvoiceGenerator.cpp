#include <QtCore/qtextstream.h>

#include "InvoiceGenerator.h"

#include "../common/currencies/CurrencyRateManager.h"

#include "model/orderimporters/Shipment.h"
#include "model/orderimporters/Refund.h"
#include "model/orderimporters/Order.h"
#include "model/orderimporters/ServiceAccounts.h"
#include "SettingInvoices.h"
#include "SettingInvoicesHeadOffice.h"
#include "model/SettingManager.h"
#include "model/CustomerManager.h"

//----------------------------------------------------------
InvoiceGenerator::InvoiceGenerator()
{
    m_html = "<html><body>";
}
//----------------------------------------------------------
QSharedPointer<InvoiceGenerator> InvoiceGenerator::createGeneratorInvoice(
        const Shipment *shipment)
{
    QSharedPointer<InvoiceGenerator> generator(
                new InvoiceGenerator);
    _initBeforeRefundInfos(generator, shipment);
    generator->addShipmentInfos(shipment);
    _initAfterRefundInfos(generator, shipment);
    return generator;
}
//----------------------------------------------------------
void InvoiceGenerator::_initBeforeRefundInfos(
        QSharedPointer<InvoiceGenerator> generator,
        const Shipment *shipment)
{
    auto order = shipment->getOrder();
    //generator->addAddressFrom(SettingInvoices::instance()->addressFrom());
    auto date = shipment->getDateTime().date();
    generator->addAddressFrom(
                SettingInvoicesHeadOffice::instance()->addressFrom(date));
    generator->addAddressTo(
                order->getAddressTo().toStringList());
    generator->startTableInvoiceInfos();
    QString invoiceOrCredit = QObject::tr("Date de la facture:");
    if (shipment->isRefund()) {
        invoiceOrCredit = QObject::tr("Date de l'avoir:");
    }
    generator->addTableInvoiceInfosLine(
                QObject::tr("Numéro de facture :") + " <b>"
            + shipment->getInvoiceName() + "</b>",
            invoiceOrCredit
            + " " + date.toString(SettingManager::DATE_FORMAT_DISPLAY));
    QDate datePayment = date.addMonths(1);
    generator->addTableInvoiceInfosLine(
                QObject::tr("Numéro de commande :") + " "
            + order->getId(),
            QObject::tr("Date du paiement:")
            + " " + datePayment.toString(SettingManager::DATE_FORMAT_DISPLAY));
    QString refundText = shipment->isRefund() ? QObject::tr("Remboursement") : "";
    double taxes = shipment->getTotalPriceTaxesConverted();
    if (!shipment->isService()) {
        generator->addTableInvoiceInfosLine(
                    QObject::tr("Site :") + " "
                    + shipment->subchannel(), refundText);
    } else {
        ServiceAccounts serviceAccounts(
                    shipment->getOrder()->getAddressTo().internalId());
        if (qAbs(taxes) < 0.001) {
            generator->addTableInvoiceInfosLine(
                        QObject::tr("Autoliquidation par le preneur"),
                        QObject::tr("Article 44 et 196 de la Directive 2006/112/CE du Conseil du 28 novembre 2006 relative au système commun de TVA"));
        } else if (serviceAccounts.hasVatToDeclareOnPayment()) {
            generator->addTableInvoiceInfosLine(
                        QObject::tr("TVA acquittée sur encaissements"),
                        QString());
        }
    }
    QString cur = CustomerManager::instance()->getSelectedCustomerCurrency();
    QString curShipment = shipment->getCurrency();
    if (cur != curShipment) {
        auto date = shipment->getDateTime().date();
        double rate = CurrencyRateManager::instance()->rate(
                    cur, curShipment, date);
        QString rateStr = " 1 " + cur
                + " = " + QString::number(rate, 'f', 5)
                + " " + curShipment;
        generator->addTableInvoiceInfosLine(rateStr, "");
    }
    generator->endTableInvoiceInfos();
}
//----------------------------------------------------------
void InvoiceGenerator::_initAfterRefundInfos(
        QSharedPointer<InvoiceGenerator> generator,
        const Shipment *shipment)
{
    generator->createTableProductList();
    auto articleShipped = shipment->getArticlesShipped();
    for (auto it = articleShipped.begin(); it != articleShipped.end(); ++it) {
        int units = it.value()->getUnits();
        generator->addArticleInfo(
                    it.value()->getName(),
                    it.value()->getUnits(),
                    it.value()->getTotalPriceUntaxed() / units,
                    it.value()->getTotalPriceTaxes() / units,
                    it.value()->getCurrency());
    }
    auto shipping = shipment->getShipping();
    double shippingPriceTaxed = shipping.totalPriceTaxed();
    double shippingPriceUntaxed = shipping.totalPriceUntaxed();
    double shippingPriceTaxes = shipping.totalTaxes();
    if (shipment->isFirstShipment()) {
        auto shippingOrder = shipment->getOrder()->getShipping();
        shippingPriceTaxed += shippingOrder.totalPriceTaxed();
        shippingPriceUntaxed += shippingOrder.totalPriceUntaxed();
        shippingPriceTaxes += shippingOrder.totalTaxes();
    }
    if (qAbs(shippingPriceTaxed) > 0.001) {
        QString title;
        if (shippingPriceTaxed > 0) {
            title = QObject::tr("Livraison");
        } else {
            title = QObject::tr("Reduction");
        }
        generator->addArticleInfo(
                    title,
                    1,
                    shippingPriceUntaxed,
                    shippingPriceTaxes,
                    shipping.currency());
    }
    generator->addArticleTotals(
                shipment->getTotalPriceUntaxed(),
                shipment->getTotalPriceTaxes(),
                shipment->getCurrency());
    generator->addTotals(
                shipment->getTotalPriceUntaxed()
                , shipment->getTotalPriceTaxes()
                , shipment->getTotalPriceUntaxedConverted()
                , shipment->getTotalPriceTaxesConverted()
                , shipment->getCurrency()
                );
    auto date = shipment->getDateTime().date();
    generator->addLegalInformations(
                //SettingInvoices::instance()->textBottomLegal());
                SettingInvoicesHeadOffice::instance()->textBottomLegal(date));
    generator->addLawInformations(
                //SettingInvoices::instance()->textBottomLaw());
                SettingInvoicesHeadOffice::instance()->textBottomLaw(date));
}
//----------------------------------------------------------
QSharedPointer<InvoiceGenerator> InvoiceGenerator::createGeneratorRefund(
        const Refund *refund)
{
    QSharedPointer<InvoiceGenerator> generator(
                new InvoiceGenerator);
    _initBeforeRefundInfos(generator, refund);
    //generator->addShipmentInfos(refund->getFirstShipment().data());
    generator->addRefundInfos(refund);
    _initAfterRefundInfos(generator, refund);
    return generator;
}
//----------------------------------------------------------
void InvoiceGenerator::saveInvoiceOrRefund(const Shipment *shipment,
        const QString &filePath,
        bool replace)
{
    if (replace || !QFile::exists(filePath)) {
        QSharedPointer<InvoiceGenerator> generator;
        if (shipment->isRefund()) {
            const Refund *refund = static_cast<const Refund *>(shipment);
            generator = createGeneratorRefund(refund);
        } else {
            generator = createGeneratorInvoice(shipment);
        }
        generator->save(filePath);
    }
}
//----------------------------------------------------------
void InvoiceGenerator::addAddressFrom(const QStringList &lines)
{
    m_html += "<div>";
    for (auto line = lines.begin(); line != lines.end(); ++line) {
        m_html += "<p>" + *line + "</p>";
    }
    m_html += "</div>";
}
//----------------------------------------------------------
void InvoiceGenerator::addAddressTo(const QStringList &lines)
{
    m_html += "<div style=\"margin-left:300px\">";
    for (auto line = lines.begin(); line != lines.end(); ++line) {
        m_html += "<p>" + *line + "</p>";
    }
    m_html += "</div><br>";
}
//----------------------------------------------------------
void InvoiceGenerator::startTableInvoiceInfos()
{
    m_html += "<table>";
}
//----------------------------------------------------------
void InvoiceGenerator::addTableInvoiceInfosLine(
        const QString &left, const QString &right)
{
    m_html += "<tr><td style=\"padding-bottom:8px\"><p>" + left + "</p></td>";
    m_html += "<td style=\"padding-bottom:8px;padding-left:40px\"><p>" + right + "</p></td></tr>";
}
//----------------------------------------------------------
void InvoiceGenerator::endTableInvoiceInfos()
{
    m_html += "</table>";
}
//----------------------------------------------------------
void InvoiceGenerator::addShipmentInfos(const Shipment *shipment)
{
    m_html += "<p>";
    if (shipment->isService()) {
        m_html += QObject::tr("Service");;
    } else if (shipment->getOrder()->getShippedBySeller()) {
        m_html += QObject::tr("Expédition");;
    } else {
        m_html += QObject::tr("Expédié par marketplace");;
    }
    m_html += " " + shipment->countryNameFrom();
    m_html += " => " + shipment->countryNameTo();
    m_html += " (" + QObject::tr("Régime", "Régime de TVA");
    m_html += " " + shipment->getRegimeVat() + ")";
    m_html += "</p>";
}
//----------------------------------------------------------
void InvoiceGenerator::addRefundInfos(const Refund *refund)
{
    m_html += "<p>";
    auto shipments = refund->getShipments();
    if (shipments.size() == 1) {
        m_html += QObject::tr("Facture d'origine :");
    } else if (shipments.size() > 1) {
        m_html += QObject::tr("Factures d'origine :");
    }
    m_html += " ";
    QStringList invoiceNames;
    for (auto shipment : shipments) {
        invoiceNames << shipment->getInvoiceName();
    }
    m_html += invoiceNames.join(" - ");
    m_html += "</p>";
}
//----------------------------------------------------------
void InvoiceGenerator::createTableProductList()
{
    m_html += "<br>";
    m_html += "<table><tr style=\"background:#afc6e9\"><th style=\"padding:8px;\">";
    QStringList colNames;
    colNames << QObject::tr("Nom du produit");
    colNames << QObject::tr("Quantité");
    colNames << QObject::tr("Taux de TVA");
    colNames << QObject::tr("Prix HT");
    colNames << QObject::tr("TVA");
    colNames << QObject::tr("Monnaie");
    m_html += colNames.join("</th><th style=\"padding:8px;\">");
    m_html += "</th></tr>";
}
//----------------------------------------------------------
void InvoiceGenerator::addArticleInfo(
        const QString &title,
        int quantity,
        double priceUntaxedTotal,
        double taxesTotal,
        const QString &currency)
{
    QStringList values;
    double vatRate = taxesTotal / priceUntaxedTotal;
    values << title;
    values << QString::number(quantity);
    values << SettingManager::formatVatRate(vatRate);
    values << QString::number(priceUntaxedTotal, 'f', 2);
    values << QString::number(taxesTotal, 'f', 2);
    values << currency;
    m_html += "<tr style=\"background:#d7f4ee\"><td style=\"padding:8px;\">";
    m_html += values.join("</td><td style=\"padding:8px;\">");
    m_html += "</td></tr>";
}
//----------------------------------------------------------
void InvoiceGenerator::addArticleTotals(
        double priceUntaxedTotal,
        double priceTaxesTotal,
        const QString &currency)
{
    QStringList values;
    values << QObject::tr("Total");
    values << "";
    values << "";
    values << QString::number(priceUntaxedTotal, 'f', 2);
    values << QString::number(priceTaxesTotal, 'f', 2);
    values << currency;
    m_html += "<tr style=\"background:#d7f4ee\"><b><td style=\"padding:8px;\"><b>";
    m_html += values.join("</b></td><td style=\"padding:8px;\"><b>");
    m_html += "</b></td></tr></table>";
}
//----------------------------------------------------------
void InvoiceGenerator::addTotals(
        double untaxed,
        double taxes,
        double untaxedConverted,
        double taxesConverted,
        const QString &currency)
{
    m_html += "<table style=\"text-align:center\">";
    QString defaultCurrency = CustomerManager::instance()->getSelectedCustomerCurrency();
    QStringList values;
    values << QObject::tr("Total HT");
    values << QString::number(untaxedConverted, 'f', 2) + " " + defaultCurrency;
    if (currency != defaultCurrency) {
        values << QString::number(untaxed, 'f', 2) + " " + currency;
    }
    m_html += "<tr><td style=\"padding:8px;\">";
    m_html += values.join("</td><td style=\"padding:8px;\">");
    m_html += "</td></tr>";
    values.clear();

    values << QObject::tr("TVA");
    values << QString::number(taxesConverted, 'f', 2) + " " + defaultCurrency;
    if (currency != defaultCurrency) {
        values << QString::number(taxes, 'f', 2) + " " + currency;
    }
    m_html += "<tr><td style=\"padding:8px;\">";
    m_html += values.join("</td><td style=\"padding:8px;\">");
    m_html += "</td></tr>";
    values.clear();

    values << QObject::tr("Total TTC");
    double taxedConverted = untaxedConverted + taxesConverted;
    values << QString::number(taxedConverted, 'f', 2) + " " + defaultCurrency;
    if (currency != defaultCurrency) {
        double taxed = untaxed + taxes;
        values << QString::number(taxed, 'f', 2) + " " + currency;
    }
    m_html += "<tr><td style=\"padding:8px;\"><b>";
    m_html += values.join("</b></td><td style=\"padding:8px;\"><b>");
    m_html += "</b></td></tr>";
    m_html += "</table><br>";
}
//----------------------------------------------------------
void InvoiceGenerator::addLegalInformations(const QStringList &lines)
{
    m_html += "<div>";
    for (auto line = lines.begin(); line != lines.end(); ++line) {
        m_html += "<p style=\"margin-top:2px;margin-bottom:2px;text-align:center;font-size:10px;size:10px;\">" + *line + "</p>";
    }
    m_html += "</div>";
}
//----------------------------------------------------------
void InvoiceGenerator::addLawInformations(const QStringList &lines)
{
    m_html += "<div>";
    for (auto line = lines.begin(); line != lines.end(); ++line) {
        m_html += "<p style=\"margin-top:2px;margin-bottom:2px;text-align:center;font-size:8px;size:8px;\">" + *line + "</p>";
    }
    m_html += "</div>";
}
//----------------------------------------------------------
void InvoiceGenerator::endHtml()
{
    m_html += "</html></body>";
}
//----------------------------------------------------------
void InvoiceGenerator::save(
        const QString &absPdfFilePath)
{
    QTextDocument document;
    QPrinter printer;
    printer.setPageMargins(10.0,10.0,10.0,10.0,printer.Millimeter);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPrinter::A4);
    auto width = printer.pageRect().width();
    document.setTextWidth(width);
    document.setHtml(m_html);
    printer.setColorMode(QPrinter::Color);
    printer.setOutputFileName(absPdfFilePath);
    document.print(&printer);
}
//----------------------------------------------------------
QString InvoiceGenerator::html() const
{
    return m_html;
}
//----------------------------------------------------------
void InvoiceGenerator::setHtml(
        const QString &html)
{
    m_html = html;
}
//----------------------------------------------------------
void InvoiceGenerator::saveHtml(
        const QString &absHtmlFilePath)
{
    QFile file(absHtmlFilePath);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << m_html;
        file.close();
    }
}
//----------------------------------------------------------

