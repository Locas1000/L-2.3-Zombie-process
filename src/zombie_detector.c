#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>   // For directory handling
#include <ctype.h>    // For isdigit()

// Forward declarations to match the assignment structure
int find_zombies(int *zombie_pids, int max_zombies);
void print_zombie_info(int pid);

// A helper struct to store zombie details for the report
typedef struct {
    int pid;
    int ppid;
    char comm[256];
} ZombieInfo;

// Helper to get the name of a process from its PID
void get_process_name(int pid, char *name_buffer, int size) {
    char path[256];
    sprintf(path, "/proc/%d/comm", pid);
    FILE *f = fopen(path, "r");
    if (f) {
        fgets(name_buffer, size, f);
        // Remove trailing newline
        name_buffer[strcspn(name_buffer, "\n")] = 0;
        fclose(f);
    } else {
        strcpy(name_buffer, "unknown");
    }
}


int main() {
    ZombieInfo zombies[1024];
    int zombie_count = 0;

    // Scan /proc to find all zombies
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("Could not open /proc");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        // We only care about directories that are numbers (PIDs)
        if (!isdigit(*entry->d_name)) {
            continue;
        }

        int pid = atoi(entry->d_name);
        char path[256];
        sprintf(path, "/proc/%d/stat", pid);

        FILE *stat_file = fopen(path, "r");
        if (!stat_file) {
            continue; // Process might have just ended
        }

        // Parse the stat file: pid (comm) state ppid ...
        int ppid;
        char comm[256], state;
        fscanf(stat_file, "%*d (%255[^)]) %c %d", comm, &state, &ppid);
        fclose(stat_file);

        if (state == 'Z') {
            zombies[zombie_count].pid = pid;
            zombies[zombie_count].ppid = ppid;
            strcpy(zombies[zombie_count].comm, comm);
            zombie_count++;
            if (zombie_count >= 1024) break;
        }
    }
    closedir(proc_dir);

    // --- Print the Report ---
    printf("=== Zombie Process Report ===\n");
    printf("Total Zombies: %d\n\n", zombie_count);

    if (zombie_count > 0) {
        printf("%-7s %-7s %-15s %-7s\n", "PID", "PPID", "Command", "State");
        printf("------- ------- --------------- -------\n");
        for (int i = 0; i < zombie_count; i++) {
            printf("%-7d %-7d %-15s %-7c\n",
                zombies[i].pid,
                zombies[i].ppid,
                zombies[i].comm,
                'Z');
        }

        printf("\nParent Process Analysis:\n");
        // Simple analysis: find unique parents and count children
        int parent_pids[1024];
        int child_counts[1024] = {0};
        int unique_parents = 0;

        for (int i = 0; i < zombie_count; i++) {
            int found = 0;
            for (int j = 0; j < unique_parents; j++) {
                if (parent_pids[j] == zombies[i].ppid) {
                    child_counts[j]++;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                parent_pids[unique_parents] = zombies[i].ppid;
                child_counts[unique_parents]++;
                unique_parents++;
            }
        }

        for (int i = 0; i < unique_parents; i++) {
            char parent_name[256];
            get_process_name(parent_pids[i], parent_name, sizeof(parent_name));
            printf("  - PID %d (%s) has %d zombie children.\n", parent_pids[i], parent_name, child_counts[i]);
        }
    }

    return 0;
}