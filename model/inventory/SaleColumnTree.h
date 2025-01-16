#ifndef SALECOLUMNTREE_H
#define SALECOLUMNTREE_H

#include <QAbstractItemModel>

class SaleColumnTreeItem;

class SaleColumnTree : public QAbstractItemModel
{
    Q_OBJECT

public:
    static const QString COL_SKU;
    static const QString COL_UNIT_PRICE;
    static const QString COL_UNIT_WEIGHT;
    static const QStringList COL_NAMES_STANDARD;
    explicit SaleColumnTree(const QString &id, QObject *parent = nullptr);
    ~SaleColumnTree() override;
    static QString createId(const QString &templateId);

    int getColIndUnitWeight() const;
    int getColIndUnitPrice() const;

    bool containsColumn(const QString &name) const;
    QStringList getHeader() const;
    QHash<QString, QSet<QString>> getGolNamesTree() const;
    void addItem(const QModelIndex &parent, const QString &name);
    void upItem(const QModelIndex &itemIndex);
    void downItem(const QModelIndex &itemIndex);
    void removeItem(const QModelIndex &itemIndex);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    SaleColumnTreeItem *m_rootItem;
    void _clear();
    void saveInSettings() const;
    void loadFromSettings();
    QString m_settingsKey;
};

#endif // SALECOLUMNTREE_H
