#include "../common/utils/CsvReader.h"

#include "EntryParserBankPaypal.h"

//----------------------------------------------------------
EntryParserBankPaypal::EntryParserBankPaypal()
    : EntryParserBank()
{
}
//----------------------------------------------------------
EntryParserBankPaypal::~EntryParserBankPaypal()
{
}
//----------------------------------------------------------
QList<QVariantList> EntryParserBankPaypal::loadValues(
        const QString &absFileName) const
{
    QString sep = ",";
    QString guill = "\"";
    CsvReader reader(absFileName, sep, guill);
    reader.readAll();
    const DataFromCsv *dataRode = reader.dataRode();
    int indDate = dataRode->header.pos("Date");
    int indName = dataRode->header.pos({"Nom", "Name"});
    int indName2 = dataRode->header.pos({"Nom de la banque", "Bank name"});
    int indComment = dataRode->header.pos("Description");
    int indAmount = dataRode->header.pos("Brut");
    int indFees = dataRode->header.pos({"Frais", "Fees"});
    int indCurrency = dataRode->header.pos({"Devise", "Currency"});
    QList<QVariantList> lines;
    int idRow = 1;
    for (auto elements : dataRode->lines) {
        QString currency = elements[indCurrency];
        if (currency == this->currency()) {
            QVariantList variants;
            variants << idRow; /// 0
            QDate date = QDateTime::fromString(
                        elements[indDate], "dd/MM/yyyy")
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
            variants << title; /// 2
            double amount = elements[indAmount].replace(",", ".").toDouble();
            if (qAbs(amount) > 0.001) {
                variants << elements[indAmount].replace(",", ".").toDouble(); /// 3
                variants << currency; /// 4
                variants << -elements[indFees].replace(",", ".").toDouble(); /// 5 fees
                lines << variants;
            }
        }
        ++idRow;
    }
    return lines;
}
//----------------------------------------------------------
bool EntryParserBankPaypal::invoicesForFees() const
{
    return false;
}
//----------------------------------------------------------
QString EntryParserBankPaypal::name() const
{
    return "Paypal" + currency();
}
//----------------------------------------------------------
QString EntryParserBankPaypal::journal() const
{
    return QObject::tr("BQ PAYPAL ") + currency();
}
//----------------------------------------------------------
