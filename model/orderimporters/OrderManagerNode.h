#ifndef ORDERMANAGERNODE_H
#define ORDERMANAGERNODE_H

#include <QtCore/qvariant.h>
#include <QtCore/qlist.h>
class OrderManager;

class OrderManagerNode
{
public:
    OrderManagerNode(const OrderManager *orderManager, OrderManagerNode *parent = nullptr);
    virtual ~OrderManagerNode();

    int rowCount() const;
    virtual QVariant data(int column, int role = Qt::DisplayRole) const = 0;
    QVariant value() const;
    void setValue(const QVariant &value);
    OrderManagerNode *child(int row) const;


    int row() const;
    void setRow(int row);

    OrderManagerNode *parent() const;

protected:
    QVariant m_value;
    int m_row;
    const OrderManager *m_orderManager;

private:
    OrderManagerNode *m_parent;
    QList<OrderManagerNode *> m_children;
};

class OrderManagerNodeRoot : public OrderManagerNode
{
public:
    OrderManagerNodeRoot(const OrderManager *orderManager, OrderManagerNode *parent = nullptr);
    QVariant data(int column, int role = Qt::DisplayRole) const override;
};

class OrderManagerNodeImporter : public OrderManagerNode
{
public:
    OrderManagerNodeImporter(const QString &name, const OrderManager *orderManager, OrderManagerNode *parent);
    QVariant data(int column, int role = Qt::DisplayRole) const override;
};

class OrderManagerNodeYear : public OrderManagerNode
{
public:
    OrderManagerNodeYear(int year, const OrderManager *orderManager, OrderManagerNode *parent);
    QVariant data(int column, int role = Qt::DisplayRole) const override;
};

class OrderManagerNodeOrder : public OrderManagerNode
{
public:
    OrderManagerNodeOrder(const QString &orderId,
                          const OrderManager *orderManager,
                          OrderManagerNode *parent);
    QVariant data(int column, int role = Qt::DisplayRole) const override;
};

class OrderManagerNodeShipment : public OrderManagerNode
{
public:
    OrderManagerNodeShipment(const QString &shipmentId,
                             const OrderManager *orderManager,
                             OrderManagerNode *parent);
    QVariant data(int column, int role = Qt::DisplayRole) const override;
};

class OrderManagerNodeArticle : public OrderManagerNode
{
public:
    OrderManagerNodeArticle(const QString &sku,
                            const OrderManager *orderManager,
                            OrderManagerNode *parent);
    QVariant data(int column, int role = Qt::DisplayRole) const override;
};




#endif // ORDERMANAGERNODE_H
