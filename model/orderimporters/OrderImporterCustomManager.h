#ifndef ORDERIMPORTERCUSTOMMANAGER_H
#define ORDERIMPORTERCUSTOMMANAGER_H

#include <QAbstractItemModel>

#include "OrderImporterCustomParams.h"
class AbstractOrderImporter;
class OrderImporterCustom;

class OrderImporterCustomManager : public QAbstractListModel
{
    Q_OBJECT

public:
    static OrderImporterCustomManager *instance();
    ~OrderImporterCustomManager() override;
    void saveInSettings() const;
    void loadFromSettings();
    QList<AbstractOrderImporter *> allImporters(); /// To update each time adding an importer
    OrderImporterCustom *importer(const QString &id) const;

    void addCustomParams(const QString &name);
    void removeCustomParams(int index);
    OrderImporterCustomParams *customParams(int index);
    OrderImporterCustomParams *customParams(const QString &id);
    QString customParamsId(int index) const;
    QString customParamsName(int index) const;
    QString customParamsName(const QString &id) const;

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

private:
    explicit OrderImporterCustomManager(QObject *parent = nullptr);
    QList<QStringList> m_names;
    QString _settingKey() const;
    QHash<QString, OrderImporterCustomParams *> m_customParamsById;
    QHash<QString, OrderImporterCustom *> m_importerById;
};

#endif // ORDERIMPORTERCUSTOMMANAGER_H
