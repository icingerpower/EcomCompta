#include <QSettings>
#include <QTextStream>
#include <QFile>
#include <QDate>

#include "../common/utils/CsvReader.h"

#include "model/inventory/SaleColumnTree.h"
#include "model/inventory/SkuEconomics.h"

#include "model/SettingManager.h"

#include "CsvOrderFolders.h"

CsvOrderFolders::CsvOrderFolders(QObject *parent)
    : QAbstractListModel(parent), UpdateToCustomer()
{}

CsvOrderFolders *CsvOrderFolders::instance()
{
    static CsvOrderFolders instance;
    static bool first = true;
    if (first)
    {
        QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
        instance.onCustomerSelectedChanged(selectedCustomerId);
        first = false;
    }
    return &instance;
}

QHash<QString, QStringList> CsvOrderFolders::getGsprData(
    SaleColumnTree *saleColumnTree) const
{
    QHash<QString, QStringList> gsprData;
    const auto &fileInfos = getFileInfos();
    auto dateTimes = fileInfos.keys();
    const auto &headerTo = saleColumnTree->getHeader();
    for (auto itDateTime = dateTimes.rbegin();
         itDateTime != dateTimes.rend(); ++itDateTime)
    {
        const auto& fileInfo = fileInfos[*itDateTime];
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QFile::ReadOnly))
        {
            QTextStream stream{&file};
            auto lines = stream.readAll().split("\n");
            auto headerFrom = lines.takeFirst().split("\t");
            QHash<QString, int> headerFromIndex;
            for (int i=0; i<headerFrom.size(); ++i)
            {
                if (saleColumnTree->containsColumn(headerFrom[i]))
                {
                    headerFromIndex[headerFrom[i].toLower()] = i;
                }
            }
            if (headerFromIndex.contains("sku"))
            {
                int colIndSku = headerFromIndex["sku"];
                for (const auto &line : qAsConst(lines))
                {
                    if (!line.trimmed().isEmpty())
                    {
                        const auto &elements = line.split("\t");
                        if (elements.size() > colIndSku)
                        {
                            const auto &sku = elements[colIndSku];
                            if (!sku.isEmpty() && elements.size() >= headerFromIndex.size())
                            {
                                if (!gsprData.contains(sku))
                                {
                                    for (const auto &colName : headerTo)
                                    {
                                        if (headerFromIndex.contains(colName))
                                        {
                                            gsprData[sku] << elements[headerFromIndex[colName]];
                                        }
                                        else
                                        {
                                            gsprData[sku] << QString{};
                                        }
                                    }
                                }
                                else
                                {
                                    for (int i=0; i<headerTo.size(); ++i)
                                    {
                                        const QString &colName = headerTo[i];
                                        if (headerFromIndex.contains(colName) && gsprData[sku][i].isEmpty())
                                        {
                                            gsprData[sku][i] = elements[headerFromIndex[colName]];
                                        }
                                    }

                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return gsprData;
}

QMap<QDateTime, QFileInfo> CsvOrderFolders::getFileInfos() const
{
    QMap<QDateTime, QFileInfo> fileInfosByDateTime;
    for (const auto &folderPath : m_folderPaths)
    {
        QList<QDir> dirs;
        QDir dir{folderPath};
        dirs << dir;
        const auto &subDirNames = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &subDirName : subDirNames)
        {
            dirs << QDir{dir.absoluteFilePath(subDirName)};
        }
        for (const auto &curDir : qAsConst(dirs))
        {
            const auto &fileInfos = curDir.entryInfoList(QStringList{"*.csv"}, QDir::Files);
            for (const auto &fileInfo : fileInfos)
            {
                fileInfosByDateTime.insert(fileInfo.lastModified(), fileInfo);
            }
        }
    }
    return fileInfosByDateTime;
}

void CsvOrderFolders::addEconomicsData(
    const QString &economicsDirectory,
    const QDate &minDate,
    int indUnitPrice,
    int indWeight,
    double shippingByKilo,
    QStringList &header,
    QHash<QString, QStringList> &gsprData)
{

    QHash<QString, SkuEconomics> skuEconomics;
    const auto &yearDirs = QDir{economicsDirectory}.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (auto itYearDir = yearDirs.rbegin();
         itYearDir != yearDirs.rend(); ++itYearDir)
    {
        const auto &fileInfos = QDir{itYearDir->absoluteFilePath()}.entryInfoList(
            QStringList{"*.csv"}, QDir::Files, QDir::Name);
        for (auto itFileInfo = fileInfos.rbegin();
             itFileInfo != fileInfos.rend(); ++itFileInfo)
        {
            CsvReader reader(itFileInfo->absoluteFilePath(), ",");
            reader.readAll();
            const DataFromCsv *dataRode = reader.dataRode();
            int indSku = dataRode->header.pos("MSKU");
            int indAmazonCountry = dataRode->header.pos("Amazon store");
            //int indDateStart = dataRode->header.pos("Start date");
            int indDateEnd = dataRode->header.pos("End date");
            int indCurrency = dataRode->header.pos("Currency code");
            int indAverageSalePrice = dataRode->header.pos("Average sales price");
            int indUnitSold = dataRode->header.pos("Units sold");
            int indUnitReturned = dataRode->header.pos("Units returned");
            int indFeesAmazonTotal = dataRode->header.pos("Base fulfilment fee total");
            int indFeesAmazonQuantity = dataRode->header.pos("Base fulfilment fee quantity");
            int indFeesStorageTotal = dataRode->header.pos("Base monthly storage fee total");
            int indFeesStorageQuantity = dataRode->header.pos("Base monthly storage fee quantity");
            int indFeesAdsTotal = dataRode->header.pos("Sponsored Products charge total");
            int indFeesAdsQuantity = dataRode->header.pos("Sponsored Products charge quantity");
            for (const auto &elements : dataRode->lines)
            {
                const auto &dateEnd = QDate::fromString(elements[indDateEnd], "MM/dd/yyyy");
                if (dateEnd < minDate)
                {
                    break;
                }
                const auto &sku = elements[indSku];
                if (gsprData.contains(sku))
                {
                    if (!skuEconomics[sku].isUnitPriceRecorder())
                    {
                        double unitPrice = gsprData[sku][indUnitPrice].toDouble();
                        int weightGrams = gsprData[sku][indWeight].toDouble();
                        skuEconomics[sku].recordUnitPrice(
                            unitPrice, weightGrams, shippingByKilo, QDate::currentDate(), "EUR");
                    }
                    int unitSold = elements[indUnitSold].toInt();
                    skuEconomics[sku].recordUnitSold(
                        elements[indUnitSold].toInt(), elements[indUnitReturned].toInt());
                    const auto &currency = elements[indCurrency];
                    skuEconomics[sku].recordAverageSalePriceTaxed(
                        elements[indAmazonCountry], sku, unitSold,
                        elements[indAverageSalePrice].toDouble(),
                        dateEnd,
                        currency);
                    skuEconomics[sku].recordFee(
                        "Base fulfilment fee",
                        elements[indFeesAmazonQuantity].toInt(),
                        elements[indFeesAmazonTotal].toDouble(),
                        dateEnd,
                        currency);
                    skuEconomics[sku].recordFee(
                        "Base monthly storage fee",
                        elements[indFeesStorageQuantity].toInt(),
                        elements[indFeesStorageTotal].toDouble(),
                        dateEnd,
                        currency);
                    skuEconomics[sku].recordFee(
                        "Sponsored Products charge",
                        elements[indFeesAdsQuantity].toInt(),
                        elements[indFeesAdsTotal].toDouble(),
                        dateEnd,
                        currency);
                }
            }
        }
    }
    header << "Avg price untaxed";
    header << "Profit";
    header << "Profit storage";
    header << "Profit ads";
    header << "Returned ratio";
    header << "Fees amazon";
    header << "Fees storage";
    header << "Fees ads";
    for (auto it = skuEconomics.begin();
         it != skuEconomics.end(); ++it)
    {
        const auto &sku = it.key();
        const auto &economics = it.value();
        gsprData[sku] << QString::number(economics.averageSalePriceUntaxed());
        gsprData[sku] << QString::number(economics.profit());
        gsprData[sku] << QString::number(economics.profitWithStorage());
        gsprData[sku] << QString::number(economics.profitWithAds());
        gsprData[sku] << QString::number(economics.returnedRatio());
        gsprData[sku] << QString::number(economics.feesAmz());
        gsprData[sku] << QString::number(economics.feesStorage());
        gsprData[sku] << QString::number(economics.feesAds());
    }
}

QString CsvOrderFolders::uniqueId() const
{
    return "CsvOrderFolders";

}

void CsvOrderFolders::onCustomerSelectedChanged(const QString &customerId)
{
    if (customerId.isEmpty()) {
        _clear();
    } else {
        loadFromSettings();
    }
}

void CsvOrderFolders::add(const QString &folderPath)
{
    if (!m_folderPaths.contains(folderPath))
    {
        beginInsertRows(QModelIndex{}, rowCount(), rowCount());
        m_folderPaths << folderPath;
        saveInSettings();
        endInsertRows();
    }
}

void CsvOrderFolders::remove(const QModelIndex &index)
{
    beginRemoveRows(QModelIndex{}, index.row(), index.row());
    m_folderPaths.removeAt(index.row());
    saveInSettings();
    endRemoveRows();
}

int CsvOrderFolders::rowCount(const QModelIndex &) const
{
    return m_folderPaths.size();
}

QVariant CsvOrderFolders::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            return m_folderPaths[index.row()];
        }
    }
    return QVariant{};
}

Qt::ItemFlags CsvOrderFolders::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void CsvOrderFolders::_clear()
{
    beginRemoveRows(QModelIndex(), 0, m_folderPaths.size() - 1);
    m_folderPaths.clear();
    endRemoveRows();
}

void CsvOrderFolders::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    settings.setValue(settingKey(), m_folderPaths);
}

void CsvOrderFolders::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    auto folderPaths = settings.value(settingKey()).toStringList();
    beginInsertRows(QModelIndex{}, 0, folderPaths.size() - 1);
    m_folderPaths = std::move(folderPaths);
    endInsertRows();
}

