#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sqlite3.h>
#include <time.h>
#include <string.h>
struct user{
    char nome[50];
    char cognome[50];
    char username[50];
    char password[50];
    int flagOp;//variabile per tenere traccia di quale operazione è stata richiesta
    int flagOk;//variabile per l'avvenuta operazione o il fallimento
};
typedef struct user User;
struct indirizzo{
    char via[50];
    char città[50];
    int  cap;
    char civico[4];

};
typedef struct indirizzo address;
struct datiSpedizione
{
    char nomeDestinatario[50];
    char cognomeDestinatario[50];
    char via[50];
    char città[50];
    int  cap;
    char civico[4];
    double pesoPacco;
    double altezza;
    double larghezza;
    double profondità;
};
struct track
{
    char trackcode[4];
    int flag;
};
typedef struct track Tracking;

typedef struct datiSpedizione destinatario;
typedef struct client Client;
pthread_mutex_t db_mutex= PTHREAD_MUTEX_INITIALIZER;
void *gestoreClient(void *arg);
void registrazioneUtente(int sock, User userData);
void accessoUtente(int sock, User userData);
void inviaPacco(User userData ,int sock);
void tracking_code_generate(char track_code[]);
void storico(User userData, int sock);


void *gestoreRichiesta(void* arg);
int main()
{
    srand(time(NULL));

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
        close(sock);
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
        userData.flagOk=1;
    }else
    {
        userData.flagOk=0;
    }

    send(sock,&userData.flagOk,sizeof(int),0);
    
    sqlite3_finalize(stm);
    sqlite3_close(db);
    close(sock);
    pthread_mutex_unlock(&db_mutex);
}
void inviaPacco(User userData ,int sock)
{
    destinatario infoPacco;
    Tracking trackInfo;
    sqlite3 *db;
    sqlite3_stmt *stm;
    sqlite3_open("database.db", &db);
    pthread_mutex_lock(&db_mutex);

    memset(&infoPacco,0,sizeof(infoPacco));
    
    int byte = recv(sock,&infoPacco,sizeof(infoPacco),0);
    if(byte!=sizeof(infoPacco))
    {
        printf("Errore in recv!\n");
        return;
    }
    char *querySelectPeso="SELECT prezzo FROM tariffe WHERE peso_max=?";
    char *queryInsert="INSERT INTO spedizioni (tracking_code,username_mittente, nome_destinatario, cognome_destinatario, via, cap, Città, data_spedizione, stato, Civico, peso ) VALUES (?,?,?,?,?,?,?,?,?,?,?)";
    char *querySelect="SELECT tracking_code FROM spedizioni WHERE tracking_code=?";
    int track=1;
    while(track){

        tracking_code_generate(trackInfo.trackcode);
        
        sqlite3_prepare_v2(db, querySelect, -1, &stm,NULL);

        sqlite3_bind_text(stm, 1,trackInfo.trackcode,-1, SQLITE_STATIC);
        /*LOGICA:  Se il codice generato casualmente esiste, imposta track = 1 
        e ricrea un nuovo codice. Nel caso in cui fosse diverso da quelli esistenti 
        nel database, va ad inserire la nuova spedizione*/
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
            
            sqlite3_bind_text(stm, 1, trackInfo.trackcode,-1, SQLITE_STATIC);
            sqlite3_bind_text(stm, 2, userData.username, -1,SQLITE_STATIC);
            sqlite3_bind_text(stm, 3, infoPacco.nomeDestinatario,-1,SQLITE_STATIC);
            sqlite3_bind_text(stm, 4, infoPacco.cognomeDestinatario, -1, SQLITE_STATIC);
            sqlite3_bind_text(stm, 5, infoPacco.via,-1,SQLITE_STATIC);
            sqlite3_bind_int(stm, 6, infoPacco.cap);
            sqlite3_bind_text(stm, 7, infoPacco.città,-1,SQLITE_STATIC);
            sqlite3_bind_text(stm, 8, time, -1, SQLITE_STATIC);
            sqlite3_bind_text(stm, 9, "In Preparazione",-1, SQLITE_STATIC);
            sqlite3_bind_text(stm, 10, infoPacco.civico, -1, SQLITE_STATIC);
            sqlite3_bind_double(stm, 11, infoPacco.pesoPacco);
            if(sqlite3_step(stm)==SQLITE_DONE)
            {
                trackInfo.flag=1;
            }else{
                trackInfo.flag=0;
            }
            send(sock,&trackInfo,sizeof(trackInfo),0);
            track=0;
        }
    }
    sqlite3_finalize(stm);
    sqlite3_close(db);
    pthread_mutex_unlock(&db_mutex);
    close(sock);
}
void tracking_code_generate(char track_code[])
{

    for(int i=0;i<4;i++)
    {
        track_code[i]='0'+rand()% 10;
    }

    track_code[4] = '\0';

}
void trackSpedizione(User userData, int sock)
{
    char *queryTrackSpedizione = "SELECT * FROM spedizioni WHERE tracking_code = ?";
    sqlite3 *db;
    sqlite3_stmt *stm;
    char track_code[5];
    recv(sock, track_code, sizeof(track_code) - 1, 0);
    track_code[4] = '\0';
    
    pthread_mutex_lock(&db_mutex);
    
    sqlite3_open("database.db",&db);

    sqlite3_prepare_v2(db, queryTrackSpedizione, -1,&stm, NULL);

    sqlite3_bind_text(stm, 1,track_code, -1, SQLITE_STATIC );

    if(sqlite3_step(stm)==SQLITE_ROW)
    {
        userData.flagOk=1;
        char *stato_spedizione = (char*) sqlite3_column_text(stm,8);  
        char buffer[256];
        
        send(sock,&userData.flagOk, sizeof(userData.flagOk),0);

        snprintf(buffer,sizeof(buffer), "Stato della spedizione: %s\n",stato_spedizione);
        send(sock, buffer, sizeof(buffer), 0);

     }else{
        userData.flagOk=0;
        send(sock, &userData.flagOk, sizeof(userData.flagOk),0);
    }
    sqlite3_finalize(stm);
    sqlite3_close(db);
    close(sock);
    pthread_mutex_unlock(&db_mutex);
}
void calcoloTariffa(int sock)
{
    sqlite3 *db;
    sqlite3_stmt *stm;
    destinatario infoPacco;
    recv(sock,&infoPacco, sizeof(infoPacco),0);
    pthread_mutex_lock(&db_mutex);
    char *querySelect = "SELECT prezzo FROM tariffe WHERE peso_max>=? AND larghezza_max >=? AND altezza_max>=? AND profondita_max >=?";
    sqlite3_open("database.db", &db);
    int rc = sqlite3_prepare_v2(db, querySelect, -1, &stm, NULL);

    sqlite3_bind_double(stm, 1, infoPacco.pesoPacco);
    sqlite3_bind_double(stm, 2, infoPacco.larghezza);
    sqlite3_bind_double(stm, 3, infoPacco.altezza);
    sqlite3_bind_double(stm, 4, infoPacco.profondità);
   
    if(sqlite3_step(stm)==SQLITE_ROW)
    {
        double prezzoSpedizione = sqlite3_column_double(stm,0);
        printf("%lf",prezzoSpedizione);
        send(sock,&prezzoSpedizione,sizeof(prezzoSpedizione),0);
    }else{
        printf("Errore nel calcolo della tariffa");
    }
    sqlite3_finalize(stm);
    sqlite3_close(db);
    close(sock);
    pthread_mutex_unlock(&db_mutex);
}
void storico(User userData, int sock)
{
    pthread_mutex_lock(&db_mutex);

    char *queryStorico ="SELECT tracking_code, nome_destinatario, cognome_destinatario, via,cap, Città, Civico, data_spedizione FROM spedizioni WHERE username_mittente = ?";
    sqlite3 *db;
    sqlite3_stmt *stm;
    sqlite3_open("database.db", &db);

    sqlite3_prepare_v2(db, queryStorico,-1,&stm,NULL);
    sqlite3_bind_text(stm, 1, userData.username,-1, SQLITE_STATIC);

    while(sqlite3_step(stm)==SQLITE_ROW)
    {
        char *tracking_code = (char*) sqlite3_column_text(stm,0);
        char *nome_destinatario = (char*) sqlite3_column_text(stm,1);
        char *cognome_destinatario = (char*) sqlite3_column_text(stm, 2);
        char *via = (char*) sqlite3_column_text(stm,3);
        int cap = (int) sqlite3_column_int(stm,4);
        char *città = (char*) sqlite3_column_text(stm,5);
        char *Civico = (char*) sqlite3_column_text(stm,6);
        char *data_spedizione = (char*) sqlite3_column_text(stm,7);
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "STORICO|%s|%s|%s|%s|%d|%s|%s|%s|\n",tracking_code,nome_destinatario,cognome_destinatario,via,cap,città,Civico,data_spedizione);
        send(sock,buffer, strlen(buffer),0);
    }

    send(sock, "END",sizeof("END"),0);

    sqlite3_finalize(stm);
    sqlite3_close(db);
    close(sock);
    pthread_mutex_unlock(&db_mutex);
}
void *gestoreRichiesta(void* arg)
{
    int sock= *(int*)arg;

    User userData;
    free(arg);
    
    recv(sock, &userData,sizeof(userData),0);
    switch (userData.flagOp)
    {
        
        case 1://Accesso Utente
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
            trackSpedizione(userData, sock);
             break;

        case 5://Calcolo Tariffa
            printf("scelto calcolo tariffa!\n");
                /*Calcolo il codice della tariffa in base ai dati immessi dall'utente come peso, dimensioni del pacco;
                invio i dati al client*/
            calcoloTariffa(sock);
            break;

        case 6://Visualizzazione Storico Spedizioni
            /*riceve la struct dell'utente connesso al server ed effettua la select sul db sui suoi dati per inviare i dati dello storico delle spedizione
            nel caso in cui  il numero di spedizione fosse maggiore di 0, invio i dati al client, altrimenti invio dato di nessuna spedizione effettuata*/                
            storico(userData,sock);
            break;
    }
    close(sock);
}