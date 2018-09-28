#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>

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

    const QString REPLACE_STRING = "S10BFFF8C000C000C000C000FD\n";//未加bootloader的app含有此内容
    const QString TARGET_STRING_AFTER_GENERATING = "S10BFFF8FC00FC00FC00FC000D\n";//已加bootloader的app含有此内容

private:
    QPushButton* m_btnChooseFile;
    QStringList fileInfo;
    QLineEdit* m_leFileInfo;
    QCheckBox* m_cbUseDefaultBootloader;
    QPushButton* m_btnLoadBootloader;
    QLineEdit* m_leBootloaderInfo;
    QPushButton* m_btnGenerate;

private:
    void componentsInitialization(void);

private slots:
    void selectFile();
    void useDefaultBootloader();
    void loadBootloader();
    void generateFirmwareWithBootloader();
};

#endif // MAINWINDOW_H
