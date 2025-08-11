#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h> // Cabeçalho principal do Winsock

#define NUM_IDS 1000000

// --- Estrutura e Funções da Pilha ---
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

// --- Função para Embaralhar ---
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

// --- Lógica Principal do Servidor ---
int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup falhou: %d", WSAGetLastError());
        return 1;
    }

    srand(time(NULL));
    StackNode* id_stack = NULL;

    printf("Iniciando pré-geração de %d IDs...\n", NUM_IDS);
    int* ids = (int*)malloc(sizeof(int) * NUM_IDS);
    for (int i = 0; i < NUM_IDS; i++) { ids[i] = i + 1; }
    printf("Embaralhando IDs...\n");
    shuffle(ids, NUM_IDS);
    printf("Armazenando IDs na pilha...\n");
    for (int i = 0; i < NUM_IDS; i++) { push(&id_stack, ids[i]); }
    free(ids);
    printf("Servidor pronto! IDs empilhados.\n");

    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char response[64];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    printf("Servidor (Pilha) aguardando conexões na porta 8080...\n");

    while (1) {
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
    
    closesocket(server_fd);
    WSACleanup();
    return 0;
}
