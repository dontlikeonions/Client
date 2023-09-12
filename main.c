#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define FALSE 0
#define TRUE 1
#define EXIT 2

#define SOC_PATH "../../OpSy1/socket"
#define BUFFER_SIZE 1024

#define CMD_EXIT "exit\n"



int create_soc(int* m_socket) {
    printf("Creating socket...\n");
    *m_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_socket < 0) {
        perror("Server: Create");
        return FALSE;
    }
    return TRUE;
}

int connect_soc(const int* m_socket, struct sockaddr_un *m_addr) {
    printf("Connecting to server...\n");
    if (connect(*m_socket,(struct sockaddr*)(m_addr),sizeof(*m_addr)) < 0) {
        return FALSE;
    }
    return TRUE;
}

int establishConnection(int* m_socket) {
    struct sockaddr_un m_addr;
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    strcpy (m_addr.sun_path, SOC_PATH);
    memcpy(m_addr.sun_path, SOC_PATH, sizeof(m_addr.sun_path));

    if (create_soc(m_socket) == FALSE) {
        return FALSE;
    }

    while (connect_soc(m_socket, &m_addr) == FALSE) {
        perror("Client: Connect");
        sleep(3);
        printf("Trying to reconnect...\n");
    }

    printf("Connection established!\n");
    return TRUE;
}

int send_request(const int* m_socket) {
    char input[BUFFER_SIZE];
    fgets(input, BUFFER_SIZE, stdin);

    int res = send(*m_socket, input, BUFFER_SIZE, MSG_NOSIGNAL);
    if (res == -1 || errno == EPIPE) {
        perror("Client: Send");
        return FALSE;
    }

    if (strcmp(input, CMD_EXIT) == 0) {
        return EXIT;
    }
    return TRUE;
}

int get_response(const int* m_socket) {
    char server_answer[BUFFER_SIZE];
    int res = read(*m_socket, server_answer, BUFFER_SIZE);
    if (res < 0) {
        perror("Reading server answer: ");
        return FALSE;
    }
    else {
        printf(server_answer);
    }
}

int main() {
    int m_socket = -1;
    int res = establishConnection(&m_socket);
    if (res == FALSE) {
        perror("Establishing connection: ");
        return 1;
    }

    while (TRUE) {
        res = send_request(&m_socket);
        if (res == TRUE) {
            get_response(&m_socket);
        }
        else if (res == EXIT) {
            break;
        }
    }

    close(m_socket);
    return TRUE;
}
