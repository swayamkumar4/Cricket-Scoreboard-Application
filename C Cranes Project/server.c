#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 4096

// Admin credentials (in a real application, these should be stored securely)
#define ADMIN_USERNAME "admin"
#define ADMIN_PASSWORD "admin123"

// Base64 decoding table
static const unsigned char base64_table[256] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

// Base64 decode function
int base64_decode(const char *in, size_t inlen, char *out) {
    int i, j;
    unsigned char a[4], b[4];
    size_t k = 0;
    int outlen = 0;

    for (i = 0, j = 0; i < inlen; i++) {
        if (in[i] == '=' || base64_table[(unsigned char)in[i]] == 64)
            continue; // Skip padding and invalid characters
        
        a[k++] = in[i];
        if (k == 4) {
            for (k = 0; k < 4; k++)
                b[k] = base64_table[(unsigned char)a[k]];
            
            out[j++] = (b[0] << 2) | (b[1] >> 4);
            out[j++] = (b[1] << 4) | (b[2] >> 2);
            out[j++] = (b[2] << 6) | b[3];
            k = 0;
        }
    }

    if (k) {
        for (i = k; i < 4; i++)
            a[i] = '=';
        for (i = 0; i < 4; i++)
            b[i] = base64_table[(unsigned char)a[i]];
        
        out[j++] = (b[0] << 2) | (b[1] >> 4);
        if (a[2] != '=')
            out[j++] = (b[1] << 4) | (b[2] >> 2);
        if (a[3] != '=')
            out[j++] = (b[2] << 6) | b[3];
    }
    
    out[j] = '\0';
    return j;
}

typedef struct {
    char name[50];
    int runs;
    int wickets;
    float overs;
    int extras;
} Team;

Team team1 = {"Team 1", 0, 0, 0.0, 0};
Team team2 = {"Team 2", 0, 0, 0.0, 0};

void debug_print_request(const char* buffer) {
    printf("\n=== Incoming Request ===\n");
    char* first_line = strdup(buffer);
    char* end = strchr(first_line, '\r');
    if (end) *end = '\0';
    printf("Request: %s\n", first_line);
    free(first_line);

    char* auth = strstr(buffer, "Authorization: Basic");
    if (auth) {
        printf("Has Authorization header: Yes\n");
    } else {
        printf("Has Authorization header: No\n");
    }
}

void send_cors_headers(SOCKET client_socket) {
    char response[1024];
    sprintf(response, 
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
            "Content-Length: 0\r\n"
            "\r\n");
    send(client_socket, response, strlen(response), 0);
}

void send_response(SOCKET client_socket, const char* content_type, const char* content) {
    char response[BUFFER_SIZE];
    sprintf(response, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
            "Content-Length: %ld\r\n"
            "\r\n"
            "%s", content_type, strlen(content), content);
    send(client_socket, response, strlen(response), 0);
    printf("Response sent: %s\n", content_type);
}

void send_error_response(SOCKET client_socket, const char* error_message) {
    char json_response[BUFFER_SIZE];
    sprintf(json_response, "{\"error\": \"%s\", \"isAdmin\": false}", error_message);
    
    char response[BUFFER_SIZE];
    sprintf(response, 
            "HTTP/1.1 401 Unauthorized\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
            "Content-Length: %ld\r\n"
            "\r\n"
            "%s", strlen(json_response), json_response);
    send(client_socket, response, strlen(response), 0);
    printf("Error response sent: %s\n", error_message);
}

void handle_get_scores(SOCKET client_socket) {
    char json[512];
    sprintf(json, 
            "{"
            "\"team1\": {\"name\": \"%s\", \"runs\": %d, \"wickets\": %d, \"overs\": %.1f, \"extras\": %d},"
            "\"team2\": {\"name\": \"%s\", \"runs\": %d, \"wickets\": %d, \"overs\": %.1f, \"extras\": %d}"
            "}",
            team1.name, team1.runs, team1.wickets, team1.overs, team1.extras,
            team2.name, team2.runs, team2.wickets, team2.overs, team2.extras);
    send_response(client_socket, "application/json", json);
}

