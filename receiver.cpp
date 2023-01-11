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
                publicKey += char((int(privateKey[i]) - convertKey)%127);
            }
            return publicKey;
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
        srand(time(0));
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


int main(){
    int sockfd, fdsock;
    struct sockaddr_in servaddr;
    Receiver rec;
    char* buff = new char(200);
    char header[100];
    char OpenPGP_Packet[200];
    bzero(OpenPGP_Packet, 200);
    bzero(header, 100);
    cout<<"\nReceiver's public key : "<<rec.Public_key<<endl;
    char ch = 'n';
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    fdsock = socket(AF_INET, SOCK_STREAM, 0);

    servaddr.sin_port = htons(PORT);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    bzero(buff, 200);

    rec.Public_key.copy(buff, rec.Public_key.length());

    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    sprintf(header,"6 %d 3 User-Defined",30+(int)strlen(buff));
    for(int i=strlen(header);i<30;i++)
    {
        strcat(header," ");
    }
    strcat(header, buff);
    cout<<header;

    send(sockfd, header, strlen(header), 0);
    close(sockfd);

    while(ch == 'n'){
        cout<<"\nIs receiver ready for execution?";
        cin>>ch;
    }

    connect(fdsock, (struct sockaddr*)&servaddr, sizeof(servaddr));
    cout<<"\nReceiver connected to server\n\n";

    sprintf(header,"6 %d 3 User-Defined",30+(int)strlen(buff));
    for(int i=strlen(header);i<30;i++)
    {
        strcat(header," ");
    }
    strcat(header, buff);
    

    recv(fdsock, OpenPGP_Packet, 200, 0);
    printf("\nOpenPGP Packet from Server to the Receiver: %s\n", OpenPGP_Packet);
    bzero(buff, 200);

    string temp = "";

    for(int i=30;i<strlen(OpenPGP_Packet);i++)
    {
        temp += OpenPGP_Packet[i];
    }

    temp.copy(buff, temp.length());
    pgpServices pgp;
    string encodedmsg = buff;

    string decodedmsg = pgp.decode(encodedmsg, {rec.rand_no, rec.ret_privateKey()});
    cout<<"\nDecoded message : "<<decodedmsg<<endl;
    return 0;

}