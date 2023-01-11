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
        Receiver_Public_key = "";
    }
};


int main(){
    pgpServices pgp;
    int sockfd;
    struct sockaddr_in servaddr;
    char* receiverPublicKeyPacket;
    receiverPublicKeyPacket = new char(200);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    cout<<"\nEnter message to be sent : "<<endl;
    
    char *MSG = new char(200);
    string msg;
    cin.getline(MSG, 200);
    msg = MSG;

    Sender sender;


    servaddr.sin_port = htons(PORT);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    char ch = 'n';
    while(ch == 'n'){{
        cout<<"\nIs sender ready for execution?\n";
        cin>>ch;
    }}

    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    char header[100];
    bzero(header, 100);
    char email_id[50] = "abc@gmail.com";

    sprintf(header,"13 %d 3 User-Defined",30+(int)strlen(email_id));
    for(int i=strlen(header);i<30;i++)
    {
        strcat(header," ");
    }

    strcat(header, email_id);
    send(sockfd, header, strlen(header), 0);//requesting receiver's public key
    recv(sockfd, receiverPublicKeyPacket, 100, 0);//receiving public key of receiver
    for(int i=30;i<strlen(receiverPublicKeyPacket);i++)
    {
        sender.Receiver_Public_key += receiverPublicKeyPacket[i];
    }    

    msg = pgp.encode(msg, sender.Receiver_Public_key);

    char* buff0 = new char(sender.Public_key.length()+1);
    char* buff1 = new char(sender.Receiver_Public_key.length()+1);
    char* buff2 = new char(msg.length()+1);

    sender.Receiver_Public_key.copy(buff1, sender.Receiver_Public_key.length());

    sender.Public_key.copy(buff0, sender.Public_key.length());
    
    msg.copy(buff2, msg.length());

    

    char OpenPGP_packet[200];
    bzero(OpenPGP_packet, 200);
    bzero(header, 100);

    sprintf(header,"9 %d 3 User-Defined",30+(int)strlen(buff2));
    for(int i=strlen(header);i<30;i++)
    {
        strcat(header," ");
    }

    strcat(OpenPGP_packet, header);
    strcat(OpenPGP_packet, buff2);
    cout<<"\nEncoded form of the message: "<<buff2<<endl;
    
    send(sockfd, OpenPGP_packet, strlen(OpenPGP_packet), 0);
    cout<<"\nSender has sent its object\n\n";

    cout<<"\nClosing sender....\n";
    close(sockfd);
    return 0;


}