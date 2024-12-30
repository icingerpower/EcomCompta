#include "ManagerSaleTypes.h"

//----------------------------------------------------------
QString ManagerSaleTypes::SALE_PRODUCTS = QObject::tr("Produits");
QString ManagerSaleTypes::SALE_SERVICES = QObject::tr("Services");
QString ManagerSaleTypes::SALE_PAYMENT_FASCILITOR = QObject::tr("Fascilité de paiement");
//----------------------------------------------------------
ManagerSaleTypes::ManagerSaleTypes(QObject *parent)
    : QAbstractListModel(parent)
{
}
//----------------------------------------------------------
ManagerSaleTypes *ManagerSaleTypes::instance()
{
    static ManagerSaleTypes instance;
    return &instance;
}
//----------------------------------------------------------
ManagerSaleTypes::~ManagerSaleTypes()
{
}
//----------------------------------------------------------
int ManagerSaleTypes::rowCount(
        const QModelIndex &) const
{
    return m_saleTypes.size();
}
//----------------------------------------------------------
QVariant ManagerSaleTypes::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        return m_saleTypes[index.row()];
    }
    return QVariant();
}
//----------------------------------------------------------
