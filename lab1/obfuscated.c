#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <libgen.h>
#include <regex.h>
#if __APPLE__
#include <libproc.h>
#endif

#define MAX_BUFFER_SIZE (1 << 20)
const int MAX_USERNAME_LENGTH = 256;
const int MAX_REPO_LENGTH = 32;
const int MAX_MESSAGE_LENGTH = 1024;
const int MAX_URL_LENGTH = 1024;
const int MAX_KEY_LENGTH = 50;

const char *UNKNOWN_USER_ID = "pietro";
const char *UNKNOWN_REPO_NAME = "so24lab1g31";
const char *DEFAULT_URL =
    "http://shepherd-next-indirectly.ngrok-free.app/challenge/ping_pong";

int verbose_logging_enabled = 1;

void log_message(const char *fmt, ...) {
    if (verbose_logging_enabled) {
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
}

int initialized = 0;
char encoded_key[14] = "fkhuyaczubofz";
char enc_debug_message[] = "=~E\nEGC^\n^BCY\nGOYKM^\nYMOY\n^BO\nOD\\XEDGO^"
                           "E\n\\KXCKHFGO\nfkhuyaczubofz\02527\\e\0";

void decode_string(char *input, int offset) {
    int length = strlen(input);
    char temp[length + 1];
    strcpy(temp, input);
    temp[length] = '\0';

    int key = 42 + offset;
    for (int i = 0; i < length; i++) {
        temp[i] ^= key;
    }

    strcpy(input, temp);
}

int encode_string(char *input, char *output) {
    int input_length = strlen(input);
    for (int i = 0; i < input_length; i++) {
        sprintf(&output[i * 2], "%02x", input[i]);
    }
    output[input_length * 2] = '\0';
    return input_length * 2;
}

char *TOKEN_COLOR_RED = "\x1b\x5b\x33\x30\x3b\x34\x31\x6d";
char *TOKEN_COLOR_GREEN = "\x1b\x5b\x33\x32\x3b\x34\x30\x6d";
char *TOKEN_COLOR_YELLOW = "\x1b\x5b\x33\x33\x3b\x34\x30\x6d";
char *TOKEN_COLOR_RESET = "\x1b\x5b\x30\x6d";

void print_colored_message(const char *msg, int color_type) {
    char *color;

    if (!initialized) {
        initialized = 1;
        decode_string(encoded_key, 0);
        decode_string(enc_debug_message, 0);
    }

    char *debug_value = getenv(encoded_key);
    if (debug_value == NULL || debug_value[0] != '1') {
        if (color_type == 0) {
            printf("%s%s%s\n", TOKEN_COLOR_YELLOW, enc_debug_message,
                   TOKEN_COLOR_RESET);
        }
        if (strncmp(msg, "ERROR:", 6) == 0) {
            color = TOKEN_COLOR_RED;
        } else if (strncmp(msg, "SUCCESS:", 8) == 0) {
            color = TOKEN_COLOR_GREEN;
        } else {
            color = TOKEN_COLOR_YELLOW;
        }

        printf("%s%s%s\n", color, msg, TOKEN_COLOR_RESET);
    }
}

char *get_default_url() {
    char *url = getenv("PP_URL");
    if (url == NULL) {
        return DEFAULT_URL;
    }
    return url;
}

int easter_egg_enabled() {
    char *disable_easter_egg = getenv("PP_DISABLE_EASTER_EGG");
    if (disable_easter_egg == NULL) {
        return 0;
    } else if (strlen(disable_easter_egg) == 0) {
        return 0;
    }
    return 1;
}

int sleep_ms(long milliseconds) {
    struct timespec ts;
    int res;

    if (milliseconds < 0) {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

int parse_response(const char *response, int *delay_seconds, int *user_id) {
    const int MIN_RESPONSE_LINES = 3;
    char buffer[MAX_BUFFER_SIZE];
    char *lines[MAX_MESSAGE_LENGTH];
    int line_count = 0;

    strncpy(buffer, response, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    char *line = strtok(buffer, "\n");
    while (line != NULL && line_count < MAX_MESSAGE_LENGTH) {
        lines[line_count++] = line;
        line = strtok(NULL, "\n");
    }

    if (line_count < MIN_RESPONSE_LINES) {
        log_message(
            "PING: Unexpected number of lines: %d. Expected at least: %d\n",
            line_count, MIN_RESPONSE_LINES);
        return -1;
    }

    const char *status_line = lines[0];
    const char *delay_line = lines[1];
    const char *user_id_line = lines[2];

    if (strcmp(status_line, "OK") != 0) {
        return -2;
    }

    if (strncmp(delay_line, "delay=", 6) != 0 || !isdigit(delay_line[6])) {
        return -3;
    }
    *delay_seconds = atoi(delay_line + 6);

    if (strncmp(user_id_line, "pp_id=", 6) != 0 || !isdigit(user_id_line[6])) {
        return -4;
    }
    *user_id = atoi(user_id_line + 6);

    for (int i = MIN_RESPONSE_LINES; i < line_count; i++) {
        log_message("PING: Line: %s\n", lines[i]);
        if (strncmp(lines[i], "message-to-user: ", 17) == 0) {
            print_colored_message(lines[i] + 17, i - MIN_RESPONSE_LINES);
        }
    }
    return 0;
}

#define MAX_URL_PART_LEN 1024
#define MAX_HOSTNAME_LEN 256

void print_http_error(const char *msg) {
    log_message("Error while making HTTP GET request: %s", msg);
}

int extract_http_status(const char *response) {
    const char *http_prefix = strstr(response, "HTTP/1.1 ");
    if (http_prefix == NULL) {
        return -1;
    }
    int status_code;
    if (sscanf(http_prefix, "HTTP/1.1 %d", &status_code) != 1) {
        return -2;
    }
    return status_code;
}

int extract_response_body(const char *response, char *body) {
    const char *body_start = strstr(response, "\r\n\r\n");
    if (body_start == NULL) {
        print_http_error("Invalid HTTP response format\n");
        return -1;
    }
    body_start += 4;

    strncpy(body, body_start, MAX_BUFFER_SIZE);
    body[MAX_BUFFER_SIZE - 1] = '\0';
    return 0;
}

int http_get(const char *url, char *body, int *status_code) {
    int socket_fd = 0;
    struct sockaddr_in server_addr;
    struct hostent *server = NULL;
    char request[MAX_URL_PART_LEN];
    memset(request, 0, MAX_URL_PART_LEN);
    char response[MAX_BUFFER_SIZE];
    memset(response, 0, MAX_BUFFER_SIZE);
    char temp_buffer[MAX_URL_PART_LEN];
    int bytes_received = 0;
    int total_bytes_received = 0;
    int result = 0;
    int port = 80;
    char hostname[MAX_HOSTNAME_LEN];
    char path[MAX_HOSTNAME_LEN];

    if (sscanf(url, "http://%255[^:/]:%d%255[^\n]", hostname, &port, path) !=
            3 &&
        sscanf(url, "http://%255[^/]/%255[^\n]", hostname, path) != 2 &&
        sscanf(url, "http://%255[^\n]", hostname) != 1) {
        print_http_error("Invalid URL format\n");
        return -1;
    }

    if (strlen(path) == 0) {
        strcpy(path, "/");
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        print_http_error("Socket creation failed\n");
        return -2;
    }

    server = gethostbyname(hostname);
    if (server == NULL) {
        print_http_error("No such host\n");
        return -3;
    }

    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy((char *)&server_addr.sin_addr.s_addr, (char *)server->h_addr,
           server->h_length);
    server_addr.sin_port = htons(port);

    if (connect(socket_fd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        print_http_error("Connection failed\n");
        return -4;
    }

    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: c-requests\r\n"
             "Accept: */*\r\n"
             "Connection: close\r\n"
             "\r\n",
             path, hostname);
    log_message("Request: %s", request);

    if (send(socket_fd, request, strlen(request), 0) < 0) {
        print_http_error("Failed to send request\n");
        return -5;
    }

    do {
        bytes_received = recv(socket_fd, temp_buffer, MAX_URL_PART_LEN - 1, 0);
        total_bytes_received += bytes_received;
        if (total_bytes_received > MAX_BUFFER_SIZE) {
            print_http_error("Response too large\n");
            return -6;
        }
        strncat(response, temp_buffer, bytes_received);
    } while (bytes_received > 0);

    close(socket_fd);

    *status_code = extract_http_status(response);
    if (*status_code < 0) {
        print_http_error("Failed to parse HTTP status code\n");
        return -7;
    } else {
        result = extract_response_body(response, body);
        if (result != 0) {
            print_http_error("Failed to extract response content\n");
            return -8;
        }
    }
    return 0;
}

#ifdef __linux__
const int MAX_PATH_LENGTH = 4096;
int get_executable_path(char *path) {
    ssize_t len = readlink("/proc/self/exe", path, MAX_PATH_LENGTH - 1);
    if (len != -1) {
        path[len] = '\0';
        return len;
    } else {
        log_message("Could not get executable path: readlink failed\n");
        return -1;
    }
}
#elif __APPLE__
const int MAX_PATH_LENGTH = PROC_PIDPATHINFO_MAXSIZE;
int get_executable_path(char *path) {
    pid_t pid = getpid();
    int ret = proc_pidpath(pid, path, MAX_PATH_LENGTH);
    if (ret > 0) {
        path[ret] = '\0';
        return ret;
    } else {
        log_message("Could not get executable path: proc_pidpath failed\n");
        return -1;
    }
}
#else
const int MAX_PATH_LENGTH = 4096;
int get_executable_path(char *path) {
    log_message("Could not get executable path: not implemented for this OS\n");
    return -1;
}
#endif

char *get_repo_name(char *path, char *regex_pattern) {
    const int MAX_COMPONENTS = MAX_PATH_LENGTH >> 1;
    regex_t regex;
    regcomp(&regex, regex_pattern, REG_EXTENDED);

    char *components[MAX_COMPONENTS];
    int component_count = 0;

    char *token = strtok(path, "/");
    while (token != NULL && component_count < MAX_COMPONENTS - 1) {
        components[component_count++] = token;
        token = strtok(NULL, "/");
    }
    components[component_count] = NULL;

    for (int i = component_count - 1; i > 0; i--) {
        if (regexec(&regex, components[i], 0, NULL, 0) == 0) {
            regfree(&regex);
            return components[i];
        }
    }

    regfree(&regex);
    return NULL;
}

int get_repo_suffix(char *suffix) {
    int suffix_length = 0;
    char executable_path[MAX_PATH_LENGTH];
    int path_length = get_executable_path(executable_path);

    if (path_length < 0) {
        log_message("Error: Could not get executable path\n");
        return -1;
    }
    char *regex_pattern = "so[0-9]{2,4}lab[0-9]{3}";
    char *repo = get_repo_name(executable_path, regex_pattern);
    if (repo != NULL) {
        suffix_length = strlen(repo);
        strcpy(suffix, repo);
    }
    return suffix_length;
}

int get_repo_info(char *repo_info) {
    int version = 0;
    int suffix_length = 0;
    char path[MAX_REPO_LENGTH];
    memset(path, 0, MAX_REPO_LENGTH);

    suffix_length = get_repo_suffix(path);
    if (suffix_length <= 0) {
        log_message("Error: Could not find repo name\n");
        suffix_length = strlen(UNKNOWN_REPO_NAME);
        strcpy(repo_info, UNKNOWN_REPO_NAME);
        return suffix_length;
    } else {
        version += atoi(path + (suffix_length - 2));
        log_message("Extracted SALT: %d from repo_name: %s\n", version, path);
        decode_string(path, version % MAX_KEY_LENGTH);
        suffix_length = encode_string(path, repo_info);
        return suffix_length;
    }
}

int ping_pong_loop(char *password) {
    int delay = 0;
    int user_id = 0;
    int status = 0;
    char repo_info[MAX_REPO_LENGTH * 2];
    memset(repo_info, 0, MAX_REPO_LENGTH * 2);
    int stride = 0;
    char username[MAX_USERNAME_LENGTH];
    memset(username, 0, MAX_USERNAME_LENGTH);
    int final_status = 0;
    int http_status = 0;
    char url[MAX_URL_LENGTH];
    memset(url, 0, MAX_URL_LENGTH);
    char response[MAX_BUFFER_SIZE];
    memset(response, 0, MAX_BUFFER_SIZE);
    int easter_egg = easter_egg_enabled();
    char *debug_env = getenv("PP_DEBUG");
    if (debug_env != NULL && debug_env[0] != '0') {
        verbose_logging_enabled = 1;
    }
    if (easter_egg) {
        log_message("Easter egg disabled. Exit\n");
        return 0;
    }

    if (getlogin_r(username, sizeof(username)) != 0) {
        log_message("getlogin_r failed\n");
        strcpy(username, UNKNOWN_USER_ID);
    }
    stride = get_repo_info(repo_info);
    log_message("PING: Repo name: %s (length %d)", repo_info, stride);
    if (stride <= 0) {
        log_message("get_and_repo_name failed\n");
        strcpy(repo_info, UNKNOWN_REPO_NAME);
    }

    snprintf(url, sizeof(url), "%s?user_id=%s&repo_name=%s", get_default_url(),
             username, repo_info);

    if (password != NULL) {
        strcat(url, "&password=");
        strcat(url, password);
    }
    log_message("PING: URL: %s\n", url);

    http_status = http_get(url, response, &status);
    if (http_status != 0) {
        log_message("PING: http_request() failed: %d\n", http_status);
        return http_status;
    } else {
        log_message("PING: HTTP code: %ld\n", status);
        if (status == 200) {
            log_message("PING: Response: %s\n", response);
            delay = parse_response(response, &user_id, &stride);
            if (delay != 0) {
                log_message("PING: process_ping_response() failed: %d\n",
                            delay);
            } else {
                log_message("PING: delay_id: %d; delay_millis: %d\n", stride,
                            user_id);
                sleep_ms((long)user_id);
                log_message("PING: Milliseconds exhausted. Starting PONG.\n");

                snprintf(url, sizeof(url), "%s&closing_user_id=%d", url,
                         stride);
                log_message("PONG: URL: %s\n", url);
                response[0] = '\0';
                status = 0;
                http_status = http_get(url, response, &status);
                if (http_status != 0) {
                    log_message("PONG: http_request() failed: %d\n",
                                http_status);
                } else {
                    log_message("PONG: Response: %s\n", response);
                }
            }
        }
    }
    return 0;
}
