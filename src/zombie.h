#ifndef ZOMBIE_H
#define ZOMBIE_H

#include <sys/types.h>

// Initialize zombie prevention. Must be called once at program start.
void zombie_init(void);

// A replacement for fork() that guarantees the child will be reaped.
pid_t zombie_safe_fork(void);

// Spawn a command in a new process and guarantee it will be reaped.
int zombie_safe_spawn(const char *command, char *const args[]);

// Structure to hold statistics about zombie management.
typedef struct {
    int zombies_created;
    int zombies_reaped;
    int zombies_active;
} zombie_stats_t;

// Get the current zombie statistics. This function is thread-safe.
void zombie_get_stats(zombie_stats_t *stats);

#endif // ZOMBIE_H