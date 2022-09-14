#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <thread>
#include <assert.h>
#include <chrono>
#include <future>
using namespace std;

/**
 * This client connects to the address and port of the server. It proceeds to
 * ping-pong data with another instance of itself once a second client connects
 * to the relay server.
 */
void punch(sockaddr_in sendSockAddr, int udpSd, std::future<void> futureObj) {

    char msg[1500];

        while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {

            cout << "bang" << endl;
            memset(&msg, 0, sizeof(msg));//clear the buffer
            strcpy(msg, "BANG");
            if (sendto(udpSd, (char*)msg, sizeof(msg), 0, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr)) == -1) {

                cout << "failed to punch" << endl;
                exit(1);

            }

            sleep(1);

        }

}

void rcv(int clientSd) {
    char msg[1500];
    while (1)
    {
        //cout << "Awaiting server response..." << endl;
        memset(&msg, 0, sizeof(msg));//clear the buffer

        if (recv(clientSd, (char*)&msg, sizeof(msg), 0) == -1) {
            cout << "cant recv" << endl;
            sleep(3);
        }
        if (!strcmp(msg, "exit"))
        {
            cout << "someone has quit the session" << endl;
            break;
        }
        cout << ">someone: " << msg << endl;
    }

}

void snd(int tcpSd) {
    char msg[1500];
    while (1)
    {

        string data, hd;
        getline(cin, data);
        memset(&msg, 0, sizeof(msg));//clear the buffer
        strcpy(msg, (data).c_str());
        if (data == "exit")
        {
            send(tcpSd, (char*)&msg, strlen(msg), 0);
            break;
        }
        if ((data.find(".txt") != std::string::npos) || (data.find(".doc") != std::string::npos) || (data.find(".docx") != std::string::npos) ||
            (data.find(".xlsx") != std::string::npos) || (data.find(".cpp") != std::string::npos) || (data.find(".c") != std::string::npos))
        {
            ifstream f1;
            string drtry;
            while (1) {
                cout << "Directory: ";
                getline(cin, drtry);
                f1.open(drtry + data);
                if (f1.is_open()) {
                    cout << "11" << endl;
                    break;
                }
                else {
                    cout << "No such file or directory  " << endl;
                    cout << "File name: ";
                    getline(cin, data);
                    if (data == "exit") {
                        break;
                    }
                }

            }
            send(tcpSd, (char*)&msg, strlen(msg), 0);

        }
        if (send(tcpSd, (char*)&msg, strlen(msg), 0) == -1) {

            cout << "didn't send through" << endl;
        }

    }

}

