#include "FileSystemModelNoDir.h"

//----------------------------------------------------------
FileSystemModelNoDir::FileSystemModelNoDir(QObject *parent)
    :QFileSystemModel (parent)
{
}
//----------------------------------------------------------
Qt::ItemFlags FileSystemModelNoDir::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        QString fileOrDirName = index.data().toString();
        if (!fileOrDirName.endsWith(".txt") && !fileOrDirName.endsWith(".csv")) {
            return Qt::ItemIsEnabled;
        }
    }
    return QFileSystemModel::flags(index);
}
//----------------------------------------------------------
