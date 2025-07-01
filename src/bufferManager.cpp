#include "BufferManager.h"
#include "DiscoFisico.h"
#include <string.h>
#include "generales.h" // Asegúrate de que aquí estén las definiciones de Operacion, Politica, etc.

DiscoFisico *discoUsado;

// ===================================================================
// Implementación de Frame
// ===================================================================

Frame::Frame(int i) : id(i), pagina(NULL), dirty_bit(false), last_used(0), reference_bit(false) {
    // El constructor ya no necesita inicializar pin_count o is_pin
}

Frame::Frame() : id(-1), pagina(NULL), dirty_bit(false), last_used(0), reference_bit(false) {
    // Constructor por defecto
}

Frame::~Frame() {
    if (pagina) {
        delete pagina;
    }
}

// NUEVO: Implementación de los métodos de gestión de procesos
void Frame::terminar_proceso_antiguo() {
    if (!procesos.empty()) {
        procesos.pop();
    }
}
void Frame::agregar_proceso(Operacion op) {
    procesos.push(op);
}

int Frame::get_pin_count() {
    return procesos.size();
}

bool Frame::get_is_pin() {
    // Una página está fijada si y solo si su cola de procesos no está vacía.
    return !procesos.empty();
}

string Frame::get_procesos_str() {
    if (procesos.empty()) return "[]";
    string str = "[";
    std::queue<Operacion> temp = procesos;
    while (!temp.empty()) {
        switch (temp.front()) {
            case Operacion::Leer: str += "R"; break;
            case Operacion::Escribir: str += "W"; break; // Asumiendo que tienes Escribir
            case Operacion::Eliminar: str += "W"; break;
            case Operacion::Insertar: str += "I"; break;
        }
        temp.pop();
        if (!temp.empty()) str += ",";
    }
    str += "]";
    return str;
}

// CAMBIO: ver_atributos ahora usa los nuevos métodos y un formato mejorado
void Frame::ver_atributos() {
    if (pagina == NULL) {
        printf("|%-7d|%-7s|%-10s|%-7s|%-9s|%-9s|%-13s|%-15s|\n", id, "-", "---", "---", "-", "-", "-", "[]");
        return;
    }
    string procesos_str = get_procesos_str();
    printf("|%-7d|%-7d|%-10s|%-7s|%-9d|%-9d|%-13s|%-15s|\n", id,
           pagina->get_id(), dirty_bit ? "true" : "false", get_is_pin() ? "true" : "false",
           get_pin_count(), last_used, reference_bit ? "true" : "false", procesos_str.c_str());
}

// CAMBIO: reset_frame ahora limpia la cola de procesos
void Frame::reset_frame() {
    if (pagina != NULL) {
        delete pagina;
        pagina = NULL;
    }
    dirty_bit = false;
    last_used = 0;
    reference_bit = false;
    while (!procesos.empty()) {
        procesos.pop();
    }
}

int Frame::get_id() {
    if (pagina != NULL)
        return pagina->get_id();
    else
        return -1;
}

void Frame::set_last_used(int time) {
    last_used = time;
}

bool Frame::set_pagina(Page *p) {
    if (!p->valido()) {
        printf("[-] id no valido\n");
        return 0;
    }
    pagina = p;
    return 1;
}

bool Frame::get_dirty_bit() {
    return dirty_bit;
}

int Frame::get_last_used() {
    return last_used;
}

void Frame::high_dirty_bit() {
    dirty_bit = true;
}

void Frame::low_dirty_bit() {
    dirty_bit = false;
}

string *Frame::get_puntero() {
    if (pagina != NULL) {
        return &(pagina->contenido);
    }
    return NULL;
}

bool Frame::get_reference_bit() {
    return reference_bit;
}

void Frame::set_reference_bit(bool value) {
    reference_bit = value;
}


// ===================================================================
// Implementación de Page
// ===================================================================

Page::Page(int id) { // CAMBIO: Constructor simplificado
    this->page_id = id;
    this->is_valido = false; // Se asume inválido hasta que se lea del disco
    this->contenido = "";
    
    // Lógica para leer del disco (asumiendo que tam_bloque es 1 para simplificar)
    char ruta[20];
    if (discoUsado->encontrarSector(ruta, id, 0)) {
        this->is_valido = true;
        this->contenido = discoUsado->leer(quitarEspacios(ruta));
    } else {
        printf("[-] Error: No se pudo encontrar el sector para la página %d\n", id);
    }
}

Page::~Page() {}

int Page::get_id() {
    return page_id;
}

bool Page::valido() {
    return is_valido;
}


// ===================================================================
// Implementación de BufferPool
// ===================================================================

BufferPool::BufferPool(int num_frames, Politica poli) {
    this->num_frames = num_frames;
    this->politica = poli;
    this->listaBuffer = new Frame[num_frames];
    for (int i = 0; i < num_frames; i++) {
        listaBuffer[i] = Frame(i);
    }
    this->num_hit = 0;
    this->num_miss = 0;
    this->tiempo_global = 0;
    this->clock_hand = 0;
}

