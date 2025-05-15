#include <iostream>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>

#include <chrono>
#include <codecvt>
#include <windows.h>

#include <cstdlib>
#include <ctime>

using namespace std;


std::mutex mutexImpresora; // Mutex para asegurar exclusión mutua al acceder a la impresora o la cola
std::condition_variable condicion; // Variable de condición para que los empleados esperen hasta que la impresora esté disponible
bool impresora_disponible = true; // Bandera que indica si la impresora está libre o en uso
std::queue<int> cola_espera; // Cola que mantiene el orden de llegada de los empleados que están esperando para imprimir

// Función que imprime el estado actual de los empleados en espera.
void imprimir_estado_espera() {
    std::cout << "  >> Empleados esperando: ";
    std::queue<int> copia = cola_espera;  // Copia temporal para recorrer sin alterar la original
    while (!copia.empty()) {
        std::cout << copia.front() << " ";  // Mostrar el ID del primer empleado en espera
        copia.pop();                        // Avanzar al siguiente
    }
    std::cout << std::endl;
}

void empleado(int id) {
    // Simular preparación del documento (espera aleatoria)
    std::this_thread::sleep_for(std::chrono::milliseconds(500 + rand() % 1000));

    {
        std::unique_lock<std::mutex> lock(mutexImpresora);
        std::cout << "[IMPRESION] Empleado " << id << " está esperando para imprimir." << std::endl;
        cola_espera.push(id);
        imprimir_estado_espera();

        condicion.wait(lock, [&]() { return impresora_disponible && cola_espera.front() == id; });

        // Toma la impresora
        impresora_disponible = false;
        std::cout << "[IMPRESION] Empleado " << id << " está imprimiendo..." << std::endl;
        cola_espera.pop();
        imprimir_estado_espera();
    }

    // Simular impresión (tiempo aleatorio)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 + rand() % 2000));

    {
        std::unique_lock<std::mutex> lock(mutexImpresora);
        std::cout << "[IMPRESION] Empleado " << id << " ha terminado de imprimir." << std::endl;
        impresora_disponible = true;
        imprimir_estado_espera();  // Mostrar quién sigue esperando
        condicion.notify_all();    // Despertar a todos los que están esperando

    }
}

// ==== Estructura para procesos simulados ====

// Estados posibles de un proceso
enum EstadoProceso { NUEVO, LISTO, EJECUTANDO, SUSPENDIDO, TERMINADO };

// Estructura que representa un proceso con sus atributos esenciales
struct ProcesoSimulado {
    string id;                // Identificador del proceso
    int arrivalTime;         // Tiempo de llegada al sistema
    int burstTime;           // Tiempo total de CPU requerido
    int remainingTime;       // Tiempo restante de CPU
    EstadoProceso estado;    // Estado actual del proceso
};

// Lista global de procesos simulados
vector<ProcesoSimulado> procesos;
int tiempoGlobal = 0;  // Tiempo actual del sistema (puede usarse para estadísticas o planificación)


// Crea un nuevo proceso y lo añade a la lista en estado NUEVO.

void crearProceso(string id, int burstTime) {
    procesos.push_back({id, tiempoGlobal, burstTime, burstTime, NUEVO});
    cout << "Proceso " << id << " creado en estado NUEVO.\n";
}


// Muestra por consola todos los procesos y su estado actual.

void mostrarProcesos() {
    cout << "\nID\tEstado\t\tTiempo restante\n";
    for (const auto& p : procesos) {
        string estado;
        switch (p.estado) {
            case NUEVO: estado = "NUEVO"; break;
            case LISTO: estado = "LISTO"; break;
            case EJECUTANDO: estado = "EJECUTANDO"; break;
            case SUSPENDIDO: estado = "SUSPENDIDO"; break;
            case TERMINADO: estado = "TERMINADO"; break;
        }
        cout << p.id << "\t" << estado << "\t" << p.remainingTime << "\n";
    }
}


// Cambia el estado de un proceso de LISTO a SUSPENDIDO.

