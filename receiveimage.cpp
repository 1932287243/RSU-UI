#include "receiveImage.h"
#include<QHostAddress>
#include<QPixmap>
#include<QImageReader>
#include<QBuffer>


ReceiveImage::ReceiveImage(int argc, char** argv, QWidget *parent) :
   init_argc(argc),
   init_argv(argv)
{
    // connect(receiver,SIGNAL(readyRead()),this,SLOT(video_receive()));
    start_flag = 1;
}

ReceiveImage::~ReceiveImage()
{
    delete receiver;
}

bool ReceiveImage::init()
{
    if (start_flag)
    {
        receiver = new QUdpSocket(this);
        receiver->bind(dstport);
        qDebug() << "port:" << dstport << endl;
        start_flag = 0;
    }
    return true;
}

void ReceiveImage::disinit()
{
    /* 解绑，不再监听 */
    receiver->abort();
    this->exit();
}

void ReceiveImage::run()
{
    //对图像进行二次处理再次实现
}

void ReceiveImage::receiveVideo()
{
    // quint64 size =  1843200;
    // QByteArray buff;
    // buff.resize(size);
    // QHostAddress adrr ;
    // quint16 port;
    // receiver->readDatagram(buff.data(),buff.size(),&adrr,&port);
    // // buff = qUncompress(buff);
    // QBuffer buffer(&buff);
    // qDebug() << "接收的图片大小" << buff.size() << endl;
    // QImageReader reader(&buffer,"JPEG");//可读入磁盘文件、设备文件中的图像、以及其他图像数据如pixmap和image，相比较更加专业。
    // //buffer属于设备文件一类，
    // QImage image = reader.read();//read()方法用来读取设备图像，也可读取视频，读取成功返回QImage*，否则返回NULL
#if 0
    quint64 size = receiver->pendingDatagramSize();
    QByteArray buff;
    buff.resize(size);
    QHostAddress adrr ;
    quint16 port;
    receiver->readDatagram(buff.data(),buff.size(),&adrr,&port);
    buff = qUncompress(buff);
    QBuffer buffer(&buff);
    QImageReader reader(&buffer,"JPEG");//可读入磁盘文件、设备文件中的图像、以及其他图像数据如pixmap和image，相比较更加专业。
    //buffer属于设备文件一类，
    QImage image = reader.read();//read()方法用来读取设备图像，也可读取视频，读取成功返回QImage*，否则返回NULL
    emit sendReceiveImageToWidget(image);
#elif 1
    while (receiver->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(receiver->pendingDatagramSize());
        QHostAddress sender_address;
        quint16 sender_port;

        // 读取收到的数据包
        receiver->readDatagram(datagram.data(), datagram.size(), &sender_address, &sender_port);

        // 解析数据包
        qint32 data_length, packet_index,data_type;
        QByteArray image_data;
        QString remote_name;
        QDataStream stream(datagram);

        stream >> data_type >> data_length >> packet_index >> remote_name;

        if(data_type == 1)
        {
            qDebug() << "data_length=" << data_length << endl;
            qDebug() << "packet_index=" << packet_index << endl;
//            qDebug() << "remote_name=" << remote_name << endl;
            image_data = datagram.mid(sizeof(data_length) + sizeof(packet_index));
            // 处理数据包
            //向上去整，防止少发
            qint32 packet_count = (data_length + packet_unit - 1) / packet_unit;
            // 将图像数据组合起来
            received_data.append(image_data);

            // 如果已经接收到所有数据包，则进行图像重组
            if (packet_index == packet_count)
            {
            // 检查接收到的数据长度是否与期望的一致
                if (received_data.size() == static_cast<int>(data_length))
                {
                    qDebug() << "received_data=" << received_data.size() <<  "data_length=" << data_length << endl;
                    // 将 QByteArray 转为 QImage
                    QImage reassembled_image;
                    reassembled_image.loadFromData(received_data);
//                    qDebug() << "remote_name" << remote_name << endl;
                    emit sendReceiveImageToWidget(reassembled_image);
                    // 在这里可以使用 reassembled_image 进行进一步的处理或者显示
                    qDebug() << "Image reassembled successfully.";
                }
                else
                {
                    qDebug() << "Error: Incomplete image data received.";
                }

                // 清空接收缓存
                received_data.clear();
            }
        }
        else if(data_type == 2)   //接收到另一台小车发来的速度
        {
//             stream >> remote_name;
            // qint32 receive_speed = data_length;   //获取到另一台小车发来的速度
            // qDebug() << "receive_speed " << receive_speed << endl;
            qDebug() << "data_type" << data_type << endl;
            qDebug() << "data_length" << data_length << endl;
            qDebug() << "remote_name" << remote_name << endl;

            emit sendRemoteCarSpeed(data_length, remote_name, packet_index);
        }
        else if(data_type == 3)   //接收到另一台小车发来的紧急情况信号
        {
//            stream >> remote_name;
            qDebug() << "data_type" << data_type << endl;
            qDebug() << "data_length" << data_length << endl;
            qDebug() << "data_length" << remote_name << endl;

            emit sendRemoteCarAlarm(data_length, remote_name, data_length);
        }
    }

#endif


}

 //接收本机的IP地址和端口号
void ReceiveImage::receiveLocalPort(quint16 dstport)
{
    this->dstport = dstport;
    qDebug() << "LocalPort" << dstport << endl;
}

void ReceiveImage::receivePacketUnit(int size)              //每次分发包的大小
{
    this->packet_unit = size;
    qDebug() << "packet_unit=" <<packet_unit << endl;
}
