#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h> // Cabeçalho principal do Winsock

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int main(int argc, char *argv[]) {
    // --- INICIALIZAÇÃO DO WINSOCK ---
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Falha ao inicializar o Winsock: %d", WSAGetLastError());
        return 1;
    }

    if (argc != 2) {
        printf("Uso: %s <tempo_em_segundos>\n", argv[0]);
        return 1;
    }
    int duration = atoi(argv[1]);

    char message[] = "GET_ID";
    char server_reply[2000];
    int ids_received = 0;

    time_t start, now;
    start = time(NULL);

    while (1) {
        now = time(NULL);
        if (difftime(now, start) >= duration)
            break;

        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            // Não imprimir erro aqui para não poluir o benchmark
            break;
        }
        struct sockaddr_in server;
        server.sin_addr.s_addr = inet_addr(SERVER_IP);
        server.sin_family = AF_INET;
        server.sin_port = htons(SERVER_PORT);

        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
            closesocket(sock);
            break;
        }

        if (send(sock, message, strlen(message), 0) < 0) {
            closesocket(sock);
            break;
        }

        memset(server_reply, 0, sizeof(server_reply));
        if (recv(sock, server_reply, 2000, 0) > 0) {
            ids_received++;
        }
        closesocket(sock); // Usando a função correta
    }
    printf("Total de identidades recebidas em %ds: %d\n", duration, ids_received);
    
    WSACleanup(); // Limpa a biblioteca Winsock
    return 0;
}
