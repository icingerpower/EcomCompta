#include "../common/currencies/CurrencyRateManager.h"
#include "model/CustomerManager.h"
#include "model/bookkeeping/SettingBookKeeping.h"

#include "AccountingEntrySet.h"

//----------------------------------------------------------
AccountingEntrySet::AccountingEntrySet(Type type) //Connection connectionType)
{
    //m_connection = connectionType;
    m_type = type;
    m_state = Correct;
    m_selected = false;
    m_currencyOrig = CustomerManager::instance()->getSelectedCustomerCurrency();
}
//----------------------------------------------------------
bool AccountingEntrySet::isConnected() const
{
    return m_connectedTo.size() > 0;
}
//----------------------------------------------------------
bool AccountingEntrySet::hasDocument() const
{
    return !m_sourceDocument.isEmpty();
}
//----------------------------------------------------------
/*
void AccountingEntrySet::connectTo(
        const QList<QSharedPointer<AccountingEntrySet> > &entrySets)
{
    m_connectedTo = entrySets;
}
//----------------------------------------------------------
void AccountingEntrySet::connectTo(
        QSharedPointer<AccountingEntrySet> &entrySet)
{
    connectTo(QList<QSharedPointer<AccountingEntrySet>>() << entrySet);
}
//----------------------------------------------------------
void AccountingEntrySet::disconnect()
{
    for (auto entrySet : m_connectedTo) {
        for (auto entrySetBack : entrySet->m_connectedTo) {
            entrySetBack->m_connectedTo.clear();
        }
        entrySet->m_connectedTo.clear();
    }
}
//*/
//----------------------------------------------------------
void AccountingEntrySet::load(const QString &filePath)
{
    /*
    auto lines = UG::StrConv::csvLines(filePath);
    if (lines.size() > 0) {
        auto first = lines.takeFirst();
        if (first.size() > 2) {
            m_entries.clear();
            m_sourceBankAccount = first[0];
            m_sourceBankAccountRow = first[1].toInt();
            m_sourceDocuments = first[2].split("\n");
            for (auto elements : lines) {
                AccountingEntry entry;
                entry.setJournal(elements[0]);
                entry.setDate(QDate::fromString(
                                  elements[1], "dd/MM/yyyy"));
                entry.setAccount(elements[2]);
                entry.setLabel(elements[3]);
                entry.setDebit(elements[4]);
                entry.setCredit(elements[5]);
                addEntry(entry);
            }
        }
    }
    //*/
}
//----------------------------------------------------------
void AccountingEntrySet::save(const QString &filePath) const
{
    /*
    QList<QStringList> lines;
    QStringList first;
    first << m_sourceBankAccount;
    first << QString::number(m_sourceBankAccountRow);
    first << m_sourceDocuments.join("\n");
    lines << first;
    for (auto entry : m_entries) {
        QStringList line;
        line << entry.journal();
        line << entry.date().toString("dd/MM/yyyy");
        line << entry.account();
        line << entry.label();
        line << entry.debit();
        line << entry.credit();
        lines << line;
    }
    UG::StrConv::writeCsvLines(
                filePath,
                lines);
                //*/
}
//----------------------------------------------------------
void AccountingEntrySet::updateOnAssociateTo(const AccountingEntrySet &other)
{
    if (type() == Bank) {
        for (auto entry : qAsConst(m_entries)) {
            entry.setLabelReplaced(other.m_entries[0].label());
        }
        if (other.type() == Bank) {
            QString internWireAccount = SettingBookKeeping::instance()->internWireAccount();
            m_entries[0].setAccountReplaced(internWireAccount);
            if (m_currencyOrig != CustomerManager::instance()->getSelectedCustomerCurrency()) {
                double rate = - other.m_amountOrig / m_amountOrig;
                for (auto it = m_entries.begin();
                     it != m_entries.end(); ++it) {
                    if (it->currency() == m_currencyOrig) {
                        /*
                        QString bookKeepingCur = SettingManager::instance()->currency();
                        double rate = CurrencyRateManager::instance()->rate(
                                    m_currencyOrig, bookKeepingCur, it->date());
                                    //*/
                        it->setRateForConversion(rate);
                    }
                }
            }
            //TODO set rate for conversion
        } else if (other.type() == Purchase
                || other.type() == Sale
                || other.type() == SaleMarketplace) {
            m_entries[0].setAccountReplaced(other.accountAssociation());
            for (auto it = m_entries.begin();
                 it != m_entries.end(); ++it) {
                double value = qMax(it->debitOrigDouble(), it->creditOrigDouble());
                QString valueString = QString::number(value, 'f', 2) + " " + it->currency();
                QString otherLabel = other.label();
                if (!otherLabel.startsWith(valueString)) {
                    otherLabel = valueString + " - " + otherLabel;
                }
                it->setLabelReplaced(otherLabel);
            }
        }
    }
    if (type() != Bank && other.type() == Bank && m_currencyOrig != CustomerManager::instance()->getSelectedCustomerCurrency()) {
        double rate = other.rateCurrencyDate();
        // TODO check why this rate no used for purchases
        for (auto it = m_entries.begin();
             it != m_entries.end(); ++it) {
            if (it->currency() == m_currencyOrig) {
                it->setRateForConversion(rate);
            }
        }
    }
}
//----------------------------------------------------------
void AccountingEntrySet::updateOnDissociateTo(const AccountingEntrySet &other)
{
    if (type() == Bank) {
        for (auto entry : m_entries) {
            entry.setLabelReplaced(other.m_entries[0].label());
        }
        if (other.type() == Purchase
                || other.type() == Sale) {
            m_entries[0].setAccountReplaced("");
            for (auto it = m_entries.begin();
                 it != m_entries.end(); ++it) {
                it->setLabelReplaced("");
            }
        }
    }
    if (type() != Bank && other.type() == Bank && m_currencyOrig != CustomerManager::instance()->getSelectedCustomerCurrency()) {
        for (auto it = m_entries.begin();
             it != m_entries.end(); ++it) {
            if (it->currency() == m_currencyOrig) {
                it->setRateForConversion(1.);
            }
        }
    }
}
//----------------------------------------------------------
void AccountingEntrySet::updateOnSelfAssociateTo(
        const QString &title, const QString &account)
{
    m_entries[0].setAccountReplaced(account);
    if (type() != Purchase) {
        for (auto it = m_entries.begin();
             it != m_entries.end(); ++it) {
            it->setLabelReplaced(title);
        }
    }
}
//----------------------------------------------------------
void AccountingEntrySet::updateOnSelfDissociate()
{
}
//----------------------------------------------------------
QString AccountingEntrySet::accountAssociation() const
{
    for (auto entry : m_entries) {
        if (entry.isMain()) {
            return entry.account();
        }
    }
    return  m_entries[0].account();
}
//----------------------------------------------------------
QString AccountingEntrySet::fileRelWorkingDir() const
{
    if (m_entries.size() > 0) {
        return m_entries[0].fileRelWorkingDir();
    }
    return "";
}
//----------------------------------------------------------
QString AccountingEntrySet::journal() const
{
    QString journal;
    if (m_entries.size() > 0) {
        journal = m_entries[0].journal();
    }
    return journal;
}
//----------------------------------------------------------
QString AccountingEntrySet::label() const
{
    QString label;
    if (m_entries.size() > 0) {
        label = m_entries[0].label();
    }
    return label;
}
//----------------------------------------------------------
QDate AccountingEntrySet::date() const
{
    QDate date;
    if (m_entries.size() > 0) {
        date = m_entries[0].date();
    }
    return date;
}
//----------------------------------------------------------
void AccountingEntrySet::setRateForConversion(double rate)
{
    for (auto it = m_entries.begin();
         it != m_entries.end(); ++it) {
        it->setRateForConversion(rate);
    }
}
//----------------------------------------------------------
double AccountingEntrySet::rateForConversion() const
{
    return m_entries[0].rateForConversion();
}
//----------------------------------------------------------
double AccountingEntrySet::rateCurrencyDate() const
{
    if (m_currencyOrig != CustomerManager::instance()->getSelectedCustomerCurrency()) {
        return CurrencyRateManager::instance()->rate(
                    m_currencyOrig,
                    CustomerManager::instance()->getSelectedCustomerCurrency(),
                    date());
    }
    return 1.;
}
//----------------------------------------------------------
int AccountingEntrySet::year() const
{
    return date().year();
}
//----------------------------------------------------------
int AccountingEntrySet::month() const
{
    return date().month();
}
//----------------------------------------------------------
QString AccountingEntrySet::monthStr() const
{
    QString monStr = QString::number(
                date().month()).rightJustified(2, '0');
    return monStr;
}
//----------------------------------------------------------
int AccountingEntrySet::size() const
{
    return m_entries.size();
}
//----------------------------------------------------------
bool AccountingEntrySet::isDebit() const
{
    return !m_entries[0].debitOrig().isNull();
}
//----------------------------------------------------------
/*
double AccountingEntrySet::amountFirstOrigCur() const
{
    for (auto entry : m_entries) {
        if (entry.isMain()) {
            return entry.amountOrigCurrencyDouble();
        }
    }
    Q_ASSERT(false);
    return 0.;
}
//----------------------------------------------------------
double AccountingEntrySet::debitFirst() const
{
    return m_entries[0].debitDouble();
}
//----------------------------------------------------------
double AccountingEntrySet::creditFirst() const
{
    return m_entries[0].creditDouble();
}
//*/
//----------------------------------------------------------
double AccountingEntrySet::debitTotalOrig() const
{
    double debit = 0.;
    for (auto entry : qAsConst(m_entries)) {
        debit += qRound(entry.debitOrigDouble()*100) / 100.;
    }
    return debit;
}
//----------------------------------------------------------
double AccountingEntrySet::creditTotalOrig() const
{
    double credit = 0.;
    for (auto entry : qAsConst(m_entries)) {
        credit += qRound(entry.creditOrigDouble()*100) / 100.;
    }
    return credit;
}
//----------------------------------------------------------
double AccountingEntrySet::debitTotalConv() const
{
    double debit = 0.;
    for (auto entry : qAsConst(m_entries)) {
        debit += qRound(entry.debitConvDouble()*100) / 100.;
    }
    return debit;
}
//----------------------------------------------------------
double AccountingEntrySet::creditTotalConv() const
{
    double credit = 0.;
    for (auto entry : qAsConst(m_entries)) {
        credit += qRound(entry.creditConvDouble()*100) / 100.;
    }
    return credit;
}
//----------------------------------------------------------
void AccountingEntrySet::roundCreditDebit(double toAddInHighestDebit, double toAddInHighestCredit)
{
    int maxIndexDebit = 0;
    int maxIndexCredit = 0;
    for (int i=1; i<m_entries.size(); ++i) {
        if (m_entries[maxIndexCredit].creditOrigDouble() < m_entries[i].creditOrigDouble()) {
            maxIndexCredit = i;
        }
        if (m_entries[maxIndexDebit].debitOrigDouble() < m_entries[i].debitOrigDouble()) {
            maxIndexDebit = i;
        }
    }
    if (toAddInHighestCredit != 0.) {
        double newCredit = m_entries[maxIndexCredit].creditOrigDouble() + toAddInHighestCredit;
        m_entries[maxIndexCredit].setCreditOrig(newCredit);
    }
    if (toAddInHighestDebit != 0.) {
        double newDebit = m_entries[maxIndexDebit].debitOrigDouble() + toAddInHighestDebit;
        m_entries[maxIndexDebit].setDebitOrig(newDebit);
    }
}
//----------------------------------------------------------
void AccountingEntrySet::roundCreditDebit()
{
    // TODO doesn't work with multiple currency (for instance for amazon payment Canada)
    // We should look difference of converted amount
    if (m_entries.size() > 1) {
        double credit = this->creditTotalConv();
        double debit = this->debitTotalConv();
        double diff = qAbs(credit - debit);
        if (diff > 0.) {
            int lastEntryIndex = m_entries.size() - 1;
            if (debit < credit) {
                while (m_entries[lastEntryIndex].debitOrigDouble() == 0.) {
                    --lastEntryIndex;
                }
                double entryDebit = m_entries[lastEntryIndex].debitOrigDouble();
                double rate = this->creditTotalOrig() / credit;
                entryDebit += diff * rate;
                m_entries[lastEntryIndex].setDebitOrig(entryDebit);
            } else {
                while (m_entries[lastEntryIndex].creditOrigDouble() == 0.) {
                    --lastEntryIndex;
                }
                double entryCredit = m_entries[lastEntryIndex].creditOrigDouble();
                double rate = this->debitTotalOrig() / debit;
                entryCredit += diff * rate;
                m_entries[lastEntryIndex].setCreditOrig(entryCredit);
            }
        }
    }
}
//----------------------------------------------------------
void AccountingEntrySet::addEntry(
        const AccountingEntry &entry)
{
    m_entries << entry;
}
//----------------------------------------------------------
void AccountingEntrySet::insertEntry(int index, const AccountingEntry &entry)
{
    m_entries.insert(index, entry);
}
//----------------------------------------------------------
void AccountingEntrySet::replaceEntry(int index, const AccountingEntry &entry)
{
    m_entries[index] = entry;
}
//----------------------------------------------------------
void AccountingEntrySet::removeEntry(int index)
{
    m_entries.removeAt(index);
}
//----------------------------------------------------------
void AccountingEntrySet::removeEntry(const QString &account)
{
    for (int i=0; i<m_entries.size(); ++i) {
        if (m_entries[i].account() == account) {
            m_entries.removeAt(i);
            break;
        }
    }
}
//----------------------------------------------------------
QList<AccountingEntry>::iterator AccountingEntrySet::entriesBegin()
{
    return m_entries.begin();
}
//----------------------------------------------------------
QList<AccountingEntry>::iterator AccountingEntrySet::entriesEnd()
{
    return m_entries.end();
}
//----------------------------------------------------------
const QList<AccountingEntry> &AccountingEntrySet::entries() const
{
    return m_entries;
}
//----------------------------------------------------------
QString AccountingEntrySet::sourceDocument() const
{
    return m_sourceDocument;
}
//----------------------------------------------------------
void AccountingEntrySet::setSourceDocument(const QString &sourceDocument)
{
    m_sourceDocument = sourceDocument;
}
//----------------------------------------------------------
//AccountingEntrySet::Connection AccountingEntrySet::connection() const
//{
    //return m_connection;
