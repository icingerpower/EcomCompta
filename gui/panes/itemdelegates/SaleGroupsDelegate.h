#ifndef SALEGROUPSDELEGATE_H
#define SALEGROUPSDELEGATE_H


#include <qstyleditemdelegate.h>

class SaleGroupsDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit SaleGroupsDelegate(QObject *parent = nullptr);
    QWidget *createEditor(
            QWidget *parent,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const override;
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;
};

#endif // SALEGROUPSDELEGATE_H
