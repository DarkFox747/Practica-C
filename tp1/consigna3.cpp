#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include <random>
#include <vector>

using namespace std;

class Proceso {
private:
    int id;
    int procesoPausar;

public:
    Proceso(int id, int procesoPausar) : id(id), procesoPausar(procesoPausar) {}

    void saludo() {
        int demora = 150;
        int veces = generarAleatorio(5, 10);
        bool pausado = false;

        for(int i = 1; i <= veces; i++) {
            if (!pausado && id == procesoPausar) {
                this_thread::sleep_for(chrono::milliseconds(600));
                pausado = true;
            }
            cout << "Soy el proceso " + to_string(id) + "\t";
            this_thread::sleep_for(chrono::milliseconds(demora));
        }
    }

    static int generarAleatorio(int min, int max) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(min, max);
        return dis(gen);
    }
};

int main() {
    const int num_procesos = 15;
    vector<thread> P(num_procesos);

    int procesoPausar;
    cout << "Ingrese el numero del proceso que se va a pausar (1-15): ";
    cin >> procesoPausar;

    // Validar que el proceso ingresado este en el rango permitido
    if (procesoPausar < 1 || procesoPausar > num_procesos) {
        cerr << "Numero de proceso no valido. Debe estar entre 1 y 15." << endl;
        return 1;
    }

    // Crear y lanzar los hilos
    for(int i = 0; i < num_procesos; i++) {
        P[i] = thread(&Proceso::saludo, Proceso(i + 1, procesoPausar));
    }

    // Esperar a que todos los hilos terminen
    for(int i = 0; i < num_procesos; i++) {
        P[i].join();
    }
    
    cout << "\n";
    cout << "Fin \n";

    return 0;
}