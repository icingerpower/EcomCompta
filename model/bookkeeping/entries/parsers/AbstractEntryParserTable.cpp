#include <QtCore/qset.h>
#include <QtGui/qcolor.h>
#include <QtGui/qbrush.h>

#include "AbstractEntryParserTable.h"
#include "TableEntryAssociations.h"
#include "model/SettingManager.h"

QString AbstractEntryParserTable::COL_ID = QObject::tr("Identifiant");
QString AbstractEntryParserTable::COL_DATE = QObject::tr("Date");
QString AbstractEntryParserTable::COL_JOURNAL = QObject::tr("Journal");
QString AbstractEntryParserTable::COL_ACCOUNT_1 = QObject::tr("Compte 1");
QString AbstractEntryParserTable::COL_ACCOUNT_2 = QObject::tr("Compte 2");
QString AbstractEntryParserTable::COL_AMOUNT_CONV = QObject::tr("Montant converti");
QString AbstractEntryParserTable::COL_AMOUNT_ORIG = QObject::tr("Montant original");
QString AbstractEntryParserTable::COL_CURRENCY_ORIG = QObject::tr("Monnaie originale");
QString AbstractEntryParserTable::COL_LABEL = QObject::tr("Label");
QString AbstractEntryParserTable::COL_FILE_NAME_REL = QObject::tr("Fichier");
//----------------------------------------------------------
AbstractEntryParserTable::AbstractEntryParserTable(QObject *object)
    : QAbstractTableModel(object), UpdateToCustomer()
{
    init();
    m_connections << connect(TableEntryAssociations::instance(),
            &TableEntryAssociations::idsUnselected,
            this,
            [this](const QSet<QString> &ids){
        _emitChangeRowsOfEntrySetIds(ids);
    });
    m_connections << connect(TableEntryAssociations::instance(),
            &TableEntryAssociations::idsSelected,
            this,
            [this](const QSet<QString> &ids){
        _emitChangeRowsOfEntrySetIds(ids);
    });
    m_connections << connect(TableEntryAssociations::instance(),
            &TableEntryAssociations::idsAssociated,
            this,
            [this](const QSet<QString> &ids){
        _addAssociatedIds(ids);
    });
    m_connections << connect(TableEntryAssociations::instance(),
            &TableEntryAssociations::idsDissociated,
            this,
            [this](const QSet<QString> &ids){
        _removeAssociatedIds(ids);
    });
    m_connections << connect(TableEntryAssociations::instance(),
            &TableEntryAssociations::idSelfAssociated,
            this,
            [this](const QString &ids){
        _emitChangeRowsOfEntrySetIds(ids);
    });
    m_connections << connect(TableEntryAssociations::instance(),
            &TableEntryAssociations::idSelfDissociated,
            this,
            [this](const QString &ids){
        _emitChangeRowsOfEntrySetIds(ids);
    });
}
//----------------------------------------------------------
AbstractEntryParserTable::~AbstractEntryParserTable()
{
    QObject::disconnect(TableEntryAssociations::instance(), nullptr, this, nullptr);
    disconnect(TableEntryAssociations::instance());
    for (auto connIt = m_connections.begin();
         connIt != m_connections.end(); ++connIt) {
        QObject::disconnect(*connIt);
    }
}
//----------------------------------------------------------
void AbstractEntryParserTable::onCustomerSelectedChanged(
        const QString &)
{
    clear();
}
//----------------------------------------------------------
QString AbstractEntryParserTable::uniqueId() const
{
    return "AbstractEntryParserTable_" + name();
}
//----------------------------------------------------------
bool AbstractEntryParserTable::displays() const
{
    return true;
}
//----------------------------------------------------------
QSharedPointer<AccountingEntrySet> AbstractEntryParserTable::entrySet(
        int index) const
{
    return m_entrySets[index];
}
//----------------------------------------------------------
QSharedPointer<AccountingEntrySet> AbstractEntryParserTable::entrySet(
        const QModelIndex &index) const
{
    return m_entrySets[index.row()];
}
//----------------------------------------------------------
void AbstractEntryParserTable::clear()
{
    if (m_entrySets.size() > 0) {
        beginInsertRows(QModelIndex(), 0, rowCount()-1);
        //m_values.clear();
        m_entryById.clear();
        m_entries.clear();
        for (auto entrySet : m_entrySets) {
            TableEntryAssociations::instance()->removeEntrySet(entrySet);
        }
        m_entrySets.clear();
        m_entrySetPositions.clear();
        endInsertRows();
    }
}
/*
//----------------------------------------------------------
QString AbstractEntryParserTable::associate(QModelIndexList &selectedRows,
        QList<QSharedPointer<AccountingEntrySet> > *entrySets)
{
    QList<QSharedPointer<AccountingEntrySet>> selectedEntrySets;
    QSet<int> rows;
    for (auto row : selectedRows) {
        rows << row.row();
    }
    double sumLeft = 0;
    double sumRight = 0;
    for (auto row : rows) {
        //m_entrySets[row]->connectTo(*entrySets);
        selectedEntrySets << m_entrySets[row];
        sumLeft += m_entrySets[row]->amountOrig();
        //emit dataChanged(index(row, 0), index(row, columnCount()-1));
    }
    for (auto entrySet : *entrySets) {
        //entrySet->connectTo(selectedEntrySets);
        sumRight += entrySet->amountOrig();
    }
    if (qAbs(qAbs(sumRight) - qAbs(sumLeft)) < 0.001) {
        for (auto row : rows) {
            m_entrySets[row]->connectTo(*entrySets);
            emit dataChanged(index(row, 0), index(row, columnCount()-1));
        }
        return "";
    } else {
        return tr("Les totaux ne correspondent pas: %1 - %2").arg(sumLeft).arg(sumRight);
    }
}
//----------------------------------------------------------
void AbstractEntryParserTable::dissociate(
        QSharedPointer<AccountingEntrySet> entrySet)
{
    auto connectedTo = entrySet->connectedTo();
    if (connectedTo.size() > 0) {
        auto connectedFrom = connectedTo[0]->connectedTo();
        for (auto entrySet : connectedFrom) {
            entrySet->disconnect();
            int row = m_entrySetPositions[entrySet.data()];
            emit dataChanged(index(row, 0), index(row, columnCount()-1));
        }
    }
}
//----------------------------------------------------------
void AbstractEntryParserTable::dissociate(const QModelIndex &sel)
{
    dissociate(m_entrySets[sel.row()]);
}
//*/
//----------------------------------------------------------
double AbstractEntryParserTable::sumEntry(QSet<QString> ids) const
{
    double sum = 0.;
    for (auto id : ids) {
        auto entrySet = m_entryById.value(
                    id, QSharedPointer<AccountingEntrySet>(nullptr));
        if (!entrySet.isNull()) {
            for (auto entry : entrySet->entries()) {
                sum += entry.debitOrigDouble();
            }
        }
    }
    return sum;
}
//----------------------------------------------------------
QVariant AbstractEntryParserTable::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        QStringList values
                = [this]() -> QStringList {
                QStringList vals;
                auto infos = *colInfos();
                for (auto info : *colInfos()) {
                    vals << info.name;
                }
                return vals;
    }();
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int AbstractEntryParserTable::rowCount(const QModelIndex &) const
{
    return m_entrySets.size();
}
//----------------------------------------------------------
int AbstractEntryParserTable::columnCount(const QModelIndex &) const
{
    return colInfos()->size();
}
//----------------------------------------------------------
void AbstractEntryParserTable::load(int year)
{
    m_entries.clear();
    int row = 0;
    fillEntries(m_entries, year);
    for (auto itYear = m_entries.begin();
         itYear != m_entries.end(); ++itYear) {
        QString year = QString::number(itYear.key());
        for (auto itJournal = itYear.value().begin();
             itJournal != itYear.value().end(); ++itJournal) {
            QString journal = itJournal.key();
            for (auto itMonth = itJournal.value().begin();
                 itMonth != itJournal.value().end(); ++itMonth) {
                QString month = itMonth.key();
                QString baseName = journal + "-" + year + "-" + month;
                QString csvFileName = baseName + ".csv";
                QString pdfFileName = baseName + ".pdf";
                for (auto itLabel = itMonth.value().begin();
                     itLabel != itMonth.value().end(); ++itLabel) {
                    auto entrySet = itLabel.value();
                    /*
                    QStringList elements;
                    elements << entrySet->id();
                    elements << entrySet->date().toString("yyyy-MM-dd");
                    elements << entrySet->journal();
                    auto entries = itLabel.value()->entries();
                    elements << entries[0].account();
                    elements << entries[1].account();
                    elements << entries[0].amount();
                    elements << entries[0].amountOrigCurrency();
                    elements << entrySet->label();
                    elements << entrySet->fileRelWorkingDir();
                    //*/
                    m_entrySets << entrySet;
                    //m_values << elements;
                    m_entryById[entrySet->id()] = entrySet;
                    m_entrySetPositions[entrySet.data()] = row;
                    TableEntryAssociations::instance()
                            ->recordEntrySet(entrySet);
                    ++row;
                }
            }
        }
    }
    beginInsertRows(QModelIndex(), 0, m_entrySets.size()-1);
    endInsertRows();
}
//----------------------------------------------------------
/*
QStringList AbstractTableEntryDisplay::_entrySetToList(
        AccountingEntrySet *entrySet)
{
    QStringList elements;
    elements << entrySet->id();
    elements << entrySet->date().toString("yyyy-MM-dd");
    elements << entrySet->journal();
    auto entries = entrySet->entries();
    elements << entries[0].account();
    elements << entries[1].account();
    elements << entries[0].amount();
    elements << entries[0].amountOrigCurrency();
    elements << entrySet->label();
    elements << entrySet->fileRelWorkingDir();
    return elements;
}
//*/
//----------------------------------------------------------
const AccountingEntries *AbstractEntryParserTable::entries() const
{
    return &m_entries;
}
//----------------------------------------------------------
QVariant AbstractEntryParserTable::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return (*colInfos())[index.column()].getValue(this, m_entrySets[index.row()].data());
        //return m_values[index.row()][index.column()];
    } else if (role == Qt::BackgroundRole) {
        auto entrySet = m_entrySets[index.row()];
        if (index.column() == 0) {
            if (entrySet->state() == AccountingEntrySet::Error) {
                return SettingManager::instance()->brushRed();
            } else if (entrySet->state() == AccountingEntrySet::Edited) {
                return SettingManager::instance()->brushOrange();
            }
        }
        QString entryId = entrySet->id();
        if (TableEntryAssociations::instance()->isAssociatedAndSelected(entryId)) {
            return SettingManager::instance()->brushOrange();
        } else if (TableEntryAssociations::instance()->isAssociated(entryId)) {
            return SettingManager::instance()->brushTurquoise();
        } else if (TableEntryAssociations::instance()->isSelfAssociated(entryId)) {
            return SettingManager::instance()->brushLightBlue();
        }
        //if (m_entrySets[index.row()]->isConnected()) {
            //return QBrush(QColor(Qt::green));
        //}
    }
    return QVariant();
}
//----------------------------------------------------------
Qt::ItemFlags AbstractEntryParserTable::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//----------------------------------------------------------
void AbstractEntryParserTable::sort(int column, Qt::SortOrder order)
{
    /*
    struct Gather{
        QStringList values;
        QSharedPointer<AccountingEntrySet> entrySet;
    };
    QList<Gather> gathered;
    for (int i=0; i<m_values.size(); ++i) {
        Gather gather;
        gather.values = m_values[i];
        gather.entrySet = m_entrySets[i];
    }
    //*/

    if (column == 5 || column == 6) {
     if (order == Qt::AscendingOrder) {
            std::sort(m_entrySets.begin(), m_entrySets.end(),
                      [this, column](const QSharedPointer<AccountingEntrySet> &v1, const QSharedPointer<AccountingEntrySet> &v2){
                auto infos = colInfos();
                return (*infos)[column].getValue(
                            this, v1.data()).toDouble()
                        < (*infos)[column].getValue(
                            this, v2.data()).toDouble();
            });
        } else {
            std::sort(m_entrySets.begin(), m_entrySets.end(),
                      [this, column](const QSharedPointer<AccountingEntrySet> &v1, const QSharedPointer<AccountingEntrySet> &v2){
                auto infos = colInfos();
                return (*infos)[column].getValue(
                            this, v1.data()).toDouble()
                        > (*infos)[column].getValue(
                            this, v2.data()).toDouble();
            });
        }
    } else {
        if (order == Qt::AscendingOrder) {
            std::sort(m_entrySets.begin(), m_entrySets.end(),
                      [this, column](const QSharedPointer<AccountingEntrySet> &v1, const QSharedPointer<AccountingEntrySet> &v2){
                auto infos = colInfos();
                //TODO compare as string or double
                return (*infos)[column].getValue(
                            this, v1.data())
                        < (*infos)[column].getValue(
                            this, v2.data());
            });
        } else {
            std::sort(m_entrySets.begin(), m_entrySets.end(),
                      [this, column](const QSharedPointer<AccountingEntrySet> &v1, const QSharedPointer<AccountingEntrySet> &v2){
                auto infos = colInfos();
                return (*infos)[column].getValue(
                            this, v1.data())
                        > (*infos)[column].getValue(
                            this, v2.data());
            });
        }
    }
    m_entrySetPositions.clear();
    int row = 0;
    for (auto entrySet : m_entrySets) {
        m_entrySetPositions[entrySet.data()] = row;
        ++row;
    }
    /*
    m_values.clear();
    m_entrySets.clear();
    m_entrySetPositions.clear();
    int row = 0;
    for (auto gather : gathered) {
        m_values << gather.values;
        m_entrySets << gather.entrySet;
        m_entrySetPositions[gather.entrySet.data()]
                = QPair<AbstractTableEntryDisplay *, int>(this, row);
        ++row;
    }
    //*/
    emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1),
                     QVector<int>() << Qt::DisplayRole);
}
//----------------------------------------------------------
QList<AbstractEntryParserTable::ColInfo> *AbstractEntryParserTable::colInfos() const {
    static QHash<QString, QList<AbstractEntryParserTable::ColInfo>> allColInfos;
    if (!allColInfos.contains(name())) {
        allColInfos[name()] = {{COL_ID, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
            return entrySet->id();
        }}
                ,{COL_DATE, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
            return entrySet->date();
        }}
                /*
                ,{COL_JOURNAL, [](const AccountingEntrySet *entrySet) -> QVariant{
            return entrySet->journal();
        }}
                //*/
                ,{COL_AMOUNT_CONV, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
            return entrySet->amountConv();
        }}
                ,{COL_AMOUNT_ORIG, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
            return entrySet->amountOrig();
        }}
                ,{COL_CURRENCY_ORIG, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
            return entrySet->entries()[0].currency();
        }}
                ,{COL_LABEL, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
            return entrySet->label();
        }}
                ,{COL_ACCOUNT_1, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
            return entrySet->entries()[0].accountReplaced();
        }}
                ,{COL_ACCOUNT_2, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
            if (entrySet->entries().size() <= 1) {
                return "";
            }
            return entrySet->entries()[1].accountReplaced();
        }}
                ,{COL_FILE_NAME_REL, [](const AbstractEntryParserTable *, const AccountingEntrySet *entrySet) -> QVariant{
            return entrySet->fileRelWorkingDir();
        }}
    };
    }
    return &allColInfos[name()];
}
//----------------------------------------------------------
void AbstractEntryParserTable::_emitChangeRowsOfEntrySetIds(
        const QSet<QString> &ids)
{
    for (auto id : ids) {
        _emitChangeRowsOfEntrySetIds(id);
    }
}
//----------------------------------------------------------
void AbstractEntryParserTable::_emitChangeRowsOfEntrySetIds(
        const QString &id)
{
    //TODO when I dissociate, check I go here and that after there is a color change
    if (m_entryById.contains(id)) {
        int position = m_entrySetPositions[m_entryById[id].data()];
        emit dataChanged(index(position, 0),
                         index(position, columnCount()-1),
                         QVector<int>() << Qt::BackgroundRole);
    }
}
//----------------------------------------------------------
void AbstractEntryParserTable::_addAssociatedIds(
        const QSet<QString> &ids)
{
    _emitChangeRowsOfEntrySetIds(ids);
}
//----------------------------------------------------------
void AbstractEntryParserTable::_removeAssociatedIds(
        const QSet<QString> &ids)
{
    _emitChangeRowsOfEntrySetIds(ids);
}
//----------------------------------------------------------
const QList<QSharedPointer<AccountingEntrySet> > *AbstractEntryParserTable::_entrySets() const
{
    return &m_entrySets;
}
//----------------------------------------------------------
void AbstractEntryParserTable::_setEntrySet(
        int index, QSharedPointer<AccountingEntrySet> entrySet)
{
    m_entrySets[index] = entrySet;
}
//----------------------------------------------------------
/*
void AbstractEntryParserTable::_saveAssociations()
{
    QDir dir = SettingManager::instance()->bookKeepingDirPurchase();
    for (auto itYear = m_entries.begin();
         itYear != m_entries.end(); ++itYear) {
        QString year = QString::number(itYear.key());
        for (auto itJournal = itYear.value().begin();
             itJournal != itYear.value().end(); ++itJournal) {
            QString journal = itJournal.key();
            for (auto itMonth = itJournal.value().begin();
                 itMonth != itJournal.value().end(); ++itMonth) {
                QString month = itMonth.key();
                QString baseName = journal + "-" + year + "-" + month;
                QString csvFileName = baseName + ".csv";
                QString pdfFileName = baseName + ".pdf";
                for (auto itLabel = itMonth.value().begin();
                     itLabel != itMonth.value().end(); ++itLabel) {
                    auto entrySet = itLabel.value();
                    QStringList elements;
// Year / journal / month / label
                    elements << entrySet->id();
                    //elements << entrySet->date().toString("yyyy-MM-dd");
                    //elements << entrySet->journal();
                    for (auto entrySetRight : entrySet->connectedTo()) {
                        QStringList elementsFinal = elements;
                        QString idRight = entrySet->id();
                        elementsFinal << idRight;
                    }
                }
            }
        }
    }
}
//----------------------------------------------------------
void AbstractEntryParserTable::_loadAssociations()
{
}
//*/
//----------------------------------------------------------
void AbstractEntryParserTable::addEntrySet(
        QSharedPointer<AccountingEntrySet> entrySet)
{
    beginInsertRows(QModelIndex(), 0, 0);
    //m_values.insert(0, _entrySetToList(entrySet.data()));
    m_entryById[entrySet->id()] = entrySet;
    m_entrySets.insert(0, entrySet);
    m_entrySetPositions[entrySet.data()] = 0;
    int year = entrySet->date().year();
    QString monthStr = QString::number(
                entrySet->date().month()).rightJustified(2, '0');
    m_entries[year][entrySet->journal()][monthStr].insert(entrySet->label(), entrySet);
    TableEntryAssociations::instance()->recordEntrySet(entrySet);
    endInsertRows();
}
//----------------------------------------------------------
void AbstractEntryParserTable::removeEntry(const QModelIndexList &indexes)
{
    QSet<int> rows;
    for (auto index : indexes) {
        rows << index.row();
    }
    QList<int> rowsSorted = rows.toList();
    std::sort(rowsSorted.begin(), rowsSorted.end());
    for (auto rowIt = rowsSorted.rbegin();
         rowIt != rowsSorted.rend(); ++rowIt) {
        int row = *rowIt;
        removeEntry(row);
    }
}
//----------------------------------------------------------
void AbstractEntryParserTable::removeEntry(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    auto entrySet = m_entrySets[row];
    m_entrySets.removeAt(row);
    m_entryById.remove(entrySet->id());
    //m_values.removeAt(row);
    m_entrySetPositions.remove(entrySet.data());
    TableEntryAssociations::instance()->removeEntrySet(entrySet);
    removeEntryStatic(m_entries, entrySet);
    endRemoveRows();
}
//----------------------------------------------------------

