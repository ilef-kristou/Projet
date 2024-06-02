#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>

#define NUM_CHILDREN 4

sem_t *sem;

void child_process(int id) {
    printf("Child %d started\n", id);
    // Attends le signal de démarrage
    sem_wait(sem);
    printf("Child %d executing task\n", id);
    // Simule une tâche en boucle
    for (int i = 0; i < 5; i++) {
        sleep(1);
    }
    printf("Child %d finished task\n", id);
    // Envoie un signal de confirmation au processus père
    kill(getppid(), SIGUSR1);
}

void signal_handler(int signum) {
    if (signum == SIGUSR1) {
        printf("Received confirmation from child\n");
    }
}

int main() {
    sem = sem_open("/my_semaphore", O_CREAT, 0644, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Attache le gestionnaire de signal
    signal(SIGUSR1, signal_handler);

    pid_t children[NUM_CHILDREN];
    // Crée les processus fils
    for (int i = 0; i < NUM_CHILDREN; i++) {
        children[i] = fork();
        if (children[i] == 0) {
            // Code pour le processus fils
            child_process(i + 1);
            exit(EXIT_SUCCESS);
        } else if (children[i] < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    printf("All children created\n");
    // Envoie le signal de démarrage aux processus fils
    for (int i = 0; i < NUM_CHILDREN; i++) {
        sem_post(sem);
    }

    // Attends la confirmation de chaque processus fils
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pause(); // Attend un signal
    }

    // Ferme et détruit le sémaphore
    sem_close(sem);
    sem_unlink("/my_semaphore");

    return 0;
}
