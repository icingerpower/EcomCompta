#ifndef FEESMANAGER_H
#define FEESMANAGER_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qset.h>

class FeesTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static FeesTableModel *instance(const QString &importerName);
    static void addFees(const QString &importerName, const QString &feeTitle);
    void addFees(const QString &feeTitle);
    void saveInSettings() const;
    void loadFromSettings();
    void sortByAccount();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void onCustomerSelectedChanged(const QString &customerId);

private:
    explicit FeesTableModel(QObject *parent = nullptr);
    QList<QStringList> m_fees;
    QSet<QString> m_allFeeNames;
    QString m_settingKey;
};

#endif // FEESMANAGER_H
