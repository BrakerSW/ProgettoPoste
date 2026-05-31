#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sqlite3.h>
#include <time.h>

struct user{
    char nome[50];
    char cognome[50];
    char username[50];
    char password[50];
    int flagLogReg;
    int flagOk;
    int op;
};
typedef struct user User;
struct indirizzo{
    char via[50];
    char città[50];
    int cap;
};
typedef struct indirizzo address;
struct spedizione
{
    char nomeDestinatario[50];
    char cognomeDestinatario[50];
    char tracking_code[4];
    address addr;
};

typedef struct spedizione destData;
typedef struct client Client;
pthread_mutex_t db_mutex= PTHREAD_MUTEX_INITIALIZER;
void *gestoreClient(void *arg);
void registrazioneUtente(int sock, User userData);
void accessoUtente(int sock, User userData);
void inviaPacco(User userData ,int sock);
void tracking_code_generate(char track_code[]);


void *gestoreRichiesta(void* arg);
int main()
{
    int serverSocket;
    struct sockaddr_in serverAddr;
    
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_addr.s_addr=INADDR_ANY;
    serverAddr.sin_port=htons(3000);
 
    if((serverSocket=socket(AF_INET, SOCK_STREAM,0))<0)
    {
        perror("Errore socket:");
        exit(1);
    }
 
    if(bind(serverSocket, (struct sockaddr*)&serverAddr,sizeof(serverAddr))<0)
    {
        perror("Errore Bind");
        exit(1);
    }
 
    listen(serverSocket, 1024);
 
    while(1)
    {

        int accept_socket;
        int addrlen= sizeof(serverAddr);
    
        if((accept_socket=accept(serverSocket, (struct sockaddr*)&serverAddr,(socklen_t*)&addrlen))<0)
        {
            perror("Errore accept");
            exit(1);
        }
    
    
        int *sock = malloc(sizeof(int));
        *sock = accept_socket;
        
        pthread_t thread;
        pthread_create(&thread, NULL, gestoreRichiesta, sock);
        pthread_detach(thread);        
        
    }
    return 0;
}
void registrazioneUtente(int sock, User userData)
{   
        sqlite3 *db;
        sqlite3_stmt *stm;
    
        printf("%s\n%s\n", userData.nome,userData.cognome);

        printf("client connesso\n");
    
        sqlite3_open("database.db", &db);
        

        pthread_mutex_lock(&db_mutex);

        char *queryInsert ="INSERT INTO utenti (Username, nome, cognome, password) VALUES (?,?,?,?)";
    
        sqlite3_prepare_v2(db, queryInsert, -1, &stm, NULL);
        sqlite3_bind_text(stm, 1, userData.username,-1, SQLITE_STATIC);
        sqlite3_bind_text(stm, 2, userData.nome, -1, SQLITE_STATIC);
        sqlite3_bind_text(stm, 3, userData.cognome,-1, SQLITE_STATIC);
        sqlite3_bind_text(stm, 4, userData.password,-1, SQLITE_STATIC);

        if(sqlite3_step(stm)==SQLITE_DONE)
        {
            userData.flagOk=1;
        }else{
            userData.flagOk=0;
        }
    
        send(sock, &userData.flagOk,sizeof(int),0);
    
        sqlite3_finalize(stm);
        sqlite3_close(db);
    
        pthread_mutex_unlock(&db_mutex);

}
void accessoUtente(int sock, User userData)
{
    sqlite3 *db;
    sqlite3_stmt *stm;
    sqlite3_open("database.db", &db);
    
    pthread_mutex_lock(&db_mutex);

    char *querySelect="SELECT username FROM utenti WHERE username = ? AND password = ?";
    
    sqlite3_prepare_v2(db, querySelect, -1, &stm, NULL);
    sqlite3_bind_text(stm, 1, userData.username,-1, SQLITE_STATIC);
    sqlite3_bind_text(stm, 2, userData.password,-1, SQLITE_STATIC);
    
    if(sqlite3_step(stm)==SQLITE_ROW)
    {
        printf("\nAccesso avvenuto con sucesso invio i dati\n");
        userData.flagOk=1;
    }else
    {
        userData.flagOk=0;
    }

    printf("flagok=%d",userData.flagOk);
    send(sock,&userData.flagOk,sizeof(int),0);
    
    sqlite3_finalize(stm);
    sqlite3_close(db);
    
    pthread_mutex_unlock(&db_mutex);
}
void inviaPacco(User userData ,int sock)
{
    destData infoPacco;
    
    sqlite3 *db;
    sqlite3_stmt *stm;
    sqlite3_open("database.db", &db);
    
    recv(sock,&infoPacco,sizeof(infoPacco),0);
    
    char *queryInsert="INSERT INTO spedizioni (tracking_code,username_mittente, nome_destinatario, cognome_destinatario, via, cap, Città, data_spedizione, stato ) VALUES (?,?,?,?,?,?,?,?,?)";
    char *querySelect="SELECT tracking_code FROM spedizioni WHERE tracking_code=?";
    int track=1;
    while(track){
        
        tracking_code_generate(infoPacco.tracking_code);
        
        sqlite3_prepare_v2(db, querySelect, -1, &stm,NULL);

        sqlite3_bind_text(stm, 1, infoPacco.tracking_code,-1, SQLITE_STATIC);


        if(sqlite3_step(stm)==SQLITE_ROW)
        {
            track=1;
            sqlite3_finalize(stm);
        }
        else{
            
            time_t data;
            
            struct tm *tm_info;
            time(&data);
            tm_info= localtime(&data);

            char time[20];

            strftime(time, sizeof(time), "%Y-%m-%d", tm_info);
            sqlite3_prepare_v2(db, queryInsert,-1,&stm,NULL);
            
            sqlite3_bind_text(stm, 1, infoPacco.tracking_code,-1, SQLITE_STATIC);
            sqlite3_bind_text(stm, 2, userData.username, -1,SQLITE_STATIC);
            sqlite3_bind_text(stm, 3, infoPacco.nomeDestinatario,-1,SQLITE_STATIC);
            sqlite3_bind_text(stm, 4, infoPacco.cognomeDestinatario, -1, SQLITE_STATIC);
            sqlite3_bind_text(stm, 5, infoPacco.addr.via,-1,SQLITE_STATIC);
            sqlite3_bind_int(stm, 6, infoPacco.addr.cap);
            sqlite3_bind_text(stm, 7, infoPacco.addr.città,-1,SQLITE_STATIC);
            sqlite3_bind_text(stm, 8, time, -1, SQLITE_STATIC);
            sqlite3_bind_text(stm, 9, "In Preparazione",-1, SQLITE_STATIC);
            
            sqlite3_finalize(stm);
            sqlite3_close(db);

            track=0;
        }

    }

}
void tracking_code_generate(char track_code[])
{
    srand(time(NULL));

    for(int i=0;i<4;i++)
    {
        track_code[i]='0'+rand()% 10;
    }
    track_code[4] = '\0';

}
void trackSpedizione()
{}
void calcoloTariffa()
{}
void storico()
{}
void *gestoreRichiesta(void* arg)
{
    int sock= *(int*)arg;
    User userData;
    free(arg);
    recv(sock, &userData,sizeof(userData),0);
    printf("%s\n%s\n%d", userData.username,userData.password, userData.flagLogReg);
    switch (userData.flagLogReg)
    {
        case 1://Accesso Utente
            printf("richiesta di accesso utente");
            accessoUtente(sock,userData);
            break;
        case 2://Registrazione Utente
            registrazioneUtente(sock, userData);
            break;
        case 3://Invia Pacco
            inviaPacco(userData, sock);
            /*Riceve i dati del destinatario e recupera i dati del mittente;
              aggiunge data e ora di spedizione;
              inserisce i dati nel database nella sezione storico legata all'utente;
              invia flag per avvenuta spedizione o errore
            */
            break;
        case 4://Tracciamento Spedizione
            /*ricevo i dati della spedizione come il codice di tracciamento ed i dati utenti per avere tracciamento di chi ha richiesto l'operazione;
            effettua select nel db per cercare la spedizione con quel codice di tracciamento
            se esiste, invio i record sul tracciamento della spedizione, se no mando errore di codice non trovato*/
            trackSpedizione();
             break;
        case 5://Calcolo Tariffa
                /*Calcolo il codice della tariffa in base ai dati immessi dall'utente come peso, dimensioni del pacco;
                invio i dati al client*/
            calcoloTariffa();
            break;
        case 6://Visualizzazione Storico Spedizioni
            /*riceve la struct dell'utente connesso al server ed effettua la select sul db sui suoi dati per inviare i dati dello storico delle spedizione
            nel caso in cui  il numero di spedizione fosse maggiore di 0, invio i dati al client, altrimenti invio dato di nessuna spedizione effettuata*/    
            storico();
            break;

    }
    close(sock);
}