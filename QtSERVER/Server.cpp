#include "Server.h"
#include <QFile>


Server::Server()
{
    if (!(this->listen(QHostAddress::Any, 1230)))
    {
        qDebug() << "error listen \n";      // output to the console
    }
}

// connecting the client
void Server::incomingConnection(qintptr socketDescriptor)
{
    socket_ = new QTcpSocket;
    socket_->setSocketDescriptor(socketDescriptor);
    connect(socket_, &QTcpSocket::readyRead, this, &Server::onReadyRead);

    // when disconnecting the client, the socket will be deleted if possible
    connect(socket_, &QTcpSocket::disconnected, this, &QTcpSocket::deleteLater);
}

// receiving a message from a client
void Server::onReadyRead()
{
    socket_ = (QTcpSocket*)sender();
    QDataStream inp(socket_);
    inp.setVersion(QDataStream::Qt_5_15);
    if (inp.status() == QDataStream::Ok)
    {
        QString str;
        inp >> str;
        qDebug() << str;
        QStringList strSplited = str.split(" ");  // separation
        if (strSplited[0] == "GET")
        {
            str = getClient(strSplited);
        }
        else if (strSplited[0] == "POST" && strSplited[1].right(4) == "exit") // exit command
        {
            socket_->close();
            this->close();
            return;
        }
        sendToClient(str);
    }
    else
    {
        qDebug() << "dataStream error ";
    }
}

void Server::sendToClient(QString str)
{
    data_.clear();
    QDataStream out(&data_, QIODevice::WriteOnly);

    out << str;
    socket_->write(data_);
}

QString Server::getClient(QStringList &strSplited)  // getting a file
{
    QFile file(strSplited[1]);
    QString str_file = file.readAll();
    file.close();
    return str_file;
}
