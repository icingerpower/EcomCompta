#include "model/SettingManager.h"
#include "../common/currencies/CurrencyRateManager.h"
#include "../common/utils/FileUtils.h"

#include "EntryParserBank.h"
#include "ManagerEntryTables.h"
#include "model/CustomerManager.h"

//----------------------------------------------------------
EntryParserBank::EntryParserBank()
    : AbstractEntryParser()
{
}
//----------------------------------------------------------
AccountingEntries EntryParserBank::entries(
        int year) const
{
    AccountingEntries allEntries;
    QDir dirBank = SettingManager::instance()->bookKeepingDirBank(
                nameFolder(), year);
    auto fileInfos = dirBank.entryInfoList(
                fileFilters(),
                QDir::Files);
    for (auto fileInfo : fileInfos) {
        QString absFileName = dirBank.filePath(fileInfo.fileName());
        auto lines = loadValues(absFileName);
        for (auto line : lines) {
            auto entrySet = _entrySetFromVariants(line, absFileName);
            addEntryStatic(allEntries, entrySet);
        }
    }
    return allEntries;
}
//----------------------------------------------------------
AbstractEntryParser::Type EntryParserBank::typeOfEntries() const
{
    return Bank;
}
//----------------------------------------------------------
QString EntryParserBank::nameFolder() const
{
    return name().replace(" ", "-");
}
//----------------------------------------------------------
QString EntryParserBank::fileName(int year, int month) const
{
    return fileName(QString::number(year),
                    QString::number(month).rightJustified(2, '0'));
}
//----------------------------------------------------------
QString EntryParserBank::fileName(
        const QString &year, const QString &month) const
{
    QString fileName(baseName(year, month));
    fileName += ".csv";
    return fileName;
}
//----------------------------------------------------------
QString EntryParserBank::baseName(int year, int month) const
{
    return baseName(QString::number(year),
                    QString::number(month).rightJustified(2, '0'));
}
//----------------------------------------------------------
QString EntryParserBank::baseName(const QString &year, const QString &month) const
{
    QString baseName(year);
    baseName += "-";
    baseName += month;
    baseName += "-";
    baseName += nameFolder();
    return baseName;
}
//----------------------------------------------------------
QList<QSharedPointer<AccountingEntrySet>>
EntryParserBank::addFilePath(
        int year,
        int month,
        const QString &filePath,
        const QString &filePathDisplay,
        bool copy)
{
    QDir dir = SettingManager::instance()->bookKeepingDirBank(
                nameFolder(), year);
    const QString &baseName = this->baseName(year, month);
    const QString &fileName = baseName + "." + QFileInfo(filePath.toLower()).suffix(); // TODO check here why dot is gone
    if (copy) {
        bool exist = FileUtils::fileContentExisting(
                    filePath, dir);
        if (exist) {
            EntryParserBankException exception;
            exception.setError(QObject::tr("Le fichier existe déjà. Il a déjà été importé."));
            exception.raise();
        }
        QFile::copy(filePath, dir.filePath(fileName));
    }
    QString filePathDisplayTo;
    if (!filePathDisplay.isEmpty()) {
        const QString &fileNameDisplay = baseName + QFileInfo(filePathDisplay.toLower()).suffix();
        filePathDisplayTo = dir.filePath(fileNameDisplay);
        QFile::copy(filePathDisplay, filePathDisplayTo);
    }
    auto lines = loadValues(filePath);
    QList<QSharedPointer<AccountingEntrySet>> entrySets;
    for (auto line : lines) {
        auto entrySet = _entrySetFromVariants(line, filePath);
        if (!filePathDisplayTo.isEmpty()) {
            entrySet->setSourceDocumentDisplay(filePathDisplayTo);
        }
        entrySets << entrySet;
    }
    return entrySets;
}
//----------------------------------------------------------
QDate EntryParserBank::guessDate(const QString &filePath) const
{
    QDate date;
    auto lines = loadValues(filePath);
    if (lines.size() > 0) {
        int index = lines.size() / 2;
        date = lines[index][1].toDate();
    }
    return date;
}
//----------------------------------------------------------
QStringList EntryParserBank::fileFilters() const
{
    return QStringList() << "*.csv";
}
//----------------------------------------------------------
QList<QSharedPointer<AccountingEntrySet>> EntryParserBank::entrySetsFromFilePath(
        const QString &filePath) const
{
    QList<QSharedPointer<AccountingEntrySet>> entrySets;
    auto lines = loadValues(filePath);
    for (auto line : lines) {
        auto entrySet = _entrySetFromVariants(line, filePath);
        entrySets << entrySet;
    }
    return entrySets;
}
//----------------------------------------------------------
QSharedPointer<AccountingEntrySet> EntryParserBank::_entrySetFromVariants(
        const QVariantList &variants, const QString &filePath) const
{
    QSharedPointer<AccountingEntrySet> entrySet(
                new AccountingEntrySet(AccountingEntrySet::Bank));
    QString id = _idEntrySet(variants[0].toInt(), variants[1].toDate());
    entrySet->setId(id);
    double amount = variants[3].toDouble();
    //double amountConv = amount;
    double fees = variants[5].toDouble();
    //double feesConv = fees;
    double amountLessFees = amount - fees;
    //double amountLessFeesConv = amountLessFees;
    QString currency = variants[4].toString();
    QDate date = variants[1].toDate();
    entrySet->setCurrencyOrig(currency);
    entrySet->setAmountOrig(amount);

    AccountingEntry entryDebit;
    QString journal = ManagerEntryTables::instance()->journalName(
                name());
    QString label = variants[2].toString();
    entryDebit.setLabel(variants[2].toString());
    entryDebit.setDate(date);
    entryDebit.setJournal(journal);
    entryDebit.setMain(true);
    QString bookKeepingCur = CustomerManager::instance()->getSelectedCustomerCurrency();
    if (currency != bookKeepingCur) {
    /* // Conversion is done by accounting saver
        double rate = CurrencyRateManager::instance()->rate(
                    currency, bookKeepingCur, date);
        amountConv *= rate;
        feesConv *= rate;
        amountLessFeesConv *= rate;
        //*/
        label = variants[3].toString() + " " + currency + " " + label;
    }
    entryDebit.setCurrency(currency);
    //entryDebit.setAmountOrigCurrency(variants[3].toString());

    QString filePathMinusWorkingDir = filePath;
    QDir workingDir = SettingManager::instance()->bookKeepingDirBank(nameFolder());
    filePathMinusWorkingDir.replace(workingDir.absolutePath(), "");
    entryDebit.setFileRelWorkingDir(filePathMinusWorkingDir);
    entrySet->setSourceDocument(filePathMinusWorkingDir);
    QStringList fileElements = filePathMinusWorkingDir.split(".");
    fileElements.last() = "pdf";
    entrySet->setSourceDocumentDisplay(fileElements.join("."));

    entryDebit.setAccount("TODO");

    AccountingEntry entryCredit = entryDebit;
    if (amount > 0.001) {
        entryDebit.setCreditOrig(amount);
    } else if (amount < -0.001) {
        entryDebit.setDebitOrig(-amount);
    } else {
        Q_ASSERT(false);
    }
    if (amountLessFees > 0.001) {
        entryCredit.setDebitOrig(amountLessFees);
    } else {
        entryCredit.setCreditOrig(-amountLessFees);
    }
    entryCredit.setAccount(account());

    entrySet->addEntry(entryDebit);
    entrySet->addEntry(entryCredit);
    if (qAbs(fees) > 0.004) {
        AccountingEntry entryFees = entryDebit;
        entryFees.setDebitOrig("");
        entryFees.setCreditOrig("");
        entryFees.setAccount(accountFees()); //TODO use from settings
        if (fees > 0.001) {
            entryFees.setDebitOrig(fees);
        } else if (amount < -0.001) {
            entryFees.setCreditOrig(fees);
        }
        entrySet->addEntry(entryFees);
    }
    return entrySet;
}
//----------------------------------------------------------
QString EntryParserBank::_idEntrySet(
        const int idRow, const QDate &date) const
{
    QString id = date.toString("yyyy-MM-");
    id += nameFolder();
    id += "-" + QString::number(idRow).rightJustified(3, '0');
    return id;
}
//----------------------------------------------------------
//----------------------------------------------------------
void EntryParserBankException::raise() const
{
    throw *this;
}
//----------------------------------------------------------
//----------------------------------------------------------
EntryParserBankException *EntryParserBankException::clone() const
{
    return new EntryParserBankException(*this);
}
//----------------------------------------------------------
//----------------------------------------------------------
QString EntryParserBankException::error() const
{
    return m_error;
}
//----------------------------------------------------------
//----------------------------------------------------------
void EntryParserBankException::setError(const QString &error)
{
    m_error = error;
}
//----------------------------------------------------------
//----------------------------------------------------------
