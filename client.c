#include "common.h"
#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFSZ 500
#define MAX_CONNECTIONS 15

typedef struct {
	int idMessage;
    int idSender;
    int idReceiver;
    char* message;
} ParsedMessage;

int users[MAX_CONNECTIONS];

// Função para verificar se um elemento existe no array
bool userExists(int *array, int value) {
    for (int i = 0; array[i] != -1; i++) {
        if (array[i] == value) {
            return true; // O elemento existe no array
        }
    }
    return false; // O elemento não existe no array
}


// Função para inserir um inteiro no array
int insertElement(int *array, int capacity, int value) {
    int size = 0;
    while (array[size] != -1) {
        size++;
    }

    if (size == capacity) {
        // O array está cheio, não é possível inserir mais elementos
        return 0;
    }

    if (userExists(array, value)) {
        // O elemento já existe no array, não é necessário inserir novamente
        return 0;
    }

    array[size] = value;
    array[size + 1] = -1; // Definindo o novo valor sentinela
    return 1; // Inserção bem-sucedida
}

// Função para remover um inteiro do array
int removeElement(int *array, int value) {
    int found = 0; // Variável para indicar se o elemento foi encontrado e removido

    for (int i = 0; array[i] != -1; i++) {
        if (array[i] == value) {
            // O elemento foi encontrado no array, removendo...
            found = 1;
            // Movendo os elementos à direita do elemento removido para a esquerda
            int j;
            for (j = i; array[j] != -1; j++) {
                array[j] = array[j + 1];
            }
            array[j] = -1; // Definindo o novo valor sentinela
            break; // Não é necessário continuar procurando
        }
    }

    return found; // Se retornar 1, o elemento foi encontrado e removido, se retornar 0, o elemento não existe no array
}

int g_running = 1;
int disconnect = 0;

