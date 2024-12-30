#ifndef FILESYSTEMMODELNODIR_H
#define FILESYSTEMMODELNODIR_H

#include <QFileSystemModel>

class FileSystemModelNoDir : public QFileSystemModel
{
    Q_OBJECT
public:
    FileSystemModelNoDir(QObject *parent);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
};

#endif // FILESYSTEMMODELNODIR_H
