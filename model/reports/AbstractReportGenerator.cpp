#include <QtPrintSupport/qprinter.h>
#include <qtextdocument.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qpainter.h>

#include "AbstractReportGenerator.h"

//----------------------------------------------------------
AbstractReportGenerator::AbstractReportGenerator()
{
    m_html = "<html><body>";
}
//----------------------------------------------------------
void AbstractReportGenerator::addTitle(const QString &title)
{
    m_html += "<h1>" + title + "</h1>";
}
//----------------------------------------------------------
void AbstractReportGenerator::addTitleH2(const QString &title)
{
    m_html += "<h2>" + title + "</h2>";
}
//----------------------------------------------------------
void AbstractReportGenerator::endHtml()
{
    m_html += "</html></body>";
}
//----------------------------------------------------------
void AbstractReportGenerator::save(const QString &absPdfFilePath)
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
void AbstractReportGenerator::saveHtml(const QString &absHtmlFilePath)
{
    QFile file(absHtmlFilePath);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << m_html;
        file.close();
    }
}
//----------------------------------------------------------
const QString &AbstractReportGenerator::html() const
{
    return m_html;
}
//----------------------------------------------------------
void AbstractReportGenerator::setHtml(const QString &newHtml)
{
    m_html = newHtml;
}
//----------------------------------------------------------
