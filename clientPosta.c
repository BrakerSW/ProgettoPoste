#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
struct user{
    char nome[50];
    char cognome[50];
    char username[50];
    char password[50];
    int flagLogReg;
    int flagOk;
};
typedef struct user User;
struct indirizzo{
    char via[50];
    char città[50];
    int cap;
};
typedef struct indirizzo address;
struct destinatario
{
    int op;
    char nome[50];
    char cognome[50];
    address addr;
};

typedef struct destinatario addressee;

void invioPacco();
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
                printf("flagok:%d",userData.flagOk);
                printf("Username: ");
                fgets(userData.username, sizeof(userData.username), stdin);
                printf("Password:");
                fgets(userData.password, sizeof(userData.password),stdin);

                userData.username[strcspn(userData.username,"\n")]=0;
                userData.password[strcspn(userData.password,"\n")]=0;
                
                userData.flagLogReg=1;
                
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
                printf("valore flagok=%d", userData.flagOk);
                sleep(3);
                if(userData.flagOk==1)
                {
                    printf("connesso al sevizio poste!\n");
                    out=0;//esce fuori dal ciclo 
                }else{
                    printf("Dati inseriti errati!\n");
                }
                
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

                userData.flagLogReg=2;

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
        addressee dest;
        system("clear");
        printf("flaglogreg = %d",userData.flagLogReg);
        printf("SERVIZIO POSTALE!\n 1. Invia Pacco \n 2. Traccia Spedizione \n 3. Calcola Tariffa \n 4. Visualizza Storico Spedizioni\n 0. Esci\n Operazione: ");
        scanf("%d", &operazione);
        
        switch (operazione)
        {
            case 1: // Richiesta di invio del Pacco
                system("clear");
                getchar();//
                dest.op=1;
                printf("INSERISCI I DATI DEL DESTINATARIO: \nNome: ");
                fgets(dest.nome, sizeof(dest.nome),stdin);
                dest.nome[strcspn(dest.nome, "\n")] = 0;
                printf("Cognome: ");
                fgets(dest.cognome, sizeof(dest.cognome), stdin);
                printf("Indirizzo di destinazione: \nVia: ");
                fgets(dest.addr.via, sizeof(dest.addr.via), stdin);
                printf("Città: ");
                fgets(dest.addr.città, sizeof(dest.addr.città), stdin);
                printf("Cap: ");
                scanf("%d", &dest.addr.cap);
                if(connect(socketClient,(struct sockaddr*)&server_addr, sizeof(server_addr))<0)
                {
                    perror("Errore connect");
                    exit(1);
                }
                printf("Invio dei dati...");
                send(socketClient,&dest,sizeof(dest),0);
                
                sleep(3);
                
                break;
                    printf("Inserisci il codice di tracciamento per la spedizione: \nCodice: ");
            case 2: // Richiesta di Tracciamento Spedizione
                break;
            case 3: // Richiesta di calcolo Tariffa
                break;
            case 4: // Richiesta di Visulizzazione dello Storico delle Spedizioni
                break;
            default:
                printf("Scegli un'opzione valida o esci!\n");
                break;
        }
        close(socketClient);
    }
   
    return 0;
}
void invioPacco()
{

}