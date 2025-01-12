#include <QSettings>

#include "model/SettingManager.h"

#include "SaleColumnTreeItem.h"

#include "SaleColumnTree.h"


SaleColumnTree::SaleColumnTree(const QString &id, QObject *parent)
    : QAbstractItemModel(parent)
{
    m_settingsKey = "SaleColumnTree_" + id;
    m_rootItem = nullptr;
    loadFromSettings();
}

void SaleColumnTree::_clear()
{
    if (m_rootItem != nullptr)
    {
        beginRemoveRows(QModelIndex(), 0, rowCount()-1);
        delete m_rootItem;
        m_rootItem = new SaleColumnTreeItem;
        endRemoveRows();
    }
    else
    {
        m_rootItem = new SaleColumnTreeItem;
    }
}

void SaleColumnTree::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (rowCount() > 0)
    {
        QStringList stringList;
        QList<QList<QVariant>> listOfVariantList;
        m_rootItem->addInListOfVariantList(-1, listOfVariantList);
        for (const auto &variantList : qAsConst(listOfVariantList))
        {
            QStringList subStringList;
            for (const auto & variant : variantList)
            {
                subStringList << variant.toString();
            }
            stringList << subStringList.join(SettingManager::SEP_COL);
        }
        settings.setValue(m_settingsKey, stringList);
    }
    else if (settings.contains(m_settingsKey))
    {
        settings.remove(m_settingsKey);
    }
}

void SaleColumnTree::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QStringList tempStringList
            = settings.value(m_settingsKey).value<QStringList>();
    _clear();
    beginInsertRows(QModelIndex{}, 0, tempStringList.size()-1);
    QList<SaleColumnTreeItem *> currentItemsByLevel;
    currentItemsByLevel << m_rootItem;
    for (const auto &stringListCompressed : qAsConst(tempStringList))
    {
        const auto &stringList = stringListCompressed.split(SettingManager::SEP_COL);
        QVariantList variantList;
        for (const auto &string : stringList)
        {
            variantList << string;
        }
        int level = variantList.takeFirst().toInt();
        const QString &className = variantList.takeFirst().toString();
        SaleColumnTreeItem *parent = currentItemsByLevel[level];
        SaleColumnTreeItem *child = new SaleColumnTreeItem(parent);
        for (int i=0; i<variantList.size(); ++i)
        {
            child->setData(i, variantList[i]);
        }
        int nextLevel = level + 1;
        if (currentItemsByLevel.size() == nextLevel)
        {
            currentItemsByLevel << nullptr;
        }
        currentItemsByLevel[nextLevel] = child;
    }
    endInsertRows();
}

SaleColumnTree::~SaleColumnTree()
{
    delete m_rootItem;
}

void SaleColumnTree::addItem(
        const QModelIndex &parent, const QString &name)
{
    SaleColumnTreeItem *itemParent = m_rootItem;
    if (parent.isValid())
    {
        itemParent = static_cast<SaleColumnTreeItem *>(
                    parent.internalPointer());
    }
    beginInsertRows(parent, itemParent->rowCount(), itemParent->rowCount());
    new SaleColumnTreeItem{name, itemParent};
    saveInSettings();
    endInsertRows();
}

void SaleColumnTree::upItem(const QModelIndex &itemIndex)
{
    QModelIndex parentIndex = itemIndex.parent();
    SaleColumnTreeItem *parent;
    if (parentIndex.isValid())
    {
        parent = static_cast<SaleColumnTreeItem *>(
                    parentIndex.internalPointer());
    }
    else
    {
        parent = m_rootItem;
    }
    int row = itemIndex.row();
    parent->up(row);
    auto index1 = index(row-1, 0, parentIndex);
    auto index2 = index(row, columnCount()-1, parentIndex);
    saveInSettings();
    emit dataChanged(index1, index2);
}

void SaleColumnTree::downItem(const QModelIndex &itemIndex)
{
    QModelIndex parentIndex = itemIndex.parent();
    SaleColumnTreeItem *parent;
    if (parentIndex.isValid())
    {
        parent = static_cast<SaleColumnTreeItem *>(
                    parentIndex.internalPointer());
    }
    else
    {
        parent = m_rootItem;
    }
    int row = itemIndex.row();
    parent->down(row);
    auto index1 = index(row, 0, parentIndex);
    auto index2 = index(row+1, columnCount()-1, parentIndex);
    saveInSettings();
    emit dataChanged(index1, index2);
}

void SaleColumnTree::removeItem(const QModelIndex &itemIndex)
{
    QModelIndex parentIndex = itemIndex.parent();
    SaleColumnTreeItem *child
            = static_cast<SaleColumnTreeItem *>(
                itemIndex.internalPointer());
    SaleColumnTreeItem *parent
            = static_cast<SaleColumnTreeItem *>(
                parentIndex.internalPointer());
    beginRemoveRows(parentIndex, child->row(), child->row());
    parent->remove(child->row());
    saveInSettings();
    endRemoveRows();
}

QVariant SaleColumnTree::headerData(
        int, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return tr("Nom");
    }
    return QVariant{};
}

QModelIndex SaleColumnTree::index(
        int row, int column, const QModelIndex &parent) const
{
    QModelIndex index;
    if (hasIndex(row, column, parent))
    {
        SaleColumnTreeItem *item = nullptr;
        if (parent.isValid())
        {
            SaleColumnTreeItem *itemParent
                    = static_cast<SaleColumnTreeItem *>(
                        parent.internalPointer());
            item = itemParent->child(row);;
        }
        else
        {
            item = m_rootItem->child(row);
        }
        index = createIndex(row, column, item);
    }
    return index;
}

QModelIndex SaleColumnTree::parent(const QModelIndex &index) const
{
    QModelIndex parentIndex;
    if (index.isValid())
    {
        SaleColumnTreeItem *item
                = static_cast<SaleColumnTreeItem *>(
                    index.internalPointer());
        if (item->parent() != nullptr)
        {
            parentIndex = createIndex(
                        item->parent()->row(), 0, item->parent());
        }
    }
    return parentIndex;
}

int SaleColumnTree::rowCount(const QModelIndex &parent) const
{
    SaleColumnTreeItem *itemParent = nullptr;
    if (parent.isValid())
    {
        itemParent = static_cast<SaleColumnTreeItem *>(
                    parent.internalPointer());
    }
    else
    {
        itemParent = m_rootItem;
    }
    if (itemParent == nullptr)
    {
        return 0;
    }
    int count = itemParent->rowCount();
    return count;
}

int SaleColumnTree::columnCount(const QModelIndex &) const
{
    if (m_rootItem == nullptr)
    {
        return 0;
    }
    return m_rootItem->columnCount();
}

QVariant SaleColumnTree::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            SaleColumnTreeItem *item
                    = static_cast<SaleColumnTreeItem *>(
                        index.internalPointer());
            return item->data(index.column());
        }
    }
    return QVariant();
}

bool SaleColumnTree::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value && role == Qt::EditRole)
    {
        SaleColumnTreeItem *item
                = static_cast<SaleColumnTreeItem *>(
                    index.internalPointer());
        item->setData(index.column(), value);
        emit dataChanged(index, index, {role});
        saveInSettings();
        return true;
    }
    return false;
}

Qt::ItemFlags SaleColumnTree::flags(const QModelIndex &index) const
{
    SaleColumnTreeItem *item
            = static_cast<SaleColumnTreeItem *>(
                index.internalPointer());
    return item->flags(index.column());
}
