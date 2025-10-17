#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

/**
 * Creates N zombie processes for testing.
 * The parent process forks 'count' children but never waits for them.
 * This causes the children to become zombie processes upon termination.
 */
int create_zombies(int count) {
    for (int i = 0; i < count; i++) {
        pid_t pid = fork();

        if (pid < 0) { // Fork failed
            perror("fork failed");
            return -1;
        }

        if (pid == 0) {
            // --- Child Process ---
            // The child exits immediately. The OS will hold its exit status
            // in the process table, waiting for the parent to collect it.
            // Since the parent never collects it, this process becomes a zombie.
            exit(i);
        } else {
            // --- Parent Process ---
            // The parent does NOT call wait(). It simply prints the child's PID
            // and continues, leaving the terminated child in a zombie state.
            printf("Created zombie: PID %d (child will exit with code %d)\n", pid, i);
            // A small sleep can help make the output more predictable
            // if the system is under heavy load, but it's not strictly necessary.
            usleep(50000); // Sleep for 50ms
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_zombies>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "Please enter a positive number of zombies.\n");
        return 1;
    }

    printf("Parent PID: %d\n", getpid());
    create_zombies(n);

    printf("\nCreated %d zombies. Open another terminal and run:\n", n);
    printf("  ps aux | grep 'Z'\n");
    printf("Press Enter to terminate parent and clean up zombies...\n");

    // The program waits here. While it's waiting, the zombies exist.
    getchar();

    // When the parent exits, 'init' (or systemd) will adopt and reap the zombies.
    printf("Parent exiting. Zombies will now be cleaned up by the OS.\n");

    return 0;
}