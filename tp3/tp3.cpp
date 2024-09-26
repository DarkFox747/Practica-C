#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <stdexcept>

const int NUM_JUGADORES = 3;  // Número de jugadores en el juego
const int MAX_CARTA = 10;     // Valor máximo de una carta
const int LIMITE_PUNTUACION = 21;  // Límite de puntuación para los jugadores
const int NUM_RONDAS = 5;     // Número de rondas en el juego

// Clase que representa a un jugador
class Jugador {
private:
    int id;  // Identificador del jugador
    int puntuacion;  // Puntuación actual del jugador
    bool se_planto;  // Indicador de si el jugador se plantó
    int pipe_lectura[2];  // Pipe para lectura
    int pipe_escritura[2];  // Pipe para escritura

public:
    // Constructor que inicializa el jugador
    Jugador(int id) : id(id), puntuacion(0), se_planto(false) {
        // Creación de los pipes y manejo de errores
        if (pipe(pipe_lectura) == -1 || pipe(pipe_escritura) == -1) {
            throw std::runtime_error("Error al crear los pipes para el jugador " + std::to_string(id));
        }
    }

    // Método para recibir una carta y actualizar la puntuación
    void recibir_carta(int carta) {
        if (!se_planto) {
            puntuacion += carta;
            std::cout << "Jugador " << id << " recibe carta: " << carta << std::endl;
            std::cout << "Jugador " << id << " tiene una puntuación de " << puntuacion << std::endl;
        }
    }

    // Método para obtener la puntuación actual del jugador
    int obtener_puntuacion() const {
        return puntuacion;
    }

    // Método para obtener el ID del jugador
    int obtener_id() const {
        return id;
    }

    // Método para verificar si el jugador se plantó en esta ronda
    bool se_planto_ronda() const {
        return se_planto;
    }

    // Método para que el jugador se plante
    void plantarse() {
        se_planto = true;
        std::cout << "Jugador " << id << " se ha plantado." << std::endl;
    }

    // Método para obtener el pipe de lectura
    int* obtener_pipe_lectura() {
        return pipe_lectura;
    }

    // Método para obtener el pipe de escritura
    int* obtener_pipe_escritura() {
        return pipe_escritura;
    }

    // Método para cerrar los pipes del jugador
    void cerrar_pipes() {
        close(pipe_lectura[0]);
        close(pipe_lectura[1]);
        close(pipe_escritura[0]);
        close(pipe_escritura[1]);
    }
};

// Clase que representa el juego
class Juego {
private:
    std::vector<Jugador> jugadores;  // Vector de jugadores
    std::vector<pid_t> pid_jugadores;  // Vector de PIDs de los jugadores

public:
    // Constructor que inicializa el juego con el número de jugadores
    Juego(int num_jugadores) {
        for (int i = 0; i < num_jugadores; ++i) {
            jugadores.emplace_back(i);
        }
    }

    // Método para iniciar el juego
    void iniciar_juego() {
        srand(time(nullptr));  // Inicialización del generador de números aleatorios

        // Creación de procesos hijos para cada jugador
        for (auto& jugador : jugadores) {
            pid_t pid = fork();
            if (pid == 0) {  // Proceso hijo
                close(jugador.obtener_pipe_lectura()[1]);
                close(jugador.obtener_pipe_escritura()[0]);
                while (true) {
                    int carta;
                    read(jugador.obtener_pipe_lectura()[0], &carta, sizeof(carta));  // Leer carta del pipe
                    if (carta == -1) break;  // Señal de fin del juego
                    jugador.recibir_carta(carta);
                    if (jugador.obtener_puntuacion() >= 17) {
                        jugador.plantarse();
                    }
                    if (jugador.se_planto_ronda() || jugador.obtener_puntuacion() >= 17) {
                        carta = -1;  // Señal de que el jugador se ha plantado o superado el límite
                    }
                    write(jugador.obtener_pipe_escritura()[1], &carta, sizeof(carta));  // Enviar respuesta al pipe
                }
                jugador.cerrar_pipes();  // Cerrar pipes y salir del proceso hijo
                exit(0);
            } else if (pid < 0) {
                throw std::runtime_error("Error al crear el proceso hijo para el jugador " + std::to_string(jugador.obtener_id()));
            }
            close(jugador.obtener_pipe_lectura()[0]);
            close(jugador.obtener_pipe_escritura()[1]);
            pid_jugadores.push_back(pid);  // Almacenar PID del proceso hijo
        }

        // Bucle principal del juego por rondas
        for (int ronda = 1; ronda <= NUM_RONDAS; ++ronda) {
            std::cout << "=== Ronda " << ronda << " ===" << std::endl;

            bool todos_se_plantaron_o_se_pasaron = true;

            // Repartir cartas a los jugadores
            for (auto& jugador : jugadores) {
                if (!jugador.se_planto_ronda()) {
                    int carta = generar_carta();
                    write(jugador.obtener_pipe_lectura()[1], &carta, sizeof(carta));  // Enviar carta al pipe
                    todos_se_plantaron_o_se_pasaron = false;
                }
            }

            if (todos_se_plantaron_o_se_pasaron) {
                break;  // Terminar el juego si todos los jugadores se plantaron o se pasaron
            }

            // Leer respuestas de los jugadores
            for (auto& jugador : jugadores) {
                int response;
                read(jugador.obtener_pipe_escritura()[0], &response, sizeof(response));
                if (response == -1) {
                    std::cout << "Jugador " << jugador.obtener_id() << " se ha plantado o superado el límite de 21 puntos." << std::endl;
                } else {
                    jugador.recibir_carta(response);
                }
            }

            // Mostrar puntuaciones al final de la ronda
            std::cout << "Puntuaciones al final de la ronda " << ronda << ":" << std::endl;
            for (const auto& jugador : jugadores) {
                std::cout << "Jugador " << jugador.obtener_id() << " tiene una puntuación de " << jugador.obtener_puntuacion() << std::endl;
            }
            std::cout << std::endl;
        }

        // Enviar señal de fin del juego a todos los jugadores
        for (auto& jugador : jugadores) {
            int end_signal = -1;
            write(jugador.obtener_pipe_lectura()[1], &end_signal, sizeof(end_signal));
            close(jugador.obtener_pipe_lectura()[1]);
            close(jugador.obtener_pipe_escritura()[0]);
        }

        // Esperar a que todos los procesos hijos terminen
        for (pid_t pid : pid_jugadores) {
            waitpid(pid, nullptr, 0);
        }

        // Mostrar puntuaciones finales
        std::cout << "Puntuaciones finales:" << std::endl;
        for (const auto& jugador : jugadores) {
            std::cout << "Jugador " << jugador.obtener_id() << " tiene una puntuación total de " << jugador.obtener_puntuacion() << std::endl;
        }
    }

private:
    // Método para generar una carta aleatoria
    int generar_carta() {
        return rand() % MAX_CARTA + 1;
    }
};

int main() {
    try {
        Juego juego(NUM_JUGADORES);  // Crear el juego con el número de jugadores
        juego.iniciar_juego();  // Iniciar el juego
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
