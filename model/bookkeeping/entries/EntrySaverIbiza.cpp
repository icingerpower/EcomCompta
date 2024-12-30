#include <QtCore/qset.h>
#include <QTextCodec>
#include <QtCore/qfile.h>
#include <QtCore/qmap.h>
#include <QtCore/qtextstream.h>

#include "EntrySaverIbiza.h"
#include "model/reports/ReportMonthlyAmazon.h"
#include "model/bookkeeping/invoices/InvoiceGenerator.h"

//----------------------------------------------------------
EntrySaverIbiza::EntrySaverIbiza() : AbstractEntrySaver ()
{
}
//----------------------------------------------------------
QString EntrySaverIbiza::name() const
{
    return "Ibiza";
}
//----------------------------------------------------------
void EntrySaverIbiza::save(
        const AccountingEntries &entries, const QDir &dir)
{
    dir.mkpath(".");
    QDir fileDir = dir;
    auto allCodecs = QTextCodec::availableCodecs();
    decltype(allCodecs) codecs;
    for (const auto &codec : qAsConst(allCodecs))
    {
        if (codec.contains("8859-15") || codec.contains("latin1"))
        {
            codecs << codec;
        }
    }
    for (auto itYear = entries.begin();
         itYear != entries.end(); ++itYear) {
        QString year = QString::number(itYear.key());
        QDir yearDir = fileDir;
        yearDir.mkpath(year);
        yearDir.cd(year);
        QMap<QString, QMultiMap<QDate, QString>> allLinesByMonthAndDate;
        for (auto itJournal = itYear.value().begin();
             itJournal != itYear.value().end(); ++itJournal) {
            QString journal = itJournal.key();
            QDir journalDir = yearDir;
            journalDir.mkpath(journal);
            journalDir.cd(journal);
            for (auto itMonth = itJournal.value().begin();
                 itMonth != itJournal.value().end(); ++itMonth) {
                QString month = itMonth.key();
                QDir monthDir = journalDir;
                monthDir.mkpath(month);
                monthDir.cd(month);
                if (!allLinesByMonthAndDate.contains(month)) {
                    allLinesByMonthAndDate[month] = QMultiMap<QDate, QString>();
                }
                QString baseName = journal + "-" + year + "-" + month;
                QString csvFileName = baseName + ".csv";
                QString pdfFileName = baseName + ".pdf";
                QMultiMap<QDate, QString> lines;
                QString csvFilePath = monthDir.filePath(csvFileName);
                QString pdfFilePath = monthDir.filePath(pdfFileName);
                //fileDir.cd("..");
                //filePath = fileDir.filePath(fileName);
                QFile file(csvFilePath);
                QSet<QString> documentsDisplayPaths;
                if (file.open(QFile::WriteOnly)) {
                    QString html;
                    for (auto itLabel = itMonth.value().begin();
                         itLabel != itMonth.value().end(); ++itLabel) {
                        auto entrySet = itLabel.value();
                        entrySet->roundCreditDebit();
                        QList<double> debits;
                        QList<double> credits;
                        double totalDebit = 0.;
                        double totalCredit = 0.;
                        auto entries = itLabel.value()->entries();
                        for (auto itEntry = entries.begin();
                             itEntry != entries.end(); ++itEntry) {
                            double debitConv = itEntry->debitConv().toDouble();
                            debits << debitConv;
                            totalDebit += debitConv;
                            double creditConv = itEntry->creditConv().toDouble();
                            credits << creditConv;
                            totalCredit += creditConv;
                        }
                        double diff = totalDebit - totalCredit;
                        if (qAbs(diff) > 0.005) {
                            if (debits.last() > 0.001) {
                                debits.last() -= diff;
                            } else {
                                credits.last() += diff;
                            }
                        }
                        int i = 0;
                        for (auto itEntry = entries.begin();
                             itEntry != entries.end(); ++itEntry) {
                            if (qAbs(debits[i]) > 0.005 || qAbs(credits[i]) > 0.005) {
                                QString line = itEntry->date().toString("dd/MM/yyyy");
                                line += ";" + journal;
                                line += ";" + itEntry->accountReplaced();
                                line += ";" + QString::number(debits[i], 'f', 2);
                                line += ";" + QString::number(credits[i], 'f', 2);
                                line += ";" + itEntry->labelReplaced();
                                lines.insert(itEntry->date(), line);
                                allLinesByMonthAndDate[month].insert(itEntry->date(), line);
                                ++i;
                            }
                        }
                        QString htmlEntrySet = entrySet->htmlDocument();
                        QString entrySetBaseName = entrySet->htmlDocBaseName();
                        if(html.isEmpty() && !htmlEntrySet.isEmpty() && entrySetBaseName.isEmpty()) {
                            html = entrySet->htmlDocument();
                        }
                        if (!entrySetBaseName.isEmpty()) {
                            QString entrySetFileName = entrySetBaseName + ".pdf";
                            QString entrySetFilePath = monthDir.filePath(entrySetFileName);
                            InvoiceGenerator invoiceGenerator; /// Can in fact generate any html
                            invoiceGenerator.setHtml(htmlEntrySet);
                            invoiceGenerator.save(entrySetFilePath);
                        } else if (!entrySetBaseName.isEmpty()) {
                        }
                        if (entrySet->hasCsvData()) {
                            auto csvFileName = entrySet->csvFileBaseName();
                            csvFileName += ".csv";
                            QString csvFilePath = monthDir.filePath(csvFileName);
                            auto csvData = entrySet->csvData();
                            QFile csvFile(csvFilePath);
                            if (csvFile.open(QFile::WriteOnly)) {
                                QTextStream stream(&csvFile);
                                for (auto itLine = csvData.begin();
                                     itLine != csvData.end(); ++itLine) {
                                    stream << itLine->join("\t") + "\n";
                                }
                                csvFile.close();
                            }
                        }
                        QString sourceDocumentDisplay = itLabel.value()->sourceDocumentDisplay();
                        if (!sourceDocumentDisplay.isEmpty()) {
                            documentsDisplayPaths << itLabel.value()->sourceDocumentDisplay();
                        }
                    }
                    QTextStream stream(&file);
                    const auto &text = lines.values().join("\n");
                    stream << text;
                    file.close();
                    for (const auto &codec : qAsConst(codecs))
                    {
                        QString csvFileNameAltCodec = baseName + "-";
                        csvFileNameAltCodec += codec;
                        csvFileNameAltCodec += ".csv";
                        QString csvFilePathMonthAltCodec = monthDir.filePath(csvFileNameAltCodec);
                        QFile fileAltCodec(csvFilePathMonthAltCodec);
                        if (fileAltCodec.open(QFile::WriteOnly))
                        {
                            QTextStream stream(&fileAltCodec);
                            stream.setCodec(codec);
                            stream << text;
                            fileAltCodec.close();
                        }
                    }

                    if (!html.isEmpty()) {
                        ReportMonthlyAmazon reportGenerator;
                        reportGenerator.setHtml(html);
                        reportGenerator.save(pdfFilePath);
                    } else if (documentsDisplayPaths.size() > 0) {
                        for (auto path: documentsDisplayPaths) {
                            QString fileName = QFileInfo(path).fileName();
                            QString displayFilePath = monthDir.filePath(fileName);
                            QFile::copy(path, displayFilePath);
                        }
                    }
                }
            }
            if (journalDir.isEmpty()) {
                journalDir.removeRecursively();
            }
        }
        QDir allDir = yearDir;
        QString dirNameAll = "all";
        allDir.mkpath(dirNameAll);
        allDir.cd(dirNameAll);
        for (auto itMonth = allLinesByMonthAndDate.begin();
             itMonth != allLinesByMonthAndDate.end(); ++itMonth) {
            QString month = itMonth.key();
            auto dates = itMonth.value().keys();
            QStringList allLines = itMonth.value().values();
            QDir allMonthDir = allDir;
            allMonthDir.mkpath(month);
            allMonthDir.cd(month);
            QString baseName = dirNameAll + "-" + year + "-" + month;
            QString csvFileName = baseName + ".csv";
            QString csvFilePathMonth = allMonthDir.filePath(csvFileName);
            QFile file(csvFilePathMonth);
            if (file.open(QFile::WriteOnly)) {
                QTextStream stream(&file);
                stream << allLines.join("\n");
                file.close();
            }
            for (const auto &codec : qAsConst(codecs))
            {
                QString csvFileNameAltCodec = baseName + "-";
                csvFileNameAltCodec += codec;
                csvFileNameAltCodec += ".csv";
                QString csvFilePathMonthAltCodec = allMonthDir.filePath(csvFileNameAltCodec);
                QFile fileAltCodec(csvFilePathMonthAltCodec);
                if (fileAltCodec.open(QFile::WriteOnly))
                {
                    QTextStream stream(&fileAltCodec);
                    stream.setCodec(codec);
                    stream << allLines.join("\n");
                    fileAltCodec.close();
                }
            }
        }
    }
}
//----------------------------------------------------------
