#ifndef PANEPARAMS_H
#define PANEPARAMS_H

#include <QWidget>

namespace Ui {
class PaneParams;
}

class PaneParams : public QWidget
{
    Q_OBJECT

public:
    explicit PaneParams(QWidget *parent = nullptr);
    ~PaneParams();

private:
    Ui::PaneParams *ui;
    void _connectSlots();
    void _showParams();
    void _hideParams();
};

#endif // PANEPARAMS_H
