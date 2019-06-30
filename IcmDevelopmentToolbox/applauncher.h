#ifndef APPLAUNCHER_H
#define APPLAUNCHER_H

#include "firmwaregenerator.h"
#include "bitmapprocess.h"
#include "canlogseparator.h"
#include <QDialogButtonBox>
#include <QSoundEffect>
#include <QMenu>

class AppLauncher : public QDialogButtonBox
{
public:
    explicit AppLauncher(QWidget *parent = 0);
    ~AppLauncher();

private:
    QPushButton* btn_appFirmwareGenerator;
    QPushButton* btn_appBmpToCArray;
    QPushButton* btn_appCanLogSeparator;
    FirmwareGenerator* appFirmwareGenerator;
    BitmapProcess* appBmpToCArray;
    CanLogSeparator* appCanLogSeparator;
    QPoint mouseMovePos;
    QSoundEffect* se_soundPlayer;
    QMenu* menu_launcher;
    QPushButton* btn_appClose;

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // LAUNCHER_H
