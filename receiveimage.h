#ifndef RECEIVEIMAGE_H
#define RECEIVEIMAGE_H


#include <QUdpSocket>
#include <QThread>
#include <QHostAddress>

namespace Ui {
class ReceiveImage;
}

class ReceiveImage : public QThread
{
    Q_OBJECT

public:
    explicit ReceiveImage(int argc, char** argv, QWidget *parent = nullptr);
    ~ReceiveImage();
    bool init();
    void disinit();
    void run() override;
    QUdpSocket *receiver;

public slots:
    void receiveVideo();
    void receiveLocalPort( quint16 dstport);          //接收目标主机的IP地址和端口号
    void receivePacketUnit(int size);               //每次分发包的大小

signals:
    void sendReceiveImageToWidget(const QImage &image);
    void sendRemoteCarSpeed(int speed, QString remote_name, int kind);
    void sendRemoteCarAlarm(int sel, QString remote_name, int kind);

private:
    int init_argc;
    char** init_argv;
    int packet_unit;
    int start_flag;
    QByteArray received_data;   //接受到的数据
    QHostAddress dstip;
    quint16 dstport;

};
#endif // ReceiveImage_H

