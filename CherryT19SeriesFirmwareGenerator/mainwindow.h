#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLayout>

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

    enum FunctinoType
    {
        BOOTLOADER,
        DIAGNOSIS,
        BOOTCODE2STRING
    };

    enum WindowSizeType
    {
        WINDOW_HEIGHT = 237,
        WINDOW_WIDTH  = 332
    };

    const QStringList FUNCTION_STRING_LIST =
    {
        "add bootloader to firmware",
        "firmware for diagnosis",
        "convert boot code to string"
    };
    const QString REPLACE_STRING = "S10BFFF8C000C000C000C000FD\n";//未加bootloader的app含有此内容
    const QString TARGET_STRING_AFTER_GENERATING_BOOTCODE = "S10BFFF8FC00FC00FC00FC000D\n";//已加bootloader的app含有此内容
private:
    QStringList fileInfo;
private:
    QPushButton* m_btnChooseFile;
    QLineEdit* m_leFileInfo;
    QCheckBox* m_ckbUseDefaultBootloader;
    QPushButton* m_btnLoadBootloader;
    QLineEdit* m_leBootloaderInfo;
    QPushButton* m_btnGenerate;
    QComboBox* m_cmbFunctionSwitch;
    QLineEdit* m_leDiagnosisS021;
    QLineEdit* m_leDiagnosisS20C;

private:
    QVBoxLayout* m_layoutGlobal;
    QGroupBox* m_gbBootloader;
    QGroupBox* m_gbS19Selector;
    QGroupBox* m_gbDiagnosis;

private:
    void componentsInitialization(void);
    void layoutsInitialization(void);
    void generateFirmwareWithBootloader();
    void generateFirmwareForDiagnosis();
    void generateFlashDriverForDiagnosis(QString dir_path);
    void generateCharArray(void);
    bool sortS19Code(QStringList &originalStringList);
    int hexCharToHex(char src);
    unsigned int calcCRC(unsigned int size, QString fileData);

private slots:
    void selectFilePressed();
    void useDefaultBootloaderPressed();
    void loadBootloaderPressed();
    void s021ReturnedPressed();
    void switchFunctionPressed();
    void generateButtonPressed();
};

#endif // MAINWINDOW_H
