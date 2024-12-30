#include <QTextStream>

#include "model/SettingManager.h"
#include "model/orderimporters/AddressesServiceCustomer.h"

#include "ServiceSalesModel.h"

//----------------------------------------------------------
const int ServiceSalesModel::IND_COL_DATE = 0;
const int ServiceSalesModel::IND_COL_REFERENCE = 1;
const int ServiceSalesModel::IND_COL_AMOUNT = 2;
const int ServiceSalesModel::IND_COL_UNIT = 3;
const int ServiceSalesModel::IND_COL_TITLE = 4;
const int ServiceSalesModel::IND_COL_CURRENCY = 5;
const int ServiceSalesModel::IND_COL_CUSTOMER_ID = 6;
const int ServiceSalesModel::IND_COL_ADDRESS_ID = 7;
const int ServiceSalesModel::N_COLS = ServiceSalesModel::IND_COL_ADDRESS_ID;
//----------------------------------------------------------
ServiceSalesModel::ServiceSalesModel(QObject *parent)
    : QAbstractTableModel(parent), UpdateToCustomer()
{
    m_year = -1;
}
//----------------------------------------------------------
ServiceSalesModel *ServiceSalesModel::instance()
{
    static ServiceSalesModel instance;
    return &instance;
}
//----------------------------------------------------------
QList<QList<QVariant>> ServiceSalesModel::loadListOfVariantList(
        const QString &filePath)
{
    QList<QList<QVariant>> listOfVariantList;
    if (QFile::exists(filePath)) {
        QFile file(filePath);
        if (file.open(QFile::ReadOnly)) {
            QTextStream stream(&file);
            auto data = stream.readAll();
            if (!data.trimmed().isEmpty()) {
                QStringList lines = data.split("\n");
                //beginInsertRows(QModelIndex(), 0, lines.size()-1);
                //int i=0;
                //QList<int> indexesTEMP = {2, 0, 0, 0};
                for (auto itLine = lines.begin();
                     itLine != lines.end(); ++itLine) {
                    QStringList elements = itLine->split(";");
                    QList<QVariant> variantList;
                    for (auto itEl = elements.begin();
                         itEl != elements.end(); ++itEl) {
                        variantList << *itEl;
                    }
                    elements[IND_COL_AMOUNT] = elements[IND_COL_AMOUNT].toDouble();
                    elements[IND_COL_UNIT] = elements[IND_COL_UNIT].toInt();
                    //if (variantList.size() < ServiceSalesModel::N_COLS + 1) {
                    //variantList << AddressesServiceCustomer::instance()
                                       //->getAddress(indexesTEMP[i]).internalId();
                    //}
                    listOfVariantList << variantList;
                    //++i;
                }
                //endInsertRows();
            }
            file.close();
        }
    }
    return listOfVariantList;
}
//----------------------------------------------------------
QString ServiceSalesModel::getFilePath(int year) const
{
    auto dirReport = SettingManager::instance()->reportDirectory();
    QString fileName = QString::number(year) + ".csv";
    QString filePath = dirReport.filePath(fileName);
    return filePath;
}
//----------------------------------------------------------
void ServiceSalesModel::load(int year)
{
    m_year = year;
    _clear();
    auto dirReport = SettingManager::instance()->reportDirectory();
    QString filePath = getFilePath(year);
    if (QFile::exists(filePath)) {
        QFile file(filePath);
        if (file.open(QFile::ReadOnly)) {
            QTextStream stream(&file);
            auto data = stream.readAll();
            if (!data.trimmed().isEmpty()) {
                QStringList lines = data.split("\n");
                beginInsertRows(QModelIndex(), 0, lines.size()-1);
                //int i=0;
                //QList<int> indexesTEMP = {2, 0, 0, 0};
                for (auto itLine = lines.begin();
                     itLine != lines.end(); ++itLine) {
                    QStringList elements = itLine->split(";");
                    QList<QVariant> variantList;
                    for (auto itEl = elements.begin();
                         itEl != elements.end(); ++itEl) {
                        variantList << *itEl;
                    }
                    elements[IND_COL_AMOUNT] = elements[IND_COL_AMOUNT].toDouble();
                    elements[IND_COL_UNIT] = elements[IND_COL_UNIT].toInt();
                    //if (variantList.size() < ServiceSalesModel::N_COLS + 1) {
                    //variantList << AddressesServiceCustomer::instance()
                                       //->getAddress(indexesTEMP[i]).internalId();
                    //}
                    //variantList.insert(3, "Commission sur publicité");
                    //variantList.insert(3, 1);
                    m_listOfVariantList << variantList;
                    //++i;
                }
                endInsertRows();
            }
            file.close();
            //_saveInSettings();
        }
    }
}
//----------------------------------------------------------
void ServiceSalesModel::addSale(
        const QDate &date,
        const QString &label,
        double amountUnit,
        int units,
        const QString &title,
        const QString &currency,
        const QString &customerId,
        const QString &addressId)
{
    beginInsertRows(QModelIndex(), 0, 0);
    QList<QVariant> line = {date
                            , label
                            , amountUnit
                            , units
                            , title
                            , currency
                            , customerId
                            , addressId};
    m_listOfVariantList.insert(0, line);
    _saveInSettings();
    endInsertRows();
}
//----------------------------------------------------------
void ServiceSalesModel::remove(const QModelIndex &index)
{
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    m_listOfVariantList.removeAt(index.row());
    _saveInSettings();
    endRemoveRows();
}
//----------------------------------------------------------
void ServiceSalesModel::onCustomerSelectedChanged(
        const QString &)
{
    m_year = -1;
    _clear();
}
//----------------------------------------------------------
QString ServiceSalesModel::uniqueId() const
{
    return "SelfInvoiceSales";
}
//----------------------------------------------------------
void ServiceSalesModel::_saveInSettings()
{
    if (m_year > 0) {
        auto dirReport = SettingManager::instance()->reportDirectory();
        QString filePath = getFilePath(m_year);
        QFile file(filePath);
        if (file.open(QFile::WriteOnly)){
            QStringList lines;
            for (auto itLine = m_listOfVariantList.begin();
                 itLine != m_listOfVariantList.end(); ++itLine) {
                QStringList elements;
                for (auto itEl = itLine->begin();
                     itEl != itLine->end(); ++itEl) {
                    elements << itEl->toString().replace(";", ":");
                }
                lines << elements.join(";");
            }
            QTextStream stream(&file);
            stream << lines.join("\n");
            file.close();
        }
    }
}
//----------------------------------------------------------
void ServiceSalesModel::_clear()
{
    if (m_listOfVariantList.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_listOfVariantList.size()-1);
        m_listOfVariantList.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------
QVariant ServiceSalesModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal
            && role == Qt::DisplayRole) {
        static QStringList values = {
            tr("Date"),
            tr("Référence"),
            tr("Montant unitaire"),
            tr("Unité"),
            tr("Titre"),
            tr("Devise"),
            tr("Client")};
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int ServiceSalesModel::rowCount(const QModelIndex &) const
{
    return m_listOfVariantList.size();
}
//----------------------------------------------------------
int ServiceSalesModel::columnCount(const QModelIndex &) const
{
    return N_COLS;
}
//----------------------------------------------------------
QVariant ServiceSalesModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_listOfVariantList[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
bool ServiceSalesModel::setData(
        const QModelIndex &index,
        const QVariant &value,
        int role)
{
    if (role == Qt::EditRole && data(index, role) != value) {
        m_listOfVariantList[index.row()][index.column()] = value;
        _saveInSettings();
        return true;
    }
    return false;
}
//----------------------------------------------------------
Qt::ItemFlags ServiceSalesModel::flags(const QModelIndex &index) const
{
    if (index.column() < 5){
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//----------------------------------------------------------
void ServiceSalesModel::sort(int column, Qt::SortOrder order)
{
    if (order == Qt::AscendingOrder) {
        std::sort(m_listOfVariantList.begin(), m_listOfVariantList.end(),
                  [column](const QList<QVariant> &v1, const QList<QVariant> &v2){
            return v1[column] < v2[column];
        });
    } else {
        std::sort(m_listOfVariantList.begin(), m_listOfVariantList.end(),
                  [column](const QList<QVariant> &v1, const QList<QVariant> &v2){
            return v1[column] > v2[column];
        });
    }
    emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1),
                     QVector<int>() << Qt::DisplayRole);
}
//----------------------------------------------------------