void handle_post_scores(SOCKET client_socket, const char* buffer) {
    // Find Authorization header
    char* auth_header = strstr(buffer, "Authorization: Basic ");
    if (!auth_header) {
        send_error_response(client_socket, "Unauthorized access");
        return;
    }

    // Move pointer to the start of the base64 string
    auth_header += 21;
    
    // Find the end of the base64 string
    char* end = auth_header;
    while (*end && *end != '\r' && *end != '\n' && *end != ' ') {
        end++;
    }
    
    // Extract just the base64 part
    char base64_creds[256] = {0};
    int len = end - auth_header;
    if (len > 0 && len < 256) {
        strncpy(base64_creds, auth_header, len);
        base64_creds[len] = '\0';
        
        // Check if credentials match admin
        if (strcmp(base64_creds, "YWRtaW46YWRtaW4xMjM=") == 0) {
            // Find the JSON body
            char* body = strstr(buffer, "\r\n\r\n");
            if (body) {
                body += 4;  // Skip \r\n\r\n
                // In a real application, properly parse JSON and update scores
                send_response(client_socket, "application/json", "{\"status\": \"success\"}");
            } else {
                send_error_response(client_socket, "Invalid request");
            }
            return;
        }
    }
    
    send_error_response(client_socket, "Unauthorized access");
}

void handle_auth(SOCKET client_socket, const char* buffer) {
    printf("\nHandling auth request...\n");
    
    // Handle OPTIONS request
    if (strstr(buffer, "OPTIONS")) {
        send_cors_headers(client_socket);
        return;
    }
    
    // Properly extract the Authorization header
    char* auth_header = strstr(buffer, "Authorization: Basic ");
    if (auth_header) {
        // Move pointer to the start of the base64 string (after "Authorization: Basic ")
        auth_header += 21;
        
        // Find the end of the base64 string (at the next CR or space)
        char* end = auth_header;
        while (*end && *end != '\r' && *end != '\n' && *end != ' ') {
            end++;
        }
        
        // Null-terminate to extract just the base64 part
        char base64_creds[256] = {0};
        int len = end - auth_header;
        if (len > 0 && len < 256) {
            strncpy(base64_creds, auth_header, len);
            base64_creds[len] = '\0';
            
            printf("Extracted base64: '%s'\n", base64_creds);
            
            // Decode credentials
            char decoded[256] = {0};
            int decoded_len = 0;
            
            // Simple base64 decode for "admin:admin123"
            if (strcmp(base64_creds, "YWRtaW46YWRtaW4xMjM=") == 0) {
                strcpy(decoded, "admin:admin123");
                decoded_len = strlen(decoded);
                
                printf("Decoded credentials: '%s'\n", decoded);
                
                // Admin authenticated
                send_response(client_socket, "application/json", 
                            "{\"status\": \"success\", \"isAdmin\": true}");
                return;
            }
        }
        
        send_error_response(client_socket, "Invalid credentials");
    } else {
        send_error_response(client_socket, "No authorization provided");
    }
}

void serve_file(SOCKET client_socket, const char* filename, const char* content_type) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        send_response(client_socket, "text/plain", "404 Not Found");
        return;
    }

    char* buffer = malloc(BUFFER_SIZE);
    size_t bytes_read = fread(buffer, 1, BUFFER_SIZE - 1, file);
    buffer[bytes_read] = '\0';
    fclose(file);

    send_response(client_socket, content_type, buffer);
    free(buffer);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Allow socket reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        printf("Setsockopt failed\n");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("Bind failed\n");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (listen(server_fd, 3) == SOCKET_ERROR) {
        printf("Listen failed\n");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Server running on port %d\n", PORT);

    while (1) {
        int addrlen = sizeof(address);
        SOCKET client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_socket == INVALID_SOCKET) {
            printf("Accept failed\n");
            continue;
        }

        char buffer[BUFFER_SIZE] = {0};
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        debug_print_request(buffer);

        if (strstr(buffer, "OPTIONS")) {
            send_cors_headers(client_socket);
        }
        else if (strncmp(buffer, "GET /api/scores", 14) == 0) {
            handle_get_scores(client_socket);
        }
        else if (strncmp(buffer, "POST /api/scores", 15) == 0) {
            handle_post_scores(client_socket, buffer);
        }
        else if (strncmp(buffer, "POST /api/auth", 13) == 0) {
            handle_auth(client_socket, buffer);
        }
        else if (strncmp(buffer, "GET / ", 6) == 0 || strncmp(buffer, "GET /index.html", 14) == 0) {
            serve_file(client_socket, "index.html", "text/html");
        }
        else if (strncmp(buffer, "GET /styles.css", 14) == 0) {
            serve_file(client_socket, "styles.css", "text/css");
        }
        else if (strncmp(buffer, "GET /script.js", 13) == 0) {
            serve_file(client_socket, "script.js", "text/javascript");
        }
        else if (strncmp(buffer, "GET /login.html", 14) == 0) {
            serve_file(client_socket, "login.html", "text/html");
        }

        closesocket(client_socket);
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
} 