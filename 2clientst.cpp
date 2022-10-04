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

bool yyn = false;
int udpSd;
int tcpSd;

/**
 * This client connects to the address and port of the server. It proceeds to
 * ping-pong data with another instance of itself once a second client connects
 * to the relay server.
 */
void punch(sockaddr_in sendSockAddr, std::future<void> futureObj) {

    bool yxn = false;
    char msg[1500];
    int i;
    for(i = 0; i < 8; i++){
        if(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {

            //cout << "bang" << endl;
            memset(&msg, 0, sizeof(msg));//clear the buffer
            strcpy(msg, "BANG");
            if (sendto(udpSd, (char*)msg, sizeof(msg), 0, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr)) == -1) {

                cout << "failed to punch" << endl;

            }

            sleep(1);

        }
        else {
            yxn = true;
        }
    }
    if (yxn == false) {
        close(udpSd);
        close(tcpSd);
        yyn = true;
    }
}

sockaddr_in smt() {
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    sockaddr_in loopback;

    if (sock == -1) {
        std::cerr << "Could not socket\n";
    }

    memset(&loopback, 0, sizeof(loopback));
    loopback.sin_family = AF_INET;
    loopback.sin_addr.s_addr = 1337;   // can be any IP address
    loopback.sin_port = htons(9999);      // using debug port

    if (connect(sock, reinterpret_cast<sockaddr*>(&loopback), sizeof(loopback)) == -1) {
        close(sock);
        std::cerr << "Could not connect\n";
    }

    socklen_t addrlen = sizeof(loopback);
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&loopback), &addrlen) == -1) {
        close(sock);
        std::cerr << "Could not getsockname\n";
    }

    close(sock);

    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &loopback.sin_addr, buf, INET_ADDRSTRLEN) == 0x0) {
        std::cerr << "Could not inet_ntop\n";
    }
    else {
        std::cout << "Local ip address: " << buf << "\n";
    }

    return loopback;

}

void rcv(int clientSd) {
    char msg[1500];
    while (1)
    {
        //cout << "Awaiting server response..." << endl;
        memset(&msg, 0, sizeof(msg));//clear the buffer
        int a = recv(clientSd, (char*)&msg, sizeof(msg), 0);

        if ( a == -1) {
            cout << "cant recv" << endl;
            sleep(3);
        }
        if (a == 0) {

            cout << "lost connection" << endl;
            break;

        }
        if (!strcmp(msg, "exit"))
        {
            cout << "someone has quit the session" << endl;
            break;
        }

        cout << ">someone: " << msg << endl;

        string sr;
        for (int i = 0; i < strlen(msg); i++) {

            sr.push_back(msg[i]);
            if ((sr.find(".txt") != std::string::npos) || (sr.find(".doc") != std::string::npos) || (sr.find(".docx") != std::string::npos) ||
                (sr.find(".xlsx") != std::string::npos) || (sr.find(".cpp") != std::string::npos) || (sr.find(".c") != std::string::npos) || (sr.find(".pptx") != std::string::npos)
                || (sr.find(".pdf") != std::string::npos) || (sr.find(".png") != std::string::npos) || (sr.find(".jpg") != std::string::npos))
            {
                cout << "receiving file.." << endl;
                memset(&msg, 0, sizeof(msg));
                recv(clientSd, (char*)&msg, sizeof(msg), 0);
                cout << "size: " << msg << "bytes" << endl;
                int i = 0;
                char* buffer = new char[atoi(msg)];
                while (i < atoi(msg)) {
                    const int l = recv(clientSd, &buffer[i], min(4096, atoi(msg) - i), 0);
                    if (l < 0) { cout << "bs" << endl; } // this is an error
                    i += l;
                }
                cout << "file received " << i << " bytes" << endl;
                ofstream file(sr, ios::binary);
                file.write(buffer, atoi(msg));
                delete[] buffer;
                file.close();
                cout << "yay" << endl;
                break;
            }
        }
    }

}

