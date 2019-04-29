#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLayout>
#include <QPlainTextEdit>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private:
    enum FileInfoType
    {
        ABSOLUTE_FILE_PATH,
        ABSOLUTE_PATH,
        FILE_NAME
    };

    enum FunctionType
    {
        BOOTLOADER,
        DIAGNOSIS,
        CMD_HANDLER
    };

    enum PlatformType
    {
        M1AFL2,
        T18,
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
        CMD_GEN_M1AFL2_FLASH_DRIVER,
        CMD_GEN_T19_FLASH_DRIVER,
        CMD_GEN_S51EVFL_FLASH_DRIVER,
        CMD_GEN_A13TEV_FLASH_DRIVER,
        CMD_GEN_COMMON_FLASH_DRIVER,
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

    const QString CONFIG_FILE_NAME = "config.json";
    const QStringList FUNCTION_STRING_LIST =
    {
        "add bootloader to firmware",
        "gen firmware for diagnosis",
        "run command"
    };
    const QStringList PLATFORM_STRING_LIST =
    {
        "m1afl2",
        "t18",
        "t19",
        "s51evfl",
        "a13tev"
    };
    const QString REPLACE_STRING = "S10BFFF8C000C000C000C000FD\n";//未加bootloader的app含有此内容
    const QString TARGET_STRING_AFTER_GENERATING_BOOTCODE = "S10BFFF8FC00FC00FC00FC000D\n";//已加bootloader的app含有此内容
    const QString DIAG_COMMON_S0  = "S02100000747395957202020202020202020202020202020200130302E30302E303040";//通用的S021行
    const QString DIAG_M1AFL2_PARTNUMBER = "701000044AA";
    const QString DIAG_T19_PARTNUMBER = "701000068AA";
    const QString DIAG_S51EVFL_PARTNUMBER = "J72-3820010FA";
    const QString DIAG_A13TEV_PARTNUMBER = "A13TEV-pn001";

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

private:
    void componentsInitialization(void);
    void layoutsInitialization(void);
    void commandsInitialization(void);
    void generateFirmwareWithBootloader();
    void generateFirmwareForDiagnosis();
    void generateFiles(CmdType cmd, QString dir_path, bool is_open_folder);
    void generateCharArray(void);
    void compressCArrayOfBitmap(void);
    bool sortS19Code(QStringList &originalStringList);
    int hexCharToHex(char src);
    unsigned short calcCRC16(QList<unsigned char> data_list);
    unsigned char calcChecksum(unsigned short crc);
    unsigned short calculateKey(unsigned short seed);
    void dealWithCalculateKeyCommand(void);
    void showHelpInfo(CmdType cmd);
    void procConfigFile(CmdType cmd);

private slots:
    void selectFilePressed();
    void useDefaultBootloaderPressed();
    void loadBootloaderPressed();
    void s021ReturnPressed();
    void switchFunctionPressed();
    void generateButtonPressed();
    void runCmdReturnPressed();
    void switchPlatformPressed();
};

#endif // MAINWINDOW_H
