#include "applauncher.h"
#include <QPainter>
#include <QPixmap>
#include <QMessageBox>
#include <QApplication>
#include <QTimer>
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
    mp_soundPlayer(new QMediaPlayer),
    menu_launcher(new QMenu),
    btn_appClose(new QPushButton),
    btn_subAppCaculateKey(new QPushButton),
    btn_appMinimize(new QPushButton),
    mp_bgmPlayer(new QMediaPlayer),
    mpl_bgmList(new QMediaPlaylist),
    btn_bgmPlayer(new QPushButton),
    btn_subAppCompressCArrayOfBitmap(new QPushButton)
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
        mp_soundPlayer->setVolume(30);
        mp_soundPlayer->setMedia(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/soundeffects/btnclicked.wav"));
        mp_soundPlayer->play();

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
    btn_subAppCaculateKey->setGeometry(100, 237, this->width() / 8, this->height() / 8);
    btn_subAppCaculateKey->setToolTip(QString::fromLocal8Bit("�����Կ����"));
    btn_subAppCaculateKey->installEventFilter(this);
    btn_subAppCaculateKey->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/sunset.png);border-radius: 5px;}"
                                         "QPushButton:hover{border:2px;}"
                                         "QPushButton:pressed{border:4px;}");
    connect(btn_subAppCaculateKey, &btn_subAppCaculateKey->clicked, this, [this](){
        mp_soundPlayer->setVolume(100);
        mp_soundPlayer->setMedia(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/soundeffects/btnclicked2.wav"));
        mp_soundPlayer->play();

        appFirmwareGenerator->switchFunctionPage(FirmwareGenerator::CMD_HANDLER_CALCULATE_KEY);
        appFirmwareGenerator->show();
        appFirmwareGenerator->dealWithCalculateKeyCommand();
    });

    //launcher��ť6����
    btn_appMinimize->setParent(this);
    btn_appMinimize->setGeometry(205, 292, this->width() / 12, this->height() / 12);
    btn_appMinimize->setToolTip(QString::fromLocal8Bit("��С��"));
    btn_appMinimize->installEventFilter(this);
    btn_appMinimize->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/watermelon.png);border-radius: 5px;}"
                                   "QPushButton:hover{border:2px;}"
                                   "QPushButton:pressed{border:4px;}");
    connect(btn_appMinimize, &btn_appMinimize->clicked, this, [this](){
        mp_soundPlayer->setVolume(100);
        mp_soundPlayer->setMedia(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/soundeffects/btnclicked2.wav"));
        mp_soundPlayer->play();

        this->showMinimized();
    });

    //��������
    mpl_bgmList->addMedia(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/bgm/work.mp3"));
    mpl_bgmList->addMedia(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/bgm/summer.mp3"));
    mpl_bgmList->addMedia(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/bgm/qianxun.mp3"));

    mp_bgmPlayer->setMedia(mpl_bgmList);
    mp_bgmPlayer->setVolume(80);
    mp_bgmPlayer->setPlaybackRate(1.0f);
    QTimer::singleShot(500, mp_bgmPlayer, &mp_bgmPlayer->play);
    connect(mp_bgmPlayer, &mp_bgmPlayer->stateChanged, this, [this](QMediaPlayer::State state){
        qDebug() << state;
        if(QMediaPlayer::StoppedState == state)
            mp_bgmPlayer->play();
    });

    //launcher��ť7����
    btn_bgmPlayer->setParent(this);
    btn_bgmPlayer->setGeometry(60, 287, this->width() / 11, this->height() / 11);
    btn_bgmPlayer->setToolTip(QString::fromLocal8Bit("��ͣ��������"));
    btn_bgmPlayer->installEventFilter(this);
    btn_bgmPlayer->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/music2.png);border-radius: 5px;}"
                                 "QPushButton:hover{border:2px;}"
                                 "QPushButton:pressed{border:4px;}");
    connect(btn_bgmPlayer, &btn_bgmPlayer->clicked, this, [this](){
        mp_soundPlayer->setVolume(100);
        mp_soundPlayer->setMedia(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/soundeffects/btnclicked2.wav"));
        mp_soundPlayer->play();

        if(QMediaPlayer::PlayingState == mp_bgmPlayer->state())
        {
            mp_bgmPlayer->pause();
            btn_bgmPlayer->setToolTip(QString::fromLocal8Bit("���ű�������"));
            btn_bgmPlayer->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/play-button2.png);border-radius: 5px;}"
                                         "QPushButton:hover{border:2px;}"
                                         "QPushButton:pressed{border:4px;}");
        }
        else
        {
            mp_bgmPlayer->play();
            btn_bgmPlayer->setToolTip(QString::fromLocal8Bit("��ͣ��������"));
            btn_bgmPlayer->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/music2.png);border-radius: 5px;}"
                                         "QPushButton:hover{border:2px;}"
                                         "QPushButton:pressed{border:4px;}");
        }
    });

    //launcher��ť8����
    btn_subAppCompressCArrayOfBitmap->setParent(this);
    btn_subAppCompressCArrayOfBitmap->setGeometry(165, 239, this->width() / 9, this->height() / 9);
    btn_subAppCompressCArrayOfBitmap->setToolTip(QString::fromLocal8Bit("Image2Lcd���ɵ�C�����ѹ������"));
    btn_subAppCompressCArrayOfBitmap->installEventFilter(this);
    btn_subAppCompressCArrayOfBitmap->setStyleSheet("QPushButton{border-image: url(:qrc:/../resources/icons/cactus.png);border-radius: 5px;}"
                                                    "QPushButton:hover{border:2px;}"
                                                    "QPushButton:pressed{border:4px;}");
    connect(btn_subAppCompressCArrayOfBitmap, &btn_subAppCompressCArrayOfBitmap->clicked, this, [this](){
        mp_soundPlayer->setVolume(100);
        mp_soundPlayer->setMedia(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/soundeffects/btnclicked2.wav"));
        mp_soundPlayer->play();

        appFirmwareGenerator->switchFunctionPage(FirmwareGenerator::CMD_HANDLER_COMPRESS_ARRAY_OF_BMP);
        appFirmwareGenerator->show();
        appFirmwareGenerator->compressCArrayOfBitmap();
    });

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

    if(Qt::RightButton == event->button())
    {
        mp_soundPlayer->setVolume(100);
        mp_soundPlayer->setMedia(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/soundeffects/btnclicked2.wav"));
        mp_soundPlayer->play();
    }

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
            mp_soundPlayer->setVolume(100);
            mp_soundPlayer->setMedia(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/soundeffects/shua.wav"));
            mp_soundPlayer->play();
        }
        else if(btn_subAppCaculateKey == watched
                || btn_subAppCompressCArrayOfBitmap == watched)
        {
            mp_soundPlayer->setVolume(100);
            mp_soundPlayer->setMedia(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/soundeffects/huaguo.wav"));
            mp_soundPlayer->play();
        }
        else if(btn_appClose == watched
                || btn_appMinimize == watched
                || btn_bgmPlayer == watched)
        {
            mp_soundPlayer->setVolume(100);
            mp_soundPlayer->setMedia(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/soundeffects/skype.wav"));
            mp_soundPlayer->play();
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
    if(Qt::Key_Alt == event->key())
    {
        menu_launcher->move(geometry().x() + width() / 2, geometry().y() + height() / 2);
        menu_launcher->show();
    }

    QWidget::keyPressEvent(event);
}
