#ifndef APPLAUNCHER_H
#define APPLAUNCHER_H

#include "firmwaregenerator.h"
#include "bitmapprocess.h"
#include "canlogseparator.h"
#include <QDialogButtonBox>

class AppLancher : public QDialogButtonBox
{
public:
    explicit AppLancher(QWidget *parent = 0);
    ~AppLancher();

private:
    QPushButton* btn_appFirmwareGenerator;
    QPushButton* btn_appBmpToCArray;
    QPushButton* btn_appCanLogSeparator;
    FirmwareGenerator* appFirmwareGenerator;
    BitmapProcess* appBmpToCArray;
    CanLogSeparator* appCanLogSeparator;
    QPoint mouseMovePos;

protected:
    void paintEvent(QPaintEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
};

#endif // LAUNCHER_H