void suspenderProceso(string id) {
    for (auto& p : procesos) {
        if (p.id == id && p.estado == LISTO) {
            p.estado = SUSPENDIDO;
            cout << "Proceso " << id << " suspendido.\n";
            return;
        }
    }
    cout << "No se pudo suspender el proceso.\n";
}


//Cambia el estado de un proceso de SUSPENDIDO a LISTO.

void reanudarProceso(string id) {
    for (auto& p : procesos) {
        if (p.id == id && p.estado == SUSPENDIDO) {
            p.estado = LISTO;
            cout << "Proceso " << id << " reanudado.\n";
            return;
        }
    }
    cout << "No se pudo reanudar el proceso.\n";
}


//Termina manualmente un proceso, actualizando su estado y tiempo restante.

void terminarProceso(string id) {
    for (auto& p : procesos) {
        if (p.id == id && p.estado != TERMINADO) {
            p.estado = TERMINADO;
            p.remainingTime = 0;
            cout << "Proceso " << id << " terminado manualmente.\n";
            return;
        }
    }
    cout << "No se pudo terminar el proceso.\n";
}

// ==== Ejecución de procesos ====
// Simula la ejecución secuencial de procesos sin aplicar algoritmos avanzados de planificación
void planificarProcesos() {
    cout << "\nIniciando planificación (simulada)\n";

    // Cambiar el estado de todos los procesos NUEVOS a LISTOS
    for (auto& p : procesos) {
        if (p.estado == NUEVO) p.estado = LISTO;
    }

    // Ejecutar todos los procesos que están en estado LISTO, uno por uno
    for (auto& p : procesos) {
        if (p.estado == LISTO) {
            p.estado = EJECUTANDO;
            cout << "Ejecutando proceso: " << p.id << "\n";

            // Simula tiempo de CPU consumido (medio segundo)
            this_thread::sleep_for(chrono::milliseconds(500));

            // El proceso termina inmediatamente después de ejecutarse
            p.remainingTime = 0;
            p.estado = TERMINADO;
            cout << "Proceso " << p.id << " terminado.\n";
        }
    }
}

// ==== Planificación Round Robin ====
// Simula la planificación de procesos usando el algoritmo Round Robin con quantum fijo
void ejecutarRoundRobin() {

    // Estructura auxiliar para mantener información adicional por proceso durante la simulación
    struct ProcesoRR {
        string id;
        int arrivalTime;
        int burstTime;
        int remainingTime;
        int completionTime;
        int turnAroundTime;
        int waitingTime;
    };

    // Crear lista de procesos que aún no han terminado
    vector<ProcesoRR> rrProcesos;
    for (const auto& p : procesos) {
        if (p.estado != TERMINADO) {
            rrProcesos.push_back({p.id, p.arrivalTime, p.burstTime, p.burstTime});
        }
    }

    int quantum = 2;                  // Tiempo fijo de CPU por proceso
    int tiempo = tiempoGlobal;       // Simula el tiempo actual del sistema
    cout << "\nEjecutando Round Robin con quantum = " << quantum << "\n";

    // Ejecutar los procesos cíclicamente hasta que todos terminen
    bool quedaTrabajo;
    do {
        quedaTrabajo = false;
        for (auto& p : rrProcesos) {
            if (p.remainingTime > 0) {
                quedaTrabajo = true;
                int ejecucion = min(quantum, p.remainingTime);
                cout << "Ejecutando " << p.id << " por " << ejecucion << " unidades de tiempo.\n";

                // Simula el uso de CPU
                this_thread::sleep_for(chrono::milliseconds(300));

                // Avanza el tiempo del sistema
                tiempo += ejecucion;
                p.remainingTime -= ejecucion;

                // Si el proceso ha terminado, se calculan métricas
                if (p.remainingTime == 0) {
                    p.completionTime = tiempo;
                    p.turnAroundTime = p.completionTime - p.arrivalTime;
                    p.waitingTime = p.turnAroundTime - p.burstTime;
                }
            }
        }
    } while (quedaTrabajo);  // Repetir hasta que todos los procesos hayan terminado

    // Mostrar tabla de resultados
    int totalWT = 0, totalTAT = 0;
    cout << "\nProceso\tLlegada\tEjecución\tFinalización\tRetorno\tEspera\n";
    for (auto& p : rrProcesos) {
        cout << p.id << "\t" << p.arrivalTime << "\t" << p.burstTime
             << "\t\t" << p.completionTime << "\t\t" << p.turnAroundTime
             << "\t" << p.waitingTime << "\n";
        totalWT += p.waitingTime;
        totalTAT += p.turnAroundTime;
    }

    // Mostrar promedios
    cout << "\nTiempo promedio de espera: " << (float)totalWT / rrProcesos.size() << " unidades\n";
    cout << "Tiempo promedio de retorno: " << (float)totalTAT / rrProcesos.size() << " unidades\n";

    // Marcar todos los procesos como TERMINADOS en la lista principal
    for (auto& p : procesos) {
        if (p.estado != TERMINADO)
            p.estado = TERMINADO, p.remainingTime = 0;
    }
}

