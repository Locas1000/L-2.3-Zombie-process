#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

// Forward declarations
void reap_explicit(int child_count);
void setup_auto_reaper(void);
void sigchld_handler(int sig);
void setup_ignore_reaper(void);

#define NUM_CHILDREN 10

/**
 * STRATEGY 2: SIGCHLD Handler
 * This function is called asynchronously by the OS when a child terminates.
 */
void sigchld_handler(int sig) {
    int saved_errno = errno; // waitpid() might change errno
    int status;
    pid_t pid;

    // The key is to use a while loop with WNOHANG.
    // This reaps all terminated children, not just one.
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // In a real application, use a safe logging method from a signal handler.
        // For this demo, printf is acceptable to show it's working.
        printf("Handler reaped child PID %d\n", pid);
    }
    errno = saved_errno;
}

/**
 * STRATEGY 1: Explicit Wait
 * Block and wait for every child process to terminate.
 */
void reap_explicit(int child_count) {
    printf("Strategy 1: Using explicit waitpid() in a loop.\n");
    for (int i = 0; i < child_count; i++) {
        int status;
        pid_t child_pid = wait(&status); // Blocks until one child terminates
        if (child_pid > 0) {
            printf("Explicitly reaped child PID %d\n", child_pid);
        }
    }
}

/**
 * STRATEGY 2: Set up SIGCHLD handler to auto-reap.
 * The parent can continue doing other work.
 */
void setup_auto_reaper(void) {
    printf("Strategy 2: Setting up SIGCHLD handler for automatic reaping.\n");
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("sigaction");
        exit(1);
    }
}

/**
 * STRATEGY 3: Use signal(SIGCHLD, SIG_IGN) for automatic reaping.
 * The kernel will auto-reap children; they never become zombies.
 */
void setup_ignore_reaper(void) {
    printf("Strategy 3: Setting SIGCHLD to SIG_IGN for kernel-level reaping.\n");
    signal(SIGCHLD, SIG_IGN);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <strategy_number (1, 2, or 3)>\n", argv[0]);
        return 1;
    }

    int strategy = atoi(argv[1]);
    srand(time(NULL)); // Seed for random sleep times

    printf("Parent PID %d starting...\n", getpid());

    // Create 10 children that do some "work" and exit.
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) {
            // Child process
            sleep(rand() % 4); // Sleep for 0-3 seconds
            //printf("Child %d exiting.\n", getpid());
            exit(i);
        }
    }

    // Use the chosen reaping strategy
    switch (strategy) {
        case 1:
            reap_explicit(NUM_CHILDREN);
            break;
        case 2:
            setup_auto_reaper();
            break;
        case 3:
            setup_ignore_reaper();
            break;
        default:
            fprintf(stderr, "Invalid strategy. Please use 1, 2, or 3.\n");
            return 1;
    }

    // For strategies 2 and 3, the parent needs to stay alive
    // to give the handler/kernel time to reap the children.
    if (strategy == 2 || strategy == 3) {
        printf("Parent is now waiting for children to be reaped asynchronously...\n");
        sleep(5); // Wait long enough for all children to exit
    }

    printf("\nVerification: Checking for remaining zombie processes...\n");
    // This command should produce NO output if reaping was successful.
    system("ps aux | awk '$8 ~ /Z/'");
    printf("Verification complete. If you saw no processes listed above, it worked.\n");

    return 0;
}