#include "EntryParserBankTable.h"

//----------------------------------------------------------
EntryParserBankTable::EntryParserBankTable(
        QObject *parent) : AbstractEntryParserTable(parent)
{
}
//----------------------------------------------------------
QString EntryParserBankTable::name() const
{
    return bankParser()->name();
}
//----------------------------------------------------------
void EntryParserBankTable::fillEntries(
        AccountingEntries &entries, int year)
{
    entries = bankParser()->entries(year);
}
//----------------------------------------------------------
QString EntryParserBankTable::journal() const
{
    return bankParser()->journal();
}
//----------------------------------------------------------
QStringList EntryParserBankTable::fileFilters() const
{
    return bankParser()->fileFilters();
}
//----------------------------------------------------------
QString EntryParserBankTable::fileFiltersDialog() const
{
    QStringList filters = fileFilters();
    QString filter = "CSV (" + filters.join(" ") + ")";
    return filter;
}
//----------------------------------------------------------
void EntryParserBankTable::addFilePath(
        int year,
        int month,
        const QString &filePath,
        const QString &filePathDisplay,
        bool copy)
{
    auto entrySets = bankParser()->addFilePath(
                year, month, filePath, filePathDisplay, copy);
    for(auto entrySet : entrySets) {
        addEntrySet(entrySet);
    }
}
//----------------------------------------------------------
QDate EntryParserBankTable::guessDate(
        const QString &filePath) const
{
    return bankParser()->guessDate(filePath);
}
//----------------------------------------------------------