// ==== Planificación SJF ====
// Simula la planificación de procesos usando el algoritmo Shortest Job First (sin desalojo)
void ejecutarSJF() {

    // Estructura auxiliar para representar un proceso con sus métricas temporales
    struct ProcesoSJF {
        string id;
        int arrivalTime;
        int burstTime;
        int completionTime;
        int turnAroundTime;
        int waitingTime;
    };

    vector<ProcesoSJF> sjfProcesos;

    // Copiar todos los procesos no terminados a una nueva lista para la simulación
    for (const auto& p : procesos) {
        if (p.estado != TERMINADO) {
            sjfProcesos.push_back({p.id, p.arrivalTime, p.burstTime});
        }
    }

    cout << "\nEjecutando SJF (Shortest Job First)\n";

    // Orden inicial por tiempo de llegada
    sort(sjfProcesos.begin(), sjfProcesos.end(), [](const ProcesoSJF& a, const ProcesoSJF& b) {
        return a.arrivalTime < b.arrivalTime;
    });

    int tiempo = 0;

    // Ejecutar todos los procesos según menor burstTime disponible
    for (size_t i = 0; i < sjfProcesos.size(); ++i) {
        // Reordenar desde la posición actual según burstTime (solo los que han llegado)
        sort(sjfProcesos.begin() + i, sjfProcesos.end(), [](const ProcesoSJF& a, const ProcesoSJF& b) {
            return a.burstTime < b.burstTime;
        });

        // Esperar si el proceso aún no ha llegado
        tiempo = max(tiempo, sjfProcesos[i].arrivalTime);

        // Calcular métricas de ejecución
        sjfProcesos[i].completionTime = tiempo + sjfProcesos[i].burstTime;
        sjfProcesos[i].turnAroundTime = sjfProcesos[i].completionTime - sjfProcesos[i].arrivalTime;
        sjfProcesos[i].waitingTime = sjfProcesos[i].turnAroundTime - sjfProcesos[i].burstTime;
        tiempo = sjfProcesos[i].completionTime;

        cout << "Ejecutando " << sjfProcesos[i].id << " por " << sjfProcesos[i].burstTime << " unidades.\n";
        this_thread::sleep_for(chrono::milliseconds(300));  // Simula tiempo de CPU
    }

    // Mostrar resultados
    int totalWT = 0, totalTAT = 0;
    cout << "\nProceso\tLlegada\tEjecución\tFinalización\tRetorno\tEspera\n";
    for (auto& p : sjfProcesos) {
        cout << p.id << "\t" << p.arrivalTime << "\t" << p.burstTime
             << "\t\t" << p.completionTime << "\t\t" << p.turnAroundTime
             << "\t" << p.waitingTime << "\n";
        totalWT += p.waitingTime;
        totalTAT += p.turnAroundTime;
    }

    // Mostrar promedios finales
    cout << "\nTiempo promedio de espera: " << (float)totalWT / sjfProcesos.size() << " unidades\n";
    cout << "Tiempo promedio de retorno: " << (float)totalTAT / sjfProcesos.size() << " unidades\n";

    // Actualizar estado de los procesos originales como TERMINADOS
    for (auto& p : procesos) {
        if (p.estado != TERMINADO)
            p.estado = TERMINADO, p.remainingTime = 0;
    }
}


