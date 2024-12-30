#ifndef VATTHRESHOLDMODEL_H
#define VATTHRESHOLDMODEL_H

#include <QtCore/qstringlist.h>
#include <QtCore/qhash.h>
#include <QtCore/qabstractitemmodel.h>

class VatThresholdModel : public QAbstractTableModel
{
    Q_OBJECT
    enum CHOICE {ALWAYS = 0, THRESHOLD = 1, CUSTOM_THRESHOLD =2};

public:
    static VatThresholdModel *instance();
    void initWithDefaultValues();
    void saveInSettings() const;
    void loadFromSettings();
    double threshold(const QString &country) const;


    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

public slots:
    void onCustomerSelectedChanged(const QString &customerId);

private:
    explicit VatThresholdModel(QObject *parent = nullptr);
    QStringList m_countries;
    QHash<QString, CHOICE>  m_countryChoices;
    QHash<QString, double>  m_thresholds;
    QString m_settingKey;

};

#endif // VATTHRESHOLDMODEL_H
