#include "applauncher.h"
#include <QPainter>
#include <QPixmap>
#include <QDebug>

AppLancher::AppLancher(QWidget *parent) :
    QDialogButtonBox(parent),
    btn_appFirmwareGenerator(new QPushButton),
    btn_appBmpToCArray(new QPushButton),
    btn_appCanLogSeparator(new QPushButton),
    appFirmwareGenerator(new FirmwareGenerator),
    appBmpToCArray(new BitmapProcess),
    appCanLogSeparator(new CanLogSeparator),
    mouseMovePos(QPoint(0, 0))
{
    setWindowTitle(QString::fromLocal8Bit("Launcher"));
    setToolTip(QString::fromLocal8Bit("开发助手") + appFirmwareGenerator->getVersion() + QString::fromLocal8Bit("，ALT+F4退出，bingshuizhilian@yeah.net"));

    addButton(btn_appFirmwareGenerator, QDialogButtonBox::AcceptRole);
    btn_appFirmwareGenerator->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/iced-tea.png);border-radius: 5px;}"
                                            "QPushButton:hover{border:2px;}"
                                            "QPushButton:pressed{border:4px;}");
    btn_appFirmwareGenerator->setToolTip(QString::fromLocal8Bit("固件合成"));
    btn_appFirmwareGenerator->setFixedSize(80, 80);

    addButton(btn_appBmpToCArray, QDialogButtonBox::AcceptRole);
    btn_appBmpToCArray->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/popsicle.png);border-radius: 5px;}"
                                      "QPushButton:hover{border:2px;}"
                                      "QPushButton:pressed{border:4px;}");
    btn_appBmpToCArray->setToolTip(QString::fromLocal8Bit("bmp图片转C语言数组"));
    btn_appBmpToCArray->setFixedSize(80, 80);

    addButton(btn_appCanLogSeparator, QDialogButtonBox::AcceptRole);
    btn_appCanLogSeparator->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/palm-tree.png);border-radius: 5px;}"
                                          "QPushButton:hover{border:2px;}"
                                          "QPushButton:pressed{border:4px;}");
    btn_appCanLogSeparator->setToolTip(QString::fromLocal8Bit("can日志分析"));
    btn_appCanLogSeparator->setFixedSize(80, 80);

    setCenterButtons(true);
    setFixedSize(250, 250);

    connect(this, &clicked, this, [=](QAbstractButton* b){
        if(b == btn_appFirmwareGenerator)
            appFirmwareGenerator->show();
        else if(b == btn_appBmpToCArray)
            appBmpToCArray->show();
        else if(b == btn_appCanLogSeparator)
            appCanLogSeparator->show();

        qDebug() << width() << height();
    });

    appFirmwareGenerator->setWindowModality(Qt::ApplicationModal);
    appBmpToCArray->setWindowModality(Qt::ApplicationModal);
    appCanLogSeparator->setWindowModality(Qt::ApplicationModal);

    setWindowOpacity(1);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
}

AppLancher::~AppLancher()
{

}

void AppLancher::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::gray);
    painter.drawPixmap(QRect(0, 0, this->width(), this->height()), QPixmap(":qrc:/../resources/icons/pamela.png"));
}

void AppLancher::mouseMoveEvent(QMouseEvent *e)
{
    if(mouseMovePos != QPoint(0, 0))
    {
        move(geometry().x() + e->globalPos().x() - mouseMovePos.x(),
              geometry().y() + e->globalPos().y() - mouseMovePos.y());
        mouseMovePos = e->globalPos();
    }
}

void AppLancher::mousePressEvent(QMouseEvent *e)
{
    mouseMovePos = e->globalPos();
}

void AppLancher::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);

    mouseMovePos = QPoint(0, 0);
}
