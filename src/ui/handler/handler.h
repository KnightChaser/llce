// src/ui/handler/handler.h
#pragma once

// core UI handlers
void handle_help(void);
void handle_attach(char *arg);
void handle_fullscan(void);
void handle_detect(void);
void handle_search(char *type_str, char *value_str);
void handle_poke(char *addr_str, char *type_str, char *value_str);

// utility function to print the command prompt
void print_prompt(void);

// cleanup function to free resources and reset state
void cleanup_app_state(void);
