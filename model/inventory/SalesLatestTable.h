#ifndef SALESLATESTTABLE_H
#define SALESLATESTTABLE_H

#include <QAbstractTableModel>

class SalesLatestTable : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SalesLatestTable(QObject *parent = nullptr);

    void recordMovement(
            const QString &code,
            const QString &title,
            const QString &lang,
            int unit);
    void compute(QSet<QString> keywordSkus, QSet<QString> subChannels, const QDate &dateFrom, const QDate &dateTo);
    void exportCsv(const QString &filePath, const QString &gsprDir);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<QString, QStringList> getGsprData(
        const QString &dirPath, const QStringList &colNames);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void sort(
            int column,
            Qt::SortOrder order = Qt::AscendingOrder) override;

private:
    void _clear();
    struct ColInfo{
        QString colName;
        std::function<QVariant(const QString &code)> value;
        std::function<bool(const QVariant &value1, const QVariant &value2)> compareInf;
    };
    QList<ColInfo> m_colInfos;
    struct ArticleInfo{
        QHash<QString, QString> names;
        int quantity;
        int nOrders;
    };

    QHash<QString, ArticleInfo> m_articleInfos;
    QStringList m_codesSorted;
    int m_indColSku;
};

#endif // SALESLATESTTABLE_H
