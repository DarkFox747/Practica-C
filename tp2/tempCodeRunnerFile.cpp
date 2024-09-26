
    // Esperar a que los hilos de tipo 2 terminen
    for (int i = 0; i < M; ++i) {
        pthread_join(hilosTipo2[i], nullptr);
    }