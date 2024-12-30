#ifndef VATNUMBERSMODEL_H
#define VATNUMBERSMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qdatetime.h>

struct VatNumberData{
    QString number;
    QDate dateRegistration;
    QVariant value(int index) const;
    void setValue(int index, const QVariant &value);
    QString countryCode() const;
    QStringList toStringList() const;
    static VatNumberData fromStringList(const QStringList &values);
    QString toString() const;
    static VatNumberData fromString(const QString &value);
    bool operator!=(const VatNumberData &other) const;
};

class VatNumbersModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static VatNumbersModel *instance();

    void clear();
    void loadFromSettings();
    void saveInSettings() const;
    void add(const QString &number, const QDate &date);
    void add(const VatNumberData &vatNumber);
    void remove(const QModelIndex &index);
    bool containsCountry(const QString &countryCode);
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    VatNumberData iossNumber() const;
    void setIossNumber(const VatNumberData &iossNumber);

    double iossThreshold() const;
    bool hasIossThreshold() const;
    void setIossThreshold(bool hasIossThreshold);

signals:
    void iossThresholdChanded(bool);
    void iossNumberChanged(const VatNumberData &);

public slots:
    void onCustomerSelectedChanged(const QString &customerId);


private:
    explicit VatNumbersModel(QObject *parent = nullptr);
    QString m_settingKeyIoss() const;
    QString m_settingKeyIossThreshold() const;
    QString m_settingKey;
    QList<VatNumberData> m_vatNumbers;
    QHash<QString, int> m_vatNumberIndexesByCountry;
    VatNumberData m_iossNumber;
    bool m_iossThreshold; /// If the threshold of 10000 is used
};

#endif // VATNUMBERSMODEL_H
