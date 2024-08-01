#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>

//#define DEFAULTSERIAL
#define RADARSERIAL

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnPortsInfo_clicked();
    void on_btnOpenPort_clicked();
    void on_btnSendDatas_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *_serialPort;
    QByteArray data;

    void loadPorts();
    void readData();
    bool CRC_check(const QByteArray& message);
    void GetCRC(QByteArray& message);

    #ifdef RADARSERIAL
    uint8_t deviceMajorVersion;
    uint8_t deviceMinorVersion;
    uint16_t servoPosition;
    uint32_t Distance;
    void parseDataForRadarApp();
    #endif


};
#endif // MAINWINDOW_H
