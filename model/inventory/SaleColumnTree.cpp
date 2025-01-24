#include <QSettings>
#include <QDebug>

#include "model/SettingManager.h"
#include "model/CustomerManager.h"

#include "SaleColumnTreeItem.h"

#include "SaleColumnTree.h"

const QString SaleColumnTree::COL_SKU{"SKU"};
const QString SaleColumnTree::COL_UNIT_PRICE{"Unit price"};
const QString SaleColumnTree::COL_UNIT_WEIGHT{"Unit weight"};
const QStringList SaleColumnTree::COL_NAMES_STANDARD{COL_SKU, COL_UNIT_PRICE, COL_UNIT_WEIGHT};

SaleColumnTree::SaleColumnTree(const QString &id, QObject *parent)
    : QAbstractItemModel(parent)
{
    m_settingsKey = "SaleColumnTree_" + id;
    m_rootItem = new SaleColumnTreeItem;
    new SaleColumnTreeItem{COL_SKU, m_rootItem};
    new SaleColumnTreeItem{COL_UNIT_PRICE, m_rootItem};
    new SaleColumnTreeItem{COL_UNIT_WEIGHT, m_rootItem};
    loadFromSettings();
}

void SaleColumnTree::_clear()
{
    if (m_rootItem != nullptr)
    {
        beginRemoveRows(QModelIndex(), 0, m_rootItem->rowCount());
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

QString SaleColumnTree::createId(const QString &templateId)
{
    return templateId + "-" + CustomerManager::instance()->getSelectedCustomerId();
}

int SaleColumnTree::getColIndUnitWeight() const
{
    int index = 0;
    for (const auto &item : m_rootItem->children())
    {
        if (item->name().compare(COL_UNIT_WEIGHT, Qt::CaseInsensitive) == 0)
        {
            return index;
        }
        ++index;
    }
    return -1;
}

int SaleColumnTree::getColIndUnitPrice() const
{
    int index = 0;
    for (const auto &item : m_rootItem->children())
    {
        if (item->name().compare(COL_UNIT_PRICE, Qt::CaseInsensitive) == 0)
        {
            return index;
        }
        ++index;
    }
    return -1;
}

QString SaleColumnTree::containsColumn(const QString &name) const
{
    for (const auto &item : m_rootItem->children())
    {
        if (item->name().compare(name, Qt::CaseInsensitive) == 0)
        {
            return item->name();
        }
        for (const auto &subItem : item->children())
        {
            if (subItem->name().compare(name, Qt::CaseInsensitive) == 0)
            {
                return item->name();
            }
        }
    }
    return QString{};
}

QStringList SaleColumnTree::getHeader() const
{
    QStringList header;
    for (const auto &item : m_rootItem->children())
    {
        header << item->name();
    }
    return header;
}

QHash<QString, QSet<QString> > SaleColumnTree::getGolNamesTree() const
{
    QHash<QString, QSet<QString> > colNames;
    for (const auto &item : m_rootItem->children())
    {
        for (const auto &subItem : item->children())
        {
            colNames[item->name()] << subItem->name();
        }
    }
    return colNames;
}

void SaleColumnTree::addItem(
    const QModelIndex &parent, const QString &name)
{
    if (!COL_NAMES_STANDARD.contains(name))
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
    else
    {
        qWarning() << "No item can be named as the default item";
    }
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
    if (!COL_NAMES_STANDARD.contains(child->name()))
    {
        SaleColumnTreeItem *parent = nullptr;
        if (child->isTopItem())
        {
            parent = m_rootItem;
        }
        else
        {
            parent = static_cast<SaleColumnTreeItem *>(
                parentIndex.internalPointer());
        }
        beginRemoveRows(parentIndex, child->row(), child->row());
        parent->remove(child->row());
        saveInSettings();
        endRemoveRows();
    }
    else
    {
        qWarning() << "The default items can't be removed";
    }
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
