#ifndef INVOICEGENERATOR_H
#define INVOICEGENERATOR_H

#include <QtCore/qstring.h>
#include <QtCore/qsharedpointer.h>

#include <QtPrintSupport/qprinter.h>
#include <qtextdocument.h>
#include <qpainter.h>
class Shipment;
class Refund;

class InvoiceGenerator
{
public:
    InvoiceGenerator();
    static QSharedPointer<InvoiceGenerator> createGeneratorInvoice(
            const Shipment *shipment);
    static QSharedPointer<InvoiceGenerator> createGeneratorRefund(
            const Refund *refund);
    static void saveInvoiceOrRefund(
            const Shipment *shipment,
            const QString &filePath,
            bool replace = false);
    void addAddressFrom(const QStringList &lines);
    void addAddressTo(const QStringList &lines);

    void startTableInvoiceInfos();
    void addTableInvoiceInfosLine(
            const QString &left, const QString &right);

    void endTableInvoiceInfos();
    void addShipmentInfos(const Shipment *shipment);
    void addRefundInfos(const Refund *refund);

    void createTableProductList();
    void addArticleInfo(
            const QString &title,
            int quantity,
            double priceUntaxedTotal,
            double vatRate,
            const QString &currency);
    void addArticleTotals(double priceUntaxedTotal,
            double priceTaxesTotal, const QString &currency);
    void addTotals(double untaxed,
                   double taxes,
                   double untaxedConverted,
                   double taxesConverted,
                   const QString &currency);
    void addLegalInformations(const QStringList &lines);
    void addLawInformations(const QStringList &lines);

    void endHtml();
    void save(const QString &absPdfFilePath);
    void saveHtml(const QString &absHtmlFilePath);

    QString html() const;
    void setHtml(const QString &html);

protected:
    QString m_html;
    static void _initBeforeRefundInfos(
            QSharedPointer<InvoiceGenerator> generator,
            const Shipment *shipment);
    static void _initAfterRefundInfos(
            QSharedPointer<InvoiceGenerator> generator,
            const Shipment *shipment);
};

#endif // INVOICEGENERATOR_H
