#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QDebug>
#include <QLabel>

#include "sendImage.h"
#include "receiveImage.h"

class SendImage;
class ReceiveImage;

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(int argc, char** argv, QWidget *parent = nullptr);
    ~Widget();

private:
    int init_argc;
    char** init_argv;

    SendImage send_image;
    ReceiveImage receive_image;
    QLabel *background;

    Ui::Widget *ui;
    /* Udp通信套接字 */
    QUdpSocket *udpSocket;
    /* 存储本地的 ip 列表地址 */
    QList<QHostAddress> IPlist;
    /* 获取本地的所有 ip */
    void getLocalHostIP();
signals:
    void targetHostInfo(QHostAddress dstip, quint16 dstport);
    void localHostInfo(quint16 dstport);
    void sendImage(const QImage &image);        //发送处理后的RGB相机的数据
    void sendPacketUnit(int size);               //每次分发包的大小
    void sendAlarmSignal(bool sel);          //发送是否开启报警信号

private slots:
    /* 绑定端口 */
    void bindPort();

    /* 解绑端口 */
    void unbindPort();
    /* 清除文本框时的内容 */
    void clearTextBrowser();

    /* 接收到消息 */
    void receiveMessages();

    /* 发送本机的IP地址和端口号 */
    void sendHostAddrPort();

     /* 发送目标主机的IP地址和端口号 */
    void sendTargetAddrPort();

    /* 发送消息 */
    void sendMessages();

    /* 广播消息 */
    void sendBroadcastMessages();

    /* 连接状态改变槽函数 */
    void socketStateChange(QAbstractSocket::SocketState state);

    // void receiveFromDstImage(const QImage &image);  //接收来自其他电脑发送来的图片

    void showReceiveCamera(const QImage &image);

    void showSendCamera(const QImage &image);
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void receiveRemoteSpeed(int speed, QString remote_name, int kind); //接收另外一台车的实时速度
    void receiveRemoteAlarm(int sel, QString remote_name, int kind);          //接收来自其他小车的紧急情况
};
#endif // WIDGET_H
