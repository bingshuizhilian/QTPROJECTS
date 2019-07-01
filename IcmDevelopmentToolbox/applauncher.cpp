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
    //launcher����
    setWindowTitle(QString::fromLocal8Bit("Launcher"));
    setCenterButtons(true);
    setFixedSize(330, 330);
    setWindowOpacity(1);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    installEventFilter(this);

    //�Ӵ�������Ϊģ̬��ʾ
    appFirmwareGenerator->setWindowModality(Qt::ApplicationModal);
    appBmpToCArray->setWindowModality(Qt::ApplicationModal);
    appCanLogSeparator->setWindowModality(Qt::ApplicationModal);

    //launcher��ť1����
    addButton(btn_appFirmwareGenerator, QDialogButtonBox::AcceptRole);
    btn_appFirmwareGenerator->installEventFilter(this);
    btn_appFirmwareGenerator->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/iced-tea.png);border-radius: 5px;}"
                                            "QPushButton:hover{border:2px;}"
                                            "QPushButton:pressed{border:4px;}");
    btn_appFirmwareGenerator->setToolTip(QString::fromLocal8Bit("�̼��ϳ�"));
    btn_appFirmwareGenerator->setFixedSize(80, 80);

    //launcher��ť2����
    addButton(btn_appBmpToCArray, QDialogButtonBox::AcceptRole);
    btn_appBmpToCArray->installEventFilter(this);
    btn_appBmpToCArray->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/popsicle.png);border-radius: 5px;}"
                                      "QPushButton:hover{border:2px;}"
                                      "QPushButton:pressed{border:4px;}");
    btn_appBmpToCArray->setToolTip(QString::fromLocal8Bit("bmpͼƬתC��������"));
    btn_appBmpToCArray->setFixedSize(80, 80);

    //launcher��ť3����
    addButton(btn_appCanLogSeparator, QDialogButtonBox::AcceptRole);
    btn_appCanLogSeparator->installEventFilter(this);
    btn_appCanLogSeparator->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/palm-tree.png);border-radius: 5px;}"
                                          "QPushButton:hover{border:2px;}"
                                          "QPushButton:pressed{border:4px;}");
    btn_appCanLogSeparator->setToolTip(QString::fromLocal8Bit("can��־����"));
    btn_appCanLogSeparator->setFixedSize(80, 80);

    //launcher��ť1��2��3����Ӧ�¼�
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

    //launcher��ť4����
    btn_appClose->setParent(this);
    btn_appClose->setGeometry(240, 285, this->width() / 8, this->height() / 8);
    btn_appClose->setToolTip(QString::fromLocal8Bit("�ر�"));
    btn_appClose->installEventFilter(this);
    btn_appClose->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/pamela.png);border-radius: 5px;}"
                                "QPushButton:hover{border:2px;}"
                                "QPushButton:pressed{border:4px;}");
    connect(btn_appClose, &btn_appClose->clicked, this, close);

    //launcher��ť5����
    btn_subAppCaculateKey->setParent(this);
    btn_subAppCaculateKey->setGeometry(90, 237, this->width() / 8, this->height() / 8);
    btn_subAppCaculateKey->setToolTip(QString::fromLocal8Bit("�����Կ����"));
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

    //����������Ч
    se_soundPlayer->setLoopCount(1);
    se_soundPlayer->setVolume(0.8f);
    se_soundPlayer->setSource(QUrl::fromLocalFile(":qrc:/../resources/soundeffects/light.wav"));
    se_soundPlayer->play();

    //�Ҽ��˵�
    menu_launcher->addAction(QIcon(":qrc:/../resources/icons/paper-plane.png"), QString::fromLocal8Bit("����(&A)"), this, [this](){
        QString info(QString::fromLocal8Bit("��������\n"));
        info.append(QString::fromLocal8Bit("�汾��") + appFirmwareGenerator->getVersion() + "\n");
        info.append(QString::fromLocal8Bit("���ߣ�ybs@HEU\n"));
        info.append(QString::fromLocal8Bit("���䣺bingshuizhilian@yeah.net"));
        QMessageBox::information(this, QString::fromLocal8Bit("����"), info, QString::fromLocal8Bit("��"));
    },
    QKeySequence(Qt::ALT | Qt::Key_A)
    );
    menu_launcher->addAction(QIcon(":qrc:/../resources/icons/close.png"), QString::fromLocal8Bit("�ر�(&C)"), this, SLOT(close()), QKeySequence(Qt::ALT | Qt::Key_C));
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
