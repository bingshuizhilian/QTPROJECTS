﻿#ifndef FIRMWAREGENERATOR_H
#define FIRMWAREGENERATOR_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLayout>
#include <QPlainTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class FirmwareGenerator : public QMainWindow
{
    Q_OBJECT

public:
    explicit FirmwareGenerator(QWidget *parent = 0);
    ~FirmwareGenerator();

private:
    Ui::MainWindow *ui;

public:
    enum FunctionType
    {
        BOOTLOADER,
        DIAGNOSIS,
        CMD_HANDLER,
        CMD_HANDLER_CALCULATE_KEY,
        CMD_HANDLER_COMPRESS_ARRAY_OF_BMP
    };

private:
    enum FileInfoType
    {
        ABSOLUTE_FILE_PATH,
        ABSOLUTE_PATH,
        FILE_NAME
    };

    enum PlatformType
    {
        M1AFL2,
        T19,
        S51EVFL,
        A13TEV
    };

    enum CmdType
    {
        CMD_HELP,
        CMD_HELP_BOOTLOADER,
        CMD_HELP_DIAGNOSIS,
        CMD_HELP_DIAG_CALCULATE_KEY,
        CMD_CLEAR_SCREEN,
        CMD_FULL_SCREEN,
        CMD_NORMAL_SCREEN,
        CMD_QUIT_APPLICATION,
        CMD_SAVE_CONFIG_FILE,
        CMD_LOAD_CONFIG_FILE,
        CMD_CODE_TO_STRING,
        CMD_COMPRESS_BMP,
        CMD_GEN_FLASH_DRIVER,
        CMD_GEN_ERASE_EEPROM,
        CMD_GEN_M1_BOOT_CODE,
        CMD_GEN_T1_BOOT_CODE,
        CMD_GEN_S51EVFL_BOOT_CODE,
        CMD_GEN_A13TEV_BOOT_CODE,
        CMD_DIAG_CALCULATE_KEY,
#if WIN32
        CMD_WINDOWS_COMMON,
        CMD_WINDOWS_CALCULATOR,
        CMD_WINDOWS_TASKMANAGER,
        CMD_WINDOWS_PAINT,
#endif
        CMD_MAX
    };

    enum WindowSizeType
    {
        WINDOW_HEIGHT = 240,
        WINDOW_WIDTH  = 334
    };

    const QString SOFTWARE_VERSION = "v1.7";
    const QString CONFIG_FILE_NAME = "config.json";
    QString versionFilePathName = "/version.txt";
    const QString appNameFirst = "IcmDevelopmentToolbox_";
    const QString appNameLast = "_boxed.exe";
    QString appFilePathName;

    const QString VERSION_DOWNLOAD_URL = "https://raw.githubusercontent.com/bingshuizhilian/QTPROJECTS-ICM-DEVELOPMENT-TOOLBOX/master/autoupdate/version.txt";
    const QString APP_DOWNLOAD_URL = "https://raw.githubusercontent.com/bingshuizhilian/QTPROJECTS-ICM-DEVELOPMENT-TOOLBOX/master/autoupdate/";

    const QStringList FUNCTION_STRING_LIST =
    {
        "add bootloader to firmware",
        "gen firmware for diagnosis",
        "run command"
    };

    const QStringList PLATFORM_STRING_LIST =
    {
        "m1afl2",
        "t19",
        "s51evfl",
        "a13tev"
    };

    const QString REPLACE_STRING = "S10BFFF8C000C000C000C000FD\n";//未加bootloader的app含有此内容
    const QString TARGET_STRING_AFTER_GENERATING_BOOTCODE = "S10BFFF8FC00FC00FC00FC000D\n";//已加bootloader的app含有此内容
    const QString DIAG_COMMON_S0  = "S02100000747395957202020202020202020202020202020200130302E30302E303040";//通用的S021行
    const QString DIAG_M1AFL2_PARTNUMBER = "J60-3820010FL";
    const QString DIAG_T19_PARTNUMBER = "701000068AA";
    const QString DIAG_S51EVFL_PARTNUMBER = "J72-3820010FA";
    const QString DIAG_A13TEV_PARTNUMBER = "701000095AA";

private:
    QStringList fileInfo;
    QList<QPair<CmdType, QStringList>> cmdList;

private:
    QPushButton* m_btnChooseFile;
    QLineEdit* m_leFileInfo;
    QComboBox* m_cmbPlatformSwitch;
    QCheckBox* m_ckbUseDefaultBootloader;
    QPushButton* m_btnLoadBootloader;
    QLineEdit* m_leBootloaderInfo;
    QPushButton* m_btnGenerate;
    QComboBox* m_cmbFunctionSwitch;
    QLineEdit* m_leRunCommand;
    QLineEdit* m_leDiagnosisS021;
    QLineEdit* m_leDiagnosisS20C;
    QPlainTextEdit* ptOutputWnd;

private:
    QVBoxLayout* m_layoutGlobal;
    QGroupBox* m_gbBootloader;
    QGroupBox* m_gbS19Selector;
    QGroupBox* m_gbDiagnosis;
    QGroupBox* m_gbSwitchFunction;

public:
    inline QString getVersion(void){return SOFTWARE_VERSION;}
    void dealWithCalculateKeyCommand(void);
    void switchFunctionPage(FunctionType functype);
    void compressCArrayOfBitmap(void);

private:
    void componentsInitialization(void);
    void layoutsInitialization(void);
    void commandsInitialization(void);
    void generateFirmwareWithBootloader(void);
    void generateFirmwareForDiagnosis(void);
    void generateFiles(CmdType cmd, QString dir_path, bool is_open_folder, QString user_part_number = "");
    void generateCharArray(void);
    bool sortS19Code(QStringList &originalStringList);
    int hexCharToHex(char src);
    unsigned short calcCRC16(QList<unsigned char> data_list);
    unsigned char calcChecksum(unsigned short crc);
    unsigned short calculateKey(unsigned short seed);
    void showHelpInfo(CmdType cmd);
    void procConfigFile(CmdType cmd);
    void autoUpdate(QString local_version);
    void autoUpdateTypeB(void);

private:
    QNetworkAccessManager* m_networkAccessMngr;
    QNetworkReply* m_httpReply;
    QFile* downloadFile;
    QTimer* versionDetectTimer;
    QTimer* appDetectTimer;

private slots:
    void selectFilePressed();
    void useDefaultBootloaderPressed();
    void loadBootloaderPressed();
    void s021ReturnPressed();
    void switchFunctionPressed();
    void generateButtonPressed();
    void runCmdReturnPressed();
    void switchPlatformPressed();
    void httpReadContent();
    void httpReplyFinished(QNetworkReply* reply);
    void httpDownloadError(QNetworkReply::NetworkError error);
    void httpDownloadProgress(qint64 bytes_received, qint64 bytes_total);
    void versionDetectTimerTimeout();
    void appDetectTimerTimeout();
};

#endif // FIRMWAREGENERATOR_H
