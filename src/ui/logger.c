// src/ui/logger.c
#include "logger.h"
#include <stdio.h>

/**
 * Prints formatted text to the console with a specific style.
 * The style is applied using ANSI escape codes for color and formatting.
 *
 * @param style The style to apply to the output.
 * @param format The format string for the output.
 * @param ... Additional arguments for the format string.
 */
void log_printf(log_style_t style, const char *format, ...) {
    // Apply the selected style using ANSI escape codes
    switch (style) {
    case LOG_BOLD_WHITE:
        printf("\x1b[1;37m");
        break;
    case LOG_GREEN:
        printf("\x1b[0;32m");
        break;
    case LOG_YELLOW:
        printf("\x1b[0;33m");
        break;
    case LOG_RED:
        printf("\x1b[0;31m");
        break;
    case LOG_DEFAULT:
    default:
        break;
    }

    // Use vprintf() to handle the variable arguments
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    // Reset the style back to default
    printf("\x1b[0m");
    fflush(stdout);
}
