#ifndef SKUSFOUNDMANAGER_H
#define SKUSFOUNDMANAGER_H

#include <QtCore/qset.h>
#include <QtCore/qabstractitemmodel.h>

class SkusFoundManager : public QAbstractListModel
{
    Q_OBJECT
public:
    static SkusFoundManager *instance();
    void add(const QString &sku);
    void select(const QString &sku);
    void unselect(const QString &sku);

    int	rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int	columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role) const override;

public slots:
    void filters(const QString &string);
    void clear();

private:
    SkusFoundManager(QObject *object = nullptr);
    QSet<QString> m_skus;
    //QHash<QString, QHash<QString, QString>> m_skus;
    QList<QString> m_filteredSkus;
    QString m_filter;
    QSet<QString> m_selSkus;
};

#endif // SKUSFOUNDMANAGER_H
