#include "../common/utils/CsvReader.h"

#include "EntryParserBankWise.h"

//----------------------------------------------------------
EntryParserBankWise::EntryParserBankWise()
    : EntryParserBank()
{
}
//----------------------------------------------------------
EntryParserBankWise::~EntryParserBankWise()
{

}
//----------------------------------------------------------
QList<QVariantList> EntryParserBankWise::loadValues(
        const QString &absFileName) const
{
    QString sep = ",";
    QString guill = "\"";
    CsvReader reader(absFileName, sep, guill);
    reader.readAll();
    const DataFromCsv *dataRode = reader.dataRode();
    int indDate = dataRode->header.pos("Date");
    int indName = dataRode->header.pos("Payer Name");
    int indName2 = dataRode->header.pos("Payee Name");
    int indName3 = dataRode->header.pos("Merchant");
    int indComment = dataRode->header.pos("Description");
    int indAmount = dataRode->header.pos("Amount");
    int indCurrency = dataRode->header.pos("Currency");
    QList<QVariantList> lines;
    int idRow = 1;
    for (auto elements : dataRode->lines) {
        QVariantList variants;
        variants << idRow; /// 0
        QDate date = QDateTime::fromString(
                        elements[indDate], "dd-MM-yyyy")
                .date();
        Q_ASSERT(date.isValid());
        variants << date; /// 1
        QString title = elements[indComment];
        if (!elements[indName].isEmpty()) {
            title = elements[indName] + " " + title;
        }
        if (!elements[indName2].isEmpty()) {
            title = elements[indName2] + " " + title;
        }
        if (!elements[indName3].isEmpty()) {
            title = elements[indName3] + " " + title;
        }
        variants << title; /// 2
        double amount = elements[indAmount].toDouble();
        if (qAbs(amount) > 0.001) {
            variants << elements[indAmount].toDouble(); /// 3
            variants << elements[indCurrency]; /// 4
            variants << 0.; /// 5
            lines << variants;
        }
        ++idRow;
    }
    return lines;
}
//----------------------------------------------------------
bool EntryParserBankWise::invoicesForFees() const
{
    return false;
}
//----------------------------------------------------------
QString EntryParserBankWise::name() const
{
    return "Wise" + currency();
}
//----------------------------------------------------------
QString EntryParserBankWise::journal() const
{
    return QObject::tr("BQ WISE ") + currency();
}
//----------------------------------------------------------
