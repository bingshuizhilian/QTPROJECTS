#include "applauncher.h"
#include <QPainter>
#include <QPixmap>
#include <QMessageBox>
#include <QDebug>

AppLauncher::AppLauncher(QWidget *parent) :
    QDialogButtonBox(parent),
    btn_appFirmwareGenerator(new QPushButton),
    btn_appBmpToCArray(new QPushButton),
    btn_appCanLogSeparator(new QPushButton),
    appFirmwareGenerator(new FirmwareGenerator),
    appBmpToCArray(new BitmapProcess),
    appCanLogSeparator(new CanLogSeparator),
    mouseMovePos(QPoint(0, 0)),
    se_soundPlayer(new QSoundEffect),
    menu_launcher(new QMenu),
    btn_appClose(new QPushButton),
    btn_subAppCaculateKey(new QPushButton)
{
    //launcher设置
    setWindowTitle(QString::fromLocal8Bit("Launcher"));
    setCenterButtons(true);
    setFixedSize(330, 330);
    setWindowOpacity(1);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    installEventFilter(this);

    //子窗体设置为模态显示
    appFirmwareGenerator->setWindowModality(Qt::ApplicationModal);
    appBmpToCArray->setWindowModality(Qt::ApplicationModal);
    appCanLogSeparator->setWindowModality(Qt::ApplicationModal);

    //launcher按钮1设置
    addButton(btn_appFirmwareGenerator, QDialogButtonBox::AcceptRole);
    btn_appFirmwareGenerator->installEventFilter(this);
    btn_appFirmwareGenerator->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/iced-tea.png);border-radius: 5px;}"
                                            "QPushButton:hover{border:2px;}"
                                            "QPushButton:pressed{border:4px;}");
    btn_appFirmwareGenerator->setToolTip(QString::fromLocal8Bit("固件合成"));
    btn_appFirmwareGenerator->setFixedSize(80, 80);

    //launcher按钮2设置
    addButton(btn_appBmpToCArray, QDialogButtonBox::AcceptRole);
    btn_appBmpToCArray->installEventFilter(this);
    btn_appBmpToCArray->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/popsicle.png);border-radius: 5px;}"
                                      "QPushButton:hover{border:2px;}"
                                      "QPushButton:pressed{border:4px;}");
    btn_appBmpToCArray->setToolTip(QString::fromLocal8Bit("bmp图片转C语言数组"));
    btn_appBmpToCArray->setFixedSize(80, 80);

    //launcher按钮3设置
    addButton(btn_appCanLogSeparator, QDialogButtonBox::AcceptRole);
    btn_appCanLogSeparator->installEventFilter(this);
    btn_appCanLogSeparator->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/palm-tree.png);border-radius: 5px;}"
                                          "QPushButton:hover{border:2px;}"
                                          "QPushButton:pressed{border:4px;}");
    btn_appCanLogSeparator->setToolTip(QString::fromLocal8Bit("can日志分析"));
    btn_appCanLogSeparator->setFixedSize(80, 80);

    //launcher按钮1、2、3的响应事件
    connect(this, &clicked, this, [=](QAbstractButton* b){
        se_soundPlayer->setVolume(0.2f);
        se_soundPlayer->setSource(QUrl::fromLocalFile(":qrc:/../resources/soundeffects/btnclicked.wav"));
        se_soundPlayer->play();

        if(b == btn_appFirmwareGenerator)
            appFirmwareGenerator->show();
        else if(b == btn_appBmpToCArray)
            appBmpToCArray->show();
        else if(b == btn_appCanLogSeparator)
            appCanLogSeparator->show();

        qDebug() << width() << height();
    });

    //launcher按钮4设置
    btn_appClose->setParent(this);
    btn_appClose->setGeometry(240, 285, this->width() / 8, this->height() / 8);
    btn_appClose->setToolTip(QString::fromLocal8Bit("关闭"));
    btn_appClose->installEventFilter(this);
    btn_appClose->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/pamela.png);border-radius: 5px;}"
                                "QPushButton:hover{border:2px;}"
                                "QPushButton:pressed{border:4px;}");
    connect(btn_appClose, &btn_appClose->clicked, this, close);

    //launcher按钮5设置
    btn_subAppCaculateKey->setParent(this);
    btn_subAppCaculateKey->setGeometry(90, 237, this->width() / 8, this->height() / 8);
    btn_subAppCaculateKey->setToolTip(QString::fromLocal8Bit("诊断密钥计算"));
    btn_subAppCaculateKey->installEventFilter(this);
    btn_subAppCaculateKey->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/sunset.png);border-radius: 5px;}"
                                         "QPushButton:hover{border:2px;}"
                                         "QPushButton:pressed{border:4px;}");
    connect(btn_subAppCaculateKey, &btn_subAppCaculateKey->clicked, this, [this](){
        se_soundPlayer->setVolume(0.8f);
        se_soundPlayer->setSource(QUrl::fromLocalFile(":qrc:/../resources/soundeffects/btnclicked2.wav"));
        se_soundPlayer->play();

        appFirmwareGenerator->switchFunctionPage(FirmwareGenerator::CMD_HANDLER);
        appFirmwareGenerator->show();
        appFirmwareGenerator->dealWithCalculateKeyCommand();
    });

    //程序启动音效
    se_soundPlayer->setLoopCount(1);
    se_soundPlayer->setVolume(0.8f);
    se_soundPlayer->setSource(QUrl::fromLocalFile(":qrc:/../resources/soundeffects/light.wav"));
    se_soundPlayer->play();

    //右键菜单
    menu_launcher->addAction(QIcon(":qrc:/../resources/icons/paper-plane.png"), QString::fromLocal8Bit("关于(&A)"), this, [this](){
        QString info(QString::fromLocal8Bit("开发助手\n"));
        info.append(QString::fromLocal8Bit("版本：") + appFirmwareGenerator->getVersion() + "\n");
        info.append(QString::fromLocal8Bit("作者：ybs@HEU\n"));
        info.append(QString::fromLocal8Bit("邮箱：bingshuizhilian@yeah.net"));
        QMessageBox::information(this, QString::fromLocal8Bit("关于"), info, QString::fromLocal8Bit("好"));
    },
    QKeySequence(Qt::ALT | Qt::Key_A)
    );
    menu_launcher->addAction(QIcon(":qrc:/../resources/icons/close.png"), QString::fromLocal8Bit("关闭(&C)"), this, SLOT(close()), QKeySequence(Qt::ALT | Qt::Key_C));
}

