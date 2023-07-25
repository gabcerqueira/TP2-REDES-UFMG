#include "common.h"
#include "handlers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>

#define BUFSZ 500
#define MAX_CONNECTIONS 15


typedef struct{
    int id;
    bool connected;
    int csock;
} User;


typedef struct {
    int csock;
    User *usersPtr; 
} ThreadArgs;


User *users[MAX_CONNECTIONS];


void initializeUserArray() {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        users[i] = (User *)malloc(sizeof(User));
        users[i]->id = i + 1;
        users[i]->connected = false;
        users[i]->csock = -1;
    }
}


//Passagens de parametros de entrada
void usage(int argc, char **argv)
{
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void sendResponse(int csock, char *msg)
{
    size_t msgSize = strlen(msg);
    char content[msgSize + 4];

    sprintf(content, "%ld-%s", msgSize, msg);
    size_t count = send(csock, content, strlen(content) + 1, 0);
    if (count != strlen(content) + 1)
    {
        logexit("error while sending message to client");
    }
}



User *findFirstDisconnectedUser(User **usersPtr) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (!usersPtr[i]->connected) {
            return usersPtr[i];
        }
    }
    return NULL;
}

User *findUserByCsock(int csock, User **usersPtr) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (usersPtr[i] != NULL && usersPtr[i]->csock == csock) {
            return usersPtr[i];
        }
    }
    return NULL;
}

User *findUserById(int userID, User **usersPtr) {
    if (userID < 1 || userID > MAX_CONNECTIONS) {
        return NULL; // ID inválido, retorna NULL
    }

    int index = userID - 1;

    if (usersPtr[index] != NULL) {
        return usersPtr[index]; // Retorna o usuário se ele existe
    }

    return NULL; // Usuário não encontrado
}


void finalizeUserByCsock(int csock, User **usersPtr) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (usersPtr[i]->csock == csock) {
            usersPtr[i]->csock = -1;
            usersPtr[i]->connected = false;
            return;
        }
    }
}

char* formatMessage(int idMessage, int idSender, int idReceiver, const char* message) {
    int bufferSize = 18 + snprintf(NULL, 0, "%d", idMessage) + snprintf(NULL, 0, "%d", idSender) + snprintf(NULL, 0, "%d", idReceiver) + strlen(message) + 1;
    char* formattedMsg = (char*)malloc(bufferSize * sizeof(char));

    if (formattedMsg == NULL) {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }

    // Formata a mensagem na string alocada
    snprintf(formattedMsg, bufferSize, "%d, %d, %d, %s", idMessage, idSender, idReceiver, message);

    return formattedMsg;
}




void broadcast(char *msg, int sendToUserID) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (users[i]->connected && i + 1 != sendToUserID) {
            sendResponse(users[i]->csock, msg);
        }
    }
}


void unicast(int csock,char *msg){

    sendResponse(csock,msg);

}

char* getMessageInQuotes(const char* inputString) {
    const char* startQuote = strchr(inputString, '"'); 
    if (startQuote == NULL) {
        return NULL; 
    }

    const char* endQuote = strchr(startQuote + 1, '"'); 
    if (endQuote == NULL) {
        return NULL; // Retorna NULL se não encontrar a aspa fechada
    }

    size_t messageLength = endQuote - (startQuote + 1); 
    char* message = (char*)malloc((messageLength + 1) * sizeof(char)); 
    if (message == NULL) {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }

    strncpy(message, startQuote + 1, messageLength); 
    message[messageLength] = '\0'; 

    return message;
}

// Função para obter o horário atual no formato "[hh:ss]"
char* getCurrentTime() {
    time_t now;
    struct tm *timeinfo;
    char *timeStr = (char*)malloc(8 * sizeof(char)); 
    if (timeStr == NULL) {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }

    time(&now);
    timeinfo = localtime(&now);
    snprintf(timeStr, 8, "[%02d:%02d]", timeinfo->tm_hour, timeinfo->tm_min);
    return timeStr;
}

