#ifndef SERVER_H
#define SERVER_H
#include <QAbstractSocket>
#include <QUdpSocket>
#include "polynom.h"
#include "sstream"

class Server : public QUdpSocket
{
    Q_OBJECT
public:
    Server();
    ~Server();
private:
    void sendToClient(QString str);
    QUdpSocket *socket;
    Polynom *poly;
    /*We store it on server, yes. This way, we can make a default polynomial.
     Also, we have to carry over the amount of the roots polynomial has in
     order to prevent it from displaying when some index in *poly->roots
     is actually blank.*/
public slots:
    void slotReadyRead(); // handler of recieved data
};

#endif // SERVER_H
