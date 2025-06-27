#ifndef BUFFERMANAGER
#define BUFFERMANAGER
#include <string>
#include <stdio.h>
#include "DiscoFisico.h"
using namespace std;

enum class Operacion
{
    Leer,
    Eliminar,
    Insertar
};

class BufferManager;
class BufferPool;
class Frame;
class Page;

//* ---------------------- Buffer Manager ----------------------

class BufferManager
{
private:
    BufferPool *buffer_pool;

public:
    BufferManager(int num_frames, DiscoFisico *mydisk);
    ~BufferManager();
    //
    string *acceder(int id_bloque, Operacion op);
    void ver_tabla();
    void high_dirty_bit(int id);
    void pin(int id);
    void unpin(int id);
    void guardar(int id);
    void eliminar(int id);
};

//* ---------------------- Buffer Pool----------------------

class BufferPool
{
private:
    Frame *listaBuffer;
    int num_frames;
    int num_hit;
    int num_miss;
    int tiempo_global;
    int clock_hand;

public:
    BufferPool(int num_frames);
    ~BufferPool();
    int buscar_pagina_id(int id);
    void print_hit_rate();
    void high_dirty_bit(int);
    string *get_puntero(int idx);
    void pin(int id);
    void unpin(int id);
    void guardar(int idx);

    void incrementar_miss();
    void set_last_used(int time);
    void set_pagina(Page *p);
    void reset_frame();
    int cargar_pagina(int id_bloque, Operacion op);
    int buscar_frame_libre();
    void actualizar_tiempo_uso(int idx);
    void incrementar_hit();
    void incrementar_pin_count(int idx);
    int tarjet_eliminar();
    void eliminar(int idx);
    void print();

    void set_reference_bit(int idx, bool value);
};

//* ---------------------- Frame ----------------------

class Frame
{
private:
    int id;
    Page *pagina;
    int pin_count;
    bool dirty_bit;
    bool is_pin;
    int last_used;
    bool reference_bit;

public:
    Frame(int i);
    Frame();
    ~Frame();
    void ver_atributos();
    int get_id();
    string *get_puntero();
    // eliminación
    bool get_is_pin();
    bool get_dirty_bit();
    int get_last_used();
    void reset_frame();
    void set_last_used(int time);
    bool set_pagina(Page *p);
    void incrementar_pin_count();
    void high_dirty_bit();
    void low_dirty_bit();
    void pin();
    void unpin();

    bool get_reference_bit();
    void set_reference_bit(bool value);
};

//* ---------------------- Page ----------------------

class Page
{
private:
    // string ruta;
    // FILE * bloque;
    int page_id;
    bool is_valido;

public:
    string contenido;
    Page(int id);
    ~Page();
    string leer();
    int get_id();
    void modificar();
    bool valido();
};

#endif