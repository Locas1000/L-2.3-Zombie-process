#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>

#define LOG_FILE "/tmp/daemon.log"

// Global flag to control the main loop
volatile sig_atomic_t keep_running = 1;

// Function to log messages to our file
void log_message(const char *message) {
    FILE *logfile = fopen(LOG_FILE, "a");
    if (!logfile) return;

    time_t now = time(NULL);
    char *date = ctime(&now);
    date[strlen(date) - 1] = '\0'; // Remove newline

    fprintf(logfile, "[%s] %s\n", date, message);
    fclose(logfile);
}

/**
 * SIGCHLD handler to reap zombie children.
 * This is the same robust handler from Part 3.
 */
void sigchld_handler(int sig) {
    int saved_errno = errno;
    char msg[128];
    pid_t pid;

    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        sprintf(msg, "Reaped child process %d.", (int)pid);
        log_message(msg);
    }
    errno = saved_errno;
}

/**
 * SIGTERM handler for graceful shutdown.
 */
void sigterm_handler(int sig) {
    log_message("SIGTERM received. Shutting down gracefully.");
    keep_running = 0;
}

/**
 * The standard daemonization procedure.
 */
void daemonize() {
    // 1. Fork and exit parent
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); // Parent exits

    // 2. Create a new session
    if (setsid() < 0) exit(EXIT_FAILURE);

    // Set up signal handlers for the daemon
    signal(SIGCHLD, sigchld_handler);
    signal(SIGTERM, sigterm_handler);

    // 3. Fork again to prevent re-acquiring a terminal
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); // First child exits

    // 4. Change the working directory to root
    chdir("/");

    // 5. Close all file descriptors
    // A more robust way than just closing 0, 1, 2
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
        close(x);
    }

    // Re-open stdin, stdout, stderr to /dev/null
    // This is to prevent issues with library functions
    // that assume these fds are open.
    open("/dev/null", O_RDWR);   // stdin
    dup(0);                      // stdout
    dup(0);                      // stderr
}

int main() {
    daemonize();
    log_message("Daemon process started.");

    // Main daemon loop
    while (keep_running) {
        // Spawn a worker process
        pid_t worker_pid = fork();
        if (worker_pid == 0) {
            // Child (worker) process
            log_message("Worker process spawned.");
            sleep(2); // Simulate doing some work
            log_message("Worker process finished.");
            exit(0);
        } else if (worker_pid > 0) {
            char msg[128];
            sprintf(msg, "Parent spawned worker with PID %d.", (int)worker_pid);
            log_message(msg);
        }

        // Wait before spawning the next worker
        sleep(5);
    }

    log_message("Daemon process shutting down.");
    return 0;
}