// ==== Memoria FIFO ====
void simularFIFO() {

    vector<int> pageReferences = {7,0,1,2,0,3,0,4,2,3,0,3,2};
    int numFrames = 4, pageFaults = 0;
    unordered_set<int> pageSet;
    queue<int> pageQueue;

    // Iterar sobre cada referencia de página en la secuencia de acceso
    for (int page : pageReferences) {
        //imprimir estado actual de memoria
        cout << "\n Estado actual de memoria: ";
        queue<int> tempQueue = pageQueue;  // Crear una copia de la cola para no modificar la original
        while (!tempQueue.empty()) {
            cout << tempQueue.front() << " ";
            tempQueue.pop();
        }
        // Verificar si la página ya está en memoria
        if (pageSet.find(page) == pageSet.end()) {
            // La página no está en memoria, por lo que hay un fallo de página
            pageFaults++;

            // Si la memoria está llena (ya hay numFrames páginas en la cola), se debe eliminar la más antigua
            if (pageQueue.size() == numFrames) {
                int oldestPage = pageQueue.front(); // Obtener la página más antigua (FIFO)
                pageQueue.pop();  // Eliminarla de la cola
                pageSet.erase(oldestPage); // Eliminarla del conjunto de páginas en memoria
                cout << "\n Página " << oldestPage << " reemplazada por " << page << ".";
            } else {
                cout << "\n Página " << page << " cargada en un marco vacío.";
            }

            // Agregar la nueva página a la cola y al conjunto
            pageQueue.push(page);
            pageSet.insert(page);
        } else {
            // Si la página ya está en memoria, solo informar que no hay fallo de página
            cout << "\n Página " << page << " ya en memoria.";
        }

        // Imprimir el estado actual de la memoria con el formato deseado
        cout << "\n Memoria: ";
        tempQueue = pageQueue;  // Crear una copia de la cola para no modificar la original
        while (!tempQueue.empty()) {
            cout << tempQueue.front() << " ";
            tempQueue.pop();
        }
        cout << "| Fallos: " << pageFaults << endl;
    }

    // Mostrar el total de fallos de página al final del proceso
    cout << "\n Total de fallos de página: " << pageFaults << endl;
}

// ==== Memoria LRU ====

void simularLRU() {

    vector<int> pages = {7,0,1,2,0,3,0,4,2,3,0,3,2};
    int frames = 4, faults = 0;
    list<int> memory;
    unordered_map<int, list<int>::iterator> pos;
    unordered_map<int, list<int>::iterator> pageMap; // Mapa para rastrear posiciones en la lista
    int pageFaults = 0; // Contador de fallos de página
    int frameCount = 4;

    // Recorremos cada página en la secuencia de paginas
    for (int page : pages) {
        // Mostrar estado antes de procesar la página
        cout << "\nProcesando página: " << page << endl;
        cout << "Estado previo del caché: ";
        for (int p : memory) cout << p << " "; // Mostramos el contenido actual del caché
        cout << endl;

        // Si la página ya está en memoria, se mueve al frente más recientemente utilizada
        if (pageMap.find(page) != pageMap.end()) {
            memory.erase(pageMap[page]); // Eliminar la página de su posición actual
            cout << "Página " << page << " ya estaba en memoria (actualizada a más reciente).\n";
        } else {
            // Si la memoria está llena, eliminar la página menos utilizada
            if (memory.size() >= frameCount) {
                int lruPage = memory.back(); // Página menos utilizada que es la última en la lista
                memory.pop_back();           // Eliminar de la memoria
                pageMap.erase(lruPage);      // Quitar del mapa
                cout << "Página " << lruPage << " reemplazada por " << page << ".\n";
            }
            pageFaults++; // Aumentar un fallo de página
            cout << "Página " << page << " cargada en memoria.\n";
        }

        // Agregamos la nueva página al frente de la lista
        memory.push_front(page);
        pageMap[page] = memory.begin(); // Actualizamos la posición en el mapa

        // Mostramos el estado actual de la memoria
        cout << "Estado actual del caché: ";
        for (int p : memory) cout << p << " "; // Mostramos la memoria después del cambio
        cout << "| Fallos: " << pageFaults << endl;
    }

    // Mostrar el total de fallos de página al final
    cout << "\nTotal de fallos de página: " << pageFaults << endl;
}
// ==== Productor-Consumidor ====

