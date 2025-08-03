// src/utils/poke.c
#include "poke.h"
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

/**
 * NOTE:
 * In computing, PEEK and POKE are commands used in some high-level programming
 * languages for accessing the contents of a specific memory cell referenced by
 * its memory address.
 * - PEEK: Reads the value from a specific memory address.
 * - POKE: Writes a value to a specific memory address.
 * I decided to continue using these terms in the codebase to respect
 * such old (BASIC era) terminology.
 */

/**
 * Writes `len` bytes from `buf` to the memory of the process with ID `pid`
 *
 * @param pid The process ID of the target process.
 * @param addr The address in the target process's memory where data should be
 * written.
 * @param buf Pointer to the buffer containing data to write.
 * @param len The number of bytes to write from `buf`.
 * @return 0 on success, or an error code on failure.
 */
int poke_mem(pid_t pid, uintptr_t addr, const void *buf, size_t len) {
    struct iovec local = {.iov_base = (void *)buf, .iov_len = len};
    struct iovec remote = {.iov_base = (void *)addr, .iov_len = len};

    ssize_t bytes_written = process_vm_writev(pid, &local, 1, &remote, 1, 0);
    if (bytes_written < 0) {
        return errno;
    }
    if ((size_t)bytes_written != len) {
        return EIO;
    }

    return 0;
}