int main(int argc, char* argv[])
{
    //we need 2 things: ip address and port number, in that order
    if (argc != 2)
    {
        cerr << "Usage: cipher " << endl; exit(0);
    } //grab the IP address and port number

    char* serverIp = "24.5.179.24"; int svport = 11111;
    char svmsg[1500], svmsg1[1500], svmsg2[1500], svmsg3[1500];
    //setup a socket and connection tools 
    struct hostent* svhost = gethostbyname(serverIp);
    sockaddr_in svAddr, sendSockAddr, myAddr;
    bzero((char*)&svAddr, sizeof(svAddr));
    svAddr.sin_family = AF_INET;
    svAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*svhost->h_addr_list));
    svAddr.sin_port = htons(svport);

    int clientSd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSd == -1) {
        cout << "cantsocket" << endl;
    }

    if (connect(clientSd, (sockaddr*)&svAddr, sizeof(svAddr)) < 0) {
        cout << "cant connect to server, try again later maybe" << endl;
        exit(1);
    }

    int tcpSd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSd == -1) {
        cout << "canttcpsocket" << endl;
    }

    const int opt = 1;
    if (setsockopt(tcpSd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cout << "prblm" << endl;
    }

    char* tgtip = argv[1];
    memset(&svmsg, 0, sizeof(svmsg));//clear the buffer
    strcpy(svmsg, tgtip);
    send(clientSd, (char*)&svmsg, strlen(svmsg), 0);
    int sport; int rport;
    memset(&svmsg1, 0, sizeof(svmsg1));
    recv(clientSd, (char*)&svmsg1, sizeof(svmsg1), 0);
    const char* pt0 = svmsg1;
    memset(&svmsg2, 0, sizeof(svmsg2));
    recv(clientSd, (char*)&svmsg2, sizeof(svmsg2), 0);
    const char* pt = svmsg2;
    memset(&svmsg3, 0, sizeof(svmsg3));
    recv(clientSd, (char*)&svmsg3, sizeof(svmsg3), 0);
    const char* pt2 = svmsg3;
    //close(clientSd);
    cout << pt0 << endl;
    cout << pt << endl;
    cout << pt2 << endl;


    //create a message buffer 
    char msg[1500]; sport = atoi(pt); rport = atoi(pt2);
    //setup a socket and connection tools 
    struct hostent* host = gethostbyname(pt0);

    socklen_t ssz = sizeof(sendSockAddr);
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(sport);

    bzero((char*)&myAddr, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons(rport);

    int udpSd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSd == -1) {
        cout << "cantsocket" << endl;
    }

    struct timeval tv = {    .tv_sec = 6    };
    if (setsockopt(udpSd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        cout << "prblm2" << endl;
    }

    if (bind(udpSd, (struct sockaddr*)&myAddr, sizeof(myAddr)) < 0) {
        cerr << "cantbind, maybe try another port" << endl;
        exit(1);
    }

    std::promise<void> exitSignal1;
    std::future<void> futureObj1 = exitSignal1.get_future();

    thread t1(&punch, sendSockAddr, udpSd, std::move(futureObj1));
    cout << "punching.." << endl;
    bool flg1 = false;
    bool flg2 = false;
    /**/while (1) {
        memset(&msg, 0, sizeof(msg));//clear the buffer
        if (recv(udpSd, (char*)msg, sizeof(msg), 0) != -1) {
            cout << "the other side: " << msg << endl;
            if (!strcmp(msg, "BANG")) {
                cout << "THE HOLE's HERE, telling others.." << endl;
                sendto(udpSd, "WE GOT THE HOLE", 15, 0, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr));
                flg1 = true;
            }
            if (!strcmp(msg, "WE GOT THE HOLE")) {
                cout << "punching done" << endl;
                exitSignal1.set_value();
                t1.join();
                flg2 = true;
            }
            if (flg1 == true && flg2 == true) {
                cout << "hole's ready" << endl;
                break;
            }
        }
        else {
            cout << "cant recv" << endl;
            exitSignal1.set_value();
            t1.join();
            break;
        }
    }

    std::thread t2;
    cout << flg1 << flg2 << endl;

    if (bind(tcpSd, (struct sockaddr*)&myAddr, sizeof(myAddr)) == -1) {
        cout << "cantbindtcp" << endl;
        exit(1);
    }

    if (flg1 == false && flg2 == false) {

        cout << "need relay" << endl;
        string data = "punchedfail";
        memset(&msg, 0, sizeof(msg));//clear the buffer
        strcpy(msg, (data).c_str());
        send(clientSd, (char*)&msg, strlen(msg), 0);
        memset(&msg, 0, sizeof(msg));//clear the buffer
        t2 = std::thread(rcv, clientSd);
        snd(clientSd);

    }
    else if (connect(tcpSd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr)) == -1) {
        cout << "cantconnect, retrying once.." << endl;
        sleep(3);
        if (connect(tcpSd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr)) == -1) {
            cout << "cantconnect, retrying twice.." << endl;
            sleep(4);
            if (connect(tcpSd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr)) == -1) {
                cout << "cantconnect, abort" << endl;
                exit(1);
            }
        }
    }
    else {

        /*cout << "hole's ready" << endl;
        exitSignal1.set_value();
        t1.join();*/
        cout << "connected" << endl;
        string data = "punchedthrough";
        memset(&msg, 0, sizeof(msg));//clear the buffer
        strcpy(msg, (data).c_str());
        send(clientSd, (char*)&msg, strlen(msg), 0);
        memset(&msg, 0, sizeof(msg));//clear the buffer
        close(clientSd);

        t2 = std::thread(rcv, tcpSd);
        snd(tcpSd);

    }

    t2.join();
    close(tcpSd);
    close(udpSd);
    cout << "********Session********" << endl;
    cout << "Connection closed" << endl;

    return 0;


}
