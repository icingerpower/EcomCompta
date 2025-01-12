#ifndef SALECOLUMNTREEITEM_H
#define SALECOLUMNTREEITEM_H

#include <QString>
#include <QVariant>


class SaleColumnTreeItem
{
public:
    static int IND_COL_NAME;
    static int IND_COL_LINK;
    static int IND_COL_FOLLOW;
    static int IND_COL_NEW_TAB;
    static int IND_COL_LAST;
    static QString CLASS_NAME;
    SaleColumnTreeItem(
            const QString &name,
            SaleColumnTreeItem *parent = nullptr);
    SaleColumnTreeItem(
            SaleColumnTreeItem *parent = nullptr);
    virtual ~SaleColumnTreeItem();
    SaleColumnTreeItem *child(int row) const;
    void remove(int row);
    void up(int row);
    void down(int row);
    SaleColumnTreeItem *parent() const;
    QVariant data(int column) const;
    void setName(const QString &name);
    void setData(int column, const QVariant &value);
    int rowCount() const;
    int row() const;
    int columnCount() const;
    virtual Qt::ItemFlags flags(int column) const;
    bool isTopItem() const;

    void addInListOfVariantList(
            int level, QList<QList<QVariant>> &listOfVariantList);
    virtual QString className() const;
    const QList<SaleColumnTreeItem *> &children() const;
    int childCount() const;


protected:
    QList<QVariant> m_values;
    SaleColumnTreeItem *m_parent;
    QList<SaleColumnTreeItem *> m_children;
    int m_row;
};



#endif // SALECOLUMNTREEITEM_H