AppLauncher::~AppLauncher()
{

}

void AppLauncher::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.drawPixmap(QRect(0, 0, this->width(), this->height()), QPixmap(":qrc:/../resources/icons/shack.png"));

    QWidget::paintEvent(event);
}

void AppLauncher::mouseMoveEvent(QMouseEvent *event)
{
    if(mouseMovePos != QPoint(0, 0))
    {
        move(geometry().x() + event->globalPos().x() - mouseMovePos.x(),
              geometry().y() + event->globalPos().y() - mouseMovePos.y());
        mouseMovePos = event->globalPos();
    }

    QWidget::mouseMoveEvent(event);
}

void AppLauncher::mousePressEvent(QMouseEvent *event)
{
    mouseMovePos = event->globalPos();

    QWidget::mousePressEvent(event);
}

void AppLauncher::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    mouseMovePos = QPoint(0, 0);

    QWidget::mouseReleaseEvent(event);
}

bool AppLauncher::eventFilter(QObject *watched, QEvent *event)
{
    if(QEvent::Enter == event->type())
    {
        if(btn_appFirmwareGenerator == watched
                || btn_appBmpToCArray == watched
                || btn_appCanLogSeparator == watched)
        {
            se_soundPlayer->setVolume(0.8f);
            se_soundPlayer->setSource(QUrl::fromLocalFile(":qrc:/../resources/soundeffects/shua.wav"));
            se_soundPlayer->play();
        }
        else if(btn_subAppCaculateKey == watched)
        {
            se_soundPlayer->setVolume(0.8f);
            se_soundPlayer->setSource(QUrl::fromLocalFile(":qrc:/../resources/soundeffects/huaguo.wav"));
            se_soundPlayer->play();
        }
        else if(btn_appClose == watched)
        {
            se_soundPlayer->setVolume(1.0f);
            se_soundPlayer->setSource(QUrl::fromLocalFile(":qrc:/../resources/soundeffects/skype.wav"));
            se_soundPlayer->play();
        }
    }

    return QWidget::eventFilter(watched, event);
}

void AppLauncher::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);

    menu_launcher->move(cursor().pos());
    menu_launcher->show();

    QWidget::contextMenuEvent(event);
}

void AppLauncher::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Alt)
    {
        menu_launcher->move(geometry().x() + width() / 2, geometry().y() + height() / 2);
        menu_launcher->show();
    }

    QWidget::keyPressEvent(event);
}
