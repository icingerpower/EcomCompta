#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>

#include "ManagerInventoryIssues.h"

//----------------------------------------------------------
QString ManagerInventoryIssues::UNKWOWN = QObject::tr("Inconnu");
QString ManagerInventoryIssues::UNAVAILABLE = QObject::tr("Stock manquant");;
//----------------------------------------------------------
ManagerInventoryIssues::ManagerInventoryIssues(QObject *parent)
    : QAbstractTableModel(parent), UpdateToCustomer ()
{
}
//----------------------------------------------------------
ManagerInventoryIssues *ManagerInventoryIssues::instance()
{
    static ManagerInventoryIssues instance;
    return &instance;
}
//----------------------------------------------------------
QString ManagerInventoryIssues::uniqueId() const
{
    return "ManagerInventoryIssues";
}
//----------------------------------------------------------
void ManagerInventoryIssues::onCustomerSelectedChanged(const QString &)
{
    clear();
}
//----------------------------------------------------------
void ManagerInventoryIssues::record(
        const QString &code,
        const QString &title,
        const QString &type,
        int unit)
{
    if (m_codeToIndex.contains(code)) {
        int position = m_codeToIndex[code];
        int previousUnit = m_values[position][3].toInt();
        int newUnit = previousUnit + unit;
        m_values[position][3] = QString::number(newUnit);
        auto indexUpdated = index(position, 3);
        emit dataChanged(indexUpdated,
                         indexUpdated,
                         QVector<int>() << Qt::DisplayRole);
    } else {
        beginInsertRows(QModelIndex(), m_values.size(), m_values.size());
        m_codeToIndex[code] = m_values.size();
        m_values << QStringList(
        {code, title, type, QString::number(unit)});
        endInsertRows();
    }
}
//----------------------------------------------------------
void ManagerInventoryIssues::exportUnknown(
        const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QFile::WriteOnly)) {
        QStringList lines;
        QStringList headerElements;
        for (int i=0; i<columnCount(); ++i) {
            headerElements << headerData(i, Qt::Horizontal).toString();
        }
        lines << headerElements.join("\t");
        for (auto it = m_values.begin(); it != m_values.end(); ++it) {
            if (it->value(2) == UNKWOWN) {
                QStringList elements;
                for (int i=0; i<columnCount(); ++i) {
                    elements << it->value(i);
                }
                lines << elements.join("\t");
            }
        }
        QTextStream stream(&file);
        stream << lines.join("\n");
        file.close();
    }
}
//----------------------------------------------------------
void ManagerInventoryIssues::exportAll(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QFile::WriteOnly)) {
        QStringList lines;
        QStringList headerElements;
        for (int i=0; i<columnCount(); ++i) {
            headerElements << headerData(i, Qt::Horizontal).toString();
        }
        lines << headerElements.join("\t");
        for (auto it = m_values.begin(); it != m_values.end(); ++it) {
            QStringList elements;
            for (int i=0; i<columnCount(); ++i) {
                elements << it->value(i);
            }
            lines << elements.join("\t");
        }
        QTextStream stream(&file);
        stream << lines.join("\n");
        file.close();
    }
}
//----------------------------------------------------------
QVariant ManagerInventoryIssues::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList titles
                = {tr("Code"), tr("Titre"), tr("Probleme"), tr("Quantite")};
        return titles[section];
    }
    return QVariant();
}
//----------------------------------------------------------
Qt::ItemFlags ManagerInventoryIssues::flags(
        const QModelIndex &index) const
{
    return Qt::ItemIsEnabled
            | Qt::ItemIsSelectable
            | Qt::ItemIsEditable;
}
//----------------------------------------------------------
int ManagerInventoryIssues::rowCount(const QModelIndex &) const
{
    return m_values.size();
}
//----------------------------------------------------------
int ManagerInventoryIssues::columnCount(const QModelIndex &) const
{
    return 4;
}
//----------------------------------------------------------
QVariant ManagerInventoryIssues::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_values[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
void ManagerInventoryIssues::clear()
{
    if (m_values.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_values.size()-1);
        m_values.clear();
        m_codeToIndex.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------

