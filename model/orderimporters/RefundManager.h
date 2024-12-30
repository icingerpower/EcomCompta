#ifndef REFUNDMANAGER_H
#define REFUNDMANAGER_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qexception.h>

#include "Refund.h"
#include "OrderMapping.h"

enum RefundState{Refunded, NotRefunded, PartiallyRefunded};

class RefundManagerNode;
class OrderManager;
class RefundManager : public QAbstractItemModel
{
    Q_OBJECT

public:
    static QString COL_VAT_REFUND;
    static QString COL_VAT_REFUND_CONV;
    explicit RefundManager(OrderManager *orderManager, QObject *parent = nullptr);
    ~RefundManager() override;
    //static RefundManager *instance();
    void setOrderManager(OrderManager *orderManager);
    RefundState isOrderRefunded(const QString &channel, const QString &orderId) const;
    RefundState isOrderRefunded(const Order *order) const;
    double getTotalRefunds(const QString &channel, const QString &orderId) const;
    double getTotalRefunds(const Order *order) const;
    bool canRefund(const Order *order, double amount) const;
    void _addRefund(const QString &channel, QSharedPointer<Refund> refund);
    void removeRefund(const QModelIndex &index);
    void addRefundAndUniteAll(const QString &channel, QSharedPointer<Refund> refund);
    void saveInSettings() const;
    void loadFromSettings();
    bool contains(const QString &channel, const QString &id) const;
    QList<int> vatColIndexes();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    const OrderManager *getOrderManager() const;

    const QMap<QString, OrdersMapping> *getRefundsManualByChannel() const;

public slots:
    void retriveRefunds();
    void onCustomerSelectedChanged(const QString &customerId);


private:
    static QList<RefundManager *> *allRefundManagers();
    OrderManager *m_orderManager;
    QMap<QString, OrdersMapping> m_refundsManualByChannel; /// Only the manual refunds
    QMap<QString, OrdersMapping> m_allRefundsUnitedByChannel; /// Copy of refunds in the order manager, updated each time orders are added (orders may come with refunds). No manual refund here
    QList<QSharedPointer<Refund>> m_refundsWithoutOrderToRecordLater; /// Manual refund for which there is no order yet, but order could be loaded later
    QString m_settingKey;
    struct ColInfo {
        QString name;
        QVariant (*getValue)(const Refund *refund, const RefundManager *manager);
    };
    const QList<ColInfo> *colInfos() const;
    friend class RefundManagerNodeRefund;
    RefundManagerNode *m_rootItem;
    void _clear();
    void _generateTree();
    void _addInTree(const QMap<QString, OrdersMapping> &ordersMappingByChannel,
                    const QString &prefixYear = QString());
    void _unitRefunds();
    void _unitRefunds(const QString &channel);
};


class RefundManagerNode {
public:
    RefundManagerNode(
            const QString &value,
            RefundManagerNode *parent = nullptr);
    virtual ~RefundManagerNode();
    virtual QVariant data(int column, int role) const;
    QString value() const;
    virtual void setValue(const QString &value);
    RefundManagerNode *parent() const;
    //bool contains(const QString &key) const;
    //RefundManagerNode *child(const QString &key) const;
    RefundManagerNode *child(int row) const;
    //QStringList keys() const;
    int rowCount() const;
    int row() const;
    void removeChild(int row);
    //void removeChild(const QString &key);
    //QList<ImpReportNode *> children() const;
    //void removeRecursively();
    //void sort(bool alphaOrder = true);
private:
    QString m_value;
    int m_row;
    RefundManagerNode *m_parent;
    QList<RefundManagerNode *> m_children;
    //QHash<QString, ImpReportNode *> m_childrenMapping;
};

class RefundManagerNodeYear : public RefundManagerNode{
public:
    RefundManagerNodeYear(const QString &value, RefundManagerNode *parent = nullptr);
};

class RefundManagerNodeChannel : public RefundManagerNode{
public:
    RefundManagerNodeChannel(const QString &value, RefundManagerNode *parent = nullptr);
};

class RefundManagerNodeSubchannl : public RefundManagerNode{
public:
    RefundManagerNodeSubchannl(const QString &value, RefundManagerNode *parent = nullptr);
};

class RefundManagerNodeRefund : public RefundManagerNode{
public:
    RefundManagerNodeRefund(
            const QString &channel,
            const QString &value,
            const RefundManager *manager,
            RefundManagerNode *parent = nullptr);
    QVariant data(int column, int role) const override;
    QString channel() const;

private:
    QString m_channel;
    const RefundManager *m_refundManager;
};

class RefundIdException : public QException
{
public:
    void raise() const override;
    RefundIdException *clone() const override;

    QString error() const;
    void setError(const QString &error);

private:
    QString m_error;
};

#endif // REFUNDMANAGER_H
