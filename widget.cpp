#include "widget.h"
#include "ui_widget.h"
#include <QScreen>
#include <QString>
#include <QPixmap>
#include <QMessageBox>
#include <QGraphicsOpacityEffect>
#include <QPalette>

Widget::Widget(int argc, char** argv, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , init_argc(argc)
    , init_argv(argv)
    , send_image(argc,argv)
    , receive_image(argc,argv)
{
    ui->setupUi(this);
    QList <QScreen *> list_screen =  QGuiApplication::screens();
//    /* 重设大小 */
    // 设置主窗口的样式表，设置背景为图片
//      this->setStyleSheet("background-image: url(D:\\V2X_UI\\res\\bg.png);");

//    this->resize(list_screen.at(0)->geometry().width(),
//                 list_screen.at(0)->geometry().height());
    //设置窗口名字
    this->setWindowTitle("V2X_UI");
//    //设置背景
//    QPalette pal = this->palette();  //获得widget的调色板
//    pal.setBrush(QPalette::Background, QBrush(QPixmap("D:\\V2X_UI\\res\\bg.png")));
//    setPalette(pal);
//    QPixmap bg("D:\\V2X_UI\\res\\bg.png");
//    QPixmap logo("D:\\V2X_UI\\res\\logo.png");

//    background = new QLabel(this);
//    background->setGeometry(0, 0, list_screen.at(0)->geometry().width(),
//                           list_screen.at(0)->geometry().height());

//    bg.scaled(background->size());
//    logo.scaled(ui->label_12->size());

//    ui->label_12->setPixmap(logo);
//    background->setPixmap(bg);
//    background->setWindowOpacity(0.5);
    //设置背景图片的透明度
    QGraphicsOpacityEffect *opacity = new QGraphicsOpacityEffect;
    opacity->setOpacity(0.8);
    ui->widget_4->setGraphicsEffect(opacity);
    ui->widget_4->raise();

    // 设置默认值
    ui->spinBox->setValue(30301);
    ui->spinBox_2->setValue(30300);
    ui->lineEdit->setText("192.168.62.199");
    ui->lineEdit_3->setText("2048");

    /* udp套接字 */
    udpSocket = new QUdpSocket(this);

    /* 设置端口号的范围，注意不要与主机的已使用的端口号冲突 */
    ui->spinBox->setRange(1000, 99999);
    ui->spinBox_2->setRange(1000, 99999);

    /* 设置停止监听状态不可用 */
    ui->pushButton_2->setEnabled(false);

    /* 设置输入框默认的文本 */
    ui->lineEdit->setText("hello V2X!!!");
    ui->lineEdit_2->setText("192.168.1.210");

    /* 获取本地ip */
    getLocalHostIP();

    /* 信号槽连接 */
    connect(ui->pushButton, SIGNAL(clicked()),this, SLOT(bindPort()));
    connect(ui->pushButton_2, SIGNAL(clicked()),this, SLOT(unbindPort()));
    connect(ui->pushButton_3, SIGNAL(clicked()),this, SLOT(clearTextBrowser()));
    connect(ui->pushButton_4, SIGNAL(clicked()),this, SLOT(sendMessages()));
    connect(ui->pushButton_5, SIGNAL(clicked()),this, SLOT(sendBroadcastMessages()));
    connect(ui->pushButton_11, SIGNAL(clicked()),this, SLOT(sendTargetAddrPort()));

    connect(ui->pushButton_12, SIGNAL(clicked()),this, SLOT(sendHostAddrPort()));

    connect(udpSocket, SIGNAL(readyRead()),this, SLOT(receiveMessages()));
    connect(udpSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(socketStateChange(QAbstractSocket::SocketState)));

    // connect(&object_detect, SIGNAL(sendRGBYOLOV5(const QImage&)), this, SLOT(showRGBCamera(const QImage &)));
    // connect(&my_camera, SIGNAL(sendRGBCamera(const QImage&)), this, SLOT(showDeepCamera(const QImage &)));
    // connect(&my_camera, SIGNAL(sendDeepCamera(const QImage&)), this, SLOT(showDeepCamera(const QImage &)));

    //发送目标主机的IP地址和端口号到发送端
    connect(this, SIGNAL(targetHostInfo(QHostAddress, quint16)), &send_image, SLOT(receiveTargetHostAddrPort(QHostAddress, quint16)));
    connect(&send_image, SIGNAL(sendYolov5ImageToWidget(const QImage &)), this, SLOT(showSendCamera(const QImage &)));

    //发送本机将要用作接收的信息的IP地址和端口号给接收端
    connect(this, SIGNAL(localHostInfo(quint16)), &receive_image, SLOT(receiveLocalPort(quint16)));
    connect(&receive_image, SIGNAL(sendReceiveImageToWidget(const QImage &)), this, SLOT(showReceiveCamera(const QImage &)));

    //发送包大小到发送和接收端
    connect(this, SIGNAL(sendPacketUnit(int)), &send_image, SLOT(receivePacketUnit(int)));
    connect(this, SIGNAL(sendPacketUnit(int)), &receive_image, SLOT(receivePacketUnit(int)));

//    void receiveRemoteAlarm(int sel, QString remote_name);          //接收来自其他小车的紧急情况
     connect(this, SIGNAL(sendAlarmSignal(bool)), &send_image, SLOT(receiveAlarmSignal(bool)));
     connect(&receive_image, SIGNAL(sendRemoteCarAlarm(int, QString, int)), this, SLOT(receiveRemoteAlarm(int, QString, int)));
     connect(&receive_image, SIGNAL(sendRemoteCarSpeed(int, QString, int)), this, SLOT(receiveRemoteSpeed(int, QString, int)));
//    sendAlarmSignal

}

Widget::~Widget()
{
    delete udpSocket;
    delete ui;
}

void Widget::getLocalHostIP()
{
    /* 获取所有的网络接口，
     * QNetworkInterface类提供主机的IP地址和网络接口的列表 */
    QList<QNetworkInterface> list
            = QNetworkInterface::allInterfaces();

    /* 遍历list */
    foreach (QNetworkInterface interface, list) {

        /* QNetworkAddressEntry类存储IP地址子网掩码和广播地址 */
        QList<QNetworkAddressEntry> entryList
                = interface.addressEntries();

        /* 遍历entryList */
        foreach (QNetworkAddressEntry entry, entryList) {
            /* 过滤IPv6地址，只留下IPv4 */
            if (entry.ip().protocol() ==
                    QAbstractSocket::IPv4Protocol) {
                ui->comboBox->addItem(entry.ip().toString());
                /* 添加到IP列表中 */
                IPlist<<entry.ip();
            }
        }
    }
}

void Widget::bindPort()
{
    quint16 port = ui->spinBox->value();

    /* 绑定端口需要在socket的状态为UnconnectedState */
    if (udpSocket->state() != QAbstractSocket::UnconnectedState)
        udpSocket->close();

    if (udpSocket->bind(port)) {
        ui->textBrowser->append("已经成功绑定端口："
                            + QString::number(port));

        /* 设置界面中的元素的可用状态 */
        ui->pushButton->setEnabled(false);
        ui->pushButton_2->setEnabled(true);
        ui->spinBox->setEnabled(false);
    }
}

void Widget::unbindPort()
{
    /* 解绑，不再监听 */
    udpSocket->abort();

    /* 设置界面中的元素的可用状态 */
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(false);
    ui->spinBox->setEnabled(true);
}

void Widget::clearTextBrowser()
{
    /* 清除文本浏览器的内容 */
    ui->textBrowser->clear();
}

void Widget::receiveMessages()
{
    /* 局部变量，用于获取发送者的IP和端口 */
    QHostAddress peerAddr;
    quint16 peerPort;

    /* 如果有数据已经准备好 */
    while (udpSocket->hasPendingDatagrams()) {
       /* udpSocket发送的数据报是QByteArray类型的字节数组 */
       QByteArray datagram;

       /* 重新定义数组的大小 */
       datagram.resize(udpSocket->pendingDatagramSize());

       /* 读取数据，并获取发送方的IP地址和端口 */
       udpSocket->readDatagram(datagram.data(),
                               datagram.size(),
                               &peerAddr,
                               &peerPort);
       /* 转为字符串 */
       QString str = datagram.data();

       /* 显示信息到文本浏览框窗口 */
       ui->textBrowser->append("接收来自"
                           + peerAddr.toString()
                           + ":"
                           + QString::number(peerPort)
                           + str);
    }
}

void Widget::sendMessages()
{
    /* 文本浏览框显示发送的信息 */
     ui->textBrowser->append("发送：" + ui->lineEdit->text());

     /* 要发送的信息，转为QByteArray类型字节数组，数据一般少于512个字节 */
     QByteArray data = ui->lineEdit->text().toUtf8();

     /* 要发送的目标Ip地址 */
//     QHostAddress peerAddr = IPlist[ui->comboBox->currentIndex()];
     QString targetIP =  ui->lineEdit_2->text();
     QHostAddress    targetAddr(targetIP);
     /* 要发送的目标端口号 */
     quint16 peerPort = ui->spinBox_2->value();

     /* 发送消息 */
     udpSocket->writeDatagram(data, targetAddr, peerPort);
}

void Widget::sendBroadcastMessages()
{
    /* 文本浏览框显示发送的信息 */
    ui->textBrowser->append("发送：" + ui->lineEdit->text());

    /* 要发送的信息，转为QByteArray类型字节数组，数据一般少于512个字节 */
    QByteArray data = ui->lineEdit->text().toUtf8();

    /* 广播地址，一般为255.255.255.255，
     * 同一网段内监听目标端口的程序都会接收到消息 */
    QHostAddress peerAddr = QHostAddress::Broadcast;

    /* 要发送的目标端口号 */
    quint16 peerPort = ui->spinBox_2->text().toInt();

    /* 发送消息 */
    udpSocket->writeDatagram(data, peerAddr, peerPort);

    // if ( !my_camera.init())
    // {
    //     QMessageBox::warning(nullptr, "失败", "接收图片失败！请检查你的网络！", QMessageBox::Yes, QMessageBox::Yes);
    //     this->close();
    // }
}

void Widget::socketStateChange(QAbstractSocket::SocketState state)
{
switch (state) {
   case QAbstractSocket::UnconnectedState:
       ui->textBrowser->append("scoket状态：UnconnectedState");
       break;
   case QAbstractSocket::ConnectedState:
       ui->textBrowser->append("scoket状态：ConnectedState");
       break;
   case QAbstractSocket::ConnectingState:
       ui->textBrowser->append("scoket状态：ConnectingState");
       break;
   case QAbstractSocket::HostLookupState:
       ui->textBrowser->append("scoket状态：HostLookupState");
       break;
   case QAbstractSocket::ClosingState:
       ui->textBrowser->append("scoket状态：ClosingState");
       break;
   case QAbstractSocket::ListeningState:
       ui->textBrowser->append("scoket状态：ListeningState");
       break;
   case QAbstractSocket::BoundState:
       ui->textBrowser->append("scoket状态：BoundState");
       break;
   default:
       break;
   }
}

//显示发送端的画面
void Widget::showSendCamera(const QImage &image)
{
    QPixmap pix = QPixmap::fromImage(image).scaled(ui->label_7->size());
    ui->label_7->setPixmap(pix);
}

//显示接收端的画面
void Widget::showReceiveCamera(const QImage &image)
{
    //  显示接收到的图片数据
    QPixmap pix = QPixmap::fromImage(image).scaled(ui->label_9->size());
    ui->label_9->setPixmap(pix);
}

//将目标主机的IP地址和端口号发送出去
void Widget::sendTargetAddrPort()
{
    emit targetHostInfo((QHostAddress)(ui->lineEdit_2->text()), ui->spinBox_2->value());
    emit sendPacketUnit((ui->lineEdit_3->text().toInt()));   //发送每次包的大小到发送端
    if (!send_image.init())
    {
        QMessageBox::warning(nullptr, "失败", "发送图片失败！请检查你的网络", QMessageBox::Yes, QMessageBox::Yes);
        this->close();
    }
    connect(send_image.udpSocket_recv,SIGNAL(readyRead()),&send_image,SLOT(receiveYolov5Image()));

    // sendFlag = 1;
}

void Widget::sendHostAddrPort()
{
    emit localHostInfo(ui->spinBox->value());
    emit sendPacketUnit((ui->lineEdit_3->text().toInt()));   //发送每次包的大小到接收端
    if ( !receive_image.init())
    {
        QMessageBox::warning(nullptr, "失败", "接收图片失败！请检查你的网络！", QMessageBox::Yes, QMessageBox::Yes);
        this->close();
    }
    connect(receive_image.receiver,SIGNAL(readyRead()),&receive_image,SLOT(receiveVideo()));
    // system("conda activate V2X");
    // system("python2.7 /home/agilex/V2V_UI_ROS/src/ui/scripts/receive_test.py");
    // if ( !object_detect.init())
    // {
    //     QMessageBox::warning(nullptr, "失败", "发送图片失败！请检查你的网络！", QMessageBox::Yes, QMessageBox::Yes);
    //     this->close();
    // }
}

void Widget::on_pushButton_6_clicked()
{
    emit targetHostInfo((QHostAddress)(ui->lineEdit_2->text()), ui->spinBox_2->value());
    if (!send_image.init())
    {
        QMessageBox::warning(nullptr, "失败", "发送图片失败！请检查你的网络", QMessageBox::Yes, QMessageBox::Yes);
        this->close();
    }
    emit sendAlarmSignal(true);
}

void Widget::on_pushButton_7_clicked()
{
    emit targetHostInfo((QHostAddress)(ui->lineEdit_2->text()), ui->spinBox_2->value());
    if (!send_image.init())
    {
        QMessageBox::warning(nullptr, "失败", "发送图片失败！请检查你的网络", QMessageBox::Yes, QMessageBox::Yes);
        this->close();
    }
    emit sendAlarmSignal(false);
}

void Widget::receiveRemoteAlarm(int sel, QString remote_name, int kind)         //接收来自其他小车的紧急情况
{
    if(sel == 1)
    {
       ui->label_13->setText(remote_name+":请提前避让!");
        // 设置标签的背景颜色
       ui->label_13->setStyleSheet("background-color: red;");

    }
    else
    {
       ui->label_13->setText("Warning");
        // 设置标签的背景颜色
       ui->label_13->setStyleSheet("background-color: green;");
    }
}

void Widget::receiveRemoteSpeed(int speed, QString remote_name, int kind) //接收另外一台车的实时速度
{
    /* 文本浏览框显示发送的信息 */
    ui->textBrowser_2->append(remote_name+"的速度：" +QString::number(speed) + "mm/s");
}

