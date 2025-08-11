#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h> // Cabeçalho principal do Winsock

// --- Estrutura e Funções da Lista Encadeada ---
typedef struct Node {
    int id;
    struct Node* next;
} Node;

Node* head = NULL;

int exists(int id) {
    Node* current = head;
    while (current != NULL) {
        if (current->id == id) return 1;
        current = current->next;
    }
    return 0;
}

void append(int id) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->id = id;
    new_node->next = NULL;
    if (head == NULL) {
        head = new_node;
        return;
    }
    Node* last = head;
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = new_node;
}

int generate_unique_id() {
    int new_id;
    do {
        new_id = rand();
    } while (exists(new_id));
    append(new_id);
    return new_id;
}

// --- Lógica Principal do Servidor ---
int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup falhou: %d", WSAGetLastError());
        return 1;
    }

    srand(time(NULL));

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

    printf("Servidor (Lista Encadeada) aguardando conexões na porta 8080...\n");

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket == INVALID_SOCKET) continue;

        memset(buffer, 0, sizeof(buffer));
        recv(new_socket, buffer, 1024, 0);

        if (strncmp(buffer, "GET_ID", 6) == 0) {
            int new_id = generate_unique_id();
            sprintf(response, "%d", new_id);
            send(new_socket, response, strlen(response), 0);
        }
        closesocket(new_socket);
    }
    
    closesocket(server_fd);
    WSACleanup();
    return 0;
}
