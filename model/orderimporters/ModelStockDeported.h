#ifndef MODELSTOCKDEPORTED_H
#define MODELSTOCKDEPORTED_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qset.h>
#include <QtCore/qstringlist.h>

enum StockDeportedComputing{DontCompute, TableValues, Percentage};

class ModelStockDeported : public QAbstractTableModel
{
    Q_OBJECT

public:
    static ModelStockDeported *instance();
    void recordSku(const QString &sku, const QString &title, double value);
    void saveInSettings() const;
    void loadFromSettings();
    void loadFromInventoryManager();
    void resetValue(const QModelIndex &index);
    double inventoryValue(const QString &sku);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

    StockDeportedComputing computingType() const;
    void setComputingType(const StockDeportedComputing &computingType);

    double percentage() const;
    double rate() const;

    double defaultValue() const;
    void setDefaultValue(double defaultValue);

public slots:
    void onCustomerSelectedChanged(const QString &customerId);
    void setPercentage(double percentage);
    void setRate(double rate);

private:
    void _clear();
    explicit ModelStockDeported(QObject *parent = nullptr);
    double m_rate;
    double m_defaultValue;
    QString m_settingKey;
    //QString m_settingKeyChanged() const;
    QString m_settingKeyComputingChoice() const;
    QString m_settingKeyPercentage() const;
    QHash<QString, QStringList> m_skuValues;
    QStringList m_skus;
    QSet<QString> m_skusChanged;
    //QHash<QString, double> m_skusOrigValues;
    StockDeportedComputing m_computingChoice;
};

#endif // MODELSTOCKDEPORTED_H
