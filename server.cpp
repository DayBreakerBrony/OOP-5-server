#include "server.h"

Server::Server() {
    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::LocalHost, 1984);
    connect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    this->poly=new Polynom(3);
    stringstream fp;
    fp << "1+0i 2+0i 3+0i 4+0i";
    fp >> *poly;
    qDebug() << "start";
}
Server::~Server(){
    delete poly;
}
void Server::slotReadyRead() {
    QHostAddress sender;
    quint16 senderPort;
    stringstream rReqSs, resSs; // recieved request stringstream
    qDebug() << "reading";
    while (socket->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());
        socket->readDatagram(datagram.data(),datagram.size(),&sender,&senderPort);
        rReqSs << QString(datagram).toStdString();
        int menu_number;
        rReqSs >> menu_number;
        qDebug() << "Recieved:";
        switch (menu_number) {
        case 1:
        {

            int new_poly_size;
            qDebug() << "1:New polynomial creation request.";
            if (rReqSs >> new_poly_size || !rReqSs.fail()){
                qDebug() << "Size:" << new_poly_size;
                if (new_poly_size <= 0) {
                    sendToClient("0"); //if it's a zero polynomial
                    return;
                }
                stringstream check;
                string current_root;
                int root_counter = 0;
                check.str(rReqSs.str());     // <==== a copy of rReqSs to check roots. BUT!
                string *dummy = new string; // rReqSs has a menu number and a number of roots
                check >> *dummy;           // in first two iterations. So we extract the first
                check >> *dummy;          // two in dummy and get rid of it.
                delete dummy;            // This way, we get an ss of roots ONLY. Profit!
                while (!check.eof()){
                    if (check >> current_root || !check.fail()){
                        root_counter++;
                    }
                    else{
                        sendToClient("Error while parsing data!");
                        qDebug() << "Error while parsing data from request! Data:"
                                 << QString(datagram);
                        return;
                    }
                }
                if (root_counter - 1 < new_poly_size) {
                    sendToClient("Expected more roots than provided");
                    qDebug() << "Not enough roots to create a polynomial. Data:"
                             << QString(datagram);
                    return;
                    /*we HAVE TO do this separately or else it
                    goes copying the only root to all indexes which is nuts*/
                }
                this->poly = new Polynom(new_poly_size);
                rReqSs >> *poly; // and finally we can create a polynomial
                resSs << "The created polynomial: " << *poly;
                sendToClient(QString::fromStdString(resSs.str()));
                qDebug() << "Polynomial has been successfully sent.";
                qDebug() << "Sent polynomial:" << resSs.str();
                return;
            }
            else {
                sendToClient("Error while parsing data!");
                qDebug() << "Error while parsing data from request! Data:"
                         << QString(datagram);
                return;
            }
            break;}
        case 2:
        {
            qDebug() << "2:Polynomial resize request.";
            int new_roots_size = 0;
            if(rReqSs >> new_roots_size || !rReqSs.fail()){
                poly->resize(new_roots_size);
                sendToClient("The array of roots has been successfully resized");
                qDebug() << "The array of roots has been successfully resized";
                return;
            }
            else{
                sendToClient("Error while parsing data!");
                qDebug() << "Error while parsing data from request! Data:"
                         << QString(datagram);
                return;
            }
            break;}
        case 3:
        {
            qDebug() << "3:Request to calculate the value in x.";
            number x;
            if(rReqSs >> x || !rReqSs.fail()){
                resSs << poly->solve_for_x(x);
                sendToClient(QString::fromStdString(resSs.str()));
                qDebug() << "Result in x has been successfully sent.";
                qDebug() << "Sent result:" << resSs.str();
                return;
            }
            else{
                sendToClient("Error while parsing data!");
                qDebug() << "Error while parsing data from request! Data:"
                         << QString(datagram);
                return;
            }
            break;}
        case 4:
        {
            qDebug() << "4:Request to change the value of A_n.";
            number new_a_n;
            if(rReqSs >> new_a_n || !rReqSs.fail()){
                poly->set_a_n(new_a_n);
                resSs << new_a_n;
                sendToClient("Coefficient A_n has been changed to: " + QString::fromStdString(resSs.str()));
                qDebug() << "Coefficient A_n has been successfully changed.";
                return;
            }
            else{
                sendToClient("Error while parsing data!");
                qDebug() << "Error while parsing data from request! Data:"
                         << QString(datagram);
                return;
            }
            break;}
        case 5:
        {
            qDebug() << "5:Request to change a certain root.";
            int index;
            number new_root;
            if(rReqSs >> index || !rReqSs.fail()){
                if(rReqSs >> new_root || !rReqSs.fail()){
                    poly->change_root(index,new_root);
                    sendToClient("Root has been changed succesfully!");
                    qDebug() << "Root has been changed succesfully.";
                    return;
                }
                else{
                    sendToClient("Error while parsing data!");
                    qDebug() << "Error while parsing data from request! Data:"
                             << QString(datagram);
                    return;
                }
            }
            else {
                sendToClient("Error while parsing data!");
                qDebug() << "Error while parsing data from request! Data:"
                         << QString(datagram);
                return;
            }
            break;}
        case 6:{
            qDebug() << "6:Request to display the polynomial in unexpanded form.";
            int index; // I want at least some clarity. Let this store the missing index if there is one due to resize.
            index = poly->are_roots_full();
            if (index == -1){
                poly->set_display_format(0);
                resSs << *poly;
                sendToClient(QString::fromStdString(resSs.str()));
                qDebug() << "Polynomial has been sent succesfully. Sent:" << QString::fromStdString(resSs.str());
                return;
            }
            else{
                resSs<<index;
                sendToClient("One or more indexes is empty. First met empty index: " + QString::fromStdString(resSs.str()));
                qDebug() << "One or more indexes is empty. Operation aborted.";
            }
            break;}
        case 7:
            qDebug() << "7:Request to display the polynomial in unexpanded form.";
            int index; // I want at least some clarity. Let this store the missing index if there is one due to resize.
            index = poly->are_roots_full();
            if (index == -1){
                poly->set_display_format(1);
                resSs << *poly;
                sendToClient(QString::fromStdString(resSs.str()));
                qDebug() << "Polynomial has been sent succesfully. Sent:" << QString::fromStdString(resSs.str());
                return;
            }
            else{
                resSs<<index;
                sendToClient("One or more indexes is empty. First met empty index: " + QString::fromStdString(resSs.str()));
                qDebug() << "One or more indexes is empty. Operation aborted.";
            }
            break;
        }
    }
}
void Server::sendToClient(QString str){
    socket->writeDatagram(str.toUtf8(), QHostAddress::LocalHost, 1985);
};