BufferPool::~BufferPool() {
    delete[] listaBuffer;
}

int BufferPool::buscar_pagina_id(int id) {
    for (int i = 0; i < num_frames; i++) {
        if (listaBuffer[i].get_id() == id) {
            return i;
        }
    }
    return -1;
}

void BufferPool::print_hit_rate() {
    if (num_hit + num_miss == 0) {
        printf("#hits = %d\n#miss = %d\nhit_rate = 0.00\n", num_hit, num_miss);
    } else {
        printf("#hits = %d\n#miss = %d\nhit_rate = %.2f\n", num_hit, num_miss, (float)num_hit / (num_hit + num_miss));
    }
}

void BufferPool::high_dirty_bit(int id) {
    int idx = buscar_pagina_id(id);
    if (idx != -1) {
        listaBuffer[idx].high_dirty_bit();
    }
}

// CAMBIO CRÍTICO: Nueva lógica de reemplazo interactiva
bool compararCandidatos(const CandidatoLRU& a, const CandidatoLRU& b) {
    return a.last_used < b.last_used;
}

int BufferPool::tarjet_eliminar() {
    if (politica == Politica::LRU) {
        vector<CandidatoLRU> candidatos;
        for (int i = 0; i < num_frames; i++) {
            if (listaBuffer[i].get_id() != -1) {
                candidatos.push_back({i, listaBuffer[i].get_last_used()});
            }
        }
        if (candidatos.empty()) return -1; // No hay páginas para reemplazar
        sort(candidatos.begin(), candidatos.end(), compararCandidatos);

        // Intenta encontrar una víctima fácil primero
        for (const auto& candidato : candidatos) {
            if (listaBuffer[candidato.frame_id].get_pin_count() == 0) {
                printf("Víctima encontrada: Página %d. No tiene procesos. Reemplazando.\n", listaBuffer[candidato.frame_id].get_id());
                return candidato.frame_id;
            }
        }
        
        // Si no hay víctimas fáciles, inicia la negociación
        printf("No se encontraron víctimas libres. Iniciando negociación LRU...\n");
        for (const auto& candidato : candidatos) {
            Frame& frame = listaBuffer[candidato.frame_id];
            printf("Candidato LRU: Página %d. Procesos: %s.\n", frame.get_id(), frame.get_procesos_str().c_str());
            printf("¿Terminar el proceso más antiguo? (s/n): ");
            char respuesta;
            scanf(" %c", &respuesta);
            getchar(); // Limpia el buffer de entrada
            if (respuesta == 's' || respuesta == 'S') {
                frame.terminar_proceso_antiguo();
                printf("OK. Proceso terminado. Página %d ahora tiene %d procesos.\n", frame.get_id(), frame.get_pin_count());
                if (frame.get_pin_count() == 0) {
                    printf("La página %d ha sido liberada y será reemplazada.\n", frame.get_id());
                    return candidato.frame_id;
                }
            }
        }
    } 
    else if (politica == Politica::CLOCK) {
        for (int i = 0; i < num_frames * 2; ++i) { // Bucle de seguridad para evitar un ciclo infinito
            Frame& frame = listaBuffer[clock_hand];
            if (frame.get_id() != -1) { // Solo si el frame no está vacío
                if (frame.get_pin_count() > 0) {
                    printf("Candidato CLOCK: Página %d. Procesos: %s.\n", frame.get_id(), frame.get_procesos_str().c_str());
                    printf("¿Terminar el proceso más antiguo? (s/n): ");
                    char respuesta; scanf(" %c", &respuesta); getchar();
                    if (respuesta == 's' || respuesta == 'S') {
                        frame.terminar_proceso_antiguo();
                        if (frame.get_pin_count() == 0) {
                            frame.set_reference_bit(false);
                        }
                    }
                } else { // Si pin_count es 0, se aplica la lógica de CLOCK
                    if (!frame.get_reference_bit()) {
                        printf("Víctima encontrada por CLOCK: Página %d.\n", frame.get_id());
                        int victim_idx = clock_hand;
                        clock_hand = (clock_hand + 1) % num_frames;
                        return victim_idx;
                    } else {
                        printf("-- Dando segunda oportunidad a la página %d\n", frame.get_id());
                        frame.set_reference_bit(false);
                    }
                }
            }
            clock_hand = (clock_hand + 1) % num_frames; // Avanzar la manecilla
        }
    }
    printf("ADVERTENCIA: No se pudo encontrar víctima en esta pasada.\n");
    return -1;
}

void BufferPool::eliminar(int idx) {
    if (idx >= 0 && idx < num_frames) {
        guardar(idx);
        listaBuffer[idx].reset_frame();
    }
}

void BufferPool::guardar(int idx) {
    if (idx >= 0 && idx < num_frames && listaBuffer[idx].get_dirty_bit()) {
        int opcion;
        printf("Desea guardar cambios de la página %d? (1=si, 0=no): ", listaBuffer[idx].get_id());
        scanf("%d", &opcion);
        getchar();
        if (opcion) {
            // discoUsado->reemplazar(listaBuffer[idx].get_id(), listaBuffer[idx].get_puntero());
            printf("Bloque %d actualizado.\n", listaBuffer[idx].get_id());
        }
        listaBuffer[idx].low_dirty_bit();
    }
}

