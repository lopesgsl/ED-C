#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define NUM_IDS 1000000

volatile int stop_server = 0; // flag para encerrar servidor

// ==========================
// Estrutura da Pilha
// ==========================
typedef struct StackNode {
    int id;
    struct StackNode* next;
} StackNode;

int isEmpty(StackNode* root) { return !root; }

void push(StackNode** root, int id) {
    StackNode* stackNode = (StackNode*)malloc(sizeof(StackNode));
    stackNode->id = id;
    stackNode->next = *root;
    *root = stackNode;
}

int pop(StackNode** root) {
    if (isEmpty(*root)) return -1;
    StackNode* temp = *root;
    *root = (*root)->next;
    int popped = temp->id;
    free(temp);
    return popped;
}

// ==========================
// Fun��o para embaralhar
// ==========================
void shuffle(int* array, size_t n) {
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

// ==========================
// Thread do Servidor
// ==========================
unsigned __stdcall server_thread(void* arg) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup falhou: %d\n", WSAGetLastError());
        return 1;
    }

    srand(time(NULL));
    StackNode* id_stack = NULL;

    printf("Servidor: Gerando %d IDs...\n", NUM_IDS);
    int* ids = (int*)malloc(sizeof(int) * NUM_IDS);
    for (int i = 0; i < NUM_IDS; i++) { ids[i] = i + 1; }
    shuffle(ids, NUM_IDS);
    for (int i = 0; i < NUM_IDS; i++) { push(&id_stack, ids[i]); }
    free(ids);

    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char response[64];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    printf("Servidor (Pilha) rodando na porta %d...\n", SERVER_PORT);

    while (!stop_server) {
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int activity = select(0, &readfds, NULL, NULL, &tv);
        if (activity > 0 && FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (new_socket == INVALID_SOCKET) continue;

            memset(buffer, 0, sizeof(buffer));
            recv(new_socket, buffer, 1024, 0);

            if (strncmp(buffer, "GET_ID", 6) == 0) {
                int new_id = pop(&id_stack);
                if (new_id != -1) {
                    sprintf(response, "%d", new_id);
                    send(new_socket, response, strlen(response), 0);
                } else {
                    send(new_socket, "NO_IDS_LEFT", 11, 0);
                }
            }
            closesocket(new_socket);
        }
    }

    closesocket(server_fd);
    WSACleanup();
    printf("Servidor encerrado.\n");
    return 0;
}

// ==========================
// Thread do Cliente
// ==========================
unsigned __stdcall client_thread(void* arg) {
    int duration = *(int*)arg;
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Cliente: Falha ao inicializar Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    char message[] = "GET_ID";
    char server_reply[2000];
    int ids_received = 0;

    time_t start = time(NULL);

    while (1) {
        if (difftime(time(NULL), start) >= duration) break;

        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) break;

        struct sockaddr_in server;
        server.sin_addr.s_addr = inet_addr(SERVER_IP);
        server.sin_family = AF_INET;
        server.sin_port = htons(SERVER_PORT);

        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
            closesocket(sock);
            break;
        }

        send(sock, message, strlen(message), 0);
        memset(server_reply, 0, sizeof(server_reply));
        if (recv(sock, server_reply, sizeof(server_reply), 0) > 0) {
            ids_received++;
        }
        closesocket(sock);
    }

    printf("Cliente: Total de IDs recebidos em %ds: %d\n", duration, ids_received);
    stop_server = 1; // sinal para encerrar o servidor
    WSACleanup();
    return 0;
}

// ==========================
// Fun��o principal
// ==========================
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <tempo_em_segundos>\n", argv[0]);
        return 1;
    }
    int duration = atoi(argv[1]);

    HANDLE hServer = (HANDLE)_beginthreadex(NULL, 0, server_thread, NULL, 0, NULL);
    Sleep(1000); // d� tempo para o servidor iniciar
    HANDLE hClient = (HANDLE)_beginthreadex(NULL, 0, client_thread, &duration, 0, NULL);

    WaitForSingleObject(hClient, INFINITE);
    WaitForSingleObject(hServer, INFINITE);

    CloseHandle(hClient);
    CloseHandle(hServer);

    return 0;
}

