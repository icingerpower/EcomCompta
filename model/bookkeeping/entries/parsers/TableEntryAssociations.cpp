#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>

#include "TableEntryAssociations.h"
#include "EntrySelfTable.h"
#include "model/SettingManager.h"
#include "model/bookkeeping/entries/AccountingEntrySet.h"

//----------------------------------------------------------
TableEntryAssociations *TableEntryAssociations::instance()
{
    static TableEntryAssociations instance;
    return &instance;
}
//----------------------------------------------------------
TableEntryAssociations::TableEntryAssociations(QObject *parent)
    : QObject(parent), UpdateToCustomer ()
{
    connect(EntrySelfTable::instance(),
            &EntrySelfTable::dataChanged,
            [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &){
        int rowFirst = topLeft.row();
        int rowLast = bottomRight.row();
        QList<EntrySelfInfo> changeAccounts;
        for (int i=rowFirst; i<= rowLast; ++i) {
            changeAccounts << EntrySelfTable::instance()->account(i);
        }
        for (auto itYear=m_selfAssociations.begin();
             itYear!=m_selfAssociations.end(); ++itYear) {
            for (auto it=itYear.value().begin();
                 it!=itYear.value().end(); ++it) {
                for (auto account : qAsConst(changeAccounts)) {
                    if (it.value() == account.id) {
                        m_entrySets[it.key()]->updateOnSelfAssociateTo(
                                    account.title, account.account);
                        // TODO also connect in bank table view
                    }
                }
            }
        }
    });
}
//----------------------------------------------------------
void TableEntryAssociations::onCustomerSelectedChanged(const QString &)
{
    m_associations.clear();
    m_associationsNoYear.clear();
    m_entrySets.clear();
}
//----------------------------------------------------------
QString TableEntryAssociations::uniqueId() const
{
    return "TableEntryAssociations";
}
//----------------------------------------------------------
void TableEntryAssociations::removeAssociation(
        int yearFrom, AccountingEntrySet *entrySetFrom)
{
    QString idFrom =  entrySetFrom->id();
    QList<QPair<int, QString>> associationsToRemove;
    for (auto itYearTo = m_associations[yearFrom].begin();
         itYearTo != m_associations[yearFrom].end(); ++itYearTo) {
        for (auto itIdFrom = itYearTo.value().begin();
             itIdFrom != itYearTo.value().end(); ++itIdFrom) {
            auto currentIdFrom = itIdFrom.key();
            if (itIdFrom.key() == idFrom) {
                for (auto itIdTo = itIdFrom.value().begin();
                     itIdTo != itIdFrom.value().end(); ++itIdTo) {
                    QPair<int, QString> association;
                    association.first = itYearTo.key();
                    association.second = *itIdTo;
                    associationsToRemove << association;
                }
            }
        }
    }
    if (m_associationsNoYear.contains(idFrom)) {
        m_associationsNoYear.remove(idFrom);
    }
    for (auto itAss = associationsToRemove.begin();
         itAss != associationsToRemove.end(); ++itAss) {
        auto entrySetTo = m_entrySets.value(itAss->second, nullptr);
        if (entrySetTo != nullptr) {
            removeAssociation(yearFrom, itAss->first, entrySetFrom, entrySetTo);
        }
    }
}
//----------------------------------------------------------
void TableEntryAssociations::removeAssociation(
        int yearFrom,
        int yearTo,
        AccountingEntrySet *entrySetFrom,
        AccountingEntrySet *entrySetTo)
{
    QString idFrom =  entrySetFrom->id();
    QString idTo =  entrySetTo->id();
    if (m_associations.contains(yearFrom)
            && m_associations[yearFrom].contains(yearTo)
            && m_associations[yearFrom][yearTo].contains(idFrom)
            && m_associations[yearFrom][yearTo][idFrom].contains(idTo)) {
        m_associations[yearFrom][yearTo][idFrom].remove(idTo);
        if (m_associations[yearFrom][yearTo][idFrom].isEmpty()) {
            m_associations[yearFrom][yearTo].remove(idFrom);
            if (m_associations[yearFrom][yearTo].isEmpty()) {
                m_associations[yearFrom].remove(yearTo);
                if (m_associations[yearFrom].isEmpty()) {
                    m_associations.remove(yearFrom);
                }
            }
        }
        if (m_associationsNoYear.contains(idFrom)) {
            m_associationsNoYear.remove(idFrom);
        }
        if (m_selected.contains(idFrom)) {
            m_selected.remove(idFrom);
        }
        QSet<QString> ids;
        ids << idFrom;
        ids << idTo;
        entrySetFrom->updateOnDissociateTo(*entrySetTo);
        _saveAssociations();
        emit idsDissociated(ids);
        removeAssociation(yearTo, yearFrom, entrySetTo, entrySetFrom);
    }
}
//----------------------------------------------------------
void TableEntryAssociations::addAssociation(
        int yearFrom,
        int yearTo,
        AccountingEntrySet *entrySetFrom,
        AccountingEntrySet *entrySetTo,
        bool save)
{
    QString idFrom =  entrySetFrom->id();
    QString idTo =  entrySetTo->id();
    if (!m_associations.contains(yearFrom)) {
        m_associations[yearFrom] = QMap<int, QHash<QString, QSet<QString>>>();
    }
    if (!m_associations.contains(yearTo)) {
        m_associations[yearTo] = QMap<int, QHash<QString, QSet<QString>>>();
    }

    if (!m_associations[yearTo].contains(yearFrom)) {
        m_associations[yearFrom][yearFrom] = QHash<QString, QSet<QString>>();
    }
    if (!m_associations[yearFrom].contains(yearTo)) {
        m_associations[yearFrom][yearTo] = QHash<QString, QSet<QString>>();
    }

    if (!m_associations[yearFrom][yearTo].contains(idFrom)) {
        m_associations[yearFrom][yearTo][idFrom] = QSet<QString>();
        m_associationsNoYear[idFrom] = QSet<QString>();
    }
    if (!m_associations[yearTo][yearFrom].contains(idTo)) {
        m_associations[yearTo][yearFrom][idTo] = QSet<QString>();
        m_associationsNoYear[idTo] = QSet<QString>();
    }

    m_associations[yearFrom][yearTo][idFrom] << idTo;
    m_associationsNoYear[idFrom] << idTo;;
    m_associations[yearTo][yearFrom][idTo] << idFrom;
    m_associationsNoYear[idTo] << idFrom;;
    entrySetFrom->updateOnAssociateTo(*entrySetTo);
    entrySetTo->updateOnAssociateTo(*entrySetFrom);
    QSet<QString> ids;
    ids << idFrom;
    ids << idTo;
    if (save) {
        _saveAssociations();
    }
    emit idsAssociated(ids);
}
//----------------------------------------------------------
void TableEntryAssociations::addSelfAssociation(
        AccountingEntrySet *entrySet,
        const QString &idSelfEntry,
        const QString &selfEntryAccount,
        const QString &selfEntryTitle,
        bool save)
{
    QDate date = entrySet->date();
    if (!m_selfAssociations.contains(date.year())) {
        m_selfAssociations[date.year()] = QHash<QString, QString>();
    }
    m_selfAssociations[date.year()][entrySet->id()] = idSelfEntry;
    entrySet->updateOnSelfAssociateTo(selfEntryTitle, selfEntryAccount);
    if (save) {
        _saveAssociations();
    }
    emit idSelfAssociated(entrySet->id());
}
//----------------------------------------------------------
void TableEntryAssociations::removeAllSelfAssociations(
        const QString &idSelfEntry)
{
    QList<QPair<int, QString>> toDelete;
    for (auto itYear = m_selfAssociations.begin();
         itYear != m_selfAssociations.end(); ++itYear) {
        for (auto it = itYear.value().begin();
             it != itYear.value().end(); ++it) {
            if (it.value() == idSelfEntry) {
                QPair<int, QString> pair(itYear.key(), it.key());
                toDelete << pair;
            }
        }
    }
    for (auto pair : qAsConst(toDelete)) {
        /// first is year and second is entry set id
        m_selfAssociations[pair.first].remove(pair.second);
        m_entrySets[pair.second]->updateOnSelfDissociate();
        emit idSelfDissociated(pair.second);
        if (m_selfAssociations[pair.first].isEmpty()) {
            m_selfAssociations.remove(pair.first);
        }
    }
    _saveAssociations();
}
//----------------------------------------------------------
void TableEntryAssociations::removeSelfAssociation(
        AccountingEntrySet *entrySet)
{
    for (auto itYear = m_selfAssociations.begin();
         itYear != m_selfAssociations.end(); ++itYear) {
        if (itYear.value().contains(entrySet->id())) {
            m_selfAssociations[itYear.key()].remove(entrySet->id());
            if (m_selfAssociations[itYear.key()].isEmpty()) {
                m_selfAssociations.remove(itYear.key());
            }
            _saveAssociations();
            entrySet->updateOnSelfDissociate();
            emit idSelfDissociated(entrySet->id());
            return;
        }
    }
}
//----------------------------------------------------------
void TableEntryAssociations::selectAssociation(const QString &id)
{
    emit idsUnselected(m_selected);
    m_selected.clear();
    if (m_associationsNoYear.contains(id)) {
        QSet<QString> rightIds = m_associationsNoYear[id];
        for (auto rightId : rightIds) {
            m_selected.unite(m_associationsNoYear[rightId]);
        }
        m_selected.unite(rightIds);
    }
    if (m_selected.size() > 0) {
        emit idsSelected(m_selected);
    }
}
//----------------------------------------------------------
void TableEntryAssociations::unselectAssociation(const QString &id)
{
    if (m_selected.contains(id)) {
        emit idsUnselected(m_selected);
        m_selected.clear();
    }
}
//----------------------------------------------------------
bool TableEntryAssociations::isSelfAssociated(const QString &id) const
{
    for (auto it = m_selfAssociations.begin();
         it != m_selfAssociations.end(); ++it) {
        if (it.value().contains(id)) {
            return true;
        }
    }
    return false;
}
//----------------------------------------------------------
bool TableEntryAssociations::isAssociated(
        const QString &id) const
{
    return m_associationsNoYear.contains(id);
}
//----------------------------------------------------------
bool TableEntryAssociations::isAssociatedAndSelected(
        const QString &id) const
{
    bool is = m_selected.contains(id);
    return is;
}
//----------------------------------------------------------
void TableEntryAssociations::recordEntrySet(
        QSharedPointer<AccountingEntrySet> entrySet)
{
    m_entrySets[entrySet->id()] = entrySet.data();
}
//----------------------------------------------------------
void TableEntryAssociations::removeEntrySet(
        QSharedPointer<AccountingEntrySet> entrySet)
{
    if (m_entrySets.contains(entrySet->id())) {
        m_entrySets.remove(entrySet->id());
    }
}
//----------------------------------------------------------
void TableEntryAssociations::_saveAssociations()
{
    QDir dir = SettingManager::instance()->bookKeepingDirPurchase();
    for (auto itYearFrom = m_associations.begin();
         itYearFrom != m_associations.end();
         ++itYearFrom) {
        for (auto itYearTo = itYearFrom.value().begin();
             itYearTo != itYearFrom.value().end();
             ++itYearTo) {
            QStringList lines;
            for (auto it = itYearTo.value().begin();
                 it != itYearTo.value().end();
                 ++it) {
                for (auto entryIdRight : it.value()) {
                    QStringList elements;
                    elements << QString::number(itYearFrom.key());
                    elements << QString::number(itYearTo.key());
                    elements << it.key();
                    elements << entryIdRight;
                    lines << elements.join(";");
                }
            }
            QString fileName = QString::number(itYearFrom.key())
                    + "-" + QString::number(itYearTo.key()) + ".csv";
            QString absFileName = dir.filePath(fileName);
            QFile file(absFileName);
            if (file.open(QFile::WriteOnly)) {
                QTextStream stream(&file);
                stream << lines.join("\n");
                file.close();
            }
        }
    }
    for (auto itYear = m_selfAssociations.begin();
         itYear != m_selfAssociations.end(); ++itYear) {
        QStringList lines;
        for (auto it = itYear->begin();
             it != itYear->end(); ++it) {
            lines << it.key() + ";" + it.value();
        }
        QString fileName = QString::number(itYear.key()) + "-self.csv";
        QString absFileName = dir.filePath(fileName);
        QFile file(absFileName);
        if (file.open(QFile::WriteOnly)) {
            QTextStream stream(&file);
            stream << lines.join("\n");
            file.close();
        }
    }
}
//----------------------------------------------------------
void TableEntryAssociations::loadAssociations(int year)
{
    QDir dir = SettingManager::instance()->bookKeepingDirPurchase();
    auto fileInfos = dir.entryInfoList(
                QStringList() << "*.csv", QDir::Files);
    for (auto fileInfo : qAsConst(fileInfos)) {
        QString fileName = fileInfo.fileName();
        if (fileName.contains(QString::number(year))) {
            QFile file(fileInfo.filePath());
            if (file.open(QFile::ReadOnly)) {
                QTextStream stream(&file);
                QStringList lines = stream.readAll().split("\n");
                if (fileName.contains("self")) {
                    for (auto line : qAsConst(lines)) {
                        if (!line.isEmpty()) {
                            QStringList elements = line.split(";");
                            QString idSelfEntry = elements[1];
                            auto account = EntrySelfTable::instance()->account(idSelfEntry);
                            if (m_entrySets.contains(elements[0])) {  //Due to conqo date update
                            addSelfAssociation(
                                        m_entrySets[elements[0]],
                                    elements[1],
                                    account.account,
                                    account.title,
                                    false);
                            }
                        }
                    }
                } else {
                    for (auto line : qAsConst(lines)) {
                        if (!line.isEmpty()) {
                            QStringList elements = line.split(";");
                            int yearFrom = elements[0].toInt();
                            int yearTo = elements[1].toInt();
                            QString idFrom = elements[2];
                            QString idTo = elements[3];
                            if (m_entrySets.contains(idFrom)
                                    && m_entrySets.contains(idTo)) {
                                addAssociation(
                                            yearFrom,
                                            yearTo,
                                            m_entrySets[idFrom],
                                            m_entrySets[idTo],
                                            false);
                            }
                        }
                    }
                }
            }
        }
    }
}
//----------------------------------------------------------
