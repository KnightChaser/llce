// src/utils/poke.h
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

int poke_mem(pid_t pid, uintptr_t addr, const void *buf, size_t len);
