#ifndef ABSTRACTREPORTGENERATOR_H
#define ABSTRACTREPORTGENERATOR_H

#include <QtCore/qstring.h>

class AbstractReportGenerator
{
public:
    AbstractReportGenerator();

    void addTitle(const QString &title);
    void addTitleH2(const QString &title);

    void endHtml();
    void save(const QString &absPdfFilePath);
    void saveHtml(const QString &absHtmlFilePath);

    const QString &html() const;
    void setHtml(const QString &newHtml);

protected:
    QString m_html;
};

#endif // ABSTRACTREPORTGENERATOR_H
