#ifndef SENDIMAGE_H
#define SENDIMAGE_H

#include <QThread>
#include <QImage>
#include <QDebug>
#include <QUdpSocket>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QBuffer>
#include<QImageReader>
#include <QFile>
#include <QTextStream>
#include <QDir>

class SendImage : public QThread
{
  Q_OBJECT
public:
    /* Udp通信套接字 */
    QUdpSocket *udpSocket_send;
    QUdpSocket *udpSocket_recv;

    SendImage(int argc, char** argv, QWidget *parent = nullptr);

    ~SendImage();

    bool init();

    void disinit();

    void run() override;

private:
    int init_argc;
    char** init_argv;
    int packet_unit;
    int start_flag;
    QHostAddress dstip;
    quint16 dstport;
    QString local_name;
    void sendYolov5ImageToTargetHost(const QByteArray recvData);
    void sendYolov5ImageToTargetHost(const QImage &image);
    void sendYolov5ImageToTargetHost(const QImage &image, int packet_unit);   //分包传输

signals:
    void sendYolov5ImageToWidget(const QImage &image);

private slots:
    void receiveTargetHostAddrPort(QHostAddress dstip, quint16 dstport);          //接收目标主机的IP地址和端口号

    void receiveYolov5Image();

    void receivePacketUnit(int size);               //每次分发包的大小

    void receiveAlarmSignal(bool sel);    //发送报警信号到目标小车
};

#endif // SendImage_H
