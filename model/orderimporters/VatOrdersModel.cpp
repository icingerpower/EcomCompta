#include <QApplication>
#include <QtCore/qdatetime.h>
#include <QtGui/qcolor.h>
#include <QtCore/qdir.h>
#include <QtCore/qset.h>
#include <QtGui/qbrush.h>

#include "../common/countries/CountryManager.h"

#include "model/LogVat.h"
#include "model/SettingManager.h"
#include "model/orderimporters/OrderManager.h"
#include "model/orderimporters/OrderMapping.h"
#include "model/orderimporters/RefundManager.h"
#include "model/CustomerManager.h"
#include "model/orderimporters/AbstractOrderImporter.h"
#include "model/orderimporters/ServiceSalesModel.h"
#include "model/orderimporters/OrderImporterServiceSales.h"
#include "model/orderimporters/ModelStockDeported.h"
#include "model/orderimporters/ImportedFileReportManager.h"
//#include "model/bookkeeping/entries/parsers/EntryParserAmazonOrdersMonthly.h"
#include "model/bookkeeping/entries/parsers/EntryParserOrdersTable.h"
#include "model/bookkeeping/entries/parsers/EntryParserOrders.h"
#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/bookkeeping/entries/AbstractEntrySaver.h"
#include "model/bookkeeping/invoices/SettingInvoices.h"
#include "model/bookkeeping/invoices/InvoiceGenerator.h"
#include "model/bookkeeping/ManagerSaleTypes.h"
#include "model/orderimporters/SkusFoundManager.h"
#include "VatOrdersModel.h"
#include "Shipment.h"