// Recursos compartidos
mutex mtx;                         // Mutex para proteger el acceso concurrente al buffer
condition_variable cv;            // Condición para sincronizar productor y consumidor
queue<int> buffer;                // Buffer compartido (cola de enteros)
const int BUFFER_SIZE = 5;        // Tamaño máximo del buffer
bool terminado = false;           // Señal para indicar que el productor ha terminado

/**
 * Función del productor.
 * Inserta 10 elementos en el buffer, esperando si el buffer está lleno.
 */
void productor() {
    for (int i = 1; i <= 10; ++i) {
        unique_lock<mutex> lock(mtx);

        // Esperar mientras el buffer esté lleno
        cv.wait(lock, [] { return buffer.size() < BUFFER_SIZE; });

        buffer.push(i);
        cout << "Productor produce: " << i << "\n";

        cv.notify_all();  // Notificar al consumidor que hay datos
    }

    // Indicar que se terminó de producir
    terminado = true;
    cv.notify_all();
}

/**
 * Función del consumidor.
 * Consume elementos del buffer mientras no se haya terminado de producir o el buffer esté vacío.
 */
void consumidor() {
    while (!terminado || !buffer.empty()) {
        unique_lock<mutex> lock(mtx);

        // Esperar hasta que haya elementos o el productor haya terminado
        cv.wait(lock, [] { return !buffer.empty() || terminado; });

        // Consumir todos los elementos disponibles
        while (!buffer.empty()) {
            cout << "Consumidor consume: " << buffer.front() << "\n";
            buffer.pop();
        }

        cv.notify_all();  // Notificar al productor que hay espacio
    }
}

/**
 * Lanza los hilos de productor y consumidor, y espera a que terminen.
 */
void simularProductorConsumidor() {
    cout << "\nSimulando Productor-Consumidor...\n";
    thread prod(productor), cons(consumidor);
    prod.join();  // Esperar a que el productor termine
    cons.join();  // Esperar a que el consumidor termine
}



// ==== Planificación de Disco FCFS / SSTF ====
// Simula el comportamiento de un disco duro usando los algoritmos FCFS y SSTF
void simularDisco() {

    // Lista de solicitudes de acceso a pistas del disco
    vector<int> solicitudes = {95, 180, 34, 119, 11, 123, 62, 64};
    int cabeza = 50;  // Posición inicial del cabezal del disco

    // --- Algoritmo FCFS (First-Come, First-Served) ---
    // Atiende las solicitudes en el orden en que llegan
    cout << "\nSimulación FCFS:\nSecuencia: ";
    for (int s : solicitudes)
        cout << s << " ";

    // --- Algoritmo SSTF (Shortest Seek Time First) ---
    // Atiende la solicitud más cercana a la posición actual del cabezal
    cout << "\n\nSimulación SSTF:\nSecuencia: ";
    vector<int> sstf = solicitudes;
    while (!sstf.empty()) {
        // Encontrar la solicitud más cercana al cabezal actual
        auto it = min_element(sstf.begin(), sstf.end(), [cabeza](int a, int b) {
            return abs(a - cabeza) < abs(b - cabeza);
        });

        // Mover el cabezal hacia la solicitud seleccionada
        cout << *it << " ";
        cabeza = *it;
        sstf.erase(it);  // Eliminar solicitud atendida
    }

    cout << "\n";
}
// Número total de filósofos (y tenedores)
const int NUM_FILOSOFOS = 5;

