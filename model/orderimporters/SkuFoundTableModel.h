#ifndef SKUFOUNDTABLEMODEL_H
#define SKUFOUNDTABLEMODEL_H

#include <QAbstractItemModel>
#include <QSet>

class Shipment;

/// Similar class to SkuFoundManager with less limitations
/// It could be possible to merge the 2 classes

typedef std::function<QVariant(const QString &sku, const QSet<const Shipment *> &shipments)> ProfitFunction;
class SkuFoundTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static SkuFoundTableModel *instance();
    explicit SkuFoundTableModel(QObject *parent = nullptr);

    void addShipmentOrRefund(const Shipment *shipmentOrRefund);
    bool containsText(int rowIndex, const QString &text) const;
    void compute();

    void fillChartData(
            QMap<QDate, int> &chartSales,
            QMap<QDate, int> &chartRefunds,
            const QStringList &skus) const;
    void fillTable(QStringList &headerVert,
            QStringList &headerHoriz,
            QList<QList<QVariant>> &elements,
            const QStringList &skus,
            double unitPrice,
            int MOQ,
            int minNumberMonth, double adsCostPerSale);

    // Header:
    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;


    int rowCount(
            const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(
            const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(
            const QModelIndex &index,
            int role = Qt::DisplayRole) const override;

//protected:
    //void addSku(const QString &sku, const QString &domain, const QString &title);

private:
    static QString ID_PARENT_SKU;
    static QString ID_SKU;
    static QString ID_ASIN;
    static QString ID_UNIT_COST;
    static QString ID_CURRENCY;
    static QString ID_MOQ;
    static QString ID_MONTH_INVENTORY;
    static QString ID_SALE_ADS_COST;
    static QString ID_PROFIT_PERC;
    static QString ID_PROFIT_PERC_NO_ADS;
    static QString ID_CAPITAL_BLOCKED;
    static QString ID_PROFIT_PERC_MONTH;
    static QString ID_CAPITAL_MONTH_RETURN;
    struct ColInfo {
        QString id;
        QString colName;
        ProfitFunction function;
    };
    ProfitFunction _getProfitFunction(const QString &id) const;
    QList<ColInfo> m_colInfos;
    QHash<QString, QHash<QString, QVariant>> m_sku_col_variant;
    QHash<QString, double> m_sku_unitCostLast;
    void _retrieveUnitCost();
    void _initColInfos();
    QHash<QString, QHash<QString, QString>> m_skusDomainsTitles;
    //QList<QList<QVariant>> m_listOfVariantList;
    QStringList m_skus;
    //QStringList m_colNames;
    //QSet<QString> m_domains;
    QHash<int, QString> m_colNameToColIndex;
    QHash<QString, QSet<const Shipment *>> m_skuToShipmentRefunds;
};

#endif // SKUFOUNDTABLEMODEL_H