QString VatOrdersModel::titleTaxes = tr("TVA");
QString VatOrdersModel::titleTotalTaxed = tr("Ventes TTC");
QString VatOrdersModel::titleTotalUntaxed = tr("Ventes HT");
QString VatOrdersModel::titleDeportedArrived = tr("Acquisitation intra-communotaire (stock déporté)");
QString VatOrdersModel::titleDeportedLeft = tr("Vente intra-communotaire (stock déporté)");
VatOrdersModel::VatOrdersModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new VatTableNode("");
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &VatOrdersModel::onCustomerSelectedChanged);
    m_orderManager = nullptr;
    m_refundManager = nullptr;
}
//----------------------------------------------------------
VatOrdersModel *VatOrdersModel::instance()
{
    static VatOrdersModel instance;
    return &instance;
}
//----------------------------------------------------------
VatOrdersModel::~VatOrdersModel()
{
    delete m_rootItem;
}
//----------------------------------------------------------
OrderManager *VatOrdersModel::orderManager()
{
    _initOrderManagerIfNeeded();
    return m_orderManager;
}
//----------------------------------------------------------
void VatOrdersModel::_initOrderManagerIfNeeded()
{
    if (m_orderManager == nullptr) {
        m_orderManager = OrderManager::instance()->copyEmpty();
        m_refundManager = new RefundManager(m_orderManager);
    }
}
//----------------------------------------------------------
void VatOrdersModel::computeVat(
        int year,
        std::function<void (const Shipment *)> callBackShipment,
        const QString &dirBookKeeping,
        const QString &dirInvoice,
        std::function<bool (const Shipment *)> acceptShipment,
        std::function<void (const Shipment *)> callBackShipmentBeforeAccept)
{
    //int yearPrevious = year - 1;
    QList<int> previousYears{year-2, year-1};
    _initOrderManagerIfNeeded();
    clear();
    m_orderManager->clearOrders();
    auto importers = AbstractOrderImporter::allImporters();
    for (auto importer : qAsConst(importers)) {
        QString importerName = importer->name();
        auto reportTypes = importer->reportTypes();
        for (auto reportType : qAsConst(reportTypes)) {
            for (const auto &yearPrevious : previousYears)
            {
                auto filePaths
                    = ImportedFileReportManager::instance()->filePaths(
                        importerName, reportType.shortName, yearPrevious); /// For previous year order with current year refund
                filePaths += ImportedFileReportManager::instance()->filePaths(
                    importerName, reportType.shortName, year);
                for (const auto &filePath : qAsConst(filePaths)) {
                    auto currentOrders = importer->loadReport(
                        reportType.shortName, filePath, year);
                    m_orderManager->recordOrders(importerName, *currentOrders.data());
                }
            }
        }
    }
    OrderImporterServiceSales serviceOrdersImporter;
    QString filePathSelfInvoiceReport
            = ServiceSalesModel::instance()->getFilePath(year);
    auto selfOrders = serviceOrdersImporter.loadReport(
                OrderImporterServiceSales::FILE_SALES,
                filePathSelfInvoiceReport,
                year);
    m_orderManager->recordOrders(serviceOrdersImporter.name(), *selfOrders.data());
    m_orderManager->createRefundsFromUncomplete();
    //*/
    emit progressed(1);


     ///  Regime         VAT country     VAT details
    double salesIoss = 0.;
    QMultiMap<QDateTime, Shipment *> shipmentsByDatePreviousYear;
    //QMultiMap<QDateTime, Shipment *> shipmentAndRefundsByDate;
    QList<QMultiMap<QDateTime, Shipment *>> shipmentAndRefundsByDateList;
    shipmentAndRefundsByDateList << QMultiMap<QDateTime, Shipment *>();
    shipmentAndRefundsByDateList << QMultiMap<QDateTime, Shipment *>();
    int nRefundShipments = 0;
    for (auto itChannel = m_orderManager->m_ordersByChannel.begin();
         itChannel != m_orderManager->m_ordersByChannel.end();
         ++itChannel) {
        if (itChannel.value().shipmentByDate.contains(year)) {
            for (auto itShipment = itChannel.value().shipmentByDate[year].begin();
                 itShipment != itChannel.value().shipmentByDate[year].end();
                 ++itShipment) {
                shipmentAndRefundsByDateList[0].insert(itShipment.key(), itShipment.value().data());
                ++nRefundShipments;
                //shipmentAndRefundsByDate.insert(itShipment.key(), itShipment.value().data());
            }
        }
        for (auto yearPrevious : previousYears)
        {
            if (itChannel.value().shipmentByDate.contains(yearPrevious)) {
                for (auto itShipment = itChannel.value().shipmentByDate[yearPrevious].begin();
                     itShipment != itChannel.value().shipmentByDate[yearPrevious].end();
                     ++itShipment) {
                    if (itShipment.value()->isComplete()
                        && itShipment.value()->getDateTime().date().year() == yearPrevious) {
                        /// Added 23 february 2022
                        /// We do this as some report from transactions report may be incomplete when
                        /// the date is wrong and can't be corrected (from end of year to begin next
                        /// year because report is then not in transactions / invoicing)
                        //shipmentAndRefundsByDateList[0].insert(itShipment.key(), itShipment.value().data());
                        shipmentsByDatePreviousYear.insert(itShipment.key(), itShipment.value().data());
                        ++nRefundShipments;
                    }
                }
            }
        }

        if (itChannel.value().refundByDate.contains(year)) {
            for (auto itRefund = itChannel.value().refundByDate[year].begin();
                 itRefund != itChannel.value().refundByDate[year].end();
                 ++itRefund) {
                shipmentAndRefundsByDateList[1].insert(itRefund.key(), itRefund.value().data());
                //shipmentAndRefundsByDate.insert(itRefund.key(), itRefund.value().data());
            }
        }
        auto refundManual = m_refundManager->getRefundsManualByChannel();
        QString channel = itChannel.key();
        if (refundManual->contains(channel)
                && (*refundManual)[channel].refundByDate.contains(year)) {
            auto refundByDate = (*refundManual)[channel].refundByDate[year];
            for (auto itRefund = refundByDate.begin();
                 itRefund != refundByDate.end(); ++itRefund) {
                if (itRefund.value()->getId() == "SCTGRATIV-refund") {
                    auto refund = itRefund.value();
                    int TEMP=10;++TEMP;
                }
                shipmentAndRefundsByDateList[1].insert(
                            itRefund.key(), itRefund.value().data());
                //shipmentAndRefundsByDate.insert(
                            //itRefund.key(), itRefund.value().data());
            }
        }
        /* I don't include refund of previous year, we don't care
        if (itChannel.value().refundByDate.contains(yearPrevious)) {
            for (auto itRefund = itChannel.value().refundByDate[yearPrevious].begin();
                 itRefund != itChannel.value().refundByDate[yearPrevious].end();
                 ++itRefund) {
                shipmentAndRefundsByDatePreviousYear.insert(itRefund.key(), itRefund.value().data());
            }
        }
        //*/
    }
    emit progressed(3);
    double salesIossPreviousYear = 0.;
    for (auto itShipment = shipmentsByDatePreviousYear.begin();
         itShipment != shipmentsByDatePreviousYear.end();
         ++itShipment) {
        auto shipment = itShipment.value();
        QString channel = itShipment.value()->channel();
        bool isRecomputeVat = AbstractOrderImporter::importer(channel)->isVatToRecompute();
        shipment->computeVatRegime(salesIossPreviousYear, isRecomputeVat);
        salesIossPreviousYear += shipment->getTotalPriceUntaxedConverted();
    }
    emit progressed(10);


     ///  Regime         Title     VAT details
    SortedMap<QString, SortedMap<QString, QVector<double>>> vatTableValuesLevel2;
    SortedMap<QString, SortedMap<QString, SortedMap<QString, QVector<double>>>> vatTableValuesLevel3;
    SortedMap<QString, SortedMap<QString, SortedMap<QString, SortedMap<QString, QVector<double>>>>> vatTableValuesLevel4;
    SortedMap<QString, SortedMap<QString, QVector<double>>> *pVatTableValuesLevel2 = &vatTableValuesLevel2;
    SortedMap<QString, SortedMap<QString, SortedMap<QString, QVector<double>>>> *pVatTableValuesLevel3 = &vatTableValuesLevel3;
    SortedMap<QString, SortedMap<QString, SortedMap<QString, SortedMap<QString, QVector<double>>>>> *pVatTableValuesLevel4 = &vatTableValuesLevel4;
    auto addIfNeededLevel1 = [pVatTableValuesLevel2]
            (const QString &title) {
        if (!pVatTableValuesLevel2->contains(title)) {
            (*pVatTableValuesLevel2)[title] = SortedMap<QString, QVector<double>>();
        }
    };
    auto addIfNeededLevel2 = [pVatTableValuesLevel2]
            (const QString &title1, const QString &title2) {
        if (!pVatTableValuesLevel2->contains(title1)) {
            (*pVatTableValuesLevel2)[title1] = SortedMap<QString, QVector<double>>();
        }
        if (!pVatTableValuesLevel2->value(title1).contains(title2)) {
            (*pVatTableValuesLevel2)[title1][title2] = QVector<double>(12, 0.);
        }
    };
    auto addIfNeededLevel3 = [pVatTableValuesLevel3]
            (const QString &title1, const QString &title2, const QString &title3) {
        if (!pVatTableValuesLevel3->contains(title1)) {
            (*pVatTableValuesLevel3)[title1] = SortedMap<QString, SortedMap<QString, QVector<double>>>();
        }
        if (!pVatTableValuesLevel3->value(title1).contains(title2)) {
            (*pVatTableValuesLevel3)[title1][title2] = SortedMap<QString, QVector<double>>();
        }
        if (!pVatTableValuesLevel3->value(title1).value(title2).contains(title3)) {
            (*pVatTableValuesLevel3)[title1][title2][title3] = QVector<double>(12, 0.);
        }
    };
    auto addIfNeededLevel4 = [pVatTableValuesLevel4]
            (const QString &title1, const QString &title2, const QString &title3, const QString &title4) {
        if (!pVatTableValuesLevel4->contains(title1)) {
            (*pVatTableValuesLevel4)[title1] = SortedMap<QString, SortedMap<QString, SortedMap<QString, QVector<double>>>>();
        }
        if (!pVatTableValuesLevel4->value(title1).contains(title2)) {
            (*pVatTableValuesLevel4)[title1][title2] = SortedMap<QString, SortedMap<QString, QVector<double>>>();
        }
        if (!pVatTableValuesLevel4->value(title1).value(title2).contains(title3)) {
            (*pVatTableValuesLevel4)[title1][title2][title3] = SortedMap<QString, QVector<double>>();
        }
        if (!pVatTableValuesLevel4->value(title1).value(title2).value(title3).contains(title4)) {
            (*pVatTableValuesLevel4)[title1][title2][title3][title4] = QVector<double>(12, 0.);
        }
    };
    auto getTitle2NormalRegime = [](const QString &countryName) -> QString {
        return titleTotalTaxed + " " + countryName;
    };
    //EntryParserAmazonOrdersMonthly entryParserAmazonOrdersMonthly;
    auto entryParserSales = ManagerEntryTables::instance()->entryDisplaySale();
    for (auto it = entryParserSales.begin();
         it != entryParserSales.end(); ++it) {
        it.value()->saleParser()->clearTransactions();
    }

    QMap<QString, QList<QStringList>> orderWithReportCombinationMissing;
    QMultiHash<QString, QString> refundsWithOrderMissing;
    //for (auto itShipment = shipmentsByDate.begin();
    //itShipment != shipmentsByDate.end();
    QHash<QString, int> invoiceNumbers;
    QDir dirInvoiceYear;
    QSet<QString> existInvoiceFileNames;
    if (!dirInvoice.isEmpty()) {
        dirInvoiceYear.setPath(dirInvoice);
        QString dirYear = QString::number(year);
        dirInvoiceYear.mkpath(dirYear);
        dirInvoiceYear.cd(dirYear);
        auto invoiceList = dirInvoiceYear.entryList(
                    QStringList() << "*.pdf", QDir::Files);
        existInvoiceFileNames = invoiceList.toSet();
        //existInvoiceFileNames = QSet<QString>(
        //invoiceList.begin(), invoiceList.end());
    }
    QMultiMap<QDateTime, Shipment *> shipmentAndRefundsByDateNotComplete;
    int idShipment = 0;
    for (auto itList = shipmentAndRefundsByDateList.begin();
     itList != shipmentAndRefundsByDateList.end(); ++itList) {
        for (auto itShipment = itList->begin();
         itShipment != itList->end();
         ++itShipment) {
            auto shipment = itShipment.value();
            if (shipment->orderId() == "407-2432847-9631546")
            {
                int TEMP=10;++TEMP;
            }
            if (shipment->isCompletelyLoaded()) {
                callBackShipmentBeforeAccept(shipment);
                /*
                /// It is not put after accept shipment so it is possible to load
                /// faster all SKUS with an acceptShipment function that always
                /// return false. If bugs, it will be find to put it after.
                /// It will just mean more loading time
                auto articlesShipped = shipment->getArticlesShipped();
                for (auto itArt = articlesShipped.begin();
                 itArt != articlesShipped.end(); ++itArt) {
                    SkusFoundManager::instance()->add(itArt.value()->getSku());
                }
                //*/
            } else if (qAbs(itShipment.value()->getTotalPriceTaxed()) > 0.005){
                shipmentAndRefundsByDateNotComplete.insert(
                            itShipment.key(), itShipment.value());
            }

            if (shipment->isCompletelyLoaded() && acceptShipment(shipment)) {
                ++idShipment;

                emit progressed(10 + idShipment / nRefundShipments * 80);
                Refund *refund = dynamic_cast<Refund *>(shipment);
                bool isRefund = refund != nullptr;
                QString channel = itShipment.value()->channel();
                bool isRecomputeVat = AbstractOrderImporter::importer(channel)->isVatToRecompute();
                shipment->computeVatRegime(salesIoss, isRecomputeVat);
                int year = itShipment.key().date().year();
                QString invoicePrefix = shipment->invoicePrefix(year);
                if (!invoiceNumbers.contains(invoicePrefix)) {
                    invoiceNumbers[invoicePrefix] = SettingInvoices::instance()->invoiceFirstNumber();
                } else {
                    ++invoiceNumbers[invoicePrefix];
                }
                QString invoiceName = invoicePrefix + QString::number(invoiceNumbers[invoicePrefix]).rightJustified(6, '0');
                shipment->setInvoiceName(invoiceName);
                if (!dirBookKeeping.isEmpty()) {
                    for (auto it = entryParserSales.begin();
                         it != entryParserSales.end(); ++it) {
                        it.value()->saleParser()->recordTransactions(shipment);
                    }
                    //entryParserAmazonOrdersMonthly.recordTransactions(shipment);
                }
                if (!dirInvoice.isEmpty()) {
                    QString orderId = shipment->getOrder()->getId();
                    QString invoicePrefix = shipment->invoicePrefix(year);
                    QString fileName = shipment->getInvoiceName() + "-" + shipment->getOrder()->getId();
                    fileName += "_" + shipment->getDateTime().toString(SettingManager::DATE_FORMAT_ORDER);
                    fileName += "_" + QString::number(shipment->getTotalPriceTaxed(), 'f', 2) + shipment->getCurrency();
                    fileName += ".pdf";
                    if (!existInvoiceFileNames.contains(fileName)) {
                        //if (!fileName.contains(Shipment::invoicePrefixRefundKeyword())) {
                        for (auto itFile = existInvoiceFileNames.begin();
                             itFile != existInvoiceFileNames.end(); ++itFile) {
                            if (itFile->contains(invoicePrefix)) {
                                //&& !itFile->contains(Shipment::invoicePrefixRefundKeyword())) {
                                QFile::remove(dirInvoiceYear.filePath(*itFile));
                            }
                        }
                    }
                    QString filePath = dirInvoiceYear.filePath(fileName);
                    InvoiceGenerator::saveInvoiceOrRefund(
                                shipment, filePath);
                }
                callBackShipment(shipment);
                salesIoss += shipment->getTotalPriceUntaxedConverted();
                Q_ASSERT(!qIsNaN(salesIoss));
                if (!isRefund || shipment->getOrder() != nullptr) {
                    auto combinationMissing = shipment->getOrder()->reportCombinationMissing();
                    if (combinationMissing.size() > 0
                            && shipment->getOrder()->getDateTime().date().daysTo(QDate::currentDate())
                            > 30
                            && qAbs(shipment->getOrder()->getTotalPriceTaxed()) > 0.005) {
                        QString orderDateTitle
                                = shipment->getOrder()->getDateTime().toString("yyyy-MM-dd ")
                                + shipment->getOrder()->getId();
                        double price = shipment->getOrder()->getTotalPriceTaxed();
                        orderWithReportCombinationMissing[orderDateTitle] = combinationMissing;
                    }
                }
                if (isRefund && shipment->getOrder() == nullptr) {
                    refundsWithOrderMissing.insert(channel, shipment->orderId());
                }

                QString vatRegime = shipment->getRegimeVat();
                QString vatCountryCode = shipment->getCountryCodeVat();
                QString vatCountryName = CountryManager::instance()->countryName(vatCountryCode);
                Q_ASSERT(!vatCountryName.isEmpty());
                //TODOCEDRIC save in a file each order of country / regime with diffence
                if (vatRegime != Shipment::VAT_REGIME_NORMAL_EXPORT) {
                    addIfNeededLevel1(vatRegime);
                }
                int monthIndex = shipment->getDateTime().date().month()-1;
                if (vatRegime == Shipment::VAT_REGIME_NORMAL || vatRegime == Shipment::VAT_REGIME_NORMAL_EXPORT) {
                    QString title2;
                    if (vatRegime == Shipment::VAT_REGIME_NORMAL) {
                        title2 = getTitle2NormalRegime(vatCountryName);
                        Q_ASSERT(!title2.endsWith(" "));
                    } else {
                        if (isRefund) {
                            auto firstShipment = refund->getFirstShipment(
                                        vatRegime, vatCountryCode);
                            if (firstShipment.isNull()) {
                                continue; /// It means order was not loaded and a warning will be returned
                            }
                            QString countryCodeFrom = refund->getFirstShipment(
                                        vatRegime, vatCountryCode)->getAddressFrom().countryCode();
                            QString countryNameFrom = CountryManager::instance()->countryName(
                                        countryCodeFrom);
                            title2 = getTitle2NormalRegime(countryNameFrom);
                            Q_ASSERT(!title2.endsWith(" "));
                        } else {
                            QString countryNameFrom = CountryManager::instance()->countryName(
                                        shipment->getAddressFrom().countryCode());
                            title2 = getTitle2NormalRegime(countryNameFrom);
                            Q_ASSERT(!title2.endsWith(" "));
                        }
                    }
                    addIfNeededLevel2(Shipment::VAT_REGIME_NORMAL, title2);
                    vatTableValuesLevel2[Shipment::VAT_REGIME_NORMAL][title2][monthIndex]
                            += shipment->getTotalPriceTaxedConverted();
                    //vatTableValuesLevel2[Shipment::VAT_REGIME_NORMAL][title2][monthIndex]
                    //+= totalTaxed[vatRegime][vatCountryCode];
                    QString title3_export = tr("Ventes Export");
                    addIfNeededLevel3(Shipment::VAT_REGIME_NORMAL, title2, title3_export);
                    if (vatRegime == Shipment::VAT_REGIME_NORMAL) {
                        auto pricesByRates = shipment->getTotalPriceTaxesByVatRateConverted();
                        for (auto itSaleType = pricesByRates.begin();
                             itSaleType != pricesByRates.end(); ++itSaleType) {
                            for (auto it = itSaleType.value().begin();
                                 it != itSaleType.value().end(); ++it) {
                                //for (auto rateString : totalTaxedByRate[vatRegime][vatCountryCode].keys()) {
                                QString rateString = it.key();
                                QString title3_untaxed = titleTotalUntaxed + " " + rateString;
                                if (itSaleType.key() != ManagerSaleTypes::SALE_PRODUCTS) {
                                    title3_untaxed += " (" + itSaleType.key() + ")";
                                }
                                //QString title3_untaxed = titleTotalUntaxed + " " + rateString;
                                addIfNeededLevel3(Shipment::VAT_REGIME_NORMAL, title2, title3_untaxed);
                                vatTableValuesLevel3[Shipment::VAT_REGIME_NORMAL][title2][title3_untaxed][monthIndex]
                                        += it.value().untaxed;
                                //+= totalTaxedByRate[vatRegime][vatCountryCode][rateString]
                                //- totalTaxesByRate[vatRegime][vatCountryCode][rateString];
                                if (rateString.toDouble() > 0.001) {
                                    QString title3_taxes = titleTaxes + " " + rateString;
                                    addIfNeededLevel3(Shipment::VAT_REGIME_NORMAL, title2, title3_taxes);
                                    vatTableValuesLevel3[Shipment::VAT_REGIME_NORMAL][title2][title3_taxes][monthIndex]
                                            //+= totalTaxesByRate[vatRegime][vatCountryCode][rateString];
                                            += it.value().taxes;
                                }
                            }
                        }
                    } else {
                        vatTableValuesLevel3[Shipment::VAT_REGIME_NORMAL][title2][title3_export][monthIndex]
                                //+= totalTaxed[vatRegime][vatCountryCode];
                                += shipment->getTotalPriceTaxedConverted();
                    }
                } else if (vatRegime == Shipment::VAT_REGIME_NONE) {
                    QString title2 = titleTotalTaxed;
                    addIfNeededLevel2(vatRegime, title2);
                    vatTableValuesLevel2[vatRegime][title2][monthIndex]
                            //+= totalTaxed[vatRegime][vatCountryCode];
                            += shipment->getTotalPriceTaxedConverted();
                    QString title3 = titleTotalTaxed + " " + vatCountryName;
                    addIfNeededLevel3(vatRegime, title2, title3);
                    vatTableValuesLevel3[vatRegime][title2][title3][monthIndex]
                            += shipment->getTotalPriceTaxedConverted();
                    //+= totalTaxed[vatRegime][vatCountryCode];
                } else if (vatRegime == Shipment::VAT_REGIME_IOSS) {
                    QString title2 = titleTotalTaxed;
                    addIfNeededLevel2(vatRegime, title2);
                    vatTableValuesLevel2[vatRegime][title2][monthIndex]
                            += shipment->getTotalPriceTaxedConverted();
                    //+= totalTaxed[vatRegime][vatCountryCode];
                    auto pricesByRates = shipment->getTotalPriceTaxesByVatRateConverted();
                    for (auto itSaleType = pricesByRates.begin();
                         itSaleType != pricesByRates.end(); ++itSaleType) {
                        for (auto it = itSaleType.value().begin();
                             it != itSaleType.value().end(); ++it) {
                            //for (auto rateString : totalTaxedByRate[vatRegime][vatCountryCode].keys()) {
                            QString rateString = it.key();
                            if (rateString.toDouble() > 0.001) {
                                QString endType;
                                if (itSaleType.key() != ManagerSaleTypes::SALE_PRODUCTS) {
                                    endType += " (" + itSaleType.key() + ")";
                                }
                                QString title3_untaxed = titleTotalUntaxed + " " + vatCountryName + " " + rateString + endType;
                                QString title3_taxes = titleTaxes + " " + vatCountryName + " " + rateString + endType;
                                addIfNeededLevel3(vatRegime, title2, title3_untaxed);
                                addIfNeededLevel3(vatRegime, title2, title3_taxes);
                                vatTableValuesLevel3[vatRegime][title2][title3_untaxed][monthIndex]
                                        += it.value().untaxed;
                                //+= totalTaxedByRate[vatRegime][vatCountryCode][rateString]
                                //- totalTaxesByRate[vatRegime][vatCountryCode][rateString];
                                vatTableValuesLevel3[vatRegime][title2][title3_taxes][monthIndex]
                                        += it.value().taxes;
                                //+= totalTaxesByRate[vatRegime][vatCountryCode][rateString];
                            }
                        }
                    }
                } else if (vatRegime == Shipment::VAT_REGIME_OSS) {
                    QString title2 = titleTotalTaxed;
                    addIfNeededLevel2(vatRegime, title2);
                    vatTableValuesLevel2[vatRegime][title2][monthIndex]
                            += shipment->getTotalPriceTaxedConverted();
                    //+= totalTaxed[vatRegime][vatCountryCode];
                    QString title3 = titleTotalTaxed + " " + vatCountryName;
                    addIfNeededLevel3(vatRegime, title2, title3);
                    vatTableValuesLevel3[vatRegime][title2][title3][monthIndex]
                            += shipment->getTotalPriceTaxedConverted();
                    //+= totalTaxed[vatRegime][vatCountryCode];
                    if (title3.contains("rèce")) {
                        int TEMP=10;++TEMP;
                    }
                    auto pricesByRatesUnconv = shipment->getTotalPriceTaxesByVatRate();
                    auto pricesByRates = shipment->getTotalPriceTaxesByVatRateConverted();
                    for (auto itSaleType = pricesByRates.begin();
                         itSaleType != pricesByRates.end(); ++itSaleType) {
                        for (auto it = itSaleType.value().begin();
                             it != itSaleType.value().end(); ++it) {
                            //for (auto rateString : totalTaxedByRate[vatRegime][vatCountryCode].keys()) {
                            QString rateString = it.key();
                            if (rateString.toDouble() > 0.001) {
                                QString endType;
                                if (itSaleType.key() != ManagerSaleTypes::SALE_PRODUCTS) {
                                    endType += " (" + itSaleType.key() + ")";
                                }
                                QString countryCodeFrom;
                                if (isRefund) {
                                    auto firstShipment = refund->getFirstShipment(
                                                vatRegime, vatCountryCode);
                                    if (firstShipment.isNull()) {
                                        continue; /// It means order was not loaded and a warning will be returned
                                    }
                                    countryCodeFrom = firstShipment->getAddressFrom().countryCode();
                                } else {
                                    countryCodeFrom = shipment->getAddressFrom().countryCode();
                                }
                                QString countryCodeTo = shipment->getOrder()->getAddressTo().countryCode();
                                QString countries = CountryManager::instance()->countryName(countryCodeFrom)
                                        + " => "
                                        + CountryManager::instance()->countryName(countryCodeTo);
                                QString title4_untaxed = titleTotalUntaxed + " " + countries+ " " + rateString + endType;
                                QString title4_taxes = titleTaxes + " " + countries + " " + rateString + endType;
                                addIfNeededLevel4(vatRegime, title2, title3, title4_untaxed);
                                addIfNeededLevel4(vatRegime, title2, title3, title4_taxes);
                                vatTableValuesLevel4[vatRegime][title2][title3][title4_untaxed][monthIndex]
                                        += it.value().untaxed;
                                //+= totalTaxedByRate[vatRegime][vatCountryCode][rateString]
                                //- totalTaxesByRate[vatRegime][vatCountryCode][rateString];
                                vatTableValuesLevel4[vatRegime][title2][title3][title4_taxes][monthIndex]
                                        += it.value().taxes;
                                //+= totalTaxesByRate[vatRegime][vatCountryCode][rateString];
                            }
                        }
                    }
                }
                LogVat::instance()->recordAmount("VatOrderModel", shipment);
                //}
                //}
            }
        }
    }
    LogVat::instance()->saveLog("VatOrderModel", "VatOrderModel.log", 2023, 7, 9);

    emit progressed(95);
    /// Deported inventory
    addIfNeededLevel1(Shipment::VAT_REGIME_NORMAL);
    QStringList skusWithNoValue;
    for (auto itChannel = m_orderManager->m_ordersByChannel.begin();
         itChannel != m_orderManager->m_ordersByChannel.end();
         ++itChannel) {
        for (auto itDate = itChannel.value().inventoryDeported[year].begin();
             itDate != itChannel.value().inventoryDeported[year].end();
             ++itDate) {
            for (auto itCountryFrom = itDate.value().begin();
                 itCountryFrom != itDate.value().end();
                 ++itCountryFrom) {
                for (auto itCountryTo = itCountryFrom.value().begin();
                     itCountryTo != itCountryFrom.value().end();
                     ++itCountryTo) {
                    for (auto itSku = itCountryTo.value().begin();
                         itSku != itCountryTo.value().end();
                         ++itSku) {
                        double inventoryValue = ModelStockDeported::instance()->inventoryValue(
                                    itSku.key()) * itSku.value();
                        if (inventoryValue < 0.001) {
                            skusWithNoValue << itSku.key();
                        }
                        int monthIndex = itDate.key().month()-1;
                        addIfNeededLevel1(Shipment::VAT_REGIME_NORMAL);
                        QString title2_from = getTitle2NormalRegime(
                                    CountryManager::instance()->countryName(itCountryFrom.key()));
                        QString title2_to = getTitle2NormalRegime(
                                    CountryManager::instance()->countryName(itCountryTo.key()));
                        addIfNeededLevel2(Shipment::VAT_REGIME_NORMAL, title2_from);
                        addIfNeededLevel2(Shipment::VAT_REGIME_NORMAL, title2_to);
                        addIfNeededLevel3(Shipment::VAT_REGIME_NORMAL, title2_from, titleDeportedLeft);
                        addIfNeededLevel3(Shipment::VAT_REGIME_NORMAL, title2_to, titleDeportedArrived);
                        vatTableValuesLevel3[Shipment::VAT_REGIME_NORMAL][title2_from][titleDeportedLeft][monthIndex]
                                += inventoryValue;
                        vatTableValuesLevel3[Shipment::VAT_REGIME_NORMAL][title2_to][titleDeportedArrived][monthIndex]
                                += inventoryValue;
                    }
                }
            }
        }
    }
    QStringList regimesOrdered
            = {Shipment::VAT_REGIME_NORMAL,
               Shipment::VAT_REGIME_OSS,
               Shipment::VAT_REGIME_IOSS,
               Shipment::VAT_REGIME_NONE};
    VatTableNodeRegime *itemRegimeNormal = nullptr;
    for (auto regime : regimesOrdered) {
        auto itemRegime = new VatTableNodeRegime(regime, m_rootItem);
        if (regime == Shipment::VAT_REGIME_NORMAL) {
            itemRegimeNormal = itemRegime;
        }
        for (auto title2 : vatTableValuesLevel2[regime].keys()) {
            auto itemVatCountry = new VatTableNodeVatCountry(
                       title2,
                        vatTableValuesLevel2[regime][title2],
                        itemRegime);
            if (vatTableValuesLevel3.contains(regime)
                    && vatTableValuesLevel3[regime].contains(title2)) {
                for (auto title3 : vatTableValuesLevel3[regime][title2].keys()) {
                    auto itemVatDetails3 = new VatTableNodeVatDetails(
                                title3,
                                vatTableValuesLevel3[regime][title2][title3],
                                itemVatCountry);
                    if (vatTableValuesLevel4.contains(regime)
                            && vatTableValuesLevel4[regime].contains(title2)
                            && vatTableValuesLevel4[regime][title2].contains(title3)) {
                        for (auto title4 : vatTableValuesLevel4[regime][title2][title3].keys()) {
                            auto itemVatDetails4 = new VatTableNodeVatDetails(
                                        title4,
                                        vatTableValuesLevel4[regime][title2][title3][title4],
                                        itemVatDetails3);
                            (void)itemVatDetails4;
                        }
                    }
                }
            }
        }
    }
    if (!dirBookKeeping.isEmpty()) {
        AccountingEntries allEntries;
        // TODO after this, I have to create a second entry parser to parse the payment reports
        auto saver = AbstractEntrySaver::selected();
        for (auto it = entryParserSales.begin();
             it != entryParserSales.end(); ++it) {
            auto entries = it.value()->saleParser()->entries(year);
            addEntriesStatic(allEntries, entries);
        }
        saver->save(allEntries, QDir(dirBookKeeping));
        //auto entries = entryParserAmazonOrdersMonthly.entries(year);
        //auto saver = AbstractEntrySaver::selected();
        //saver->save(entries, QDir(dirBookKeeping));
    }
    if (m_rootItem->rowCount() > 0) {
        beginInsertRows(QModelIndex(), 0, m_rootItem->rowCount()-1);
        endInsertRows();
    }
    emit progressed(100);
    if (orderWithReportCombinationMissing.size() > 0) {
        // emit too late at a time it seems impossible to work
        emit orderWithUncompleteReports(orderWithReportCombinationMissing);
    }
    if (shipmentAndRefundsByDateNotComplete.size() > 0) {
        emit shipmentsNotCompletelyLoaded(shipmentAndRefundsByDateNotComplete);
    }
    if (refundsWithOrderMissing.size() > 0) {
        emit refundsWithMissingOrders(refundsWithOrderMissing);
    }
    if (skusWithNoValue.size() > 0) {
        emit skusWithNoValuesFound(skusWithNoValue);
    }
}
//----------------------------------------------------------
void VatOrdersModel::computeVatMinimalyEcom(
        const QList<int> &years,
        std::function<void (const Shipment *)> callBackShipment,
        std::function<bool (const Shipment *)> acceptShipment,
        std::function<void (const Shipment *)> callBackShipmentBeforeAccept)
{
    QList<int> yearsSorted = years;
    std::sort(yearsSorted.begin(), yearsSorted.end());
    int yearPrevious = years.first()-1;
    _initOrderManagerIfNeeded();
    clear();
    m_orderManager->clearOrders();
    auto importers = AbstractOrderImporter::allImporters();
    for (auto importer : qAsConst(importers)) {
        QString importerName = importer->name();
        auto reportTypes = importer->reportTypes();
        for (auto reportType : qAsConst(reportTypes)) {
            auto filePaths
                    = ImportedFileReportManager::instance()->filePaths(
                        importerName, reportType.shortName, yearPrevious); /// For previous year order with current year refund
            for (auto filePath : qAsConst(filePaths)) {
                auto currentOrders = importer->loadReport(
                            reportType.shortName, filePath, yearsSorted[0]);
                m_orderManager->recordOrders(importerName, *currentOrders.data());
            }
            for (const auto year : yearsSorted) {
                auto filePaths = ImportedFileReportManager::instance()->filePaths(
                            importerName, reportType.shortName, year);
                for (auto filePath : qAsConst(filePaths)) {
                    auto currentOrders = importer->loadReport(
                                reportType.shortName, filePath, year);
                    m_orderManager->recordOrders(importerName, *currentOrders.data());
                }
            }

        }
    }
   m_orderManager->createRefundsFromUncomplete();
    //*/
    emit progressed(1);


     ///  Regime         VAT country     VAT details
    double salesIoss = 0.;
    QMultiMap<QDateTime, Shipment *> shipmentsByDatePreviousYear;
    //QMultiMap<QDateTime, Shipment *> shipmentAndRefundsByDate;
    QList<QMultiMap<QDateTime, Shipment *>> shipmentAndRefundsByDateList;
    shipmentAndRefundsByDateList << QMultiMap<QDateTime, Shipment *>();
    shipmentAndRefundsByDateList << QMultiMap<QDateTime, Shipment *>();
    int nRefundShipments = 0;
    for (auto itChannel = m_orderManager->m_ordersByChannel.begin();
         itChannel != m_orderManager->m_ordersByChannel.end();
         ++itChannel) {
        for (const auto year : yearsSorted) {
            if (itChannel.value().shipmentByDate.contains(year)) {
                for (auto itShipment = itChannel.value().shipmentByDate[year].begin();
                     itShipment != itChannel.value().shipmentByDate[year].end();
                     ++itShipment) {
                    if (itShipment.value()->getOrder()->getId() == "402-4429555-7470714")
                    {
                        int TEMP=10;++TEMP;
                    }
                    shipmentAndRefundsByDateList[0].insert(itShipment.key(), itShipment.value().data());
                    ++nRefundShipments;
                    //shipmentAndRefundsByDate.insert(itShipment.key(), itShipment.value().data());
                }
            }
        }
        if (itChannel.value().shipmentByDate.contains(yearPrevious)) {
            for (auto itShipment = itChannel.value().shipmentByDate[yearPrevious].begin();
                 itShipment != itChannel.value().shipmentByDate[yearPrevious].end();
                 ++itShipment) {
                if (itShipment.value()->getOrder()->getId() == "402-4429555-7470714")
                {
                    int TEMP=10;++TEMP;
                }
                if (itShipment.value()->isComplete()
                        && itShipment.value()->getDateTime().date().year() == yearPrevious) {
                    /// Added 23 february 2022
                    /// We do this as some report from transactions report may be incomplete when
                    /// the date is wrong and can't be corrected (from end of year to begin next
                    /// year because report is then not in transactions / invoicing)
                    //shipmentAndRefundsByDateList[0].insert(itShipment.key(), itShipment.value().data());
                    shipmentsByDatePreviousYear.insert(itShipment.key(), itShipment.value().data());
                    ++nRefundShipments;
                }
            }
        }

        for (const auto year : yearsSorted) {
            if (itChannel.value().refundByDate.contains(year)) {
                for (auto itRefund = itChannel.value().refundByDate[year].begin();
                     itRefund != itChannel.value().refundByDate[year].end();
                     ++itRefund) {
                    shipmentAndRefundsByDateList[1].insert(itRefund.key(), itRefund.value().data());
                    //shipmentAndRefundsByDate.insert(itRefund.key(), itRefund.value().data());
                }
            }
        }
        auto refundManual = m_refundManager->getRefundsManualByChannel();
        QString channel = itChannel.key();
        for (const auto year : yearsSorted) {
            if (refundManual->contains(channel)
                    && (*refundManual)[channel].refundByDate.contains(year)) {
                auto refundByDate = (*refundManual)[channel].refundByDate[year];
                for (auto itRefund = refundByDate.begin();
                     itRefund != refundByDate.end(); ++itRefund) {
                    shipmentAndRefundsByDateList[1].insert(
                                itRefund.key(), itRefund.value().data());
                    //shipmentAndRefundsByDate.insert(
                    //itRefund.key(), itRefund.value().data());
                }
            }
        }
        /* I don't include refund of previous year, we don't care
        if (itChannel.value().refundByDate.contains(yearPrevious)) {
            for (auto itRefund = itChannel.value().refundByDate[yearPrevious].begin();
                 itRefund != itChannel.value().refundByDate[yearPrevious].end();
                 ++itRefund) {
                shipmentAndRefundsByDatePreviousYear.insert(itRefund.key(), itRefund.value().data());
            }
        }
        //*/
    }
    emit progressed(3);
    double salesIossPreviousYear = 0.;
    for (auto itShipment = shipmentsByDatePreviousYear.begin();
         itShipment != shipmentsByDatePreviousYear.end();
         ++itShipment) {
        auto shipment = itShipment.value();
        QString channel = itShipment.value()->channel();
        bool isRecomputeVat = AbstractOrderImporter::importer(channel)->isVatToRecompute();
        shipment->computeVatRegime(salesIossPreviousYear, isRecomputeVat);
        salesIossPreviousYear += shipment->getTotalPriceUntaxedConverted();
    }
    emit progressed(10);


    /*
     ///  Regime         Title     VAT details
    SortedMap<QString, SortedMap<QString, QVector<double>>> vatTableValuesLevel2;
    SortedMap<QString, SortedMap<QString, SortedMap<QString, QVector<double>>>> vatTableValuesLevel3;
    SortedMap<QString, SortedMap<QString, SortedMap<QString, SortedMap<QString, QVector<double>>>>> vatTableValuesLevel4;
    SortedMap<QString, SortedMap<QString, QVector<double>>> *pVatTableValuesLevel2 = &vatTableValuesLevel2;
    SortedMap<QString, SortedMap<QString, SortedMap<QString, QVector<double>>>> *pVatTableValuesLevel3 = &vatTableValuesLevel3;
    SortedMap<QString, SortedMap<QString, SortedMap<QString, SortedMap<QString, QVector<double>>>>> *pVatTableValuesLevel4 = &vatTableValuesLevel4;
    auto addIfNeededLevel1 = [pVatTableValuesLevel2]
            (const QString &title) {
        if (!pVatTableValuesLevel2->contains(title)) {
            (*pVatTableValuesLevel2)[title] = SortedMap<QString, QVector<double>>();
        }
    };
    auto addIfNeededLevel2 = [pVatTableValuesLevel2]
            (const QString &title1, const QString &title2) {
        if (!pVatTableValuesLevel2->contains(title1)) {
            (*pVatTableValuesLevel2)[title1] = SortedMap<QString, QVector<double>>();
        }
        if (!pVatTableValuesLevel2->value(title1).contains(title2)) {
            (*pVatTableValuesLevel2)[title1][title2] = QVector<double>(12, 0.);
        }
    };
    auto addIfNeededLevel3 = [pVatTableValuesLevel3]
            (const QString &title1, const QString &title2, const QString &title3) {
        if (!pVatTableValuesLevel3->contains(title1)) {
            (*pVatTableValuesLevel3)[title1] = SortedMap<QString, SortedMap<QString, QVector<double>>>();
        }
        if (!pVatTableValuesLevel3->value(title1).contains(title2)) {
            (*pVatTableValuesLevel3)[title1][title2] = SortedMap<QString, QVector<double>>();
        }
        if (!pVatTableValuesLevel3->value(title1).value(title2).contains(title3)) {
            (*pVatTableValuesLevel3)[title1][title2][title3] = QVector<double>(12, 0.);
        }
    };
    auto addIfNeededLevel4 = [pVatTableValuesLevel4]
            (const QString &title1, const QString &title2, const QString &title3, const QString &title4) {
        if (!pVatTableValuesLevel4->contains(title1)) {
            (*pVatTableValuesLevel4)[title1] = SortedMap<QString, SortedMap<QString, SortedMap<QString, QVector<double>>>>();
        }
        if (!pVatTableValuesLevel4->value(title1).contains(title2)) {
            (*pVatTableValuesLevel4)[title1][title2] = SortedMap<QString, SortedMap<QString, QVector<double>>>();
        }
        if (!pVatTableValuesLevel4->value(title1).value(title2).contains(title3)) {
            (*pVatTableValuesLevel4)[title1][title2][title3] = SortedMap<QString, QVector<double>>();
        }
        if (!pVatTableValuesLevel4->value(title1).value(title2).value(title3).contains(title4)) {
            (*pVatTableValuesLevel4)[title1][title2][title3][title4] = QVector<double>(12, 0.);
        }
    };
    auto getTitle2NormalRegime = [](const QString &countryName) -> QString {
        return titleTotalTaxed + " " + countryName;
    };
    //EntryParserAmazonOrdersMonthly entryParserAmazonOrdersMonthly;
    auto entryParserSales = ManagerEntryTables::instance()->entryDisplaySale();
    for (auto it = entryParserSales.begin();
         it != entryParserSales.end(); ++it) {
        it.value()->saleParser()->clearTransactions();
    }

    //for (auto itShipment = shipmentsByDate.begin();
    //itShipment != shipmentsByDate.end();
    QDir dirInvoiceYear;
    QSet<QString> existInvoiceFileNames;
    if (!dirInvoice.isEmpty()) {
        dirInvoiceYear.setPath(dirInvoice);
        QString dirYear = QString::number(year);
        dirInvoiceYear.mkpath(dirYear);
        dirInvoiceYear.cd(dirYear);
        auto invoiceList = dirInvoiceYear.entryList(
                    QStringList() << "*.pdf", QDir::Files);
        existInvoiceFileNames = invoiceList.toSet();
        //existInvoiceFileNames = QSet<QString>(
        //invoiceList.begin(), invoiceList.end());
    }
    //*/
    QMultiHash<QString, QString> refundsWithOrderMissing;
    QMap<QString, QList<QStringList>> orderWithReportCombinationMissing;
    QHash<QString, int> invoiceNumbers;
    QMultiMap<QDateTime, Shipment *> shipmentAndRefundsByDateNotComplete;
    int idShipment = 0;
    for (auto itList = shipmentAndRefundsByDateList.begin();
     itList != shipmentAndRefundsByDateList.end(); ++itList) {
        for (auto itShipment = itList->begin();
         itShipment != itList->end();
         ++itShipment) {
            auto shipment = itShipment.value();
            if (shipment->getId() == "pradize-1321-ship-1"){
                int TEMP=10;++TEMP;
            }
            if (shipment->isCompletelyLoaded()) {
                callBackShipmentBeforeAccept(shipment);
             } else if (qAbs(itShipment.value()->getTotalPriceTaxed()) > 0.005){
                shipmentAndRefundsByDateNotComplete.insert(
                            itShipment.key(), itShipment.value());
            }

            if (shipment->getOrder()->getId() == "402-4429555-7470714")
            {
                int TEMP=10;++TEMP;
            }
            if (shipment->isCompletelyLoaded() && acceptShipment(shipment)) {
                ++idShipment;

                emit progressed(10 + idShipment / nRefundShipments * 80);
                Refund *refund = dynamic_cast<Refund *>(shipment);
                bool isRefund = refund != nullptr;
                QString channel = itShipment.value()->channel();
                bool isRecomputeVat = AbstractOrderImporter::importer(channel)->isVatToRecompute();
                if (shipment->getId() == "amzn1:crow:gKomp5gITCiEyeBAA3QipA") {
                    int TEMP=10;++TEMP;
                }
                shipment->computeVatRegime(salesIoss, isRecomputeVat);
                int year = itShipment.key().date().year();
                QString invoicePrefix = shipment->invoicePrefix(year);
                if (!invoiceNumbers.contains(invoicePrefix)) {
                    invoiceNumbers[invoicePrefix] = SettingInvoices::instance()->invoiceFirstNumber();
                } else {
                    ++invoiceNumbers[invoicePrefix];
                }
                QString invoiceName = invoicePrefix + QString::number(invoiceNumbers[invoicePrefix]);
                shipment->setInvoiceName(invoiceName);
                /*
                if (!dirBookKeeping.isEmpty()) {
                    for (auto it = entryParserSales.begin();
                         it != entryParserSales.end(); ++it) {
                        it.value()->saleParser()->recordTransactions(shipment);
                    }
                    //entryParserAmazonOrdersMonthly.recordTransactions(shipment);
                }
                //*/
                callBackShipment(shipment);
                salesIoss += shipment->getTotalPriceUntaxedConverted();
                Q_ASSERT(!qIsNaN(salesIoss));
                if (!isRefund || shipment->getOrder() != nullptr) {
                    auto combinationMissing = shipment->getOrder()->reportCombinationMissing();
                    if (shipment->getOrder()->getId() == "402-4429555-7470714")
                    {
                        int TEMP=10;++TEMP;
                    }
                    if (combinationMissing.size() > 0
                            && shipment->getOrder()->getDateTime().date().daysTo(QDate::currentDate())
                            > 30
                            && qAbs(shipment->getOrder()->getTotalPriceTaxed()) > 0.005) {
                        QString orderDateTitle
                                = shipment->getOrder()->getDateTime().toString("yyyy-MM-dd ")
                                + shipment->getOrder()->getId();
                        double price = shipment->getOrder()->getTotalPriceTaxed();
                        orderWithReportCombinationMissing[orderDateTitle] = combinationMissing;
                    }
                }
                if (isRefund && shipment->getOrder() == nullptr) {
                    refundsWithOrderMissing.insert(channel, shipment->orderId());
                }
                /*

                QString vatRegime = shipment->getRegimeVat();
                QString vatCountryCode = shipment->getCountryCodeVat();
                QString vatCountryName = SettingManager::instance()->countryName(vatCountryCode);
                if (vatRegime != Shipment::VAT_REGIME_NORMAL_EXPORT) {
                    addIfNeededLevel1(vatRegime);
                }
                int monthIndex = shipment->getDateTime().date().month()-1;
                if (vatRegime == Shipment::VAT_REGIME_NORMAL || vatRegime == Shipment::VAT_REGIME_NORMAL_EXPORT) {
                    QString title2;
                    if (vatRegime == Shipment::VAT_REGIME_NORMAL) {
                        title2 = getTitle2NormalRegime(vatCountryName);
                        Q_ASSERT(!title2.endsWith(" "));
                    } else {
                        if (isRefund) {
                            auto firstShipment = refund->getFirstShipment(
                                        vatRegime, vatCountryCode);
                            if (firstShipment.isNull()) {
                                continue; /// It means order was not loaded and a warning will be returned
                            }
                            QString countryCodeFrom = refund->getFirstShipment(
                                        vatRegime, vatCountryCode)->getAddressFrom().countryCode();
                            QString countryNameFrom = SettingManager::instance()->countryName(
                                        countryCodeFrom);
                            title2 = getTitle2NormalRegime(countryNameFrom);
                            Q_ASSERT(!title2.endsWith(" "));
                        } else {
                            QString countryNameFrom = SettingManager::instance()->countryName(
                                        shipment->getAddressFrom().countryCode());
                            title2 = getTitle2NormalRegime(countryNameFrom);
                            Q_ASSERT(!title2.endsWith(" "));
                        }
                    }
                    addIfNeededLevel2(Shipment::VAT_REGIME_NORMAL, title2);
                    vatTableValuesLevel2[Shipment::VAT_REGIME_NORMAL][title2][monthIndex]
                            += shipment->getTotalPriceTaxedConverted();
                    //vatTableValuesLevel2[Shipment::VAT_REGIME_NORMAL][title2][monthIndex]
                    //+= totalTaxed[vatRegime][vatCountryCode];
                    QString title3_export = tr("Ventes Export");
                    addIfNeededLevel3(Shipment::VAT_REGIME_NORMAL, title2, title3_export);
                    if (vatRegime == Shipment::VAT_REGIME_NORMAL) {
                        auto pricesByRates = shipment->getTotalPriceTaxesByVatRateConverted();
                        for (auto itSaleType = pricesByRates.begin();
                             itSaleType != pricesByRates.end(); ++itSaleType) {
                            for (auto it = itSaleType.value().begin();
                                 it != itSaleType.value().end(); ++it) {
                                //for (auto rateString : totalTaxedByRate[vatRegime][vatCountryCode].keys()) {
                                QString rateString = it.key();
                                QString title3_untaxed = titleTotalUntaxed + " " + rateString;
                                if (itSaleType.key() != ManagerSaleTypes::SALE_PRODUCTS) {
                                    title3_untaxed += " (" + itSaleType.key() + ")";
                                }
                                //QString title3_untaxed = titleTotalUntaxed + " " + rateString;
                                addIfNeededLevel3(Shipment::VAT_REGIME_NORMAL, title2, title3_untaxed);
                                vatTableValuesLevel3[Shipment::VAT_REGIME_NORMAL][title2][title3_untaxed][monthIndex]
                                        += it.value().untaxed;
                                //+= totalTaxedByRate[vatRegime][vatCountryCode][rateString]
                                //- totalTaxesByRate[vatRegime][vatCountryCode][rateString];
                                if (rateString.toDouble() > 0.001) {
                                    QString title3_taxes = titleTaxes + " " + rateString;
                                    addIfNeededLevel3(Shipment::VAT_REGIME_NORMAL, title2, title3_taxes);
                                    vatTableValuesLevel3[Shipment::VAT_REGIME_NORMAL][title2][title3_taxes][monthIndex]
                                            //+= totalTaxesByRate[vatRegime][vatCountryCode][rateString];
                                            += it.value().taxes;
                                }
                            }
                        }
                    } else {
                        vatTableValuesLevel3[Shipment::VAT_REGIME_NORMAL][title2][title3_export][monthIndex]
                                //+= totalTaxed[vatRegime][vatCountryCode];
                                += shipment->getTotalPriceTaxedConverted();
                    }
                } else if (vatRegime == Shipment::VAT_REGIME_NONE) {
                    QString title2 = titleTotalTaxed;
                    addIfNeededLevel2(vatRegime, title2);
                    vatTableValuesLevel2[vatRegime][title2][monthIndex]
                            //+= totalTaxed[vatRegime][vatCountryCode];
                            += shipment->getTotalPriceTaxedConverted();
                    QString title3 = titleTotalTaxed + " " + vatCountryName;
                    addIfNeededLevel3(vatRegime, title2, title3);
                    vatTableValuesLevel3[vatRegime][title2][title3][monthIndex]
                            += shipment->getTotalPriceTaxedConverted();
                    //+= totalTaxed[vatRegime][vatCountryCode];
                } else if (vatRegime == Shipment::VAT_REGIME_IOSS) {
                    QString title2 = titleTotalTaxed;
                    addIfNeededLevel2(vatRegime, title2);
                    vatTableValuesLevel2[vatRegime][title2][monthIndex]
                            += shipment->getTotalPriceTaxedConverted();
                    //+= totalTaxed[vatRegime][vatCountryCode];
                    auto pricesByRates = shipment->getTotalPriceTaxesByVatRateConverted();
                    for (auto itSaleType = pricesByRates.begin();
                         itSaleType != pricesByRates.end(); ++itSaleType) {
                        for (auto it = itSaleType.value().begin();
                             it != itSaleType.value().end(); ++it) {
                            //for (auto rateString : totalTaxedByRate[vatRegime][vatCountryCode].keys()) {
                            QString rateString = it.key();
                            if (rateString.toDouble() > 0.001) {
                                QString endType;
                                if (itSaleType.key() != ManagerSaleTypes::SALE_PRODUCTS) {
                                    endType += " (" + itSaleType.key() + ")";
                                }
                                QString title3_untaxed = titleTotalUntaxed + " " + vatCountryName + " " + rateString + endType;
                                QString title3_taxes = titleTaxes + " " + vatCountryName + " " + rateString + endType;
                                addIfNeededLevel3(vatRegime, title2, title3_untaxed);
                                addIfNeededLevel3(vatRegime, title2, title3_taxes);
                                vatTableValuesLevel3[vatRegime][title2][title3_untaxed][monthIndex]
                                        += it.value().untaxed;
                                //+= totalTaxedByRate[vatRegime][vatCountryCode][rateString]
                                //- totalTaxesByRate[vatRegime][vatCountryCode][rateString];
                                vatTableValuesLevel3[vatRegime][title2][title3_taxes][monthIndex]
                                        += it.value().taxes;
                                //+= totalTaxesByRate[vatRegime][vatCountryCode][rateString];
                            }
                        }
                    }
                } else if (vatRegime == Shipment::VAT_REGIME_OSS) {
                    QString title2 = titleTotalTaxed;
                    addIfNeededLevel2(vatRegime, title2);
                    vatTableValuesLevel2[vatRegime][title2][monthIndex]
                            += shipment->getTotalPriceTaxedConverted();
                    //+= totalTaxed[vatRegime][vatCountryCode];
                    QString title3 = titleTotalTaxed + " " + vatCountryName;
                    addIfNeededLevel3(vatRegime, title2, title3);
                    vatTableValuesLevel3[vatRegime][title2][title3][monthIndex]
                            += shipment->getTotalPriceTaxedConverted();
                    //+= totalTaxed[vatRegime][vatCountryCode];
                    if (title3.contains("rèce")) {
                        int TEMP=10;++TEMP;
                    }
                    auto pricesByRatesUnconv = shipment->getTotalPriceTaxesByVatRate();
                    auto pricesByRates = shipment->getTotalPriceTaxesByVatRateConverted();
                    for (auto itSaleType = pricesByRates.begin();
                         itSaleType != pricesByRates.end(); ++itSaleType) {
                        for (auto it = itSaleType.value().begin();
                             it != itSaleType.value().end(); ++it) {
                            //for (auto rateString : totalTaxedByRate[vatRegime][vatCountryCode].keys()) {
                            QString rateString = it.key();
                            if (rateString.toDouble() > 0.001) {
                                QString endType;
                                if (itSaleType.key() != ManagerSaleTypes::SALE_PRODUCTS) {
                                    endType += " (" + itSaleType.key() + ")";
                                }
                                QString countryCodeFrom;
                                if (isRefund) {
                                    auto firstShipment = refund->getFirstShipment(
                                                vatRegime, vatCountryCode);
                                    if (firstShipment.isNull()) {
                                        continue; /// It means order was not loaded and a warning will be returned
                                    }
                                    countryCodeFrom = firstShipment->getAddressFrom().countryCode();
                                } else {
                                    countryCodeFrom = shipment->getAddressFrom().countryCode();
                                }
                                QString countryCodeTo = shipment->getOrder()->getAddressTo().countryCode();
                                QString countries = SettingManager::instance()->countryName(countryCodeFrom)
                                        + " => "
                                        + SettingManager::instance()->countryName(countryCodeTo);
                                QString title4_untaxed = titleTotalUntaxed + " " + countries+ " " + rateString + endType;
                                QString title4_taxes = titleTaxes + " " + countries + " " + rateString + endType;
                                addIfNeededLevel4(vatRegime, title2, title3, title4_untaxed);
                                addIfNeededLevel4(vatRegime, title2, title3, title4_taxes);
                                vatTableValuesLevel4[vatRegime][title2][title3][title4_untaxed][monthIndex]
                                        += it.value().untaxed;
                                //+= totalTaxedByRate[vatRegime][vatCountryCode][rateString]
                                //- totalTaxesByRate[vatRegime][vatCountryCode][rateString];
                                vatTableValuesLevel4[vatRegime][title2][title3][title4_taxes][monthIndex]
                                        += it.value().taxes;
                                //+= totalTaxesByRate[vatRegime][vatCountryCode][rateString];
                            }
                        }
                    }
                }
                //*/
            }
        }
    }

    emit progressed(95);
    /*
    /// Deported inventory
    addIfNeededLevel1(Shipment::VAT_REGIME_NORMAL);
    QStringList skusWithNoValue;
    for (auto itChannel = m_orderManager->m_ordersByChannel.begin();
         itChannel != m_orderManager->m_ordersByChannel.end();
         ++itChannel) {
        for (auto itDate = itChannel.value().inventoryDeported[year].begin();
             itDate != itChannel.value().inventoryDeported[year].end();
             ++itDate) {
            for (auto itCountryFrom = itDate.value().begin();
                 itCountryFrom != itDate.value().end();
                 ++itCountryFrom) {
                for (auto itCountryTo = itCountryFrom.value().begin();
                     itCountryTo != itCountryFrom.value().end();
                     ++itCountryTo) {
                    for (auto itSku = itCountryTo.value().begin();
                         itSku != itCountryTo.value().end();
                         ++itSku) {
                        double inventoryValue = ModelStockDeported::instance()->inventoryValue(
                                    itSku.key()) * itSku.value();
                        if (inventoryValue < 0.001) {
                            skusWithNoValue << itSku.key();
                        }
                        int monthIndex = itDate.key().month()-1;
                        addIfNeededLevel1(Shipment::VAT_REGIME_NORMAL);
                        QString title2_from = getTitle2NormalRegime(
                                    SettingManager::instance()->countryName(itCountryFrom.key()));
                        QString title2_to = getTitle2NormalRegime(
                                    SettingManager::instance()->countryName(itCountryTo.key()));
                        addIfNeededLevel2(Shipment::VAT_REGIME_NORMAL, title2_from);
                        addIfNeededLevel2(Shipment::VAT_REGIME_NORMAL, title2_to);
                        addIfNeededLevel3(Shipment::VAT_REGIME_NORMAL, title2_from, titleDeportedLeft);
                        addIfNeededLevel3(Shipment::VAT_REGIME_NORMAL, title2_to, titleDeportedArrived);
                        vatTableValuesLevel3[Shipment::VAT_REGIME_NORMAL][title2_from][titleDeportedLeft][monthIndex]
                                += inventoryValue;
                        vatTableValuesLevel3[Shipment::VAT_REGIME_NORMAL][title2_to][titleDeportedArrived][monthIndex]
                                += inventoryValue;
                    }
                }
            }
        }
    }
    QStringList regimesOrdered
            = {Shipment::VAT_REGIME_NORMAL,
               Shipment::VAT_REGIME_OSS,
               Shipment::VAT_REGIME_IOSS,
               Shipment::VAT_REGIME_NONE};
    VatTableNodeRegime *itemRegimeNormal = nullptr;
    for (auto regime : regimesOrdered) {
        auto itemRegime = new VatTableNodeRegime(regime, m_rootItem);
        if (regime == Shipment::VAT_REGIME_NORMAL) {
            itemRegimeNormal = itemRegime;
        }
        for (auto title2 : vatTableValuesLevel2[regime].keys()) {
            auto itemVatCountry = new VatTableNodeVatCountry(
                       title2,
                        vatTableValuesLevel2[regime][title2],
                        itemRegime);
            if (vatTableValuesLevel3.contains(regime)
                    && vatTableValuesLevel3[regime].contains(title2)) {
                for (auto title3 : vatTableValuesLevel3[regime][title2].keys()) {
                    auto itemVatDetails3 = new VatTableNodeVatDetails(
                                title3,
                                vatTableValuesLevel3[regime][title2][title3],
                                itemVatCountry);
                    if (vatTableValuesLevel4.contains(regime)
                            && vatTableValuesLevel4[regime].contains(title2)
                            && vatTableValuesLevel4[regime][title2].contains(title3)) {
                        for (auto title4 : vatTableValuesLevel4[regime][title2][title3].keys()) {
                            auto itemVatDetails4 = new VatTableNodeVatDetails(
                                        title4,
                                        vatTableValuesLevel4[regime][title2][title3][title4],
                                        itemVatDetails3);
                            (void)itemVatDetails4;
                        }
                    }
                }
            }
        }
    }
    if (!dirBookKeeping.isEmpty()) {
        AccountingEntries allEntries;
        // TODO after this, I have to create a second entry parser to parse the payment reports
        auto saver = AbstractEntrySaver::selected();
        for (auto it = entryParserSales.begin();
             it != entryParserSales.end(); ++it) {
            auto entries = it.value()->saleParser()->entries(year);
            addEntriesStatic(allEntries, entries);
        }
        saver->save(allEntries, QDir(dirBookKeeping));
        //auto entries = entryParserAmazonOrdersMonthly.entries(year);
        //auto saver = AbstractEntrySaver::selected();
        //saver->save(entries, QDir(dirBookKeeping));
    }
    if (m_rootItem->rowCount() > 0) {
        beginInsertRows(QModelIndex(), 0, m_rootItem->rowCount()-1);
        endInsertRows();
    }
    //*/
    emit progressed(100);
    if (orderWithReportCombinationMissing.size() > 0) {
        // emit too late at a time it seems impossible to work
        emit orderWithUncompleteReports(orderWithReportCombinationMissing);
    }
    if (shipmentAndRefundsByDateNotComplete.size() > 0) {
        emit shipmentsNotCompletelyLoaded(shipmentAndRefundsByDateNotComplete);
    }
    if (refundsWithOrderMissing.size() > 0) {
        emit refundsWithMissingOrders(refundsWithOrderMissing);
    }
    //*if (skusWithNoValue.size() > 0) {
        //emit skusWithNoValuesFound(skusWithNoValue);
    //}
}
//----------------------------------------------------------
void VatOrdersModel::onCustomerSelectedChanged(const QString &)
{
    clear();
}
//----------------------------------------------------------
QVariant VatOrdersModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        static QStringList months = QStringList("") << *SettingManager::instance()->months();
        value = months[section];
    }
    return value;
}
//----------------------------------------------------------
Qt::ItemFlags VatOrdersModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if (index.column() > 0) {
        VatTableNode *item
                = static_cast<VatTableNode *>(
                    index.internalPointer());
        if (dynamic_cast<VatTableNodeVatCountry*>(item) != nullptr
                && dynamic_cast<VatTableNodeVatDetails*>(item) == nullptr) {
            flags |= Qt::ItemIsSelectable;
        }
    }
    return flags;
}
//----------------------------------------------------------
QModelIndex VatOrdersModel::index(
        int row, int column, const QModelIndex &parent) const
{
    QModelIndex index;
    if (hasIndex(row, column, parent)) {
        VatTableNode *item = nullptr;
        if (parent.isValid()) {
            VatTableNode *itemParent
                    = static_cast<VatTableNode *>(
                        parent.internalPointer());
            item = itemParent->child(row);;
        } else {
            item = m_rootItem->child(row);
        }
        index = createIndex(row, column, item);
    }
    return index;
}
//----------------------------------------------------------
QModelIndex VatOrdersModel::parent(const QModelIndex &index) const
{
    QModelIndex parentIndex;
    if (index.isValid()) {
        VatTableNode *item
                = static_cast<VatTableNode *>(
                    index.internalPointer());
        if (item->parent() != nullptr) {
            parentIndex = createIndex(item->parent()->row(), 0, item->parent());
        }
    }
    return parentIndex;
}
//----------------------------------------------------------
int VatOrdersModel::rowCount(const QModelIndex &parent) const
{
    VatTableNode *itemParent = nullptr;
    if (parent.isValid()) {
        itemParent = static_cast<VatTableNode *>(
                    parent.internalPointer());
    } else {
        itemParent = m_rootItem;
    }
    int count = itemParent->rowCount();
    return count;
}
//----------------------------------------------------------
int VatOrdersModel::columnCount(const QModelIndex &) const
{
    return 13;
}
//----------------------------------------------------------
QVariant VatOrdersModel::data(const QModelIndex &index, int role) const
{
    VatTableNode *item
            = static_cast<VatTableNode *>(
                index.internalPointer());
    if (role == Qt::BackgroundRole && item->parent() != m_rootItem) {
        QModelIndex parent = index;
        VatTableNode *itemParentParent = item->parent();
        while (itemParentParent->parent() != m_rootItem) {
            parent = parent.parent();
            itemParentParent = itemParentParent->parent();
        }
        /*
        VatTableNode *parentCountry = item;
        QModelIndex parentIndex = index;
        while (parentCountry->parent() != nullptr
               && (dynamic_cast<VatTableNodeVatCountry *>(parentCountry->parent()) != nullptr
                   || dynamic_cast<VatTableNodeVatDetails *>(parentCountry->parent()) != nullptr)){
            parentCountry = parentCountry->parent();
            parentIndex = index.parent();
        }
        /*
        while (dynamic_cast<VatTableNodeVatCountry *>(parentCountry) == nullptr
               && parentCountry->parent() != nullptr) {
            parentCountry = parentCountry->parent();
            parentIndex = index.parent();
        }
        //*/
        //Q_ASSERT(parentCountry->parent() == m_rootItem
                 //|| dynamic_cast<VatTableNodeRegime *>(parentCountry->parent()) != nullptr);
        //if (dynamic_cast<VatTableNodeVatCountry *>(parentCountry) != nullptr) {
        int row = parent.row();
        if (row % 2 == 0) {
            return QBrush(QColor("#eaf2ff"));
        }
        //}

    }
    return item->data(index.column(), role);
}
//----------------------------------------------------------
void VatOrdersModel::clear()
{
    if (m_rootItem->rowCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, rowCount()-1);
        delete m_rootItem;
        m_rootItem = new VatTableNode("");
        m_refundManager->deleteLater();
        m_orderManager->deleteLater();
        m_orderManager = OrderManager::instance()->copyEmpty();
        m_refundManager = new RefundManager(m_orderManager);
        endRemoveRows();
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
VatTableNode::VatTableNode(const QString &title,
        VatTableNode *parent)
{
    m_title = title;
    m_row = 0;
    m_parent = parent;
    if (parent != nullptr) {
        m_row = parent->m_children.size();
        parent->m_children << this;
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
VatTableNode::~VatTableNode()
{
    qDeleteAll(m_children);
}
//----------------------------------------------------------
//----------------------------------------------------------
QVariant VatTableNode::data(int column, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole && column == 0) {
        value = m_title;
    } else if (role == Qt::BackgroundRole && m_brush.isValid()) {
        value = m_brush;
    } else if (role == Qt::FontRole && m_color.isValid()) {
        value = m_color;
    }
    return value;
}
//----------------------------------------------------------
//----------------------------------------------------------
VatTableNode *VatTableNode::parent() const
{
    return m_parent;
}
//----------------------------------------------------------
//----------------------------------------------------------
VatTableNode *VatTableNode::child(int row) const
{
    return m_children[row];
}
//----------------------------------------------------------
//----------------------------------------------------------
int VatTableNode::rowCount() const
{
    return m_children.size();
}
//----------------------------------------------------------
//----------------------------------------------------------
int VatTableNode::row() const
{
    return m_row;
}
//----------------------------------------------------------
//----------------------------------------------------------
void VatTableNode::removeChild(int row)
{
    m_children.removeAt(row);
}
//----------------------------------------------------------
//----------------------------------------------------------
QString VatTableNode::title() const
{
    return m_title;
}
//----------------------------------------------------------
//----------------------------------------------------------
QVector<double> VatTableNodeVatCountry::values() const
{
    return m_values;
}
//----------------------------------------------------------
//----------------------------------------------------------
double VatTableNodeVatCountry::value(int index) const
{
    return m_values[index];
}
//----------------------------------------------------------
double VatTableNodeVatCountry::total(QList<int> monthsNumber)
{
    double total = 0.;
    for (auto month : monthsNumber) {
        total += m_values[month - 1];
    }
    return total;
}
//----------------------------------------------------------
double VatTableNodeVatCountry::total(QSet<int> monthsNumber)
{
    double total = 0.;
    for (auto month : monthsNumber) {
        total += m_values[month - 1];
    }
    return total;
}
//----------------------------------------------------------
//----------------------------------------------------------
QVariant VatTableNode::color() const
{
    return m_color;
}
//----------------------------------------------------------
//----------------------------------------------------------
void VatTableNode::setColor(const QVariant &color)
{
    m_color = color;
}
//----------------------------------------------------------
//----------------------------------------------------------
QVariant VatTableNode::brush() const
{
    return m_brush;
}
//----------------------------------------------------------
//----------------------------------------------------------
void VatTableNode::setBrush(const QVariant &brush)
{
    m_brush = brush;
}
//----------------------------------------------------------
//----------------------------------------------------------
QList<VatTableNode *> VatTableNode::children() const
{
    return m_children;
}
//----------------------------------------------------------
//----------------------------------------------------------
void VatTableNodeVatCountry::setValues(const QVector<double> &values)
{
    m_values = values;
}
//----------------------------------------------------------
//----------------------------------------------------------
VatTableNodeRegime::VatTableNodeRegime(
        const QString &title, VatTableNode *parent)
    : VatTableNode (title, parent)
{
}
//----------------------------------------------------------
//----------------------------------------------------------
VatTableNodeVatCountry::VatTableNodeVatCountry(
        const QString &title, const QVector<double> &values, VatTableNode *parent)
    : VatTableNode(title, parent)
{
    setValues(values);
}
//----------------------------------------------------------
//----------------------------------------------------------
QVariant VatTableNodeVatCountry::data(int column, int role) const
{
    if (role == Qt::DisplayRole && column > 0) {
        return QString::number(value(column-1), 'f', 2);
    }
    return VatTableNode::data(column, role);
}
//----------------------------------------------------------
//----------------------------------------------------------
VatTableNodeVatDetails::VatTableNodeVatDetails(
        const QString &title,
        const QVector<double> &values,
        VatTableNode *parent)
    : VatTableNodeVatCountry(title, values, parent)
{
}
//----------------------------------------------------------
//----------------------------------------------------------
