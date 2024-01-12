#include "sendImage.h"

SendImage::SendImage(int argc, char** argv, QWidget *parent):
    init_argc(argc),
    init_argv(argv)
{
    start_flag = 1;
}

SendImage::~SendImage()
{
    delete udpSocket_send;
}

void SendImage::run()
{
    //对图像进行二次处理再次实现
}

bool SendImage::init()
{
    if(start_flag)
    {
        udpSocket_send = new QUdpSocket(this);    // 发送目标主机的UDP
        udpSocket_recv = new QUdpSocket(this);    // 接收YOLOV5处理后的视频数据
        udpSocket_recv->bind(8081);               // 监听本机的8081端口
        // 获取当前工作目录
        QString currentPath = QDir::currentPath();
//        qDebug() << "currentPath" << currentPath << endl;
        // 拼接相对路径和当前工作目录
        QString absoluteFilePath = currentPath + "/" + "rsuname.txt";
        // 创建文件对象
        QFile file(absoluteFilePath);
        // 打开文件
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
           // 使用文本流读取文件
           QTextStream in(&file);

           // 读取文件内容
           while (!in.atEnd())
           {
               local_name = in.readLine();
//               qDebug() << local_name << endl;
           }
           // 关闭文件
           file.close();
        }
        else
        {
           // 文件打开失败
           qDebug() << "Failed to open file";
        }
        start_flag = 0;   //防止重新初始化
    }
    return true;
}

void SendImage::disinit()
{
    this->exit();
}

// 获取将要发送到的目标主机的IP地址和端口号
void SendImage::receiveTargetHostAddrPort(QHostAddress dstip, quint16 dstport)         //接收目标主机的IP地址和端口号
{
    this->dstip = dstip;
    this->dstport = dstport;
    qDebug() << "TargetIP" << dstip << endl;
}

// 发送经过处理的视频流到目标主机
void SendImage::sendYolov5ImageToTargetHost(const QByteArray recvData)
{
    quint64 size =  1843200;
    QByteArray buff;
    buff.resize(size);
    buff = recvData;
    // QBuffer buff(&byte);
    // buff.open(QIODevice::WriteOnly);
    // image.save(&buff,"JPEG");
    // QByteArray ss = qCompress(byte,0);  //不压缩
    udpSocket_send->writeDatagram(buff.data(),dstip,dstport);
     qDebug() << "发送的图片大小" << buff.size() << endl;
}

void SendImage::sendYolov5ImageToTargetHost(const QImage &image)
{
    QByteArray byte;
    QBuffer buff(&byte);
    buff.open(QIODevice::WriteOnly);
    image.save(&buff,"JPEG");
    QByteArray ss = qCompress(byte,5);
    udpSocket_send->writeDatagram(ss,dstip,dstport);
}

void SendImage::sendYolov5ImageToTargetHost(const QImage &image, int packet_unit)   //分包传输
{
    // 将图像编码为 JPEG 格式
    QByteArray encoded_image;
    QBuffer buffer(&encoded_image);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPEG", 50);

    /*1代表是图片数据、2代表是速度*/
    qint32 data_type = 1;         //发送的数据类别

    QByteArray image_data;      //图片的数据
    qint32 data_length;         //数据的长度

    // 获取图像数据
    data_length = encoded_image.size();
    image_data = encoded_image;

    qint32 current_packet_index = 0;

    //向上去整，防止少发
    qint32 packet_count = (data_length + packet_unit - 1) / packet_unit;

//     qDebug() << "data_length=" << data_length << endl;
//     qDebug() << "packet_count=" << packet_count << endl;

    for(int i = 0; i < packet_count; i++)
    {
        // 判断是否为最后一个数据包
        bool is_last_packet = (current_packet_index == packet_count - 1);

        // 构建数据包
        QByteArray packet = image_data.mid(current_packet_index * packet_unit, packet_unit);
        ++current_packet_index;

        // 添加包头信息
        QByteArray send_data;
        QDataStream stream(&send_data, QIODevice::WriteOnly);
        stream << qint32(data_type) << qint32(data_length) << qint32(current_packet_index);
        send_data.append(packet);

        // 发送数据包
        udpSocket_send->writeDatagram(send_data,dstip,dstport);

        if (is_last_packet) {
            // qDebug() << "Image sent successfully.";
        }
    }
}

//接收经过yolov5处理后的视频流
void SendImage::receiveYolov5Image()
{
    quint64 size =  1843200;
    QByteArray buff;
    buff.resize(size);
    QHostAddress adrr ;
    quint16 port;
    udpSocket_recv->readDatagram(buff.data(),buff.size(),&adrr,&port);
    // buff = qUncompress(buff);
    QBuffer buffer(&buff);
    // qDebug() << "接收的图片大小" << buff.size() << endl;
    QImageReader reader(&buffer,"JPEG");//可读入磁盘文件、设备文件中的图像、以及其他图像数据如pixmap和image，相比较更加专业。
    //buffer属于设备文件一类，
    QImage image = reader.read();//read()方法用来读取设备图像，也可读取视频，读取成功返回QImage*，否则返回NULL
    // sendYolov5ImageToTargetHost(buff);     //发送给目标主机
    sendYolov5ImageToTargetHost(image,packet_unit);     //发送给目标主机
    emit sendYolov5ImageToWidget(image);    //发送到窗口显示
}

void SendImage::receivePacketUnit(int size)              //每次分发包的大小
{
    this->packet_unit = size;
    qDebug() << "packet_unit=" <<packet_unit << endl;
}


void SendImage::receiveAlarmSignal(bool sel)    //发送报警信号到目标小车
{
//    qDebug() << __FUNCTION__ << __LINE__ << endl;
    /*1代表是图片数据、2代表是速度、3代表是紧急情况*/
    qint32 data_type = 3;           //发送的数据类别
    qint32 device_kind = 1;         // 0代表OBU警报，1代表RSU警报
    QByteArray send_data ;
    QDataStream stream(&send_data, QIODevice::WriteOnly);
    if(sel)
    {
        // 添加包头信息
        int ctrl_sig = 1;
        stream << qint32(data_type) << ctrl_sig << device_kind << local_name;
        /* 发送小车速度 */
        udpSocket_send->writeDatagram(send_data,dstip,dstport);
    }
    else
    {
        int ctrl_sig = 0;
        stream << qint32(data_type) << ctrl_sig << ctrl_sig << local_name;
        /* 发送小车速度 */
        udpSocket_send->writeDatagram(send_data,dstip,dstport);
    }
    send_data.clear();
}
