#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qspinbox.h>

#include "../common/countries/CountryManager.h"

#include "EntryParserPurchasesTable.h"
#include "model/SettingManager.h"
#include "model/CustomerManager.h"
#include "model/bookkeeping/entries/parsers/EntryParserPurchases.h"
#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"

//----------------------------------------------------------
QString EntryParserPurchasesTable::COL_VAT_CONV = QObject::tr("TVA converti");
QString EntryParserPurchasesTable::COL_VAT_ORIG = QObject::tr("TVA original");
QString EntryParserPurchasesTable::COL_VAT_COUNTRY = QObject::tr("Pays TVA");
QString EntryParserPurchasesTable::COL_VAT_CURRENCY = QObject::tr("Monnaie TVA");
//----------------------------------------------------------
EntryParserPurchasesTable::EntryParserPurchasesTable(QObject *object)
    : AbstractEntryParserTable(object)
{
    m_entryParserPurchases = new EntryParserPurchases();
}
//----------------------------------------------------------
EntryParserPurchasesTable::~EntryParserPurchasesTable()
{
    delete m_entryParserPurchases;
}
//----------------------------------------------------------
QString EntryParserPurchasesTable::name() const
{
    return tr("Achats");
}
//----------------------------------------------------------
QString EntryParserPurchasesTable::journal() const
{
    return m_entryParserPurchases->journal();
}
//----------------------------------------------------------
void EntryParserPurchasesTable::fillEntries(AccountingEntries &entries, int year)
{
    entries = m_entryParserPurchases->entries(year);
}
//----------------------------------------------------------
void EntryParserPurchasesTable::addInvoice(PurchaseInvoiceInfo &infos)
{
    auto entrySet = m_entryParserPurchases->addInvoice(
                infos);
    addEntrySet(entrySet);
}
//----------------------------------------------------------
void EntryParserPurchasesTable::removeInvoices(
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
void EntryParserPurchasesTable::removeEntry(int row)
{
    auto entrySet = _entrySets()->value(row);
    m_entryParserPurchases->removeInvoice(
                entrySet->year(),
                entrySet->month(),
                entrySet->id());
    AbstractEntryParserTable::removeEntry(row);
}
//----------------------------------------------------------
Qt::ItemFlags EntryParserPurchasesTable::flags(const QModelIndex &index) const
{
    // TODO we need to create a system so we can edit more fields, for instance VAT for invoices
    QString colName = headerData(index.column(), Qt::Horizontal).toString();
    if (_editableColNames()->contains(colName)) {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
    }
    return AbstractEntryParserTable::flags(index);
}
//----------------------------------------------------------
QStringList *EntryParserPurchasesTable::_editableColNames() const
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
    };
    return &editableColNames;
}
//----------------------------------------------------------
bool EntryParserPurchasesTable::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole
            && value != data(index)) {
         QString colName = headerData(index.column(), Qt::Horizontal).toString();
         if (_editableColNames()->contains(colName)) {
             auto entrySet = _entrySets()->value(index.row());
             QString fileName = entrySet->fileRelWorkingDir();
             PurchaseInvoiceInfo infos = EntryParserPurchases::invoiceInfoFromFileName(
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
                 infos.accountSupplier = value.toString();
             } else if (colName == COL_ACCOUNT_2) {
                 infos.account = value.toString();
             } else if (colName == COL_CURRENCY_ORIG) {
                 infos.currency = value.toString();
             } else if (colName == COL_AMOUNT_ORIG) {
                 infos.amount = value.toDouble();
             }
             //QString invoiceId = entrySet->id();
             auto newEntrySet = EntryParserPurchases().updateInvoice(infos);
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
QList<AbstractEntryParserTable::ColInfo> *EntryParserPurchasesTable::colInfos() const
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
                   });
    return &infos;
}
//----------------------------------------------------------
//----------------------------------------------------------
TableEntryPurchasesDelegate::TableEntryPurchasesDelegate(
        const EntryParserPurchasesTable *model, QObject *parent)
    : QStyledItemDelegate(parent)
{
    m_model = model;
}
//----------------------------------------------------------
//----------------------------------------------------------
QWidget *TableEntryPurchasesDelegate::createEditor(
        QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QWidget *widget = nullptr;
    QString colName = m_model->headerData(
                index.column(), Qt::Horizontal).toString();
    if (colName == EntryParserPurchasesTable::COL_VAT_CURRENCY
            || colName == EntryParserPurchasesTable::COL_CURRENCY_ORIG) {
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->addItems(SettingManager::instance()->currencies());
        widget = comboBox;
    } else if (colName == EntryParserPurchasesTable::COL_AMOUNT_ORIG
               || colName == EntryParserPurchasesTable::COL_VAT_ORIG) {
        QDoubleSpinBox *spinBox = new QDoubleSpinBox(parent);
        spinBox->setRange(-999999, 999999);
        spinBox->setDecimals(2);
        widget = spinBox;
    } else if (colName == EntryParserPurchasesTable::COL_VAT_COUNTRY) {
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->addItems(*CountryManager::instance()->countriesNamesUEfrom2020());
        widget = comboBox;
    } else {
        widget =  QStyledItemDelegate::createEditor(
                    parent, option, index);
    }
    return widget;
}
//----------------------------------------------------------
//----------------------------------------------------------
void TableEntryPurchasesDelegate::setEditorData(
        QWidget *editor, const QModelIndex &index) const
{
    QString colName = m_model->headerData(
                index.column(), Qt::Horizontal).toString();
    if (colName == EntryParserPurchasesTable::COL_VAT_CURRENCY
            || colName == EntryParserPurchasesTable::COL_CURRENCY_ORIG
            || colName == EntryParserPurchasesTable::COL_VAT_COUNTRY) {
        QComboBox *comboBox = static_cast<QComboBox *>(editor);
        QString currentValue = index.data().toString();
        comboBox->setCurrentText(currentValue);
    } else if (colName == EntryParserPurchasesTable::COL_AMOUNT_ORIG
               || colName == EntryParserPurchasesTable::COL_VAT_ORIG) {
        QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox *>(editor);
        spinBox->setValue(index.data().toDouble());
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
