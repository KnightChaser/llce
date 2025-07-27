// src/main.c
#include "ui/ui.h"

int main(int argc __attribute__((unused)),
         char **argv __attribute__((unused))) {
    // The main function now only needs to launch the UI.
    // We are no longer taking command-line arguments directly.
    run_ui();
    return 0;
}
