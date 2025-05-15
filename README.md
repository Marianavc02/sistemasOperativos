# Simulador de Núcleo de Sistema Operativo

Este proyecto implementa un simulador de distintos procesos del núcleo de un sistema operativo, incluyendo:

- **Planificación de procesos** (Round Robin, SJF)  
- **Gestión de memoria virtual** (FIFO, LRU)  
- **Sistema de impresión compartida**  
- **Planificación de disco** (FCFS, SSTF)  
- **Simulador Productor-Consumidor**  
- **Cena de los Filósofos**  

Cada módulo puede ser ejecutado desde un menú interactivo en consola.

---

## 👥 Integrantes

- Camilo Salazar  
- Alexandra Hurtado  
- Mariana Valderrama  

---

## 📦 Requisitos

- Compilador C++ (G++ recomendado)
- Sistema operativo compatible con C++11 o superior
- Terminal que soporte ejecución multihilo

---
## Nota importante para el usuario 
Para ejecutar la opción 2 del menú (Ejecutar procesos del simulador de kernel), debes primero crear procesos utilizando la opción 1.

Si ejecutas la opción 2 y el proceso se termina, todos los procesos previamente creados se eliminan y no se ejecutarán nuevamente.

Por lo tanto, si deseas volver a ejecutar la opción 2, debes repetir la creación de procesos desde la opción 1 antes de continuar.

## ⚙️ Compilación

Para compilar el proyecto, usa el siguiente comando en la terminal:

```bash
g++ -std=c++11 -pthread main.cpp -o simulador

