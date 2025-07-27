// src/ui/logger.h
#pragma once
#include <stdarg.h>

// Enum for different console output styles
typedef enum {
    LOG_DEFAULT,
    LOG_BOLD_WHITE,
    LOG_GREEN,
    LOG_YELLOW,
    LOG_RED,
} log_style_t;

void log_printf(log_style_t style, const char *format, ...);
