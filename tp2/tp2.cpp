#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

// Variable global compartida
int Compartida = 0;

// Función para los hilos de tipo 1
void* tipo1(void* arg) {
    int x = *(int*)arg;
    std::cout << "\tInstancia " << x << " del hilo 1" << std::endl;
    
    // Suspender por un tiempo aleatorio entre 0 y 2 segundos
    usleep((rand() % 3) * 1000000);
    
    // Incrementar la variable global "Compartida"
    __sync_fetch_and_add(&Compartida, 1); // Para incrementar de manera atómica
    pthread_exit(nullptr);
}

// Función para los hilos de tipo 2
void* tipo2(void* arg) {
    int y = *(int*)arg;
    std::cout << "\tInstancia " << y << " del hilo 2" << std::endl;
    
    // Suspender por un tiempo aleatorio entre 0 y 2 segundos
    usleep((rand() % 3) * 1000000);
    
    // Leer y mostrar el valor de la variable global "Compartida"
    std::cout << "\tValor de la variable Compartida: " << Compartida << std::endl;
    pthread_exit(nullptr);
}

int main() {
    srand(time(nullptr)); // Inicializar la semilla para los números aleatorios

    int N, M;
    std::cout << "Introduce el número de hilos de tipo 1: ";
    std::cin >> N;
    std::cout << "Introduce el número de hilos de tipo 2: ";
    std::cin >> M;

    pthread_t hilosTipo1[N];
    pthread_t hilosTipo2[M];
    int indicesTipo1[N];
    int indicesTipo2[M];

    // Crear hilos de tipo 1
    for (int i = 0; i < N; ++i) {
        indicesTipo1[i] = i;
        if (pthread_create(&hilosTipo1[i], nullptr, tipo1, &indicesTipo1[i])) {
            std::cerr << "Error al crear el hilo de tipo 1, instancia " << i << std::endl;
            return 1;
        }
    }

    // Crear hilos de tipo 2
    for (int i = 0; i < M; ++i) {
        indicesTipo2[i] = i;
        if (pthread_create(&hilosTipo2[i], nullptr, tipo2, &indicesTipo2[i])) {
            std::cerr << "Error al crear el hilo de tipo 2, instancia " << i << std::endl;
            return 1;
        }
    }

    // Esperar a que los hilos de tipo 1 terminen
    for (int i = 0; i < N; ++i) {
        pthread_join(hilosTipo1[i], nullptr);
    }

    // Esperar a que los hilos de tipo 2 terminen
    for (int i = 0; i < M; ++i) {
        pthread_join(hilosTipo2[i], nullptr);
    }

    std::cout << "Se ha finalizado la ejecución" << std::endl;

    return 0;
}
