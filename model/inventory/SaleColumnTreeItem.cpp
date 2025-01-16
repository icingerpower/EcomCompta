#include "SaleColumnTreeItem.h"

SaleColumnTreeItem::SaleColumnTreeItem(SaleColumnTreeItem *parent)
{
    m_parent = parent;
    m_row = 0;
    if (parent != nullptr)
    {
        m_row = m_parent->m_children.size();
        m_parent->m_children << this;
    }
    m_values << QString{};
}

SaleColumnTreeItem::SaleColumnTreeItem(
        const QString &name, SaleColumnTreeItem *parent)
    : SaleColumnTreeItem(parent)
{
    m_values[0] = name;
}

SaleColumnTreeItem::~SaleColumnTreeItem()
{
    qDeleteAll(m_children);
}

SaleColumnTreeItem *SaleColumnTreeItem::child(int row) const
{
    return m_children[row];
}

void SaleColumnTreeItem::remove(int row)
{
    auto item = m_children.takeAt(row);
    for (int i=row; i<m_children.size(); ++i)
    {
        m_children[i]->m_row = i;
    }
    delete item;
}

void SaleColumnTreeItem::up(int row)
{
    if (row > 0)
    {
        auto temp = m_children[row-1];
        m_children[row-1] = m_children[row];
        m_children[row] = temp;
        m_children[row-1]->m_row = row-1;
        m_children[row]->m_row = row;
    }
}

void SaleColumnTreeItem::down(int row)
{
    if (row < m_children.size()-1)
    {
        auto temp = m_children[row+1];
        m_children[row+1] = m_children[row];
        m_children[row] = temp;
        m_children[row+1]->m_row = row+1;
        m_children[row]->m_row = row;
    }
}

SaleColumnTreeItem *SaleColumnTreeItem::parent() const
{
    return m_parent;
}

QVariant SaleColumnTreeItem::data(int column) const
{
    return m_values[column];
}

QString SaleColumnTreeItem::name() const
{
    return m_values[0].toString();
}

void SaleColumnTreeItem::setName(const QString &name)
{
    m_values[0] = name;
}

void SaleColumnTreeItem::setData(int column, const QVariant &value)
{
    m_values[column] = value;
}

int SaleColumnTreeItem::rowCount() const
{
    return m_children.size();
}

int SaleColumnTreeItem::row() const
{
    return m_row;
}

int SaleColumnTreeItem::columnCount() const
{
    return m_values.size();
}

Qt::ItemFlags SaleColumnTreeItem::flags(int) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool SaleColumnTreeItem::isTopItem() const
{
    return m_parent != nullptr && m_parent->m_parent == nullptr;
}

void SaleColumnTreeItem::addInListOfVariantList(
        int level, QList<QList<QVariant> > &listOfVariantList)
{
    if (level >= 0)
    {
        QList<QVariant> values = {level, className()};
        for (auto it=m_values.begin(); it!=m_values.end(); ++it)
        {
            values << *it;
        }
        listOfVariantList << values;
    }
    for (auto it = m_children.begin(); it != m_children.end(); ++it)
    {
        (*it)->addInListOfVariantList(level + 1, listOfVariantList);
    }
}

QString SaleColumnTreeItem::className() const
{
    return "SaleColumnTreeItem";
}

const QList<SaleColumnTreeItem *> &SaleColumnTreeItem::children() const
{
    return m_children;
}

int SaleColumnTreeItem::childCount() const
{
    return m_children.size();
}

