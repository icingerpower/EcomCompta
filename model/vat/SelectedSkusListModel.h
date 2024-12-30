#ifndef SELECTEDSKUSLISTMODEL_H
#define SELECTEDSKUSLISTMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qset.h>
#include <QtCore/qstringlist.h>

class SelectedSkusListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    SelectedSkusListModel(const QString &settingKey, QObject *parent = nullptr);
    ~SelectedSkusListModel() override;
    bool addSku(const QString &sku);
    void removeSku(const QString &sku);
    bool contains(const QString &sku) const;
    void loadFromSettings();
    void saveInSettings() const;
    void clear();




    int	rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int	columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role) const override;
    QVariant data(const QModelIndex &index,
                  int role) const override;
    //bool setData(
            //const QModelIndex &index,
            //const QVariant &value,
            //int role = Qt::EditRole) override;


private:
    QSet<QString> m_skus;
    QStringList m_skusList;
    QString m_settingKey;

};

#endif // SELECTEDSKUSLISTMODEL_H