char* listConnectedUsers(User **usersPtr,int ownUserID,bool considerOwnId) {
    char* connectedUsersMsg = (char*)malloc((3 * MAX_CONNECTIONS + 1) * sizeof(char));
    if (connectedUsersMsg == NULL) {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }

    int connectedCount = 0;
    int index = 0;

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        
        
        if(!considerOwnId){
            if (usersPtr[i]->connected && usersPtr[i]->id != ownUserID) {
            connectedCount++;
            // Escreve o número formatado na string
            index += snprintf(&connectedUsersMsg[index], 4, "%02d ", i + 1);
            }
        }
        
        
        
        else{
            if (usersPtr[i]->connected) {
            connectedCount++;
            // Escreve o número formatado na string
            index += snprintf(&connectedUsersMsg[index], 4, "%02d ", i + 1);
            }

        }
        
    }

    if (connectedCount == 0) {
        strcpy(connectedUsersMsg, "Nenhum usuário conectado.");
    }

    return connectedUsersMsg;
}

int listenToClient(int csock,User *usersPtr)
{
    
    char buf[BUFSZ];
    char reader[BUFSZ];
    int exitCode = 0;
    char response[BUFSZ];


    

    for (;;)
    {
        
        bzero(buf, BUFSZ);
        bzero(reader, BUFSZ);
  
       while(1){

        size_t count = recv(csock, reader, BUFSZ - 1, 0);
        strcpy(buf,reader);
        bzero(response,BUFSZ);

        if(count != 0){
            
            break;

        }

       }

        // FLUXO DE DESCONEXÃO DO USUARIO
         if (strcmp(buf, "close connection") == 0){

            //verificar se idUser existe na base de dados
            User *user = findUserByCsock(csock,usersPtr);

            if(user == NULL){

                //Enviar mensagem de erro
                char *errorMsg = "User not found";
                unicast(csock,formatMessage(07,0,0,errorMsg));
                break;
            }

            //Remover usuario da base de dados
            finalizeUserByCsock(csock,usersPtr);



            //print de remoção no server
            printf("User %02d removed\n",user->id);



            //Responder ao userI OK(01)
            char *okMsg = "Removed Successfully";
            unicast(csock,formatMessage(8,0,0,okMsg));

            char *broadMsg = (char *)malloc((2 + 23) * sizeof(char));
            //Broadcast da saida de usuario
            sprintf(broadMsg, "User %02d left the group!", user->id);
           
            broadcast(formatMessage(02,user->id,0,broadMsg),0);
            
            free(broadMsg);
        }

        // FLUXO DE RES_LIST LISTAR USUARIOS
        else if(strcmp(buf, "list users") == 0){

            User *user = findUserByCsock(csock,usersPtr);

            if(user == NULL){

                //Enviar mensagem de erro
                char *errorMsg = "User not found";
                unicast(csock,formatMessage(07,0,0,errorMsg));
                break;
            }

             unicast(user->csock,formatMessage(04,0,0,listConnectedUsers(usersPtr,user->id,false)));

        }

        else if(strstr(buf, "send to") != NULL){

            User *user = findUserByCsock(csock,usersPtr);

            char *msg = getMessageInQuotes(buf);

            //Recuperar o id
            int id;
            sscanf(buf, "send to %d", &id);


            //Recuperar o horario
            char *currentTime = getCurrentTime();

            char* formattedMessage = (char*)malloc((14 + strlen(currentTime) + strlen(msg)) * sizeof(char));
            if (formattedMessage == NULL) {
                 perror("Erro ao alocar memória");
                exit(EXIT_FAILURE);
            }

            sprintf(formattedMessage, "P %s -> %02d: %s", currentTime, user->id, msg);


            //Recuperar o socket pelo id
             User *foundUser = findUserById(id, usersPtr);

             

            if (foundUser != NULL && foundUser->connected) {
                //Envio da mensagem privada
                unicast(foundUser->csock,formatMessage(07,user->id,id,formattedMessage));
                sprintf(formattedMessage, "P %s -> %02d: %s", currentTime, id, msg);
                unicast(user->csock,formatMessage(07,user->id,0,formattedMessage));

            } else {
                //USER NOT FOUND
                printf("User %02d not found\n",id);

                char *receiverMsg = "Receiver not found";
                unicast(user->csock, formatMessage(07, user->id, 0, receiverMsg));
 
            }



            free(currentTime);
            free(formattedMessage);


        }

        else if(strstr(buf, "send all") != NULL){

            User *user = findUserByCsock(csock,usersPtr);
            
			

			char *msg = getMessageInQuotes(buf);
            
            // Obtém o horário atual
            char *currentTime = getCurrentTime();
			
            


            //formatar mensagem para broadcast
            char *formattedMessage = (char*)malloc((strlen(currentTime) + 12 + strlen(msg)) * sizeof(char));
            if (formattedMessage == NULL) {
                perror("Erro ao alocar memória");
                exit(EXIT_FAILURE);
            }

            snprintf(formattedMessage, strlen(currentTime) + 12 + strlen(msg), "%s %02d: %s", currentTime, user->id, msg);
            
            
            
            broadcast(formatMessage(07,user->id,0,formattedMessage),user->id);

			printf("%s\n",formattedMessage);
            

            //formatar mensagem para unicast
            snprintf(formattedMessage, strlen(currentTime) + 12 + strlen(msg), "%s -> all: %s", currentTime,msg);

            unicast(user->csock,formatMessage(07,user->id,0,formattedMessage));

            
            free(currentTime);
            free(formattedMessage);
        }
        else {
            
        }

       
    }

    return exitCode;
}


