#ifndef CSVORDERFOLDERS_H
#define CSVORDERFOLDERS_H

#include <QAbstractListModel>
#include <QFileInfo>
#include <QDateTime>
#include <QSet>

#include "model/UpdateToCustomer.h"

class SaleColumnTree;

class CsvOrderFolders : public QAbstractListModel, public UpdateToCustomer
{
    Q_OBJECT

public:
    static CsvOrderFolders *instance();
    QHash<QString, QStringList> getGsprData(SaleColumnTree *saleColumnTree) const;
    QMap<QDateTime, QFileInfo> getFileInfos() const;
    QMap<QDateTime, QFileInfo> getFileInfos(const QString &dirPath) const;
    void addEconomicsData(const QSet<QString> &extAmazons,
                          const QString &economicsDirectory,
                          const QDate &minDate,
                          int indUnitPrice,
                          int indWeight,
                          double shippingByKilo,
                          QStringList &header,
                          QHash<QString, QStringList> &gsprData);
    QString uniqueId() const override;
    void onCustomerSelectedChanged(
            const QString &customerId) override;

    void add(const QString &folderPath);
    void remove(const QModelIndex &index);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    explicit CsvOrderFolders(QObject *parent = nullptr);
    QStringList m_folderPaths;
    void _clear();
    void saveInSettings() const;
    void loadFromSettings();
};

#endif // CSVORDERFOLDERS_H
