#include <QtWidgets/qcombobox.h>

#include "model/SettingManager.h"
#include "model/CustomerManager.h"
#include "EntryParserAmazonPaymentsTable.h"
#include "EntryParserAmazonPayments.h"

QString EntryParserAmazonPaymentsTable::COL_AMOUNT_PAID_CONV = QObject::tr("Montant reçu banque");
QString EntryParserAmazonPaymentsTable::COL_AMOUNT_PAID_CONV_CURRENCY = QObject::tr("Monnaie montant reçu");
QString EntryParserAmazonPaymentsTable::COL_AMOUNT_SALES_ORIG = QObject::tr("Ventes");
QString EntryParserAmazonPaymentsTable::COL_AMOUNT_FEES_ORIG = QObject::tr("Frais");
QString EntryParserAmazonPaymentsTable::COL_AMOUNT_RESERVE_PREVIOUS_ORIG = QObject::tr("Balance précédente");
QString EntryParserAmazonPaymentsTable::COL_AMOUNT_RESERVE_CURRENT_ORIG = QObject::tr("Balance actuelle");
//----------------------------------------------------------
EntryParserAmazonPaymentsTable::EntryParserAmazonPaymentsTable(
        QObject *parent) : EntryParserOrdersTable(parent)
{
    m_entryParser = new EntryParserAmazonPayments();
}
//----------------------------------------------------------
EntryParserAmazonPaymentsTable::~EntryParserAmazonPaymentsTable()
{
    delete m_entryParser;
}
//----------------------------------------------------------
void EntryParserAmazonPaymentsTable::onCustomerSelectedChanged(
        const QString &customerId)
{
    m_entryParser->loadReplacedInfos();
}
//----------------------------------------------------------
QString EntryParserAmazonPaymentsTable::journal() const
{
    return m_entryParser->journal();
}
//----------------------------------------------------------
void EntryParserAmazonPaymentsTable::fillEntries(
        AccountingEntries &entries, int year)
{
    entries = m_entryParser->entries(year);
}
//----------------------------------------------------------
EntryParserOrders *EntryParserAmazonPaymentsTable::saleParser() const
{
    return m_entryParser;
}
//----------------------------------------------------------
QStringList *EntryParserAmazonPaymentsTable::_editableColNames() const
{
    static QStringList editableColNames = {
        COL_AMOUNT_PAID_CONV
        , COL_AMOUNT_PAID_CONV_CURRENCY
        , COL_AMOUNT_SALES_ORIG
        , COL_AMOUNT_FEES_ORIG
        , COL_AMOUNT_RESERVE_PREVIOUS_ORIG
        , COL_AMOUNT_RESERVE_CURRENT_ORIG
    };
    return &editableColNames;
}
//----------------------------------------------------------
Qt::ItemFlags EntryParserAmazonPaymentsTable::flags(
        const QModelIndex &index) const
{
    QString colName = headerData(index.column(), Qt::Horizontal).toString();
    if (_editableColNames()->contains(colName)) {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
    }
    return AbstractEntryParserTable::flags(index);
}
//----------------------------------------------------------
bool EntryParserAmazonPaymentsTable::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole
            && value != data(index)) {
        static int indEntrySetId = [this]() -> int {
            auto colInfos = this->AbstractEntryParserTable::colInfos();
            int id = 0;
            for (auto colInfo : *colInfos) {
                if (colInfo.name == AbstractEntryParserTable::COL_ID) {
                    return id;
                }
                ++id;
            }
            return 0;
        }();
        auto indexEntryId = this->index(index.row(), indEntrySetId);
        QString entrySetId = this->data(indexEntryId).toString();
        QString colName = headerData(index.column(), Qt::Horizontal).toString();
        m_entryParser->replaceInfo(
                    entrySet(index.row()),
                    colName,
                    value);
        return true;
    }
    return false;
}
//----------------------------------------------------------
QList<AbstractEntryParserTable::ColInfo>
*EntryParserAmazonPaymentsTable::colInfos() const
{
    static QList<ColInfo> infos
            = *AbstractEntryParserTable::colInfos()
            << QList<ColInfo>(
    {{COL_AMOUNT_PAID_CONV, [](const AbstractEntryParserTable *table, const AccountingEntrySet *entrySet) -> QVariant{
          QString entrySetId = entrySet->id();
          const EntryParserAmazonPaymentsTable *tableConv
          = static_cast<const EntryParserAmazonPaymentsTable *>(table);
          if (tableConv->m_entryParser->isReplacedInfo(entrySetId, COL_AMOUNT_PAID_CONV)) {
              return tableConv->m_entryParser->replacedInfo(entrySetId, COL_AMOUNT_PAID_CONV);
          }
          double paid = 0.;
          if (entrySet->size() > 0) {
              paid = entrySet->amountConv();
          }
          return paid;
      }}
     ,{COL_AMOUNT_PAID_CONV_CURRENCY, [](const AbstractEntryParserTable *table, const AccountingEntrySet *entrySet) -> QVariant{
          QString entrySetId = entrySet->id();
          const EntryParserAmazonPaymentsTable *tableConv
          = static_cast<const EntryParserAmazonPaymentsTable *>(table);
          if (tableConv->m_entryParser->isReplacedInfo(entrySetId, COL_AMOUNT_PAID_CONV_CURRENCY)) {
              return tableConv->m_entryParser->replacedInfo(entrySetId, COL_AMOUNT_PAID_CONV_CURRENCY);
          }
          if (entrySet->size() > 1) {
              return entrySet->entries()[0].currency();
          }
          return CustomerManager::instance()->getSelectedCustomerCurrency();
       }}
     ,{COL_AMOUNT_SALES_ORIG, [](const AbstractEntryParserTable *table, const AccountingEntrySet *entrySet) -> QVariant{
          QString entrySetId = entrySet->id();
          const EntryParserAmazonPaymentsTable *tableConv
          = static_cast<const EntryParserAmazonPaymentsTable *>(table);
          if (tableConv->m_entryParser->isReplacedInfo(entrySetId, COL_AMOUNT_SALES_ORIG)) {
              return tableConv->m_entryParser->replacedInfo(entrySetId, COL_AMOUNT_SALES_ORIG);
          }
          for (auto entry : entrySet->entries()) {
              QString account = entry.account();
              if (account.startsWith("4")
              || account.startsWith("7")) { // TODO will create bug if user use other kind of account
                  return entry.debitConvDouble() + entry.creditConvDouble();
              }
          }
          return 0.;
       }}
     ,{COL_AMOUNT_FEES_ORIG, [](const AbstractEntryParserTable *table, const AccountingEntrySet *entrySet) -> QVariant{
          QString entrySetId = entrySet->id();
          const EntryParserAmazonPaymentsTable *tableConv
          = static_cast<const EntryParserAmazonPaymentsTable *>(table);
          if (tableConv->m_entryParser->isReplacedInfo(entrySetId, COL_AMOUNT_FEES_ORIG)) {
              return tableConv->m_entryParser->replacedInfo(entrySetId, COL_AMOUNT_FEES_ORIG);
          }
          for (auto entry : entrySet->entries()) {
              QString account = entry.account();
              if (account.startsWith("6")) { // TODO will create bug if user use other kind of account
                  return entry.debitConvDouble() + entry.creditConvDouble();
              }
          }
          return 0.;
       }}
     ,{COL_AMOUNT_RESERVE_PREVIOUS_ORIG, [](const AbstractEntryParserTable *table, const AccountingEntrySet *entrySet) -> QVariant{
          QString entrySetId = entrySet->id();
          const EntryParserAmazonPaymentsTable *tableConv
          = static_cast<const EntryParserAmazonPaymentsTable *>(table);
          if (tableConv->m_entryParser->isReplacedInfo(entrySetId, COL_AMOUNT_RESERVE_PREVIOUS_ORIG)) {
              return tableConv->m_entryParser->replacedInfo(entrySetId, COL_AMOUNT_RESERVE_PREVIOUS_ORIG);
          }
          for (auto entry : entrySet->entries()) {
              QString account = entry.account();
              if (account.startsWith("2") && !entry.creditOrig().isEmpty()) {
                  return entry.creditConvDouble();
              }
          }
          return 0.;
       }}
     ,{COL_AMOUNT_RESERVE_CURRENT_ORIG, [](const AbstractEntryParserTable *table, const AccountingEntrySet *entrySet) -> QVariant{
          QString entrySetId = entrySet->id();
          const EntryParserAmazonPaymentsTable *tableConv
          = static_cast<const EntryParserAmazonPaymentsTable *>(table);
          if (tableConv->m_entryParser->isReplacedInfo(entrySetId, COL_AMOUNT_RESERVE_CURRENT_ORIG)) {
              return tableConv->m_entryParser->replacedInfo(entrySetId, COL_AMOUNT_RESERVE_CURRENT_ORIG);
          }
          for (auto entry : entrySet->entries()) {
              QString account = entry.account();
              if (account.startsWith("2") && !entry.debitOrig().isEmpty()) {
                  return entry.debitConvDouble();
              }
          }
          return 0.;
       }}
                   });
    return &infos;
}
//----------------------------------------------------------
//----------------------------------------------------------
EntryParserAmazonPaymentsDelegate::EntryParserAmazonPaymentsDelegate(
        const EntryParserAmazonPaymentsTable *model, QObject *parent)
    : QStyledItemDelegate(parent)
{
    m_model = model;
}
//----------------------------------------------------------
//----------------------------------------------------------
QWidget *EntryParserAmazonPaymentsDelegate::createEditor(
        QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QWidget *widget = nullptr;
    QString colName = m_model->headerData(
                index.column(), Qt::Horizontal).toString();
    if (colName == EntryParserAmazonPaymentsTable::COL_AMOUNT_PAID_CONV_CURRENCY) {
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->addItems(SettingManager::instance()->currencies());
        widget = comboBox;
    } else {
        widget =  QStyledItemDelegate::createEditor(
                    parent, option, index);
    }
    return widget;
}
//----------------------------------------------------------
//----------------------------------------------------------
void EntryParserAmazonPaymentsDelegate::setEditorData(
        QWidget *editor, const QModelIndex &index) const
{
    QString colName = m_model->headerData(
                index.column(), Qt::Horizontal).toString();
    if (colName == EntryParserAmazonPaymentsTable::COL_AMOUNT_PAID_CONV_CURRENCY) {
        QComboBox *comboBox = static_cast<QComboBox *>(editor);
        QString currentValue = index.data().toString();
        comboBox->setCurrentText(currentValue);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
