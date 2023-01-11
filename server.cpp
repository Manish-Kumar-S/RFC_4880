// Server side C/C++ program to demonstrate Socket
// programming
#include<bits/stdc++.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>

using namespace std;
#define SAI struct sockaddr_in
#define SA struct sockaddr
#define PORT 8080

class pgpServices
{
    public:

        string AlphaNumeric ;
        int myLen;

        char randomStr(){
            return AlphaNumeric[rand() % myLen];
        }
        
        string generatePublicKey(){
            string str = "";
            int l = 16;
            for(int i = 0; i<l; i++){
                str += randomStr();
            }
            return str;
        }

        static string encode(string message,string publicKey)
        {
            string encodedMessage = "";
            int encodeFactor = publicKey[0];
            int i = 0;
            while(i<message.length()){
                encodeFactor = publicKey[i%publicKey.length()];
                encodedMessage += (char((int(message[i]) + encodeFactor)%127));    
                i++;
            }
            return encodedMessage;
        }

        static string decode(string encodedMessage,pair<int,string> privateKeyIntPair)
        {   
            string publicKey = convertToPublicKey(privateKeyIntPair.first,privateKeyIntPair.second);
            int decodeFactor;
            string decodedMessage = "";
            int i = 0;
            while(i<encodedMessage.length()){
                decodeFactor = publicKey[i%publicKey.length()];
                decodedMessage += char(encodedMessage[i]- decodeFactor)<0?(127+(encodedMessage[i]- decodeFactor)%127):(encodedMessage[i]- decodeFactor); 
                i++;
            }
            return decodedMessage;
        }
        pgpServices()
        {
            AlphaNumeric = "0123456789QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm";
            myLen = sizeof(AlphaNumeric) - 1;

        }

    private :
        static string convertToPublicKey(int convertKey,string privateKey)
        {
            string publicKey = "";
            for(int i=0;i<privateKey.length();i++)
            {
                publicKey += char((int(privateKey[i]) + convertKey)%127);
            }
            return publicKey;
        }
};

class Sender{
    public:
    string Receiver_Public_key;
    string Public_key;
    Sender(){
        pgpServices ob;
        Public_key = "";
        Public_key = ob.generatePublicKey();
        cout<<"Enter receiver's public key:";
        cin>>Receiver_Public_key;
    }
};
class Receiver{
    string Private_key;
    public:

    string Public_key;
    int rand_no;
    Receiver(){
        pgpServices ob;
        Public_key = "";
        Private_key = "";
        rand_no = (rand()%16);
        Public_key = ob.generatePublicKey();

        for(auto i: Public_key){
            Private_key.push_back((i+rand_no)%127);
        }

    }
    string ret_privateKey(){
        return this->Private_key;
    }

};
class Message{
    public:
    string senderPublicKey;
    string receiverPublicKey;
    string message;

    Message(char* m,char* sPublicKey,char* rPublicKey)
    {
        senderPublicKey = sPublicKey;
        receiverPublicKey = rPublicKey;
        message = m;
    }
    Message()
    {
        message = "";
        receiverPublicKey = "";
        senderPublicKey ="";
    }
};

int main()
{
    int sockFd,connFd;
    char OpenPGP_Packet[250];
    bzero(OpenPGP_Packet, 250);
    char receiverPublicKey[250];
    char receiverPublicKeyPacket[250];
    char receiverPublicKeyFromReceiver[250];
    bzero(receiverPublicKey, 250);
    bzero(receiverPublicKeyPacket, 250);
    
    SAI serverAddr,senderAddr,receiverAddr;
    socklen_t lenOfSenderAddr = sizeof(senderAddr);
    socklen_t lenOfReceiverAddr = sizeof(receiverAddr);

    sockFd = socket(AF_INET,SOCK_STREAM,0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");


    if(bind(sockFd,(SA*)&serverAddr,sizeof(serverAddr))<0){
      perror("\nError binding\n");
      exit(1);
    }

    listen(sockFd, 5);

    //1st publickey packet
    connFd = accept(sockFd,(SA*)&receiverAddr,&lenOfReceiverAddr);
    string temp = "";

    recv(connFd,receiverPublicKeyPacket,250,0);
    for(int i=30;i<strlen(receiverPublicKeyPacket);i++)
    {
        temp += receiverPublicKeyPacket[i];
    }
    close(connFd);
    char ch = 'n';
    while(ch == 'n'){{
        cout<<"\nIs sender ready for execution?\n";
        cin>>ch;
    }}

    temp.copy(receiverPublicKey, temp.length());


    connFd = accept(sockFd,(SA*)&senderAddr,&lenOfSenderAddr);
    recv(connFd,OpenPGP_Packet,200,0);//req from sender to server, for public key
    printf("\nUser ID Packet from Sender: %s\n", OpenPGP_Packet);
    send(connFd,receiverPublicKeyPacket,strlen(receiverPublicKeyPacket),0);//2nd public key

    bzero(OpenPGP_Packet, 200);
    recv(connFd,OpenPGP_Packet,200,0);
    printf("\nSymmetrically Encrypted Data Packet from Sender : %s \n", OpenPGP_Packet);

    close(connFd);

    if((connFd = accept(sockFd,(SA*) &receiverAddr,&lenOfReceiverAddr))<0)
    {
        cout<<"\nError in connecting to receiver\n";
        return -4;
    }

    send(connFd, OpenPGP_Packet, strlen(OpenPGP_Packet), 0);

    close(connFd);
    close(sockFd);
    return 0;
}