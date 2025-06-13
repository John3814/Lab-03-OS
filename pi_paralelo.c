#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define REPETICIONES 30  // Cuantas veces repetir cada configuracion

// Estructura de datos que se pasara a cada hilo
typedef struct {
    int id;               // Identificador del hilo
    int n;                // Numero total de pasos (precision)
    int num_threads;      // Numero total de hilos
    double h;             // Ancho de cada paso (1.0 / n)
    double sum;           // Suma parcial calculada por este hilo
} ThreadData;

// Funcion que evalua f(x) = 4 / (1 + x^2)
double f(double a) {
    return (4.0 / (1.0 + a * a));
}

// Funcion que ejecuta cada hilo
void* thread_func(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    int id = data->id;
    int n = data->n;
    int num_threads = data->num_threads;
    double h = data->h;
    double local_sum = 0.0;

    // Cada hilo procesa indices separados por num_threads
    for (int i = id; i < n; i += num_threads) {
        double x = h * ((double)i + 0.5);  // Punto medio
        local_sum += f(x);
    }

    data->sum = local_sum;  // Guarda la suma parcial
    return NULL;
}

// Calcula pi usando varios hilos
double CalcPi(int n, int num_threads) {
    pthread_t threads[num_threads];         // Arreglo de hilos
    ThreadData thread_data[num_threads];    // Datos para cada hilo
    double h = 1.0 / (double)n;
    double total_sum = 0.0;

    // Crear los hilos
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].id = i;
        thread_data[i].n = n;
        thread_data[i].num_threads = num_threads;
        thread_data[i].h = h;
        thread_data[i].sum = 0.0;
        pthread_create(&threads[i], NULL, thread_func, &thread_data[i]);
    }

    // Esperar a que todos los hilos terminen y acumular resultados
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_sum += thread_data[i].sum;
    }

    return h * total_sum;  // Resultado final
}

int main() {
    const int n = 2000000000;  // Numero de pasos de integracion (valor original solicitado)
    const double fPi25DT = 3.141592653589793238462643; // Valor de referencia de pi
    const int hilos_prueba[] = {1, 2, 4, 8, 16};
    const int total_configuraciones = sizeof(hilos_prueba) / sizeof(hilos_prueba[0]);
    struct timespec start, end;
    double tiempo;

    // Abrir archivo de resultados
    FILE* archivo = fopen("resultados.txt", "w");
    if (archivo == NULL) {
        perror("Error al abrir archivo");
        return 1;
    }

    // Encabezado del archivo
    fprintf(archivo, "Hilos\tIteracion\tTiempo(s)\tPi\tError\n");

    // Bucle por cada numero de hilos
    for (int h = 0; h < total_configuraciones; h++) {
        int num_hilos = hilos_prueba[h];

        // Repetir la medicion varias veces
        for (int i = 0; i < REPETICIONES; i++) {
            clock_gettime(CLOCK_MONOTONIC, &start);  // Tiempo inicio
            double pi = CalcPi(n, num_hilos);    // Calculo de pi
            clock_gettime(CLOCK_MONOTONIC, &end);    // Tiempo fin
        
            tiempo = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            double error = fabs(pi - fPi25DT);       // Calculo del error

            // Guardar en archivo
            fprintf(archivo, "%d\t%d\t%.20f\t%.20f\t%.20f\n", num_hilos, i + 1, tiempo, pi, error);

            // Mostrar en pantalla
            printf("Hilos: %2d | Iteracion: %2d | Tiempo: %.20f s | Pi \u2248 %.20f | Error: %.20f\n", num_hilos, i + 1, tiempo, pi, error);
        }
    }

    fclose(archivo);  // Cerrar archivo
    printf("\nResultados guardados en 'resultados.txt'\n");

    return 0;
}
