#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _serialPort(nullptr)
{
    ui->setupUi(this);

    loadPorts();

    ui->cmbBaudrates->setCurrentText("115200");
    ui->cmbParitys->setCurrentText("None");
    ui->cmbDataBits->setCurrentText("8");
    ui->cmbStopBits->setCurrentText("1");
    ui->cmbParitysError->setCurrentText("63 ('?')");

#ifdef RADARSERIAL
    servoPosition = 0;
    Distance = 0;
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

// For Reflesh Ports:
void MainWindow::on_btnPortsInfo_clicked()
{
    ui->cmbPorts->clear();
    foreach (auto &port, QSerialPortInfo::availablePorts()) {
        ui->cmbPorts->addItem(port.portName());
    }
}

void MainWindow::loadPorts()
{
    foreach (auto &port, QSerialPortInfo::availablePorts()) {
        ui->cmbPorts->addItem(port.portName());
    }
}

void MainWindow::on_btnOpenPort_clicked()
{
    if (_serialPort != nullptr) {
        _serialPort->close();
        delete _serialPort;
    }

    _serialPort = new QSerialPort(this);
    _serialPort->setPortName(ui->cmbPorts->currentText());
    //_serialPort->setBaudRate(QSerialPort::Baud115200);
    //_serialPort->setDataBits(QSerialPort::Data8);
    //_serialPort->setParity(QSerialPort::NoParity);
    //_serialPort->setStopBits(QSerialPort::OneStop);

    // Set Baud Rate Value:
    _serialPort->setBaudRate(ui->cmbBaudrates->currentText().toInt());
    // Set Data Bit Value:
    if(ui->cmbDataBits->currentText().toInt() == QSerialPort::Data5)
        _serialPort->setDataBits(QSerialPort::Data5);
    else if (ui->cmbDataBits->currentText().toInt() == QSerialPort::Data6)
        _serialPort->setDataBits(QSerialPort::Data6);
    else if (ui->cmbDataBits->currentText().toInt() == QSerialPort::Data7)
        _serialPort->setDataBits(QSerialPort::Data7);
    else
        _serialPort->setDataBits(QSerialPort::Data8);
    // Set Parity Value:
    if(ui->cmbParitys->currentIndex() == 0)
        _serialPort->setParity(QSerialPort::NoParity);
    else if(ui->cmbParitys->currentIndex() == 1)
        _serialPort->setParity(QSerialPort::EvenParity);
    else if(ui->cmbParitys->currentIndex() == 2)
        _serialPort->setParity(QSerialPort::OddParity);
    else if(ui->cmbParitys->currentIndex() == 3)
        _serialPort->setParity(QSerialPort::SpaceParity);
    else if(ui->cmbParitys->currentIndex() == 4)
        _serialPort->setParity(QSerialPort::MarkParity);
    // Set Stop Bit Value:
    if(ui->cmbStopBits->currentIndex() == 0)
        _serialPort->setStopBits(QSerialPort::OneStop);
    else if(ui->cmbStopBits->currentIndex() == 1)
        _serialPort->setStopBits(QSerialPort::OneAndHalfStop);
    else if(ui->cmbStopBits->currentIndex() == 2)
        _serialPort->setStopBits(QSerialPort::TwoStop);


    if (_serialPort->open(QIODevice::ReadWrite)) {
        QMessageBox::information(this, "Result", "Port opened successfully...");
        connect(_serialPort, &QSerialPort::readyRead, this, &MainWindow::readData);
    }
    else {
        QMessageBox::critical(this, "Port Error", "Unable to open specified port!!!");
    }

}


void MainWindow::on_btnSendDatas_clicked()
{
    if(!_serialPort->isOpen()) {
        QMessageBox::critical(this, "Port Error", "Port is not opened!!!");
        return;
    }
    _serialPort->write(ui->lnTransmitTxt->text().toUtf8());
}

void MainWindow::readData()
{
    if(!_serialPort->isOpen()) {
        QMessageBox::critical(this, "Port Error", "Port is not opened!!!");
        return;
    }
    //auto data = _serialPort->readAll();
    data = _serialPort->read(128);

    #ifdef DEFAULTSERIAL
        QString hexData = data.toHex();
        ui->txtReceive->clear();
        ui->txtReceive->append(hexData);
        data.clear();
    #endif

    #ifdef RADARSERIAL
        // For Radar App:
        parseDataForRadarApp();
    #endif
}

#ifdef RADARSERIAL
    void MainWindow::parseDataForRadarApp()
    {
        if(data.size() > 0 && CRC_check(data)) {
            if((static_cast<unsigned char>(data[0]) == 0xFA) && (static_cast<unsigned char>(data[1]) == 0xFB) && (static_cast<unsigned char>(data[124]) == 0xFB) && (static_cast<unsigned char>(data[125]) == 0xFA)) {
                deviceMajorVersion = static_cast<uint8_t>(data[2]);
                deviceMinorVersion = static_cast<uint8_t>(data[3]);
                servoPosition = ((static_cast<uint8_t>(data[8])) << 8) | (static_cast<uint8_t>(data[9]));
                Distance = ((static_cast<uint8_t>(data[10])) << 24) | ((static_cast<uint8_t>(data[11])) << 16) | ((static_cast<uint8_t>(data[12])) << 8) | (static_cast<uint8_t>(data[13]));

                ui->txtReceive->clear();
                ui->txtReceive->append("Device Major Version: " + QString::number(deviceMajorVersion));
                ui->txtReceive->append("Device Minor Version: " + QString::number(deviceMinorVersion));
                ui->txtReceive->append("Angle: " + QString::number(servoPosition));
                ui->txtReceive->append("Range: " + QString::number(Distance));
            }
        }

        data.clear();
    }
#endif

bool MainWindow::CRC_check(const QByteArray& message)
{
    unsigned int CRCFull = 0xFFFF;
    unsigned int CRCLSB;

    // Verinin uzunluğunu hesapla (Son 2 byte CRC olduğu için -2 yapıyoruz)
    int message_length = message.size() - 2;

    // CRC hesaplama döngüsü
    for (int i = 0; i < message_length; i++) {
        CRCFull = static_cast<unsigned int>(CRCFull ^ static_cast<unsigned char>(message[i]));

        for (int j = 0; j < 8; j++) {
            CRCLSB = static_cast<unsigned int>(CRCFull & 0x0001);
            CRCFull = static_cast<unsigned int>((CRCFull >> 1) & 0x7FFF);

            if (CRCLSB == 1) {
                CRCFull = static_cast<unsigned int>(CRCFull ^ 0xA001);
            }
        }
    }

    // CRC sonuçlarını ayır
    unsigned char CRCHigh = static_cast<unsigned char>((CRCFull >> 8) & 0xFF);
    unsigned char CRCLow  = static_cast<unsigned char>(CRCFull & 0xFF);

    // CRC'yi kontrol et (Verinin son iki byte'ı ile hesaplanan CRC'nin eşleşip eşleşmediğini kontrol ediyoruz)
    return (CRCLow == static_cast<unsigned char>(message[message_length]) &&
            CRCHigh == static_cast<unsigned char>(message[message_length + 1]));
}


void MainWindow::GetCRC(QByteArray& message)
{
    unsigned int CRCFull = 0xFFFF;
    unsigned int CRCLSB;

    // Mesajın uzunluğu (Son iki byte CRC için ayrıldığından -2 yapıyoruz)
    int message_length = message.size() - 2;

    // CRC hesaplama döngüsü
    for (int i = 0; i < message_length; i++) {
        CRCFull = static_cast<unsigned int>(CRCFull ^ static_cast<unsigned char>(message[i]));

        for (int j = 0; j < 8; j++) {
            CRCLSB = static_cast<unsigned int>(CRCFull & 0x0001);
            CRCFull = static_cast<unsigned int>((CRCFull >> 1) & 0x7FFF);

            if (CRCLSB == 1) {
                CRCFull = static_cast<unsigned int>(CRCFull ^ 0xA001);
            }
        }
    }

    // CRC sonuçlarını ayır
    unsigned char CRCHigh = static_cast<unsigned char>((CRCFull >> 8) & 0xFF);
    unsigned char CRCLow  = static_cast<unsigned char>(CRCFull & 0xFF);

    // CRC değerlerini mesaja ekle
    message[message_length] = CRCLow;
    message[message_length + 1] = CRCHigh;
}
