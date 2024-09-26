#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

const int NUM_JUGADORES = 3;
const int MAX_CARTA = 10;
const int LIMITE_PUNTUACION = 21;
const int NUM_RONDAS = 5; // Número de rondas que se jugarán

class Jugador {
private:
    int id;
    int puntuacion;
    int pipe_jugador[2]; // Pipe para la comunicación con el dealer
    bool se_planto; // Indica si el jugador se plantó en esta ronda

public:
    Jugador(int id) : id(id), puntuacion(0), se_planto(false) {
        // Crear el pipe para la comunicación con el dealer
        if (pipe(pipe_jugador) == -1) {
            std::cerr << "Error al crear el pipe para el jugador " << id << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    ~Jugador() {
        // Cerrar los descriptores de archivo del pipe
        close(pipe_jugador[0]);
        close(pipe_jugador[1]);
    }

    void recibir_carta(int carta) {
        puntuacion += carta;
        std::cout << "Jugador " << id << " recibe carta: " << carta << std::endl;
        std::cout << "Jugador " << id << " tiene una puntuación de " << puntuacion << std::endl;
    }

    int obtener_puntuacion() const {
        return puntuacion;
    }

    int obtener_id() const {
        return id;
    }

    int obtener_pipe_escritura() const {
        return pipe_jugador[1];
    }

    int obtener_pipe_lectura() const {
        return pipe_jugador[0];
    }

    bool se_planto_ronda() const {
        return se_planto;
    }

    void plantarse() {
        se_planto = true;
        std::cout << "Jugador " << id << " se ha plantado para esta ronda." << std::endl;
    }

    int tirarMoneda() const {
        return rand() % 2; // Retorna 0 o 1 aleatoriamente
    }
};

class Dealer {
private:
    std::vector<Jugador> jugadores;

public:
    Dealer() {
        for (int i = 0; i < NUM_JUGADORES; ++i) {
            jugadores.emplace_back(i);
        }
    }

    void iniciar_juego() {
        srand(time(nullptr));

        std::cout << "Número de jugadores: " << NUM_JUGADORES << std::endl;

        for (int ronda = 1; ronda <= NUM_RONDAS; ++ronda) {
            std::cout << "=== Ronda " << ronda << " ===" << std::endl;

            for (auto& jugador : jugadores) {
                if (ronda == 1) {
                    // Repartir las dos primeras cartas en la primera ronda
                    int carta1 = generar_carta();
                    int carta2 = generar_carta();
                    jugador.recibir_carta(carta1);
                    jugador.recibir_carta(carta2);
                } else {
                    // Permitir a los jugadores pedir una carta en rondas posteriores
                    if (!jugador.se_planto_ronda() && jugador.obtener_puntuacion() <= LIMITE_PUNTUACION) {
                        if (jugador.obtener_puntuacion() < 12) {
                            int carta = generar_carta();
                            jugador.recibir_carta(carta);
                        } else if (jugador.obtener_puntuacion() >= 12 && jugador.obtener_puntuacion() <= 17) {
                            // Ejecutar el método tirarMoneda para decidir si pedir carta
                            if (jugador.tirarMoneda() == 1) {
                                int carta = generar_carta();
                                jugador.recibir_carta(carta);
                            } else {
                                jugador.plantarse();
                            }
                        } else if (jugador.obtener_puntuacion() > 17) {
                            jugador.plantarse();
                        }
                    }
                }
            }

            // Verificar si todos los jugadores se han plantado o se han pasado de 21
            bool todos_plantados_o_pasaron = true;
            for (const auto& jugador : jugadores) {
                if (!jugador.se_planto_ronda() && jugador.obtener_puntuacion() <= LIMITE_PUNTUACION) {
                    todos_plantados_o_pasaron = false;
                    break;
                }
            }

            if (todos_plantados_o_pasaron) {
                std::cout << "Todos los jugadores se han plantado o se han pasado de 21 puntos." << std::endl;
                break; // Terminar el juego si todos los jugadores se plantaron o se pasaron
            }

            // Mostrar puntuaciones al final de cada ronda
            std::cout << "Puntuaciones al final de la ronda " << ronda << ":" << std::endl;
            for (const auto& jugador : jugadores) {
                std::cout << "Jugador " << jugador.obtener_id() << " tiene una puntuación de " << jugador.obtener_puntuacion() << std::endl;
            }
            std::cout << std::endl;
        }
    }

private:
    int generar_carta() {
        return rand() % MAX_CARTA + 1;
    }
};

int main() {
    Dealer dealer;
    dealer.iniciar_juego();

    return 0;
}
