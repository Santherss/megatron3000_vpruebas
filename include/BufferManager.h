#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include <string>
#include <stdio.h>
#include <queue>
#include <vector>
#include <algorithm>
#include "DiscoFisico.h"

using namespace std;

// --- ENUMS Y ESTRUCTURAS AUXILIARES ---
enum class Operacion { Leer, Escribir, Eliminar, Insertar };
enum class Politica { LRU, CLOCK };

struct CandidatoLRU {
    int frame_id;
    int last_used;
};

// --- DECLARACIONES DE CLASES ---
class BufferManager;
class BufferPool;
class Frame;
class Page;

// ======================================================
// Frame: La base con la nueva lógica de procesos
// ======================================================
class Frame {
private:
    int id;
    Page* pagina;
    bool dirty_bit;
    int last_used;
    bool reference_bit;
    std::queue<Operacion> procesos; // <-- La nueva fuente de verdad

public:
    Frame(int i);
    Frame();
    ~Frame();

    void ver_atributos();
    int get_id();
    string* get_puntero();
    bool set_pagina(Page* p);
    void reset_frame();
    
    // Getters y Setters de metadatos
    bool get_dirty_bit();
    int get_last_used();
    bool get_reference_bit();
    void set_last_used(int time);
    void set_reference_bit(bool value);
    void high_dirty_bit();
    void low_dirty_bit();
    
    // Interfaz de gestión de procesos
    void agregar_proceso(Operacion op);
    void terminar_proceso_antiguo();
    int get_pin_count();
    bool get_is_pin();
    string get_procesos_str();
};

// ======================================================
// BufferPool: Interfaz pública actualizada
// ======================================================
class BufferPool {
private:
    Frame* listaBuffer;
    int num_frames;
    int num_hit;
    int num_miss;
    int tiempo_global;
    int clock_hand;
    Politica politica;

public:
    BufferPool(int num_frames, Politica modo_politica);
    ~BufferPool();
    
    int buscar_pagina_id(int id);
    string* get_puntero(int idx);
    void print();
    void print_hit_rate();
    
    void incrementar_hit();
    void incrementar_miss();
    
    void high_dirty_bit(int id);
    void guardar(int idx);
    void eliminar(int idx);
    
    int cargar_pagina(int id_bloque, Operacion op);
    int buscar_frame_libre();
    int tarjet_eliminar();

    void actualizar_tiempo_uso(int idx);
    void set_reference_bit(int idx, bool value);

    // Nueva interfaz para manejar procesos
    void agregar_proceso_a_pagina(int idx, Operacion op);
    void terminar_proceso_de_pagina(int id_pagina);
};

// ======================================================
// BufferManager: Interfaz para el simulador
// ======================================================
class BufferManager {
private:
    BufferPool* buffer_pool;

public:
    BufferManager(int num_frames, Politica poli, DiscoFisico *mydisk);
    ~BufferManager();
    
    string* acceder(int id_bloque, Operacion op);
    void ver_tabla();

    // Interfaz pública para control manual desde terminal.cpp
    void terminar_proceso_manual(int id_bloque);
    
    BufferPool* get_buffer_pool();
};

// ======================================================
// Page
// ======================================================
class Page {
private:
    int page_id;
    bool is_valido;

public:
    string contenido;
    Page(int id);
    ~Page();
    int get_id();
    bool valido();
};

#endif
