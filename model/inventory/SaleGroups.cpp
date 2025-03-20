#include <QSettings>
#include <QDateTime>

#include "SaleGroups.h"

#include "model/SettingManager.h"

const QStringList SaleGroups::HEADERS{QObject::tr("Nom"), QObject::tr("Amazon")};
const int SaleGroups::IND_NAME{0};
const int SaleGroups::IND_AMAZON{1};

const QString SaleGroups::AMAZON_EU{"Amazon EU"};
const QString SaleGroups::AMAZON_CA{"Amazon CA"};
const QString SaleGroups::AMAZON_US{"Amazon US"};
const QString SaleGroups::AMAZON_JP{"Amazon JP"};
const QString SaleGroups::AMAZON_UK{"Amazon UK"};
const QStringList SaleGroups::AMAZONS{
    AMAZON_EU
            , AMAZON_CA
            , AMAZON_US
            , AMAZON_UK
            , AMAZON_JP
};

SaleGroups::SaleGroups(QObject *parent)
    : QAbstractTableModel(parent), UpdateToCustomer()
{
}

void SaleGroups::_clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    m_listOfVariantList.clear();
    endRemoveRows();
}

QString SaleGroups::settingsKeyKeywordSkus() const
{
    return settingKey() + "KeywordSkus";
}

void SaleGroups::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QStringList stringList;
    for (const auto &variantList : m_listOfVariantList)
    {
        QStringList subStringList;
        for (const auto & variant : variantList)
        {
            subStringList << variant.toString();
        }
        stringList << subStringList.join(SettingManager::SEP_COL);
    }
    settings.setValue(settingKey(), stringList);
    settings.setValue(settingsKeyKeywordSkus(), m_listOfKeywordSkus);
}

void SaleGroups::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QStringList tempStringList
            = settings.value(settingKey()).value<QStringList>();
    _clear();
    beginInsertRows(QModelIndex{}, 0, tempStringList.size()-1);
    m_listOfVariantList.clear();
    for (const auto &stringListCompressed : qAsConst(tempStringList))
    {
        const auto &stringList = stringListCompressed.split(SettingManager::SEP_COL);
        QVariantList variantList;
        for (const auto &string : stringList)
        {
            variantList << string;
        }
        m_listOfVariantList << variantList;
    }
    m_listOfKeywordSkus = settings.value(settingsKeyKeywordSkus()).toStringList();
    endInsertRows();
}

SaleGroups *SaleGroups::instance()
{
    static SaleGroups saleGroups;
    static bool first = true;
    if (first)
    {
        QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
        saleGroups.onCustomerSelectedChanged(selectedCustomerId);
        first = false;
    }
    return &saleGroups;
}

void SaleGroups::add(const QString &name)
{
    beginInsertRows(QModelIndex{}, rowCount(), rowCount());
    m_listOfVariantList << QVariantList{
                           name,
                           AMAZON_EU,
                           name + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")};
    m_listOfKeywordSkus << QString{};
    saveInSettings();
    endInsertRows();
}

void SaleGroups::remove(const QModelIndex &index)
{
    beginRemoveRows(QModelIndex{}, index.row(), index.row());
    m_listOfVariantList.removeAt(index.row());
    saveInSettings();
    endRemoveRows();
}

QSet<QString> SaleGroups::getAmazons(
        const QModelIndex &index) const
{
    const QString &amazonGroup = m_listOfVariantList[index.row()][IND_AMAZON].toString();
    QSet<QString> amazons;
    if (amazonGroup == AMAZON_CA)
    {
        amazons.insert("amazon.ca");
    }
    else if (amazonGroup == AMAZON_US)
    {
        amazons.insert("amazon.com");
    }
    else if (amazonGroup == AMAZON_UK)
    {
        amazons.insert("amazon.co.uk");
    }
    else if (amazonGroup == AMAZON_JP)
    {
        amazons.insert("amazon.jp");
    }
    else if (amazonGroup == AMAZON_EU)
    {
        amazons.insert("amazon.fr");
        amazons.insert("amazon.de");
        amazons.insert("amazon.es");
        amazons.insert("amazon.it");
        amazons.insert("amazon.com.be");
        amazons.insert("amazon.com.tr");
        amazons.insert("amazon.nl");
        amazons.insert("amazon.se");
        amazons.insert("amazon.pl");
    }
    return amazons;
}

QSet<QString> SaleGroups::getExtAmazons(const QModelIndex &index) const
{
    const auto amazons = getAmazons(index);
    QSet<QString> extAmazons;
    for (const auto &amazon : amazons)
    {
        extAmazons << amazon.split(".").last().toUpper();
    }
    return extAmazons;
}

QStringList SaleGroups::getKeywordsSkus(const QModelIndex &index) const
{
    QStringList values = m_listOfKeywordSkus[index.row()].split("\n");
    for (auto &value : values)
    {
        value = value.trimmed();
    }
    values.removeDuplicates();
    if (values.contains(QString{}))
    {
        values.removeOne(QString{});
    }
    return values;
}

QSet<QString> SaleGroups::getKeywordsSkusAsSet(const QModelIndex &index) const
{
    const auto &keywordSkus = getKeywordsSkus(index);
    QSet<QString> keywordSkusSet{keywordSkus.begin(), keywordSkus.end()};
    return keywordSkusSet;
}

void SaleGroups::setKeywordsSkus(
        const QModelIndex &index, const QString &text)
{
    m_listOfKeywordSkus[index.row()] = text;
    m_listOfKeywordSkus[index.row()] = getKeywordsSkus(index).join("\n");
    saveInSettings();
}

QString SaleGroups::uniqueId() const
{
    return "SaleGroups";
}

void SaleGroups::onCustomerSelectedChanged(const QString &customerId)
{
    if (customerId.isEmpty()) {
        _clear();
    } else {
        loadFromSettings();
    }
}

QVariant SaleGroups::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return HEADERS[section];
    }
    return QVariant{};
}

int SaleGroups::rowCount(const QModelIndex &) const
{
    return m_listOfVariantList.size();
}

int SaleGroups::columnCount(const QModelIndex &) const
{
    return HEADERS.size();
}

QVariant SaleGroups::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return m_listOfVariantList[index.row()][index.column()];
    }
    return QVariant();
}

bool SaleGroups::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        m_listOfVariantList[index.row()][index.column()] = value;
        saveInSettings();
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

Qt::ItemFlags SaleGroups::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

