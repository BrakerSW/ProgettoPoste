struct user{
    char nome[50];
    char cognome[50];
    char username[50];
    char password[50];
    int flagOp;//variabile per tenere traccia di quale operazione è stata richiesta
    int flagOk;//variabile per l'avvenuta operazione o il fallimento
};
struct datiSpedizione
{
    char nomeDestinatario[50];
    char cognomeDestinatario[50];
    char via[50];
    char città[50];
    int  cap;
    char civico[4];
};
struct track
{
    char trackcode[4];
    int flag;
};
typedef struct track Tracking;
typedef struct datiSpedizione Destinatario;
typedef struct user User;
struct sockaddr_in server_addr;
void loginUtente(User userData,int sockLogin,struct sockaddr_in server_addr, char addressString[]);
void storicoUtente(User userData, int sockStorico,struct sockaddr_in server_addr, char addressString[]);
void registrazioneUtente(User userData, int sockRegistrazione, struct sockaddr_in server_addr, char addressString[]);
void invioPacco(User userData, int sockInvioPacco, struct sockaddr_in server_addr, char addressString[], Destinatario infoPacco, Tracking trackInfo);
void trackSpedizione(User userData, int sockTrackSpedizione, struct sockaddr_in server_addr, char addressString[]);
void calcoloTariffa(User userData, int sockCalcoloTariffa, struct sockaddr_in server_addr, char adressString[]);

