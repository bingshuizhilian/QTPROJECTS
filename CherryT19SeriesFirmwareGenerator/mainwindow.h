#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>

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

private:
    QPushButton* m_btnChooseFile;
    QStringList fileInfo;
    QLineEdit* m_leFileInfo;
    QPushButton* m_btnGenerate;

private:
    void componentsInitialization(void);

private slots:
    void selectFile();
    void generateBootloader();
};

#endif // MAINWINDOW_H
