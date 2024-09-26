#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <ctime>
#include <sys/shm.h>

// Tamaño de la memoria compartida
#define SHM_SIZE 1024

// Estructura para la memoria compartida
struct shared_data {
    int Compartida;
};

// Función para los procesos de tipo 1
void tipo1(int x, struct shared_data *shared) {
    // Incrementar la variable global "Compartida" en la memoria compartida
    __sync_fetch_and_add(&(shared->Compartida), 1); // Para incrementar de manera atómica
    std::cout << "\tInstancia " << x << " del proceso 1" << std::endl;
    
    // Suspender por un tiempo aleatorio entre 0 y 2 segundos
    usleep((rand() % 3) * 1000000);
}

// Función para los procesos de tipo 2
void tipo2(int y, struct shared_data *shared) {
    // Incrementar la variable global "Compartida" en la memoria compartida
    __sync_fetch_and_add(&(shared->Compartida), 1); // Para incrementar de manera atómica
    std::cout << "\tInstancia " << y << " del proceso 2" << std::endl;
    
    // Suspender por un tiempo aleatorio entre 0 y 2 segundos
    usleep((rand() % 3) * 1000000);
}

int main() {
    srand(time(nullptr)); // Inicializar la semilla para los números aleatorios

    int N, M;
    std::cout << "Introduce el número de procesos de tipo 1: ";
    std::cin >> N;
    std::cout << "Introduce el número de procesos de tipo 2: ";
    std::cin >> M;

    // Crear y adjuntar la memoria compartida
    int shm_id = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id < 0) {
        std::cerr << "Error al crear la memoria compartida" << std::endl;
        return 1;
    }

    struct shared_data *shared = (struct shared_data*)shmat(shm_id, nullptr, 0);
    if (shared == (struct shared_data*)(-1)) {
        std::cerr << "Error al adjuntar la memoria compartida" << std::endl;
        return 1;
    }

    shared->Compartida = 0;

    for (int i = 0; i < N; ++i) {
        pid_t pid = fork();

        if (pid == 0) {
            // Proceso hijo de tipo 1
            tipo1(i, shared);
            exit(0); // Salir del proceso hijo
        } else if (pid < 0) {
            std::cerr << "Error al crear el proceso de tipo 1, instancia " << i << std::endl;
            return 1;
        }
    }

    for (int i = 0; i < M; ++i) {
        pid_t pid = fork();

        if (pid == 0) {
            // Proceso hijo de tipo 2
            tipo2(i, shared);
            exit(0); // Salir del proceso hijo
        } else if (pid < 0) {
            std::cerr << "Error al crear el proceso de tipo 2, instancia " << i << std::endl;
            return 1;
        }
    }

    // Esperar a que todos los procesos terminen
    for (int i = 0; i < N + M; ++i) {
        wait(nullptr);
    }

    // Mostrar el valor de la variable compartida después de que todos los procesos hayan terminado
    std::cout << "\tValor final de la variable Compartida después de procesos: " << shared->Compartida << std::endl;

    // Desadjuntar y eliminar la memoria compartida
    shmdt(shared);
    shmctl(shm_id, IPC_RMID, nullptr);

    std::cout << "Se ha finalizado la ejecución" << std::endl;

    return 0;
}