void snd(int tcpSd1) {
    char msg[1500];
    while (1)
    {

        memset(&msg, 0, sizeof(msg));//clear the buffer
        string data, hd;
        getline(cin, data);
        strcpy(msg, (data).c_str());
        if (data == "exit")
        {
            send(tcpSd1, (char*)&msg, strlen(msg), 0);
            break;
        }
        if ((data.find(".txt") != std::string::npos) || (data.find(".doc") != std::string::npos) || (data.find(".docx") != std::string::npos) ||
            (data.find(".xlsx") != std::string::npos) || (data.find(".cpp") != std::string::npos) || (data.find(".c") != std::string::npos) || (data.find(".jpg") != std::string::npos)
            || (data.find(".pptx") != std::string::npos) || (data.find(".pdf") != std::string::npos) || (data.find(".png") != std::string::npos))
        {
            ifstream f1;
            string drtry;
            while (1) {
                cout << "Directory: ";
                getline(cin, drtry);
                f1.open(drtry + data, ios::binary);
                if (f1.is_open()) {
                    send(tcpSd1, (char*)&msg, strlen(msg), 0);
                    cout << "11" << endl;
                    f1.seekg(0, ios::end);
                    int s1 = f1.tellg();
                    f1.seekg(0, ios::beg);
                    char* buffer = new char[s1];
                    f1.read(buffer, s1);
                    f1.close();
                    memset(&msg, 0, sizeof(msg));
                    strcpy(msg, (to_string(s1)).c_str());
                    send(tcpSd1, (char*)&msg, strlen(msg), 0);
                    usleep(200000);
                    cout << "size: " << msg << endl;
                    int i = 0;
                    while (i < s1) {
                        const int l = send(tcpSd1, &buffer[i], min(4096, s1 - i), 0);
                        if (l < 0) { cout << "bs" << endl; } // this is an error
                        i += l;
                    }
                    delete[] buffer;
                    cout << "file sent " << i << " bytes" << endl;
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

        }
        else if (send(tcpSd1, (char*)&msg, strlen(msg), 0) == -1) {

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
    char svmsg[50], svmsg1[50], svmsg2[50], svmsg3[50], svmsg4[50];
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

    tcpSd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSd == -1) {
        cout << "canttcpsocket" << endl;
    }

    const int opt = 1;
    if (setsockopt(tcpSd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cout << "prblm" << endl;
    }

    char* tgtip = argv[1]; char abb[INET_ADDRSTRLEN];
    memset(&svmsg, 0, sizeof(svmsg));//clear the buffer
    strcpy(svmsg, tgtip);
    send(clientSd, (char*)&svmsg, strlen(svmsg), 0);
    memset(&svmsg4, 0, sizeof(svmsg4));
    sockaddr_in fm = smt();
    strcpy(svmsg4, inet_ntop(AF_INET, &(fm.sin_addr.s_addr), abb, INET_ADDRSTRLEN));
    sleep(1);
    send(clientSd, (char*)svmsg4, sizeof(svmsg4), 0);
    bzero((char*)&fm, sizeof(fm));
    int sport; int rport;
    memset(&svmsg1, 0, sizeof(svmsg1)); 
    memset(&svmsg2, 0, sizeof(svmsg2));
    memset(&svmsg3, 0, sizeof(svmsg3));
    int f1, f2, f3;
    f1 = recv(clientSd, (char*)&svmsg1, sizeof(svmsg1), 0);
    if ( f1 <= 0) {
            cout << "didntrcv" << endl;
    }
    const char* pt0 = svmsg1;
    cout << svmsg1 << "(bytes:" << f1 << ")" << endl;
    cout << pt0 << endl;


    f2 = recv(clientSd, (char*)&svmsg2, sizeof(svmsg2), 0);
    if ( f2 <= 0) {
            cout << "didntrcv" << endl;
    }

    const char* pt = svmsg2;
    cout << svmsg2 << "(bytes:" << f2 << ")" << endl;
    cout << pt << endl;


    f3 = recv(clientSd, (char*)&svmsg3, sizeof(svmsg3), 0);
    if ( f3 <= 0) {
            cout << "didntrcv" << endl;
    }

    const char* pt2 = svmsg3;
    cout << svmsg3 << "(bytes:" << f3 << ")" << endl;
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

    udpSd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSd == -1) {
        cout << "cantsocket" << endl;
    }

    if (setsockopt(udpSd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cout << "prblm2" << endl;
    }


    if (bind(udpSd, (struct sockaddr*)&myAddr, sizeof(myAddr)) < 0) {
        cerr << "cantbind, maybe try another port" << endl;
        exit(1);
    }

    std::promise<void> exitSignal1;
    std::future<void> futureObj1 = exitSignal1.get_future();

    thread t1(&punch, sendSockAddr, std::move(futureObj1));
    cout << "punching.." << endl;
    bool flg1 = false;
    bool flg2 = false;
    /*while (1) {
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
    }*/

    std::thread t2;

    if (bind(tcpSd, (struct sockaddr*)&myAddr, sizeof(myAddr)) == -1) {
        cout << "cantbindtcp" << endl;
        exit(1);
    }

    if (connect(tcpSd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr)) == -1) {

        if (yyn == false) {
            cout << "cantconnect, retrying once.." << endl;
            sleep(2);
            if ((connect(tcpSd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr)) == -1) && yyn == false) {
                cout << "cantconnect, retrying twice.." << endl;
                sleep(5);
                if ((connect(tcpSd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr)) == -1) && yyn == false) {
                    cout << "cantconnect, abort" << endl;
                    exit(1);
                }
            }

        }

    }

    if(yyn == false) {

        cout << "hole's ready" << endl;
        exitSignal1.set_value();
        t1.join();
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
    else {

        t1.join();
        cout << "relaying" << endl;
        string data = "punchedfail";
        memset(&msg, 0, sizeof(msg));//clear the buffer
        strcpy(msg, (data).c_str());
        send(clientSd, (char*)&msg, strlen(msg), 0);
        memset(&msg, 0, sizeof(msg));//clear the buffer
        close(tcpSd);

        t2 = std::thread(rcv, clientSd);
        snd(clientSd);

    }

    t2.join();
    if (yyn == true) {
        close(clientSd);
    }
    else {
        close(tcpSd);
    }
    close(udpSd);
    cout << "********Session********" << endl;
    cout << "Connection closed" << endl;

    return 0;


}
