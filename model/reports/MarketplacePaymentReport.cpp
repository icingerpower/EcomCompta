#include <QtCore/qobject.h>

#include "MarketplacePaymentReport.h"

//----------------------------------------------------------
MarketplacePaymentReport::MarketplacePaymentReport()
    : AbstractReportGenerator()
{
}
//----------------------------------------------------------
void MarketplacePaymentReport::addMarketplaceName(
        const QString &name)
{
    QString title = QObject::tr("Paiement ") + name;
    m_html += "<h1>" + title + "</h1>";
}
//----------------------------------------------------------
void MarketplacePaymentReport::addPaymentDate(
        const QDateTime &start, QDateTime &end)
{
    m_html += "<h2>";
    if (!start.isNull()) {
        m_html += start.toString() + " => ";
    }
    m_html += end.toString() + "</h2>";
}
//----------------------------------------------------------
void MarketplacePaymentReport::addPaymentId(
        const QString &paymentId)
{
    m_html += "<p>";
    m_html += QObject::tr("Paiement ") + paymentId;
    m_html +* "</p";
}
//----------------------------------------------------------
void MarketplacePaymentReport::startTableAmount()
{
    m_html += "<br>";
    m_html += "<table><tr style=\"background:#afc6e9\"><th style=\"padding:8px;\">";
    QStringList colNames;
    colNames << QObject::tr("Compte");
    colNames << QObject::tr("Titre");
    colNames << QObject::tr("Montant");
    m_html += colNames.join("</th><th style=\"padding:8px;\">");
    m_html += "</th></tr>";
}
//----------------------------------------------------------
void MarketplacePaymentReport::addAmount(
        const QString &account, const QString &name, double amount)
{
    QStringList values;
    values << account;
    values << name;
    values << QString::number(amount, 'f', 2);
    m_html += "<tr style=\"background:#d7f4ee\"><td style=\"padding:8px;\">";
    m_html += values.join("</td><td style=\"padding:8px;\">");
    m_html += "</td></tr>";
}
//----------------------------------------------------------
void MarketplacePaymentReport::endTableAmount()
{
    m_html += "</table>";
}
//----------------------------------------------------------
void MarketplacePaymentReport::addAmountWithDetails(
        const QString &account,
        const QString &name,
        double amount,
        QStringList details,
        QList<double> amounts)
{
    startTableAmount();
    QStringList values;
    values << account;
    values << name;
    values << QString::number(amount, 'f', 2);
    m_html += "<tr style=\"background:#d7f4ee\"><b><td style=\"padding:8px;\"><b>";
    m_html += values.join("</b></td><td style=\"padding:8px;\"><b>");
    m_html += "</b></td></tr>";
    for (int i=0; i<details.size(); ++i) {
        values[1] = details[i];
        values[2] = amounts[i];
        m_html += "<tr style=\"background:#d7f4ee\"><td style=\"padding:8px;\">";
        m_html += values.join("</td><td style=\"padding:8px;\">");
        m_html += "</td></tr>";
    }
    endTableAmount();
}
//----------------------------------------------------------