// Cada tenedor se representa con un mutex
mutex tenedores[NUM_FILOSOFOS];

/**
 * Función que simula el comportamiento de un filósofo.
 * Cada filósofo alterna entre pensar y comer, asegurando el acceso exclusivo a los tenedores adyacentes.
 */
void filosofo(int id) {
    for (int i = 0; i < 3; ++i) {
        cout << "Filósofo " << id << " está pensando...\n";
        this_thread::sleep_for(chrono::milliseconds(500));  // Simula tiempo de pensamiento

        int izq = id;                         // Índice del tenedor izquierdo
        int der = (id + 1) % NUM_FILOSOFOS;   // Índice del tenedor derecho (circular)

        // Para evitar interbloqueos, los filósofos pares toman primero el tenedor izquierdo,
        // y los impares el derecho. Esto rompe el ciclo de espera circular.
        if (id % 2 == 0) {
            lock(tenedores[izq], tenedores[der]);
            lock_guard<mutex> lockIzq(tenedores[izq], adopt_lock);
            lock_guard<mutex> lockDer(tenedores[der], adopt_lock);
        } else {
            lock(tenedores[der], tenedores[izq]);
            lock_guard<mutex> lockDer(tenedores[der], adopt_lock);
            lock_guard<mutex> lockIzq(tenedores[izq], adopt_lock);
        }

        // Sección crítica: el filósofo está comiendo
        cout << "Filósofo " << id << " está comiendo...\n";
        this_thread::sleep_for(chrono::milliseconds(500));

        cout << "Filósofo " << id << " ha terminado de comer.\n";
        // Al salir del bloque lock_guard, se liberan automáticamente los tenedores
    }
}

/**
 * Lanza un hilo por cada filósofo y simula la cena completa.
 */
void simularCenaFilosofos() {
    cout << "\nSimulando Cena de los Filósofos...\n";

    vector<thread> filosofos;

    // Crear hilos para cada filósofo
    for (int i = 0; i < NUM_FILOSOFOS; ++i)
        filosofos.emplace_back(filosofo, i);

    // Esperar que todos terminen
    for (auto& t : filosofos)
        t.join();
}



// ==== CLI principal ====
// Muestra el menú principal del simulador de kernel
void mostrarMenu() {
    cout << "\n===== SIMULADOR DE KERNEL =====\n";
    cout << "1. Gestión de Procesos\n";
    cout << "2. Ejecutar\n";
    cout << "3. Memoria\n";
    cout << "4. Simular E/S (Impresora)\n";
    cout << "5. Planificación Disco (FCFS/SSTF)\n";
    cout << "6. Simular Productor-Consumidor\n";
    cout << "7. Simular Cena de Filósofos\n";
    cout << "8. Salir\n";
    cout << "Seleccione opción: ";
}

/**
 * Muestra un submenú para la gestión de procesos:
 * crear, suspender, reanudar, terminar y ver procesos.
 */
void menuGestionProcesos() {
    int opcion;
    string id;

    do {
        cout << "\n=== Gestión de Procesos ===\n";
        cout << "1. Crear Proceso\n";
        cout << "2. Suspender Proceso\n";
        cout << "3. Reanudar Proceso\n";
        cout << "4. Terminar Proceso\n";
        cout << "5. Ver Procesos\n";
        cout << "6. Regresar\n";
        cout << "Seleccione opción: ";
        cin >> opcion;

        int tiempo;
        switch (opcion) {
            case 1:
                cout << "ID del proceso: ";
                cin >> id;
                cout << "Tiempo de CPU: ";
                cin >> tiempo;
                crearProceso(id, tiempo);
                break;
            case 2:
                cout << "ID del proceso a suspender: ";
                cin >> id;
                suspenderProceso(id);
                break;
            case 3:
                cout << "ID del proceso a reanudar: ";
                cin >> id;
                reanudarProceso(id);
                break;
            case 4:
                cout << "ID del proceso a terminar: ";
                cin >> id;
                terminarProceso(id);
                break;
            case 5:
                mostrarProcesos();
                break;
            case 6:
                break;  // Regresa al menú principal
            default:
                cout << "Opción inválida.\n";
        }
    } while (opcion != 6);
}

