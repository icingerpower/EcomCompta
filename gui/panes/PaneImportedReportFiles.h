#ifndef PANEIMPORTEDREPORTFILES_H
#define PANEIMPORTEDREPORTFILES_H

#include <QtWidgets/qwidget.h>

namespace Ui {
class PaneImportedReportFiles;
}

class PaneImportedReportFiles : public QWidget
{
    Q_OBJECT

public:
    explicit PaneImportedReportFiles(QWidget *parent = nullptr);
    ~PaneImportedReportFiles();

public slots:
    void removeSelection();
    void expandAll();
    void sortAll();

private:
    Ui::PaneImportedReportFiles *ui;
    void _connectSlots();
};

#endif // PANEIMPORTEDREPORTFILES_H