int BufferPool::cargar_pagina(int id_bloque, Operacion op) {
    int frame_idx = buscar_frame_libre();
    if (frame_idx == -1) {
        frame_idx = tarjet_eliminar();
        if (frame_idx != -1) {
            eliminar(frame_idx);
        } else {
            printf("[-] Error: Buffer bloqueado. No se pudo encontrar víctima.\n");
            return -1;
        }
    }

    if (frame_idx != -1) {
        Page *nueva_pagina = new Page(id_bloque);
        if (!nueva_pagina->valido()) {
            delete nueva_pagina;
            return -1;
        }
        listaBuffer[frame_idx].set_pagina(nueva_pagina);
        if (op == Operacion::Escribir || op == Operacion::Insertar || op == Operacion::Eliminar) {
            listaBuffer[frame_idx].high_dirty_bit();
        }
    }
    printf("Página %d cargada en frame %d.\n", id_bloque, frame_idx);
    return frame_idx;
}

int BufferPool::buscar_frame_libre() {
    for (int i = 0; i < num_frames; i++) {
        if (listaBuffer[i].get_id() == -1) {
            return i;
        }
    }
    return -1;
}


void BufferPool::set_reference_bit(int idx, bool value) {
    if (idx >= 0 && idx < num_frames) {
        listaBuffer[idx].set_reference_bit(value);
    }
}

void BufferPool::agregar_proceso_a_pagina(int idx, Operacion op) {
    if (idx >= 0 && idx < num_frames) {
        listaBuffer[idx].agregar_proceso(op);
    }
}
string *BufferPool::get_puntero(int idx) {
    if (idx >= 0 && idx < num_frames) {
        return listaBuffer[idx].get_puntero();
    }
    return NULL;
}

void BufferPool::actualizar_tiempo_uso(int idx) {
    if (idx >= 0 && idx < num_frames) {
        listaBuffer[idx].set_last_used(++tiempo_global);
    }
}
void BufferPool::incrementar_hit() { num_hit++; }
void BufferPool::incrementar_miss() { num_miss++; }

void BufferPool::print() {
    for (int i = 0; i < num_frames; i++) {
        if (politica == Politica::CLOCK) {
            printf(i == clock_hand ? "->" : "  ");
        }
        listaBuffer[i].ver_atributos();
    }
}

// NUEVOS wrappers para la gestión de procesos

void BufferPool::terminar_proceso_de_pagina(int id_pagina) {
    int idx = buscar_pagina_id(id_pagina);
    if (idx != -1) {
        listaBuffer[idx].terminar_proceso_antiguo();
    } else {
        printf("[-] Error: Intento de terminar proceso en página %d que no está en buffer.\n", id_pagina);
    }
}

// ===================================================================
// Implementación de BufferManager
// ===================================================================

BufferManager::BufferManager(int num_frames, Politica poli, DiscoFisico *mydisk) {
    this->buffer_pool = new BufferPool(num_frames, poli);
    discoUsado = mydisk;
    printf("------------------\n");
    printf("BUFFER MANAGER CREADO\n");
    printf("# de frames = %d\n", num_frames);
    printf("Politica de reemplazo = %s\n", poli == Politica::LRU ? "LRU" : "CLOCK");
    printf("------------------\n");
}

BufferManager::~BufferManager() {
    delete buffer_pool;
}

// CAMBIO: `acceder` ahora usa la nueva lógica de procesos
string* BufferManager::acceder(int id_bloque, Operacion op) {
    ver_tabla();
    int idx = buffer_pool->buscar_pagina_id(id_bloque);
    if (idx != -1) { // HIT
        buffer_pool->incrementar_hit();
        buffer_pool->high_dirty_bit(id_bloque);
    } else { // MISS
        buffer_pool->incrementar_miss();
        idx = buffer_pool->cargar_pagina(id_bloque, op);
        if (idx == -1) {
            return NULL;
        }
    }
    
    // Acciones comunes para HIT y MISS
    buffer_pool->actualizar_tiempo_uso(idx);
    buffer_pool->set_reference_bit(idx, true);
    buffer_pool->agregar_proceso_a_pagina(idx, op);

    ver_tabla();
    return buffer_pool->get_puntero(idx);
}

void BufferManager::ver_tabla() {
    printf("|%-7s|%-7s|%-10s|%-7s|%-9s|%-9s|%-13s|%-15s|\n", 
           "FrameId", "PageId", "Dirty Bit", "is Pin", "Pin Count", "Last Used", "Reference Bit", "Processes");
    buffer_pool->print();
    buffer_pool->print_hit_rate();
}

// Implementación de los wrappers de control manual
void BufferManager::terminar_proceso_manual(int id_bloque) {
    buffer_pool->terminar_proceso_de_pagina(id_bloque);
}

BufferPool* BufferManager::get_buffer_pool() {
    return buffer_pool;
}
