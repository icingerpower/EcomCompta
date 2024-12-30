#include "../common/countries/CountryManager.h"

#include "EntryParserImportations.h"

#include "EntryParserImportationsTable.h"

//----------------------------------------------------------
QString EntryParserImportationsTable::COL_VAT_CONV = QObject::tr("TVA converti");
QString EntryParserImportationsTable::COL_VAT_ORIG = QObject::tr("TVA original");
QString EntryParserImportationsTable::COL_VAT_COUNTRY = QObject::tr("Pays TVA");
QString EntryParserImportationsTable::COL_VAT_CURRENCY = QObject::tr("Monnaie TVA");
QString EntryParserImportationsTable::COL_COUNTRY_CODE_FROM = QObject::tr("Country code from");
QString EntryParserImportationsTable::COL_COUNTRY_CODE_TO = QObject::tr("Country code to");
//----------------------------------------------------------
EntryParserImportationsTable::EntryParserImportationsTable(
        QObject *object)
    : AbstractEntryParserTable(object)
{
}
//----------------------------------------------------------
EntryParserImportationsTable::~EntryParserImportationsTable()
{
}
//----------------------------------------------------------
QString EntryParserImportationsTable::name() const
{
    return m_entryParserPurchases->name();
}
//----------------------------------------------------------
QString EntryParserImportationsTable::journal() const
{
    return m_entryParserPurchases->journal();
}
//----------------------------------------------------------
void EntryParserImportationsTable::fillEntries(
        AccountingEntries &entries, int year)
{
    entries = m_entryParserPurchases->entries(year);
}
//----------------------------------------------------------
void EntryParserImportationsTable::addInvoice(
        ImportInvoiceInfo &infos)
{
    auto entrySet = m_entryParserPurchases->addInvoice(
                infos);
    addEntrySet(entrySet);
}
//----------------------------------------------------------
void EntryParserImportationsTable::removeInvoices(
        const QModelIndexList &indexes)
{
    QSet<int> rows;
    for (auto index : indexes) {
        rows << index.row();
    }
    QList<int> rowsSorted = rows.toList();
    std::sort(rowsSorted.begin(), rowsSorted.end());
    for (auto rowIt = rowsSorted.rbegin();
         rowIt != rowsSorted.rend(); ++rowIt) {
    }
}
//----------------------------------------------------------
void EntryParserImportationsTable::removeEntry(int row)
{
    auto entrySet = _entrySets()->value(row);
    m_entryParserPurchases->removeInvoice(
                entrySet->year(),
                entrySet->month(),
                entrySet->id());
    AbstractEntryParserTable::removeEntry(row);
}
//----------------------------------------------------------
Qt::ItemFlags EntryParserImportationsTable::flags(
        const QModelIndex &index) const
{
    QString colName = headerData(index.column(), Qt::Horizontal).toString();
    if (_editableColNames()->contains(colName)) {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
    }
    return AbstractEntryParserTable::flags(index);
}
//----------------------------------------------------------
bool EntryParserImportationsTable::setData(
        const QModelIndex &index,
        const QVariant &value,
        int role)
{
    if (role == Qt::EditRole
            && value != data(index)) {
         QString colName = headerData(index.column(), Qt::Horizontal).toString();
         if (_editableColNames()->contains(colName)) {
             auto entrySet = _entrySets()->value(index.row());
             QString fileName = entrySet->fileRelWorkingDir();
             ImportInvoiceInfo infos = EntryParserImportations::invoiceInfoFromFileName(
                         fileName);
             if (colName == COL_VAT_ORIG) {
                 infos.amountVat = value.toDouble();
             } else if (colName == COL_VAT_COUNTRY) {
                 infos.vatCountryName = value.toString();
             } else if (colName == COL_VAT_CURRENCY) {
                 infos.currencyVat = value.toString();
             } else if (colName == COL_DATE) {
                 infos.date = value.toDate();
             } else if (colName == COL_LABEL) {
                 infos.label = value.toString();
             } else if (colName == COL_ACCOUNT_1) {
                 infos.accountOrig6 = value.toString();
             } else if (colName == COL_ACCOUNT_2) {
                 //infos.account = value.toString();
             } else if (colName == COL_CURRENCY_ORIG) {
                 infos.currency = value.toString();
             } else if (colName == COL_AMOUNT_ORIG) {
                 infos.amount = value.toDouble();
             }
             QString invoiceId = entrySet->id();
             auto newEntrySet = EntryParserImportations().updateInvoice(infos);
             auto currentEntrySet = entrySet;
             m_entrySetPositions.remove(currentEntrySet.data());
             m_entrySetPositions[newEntrySet.data()] = index.row();
             m_entryById[newEntrySet->id()] = newEntrySet;
             _setEntrySet(index.row(), newEntrySet);
             return true;
         }
    }
    return false;
}
//----------------------------------------------------------
QList<AbstractEntryParserTable::ColInfo> *EntryParserImportationsTable::colInfos() const
{
    static QList<ColInfo> infos
            = *AbstractEntryParserTable::colInfos()
            << QList<ColInfo>(
    {{COL_VAT_CONV, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
          double vat = 0.;
          if (entrySet->size() > 2) {
              double amountOrig = entrySet->amountOrig();
              auto lastEntry = entrySet->entries().last();
              double sum = lastEntry.debitOrigDouble() + lastEntry.creditOrigDouble();
              vat = amountOrig/qAbs(entrySet->amountOrig()) * sum;
          }
          return vat;
      }}
     ,{COL_VAT_ORIG, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
          double vat = 0.;
          if (entrySet->size() > 2) {
              double amountOrig = entrySet->amountOrig();
              auto lastEntry = entrySet->entries().last();
              double sum = lastEntry.debitOrigDouble() + lastEntry.creditOrigDouble();
              vat = amountOrig/qAbs(entrySet->amountOrig()) * sum;
          } // TODO fix this, I should reverse the rate use for convertion I think
          return vat;
       }}
     ,{COL_VAT_COUNTRY, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
           QString country;
           QString fileName = entrySet->fileRelWorkingDir();
           if (fileName.contains("-TVA-")) {
               QString countryCode = fileName.split("-TVA-")[0].split("__").last();
               country = CountryManager::instance()->countryName(countryCode);
           }
           return country;
       }}
     ,{COL_VAT_CURRENCY, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
           if (entrySet->size() > 2) {
               return entrySet->entries()[0].currency();
           }
           return CustomerManager::instance()->getSelectedCustomerCurrency();
       }}
     ,{COL_COUNTRY_CODE_FROM, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
           QString fileName = entrySet->fileRelWorkingDir();
           QStringList elements = fileName.split("__");
           while (elements.first().size() != 2
           || elements.first() != "GB-NIR") {
               elements.takeFirst();
           }
           return elements.first();
       }}
     ,{COL_COUNTRY_CODE_TO, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
           QString fileName = entrySet->fileRelWorkingDir();
           QStringList elements = fileName.split("__");
           while (elements.first().size() != 2
           || elements.first() != "GB-NIR") {
               elements.takeFirst();
           }
           return elements[1];
       }}
                   });
    return &infos;
}
//----------------------------------------------------------
QStringList *EntryParserImportationsTable::_editableColNames() const
{
    static QStringList editableColNames = {
        COL_VAT_ORIG
        , COL_VAT_COUNTRY
        , COL_VAT_CURRENCY
        , COL_DATE
        , COL_LABEL
        , COL_ACCOUNT_1
        , COL_ACCOUNT_2
        , COL_CURRENCY_ORIG
        , COL_AMOUNT_ORIG
        , COL_COUNTRY_CODE_FROM
        , COL_COUNTRY_CODE_TO
    };
    return &editableColNames;
}
//----------------------------------------------------------
