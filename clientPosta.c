#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "header.h"
struct user{
    char nome[50];
    char cognome[50];
    char username[50];
    char password[50];
    int flagOp;//variabile per tenere traccia di quale operazione è stata richiesta
    int flagOk;//variabile per l'avvenuta operazione o il fallimento
};
typedef struct user User;
struct datiSpedizione
{
    char nomeDestinatario[50];
    char cognomeDestinatario[50];
    char via[50];
    char città[50];
    int  cap;
    char civico[4];
};
typedef struct datiSpedizione destinatario;
struct track
{
    char trackcode[4];
    int flag;
};
typedef struct track Tracking;

void invioPacco();
void storico(User userData, int sockStorico,struct sockaddr_in server_addr, char addressString[]);
int main(){
    char addressString[] = "127.0.0.1";
    int socketClient;
    int operazione=1;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port=htons(3000);
    /*if((socketClient=socket(AF_INET, SOCK_STREAM,0))<0)
    {
        perror("socket");
        exit(1);
    }    
    inet_pton(AF_INET, addressString, &server_addr.sin_addr);*/
        
    User userData;

    int scelta=1;
    int out=1;
    while(out!=0){
        int socketLog;
        system("clear");

        printf("Servizio Postale!\n1) Accedi\n2) Registrati\nSelezione: ");
        scanf("%d", &scelta );
        switch (scelta)
        {
            case 1: //accesso al servizio poste
                system("clear");
                getchar();//

                printf("Username: ");
                fgets(userData.username, sizeof(userData.username), stdin);
                userData.username[strcspn(userData.username,"\n")] = '\0';
                printf("Password:");
                fgets(userData.password, sizeof(userData.password),stdin);
                userData.password[strcspn(userData.password,"\n")] = '\0';

                userData.flagOp=1;
                
                if(strlen(userData.username) ==0|| strlen(userData.password) == 0)
                {
                    printf("Campi Vuoti!\n");
                    break;
                }

                sleep(3);
                if((socketLog=socket(AF_INET, SOCK_STREAM,0))<0)
                {
                    perror("socket");
                    exit(1);
                }
                inet_pton(AF_INET, addressString, &server_addr.sin_addr);

                if(connect(socketLog,(struct sockaddr*)&server_addr, sizeof(server_addr))<0)
                {
                    perror("Errore connect");
                    exit(1);
                }               

                send(socketLog,&userData,sizeof(userData),0);
                recv(socketLog,&userData.flagOk,sizeof(int),0);


                if(userData.flagOk==1)
                {
                    printf("connesso al sevizio poste!\n");
                    out=0;//esce fuori dal ciclo 
                }else{
                    printf("Dati inseriti errati!\n");
                }
                sleep(1);

                close(socketLog);                
                break;
            case 2: //Registrazione nuovo utente
                system("clear");
                int socketReg;
                getchar();//

                printf("Nome: ");
                fgets(userData.nome, sizeof(userData.nome), stdin);
                userData.nome[strcspn(userData.nome,"\n")]=0;

                printf("Cognome: ");
                fgets(userData.cognome,sizeof(userData.cognome),stdin);
                userData.cognome[strcspn(userData.cognome,"\n")]=0;

                printf("Username: ");
                fgets(userData.username,sizeof(userData.username),stdin);
                userData.username[strcspn(userData.username,"\n")]=0;

                printf("Password: ");
                fgets(userData.password,sizeof(userData.password),stdin);
                userData.password[strcspn(userData.password,"\n")]=0;

                userData.flagOp=2;

                if((socketReg=socket(AF_INET, SOCK_STREAM,0))<0)
                {
                    perror("socket");
                    exit(1);
                }

                inet_pton(AF_INET, addressString, &server_addr.sin_addr);

                if(connect(socketReg,(struct sockaddr*)&server_addr, sizeof(server_addr))<0)
                {
                    perror("Errore connect");
                    exit(1);
                }
                
                send(socketReg, &userData,sizeof(userData),0);

                printf("Sizeof = %ld\n", sizeof(userData));
                recv(socketReg,&userData.flagOk, sizeof(int),0);
                if(userData.flagOk==1)
                {
                    printf("Registrazione avvenuta con successo!\n");
                    printf("flagok=%d\n",userData.flagOk);

                }else{
                    
                    printf("flagok=%d\n",userData.flagOk);

                    printf("Registrazione fallita!\n Utente già esistente con questo username\n");
                }
                close(socketReg);
                break;
        }
    };
    while(operazione!=0)
    {
        destinatario infoPacco;
        system("clear");
        printf("SERVIZIO POSTALE!\n 1. Invia Pacco \n 2. Traccia Spedizione \n 3. Calcola Tariffa \n 4. Visualizza Storico Spedizioni\n 0. Esci\n Operazione: ");
        scanf("%d", &operazione);
        int sockInvioPacco, sockTracciamento, sockTariffa, sockStorico;
        switch (operazione)
        {
            
            case 1: // Richiesta di invio del Pacco
                system("clear");
                getchar();//
                
                Tracking trackInfo;

                userData.flagOp=3;
                
                printf("INSERISCI I DATI DEL DESTINATARIO: \nNome: ");
                fgets(infoPacco.nomeDestinatario, sizeof(infoPacco.nomeDestinatario),stdin);
                infoPacco.nomeDestinatario[strcspn(infoPacco.nomeDestinatario, "\n")] = '\0';

                printf("Cognome: ");
                fgets(infoPacco.cognomeDestinatario, sizeof(infoPacco.cognomeDestinatario), stdin);
                infoPacco.cognomeDestinatario[strcspn(infoPacco.cognomeDestinatario,"\n")] = '\0';

                printf("Indirizzo di destinazione: \nVia: ");
                fgets(infoPacco.via, sizeof(infoPacco.via), stdin);
                infoPacco.via[strcspn(infoPacco.via,"\n")] = '\0';

                printf("Numero Civico:  ");
                fgets(infoPacco.civico, sizeof(infoPacco.civico), stdin);
                infoPacco.civico[strcspn(infoPacco.civico,"\n")] = '\0';

                printf("Città: ");
                fgets(infoPacco.città, sizeof(infoPacco.città), stdin);
                infoPacco.città[strcspn(infoPacco.città,"\n")] = '\0';
                
                printf("Cap: ");
                scanf("%d",&infoPacco.cap);
                

                if((sockInvioPacco = socket(AF_INET, SOCK_STREAM,0))<0)
                {
                    perror("sockInvioPacco");
                    exit(1);
                }
                
                inet_pton(AF_INET, addressString, &server_addr.sin_addr);
                
                if(connect(sockInvioPacco,(struct sockaddr*)&server_addr, sizeof(server_addr))<0)
                {
                    perror("Errore connect");
                    exit(1);
                }
                
                printf("Invio dei dati...\n");
                
                send(sockInvioPacco, &userData,sizeof(userData),0);
                send(sockInvioPacco,&infoPacco,sizeof(infoPacco),0);

                char trackcode[4];
                recv(sockInvioPacco,&trackInfo,sizeof(trackInfo),0);
                if(trackInfo.flag==1)
                {
                    printf("Spedizione presa in carico!!!\nCodice della spedizione: %d",trackInfo.trackcode);
                }else{
                    printf("Spedizione fallita!!!\n");
                }
                close(sockInvioPacco);
                
                sleep(4);
                
                break;
                   
            case 2: // Richiesta di Tracciamento Spedizione
                userData.flagOp=4;
                char track_code[5];
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                printf("Inserisci il codice di tracciamento per la spedizione: \nCodice: ");
                fgets(track_code, sizeof(track_code), stdin);
                track_code[strcspn(track_code,"\n")]  = '\0';
                printf("\nsizeof %ld\n", sizeof(track_code));
                if((sockTracciamento = socket(AF_INET, SOCK_STREAM,0))<0)
                {
                    perror("SockTracciamento");
                    exit(1);
                }

                inet_pton(AF_INET,addressString,&server_addr.sin_addr);
            
                if(connect(sockTracciamento,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
                {
                    perror("Errore Connect");
                    exit(1);
                }
                send(sockTracciamento, &userData,sizeof(userData),0);

                send(sockTracciamento,track_code,sizeof(track_code),0);
               
                char bufferTrack[256];
                
                recv(sockTracciamento,&userData.flagOk,sizeof(userData.flagOk),0);
                
                if(userData.flagOk==1)
                {
                    recv(sockTracciamento, bufferTrack, sizeof(bufferTrack),0);
                    
                }else{
                    printf("Codice di Tracciamento errato!\n");
                    break;
                }
                printf("%s", bufferTrack);
                close(sockTracciamento);
                sleep(5);
                break;
            case 3: // Richiesta di calcolo Tariffa
                break;
            case 4: 
            
                storico(userData, sockStorico, server_addr,addressString);
            // Richiesta di Visualizzazione dello Storico delle Spedizioni
             /*   if((sockStorico = socket(AF_INET, SOCK_STREAM,0))<0)
                {
                    perror("sockStorico");
                    exit(1);
                }
                inet_pton(AF_INET, addressString, &server_addr.sin_addr);
                if(connect(sockStorico,(struct sockaddr*)&server_addr, sizeof(server_addr))<0)
                {
                    perror("Errore connect");
                    exit(1);
                }
                userData.flagOp=4+2;

                send(sockStorico,&userData,sizeof(userData),0);

                char buffer[512];
                int bytes;
                while((bytes=recv(sockStorico,&buffer, sizeof(buffer),0))>0)
                {
                    buffer[bytes]='\0';
                    if(strncmp(buffer,"END",3)==0)
                    {
                        break;
                    }
                    printf("%s",buffer);
                }  
                sleep(5);
                close(sockStorico);*/
                break;
            default:
                printf("Scegli un'opzione valida o esci!\n");
                break;
        }
    }
    return 0;
}
void invioPacco()
{

}
void storico(User userData, int sockStorico,struct sockaddr_in server_addr, char addressString[])
{
    // Richiesta di Visualizzazione dello Storico delle Spedizioni
                if((sockStorico = socket(AF_INET, SOCK_STREAM,0))<0)
                {
                    perror("sockStorico");
                    exit(1);
                }
                inet_pton(AF_INET, addressString, &server_addr.sin_addr);
                if(connect(sockStorico,(struct sockaddr*)&server_addr, sizeof(server_addr))<0)
                {
                    perror("Errore connect");
                    exit(1);
                }
                userData.flagOp=4+2;

                send(sockStorico,&userData,sizeof(userData),0);

                char buffer[512];
                int bytes;
                while((bytes=recv(sockStorico,&buffer, sizeof(buffer),0))>0)
                {
                    buffer[bytes]='\0';
                    if(strncmp(buffer,"END",3)==0)
                    {
                        break;
                    }
                    printf("%s",buffer);
                }  
                sleep(5);
                close(sockStorico);  
}