void *connectionHandler(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int csock = args->csock;

    
    //Criação de usuário
    User *user = findFirstDisconnectedUser(args->usersPtr);
    if (user == NULL) {
        printf("User limit exceeded\n");
        close(csock);
        free(args);
        pthread_exit((void *)(intptr_t)1);
        return NULL;
    }

    user->csock = csock;
    user->connected = true;


    //Tratamento do broadcast de criação de usuario
    char message[50];
    sprintf(message, "User %02d joined the group!", user->id);
    broadcast(formatMessage(06,user->id,0,message),0);

    printf("User %02d added\n", user->id);

    //Tratamento do unicast de criação de usuario
    unicast(user->csock,formatMessage(01,0,0,listConnectedUsers(args->usersPtr,user->id,true)));

    
    int exitCode = listenToClient(csock,args->usersPtr);
    

    finalizeUserByCsock(csock,args->usersPtr);
    close(csock);
    free(args);
    pthread_exit((void *)(intptr_t)exitCode);
    return NULL;
}

int main(int argc, char **argv)
{

    initializeUserArray();
    User **usersPtr = users;


    if (argc < 3)
    {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage))
    {
        usage(argc, argv);
    }

    int serversock = socket(storage.ss_family, SOCK_STREAM, 0);
    if (serversock == -1)
    {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
    {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(serversock, addr, sizeof(storage)))
    {
        logexit("bind");
    }

    if (0 != listen(serversock, MAX_CONNECTIONS))
    {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    pthread_t threads[MAX_CONNECTIONS];
    int threadCount = 0;

    
    
    while (1)
    {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(serversock, caddr, &caddrlen);
        if (csock == -1)
        {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        //printf("[log] connection from %s\n", caddrstr);
        
         if (threadCount < MAX_CONNECTIONS) {
            // Criação de nova thread para lidar com a conexão do cliente
            ThreadArgs *args = (ThreadArgs *)malloc(sizeof(ThreadArgs));
            args->csock = csock;

            args->usersPtr = usersPtr;
            


            if (pthread_create(&threads[threadCount], NULL, connectionHandler, args) != 0) {
                perror("Erro ao criar a thread");
                close(csock);
                free(args);
            } else {
                threadCount++;
            }
        } else {
            // Limite máximo de conexões atingido
            
            
            //Enviar a mensagem para o usuário
            char *limitExceededMsg = "User limit exceeded";
            unicast(csock,formatMessage(8,0,0,limitExceededMsg));

            close(csock);
        }
        
        
    }

    printf("Shutting down server...\n");
    close(serversock);
    exit(EXIT_SUCCESS);
}


