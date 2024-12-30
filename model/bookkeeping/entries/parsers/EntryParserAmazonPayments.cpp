#include <QDebug>
#include <QSettings>

#include "../common/countries/CountryManager.h"

#include "model/SettingManager.h"
#include "model/CustomerManager.h"
#include "EntryParserAmazonPayments.h"
#include "EntryParserAmazonPaymentsTable.h"
#include "EntryParserAmazonOrdersMonthly.h"
#include "model/orderimporters/ImporterYearsManager.h"
#include "model/orderimporters/ImportedFileReportManager.h"
#include "model/orderimporters/OrderImporterAmazonUE.h"
#include "model/orderimporters/OrderImporterAmazon.h"
#include "model/bookkeeping/ManagerAccountsAmazon.h"
#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/reports/ReportMonthlyAmazon.h"
#include "EntryParserMarketplaceMonthly.h"

using COL = EntryParserAmazonPaymentsTable;

QString EntryParserAmazonPayments::NAME
= QObject::tr("Paiements amazon");
//----------------------------------------------------------
EntryParserAmazonPayments::EntryParserAmazonPayments()
{
    loadReplacedInfos();
}
//----------------------------------------------------------
EntryParserAmazonPayments::~EntryParserAmazonPayments()
{
}
//----------------------------------------------------------
void EntryParserAmazonPayments::clearTransactions()
{
}
//----------------------------------------------------------
void EntryParserAmazonPayments::recordTransactions(
        const Shipment *)
{
}
//----------------------------------------------------------
AccountingEntries EntryParserAmazonPayments::entries(int year) const
{
    AccountingEntries allEntries;
    QList<QStringList> reportInfos;
    reportInfos << QStringList({OrderImporterAmazonUE::NAME,
                           OrderImporterAmazonUE::REPORT_ORDERS_PAYMENTS});
    reportInfos << QStringList({OrderImporterAmazon::NAME,
                           OrderImporterAmazon::REPORT_ORDERS_PAYMENTS});
    QString journal =
                ManagerEntryTables::instance()->journalName(
                this->name());
    for (auto reportInfo : reportInfos) {
        QString importer = reportInfo[0];
        QString reportType = reportInfo[1];
        QStringList filePaths
                = ImportedFileReportManager::instance()->filePaths(
                    importer,
                    reportType,
                    year,
                    true);
        /*
        QStringList fileNames;
        for (auto filePath : filePaths) {
            fileNames << QFileInfo(filePath).fileName();
        }
        std::sort(fileNames.begin(), fileNames.end());
        //*/
        for (auto filePath : filePaths) {
            QString fileName = QFileInfo(filePath).fileName();
            QString amazon = _findAmazon(filePath);
            CsvReader reader
                    = OrderImporterAmazonUE().createAmazonReader(filePath);
            if (reader.readAll()) {
                const DataFromCsv *dataRode = reader.dataRode();
                int indSettlementId = dataRode->header.pos("settlement-id");
                int indDateStart = dataRode->header.pos("settlement-start-date");
                int indDateEnd = dataRode->header.pos("settlement-end-date");
                int indCurrency = dataRode->header.pos("currency");
                int indChargedAmount = dataRode->header.pos("total-amount");
                int indPostedDateTime = dataRode->header.pos("posted-date-time");
                int indOrderId = dataRode->header.pos("order-id");
                auto firstLine = dataRode->lines[0];
                QString settlementId = firstLine[indSettlementId];
                QString currency = firstLine[indCurrency];
                QString dateTimeEndString = firstLine[indDateEnd];
                QString dateTimeStartString = firstLine[indDateStart];
                QDateTime dateTimeEnd = OrderImporterAmazonUE::dateTimeFromString(
                            dateTimeEndString);
                QDateTime dateTimeStart = OrderImporterAmazonUE::dateTimeFromString(
                            dateTimeStartString);
                Q_ASSERT(dateTimeEnd.isValid());
                QString chargedAmountStr = firstLine[indChargedAmount];
                double chargedAmount = chargedAmountStr.replace(",", ".").toDouble();
                double fees = 0.;
                QList<EntryParserMarketplaceMonthly::BalanceAmount> detailFees;
                double sales = 0.;
                QList<EntryParserMarketplaceMonthly::BalanceAmount> detailSales;
                double reservePrevious = 0.;
                QList<EntryParserMarketplaceMonthly::BalanceAmount> detailReserve;
                double reserveCurrent = 0.;
                double payableToAmazon = 0.;
                int indType = dataRode->header.pos("amount-type");
                int indDescription = dataRode->header.pos("amount-description");
                int indAmount = dataRode->header.pos("amount");
                for (int i=1; i<dataRode->lines.size(); ++i) {
                    auto elements = dataRode->lines[i];
                    if (elements.size() > 0) {
                        QString type = elements[indType];
                        QString description = elements[indDescription];
                        QString amountStr = elements[indAmount].replace(",", ".");
                        double amount = amountStr.toDouble();
                        QString dateString = elements[indPostedDateTime];
                        EntryParserMarketplaceMonthly::BalanceAmount infos;
                        infos.row = QString::number(i+2);
                        infos.date = OrderImporterAmazonUE::dateTimeFromString(
                                    dateString).date();
                        infos.title = type + " " + description;
                        infos.amount = amount;
                        infos.orderId = elements[indOrderId];
                        infos.reportId = settlementId;
                        if (settlementId.contains("23389301832"))
                        {
                            int TEMP=10;++TEMP;
                        }
                        infos.reportFileName = ""; /// fileName; /// It takes too much space
                        if (_isSales(type, description)) {
                            sales += amount;
                            detailSales.append(infos);
                        } else if (_isCurrentReserve(type, description)) {
                            reserveCurrent += amount;
                            detailReserve.append(infos);
                        } else if (_isPreviousReserve(type, description)) {
                            reservePrevious += amount;
                            detailReserve.append(infos);
                        } else if (_isPayableAmazon(type, description)) {
                            reservePrevious += amount;
                            detailReserve.append(infos);
                        } else if (_isFees(type, description)) {
                            fees += amount;
                            detailFees.append(infos);
                        }
                    }
                }
                auto amazonAccount = ManagerAccountsAmazon::instance()
                        ->amazonAccount(amazon);
                QSharedPointer<AccountingEntrySet> entrySet(
                            new AccountingEntrySet(AccountingEntrySet::SaleMarketplace));
                QString id = amazon + "-" + settlementId + dateTimeEnd.toString("-yyyy-MM-dd");
                ReportMonthlyAmazon reportGenerator;
                QString reportTitle = QObject::tr("Paiement") + " " + amazon
                        + " - " + settlementId + " - "
                        + dateTimeStart.toString("yyyy-MM-dd")
                        + " => " + dateTimeEnd.toString("yyyy-MM-dd")
                        + " (" + fileName + ")";
                reportTitle.replace(".??", "");
                reportGenerator.addTitle(reportTitle);
                //if (settlementId.contains("16788388272")) {
                    //int TEMP=10;++TEMP;
                //}

                double total = sales + fees - chargedAmount + reserveCurrent + reservePrevious + payableToAmazon;
                Q_ASSERT(qAbs(total) < 0.01);
                entrySet->setId(id);
                entrySet->setAmountOrig(-chargedAmount);
                entrySet->setCurrencyOrig(currency);
                QString label = QObject::tr("Paiement")
                        + " " + amazon + " " + chargedAmountStr + " " + currency;
                if (label == "Paiement amazon.se 167.28 SEK") {
                    int TEMP=10;++TEMP;
                }
                QString titleDetail = QObject::tr("Détails du compte") + " ";
                AccountingEntry entryDebitCustomer;
                entryDebitCustomer.setJournal(journal);
                entryDebitCustomer.setLabel(label);
                entryDebitCustomer.setDate(dateTimeEnd.date());
                entryDebitCustomer.setCurrency(currency);
                AccountingEntry entryCreditSales = entryDebitCustomer;
                AccountingEntry entryDebitCharges = entryDebitCustomer;
                AccountingEntry entryCreditBalancePrevious = entryDebitCustomer;
                AccountingEntry entryDebitBalanceCurrent = entryDebitCustomer;

                entryDebitCustomer.setAccount(amazonAccount.client);
                entryDebitCustomer.setType("client");
                if (chargedAmount > 0) {
                    entryDebitCustomer.setDebitOrig(chargedAmount);
                } else {
                    entryDebitCustomer.setCreditOrig(-chargedAmount);
                }
                if (qAbs(chargedAmount) >= 0.01) {
                    entrySet->addEntry(entryDebitCustomer);
                }

                /// SALES
                entryCreditSales.setAccount(amazonAccount.salesUnknown);
                entryDebitCustomer.setType("salesUnknown");
                if (sales > 0) {
                    entryCreditSales.setCreditOrig(sales);
                } else {
                    entryCreditSales.setDebitOrig(-sales);
                }
                if (qAbs(sales) >= 0.01) {
                    entrySet->addEntry(entryCreditSales);
                }
                QString titleSales = titleDetail + entryCreditSales.account();
                if (currency == CustomerManager::instance()->getSelectedCustomerCurrency()) {
                    titleSales += " (" + entryCreditSales.amountOrig()
                            + " " + CustomerManager::instance()->getSelectedCustomerCurrency() + ")";
                } else {
                    titleSales += " (" + entryCreditSales.amountOrig()
                            + currency + " = "
                            + entryCreditSales.amountConv()
                            + " " + CustomerManager::instance()->getSelectedCustomerCurrency() + ")";
                }
                reportGenerator.addTitle(titleSales);
                reportGenerator.addTableNonSale();
                for (auto it = detailSales.begin(); it != detailSales.end(); ++it) {
                    reportGenerator.addTableNonSaleRow(
                                it->reportFileName,
                                it->reportId,
                                it->title,
                                it->orderId,
                                it->date,
                                it->row,
                                it->amount);
                }
                reportGenerator.addTableNonSaleTotal(sales);

                /// FEES
                entryDebitCharges.setAccount(amazonAccount.supplier);
                entryDebitCharges.setType("fees");
                if (fees > 0) {
                    entryDebitCharges.setCreditOrig(fees);
                } else {
                    entryDebitCharges.setDebitOrig(-fees);
                }
                if (qAbs(fees) >= 0.01) {
                    entrySet->addEntry(entryDebitCharges);
                }
                QString titleFees = titleDetail + entryDebitCharges.account();
                if (currency == CustomerManager::instance()->getSelectedCustomerCurrency()) {
                    titleFees += " (" + entryDebitCharges.amountOrig()
                            + " " + CustomerManager::instance()->getSelectedCustomerCurrency() + ")";
                } else {
                    titleFees += " (" + entryDebitCharges.amountOrig()
                            + currency + " = "
                            + entryDebitCharges.amountConv()
                            + " " + CustomerManager::instance()->getSelectedCustomerCurrency() + ")";
                }
                reportGenerator.addTitle(titleFees);

                reportGenerator.addTableNonSale();
                for (auto it = detailFees.begin(); it != detailFees.end(); ++it) {
                    reportGenerator.addTableNonSaleRow(
                                it->reportFileName,
                                it->reportId,
                                it->title,
                                it->orderId,
                                it->date,
                                it->row,
                                it->amount);
                }
                reportGenerator.addTableNonSaleTotal(fees);

                /// Reserve
                entryCreditBalancePrevious.setAccount(amazonAccount.reserve);
                entryCreditBalancePrevious.setType("reserve");
                if (reservePrevious > 0) {
                    entryCreditBalancePrevious.setCreditOrig(reservePrevious);
                } else {
                    entryCreditBalancePrevious.setDebitOrig(-reservePrevious);
                }
                if (qAbs(reservePrevious) >= 0.01) {
                    entrySet->addEntry(entryCreditBalancePrevious);
                }

                entryDebitBalanceCurrent.setAccount(amazonAccount.reserve);
                entryDebitBalanceCurrent.setType("reserve");
                if (reserveCurrent < 0) {
                    entryDebitBalanceCurrent.setDebitOrig(-reserveCurrent);
                } else {
                    entryDebitBalanceCurrent.setCreditOrig(reserveCurrent);
                }
                if (qAbs(reserveCurrent) >= 0.01) {
                    entrySet->addEntry(entryDebitBalanceCurrent);
                }
                reportGenerator.addTitle(titleDetail + amazonAccount.reserve);
                reportGenerator.addTableNonSale();
                double totalReserve = 0.;
                for (auto it = detailReserve.begin(); it != detailReserve.end(); ++it) {
                    reportGenerator.addTableNonSaleRow(
                                it->reportFileName,
                                it->reportId,
                                it->title,
                                it->orderId,
                                it->date,
                                it->row,
                                it->amount);
                    totalReserve += it->amount;
                }
                reportGenerator.addTableNonSaleTotal(totalReserve);
                reportGenerator.endHtml();
                QString baseFileName = id + "__" + QString::number(chargedAmount, 'f', 2) + currency;
                entrySet->setHtmlDocument(reportGenerator.html(),
                                          baseFileName);


                if (entrySet->entries().size() > 0) {
                    _updateEntrySetFromReplacedInfo(entrySet);
                    double diff = entrySet->creditTotalConv()-entrySet->debitTotalConv();
                    if (qAbs(diff) > 0.009) {
                        entrySet->setState(AccountingEntrySet::Error);
                    }
                    //entrySet->roundCreditDebit();
                    addEntryStatic(allEntries, entrySet);
                }
            }
        }
    }
    return allEntries;
}
//----------------------------------------------------------
AbstractEntryParser::Type EntryParserAmazonPayments::typeOfEntries() const
{
    return SaleMarketplace;
}
//----------------------------------------------------------
QString EntryParserAmazonPayments::name() const
{
    return NAME;
}
//----------------------------------------------------------
QString EntryParserAmazonPayments::journal() const
{
    return EntryParserAmazonOrdersMonthly::JOURNAL;
}
//----------------------------------------------------------
bool EntryParserAmazonPayments::isReplacedInfo(
        const QString &entrySetId, const QString &colName) const
{
    return m_replacedInfo.contains(entrySetId)
            && m_replacedInfo[entrySetId].contains(colName);
}
//----------------------------------------------------------
QVariant EntryParserAmazonPayments::replacedInfo(
        const QString &entrySetId, const QString &colName) const
{
    return m_replacedInfo[entrySetId][colName];
}
//----------------------------------------------------------
bool EntryParserAmazonPayments::isAmountPaidReplaced(
        const QString &entrySetId) const
{
    return isReplacedInfo(
                entrySetId,
                EntryParserAmazonPaymentsTable::COL_AMOUNT_PAID_CONV_CURRENCY);
}
//----------------------------------------------------------
double EntryParserAmazonPayments::amountPaid(const QString &entrySetId) const
{
    return replacedInfo(
                entrySetId,
                EntryParserAmazonPaymentsTable::COL_AMOUNT_PAID_CONV).toDouble();
}
//----------------------------------------------------------
void EntryParserAmazonPayments::replaceInfo(
        QSharedPointer<AccountingEntrySet> entrySet, const QString &colName, const QVariant &value)
{
    QString entrySetId = entrySet->id();
    if (!m_replacedInfo.contains(entrySetId)) {
        m_replacedInfo[entrySetId] = QHash<QString, QVariant>();
    }
    m_replacedInfo[entrySetId][colName] = value;
    _updateEntrySetFromReplacedInfo(entrySet);
    _saveReplacedInfos();
}
//----------------------------------------------------------
void EntryParserAmazonPayments::loadReplacedInfos()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(_settingKey())) {
        QString string = settings.value(_settingKey()).toString();
        for (auto line : string.split(":::")) {
            QStringList elements = line.split(";;;");
            if (!m_replacedInfo.contains(elements[0])) {
                m_replacedInfo[elements[0]]
                        = QHash<QString, QVariant>();
            }
            if (elements[1] == EntryParserAmazonPaymentsTable::COL_AMOUNT_PAID_CONV_CURRENCY) {
                m_replacedInfo[elements[0]][elements[1]] = elements[2];
            } else {
                m_replacedInfo[elements[0]][elements[1]] = elements[2].toDouble();
            }
        }
    }
}
//----------------------------------------------------------
void EntryParserAmazonPayments::_updateEntrySetFromReplacedInfo(
        QSharedPointer<AccountingEntrySet> entrySet) const
{
    if (entrySet->label() == "Paiement amazon.se 167.28 SEK") {
        int TEMP=10;++TEMP;
    }
    QString entrySetId = entrySet->id();
    AccountingEntry *entryDebitCustomer = nullptr;
    int posDebitCustomer = -1;
    AccountingEntry *entryCreditSales = nullptr;
    int posCreditSales = -1;
    AccountingEntry *entryDebitFees = nullptr;
    int posDebitFees = -1;
    AccountingEntry *entryCreditBalancePrevious = nullptr;
    int posCreditBalancePrevious = -1;
    AccountingEntry *entryDebitBalanceCurrent = nullptr;
    int posDebitBalanceCurrent = -1;
    QString labelLower = entrySet->label().toLower();
    QStringList elementsLabel = labelLower.split(" ");
    elementsLabel.takeFirst();
    while (!elementsLabel[0].startsWith("amazon")) {
        elementsLabel.takeFirst();
    }
    QString amazon = elementsLabel[0];
    auto amazonAccount = ManagerAccountsAmazon::instance()
            ->amazonAccount(amazon);
    int id = 0;
    for (auto entry = entrySet->entriesBegin();
         entry != entrySet->entriesEnd(); ++entry) {
        //if (entry->account() == amazonAccount.client) {
        if (entry->type() == "client") {
            entryDebitCustomer = &(*entry);
            posDebitCustomer = id;
        //} else if (entry->account() == amazonAccount.salesUnknown) {
        } else if (entry->type() == "salesUnknown") {
            entryCreditSales = &(*entry);
            posCreditSales = id;
        //} else if (entry->account() == amazonAccount.fees) {
        } else if (entry->type() == "fees") {
            entryDebitFees = &(*entry);
            posDebitFees = id;
        //} else if (entry->account() == amazonAccount.reserve) {
        } else if (entry->type() == "reserve") {
            if (entry->debitOrig().isEmpty()) { /// previous reserve
                entryCreditBalancePrevious = &(*entry);
                posCreditBalancePrevious = id;
            } else { /// current reserve
                entryDebitBalanceCurrent = &(*entry);
                posDebitBalanceCurrent = id;
            }
        }
        ++id;
    }

    AccountingEntry entryBase;
    entryBase.setJournal(entrySet->journal());
    entryBase.setLabel(entrySet->label());
    entryBase.setDate(entrySet->date());
    entryBase.setCurrency(entrySet->currencyOrig());

    bool toAddEntryDebitClient = false;
    AccountingEntry entryDebitClientNew = entryBase;

    QPair<bool, double> rateConvChanged = {false, 1.};
    if (isReplacedInfo(entrySetId, COL::COL_AMOUNT_PAID_CONV)) {
        entrySet->setState(AccountingEntrySet::Edited);
        double paid = replacedInfo(entrySetId, COL::COL_AMOUNT_PAID_CONV).toDouble();
        double amoundOrig = entrySet->amountOrig();
        entrySet->setAmountOrig(paid);
        rateConvChanged.first = isReplacedInfo(entrySetId, COL::COL_AMOUNT_PAID_CONV_CURRENCY);
        rateConvChanged.second = paid / amoundOrig;
        if (entryDebitCustomer == nullptr) {
            if (qAbs(paid) >= 0.01) {
                entryDebitClientNew = entryBase;
                entryDebitClientNew.setAccount(amazonAccount.client);
                entryDebitClientNew.setType("client");
                if (paid > 0.) {
                    entryDebitClientNew.setCreditOrig(paid);
                } else {
                    entryDebitClientNew.setDebitOrig(-paid);
                }
            }
        } else {
            entryDebitCustomer->setDebitOrig("");
            entryDebitCustomer->setCreditOrig("");
            if (qAbs(paid) < 0.001) {
                entrySet->removeEntry(posDebitCustomer);
                entryDebitCustomer = nullptr;
            } else if (paid > 0.) {
                entryDebitCustomer->setCreditOrig(paid);
            } else {
                //entryDebitClientNew.setRateForConversion(rateForConversion);
                entryDebitCustomer->setDebitOrig(-paid);
            }
        }
    }
    if (isReplacedInfo(entrySetId, COL::COL_AMOUNT_PAID_CONV_CURRENCY)) {
        entrySet->setState(AccountingEntrySet::Edited);
        QString currency = replacedInfo(entrySetId, COL::COL_AMOUNT_PAID_CONV_CURRENCY).toString();
        entrySet->setCurrencyOrig(currency);
        if (entryDebitCustomer != nullptr) {
            entryDebitCustomer->setCurrency(currency);
        }
        entryDebitClientNew.setCurrency(currency);
    }
    if (toAddEntryDebitClient) {
        entrySet->insertEntry(0, entryDebitClientNew);
    }

    if (isReplacedInfo(entrySetId, COL::COL_AMOUNT_FEES_ORIG)) {
        entrySet->setState(AccountingEntrySet::Edited);
        double fees = replacedInfo(entrySetId, COL::COL_AMOUNT_FEES_ORIG).toDouble();
        if (entryDebitFees == nullptr) {
            if (qAbs(fees) >= 0.01) {
                AccountingEntry entryDebitFeestNew = entryBase;
                entryDebitFeestNew.setAccount(entryDebitFees->account());
                entryDebitFeestNew.setType("fees");
                if (fees > 0.) {
                    entryDebitClientNew.setDebitOrig(fees);
                } else {
                    entryDebitClientNew.setCreditOrig(-fees);
                }
                entrySet->addEntry(entryDebitClientNew);
            }
        } else {
            entryDebitFees->setDebitOrig("");
            entryDebitFees->setCreditOrig("");
            if (qAbs(fees) < 0.001) {
                entrySet->removeEntry(posDebitFees);
                entryDebitFees = nullptr;
            } else if (fees > 0.) {
                entryDebitFees->setDebitOrig(fees);
            } else {
                entryDebitFees->setCreditOrig(-fees);
            }
        }
    }

    if (isReplacedInfo(entrySetId, COL::COL_AMOUNT_SALES_ORIG)) {
        entrySet->setState(AccountingEntrySet::Edited);
        double sales = replacedInfo(entrySetId, COL::COL_AMOUNT_SALES_ORIG).toDouble();
        if (entryCreditSales == nullptr) {
            if (qAbs(sales) >= 0.01) {
                AccountingEntry entryCreditSalesNew = entryBase;
                entryCreditSalesNew.setAccount(amazonAccount.salesUnknown);
                entryCreditSalesNew.setType("salesUnknown");
                if (sales > 0.) {
                    entryCreditSalesNew.setCreditOrig(sales);
                } else {
                    entryCreditSalesNew.setDebitOrig(-sales);
                }
                entrySet->addEntry(entryCreditSalesNew);
            }
        } else {
            entryCreditSales->setDebitOrig("");
            entryCreditSales->setCreditOrig("");
            if (qAbs(sales) < 0.001) {
                entrySet->removeEntry(posCreditSales);
                entryCreditSales = nullptr;
            } else if (sales > 0.) {
                entryCreditSales->setCreditOrig(sales);
            } else {
                entryCreditSales->setDebitOrig(-sales);
            }
        }
    }

    if (isReplacedInfo(entrySetId, COL::COL_AMOUNT_RESERVE_PREVIOUS_ORIG)) {
        entrySet->setState(AccountingEntrySet::Edited);
        double reserve = replacedInfo(
                    entrySetId,
                    COL::COL_AMOUNT_RESERVE_PREVIOUS_ORIG).toDouble();
        if (entryCreditBalancePrevious == nullptr) {
            if (qAbs(reserve) >= 0.01) {
                AccountingEntry entryCreditBalancePreviousNew = entryBase;
                entryCreditBalancePreviousNew.setAccount(amazonAccount.reserve);
                entryCreditBalancePreviousNew.setAccount("reserve");
                if (reserve > 0.) {
                    entryCreditBalancePreviousNew.setCreditOrig(reserve);
                } else {
                    entryCreditBalancePreviousNew.setDebitOrig(-reserve);
                }
                entrySet->addEntry(entryCreditBalancePreviousNew);
            }
        } else {
            entryCreditBalancePrevious->setDebitOrig("");
            entryCreditBalancePrevious->setCreditOrig("");
            if (qAbs(reserve) < 0.001) {
                entrySet->removeEntry(posCreditBalancePrevious);
                entryCreditSales = nullptr;
            } else if (reserve > 0.) {
                entryCreditBalancePrevious->setCreditOrig(reserve);
            } else {
                entryCreditBalancePrevious->setDebitOrig(-reserve);
            }
        }
    }

    if (isReplacedInfo(entrySetId, COL::COL_AMOUNT_RESERVE_CURRENT_ORIG)) {
        entrySet->setState(AccountingEntrySet::Edited);
        double reserve = replacedInfo(
                    entrySetId,
                    COL::COL_AMOUNT_RESERVE_CURRENT_ORIG).toDouble();
        if (entryDebitBalanceCurrent == nullptr) {
            if (qAbs(reserve) >= 0.01) {
                AccountingEntry entryDebitBalanceCurrentNew = entryBase;
                entryDebitBalanceCurrentNew.setAccount(amazonAccount.reserve);
                entryDebitBalanceCurrentNew.setType("reserve");
                if (reserve > 0.) {
                    entryDebitBalanceCurrentNew.setDebitOrig(reserve);
                } else {
                    entryDebitBalanceCurrentNew.setCreditOrig(-reserve);
                }
                entrySet->addEntry(entryDebitBalanceCurrentNew);
            }
        } else {
            entryDebitBalanceCurrent->setDebitOrig("");
            entryDebitBalanceCurrent->setCreditOrig("");
            if (qAbs(reserve) < 0.001) {
                entrySet->removeEntry(posDebitBalanceCurrent);
                entryCreditSales = nullptr;
            } else if (reserve > 0.) {
                entryDebitBalanceCurrent->setDebitOrig(reserve);
            } else {
                entryDebitBalanceCurrent->setCreditOrig(-reserve);
            }
        }
    }
    if (rateConvChanged.first) {
        entrySet->setRateForConversion(rateConvChanged.second);
    }
    //entrySet->roundCreditDebit();
}
//----------------------------------------------------------
void EntryParserAmazonPayments::_saveReplacedInfos()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (m_replacedInfo.size() > 0) {
        QStringList lines;
        for (auto it1 = m_replacedInfo.begin();
             it1 != m_replacedInfo.end(); ++it1) {
            for (auto it2 = it1.value().begin();
                 it2 != it1.value().end(); ++it2) {
                QStringList elements
                        = {it1.key(),
                           it2.key(),
                          it2.value().toString()};
                lines << elements.join(";;;");
            }
        }
        settings.setValue(_settingKey(),
                          lines.join(":::"));
    } else if (settings.contains(_settingKey())) {
        settings.remove(_settingKey());
    }
}
//----------------------------------------------------------
QString EntryParserAmazonPayments::_settingKey()
{
    return "EntryParserAmazonPayments___settingKey-"
            + CustomerManager::instance()->getSelectedCustomerId();
}
//----------------------------------------------------------
QString EntryParserAmazonPayments::_findAmazon(const QString &filePath) const
{
    QString amazon = "amazon.??";
    CsvReader reader = OrderImporterAmazonUE().createAmazonReader(filePath);
    if (reader.readSomeLines(2)) {
        auto dataRode = reader.dataRode();
        int indexCurrency = dataRode->header.pos("currency");
        QString currency = dataRode->lines.first()[indexCurrency];
        reader.removeFirstLine();
        int indexAmazon = dataRode->header.pos("marketplace-name");
        QString value;
        while (value.isEmpty() && dataRode->lines.size() > 0) {
            auto elements = reader.takeFirstLine();
            value = elements[indexAmazon];
            if (!value.isEmpty()) {
                amazon = value.toLower();
                break;
            }
        }
        if (amazon.endsWith("?")) {
            if (currency == "GBP") { // TODO factorize somewhere amazon information
                amazon = "amazon.co.uk";
            } else if (currency == "SEK") {
                amazon = "amazon.se";
            } else if (currency == "PLN") {
                amazon = "amazon.pl";
            } else if (currency == "USD") {
                amazon = "amazon.com";
            } else if (currency == "CAD") {
                amazon = "amazon.ca";
            } else if (currency == "JPY") {
                amazon = "amazon.jp";
            } else if (currency == "MXN") {
                amazon = "amazon.com.mx";
            } else if (currency == "INR") {
                amazon = "amazon.in";
            }
        }
    }
    return amazon;
}
//----------------------------------------------------------
bool EntryParserAmazonPayments::_isAmazonUE(const QString &amazon, int year) const
{
    static QHash<QString, QString> amazonToCountryCode
            = {{"amazon.fr", "FR"}
              , {"amazon.de", "DE"}
              , {"amazon.es", "ES"}
              , {"amazon.it", "IT"}
              , {"amazon.nl", "NL"}
              , {"amazon.se", "SE"}
              , {"amazon.pl", "PL"}
              , {"amazon.com.be", "BE"}
              , {"amazon.com.tr", "TR"}
              , {"amazon.co.uk", "UK"}
              }; // TODO adds more and Q_ASSERT on amazon to make sure nothing is forgoten
    QString countryCode = amazonToCountryCode.value(amazon, "");
    bool is = CountryManager::instance()->countriesCodeUE(
                year)->contains(countryCode);
    return is;
}
//----------------------------------------------------------
bool EntryParserAmazonPayments::_isSales(
        const QString &type, const QString &) const
{
    if (type == "ItemPrice"
            || type == "ItemWithheldTax"
            || type == "Promotion") {
        return true;
    }
    return false;
}
//----------------------------------------------------------
bool EntryParserAmazonPayments::_isFees(
        const QString &type, const QString &description) const
{
    bool is = !_isSales(type, description)
            && !_isCurrentReserve(type, description)
            && !_isPreviousReserve(type, description);
    return is;
}
//----------------------------------------------------------
bool EntryParserAmazonPayments::_isPayableAmazon(
        const QString &, const QString &description) const
{
    if (description.contains("Payable to Amazon")
            || description.contains("Successful charge")) {
        return true;
    }
    return false;
}
//----------------------------------------------------------
bool EntryParserAmazonPayments::_isCurrentReserve(
        const QString &, const QString &description) const
{
    bool is = description.contains("Current Reserve Amount")
            || description.toLower().contains("current reserve");
    return is;
}
//----------------------------------------------------------
bool EntryParserAmazonPayments::_isPreviousReserve(
        const QString &, const QString &description) const
{
    if (description == "Previous Reserve Amount Balance"
            || description.contains("Transfer of funds unsuccessful")
            || description.toLower().contains("previous reserve")
            //|| description == "Payable to Amazon."
//            || description == "Successful charge."
            ) {
        return true;
    }
    return false;
}
//----------------------------------------------------------
