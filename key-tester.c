/*
 * Keyboard Dropout and Chatter Tester (WSL2 Compatible Version)
 * Tests for keyboard hardware issues including:
 * 1. Dropout test - detects key release dropouts during continuous hold
 * 2. Chatter test - detects key bouncing/chattering when alternating between two keys
 *
 * NOTE: This version uses terminal input and is less accurate than the /dev/input version,
 * but works on WSL2 and other environments without direct hardware access.
 */

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <signal.h>
#include <sys/select.h>
#include <errno.h>

#define DROPOUT_THRESHOLD_MS 50
#define CHATTER_THRESHOLD_MS 200
#define CHATTER_SUPPRESS_MS 1000  /* Suppress chatter detection after dropout/hold end */

/* VT100 color codes */
#define COLOR_BG_RED "\033[41m"
#define COLOR_RESET "\033[0m"

static volatile int running = 1;
static struct termios orig_termios;

void signal_handler(int signum) {
    (void)signum;
    running = 0;
}

/* Restore terminal settings */
void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

/* Set terminal to raw mode */
void set_raw_mode(void) {
    struct termios raw;

    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(restore_terminal);

    raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/* Get current timestamp in milliseconds */
double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

/* Get formatted timestamp string */
void get_timestamp_str(char *buf, size_t size) {
    struct timespec ts;
    struct tm *tm_info;
    clock_gettime(CLOCK_REALTIME, &ts);
    tm_info = localtime(&ts.tv_sec);
    snprintf(buf, size, "%02d:%02d:%02d.%03ld",
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
             ts.tv_nsec / 1000000);
}

/* Read a single character with timeout */
int read_char_timeout(int timeout_ms) {
    fd_set readfds;
    struct timeval tv;
    int ret;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
    if (ret > 0) {
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            return (unsigned char)c;
        }
    }
    return -1;
}

/* Get printable key name */
const char* get_key_name(int c) {
    static char name[32];

    switch(c) {
        case 27: return "ESC";
        case '\n': return "ENTER";
        case '\r': return "RETURN";
        case '\t': return "TAB";
        case ' ': return "SPACE";
        case 127: return "BACKSPACE";
        default:
            if (c >= 32 && c < 127) {
                snprintf(name, sizeof(name), "'%c'", c);
            } else if (c == -1) {
                return "TIMEOUT";
            } else {
                snprintf(name, sizeof(name), "0x%02X", c);
            }
            return name;
    }
}

/* Auto test - detect both dropouts and chattering */
void auto_test(void) {
    char timestamp[32];
    int last_key = -1;
    double last_time = 0;
    double first_press_time = 0;
    int dropout_count = 0;
    int chatter_count = 0;
    int repeat_count = 0;
    int press_count = 0;
    double chatter_suppress_until = 0;  /* Suppress chatter detection until this time */

    printf("\n============================================================\n");
    printf("Auto Test (WSL2 Version)\n");
    printf("============================================================\n");
    printf("Press any keys freely.\n");
    printf("Dropouts and chattering will be detected automatically.\n");
    printf("\n");
    printf("Detection:\n");
    printf("  - Dropout: Key repeat suddenly stops (within %.0fms)\n", 500.0);
    printf("  - Chattering: Same key pressed consecutively (within %.0fms)\n", (double)CHATTER_THRESHOLD_MS);
    printf("\n");
    printf("Note: This method uses terminal input and is not completely accurate.\n");
    printf("Press Ctrl+C to exit the test.\n");
    printf("------------------------------------------------------------\n");

    set_raw_mode();

    while (running) {
        int c = read_char_timeout(100);
        double current_time = get_time_ms();
        get_timestamp_str(timestamp, sizeof(timestamp));

        if (c > 0) {
            const char *key_name = get_key_name(c);
            press_count++;

            if (c == last_key && last_time > 0) {
                /* Same key pressed/repeated */
                double interval = current_time - last_time;

                /* Check for chattering first (short interval, likely first repeat) */
                /* Skip chatter detection if we're in suppression period */
                if (repeat_count == 0 && interval < CHATTER_THRESHOLD_MS && current_time > chatter_suppress_until) {
                    chatter_count++;
                    printf(COLOR_BG_RED "[%s] ⚠️  CHATTER DETECTED! %s pressed again after %.1fms (#%d)" COLOR_RESET "\n",
                           timestamp, key_name, interval, chatter_count);
                } else {
                    /* Normal key repeat */
                    repeat_count++;
                    printf("[%s] %s repeat #%d (interval: %.1fms)\n",
                           timestamp, key_name, repeat_count, interval);
                }
            } else {
                /* Different key pressed */
                if (last_key != -1 && repeat_count > 0) {
                    /* Previous key was being held */
                    double hold_duration = last_time - first_press_time;
                    printf("[%s] %s hold ended (%.2fs, %d repeats)\n",
                           timestamp, get_key_name(last_key), hold_duration / 1000.0, repeat_count);

                    /* Suppress chatter detection for a while after hold ends */
                    chatter_suppress_until = current_time + CHATTER_SUPPRESS_MS;
                }

                printf("[%s] %s pressed\n", timestamp, key_name);
                first_press_time = current_time;
                repeat_count = 0;
            }

            last_key = c;
            last_time = current_time;

        } else if (c == -1 && last_key != -1 && last_time > 0) {
            /* Timeout - check if key was being held */
            double silence_duration = current_time - last_time;

            if (repeat_count > 0 && silence_duration > 100) {
                /* Key was being held and now stopped */
                if (silence_duration < 500) {
                    /* Stopped too soon - possible dropout */
                    dropout_count++;
                    printf(COLOR_BG_RED "[%s] ⚠️  DROPOUT DETECTED! %s repeat stopped after %.0fms (#%d)" COLOR_RESET "\n",
                           timestamp, get_key_name(last_key), silence_duration, dropout_count);
                }

                /* Suppress chatter detection for a while after dropout */
                chatter_suppress_until = current_time + CHATTER_SUPPRESS_MS;

                last_key = -1;
                repeat_count = 0;
            }
        }

        if (c == 3) {  /* Ctrl+C */
            running = 0;
            break;
        }
    }

    restore_terminal();
    printf("\n\n============================================================\n");
    printf("Test Complete\n");
    printf("============================================================\n");
    printf("Total key presses: %d\n", press_count);
    printf("Dropouts detected: %d\n", dropout_count);
    printf("Chatters detected: %d\n", chatter_count);
    printf("============================================================\n");
}

int main(void) {
    printf("Keyboard Dropout & Chatter Tester (WSL2 Version)\n");
    printf("==========================================================\n");
    printf("Note: This version gets input via terminal and is less\n");
    printf("      accurate than the /dev/input version.\n");
    printf("      For more accurate testing, run on native Linux.\n");
    printf("==========================================================\n");

    /* Set up signal handler for Ctrl+C */
    signal(SIGINT, signal_handler);

    /* Run auto test */
    auto_test();

    return 0;
}
