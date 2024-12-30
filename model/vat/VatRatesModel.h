#ifndef UEVATTABLEMODEL_H
#define UEVATTABLEMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qhash.h>

class VatRatesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    VatRatesModel(const QString &settingKey, QObject *parent = nullptr);
    ~VatRatesModel() override;
    void initWithDefaultValues();
    void loadFromSettings(const QString &settingKey);
    void loadFromSettings();
    void saveInSettings() const;

    QStringList countries() const;
    QHash<QString, double> getVatRates() const;
    double vatRate(const QString &countryCode) const;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int	rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int	columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role) const override;
    QVariant data(const QModelIndex &index,
                  int role) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;
    void setUniqueValue(double rate);



private:
    QStringList m_countries;
    QHash<QString, double> m_vatRates;
    QString m_settingKey;
};

#endif // UEVATTABLEMODEL_H