/**
 * Submenú para ejecutar algoritmos de planificación de procesos:
 * Básica, Round Robin y SJF.
 */
void menuEjecutar() {
    int opcion;
    do {
        cout << "\n=== Ejecutar Planificación ===\n";
        cout << "1. Planificación Básica\n";
        cout << "2. Planificación Round Robin\n";
        cout << "3. Planificación SJF\n";
        cout << "4. Regresar\n";
        cout << "Seleccione opción: ";
        cin >> opcion;

        switch (opcion) {
            case 1:
                planificarProcesos();
                break;
            case 2:
                ejecutarRoundRobin();
                break;
            case 3:
                ejecutarSJF();
                break;
            case 4:
                break;  // Regresar al menú principal
            default:
                cout << "Opción inválida.\n";
        }
    } while (opcion != 4);
}

/**
 * Submenú para simulación de algoritmos de administración de memoria:
 * FIFO y LRU.
 */
void menuMemoria() {
    int opcion;
    do {
        cout << "\n=== Memoria ===\n";
        cout << "1. Memoria FIFO\n";
        cout << "2. Memoria LRU\n";
        cout << "3. Regresar\n";
        cout << "Seleccione opción: ";
        cin >> opcion;

        switch (opcion) {
            case 1:
                simularFIFO();
                break;
            case 2:
                simularLRU();
                break;
            case 3:
                break;  // Regresar al menú principal
            default:
                cout << "Opción inválida.\n";
        }
    } while (opcion != 3);
}


int main() {
    // Configuración para permitir la salida en UTF-8 en consola de Windows (comentada actualmente)
    //SetConsoleOutputCP(CP_UTF8);
    //SetConsoleCP(CP_UTF8);
    //std::locale::global(std::locale(""));
    //std::wcout.imbue(std::locale(""));

    std::vector<std::thread> hilos;           // Vector para almacenar los hilos del sistema de impresión
    srand(time(nullptr));                     // Inicializa la semilla para generar números aleatorios
    const int num_empleados = 5;              // Número de empleados para el sistema de impresión
    int opcion;                               // Variable para guardar la opción del menú

    do {
        mostrarMenu();                        // Muestra el menú principal
        cin >> opcion;                        // Lee la opción del usuario

        switch (opcion) {
            case 1:
                menuGestionProcesos();        // Ejecuta el submenú de planificación de procesos (RR, SJF, etc.)
                break;
            case 2:
                menuEjecutar();               // Ejecuta programas o simulaciones específicas (podrías tener un submenú aquí)
                break;
            case 3:
                menuMemoria();                // Ejecuta el submenú de gestión de memoria virtual (FIFO, LRU, etc.)
                break;
            case 4:
                // Simulación del sistema de impresión compartida usando múltiples hilos
                std::cout << "=== SISTEMA DE IMPRESION COMPARTIDA ===\n";
                for (int i = 1; i <= num_empleados; ++i) {
                    hilos.emplace_back(empleado, i);  // Crea un hilo por cada empleado
                }
                for (auto& t : hilos) {
                    t.join();                         // Espera que todos los hilos terminen
                }
                break;
            case 5:
                simularDisco();               // Simula el acceso a disco usando algoritmos FCFS y SSTF
                break;
            case 6:
                simularProductorConsumidor(); // Ejecuta la simulación del problema productor-consumidor
                break;
            case 7:
                simularCenaFilosofos();       // Ejecuta la simulación de la cena de los filósofos
                break;
            case 8:
                cout << "¡Gracias por usar nuestro Sistema Operativo!\n"; // Mensaje de despedida
                break;
            default:
                cout << L"La opción es inválida, por favor intenta de nuevo.\n"; // Opción no válida
        }
    } while (opcion != 8); // Repite mientras no se elija salir (opción 8)

    return 0; // Fin del programa
}
