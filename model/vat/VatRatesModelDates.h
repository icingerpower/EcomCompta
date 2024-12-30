#ifndef UEVATTABLEMODELDATES_H
#define UEVATTABLEMODELDATES_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qhash.h>

class VatRatesModelDates : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit VatRatesModelDates(QObject *parent = nullptr);
    void add();
    void deleteRow(const QModelIndex &index);
    void deleteRow(int index);
    double vatRate(const QString &country, const QDate &date, double defaultRate) const;
    void loadFromSettings(const QString &settingKey);
    void loadFromSettings();
    void saveInSettings() const;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

    void setSettingKey(const QString &settingKey);

private:
    QList<QVariantList> m_values;
    QString m_settingKey;
    struct Info{
        QDate begin;
        QDate end;
        double rate;
    };
    void _generateValuesByCountry();
    QMultiHash<QString, Info> m_valuesByCounty;
};

#endif // UEVATTABLEMODELDATES_H
