#include "../common/utils/CsvReader.h"

#include "EntryParserBankStripe.h"

//----------------------------------------------------------
EntryParserBankStripe::EntryParserBankStripe() : EntryParserBank()
{
}
//----------------------------------------------------------
EntryParserBankStripe::~EntryParserBankStripe()
{

}
//----------------------------------------------------------
QList<QVariantList> EntryParserBankStripe::loadValues(
        const QString &absFileName) const
{
    QString sep = ",";
    QString guill = "\"";
    CsvReader reader(absFileName, sep, guill);
    reader.readAll();
    const DataFromCsv *dataRode = reader.dataRode();
    int indDate = dataRode->header.pos({"created", "Created (UTC)", "Created date (UTC)"});
    int indName = dataRode->header.pos({"type", "Type", "Description"});
    int indComment = dataRode->header.pos("id");
    int indAmount = dataRode->header.pos({"Amount", "amount"});
    int indFees = dataRode->header.pos({"Fee", "fee"});
    int indCurrency = dataRode->header.pos({"Currency", "currency"});
    QList<QVariantList> lines;
    int idRow = 1;
    for (auto elements : dataRode->lines) {
        QString currency = elements[indCurrency].toUpper();
        if (currency == this->currency()) {
            QVariantList variants;
            variants << idRow; /// 0
            QDate date = QDateTime::fromString(
                        elements[indDate].split(" ")[0], "yyyy-MM-dd")
                    .date();
            Q_ASSERT(date.isValid());
            variants << date; /// 1
            QString title = elements[indName] + " " + elements[indComment];
            variants << title; /// 2
            double amount = elements[indAmount].replace(",", ".").toDouble();
            if (qAbs(amount) > 0.001) {
                variants << amount; /// 3
                variants << currency; /// 4
                variants << elements[indFees].replace(",", ".").toDouble(); /// 5 fees
                lines << variants;
            }
        }
        ++idRow;
    }
    return lines;
}
//----------------------------------------------------------
bool EntryParserBankStripe::invoicesForFees() const
{
    return false;
}
//----------------------------------------------------------
QString EntryParserBankStripe::name() const
{
    return "Stripe " + currency();
}
//----------------------------------------------------------
QString EntryParserBankStripe::journal() const
{
    return "STRIPE " + currency();

}
//----------------------------------------------------------