void usage(int argc, char **argv)
{
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

// Receber a resposta do server
char* receiveResponse(int sfd) {
    char buf[BUFSZ];
    char reader[BUFSZ];
    char* result = NULL; // Ponteiro para o array que será retornado
    size_t result_len = 0; // Tamanho atual do array retornado

    bzero(buf, BUFSZ); // Reinicializa o buffer
    bzero(reader, BUFSZ);

    size_t count = recv(sfd, reader, BUFSZ - 1, 0);
    getWholeMsg(sfd, reader, buf, count);

    if (strcmp(buf, "b") == 0) { // Múltiplos retornos são esperados
        while (1) {
            bzero(buf, BUFSZ);
            bzero(reader, BUFSZ);

            count = recv(sfd, reader, BUFSZ - 1, 0);
            getWholeMsg(sfd, reader, buf, count);

            if (strcmp(buf, "e") == 0) {
                // Quando encontrar "e", aloca memória suficiente para armazenar o resultado
                result = (char*)realloc(result, result_len + 1);
                result[result_len] = '\0'; // Garante que o array esteja terminado em null
                break;
            }

            //printf("< %s\n", buf);

            if (strcmp(buf, "error") == 0) {
                // Libera a memória alocada e retorna NULL em caso de erro
                free(result);
                return NULL;
            }

            // Aloca memória suficiente para armazenar o resultado atual
            result = (char*)realloc(result, result_len + strlen(buf) + 1);
            strcat(result, buf); // Concatena o resultado atual ao array de retorno
            result_len = strlen(result); // Atualiza o tamanho do array de retorno
        }
    } else {
        //printf("< %s\n", buf);
    }

    if (strcmp(buf, "error") == 0) {
        // Libera a memória alocada e retorna NULL em caso de erro
        free(result);
        return NULL;
    }

    // Se não houve múltiplos retornos, copia o conteúdo do buffer para o array de retorno
    result = (char*)realloc(result, strlen(buf) + 1);
    strcpy(result, buf);

    return result;
}

ParsedMessage* parseMessage(const char* formattedMsg) {
    // Verificando o início da mensagem formatada
    if (formattedMsg == NULL ) {
        printf("Mensagem formatada inválida!\n");
        return NULL;
    }

    ParsedMessage* parsedMsg = (ParsedMessage*)malloc(sizeof(ParsedMessage));

    if (parsedMsg == NULL) {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }

    // Usando sscanf para capturar os valores numéricos e a mensagem
    sscanf(formattedMsg, "%d ,%d, %d, %m[^\n]",&parsedMsg->idMessage, &parsedMsg->idSender, &parsedMsg->idReceiver, &parsedMsg->message);

    return parsedMsg;
}

void freeParsedMessage(ParsedMessage* parsedMsg) {
    if (parsedMsg == NULL) return;

    // Liberando a memória alocada para a mensagem
    free(parsedMsg->message);
    // Liberando a memória alocada para a estrutura
    free(parsedMsg);
}

void processParsedMessage(int csock,ParsedMessage* parsedMsg) {
    switch (parsedMsg->idMessage) {
        case 1:
            // Executar ação para idMessage == 1
            
			//Transformar a mensagem em um array de ids
			int ids[100];
			int num_ids = 0;
			char *token = strtok(parsedMsg->message, " ");
    		while (token != NULL) {
        	// Convertendo o token para inteiro e armazenando no array de IDs
        	ids[num_ids] = atoi(token);
        	num_ids++;
        	// Continuando a dividir a string em tokens
        	token = strtok(NULL, " ");
    		}
			
			//Inserir usuários
			for (int i = 0; i < num_ids; i++) {
				insertElement(users,MAX_CONNECTIONS,ids[i]);
			}

            break;
        case 2:
            // Executar ação para idMessage == 2

			int userId;

			//Remover usuario da base
    		if (sscanf(parsedMsg->message,"User %d left the group!", &userId) == 1) {
    		} else {
        		printf("Padrão não encontrado na mensagem.\n");
    		}		

			removeElement(users,userId);

			//print da mensagem
			printf("%s\n",parsedMsg->message);
           
            break;
        case 4:
            // Executar ação para idMessage == 4
            

			printf("%s\n",parsedMsg->message);


            break;
        case 6:
            // Executar ação para idMessage == 6
            

            // Registrar o usuário na rede
			if(parsedMsg->idSender != NULL){
				insertElement(users,MAX_CONNECTIONS,parsedMsg->idSender);

			}

			//print da mensagem
			printf("%s\n",parsedMsg->message);

            break;
        case 7:
            // Executar ação para idMessage == 7
            

			printf("%s\n",parsedMsg->message);

            break;
        case 8:
            // Executar ação para idMessage == 8
           

			//print da mensagem
			printf("%s\n",parsedMsg->message);

			//Desconectar usuario
			disconnect = 1;
			close(csock);
			exit(EXIT_SUCCESS);

            break;
        default:
            printf("idMessage desconhecido: %d\n", parsedMsg->idMessage);
            // Ação para idMessage desconhecido (ou qualquer outra ação padrão)
            break;
    }

    // Libera a memória alocada para a mensagem
    free(parsedMsg->message);
    parsedMsg->message = NULL;
}


void *msgHandler(int sfd) {

	
	char *contentServer;
	ParsedMessage* parsedMsg;
	contentServer = receiveResponse(sfd);

	if(contentServer == NULL){
		return NULL;
	}

	parsedMsg = parseMessage(contentServer);
	

	processParsedMessage(sfd,parsedMsg);


}

void *receiveMessages(void* arg) {
    int sfd = *(int*)arg;
    while (g_running) {
        // Recebe e processa as mensagens do servidor
        msgHandler(sfd); 
    }
    pthread_exit(NULL);
}



void runClient(int sfd)
{
    
	



	int command;
	char buf[BUFSZ];
	char *contentServer;
	pthread_t receive_thread;
	if (pthread_create(&receive_thread, NULL, receiveMessages, &sfd) != 0) {
        perror("Erro ao criar a thread de recepção de mensagens");
        return;
    }
	
	for (;;)
	{
		
		if (disconnect == 1) {
            g_running = 0; // Sinaliza a thread secundária para parar
            pthread_join(receive_thread, NULL); // Aguarda a finalização da thread de recepção
            break;
        }

		

		fgets(buf, BUFSZ - 1, stdin);

		//Formatar a entrada do fgets
		buf[strcspn(buf, "\n")] = '\0';

		command = handleCommand(buf);

		//Close connection
		if(command == 1){

			char *closeConnection = "close connection";
			
			send(sfd, closeConnection, strlen(closeConnection) + 1, 0);

		}
		//LIST USERS
		else if (command == 2){

			char *listUsers = "list users";
			
			send(sfd, listUsers, strlen(listUsers) + 1, 0);

		}
		//SEND TO
		else if(command == 3){
			

			//printf("tamanho de buf : %d",strlen(buf));

			send(sfd, buf, strlen(buf) + 1, 0);

			

		}//SEND ALL
		else if(command == 4){


			send(sfd, buf, strlen(buf) + 1, 0);

		}else {
			
		}

	};
}

int main(int argc, char **argv)
{

	//VERIFICAR PARAMETROS DO TERMINAL (QUANTIDADE)
	if (argc < 3)
	{
		usage(argc, argv);
	}

	struct sockaddr_storage storage;
	//VERIFICAR CORRETUDE DE PARAMETROS
	if (0 != addrparse(argv[1], argv[2], &storage))
	{
		usage(argc, argv);
	}

	int sockfd = socket(storage.ss_family, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		logexit("socket");
	}
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(sockfd, addr, sizeof(storage)))
	{
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	memset(users, -1, sizeof(users));

	
	
	runClient(sockfd);

	
	close(sockfd);
	exit(EXIT_SUCCESS);
}