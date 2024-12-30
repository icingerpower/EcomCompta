#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

#include <QAbstractTableModel>
#include <QSet>
//#include <qvector.h>

#include "model/UpdateToCustomer.h"

class InventoryManager : public QAbstractTableModel, public UpdateToCustomer
{
    Q_OBJECT

public:
    static QString COL_ORDER;
    static QString COL_UNIT_PRICE;
    static QString COL_WEIGHT_GR;
    static QString COL_UNIT_LEFT;
    static QString COL_UNIT_TOTAL;
    static QString COL_UNIT_BEGIN;
    static QString COL_TITLES;
    static QString COL_CODE;
    static QString COL_CODES_OTHER;
    static QString COL_CODE_PARENT;
    static QString COL_DATE;
    static QString COL_LEFT_MONTHS;
    static QString COL_SALES_365J;
    static QString COL_CURRENCY;
    static InventoryManager *instance();
    ~InventoryManager() override;
    QStringList getInventoryFilePaths(const QList<int> &years) const;
    void load(int year);
    QString uniqueId() const override;
    void onCustomerSelectedChanged(
            const QString &customerId) override;
    double inventoryValue() const;
    QString mainCode(const QString &code) const;
    void exportInventoryUnsold(const QString &filePath);
    void exportInventory(const QString &filePath);
    void addInventoryBeginFile(const QString filePath, int year);
    void addPurchaseFile(const QString filePath, const QDate &date);
    void addAmazonReturnFile(const QString filePath, const QDate &date);
    void addMergingCodeFile(const QString filePath);
    void recordMovement(const QString &code,
                        const QString &title,
                        int unit,
                        const QDate &date);
    double valueUnitAverage(const QString &sku) const;
    void clearMovementsBeforeRecording();
    void refresh(int year); //will compute m_mainCodesByOrder
    //QVector<int> salesCountByMonth(const QString &code) const;
    int salesCount(const QString &code, int nDays) const;
    int salesCountParent(const QString &codeParent, int nDays) const;
    void mergeCodes(const QModelIndexList &indexes);
    void mergeCodes(const QString &mainCode,
                    const QStringList &otherCodes);
    void sort(
            int column,
            Qt::SortOrder order = Qt::AscendingOrder) override;

    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int indexColumn(const QString &title) const;

private:
    struct QtyPrice{
        int units;
        double priceUnit;
        int weightGr;
        QtyPrice(){
            units = 0;
            priceUnit = 0.;
            weightGr = 0;
        }
    };
    struct Infos{
        QMultiMap<QDate, int> unitsSold;
        QMultiMap<QDate, int> unitsReturned;
        QtyPrice unitsPurchased;
        QString orderName;
        int left() const;
    };
    struct InfoTitles{
        QSet<QString> titles;
        QString parent;
        QSet<QString> otherCodes;
    };
    struct InfoTotals{
        int unitsSold;
        int unitsPurchased;
        int unitsReturned;
        int left() const;
    };

    struct ColInfo {
        QString name;
        QString (*getValue)(
                const InventoryManager *manager,
                const QString &code,
                const QDate &date,
                const Infos &infosBegin,
                const InfoTotals &totals);
    };

    explicit InventoryManager(QObject *parent = nullptr);
    void _clear();
    void _addMergingCodeFile(const QString &filePath);
    void _addInventoryBeginFile(const QString &filePath, int year);
    void _addPurchaseFile(const QString &filePath, const QDate &date);
    void _addPurchaseOrInventoryFile(const QString &filePath, const QDate &date, bool purchase = true);
    void _addAmazonReturnFile(const QString &filePath, const QDate &date);
    QList<ColInfo> *_colInfos() const;


    QList<QStringList> m_valuesTable;
    QHash<QString, QSet<QString>> m_parentToMainCodes;
    QHash<QString, InfoTitles> m_mainCodeToOtherInfos;
    QHash<QString, QString> m_otherCodeToMain;
    QMap<int, QHash<QString, InfoTotals>> m_totals;
    QMap<int, QMap<QString, QList<Infos>>> m_inventoryBeginYear;
    QMap<int, QMap<QString, QMultiMap<QDate, Infos>>> m_purchases;
};

#endif // INVENTORYMANAGER_H
