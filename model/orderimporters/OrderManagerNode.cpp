#include "OrderManagerNode.h"
#include "OrderManager.h"
#include "Order.h"

//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
OrderManagerNode::OrderManagerNode(
        const OrderManager *orderManager, OrderManagerNode *parent)
{
    m_parent = parent;
    m_orderManager = orderManager;
    m_row = 0;
    if (parent != nullptr) {
        m_row = parent->m_children.size();
        parent->m_children << this;
    }
}
//----------------------------------------------------------
OrderManagerNode::~OrderManagerNode()
{
    qDeleteAll(m_children);
}
//----------------------------------------------------------
int OrderManagerNode::rowCount() const
{
    return m_children.size();
}
//----------------------------------------------------------
QVariant OrderManagerNode::value() const
{
    return m_value;
}

void OrderManagerNode::setValue(const QVariant &value)
{
    m_value = value;
}
//----------------------------------------------------------
OrderManagerNode *OrderManagerNode::child(int row) const
{
    return m_children[row];
}
//----------------------------------------------------------
int OrderManagerNode::row() const
{
    return m_row;
}
//----------------------------------------------------------
void OrderManagerNode::setRow(int row)
{
    m_row = row;
}
//----------------------------------------------------------
OrderManagerNode *OrderManagerNode::parent() const
{
    return m_parent;
}
//----------------------------------------------------------
OrderManagerNodeRoot::OrderManagerNodeRoot(
        const OrderManager *orderManager, OrderManagerNode *parent)
    : OrderManagerNode(orderManager, parent)
{
}
//----------------------------------------------------------
QVariant OrderManagerNodeRoot::data(int, int) const
{
    return QVariant();
}
//----------------------------------------------------------
//----------------------------------------------------------
OrderManagerNodeImporter::OrderManagerNodeImporter(
        const QString &name,
        const OrderManager *orderManager,
        OrderManagerNode *parent)
    : OrderManagerNode(orderManager, parent)
{
    m_value = name;
}
//----------------------------------------------------------
//----------------------------------------------------------
QVariant OrderManagerNodeImporter::data(int column, int role) const
{
    QVariant value;
    if (column == 0 && role == Qt::DisplayRole) {
        value = m_value;
    }
    return value;
}
//----------------------------------------------------------
//----------------------------------------------------------
OrderManagerNodeYear::OrderManagerNodeYear(
        int year, const OrderManager *orderManager, OrderManagerNode *parent)
    : OrderManagerNode(orderManager, parent)
{
    m_value = year;
}
//----------------------------------------------------------
QVariant OrderManagerNodeYear::data(int column, int role) const
{
    QVariant value;
    if (column == 0 && role == Qt::DisplayRole) {
        value = m_value;
    }
    return value;
}
//----------------------------------------------------------
OrderManagerNodeOrder::OrderManagerNodeOrder(
        const QString &orderId,
        const OrderManager *orderManager,
        OrderManagerNode *parent)
    : OrderManagerNode(orderManager, parent)
{
    m_value = orderId;
}
//----------------------------------------------------------
QVariant OrderManagerNodeOrder::data(int column, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole) {
        QString channel = parent()->parent()->value().toString();
        QString orderId = m_value.toString();
        auto order = m_orderManager->m_ordersByChannel[channel].orderById[orderId];
        value = m_orderManager->colInfos()[column].getValue(order.data());
    }
    return value;
}
//----------------------------------------------------------
OrderManagerNodeShipment::OrderManagerNodeShipment(
        const QString &shipmentId,
        const OrderManager *orderManager,
        OrderManagerNode *parent)
    : OrderManagerNode(orderManager, parent)
{
    m_value = shipmentId;
}
//----------------------------------------------------------
QVariant OrderManagerNodeShipment::data(int column, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole) {
        auto parentChannelItem = parent();
        while (dynamic_cast<OrderManagerNodeImporter*>(parentChannelItem) == nullptr) {
            parentChannelItem = parentChannelItem->parent();
        }
        QString channel = parentChannelItem->value().toString();
        QString shipmentId = m_value.toString();
        auto shipment = m_orderManager->m_ordersByChannel[channel].shipmentById[shipmentId];
        value = m_orderManager->colInfos()[column].getValueShipment(shipment.data());
    }
    return value;
}
//----------------------------------------------------------
OrderManagerNodeArticle::OrderManagerNodeArticle(
        const QString &sku,
        const OrderManager *orderManager,
        OrderManagerNode *parent)
    : OrderManagerNode(orderManager, parent)
{
    m_value = sku;
}
//----------------------------------------------------------
QVariant OrderManagerNodeArticle::data(int column, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole) {
        auto parentChannelItem = parent();
        while (dynamic_cast<OrderManagerNodeImporter*>(parentChannelItem) == nullptr) {
            parentChannelItem = parentChannelItem->parent();
        }
        QString channel = parentChannelItem->value().toString();
        auto parentItem = parent();
        QString shipmentId;
        if (dynamic_cast<OrderManagerNodeShipment*>(parentItem)) { ///Parent can be order or shipment
            shipmentId = parentItem->value().toString();
        } else {
            QString orderId = parentItem->value().toString();
            auto order = m_orderManager->m_ordersByChannel[channel].orderById[orderId];
            shipmentId = order->getShipments().values()[0]->getId();
        }
        QString sku = m_value.toString();
        auto shipment = m_orderManager->m_ordersByChannel[channel].shipmentById[shipmentId];
        auto article = shipment->getArticleShipped(sku);
        value = m_orderManager->colInfos()[column].getValueArticle(article.data());
    }
    return value;
}
//----------------------------------------------------------