//}
//----------------------------------------------------------
bool AccountingEntrySet::hasHtmlDocument() const
{
    return !m_htmlDocument.isEmpty();
}
//----------------------------------------------------------
QString AccountingEntrySet::htmlDocument() const
{
    return m_htmlDocument;
}
//----------------------------------------------------------
void AccountingEntrySet::setHtmlDocument(
        const QString &htmlDocument, const QString &docBaseName)
{
    m_htmlDocument = htmlDocument;
    m_htmlDocBaseName = docBaseName;
}
//----------------------------------------------------------
bool AccountingEntrySet::hasCsvData() const
{
    return m_csvData.size() > 0 && !m_csvFileBaseName.isEmpty();

}
//----------------------------------------------------------
const QString &AccountingEntrySet::csvFileBaseName() const
{
    return m_csvFileBaseName;
}
//----------------------------------------------------------
const QList<QStringList> &AccountingEntrySet::csvData() const
{
    return m_csvData;
}
//----------------------------------------------------------
void AccountingEntrySet::setCsvData(
        const QList<QStringList> &csvData,
        const QString &csvFileBaseName)
{
    m_csvData = csvData;
    m_csvFileBaseName = csvFileBaseName;
}
//----------------------------------------------------------
void AccountingEntrySet::select()
{
    m_selected = true;
    for (auto connected : m_connectedTo) {
        connected->m_selected = true;
    }
}
//----------------------------------------------------------
void AccountingEntrySet::unselect()
{
    m_selected = false;
    for (auto connected : m_connectedTo) {
        connected->m_selected = false;
    }
}
//----------------------------------------------------------
QString AccountingEntrySet::id() const
{
    return m_id;
}
//----------------------------------------------------------
void AccountingEntrySet::setId(const QString &id)
{
    m_id = id;
}
//----------------------------------------------------------
/*
QList<QSharedPointer<AccountingEntrySet> > AccountingEntrySet::connectedTo() const
{
    return m_connectedTo;
}
//*/
//----------------------------------------------------------
QString AccountingEntrySet::sourceDocumentDisplay() const
{
    if (m_sourceDocumentDisplay.isEmpty()) {
        return m_sourceDocument;
    }
    return m_sourceDocumentDisplay;
}
//----------------------------------------------------------
void AccountingEntrySet::setSourceDocumentDisplay(const QString &sourceDocumentDisplay)
{
    m_sourceDocumentDisplay = sourceDocumentDisplay;
}
//----------------------------------------------------------
double AccountingEntrySet::amountConv() const
{
    if (m_id == "SHIP-pradize-1304") {
        int TEMP=10;++TEMP;
    }
    if (m_currencyOrig == CustomerManager::instance()->getSelectedCustomerCurrency()) {
        return m_amountOrig;
    }
    double rate = rateForConversion();
    if (qAbs(rate-1) < 0.0001) {
        rate = rateCurrencyDate();
    }
    return m_amountOrig * rate;
}
//----------------------------------------------------------
double AccountingEntrySet::amountOrig() const
{
    return m_amountOrig;
}
//----------------------------------------------------------
void AccountingEntrySet::setAmountOrig(double amountOrig)
{
    if (m_id == "amazon.es-23389301832-2024-11-28")
    {
        int TEMP=10;++TEMP;
    }
    m_amountOrig = amountOrig;
}
//----------------------------------------------------------
QString AccountingEntrySet::currencyOrig() const
{
    return m_currencyOrig;
}
//----------------------------------------------------------
void AccountingEntrySet::setCurrencyOrig(const QString &currencyOrig)
{
    m_currencyOrig = currencyOrig;
}
//----------------------------------------------------------
AccountingEntrySet::Type AccountingEntrySet::type() const
{
    return m_type;
}
//----------------------------------------------------------
AccountingEntrySet::State AccountingEntrySet::state() const
{
    return m_state;
}
//----------------------------------------------------------
void AccountingEntrySet::setState(const State &state)
{
    m_state = state;
}
//----------------------------------------------------------
QString AccountingEntrySet::htmlDocBaseName() const
{
    return m_htmlDocBaseName;
}
//----------------------------------------------------------
/*
typedef QMap<int, QMap<QString, QMap<QString, QMap<QString, AccountingEntrySet>>>> AccountingEntries;
//----------------------------------------------------------
void addEntryKeys(
        AccountingEntries &entries,
        int year,
        const QString &journal,
        const QString &month,
        const QString &label)
{
    if (!entries.contains(year)) {
        entries[year] = QMap<QString, QMap<QString, QMap<QString, AccountingEntrySet>>>();
    }
    if (!entries[year].contains(journal)) {
        entries[year][journal] = QMap<QString, QMap<QString, AccountingEntrySet>>();
    }
}
//*/
//----------------------------------------------------------
void removeEntryStatic(AccountingEntries &entries, QSharedPointer<AccountingEntrySet> entry)
{
// Year / journal / month / label
    entries[entry->year()][entry->journal()][entry->monthStr()].remove(entry->label(), entry);
    if (entries[entry->year()][entry->journal()][entry->monthStr()].isEmpty()) {
        entries[entry->year()][entry->journal()].remove(entry->monthStr());
        if (entries[entry->year()][entry->journal()].isEmpty()) {
            entries[entry->year()].remove(entry->journal());
            if (entries[entry->year()].isEmpty()) {
                entries.remove(entry->year());
            }
        }
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
void addEntryStatic(AccountingEntries &entries,
                    QSharedPointer<AccountingEntrySet> entrySet)
{
    // Year / journal / month / label
    int year = entrySet->year();
    /*

    if (!entries.contains(year)) {
        entries[year] = QMap<QString, QMap<QString, QMultiMap<QString, QSharedPointer<AccountingEntrySet>>>>();
    }
    //*/
    Q_ASSERT(!entrySet->journal().isEmpty());
    Q_ASSERT(!entrySet->label().isEmpty());
    /*
    if (!entries[year].contains(entrySet->journal())) {
        entries[year][entrySet->journal()] = QMap<QString, QMultiMap<QString, QSharedPointer<AccountingEntrySet>>>();
    }
    //*/
    QString monthStr = entrySet->monthStr();
    /*
    if (!entries[year][entrySet->journal()].contains(monthStr)) {
        entries[year][entrySet->journal()][monthStr]
                = QMultiMap<QString, QSharedPointer<AccountingEntrySet>>();
    }
    //*/
    entries[year][entrySet->journal()][monthStr].insert(
                entrySet->label(), entrySet);
}
//----------------------------------------------------------
//----------------------------------------------------------
void addEntriesStatic(AccountingEntries &entries, const AccountingEntries &entriesToAdd)
{
    for (auto itYear = entriesToAdd.begin(); itYear != entriesToAdd.end(); ++itYear) {
        if (!entries.contains(itYear.key())) {
            entries[itYear.key()] = itYear.value();
        } else {
            for (auto itJournal = itYear.value().begin(); itJournal != itYear.value().end(); ++itJournal) {
                if (!entries[itYear.key()].contains(itJournal.key())) {
                    entries[itYear.key()][itJournal.key()] = itJournal.value();
                } else {
                    for (auto itMonth = itJournal.value().begin(); itMonth != itJournal.value().end(); ++itMonth) {
                        if (!entries[itYear.key()][itJournal.key()].contains(itMonth.key())) {
                            entries[itYear.key()][itJournal.key()][itMonth.key()] = itMonth.value();
                        } else {
                            for (auto itLabel = itMonth.value().begin(); itLabel != itMonth.value().end(); ++itLabel) {
                                entries[itYear.key()][itJournal.key()][itMonth.key()].insert(
                                            itLabel.key(), itLabel.value());
                            }
                        }
                    }
                }
            }
        }
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
