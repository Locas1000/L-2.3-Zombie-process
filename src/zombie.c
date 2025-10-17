#include "zombie.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h> // For mutex

// --- Global state for the library ---
static zombie_stats_t g_stats = {0, 0, 0};
static pthread_mutex_t g_stats_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_initialized = 0;

/**
 * The signal handler that reaps any terminated child process.
 */
static void sigchld_handler(int sig) {
    int status;
    pid_t pid;

    // Reap all terminated children
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        pthread_mutex_lock(&g_stats_mutex);
        g_stats.zombies_reaped++;
        pthread_mutex_unlock(&g_stats_mutex);
    }
}

/**
 * Initialize zombie prevention. Must be called once at program start.
 */
void zombie_init(void) {
    if (g_initialized) {
        return; // Only initialize once
    }

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("sigaction");
        exit(1);
    }
    g_initialized = 1;
}

/**
 * A replacement for fork() that guarantees the child will be reaped.
 */
pid_t zombie_safe_fork(void) {
    if (!g_initialized) {
        // Auto-initialize if user forgot, though calling zombie_init() is best practice.
        zombie_init();
    }

    pid_t pid = fork();

    if (pid > 0) { // Parent process
        pthread_mutex_lock(&g_stats_mutex);
        g_stats.zombies_created++;
        pthread_mutex_unlock(&g_stats_mutex);
    }
    return pid;
}

/**
 * Spawn a command in a new process and guarantee it will be reaped.
 */
int zombie_safe_spawn(const char *command, char *const args[]) {
    pid_t pid = zombie_safe_fork();

    if (pid < 0) {
        perror("fork in zombie_safe_spawn");
        return -1;
    }

    if (pid == 0) {
        // Child process: execute the command
        execvp(command, args);
        // execvp only returns on error
        perror("execvp failed");
        _exit(127); // Use _exit in child after fork
    }

    // Parent process returns the new child's PID
    return pid;
}

/**
 * Get the current zombie statistics. This function is thread-safe.
 */
void zombie_get_stats(zombie_stats_t *stats) {
    if (!stats) return;

    pthread_mutex_lock(&g_stats_mutex);
    stats->zombies_created = g_stats.zombies_created;
    stats->zombies_reaped = g_stats.zombies_reaped;
    stats->zombies_active = g_stats.zombies_created - g_stats.zombies_reaped;
    pthread_mutex_unlock(&g_stats_mutex);
}