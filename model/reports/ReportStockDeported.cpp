#include <QObject>

#include "ReportStockDeported.h"

//----------------------------------------------------------
ReportStockDeported::ReportStockDeported()
    : AbstractReportGenerator()
{
}
//----------------------------------------------------------
void ReportStockDeported::addTable()
{
    m_html += "<br>";
    m_html += "<table><tr style=\"background:#afc6e9\"><th style=\"padding:8px;\">";
    static QStringList colNames = {
        QObject::tr("Code article")
        , QObject::tr("Unité")
        , QObject::tr("Prix unitaire")
        , QObject::tr("Prix total")
    };
    m_html += colNames.join("</th><th style=\"padding:8px;\">");
    m_html += "</th></tr>";
}
//----------------------------------------------------------
void ReportStockDeported::addTableRow(
        const QString &code, int units, double unitPrice)
{
    QStringList values = {
        code
        , QString::number(units)
        , QString::number(unitPrice, 'f', 2)
        , QString::number(units * unitPrice, 'f', 2)
    };
    m_html += "<tr style=\"background:#d7f4ee\"><td style=\"padding:8px;\">";
    m_html += values.join("</td><td style=\"padding:8px;\">");
    m_html += "</td></tr>";
}
//----------------------------------------------------------
void ReportStockDeported::addTableTotal(double totalPrice)
{
    QStringList values = {
        QString()
        , QString()
        , QString()
        , QString::number(totalPrice, 'f', 2)
    };
    m_html += "<tr style=\"background:#d7f4ee\"><b><td style=\"padding:8px;\"><b>";
    m_html += values.join("</b></td><td style=\"padding:8px;\"><b>");
    m_html += "</b></td></tr></table>";
}
//----------------------------------------------------------
