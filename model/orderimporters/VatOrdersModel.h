#ifndef VATTABLEUEMODEL_H
#define VATTABLEUEMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include "model/utils/SortedMap.h"

class Shipment;
class VatTableNode;
class OrderManager;
class RefundManager;

class VatOrdersModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    static VatOrdersModel *instance();
    ~VatOrdersModel() override;
    static QString titleTaxes;
    static QString titleTotalTaxed;
    static QString titleTotalUntaxed;
    static QString titleDeportedArrived;
    static QString titleDeportedLeft;
    OrderManager *orderManager();
    //static QHash<QString, QHash<QString, QHash<QString, QVector<double>>>> vatTableValues(
            //const QString &paymentId);
    void computeVat(int year
                    , std::function<void(const Shipment *shipment)> callBackShipment
                        = [](const Shipment *) {}
                    , const QString &dirBookKeeping = ""
                    , const QString &dirInvoice = ""
                    , std::function<bool(const Shipment *shipment)> acceptShipment
                        = [](const Shipment *) -> bool { return true;}
                    , std::function<void(const Shipment *shipment)> callBackShipmentBeforeAccept
                        = [](const Shipment *) {}
    );

    void computeVatMinimalyEcom(const QList<int> &years
                    , std::function<void(const Shipment *shipment)> callBackShipment
                        = [](const Shipment *) {}
                    , std::function<bool(const Shipment *shipment)> acceptShipment
                        = [](const Shipment *) -> bool { return true;}
                    , std::function<void(const Shipment *shipment)> callBackShipmentBeforeAccept
                        = [](const Shipment *) {}
    );


    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void clear();
    void onCustomerSelectedChanged(const QString &customerId);

signals:
    void orderWithUncompleteReports(const QMap<QString, QList<QStringList>> &);
    void refundsWithMissingOrders(QMultiHash<QString, QString> &);
    void skusWithNoValuesFound(const QStringList &);
    void shipmentsNotCompletelyLoaded(const QMultiMap<QDateTime, Shipment *> &);
    void progressed(int percentage);

private:
    explicit VatOrdersModel(QObject *parent = nullptr);
    void _initOrderManagerIfNeeded();
    VatTableNode *m_rootItem;
    OrderManager *m_orderManager;
    RefundManager *m_refundManager;
};


class VatTableNode {
public:
    VatTableNode(const QString &title,
                 VatTableNode *parent = nullptr);
    virtual ~VatTableNode();
    virtual QVariant data(int column, int role = Qt::DisplayRole) const;
    VatTableNode *parent() const;
    VatTableNode *child(int row) const;
    int rowCount() const;
    int row() const;
    void removeChild(int row);
    //void removeRecursively();
    QString title() const;


    QVariant color() const;
    void setColor(const QVariant &color);

    QVariant brush() const;
    void setBrush(const QVariant &brush);


    QList<VatTableNode *> children() const;

private:
    int m_row;
    QString m_title;
    VatTableNode *m_parent;
    QList<VatTableNode *> m_children;
    QVariant m_color;
    QVariant m_brush;
};

class VatTableNodeRegime : public VatTableNode{
public:
    VatTableNodeRegime(const QString &title,
                 VatTableNode *parent = nullptr);
};

class VatTableNodeVatCountry : public VatTableNode{
public:
    VatTableNodeVatCountry(const QString &title,
                 const QVector<double> &values,
                 VatTableNode *parent = nullptr);
    QVariant data(int column, int role = Qt::DisplayRole) const override;
    void setValues(const QVector<double> &values);
    QVector<double> values() const;
    double value(int index) const;
    double total(QList<int> monthsNumber);
    double total(QSet<int> monthsNumber);
private:
    QVector<double> m_values;
};

class VatTableNodeVatDetails : public VatTableNodeVatCountry{
public:
    VatTableNodeVatDetails(const QString &title,
                 const QVector<double> &values,
                 VatTableNode *parent = nullptr);
};


#endif // VATTABLEUEMODEL_H
