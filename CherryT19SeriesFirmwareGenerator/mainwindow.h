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

    enum CmdType
    {
        CMD_HELP,
        CMD_HELP_BOOTLOADER,
        CMD_HELP_DAIGNOSIS,
        CMD_CLEAR_SCREEN,
        CMD_FULL_SCREEN,
        CMD_NORMAL_SCREEN,
        CMD_SAVE_CONFIG_FILE,
        CMD_LOAD_CONFIG_FILE,
        CMD_CODE_TO_STRING,
        CMD_GEN_FLASH_DRIVER,
        CMD_CRC_CALCULATOR,
        CMD_DIAG_M1A_S021,
        CMD_DIAG_M1A_S021_AUTOFILL,
        CMD_DIAG_T19_S021,
        CMD_DIAG_T19_S021_AUTOFILL,
#if WIN32
        CMD_WINDOWS_CALCULATOR,
        CMD_WINDOWS_SNIPPINGTOOL,
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
    const QString REPLACE_STRING = "S10BFFF8C000C000C000C000FD\n";//未加bootloader的app含有此内容
    const QString TARGET_STRING_AFTER_GENERATING_BOOTCODE = "S10BFFF8FC00FC00FC00FC000D\n";//已加bootloader的app含有此内容
    const int DIAG_M1A_S021_MIN_LENGTH = 22;
    const QString DIAG_M1A_S021 = "S02100000747395957373031303030303434414120202020200130302E30302E303040";//M1A、M1D的S0行
    const QString DIAG_T19_S021 = "to be filled";//T19、T18的S0行

private:
    QStringList fileInfo;
    QList<QPair<CmdType, QStringList>> cmdList;

private:
    QPushButton* m_btnChooseFile;
    QLineEdit* m_leFileInfo;
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
    void generateFlashDriverForDiagnosis(QString dir_path, bool is_open_folder);
    void generateCharArray(void);
    bool sortS19Code(QStringList &originalStringList);
    int hexCharToHex(char src);
    unsigned int calcCRC(unsigned int size, QString fileData);
    void crcTest();
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
};

#endif // MAINWINDOW_H
