#include "../common/utils/CsvReader.h"

#include "EntryParserBankQonto.h"

//----------------------------------------------------------
EntryParserBankQonto::EntryParserBankQonto()
    : EntryParserBank()
{
}
//----------------------------------------------------------
EntryParserBankQonto::~EntryParserBankQonto()
{

}
//----------------------------------------------------------
QList<QVariantList> EntryParserBankQonto::loadValues(
        const QString &absFileName) const
{
    QString sep = ",";
    QString guill = "\"";
    CsvReader reader(absFileName, sep, guill);
    reader.readAll();
    const DataFromCsv *dataRode = reader.dataRode();
    int indDate = dataRode->header.pos({"value_date_local", "operation_date_local", "Operation date (UTC)"});
    int indDate2 = dataRode->header.pos({"settlement_date_local", "Settlement date (local)"});
    int indName = dataRode->header.pos({"comment", "Reference"});
    int indComment = dataRode->header.pos({"counterpart_name", "Counterparty name"});
    int indAmount = dataRode->header.pos({"amount", "Total amount (incl. VAT)"});
    int indAmountLocal = dataRode->header.pos({"local_amount", "Total amount (incl. VAT) (local)"});
    int indCurrencyLocal = dataRode->header.pos({"local_amount_currency", "Currency"});
    QList<QVariantList> lines;
    int idRow = 1;
    for (auto elements : dataRode->lines) {
        QVariantList variants;
        variants << idRow; /// 0
        QDate date;
        if (!elements[indDate2].isEmpty()) {
            date = QDateTime::fromString(
                        elements[indDate2], "dd-MM-yyyy hh:mm:ss").date();
        } else {
            date = QDateTime::fromString(
                        elements[indDate], "dd-MM-yyyy hh:mm:ss").date();
        }
        Q_ASSERT(date.isValid());
        variants << date; /// 1
        QString title = elements[indName] + " " + elements[indComment];
        title.replace("'", "");
        QString origCurency = elements[indCurrencyLocal];
        if (origCurency != "EUR") {
            title = elements[indAmountLocal] + " " + origCurency + " " + title;
        }
        variants << title; /// 2
        double amount = elements[indAmount].toDouble();
        if (qAbs(amount) > 0.001) {
            variants << elements[indAmount].toDouble(); /// 3
            //variants << elements[indCurrency]; /// 4
            variants << "EUR";
            variants << 0.; /// 5
            lines << variants;
        }
        ++idRow;
    }
    return lines;
}
//----------------------------------------------------------
QString EntryParserBankQonto::account() const
{
    return "512501";
}
//----------------------------------------------------------
QString EntryParserBankQonto::accountFees() const
{
    return "627340";
}
//----------------------------------------------------------
bool EntryParserBankQonto::invoicesForFees() const
{
    return true;
}
//----------------------------------------------------------
QString EntryParserBankQonto::name() const
{
    return "Qonto";
}
//----------------------------------------------------------
QString EntryParserBankQonto::journal() const
{
    return QObject::tr("BQ", "bank") + " QONTO";
}
//----------------------------------------------------------
