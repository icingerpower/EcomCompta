#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <QtCore/qabstractitemmodel.h>

#include "model/orderimporters/AbstractOrderImporter.h"
#include "model/orderimporters/OrderManagerNode.h"

class OrderManager : public QAbstractItemModel
{
    Q_OBJECT
public:
    static QString COL_VAT;
    static QString COL_VAT_REGIME;
    static QString COL_COUNTRY_VAT;
    static QString COL_COUNTRY_VAT_DECL;
    static OrderManager *instance();
    ~OrderManager() override;
    OrderManager *copyFilter(std::function<bool (const Order *)> filterIsOrderOk) const;
    OrderManager *copyFilter(std::function<bool (const Order *)> isOrderOk,
                             std::function<bool (const Refund *)> isRefundOk) const;
    OrderManager *copyEmpty() const;
    void _generateTree();
    void clearOrders();
    void recordOrders(
            const QString &channel, const OrdersMapping &ordersMapping);
    void recordOrders(const OrderManager *otherManager);
    void createRefundsFromUncomplete();
    void saveInSettings() const;
    void loadFromSettings();
    void updateShippingAddress(const QModelIndex &index, const Address &address);
    QSet<QString> shippingFromCountry() const;
    void cancelUpdatedShippingAddress(const QModelIndex &index);
    void updateDateTime(const QModelIndex &index, const QDateTime &dateTime);
    void cancelUpdatedDateTime(const QModelIndex &index);
    QMultiMap<QString, double> getNonOrderFees(
            const QString &channel, int year, const QString &paymentId) const;
    QMultiMap<QDateTime, QSharedPointer<Shipment>> getShipmentsOfPayment(
            const QString &channel, int year, const QString &paymentId) const;
    QMultiMap<QDateTime, Shipment *> getShipmentsAndRefunds(
            const QDate &begin, const QDate &end) const;
    QMultiMap<QDateTime, Shipment *> getShipmentsAndRefunds(
            std::function<bool (const Shipment *)> filterIsOk) const;
    QList<int> vatColIndexes();

    const QMap<QString, OrdersMapping> &getOrdersByChannel() const;
    QStringList channels() const;
    QString channelSelected() const;
    void setChannelSelected(const QString &channelSelected);
    bool isIndexOfOrder(const QModelIndex &index) const;
    bool isIndexOfShipmentOrOrderOneShip(const QModelIndex &index) const;
    bool isIndexInOrder(QModelIndex index) const;
    const Order *getOrder(const QString &channel, const QString &orderId) const;
    Order *getOrderNotConst(const QString &channel, const QString &orderId);
    QList<QSharedPointer<Order>> getOrders(
            const QString &channel,
            std::function<bool (const Order *)> filterIsOrderOk) const;
    bool containsOrder(const QString &channel, const QString &orderId);
    void exportCsv(const QString filePath) const;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(
            int row,
            int column,
            const QModelIndex &parent = QModelIndex()
            ) const override;
    QModelIndex parent(
            const QModelIndex &index) const override;
    //bool setData(
            //const QModelIndex &index,
            //const QVariant &value,
            //int role = Qt::EditRole) override;

signals:
    void ordersRecorded();
    void addressChanged(OrderManager *emeter, const QString &channel, const QString &shipmentId); ///Sent only by main instance but it is Ugly and should be handled by other class
    void addressChangeCancelled(OrderManager *emeter, const QString &channel, const QString &shipmentId); ///Sent only by main instance but it is Ugly and should be handled by other class
    void dateTimeChanged(OrderManager *emeter, const QString &channel, const QString &shipmentId); ///Sent only by main instance but it is Ugly and should be handled by other class
    void dateChangedCancelled(OrderManager *emeter, const QString &channel, const QString &shipmentId); ///Sent only by main instance but it is Ugly and should be handled by other class

public slots:
    void onCustomerSelectedChanged(const QString &customerId);
    void onAddressBeingRemoved(const QString &internalId);

private:
    explicit OrderManager(QObject *parent = nullptr);
    struct ColInfo {
        QString name;
        QVariant (*getValue)(const Order *order);
        QVariant (*getValueShipment)(const Shipment *shipment);
        QVariant (*getValueArticle)(const ArticleSold *article);
    };
    struct ColCsvInfo {
        QString name;
        QString (*getValueShipment)(const Shipment *shipment);
    };
    void _clear();
    QList<ColInfo> colInfos() const;
    int colIndex(const QString &name) const;
    QString m_settingKey;
    QString _settingKeyUpdatedDate() const;
    QMap<QString, OrdersMapping> m_ordersByChannel;
    QHash<QString, QString> m_updatedAddressByShipment;
    QHash<QString, Address> m_originalAddressByShipment;
    QHash<QString, QDateTime> m_updatedDateByShipmentRefund;
    QHash<QString, QDateTime> m_originalDateByShipmentRefund;
    OrderManagerNodeRoot *m_rootItem;
    friend class OrderManagerNodeRoot;
    friend class ModelDiffAmazonUE;
    friend class OrderManagerNodeImporter;
    friend class OrderManagerNodeYear;
    friend class OrderManagerNodeOrder;
    friend class OrderManagerNodeShipment;
    friend class OrderManagerNodeArticle;
    friend class RefundManager;
    friend class VatOrdersModel;
};

#endif // ORDERMANAGER_H

