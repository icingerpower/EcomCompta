#include "../common/currencies/CurrencyRateManager.h"
#include "../common/countries/CountryManager.h"
#include "../common/utils/FileUtils.h"

#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/bookkeeping/ManagerAccountPurchase.h"
#include "model/CustomerManager.h"
#include "model/SettingManager.h"
#include "model/vat/VatRateManager.h"
#include "EntryParserPurchases.h"
#include "EntryParserImportationsTable.h"

#include "EntryParserImportations.h"

//----------------------------------------------------------
EntryParserImportations::EntryParserImportations()
    : AbstractEntryParser ()
{
}
//----------------------------------------------------------
AbstractEntryParser::Type EntryParserImportations::typeOfEntries() const
{
    return Purchase;
}
//----------------------------------------------------------
QString EntryParserImportations::name() const
{
    return EntryParserPurchases().name();
}
//----------------------------------------------------------
QString EntryParserImportations::journal() const
{
    return EntryParserPurchases().journal();
}
//----------------------------------------------------------
QSharedPointer<AccountingEntrySet> EntryParserImportations::addInvoice(
        const ImportInvoiceInfo &invoiceInfo)
{
    QString finalFileName = invoiceInfoToFileName(
                invoiceInfo);
    QDir dir = SettingManager::instance()->bookKeepingDirPurchaseImport(
                invoiceInfo.date.year(), invoiceInfo.date.month());
    QString absfileNameCopy = dir.filePath(finalFileName);
    bool exist = FileUtils::fileContentExisting(
                invoiceInfo.absFileName, dir);
    if (exist) {
        EntryParserPurchasesException exception;
        exception.setError(QObject::tr("Le fichier existe déjà. Il a déjà été importé: ") + finalFileName);
        exception.raise();
    }
    QFile::copy(invoiceInfo.absFileName, absfileNameCopy);
    QSharedPointer<AccountingEntrySet> entrySet
            = entrySetFromFileName(finalFileName);
    return entrySet;
}
//----------------------------------------------------------
void EntryParserImportations::removeInvoice(
        int year, int month, const QString &id) const
{
    QDir dir = SettingManager::instance()
            ->bookKeepingDirPurchaseImport(year, month);
    auto entryFileInfo = dir.entryInfoList(
                QStringList() << "*"+id+"*", QDir::Files);
    if (entryFileInfo.size() > 0) {
        QFile::remove(entryFileInfo.first()
                      .absoluteFilePath());
    } else {
        Q_ASSERT(false);
    }
}
//----------------------------------------------------------
QSharedPointer<AccountingEntrySet> EntryParserImportations::updateInvoice(
        const ImportInvoiceInfo &invoiceInfos) const
{
    QDir dir = SettingManager::instance()->bookKeepingDirPurchaseImport();
    QString invoiceFilePathFrom = dir.path() + invoiceInfos.absFileName;
    QString invoiceFileNameTo = invoiceInfoToFileName(invoiceInfos);
    QDir dirTo = SettingManager::instance()->bookKeepingDirPurchaseImport(
                invoiceInfos.date.year(),
                invoiceInfos.date.month());
    QString invoiceFilePathTo = dirTo.filePath(invoiceFileNameTo);
    QFile::copy(invoiceFilePathFrom, invoiceFilePathTo);
    QFile::remove(invoiceFilePathFrom);
    QSharedPointer<AccountingEntrySet> entrySet = entrySetFromFileName(
                invoiceFileNameTo);
    return entrySet;
}
//----------------------------------------------------------
QSharedPointer<AccountingEntrySet> EntryParserImportations::entrySetFromFileName(
        const QString &relFileName) const
{
    // TODO I stop here
    Q_ASSERT(false);
    QFileInfo infoFileName(relFileName);
    QSharedPointer<AccountingEntrySet> entrySet(
                new AccountingEntrySet(AccountingEntrySet::Purchase)); //AccountingEntrySet::OtherToBank));
    QStringList elementsFileNames = infoFileName.fileName().split(".");
    elementsFileNames.takeLast();
    QString baseName = elementsFileNames.join(".");
    QStringList elements = baseName.split("__");
    QDate date = QDate::fromString(elements[0], "yyyy-MM-dd");
    QString account;
    bool isNumber = false;
    int i = 1;
    for (i = 1; i < elements.size(); ++i) {
        int accountInt = elements[i].toInt(&isNumber);
        if (isNumber) {
            account = QString::number(accountInt);
            break;
        }
    }
    QString amountTaxed = elements.takeLast();
    QString currency = amountTaxed.right(3);
    amountTaxed = amountTaxed.left(amountTaxed.size()-3);
    double amountTaxedDouble = amountTaxed.toDouble();

    QString currencyVat = currency;
    QString countryCodeVat;
    double amountVatDouble = 0.;
    QString amountVat;
    if (elements.last().contains("TVA")) {
        QStringList vatElements = elements.last().split("-");
        QString amountVatString = vatElements.last();
        countryCodeVat = vatElements[0];
        currencyVat = elements.last().right(3);
        amountVat = amountVatString.left(amountVatString.size()-3);
        amountVatDouble = amountVat.toDouble();
    }

    /*
    double amountTaxedDoubleConv = amountTaxedDouble;
    double amountVatDoubleConv = amountVatDouble;
    QString bookKeepingCur = SettingManager::instance()->currency();
    if (currency != SettingManager::instance()->currency()) {
        double rate = CurrencyRateManager::instance()->rate(
                    currency, bookKeepingCur, date);
        amountTaxedDoubleConv = amountTaxedDouble * rate;
    }
    if (qAbs(amountVatDouble) > 0.001
            && currencyVat != SettingManager::instance()->currency()) {
        double rate = CurrencyRateManager::instance()->rate(
                    currencyVat, bookKeepingCur, date);
        amountVatDoubleConv = amountVatDouble * rate;

    }
    double amountUntaxedDoubleConv = amountTaxedDoubleConv - amountVatDoubleConv;
    //*/
    double amountUntaxedDouble = amountTaxedDouble - amountVatDouble;

    ++i;
    QString label = elements[i].replace("-", " ");
    if (!amountVat.isEmpty() && currencyVat != CustomerManager::instance()->getSelectedCustomerCurrency()) {
        label = QObject::tr("TVA :") + amountVat + " " + currencyVat + " " + label;
    }
    if (currency != CustomerManager::instance()->getSelectedCustomerCurrency()) {
        label = amountTaxed + " " + currency + " " + label;
    }
    ++i;
    QString supplier = elements[i];
    ++i;
    QString purchaseId;
    if (elements[i].contains("ID-")) {
        purchaseId = elements[i].split("ID-")[1];
    } else {
        purchaseId = EntryParserPurchases::genPurchaseId(date, label, amountTaxed, currency);
    }
    entrySet->setId(purchaseId);
    entrySet->setCurrencyOrig(currency);
    entrySet->setAmountOrig(amountTaxedDouble);

    AccountingEntry entryCredit;
    entryCredit.setLabel(label);
    entryCredit.setDate(date);
    entryCredit.setJournal(
                ManagerEntryTables::instance()->journalName(
                    ManagerEntryTables::instance()->namePurchases()));
    entryCredit.setMain(true);
    entryCredit.setCurrency(currency);
    QString filePathMinusWorkingDir = infoFileName.filePath();
    QString newAbsfilePath = filePathMinusWorkingDir;
    QDir workingDir = SettingManager::instance()->bookKeepingDirPurchaseImport();
    filePathMinusWorkingDir.replace(workingDir.absolutePath(), "");
    entryCredit.setFileRelWorkingDir(filePathMinusWorkingDir);
    entrySet->setSourceDocumentDisplay(newAbsfilePath);

    AccountingEntry entryDebit = entryCredit;
    AccountingEntry entryDebitVat = entryCredit;
    entryCredit.setAccount(supplier);
    if (amountTaxedDouble > 0.001) {
        entryCredit.setCreditOrig(amountTaxedDouble);
    } else if (amountTaxedDouble < -0.001) {
        entryCredit.setDebitOrig(-amountTaxedDouble);
    } else {
        Q_ASSERT(false);
    }
    //entryCredit.setAmountOrigCurrency(amountTaxed);

    entryDebit.setAccount(account);
    if (amountUntaxedDouble > 0.001) {
        entryDebit.setDebitOrig(amountUntaxedDouble);
    } else if (amountUntaxedDouble < -0.001) {
        entryDebit.setCreditOrig(-amountUntaxedDouble);
    } else {
        Q_ASSERT(false);
    }

    entrySet->addEntry(entryCredit);
    entrySet->addEntry(entryDebit);

    if (qAbs(amountVatDouble) > 0.001) {
        if (amountVatDouble > 0.0) {
            entryDebitVat.setDebitOrig(amountVatDouble);
        } else if (amountVatDouble < -0.001) {
            entryDebitVat.setCreditOrig(-amountVatDouble);
        }
        QString vatRateDefault = QString::number(
                    VatRateManager::instance()->vatRateDefault(countryCodeVat, date),
                    'f', 2);

        QString countryNameVat = CountryManager::instance()->countryName(
                    countryCodeVat);
        QString accountVat = ManagerAccountPurchase::instance()
                ->accountVat(countryNameVat);
        entryDebitVat.setAccount(accountVat);
        entryDebitVat.setCurrency(currencyVat);
        entrySet->addEntry(entryDebitVat);
    }
    return entrySet;

}
//----------------------------------------------------------
ImportInvoiceInfo EntryParserImportations::invoiceInfoFromFileName(
        const QString &fileName)
{
    ImportInvoiceInfo infos;
    QStringList elementsFileNames
            = QFileInfo(fileName).fileName().split(".");
    elementsFileNames.takeLast();
    QString baseName = elementsFileNames.join(".");
    QStringList elements = baseName.split("__");
    infos.date = QDate::fromString(elements[0], "yyyy-MM-dd");
    bool isNumber = false;
    int i = 1;
    for (i = 1; i < elements.size(); ++i) {
        int accountInt = elements[i].toInt(&isNumber);
        if (isNumber) {
            infos.accountOrig6 = QString::number(accountInt);
            break;
        }
    }
    infos.countryCodeFrom = elements[i+1];
    infos.countryCodeTo = elements[i+2];
    QString amountTaxed = elements.takeLast();
    infos.currency = amountTaxed.right(3);
    amountTaxed = amountTaxed.left(amountTaxed.size()-3);
    infos.amount = amountTaxed.toDouble();

    infos.currencyVat = infos.currency;
    infos.amountVat = 0.;
    QString amountVatString;
    if (elements.last().contains("TVA")) {
        QStringList vatElements = elements.last().split("-");
        amountVatString = vatElements.last();
        infos.vatCountryCode = vatElements[0];
        infos.currencyVat = elements.last().right(3);
        amountVatString = amountVatString.left(amountVatString.size()-3);
        infos.amountVat = amountVatString.toDouble();
        Q_ASSERT(!infos.vatCountryCode.isEmpty());
    }

    /*
    double amountTaxedDoubleConv = infos.amount;
    double amountVatDoubleConv = infos.amountVat;
    QString bookKeepingCur = CustomerManager::instance()->getSelectedCustomerCurrency();
    if (infos.currency != CustomerManager::instance()->getSelectedCustomerCurrency()) {
        double rate = CurrencyRateManager::instance()->rate(
                    infos.currency, bookKeepingCur, infos.date);
        amountTaxedDoubleConv = infos.amount * rate;
    }
    if (qAbs(infos.amountVat) > 0.001
            && infos.currencyVat != CustomerManager::instance()->getSelectedCustomerCurrency()) {
        double rate = CurrencyRateManager::instance()->rate(
                    infos.currencyVat, bookKeepingCur, infos.date);
        amountVatDoubleConv = infos.amountVat * rate;
    }
    //*/

    ++i;
    infos.label = elements[i].replace("-", " ");
    ++i;
    QDir dir = SettingManager::instance()->bookKeepingDirPurchaseImport(
                infos.date.year(), infos.date.month());
    infos.absFileName = dir.filePath(QFileInfo(fileName).fileName());
    infos.absFileName = infos.absFileName.replace(
                SettingManager::instance()->bookKeepingDirPurchaseImport().absolutePath(), "");
    return infos;
}
//----------------------------------------------------------
QString EntryParserImportations::invoiceInfoToFileName(
        const ImportInvoiceInfo &infos)
{
    QString labelCopy = infos.label;
    QString amountString = QString::number(infos.amount, 'f', 2);
    QString purchaseId = EntryParserPurchases::genPurchaseId(
                infos.date, infos.label, amountString, infos.currency);
    QFileInfo fileInfo(infos.absFileName);
    QString finalFileName = infos.date.toString("yyyy-MM-dd");
    finalFileName += "__" + infos.accountOrig6;
    finalFileName += "__" + infos.countryCodeFrom;
    finalFileName += "__" + infos.countryCodeTo;
    finalFileName += "__" + labelCopy.replace(" ", "-");
    finalFileName += "__ID-" + purchaseId;
    if (qAbs(infos.amountVat) > 0.001) {
        QString vatCountryCode = infos.vatCountryCode;
        if (vatCountryCode.isEmpty()) {
            vatCountryCode = CountryManager::instance()
                ->countryCode(infos.vatCountryName);
        }
        finalFileName += "__" + vatCountryCode + "-TVA-"
                + QString::number(infos.amountVat, 'f', 2) + infos.currencyVat;
    }
    finalFileName += "__" + QString::number(infos.amount, 'f', 2) + infos.currency
            + "." + fileInfo.suffix();
    return finalFileName;
}
//----------------------------------------------------------
AccountingEntries EntryParserImportations::entries(
        int year) const
{
    AccountingEntries allEntries;
    QDir dir = SettingManager::instance()->bookKeepingDirPurchaseImport(year);
    auto infoList = dir.entryInfoList(
                QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto monthDirInfo : infoList) {
        QDir monthDir = monthDirInfo.filePath();
        auto infoFileNames = monthDir.entryInfoList(QDir::Files);
        for (auto infoFileName : infoFileNames) {
            auto entrySet = entrySetFromFileName(infoFileName.fileName());
            QString journal = ManagerEntryTables::instance()->journalName(
                        ManagerEntryTables::instance()
                        ->entryDisplayImportations()->name());
            addEntryStatic(allEntries, entrySet);
            //allEntries[year][journal][entrySet->monthStr()].insert(
                        //entrySet->label(), entrySet);
        }
    }
    return allEntries;

}
//----------------------------------------------------------

