// cookie_clicker.c
#define _POSIX_C_SOURCE 200809L // for nanosleep
#include <stdint.h>
#include <stdio.h>
#include <time.h>

uint64_t cookies = 30;
uint32_t cursors = 0;
uint32_t factories = 0;
uint32_t cursor_cost = 10;
uint32_t factory_cost = 100;
char player_name[] = "KnightChaser";

// How many cookies each building makes per second
const uint32_t CPC = 1;  // Cursor per second
const uint32_t FPC = 10; // Factory per second

// Clear the screen (ANSI escape)
#define CLEAR_SCREEN() printf("\033[H\033[J")

/**
 * Print the current game status.
 * This includes the player's name, total cookies,
 * number of buildings, and shop options.
 */
void print_status(void) {
    CLEAR_SCREEN();
    printf("========================================\n");
    printf("Player : %s\n", player_name);
    printf("Cookies: %llu\n", (unsigned long long)cookies);
    printf("----------------------------------------\n");
    printf("Buildings:\n");
    printf("  Cursors : %u\n", cursors);
    printf("  Factories: %u\n", factories);
    printf("----------------------------------------\n");
    printf("Shop:\n");
    printf("  [c] Buy Cursor  (cost: %u cookies)\n", cursor_cost);
    printf("  [f] Buy Factory (cost: %u cookies)\n", factory_cost);
    printf("  [r] Refresh the status\n");
    printf("  [q] Quit\n");
    printf("----------------------------------------\n");
    printf("Any other key = Cookie click (+1)\n");
    printf("Enter command > ");
    fflush(stdout);
}

int main(void) {
    time_t last_time = time(NULL);
    while (1) {
        print_status();

        int cmd = getchar();
        // swallow extra chars (e.g. newline)
        while (getchar() != '\n' && !feof(stdin))
            ;

        // generate cookies since last action
        time_t now = time(NULL);
        if (now > last_time) {
            time_t dt = now - last_time;
            cookies += (uint64_t)(cursors * CPC + factories * FPC) * dt;
            last_time = now;
        }

        switch (cmd) {
        case 'c':
            if (cookies >= cursor_cost) {
                cookies -= cursor_cost;
                cursors++;
                cursor_cost = (uint32_t)(cursor_cost * 1.15);
                printf("→ Cursor purchased!\n");
            } else {
                printf("✗ Not enough cookies for a cursor. Current cost: %u, "
                       "Current cookies: %llu\n",
                       cursor_cost, (unsigned long long)cookies);
            }
            break;
        case 'f':
            if (cookies >= factory_cost) {
                cookies -= factory_cost;
                factories++;
                factory_cost = (uint32_t)(factory_cost * 1.15);
                printf("→ Factory purchased!\n");
            } else {
                printf("✗ Not enough cookies for a factory. Current cost: %u, "
                       "Current cookies: %llu\n",
                       factory_cost, (unsigned long long)cookies);
            }
            break;
        case 'r':
            // refresh the status
            print_status();
            break;
        case 'q':
            printf("Goodbye, cookie monster.\n");
            return 0;
        default:
            // click!
            cookies++;
            break;
        }

        // small pause so purchase messages show
        struct timespec ts = {.tv_sec = 0, .tv_nsec = 3500000000};
        nanosleep(&ts, NULL);
    }

    return 0;
}
