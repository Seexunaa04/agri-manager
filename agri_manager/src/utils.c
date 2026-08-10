#include "../include/utils.h"
#include "../include/models.h"

#define XOR_KEY "AGRIMANAGER2026"

void init_directories(void) {
    _mkdir(DIR_DATABASE);
    _mkdir(DIR_REPORTS);
    _mkdir(DIR_REPORTS_ORDERS);
    _mkdir(DIR_REPORTS_DELIVERIES);
    _mkdir(DIR_REPORTS_DAILY);
}

void get_current_date(char *buffer, size_t size) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, size, "%d/%m/%Y %H:%M:%S", tm_info);
}

void get_date_code(char *buffer, size_t size) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, size, "%Y%m%d%H%M%S", tm_info);
}

void get_date_only(char *buffer, size_t size) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, size, "%Y%m%d", tm_info);
}

void get_future_date_days(int days, char *buffer, size_t size) {
    time_t t = time(NULL) + (days * 86400);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, size, "%d/%m/%Y %H:%M:%S", tm_info);
}

int calculate_days_difference(const char *date_start, const char *date_end) {
    int d1, m1, y1, h1, min1, s1;
    int d2, m2, y2, h2, min2, s2;
    struct tm tm1 = {0}, tm2 = {0};

    if (sscanf(date_start, "%d/%d/%d %d:%d:%d", &d1, &m1, &y1, &h1, &min1, &s1) >= 3) {
        tm1.tm_mday = d1;
        tm1.tm_mon = m1 - 1;
        tm1.tm_year = y1 - 1900;
        tm1.tm_hour = h1;
        tm1.tm_min = min1;
        tm1.tm_sec = s1;
    }

    if (sscanf(date_end, "%d/%d/%d %d:%d:%d", &d2, &m2, &y2, &h2, &min2, &s2) >= 3) {
        tm2.tm_mday = d2;
        tm2.tm_mon = m2 - 1;
        tm2.tm_year = y2 - 1900;
        tm2.tm_hour = h2;
        tm2.tm_min = min2;
        tm2.tm_sec = s2;
    }

    time_t t1 = mktime(&tm1);
    time_t t2 = mktime(&tm2);

    if (t1 == -1 || t2 == -1) return 0;
    double diff = difftime(t2, t1);
    return (int)(diff / 86400.0);
}

void encrypt_password(const char *input, char *output) {
    size_t key_len = strlen(XOR_KEY);
    size_t in_len = strlen(input);
    output[0] = '\0';

    for (size_t i = 0; i < in_len; i++) {
        char encrypted_char = input[i] ^ XOR_KEY[i % key_len];
        char hex[3];
        snprintf(hex, sizeof(hex), "%02X", (unsigned char)encrypted_char);
        strcat(output, hex);
    }
}

void decrypt_password(const char *input, char *output) {
    size_t key_len = strlen(XOR_KEY);
    size_t in_len = strlen(input);
    size_t out_idx = 0;

    for (size_t i = 0; i < in_len; i += 2) {
        char hex[3] = {input[i], input[i+1], '\0'};
        unsigned char val = (unsigned char)strtol(hex, NULL, 16);
        output[out_idx] = val ^ XOR_KEY[(out_idx) % key_len];
        out_idx++;
    }
    output[out_idx] = '\0';
}

int validate_login(const char *login) {
    if (strlen(login) != 6) return 0;
    for (int i = 0; i < 6; i++) {
        if (!isupper(login[i])) return 0;
    }
    return 1;
}

void read_string(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
}

int read_int(const char *prompt) {
    char buf[100];
    int val = 0;
    printf("%s", prompt);
    if (fgets(buf, sizeof(buf), stdin) != NULL) {
        val = atoi(buf);
    }
    return val;
}

double read_double(const char *prompt) {
    char buf[100];
    double val = 0.0;
    printf("%s", prompt);
    if (fgets(buf, sizeof(buf), stdin) != NULL) {
        val = atof(buf);
    }
    return val;
}

void read_password_masked(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
}

void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pause_screen(void) {
    printf("\nAppuyez sur Entree pour continuer...");
    getchar();
}

void print_header(const char *title) {
    printf("\n=======================================================\n");
    printf("   AGRI-MANAGER - %s\n", title);
    printf("=======================================================\n\n");
}
