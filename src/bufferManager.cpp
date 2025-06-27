#include "BufferManager.h"
#include "DiscoFisico.h"
#include <string.h>
#include "generales.h"

DiscoFisico *discoUsado;
//* ---------------------- Buffer Manager ----------------------
BufferManager::BufferManager(int num_frames, Politica poli, DiscoFisico *mydisk)
{
    this->buffer_pool = new BufferPool(num_frames, poli);
    discoUsado = mydisk;
    printf("------------------\n");
    printf("BUFFER MANAGER CREADO\n");
    printf("# de frames = %d\n", num_frames);
    printf("politica de reempazo = %s\n", poli == Politica::LRU ? "LRU" : "CLOCK");
    printf("------------------\n");
}

BufferManager::~BufferManager()
{
    delete buffer_pool;
}

//! falta usar operación
string *BufferManager::acceder(int id_bloque, Operacion op)
{
    ver_tabla();
    int idx = buffer_pool->buscar_pagina_id(id_bloque);
    if (idx != -1)
    {
        // HIT: actualizar tiempo de uso
        buffer_pool->actualizar_tiempo_uso(idx);
        buffer_pool->incrementar_pin_count(idx);
        buffer_pool->incrementar_hit();
        buffer_pool->pin(idx);
        buffer_pool->set_reference_bit(idx, true);
        /* if(op == Operacion::Insertar){
            buffer_pool->high_dirty_bit(idx);
        } */
        ver_tabla();
        return buffer_pool->get_puntero(idx);
    }
    else
    {
        // MISS: cargar página
        int frame_libre = buffer_pool->cargar_pagina(id_bloque, op);
        if (frame_libre == -1)
            return NULL;
        buffer_pool->pin(frame_libre);
        buffer_pool->incrementar_miss();
        buffer_pool->set_reference_bit(frame_libre, true);

        ver_tabla();
        return buffer_pool->get_puntero(frame_libre);
    }
}

void BufferPool::set_reference_bit(int idx, bool value)
{
    listaBuffer[idx].set_reference_bit(value);
}

void BufferManager::ver_tabla()
{
    //    ,is_pin?"true":"false",pin_count,last_used);
    printf("|FrameId|PageId\t|Dirty Bit\t|is Pin\t|Pin Count\t|Last Used|Reference Bit|Modo\t|\n");
    buffer_pool->print();
    buffer_pool->print_hit_rate();
}

void BufferManager::high_dirty_bit(int id)
{
    buffer_pool->high_dirty_bit(id);
}

void BufferManager::pin(int id)
{
    buffer_pool->pin(id);
}

void BufferManager::unpin(int id)
{
    buffer_pool->unpin(id);
}

void BufferManager::guardar(int id)
{
    int idx = buffer_pool->buscar_pagina_id(id);
    buffer_pool->guardar(idx);
}

void BufferManager::eliminar(int id)
{
    int idx = buffer_pool->buscar_pagina_id(id);
    buffer_pool->eliminar(idx);
}

//* ---------------------- Buffer Pool----------------------
BufferPool::BufferPool(int num_frames, Politica poli)
{
    listaBuffer = new Frame[num_frames];
    this->num_frames = num_frames;
    num_hit = 0;
    num_miss = 0;
    tiempo_global = 0;
    clock_hand = 0;
    politica = poli;
    for (int i = 0; i < num_frames; i++)
    {
        listaBuffer[i] = Frame(i);
    }
}

BufferPool::~BufferPool()
{
    delete[] listaBuffer;
}

int BufferPool::buscar_pagina_id(int id)
{
    for (int i = 0; i < num_frames; i++)
    {
        if (listaBuffer + i != NULL and id == listaBuffer[i].get_id())
            return i;
    }
    return -1;
}

void BufferPool::print_hit_rate()
{
    printf("#hits = %d\n#miss = %d\nhit_rate = %.2f\n", num_hit, num_miss, (float)num_hit / (num_hit + num_miss));
}

void BufferPool::high_dirty_bit(int id)
{
    for (int i = 0; i < num_frames; i++)
    {
        if (listaBuffer[i].get_id() == id)
        {
            listaBuffer[i].high_dirty_bit();
            break;
        }
    }

    // listaBuffer[idx].high_dirty_bit();
}

int BufferPool::tarjet_eliminar()
{
    if (politica == Politica::LRU)
    {
        int lru_idx = -1;
        int min_time;
        bool first_found = false;

        for (int i = 0; i < num_frames; i++)
        {
            if (!listaBuffer[i].get_is_pin())
            {
                if (!first_found)
                {
                    min_time = listaBuffer[i].get_last_used();
                    lru_idx = i;
                    first_found = true;
                }
                else if (listaBuffer[i].get_last_used() < min_time)
                {
                    min_time = listaBuffer[i].get_last_used();
                    lru_idx = i;
                }
            }
        }
        return lru_idx;
    }
    else if (politica == Politica::CLOCK)
    {
        while (true)
        {
            Frame &frame = listaBuffer[clock_hand];

            if (!frame.get_is_pin())
            {
                if (frame.get_reference_bit())
                {
                    printf("-- dando segunda oportunidad a la pagina %d\n", frame.get_id());
                    frame.set_reference_bit(false);
                }
                else
                {
                    int idx = clock_hand;
                    clock_hand = (clock_hand + 1) % num_frames;
                    return idx;
                }
            }

            clock_hand = (clock_hand + 1) % num_frames;
        }
    }
    else
    {
        printf("[-] Error: no hay politica de reemplazo\n");
        return 0;
    }
}

bool Frame::get_reference_bit()
{
    return reference_bit;
}

void Frame::set_reference_bit(bool value)
{
    reference_bit = value;
}

void BufferPool::eliminar(int idx)
{
    guardar(idx);
    listaBuffer[idx].reset_frame();
}

void BufferPool::guardar(int idx)
{
    if (listaBuffer[idx].get_dirty_bit())
    {
        int opcion;
        printf("desea guardar cambios? de %d\n si == 1\n no== 0\nopcion:    ", listaBuffer[idx].get_id());
        scanf("%d", &opcion);
        getchar();
        if (opcion)
        {
            discoUsado->reemplazar(listaBuffer[idx].get_id(), listaBuffer[idx].get_puntero());
            printf("bloque %d actualizado\n", listaBuffer[idx].get_id());
        }
        listaBuffer[idx].low_dirty_bit();
    }
}

int BufferPool::cargar_pagina(int id_bloque, Operacion op)
{
    // Buscar frame libre
    int frame_idx = buscar_frame_libre();

    if (frame_idx == -1)
    {
        // No hay frames libres, aplicar LRU
        frame_idx = tarjet_eliminar();
        if (frame_idx != -1)
        {
            eliminar(frame_idx);
        }
    }

    // Cargar nueva página
    if (frame_idx != -1)
    {
        Page *nueva_pagina = new Page(id_bloque, op);

        if (!listaBuffer[frame_idx].set_pagina(nueva_pagina))
            return -1;
        listaBuffer[frame_idx].set_last_used(++tiempo_global);

        if (op == Operacion::Insertar)
        {
            listaBuffer[frame_idx].high_dirty_bit();
        }
    }
    printf("pagina cargada\n");
    return frame_idx;
}

int BufferPool::buscar_frame_libre()
{
    for (int i = 0; i < num_frames; i++)
    {
        if (listaBuffer[i].get_id() == -1)
        {
            return i;
        }
    }
    return -1;
}

string *BufferPool::get_puntero(int idx)
{
    return listaBuffer[idx].get_puntero();
}

void BufferPool::actualizar_tiempo_uso(int idx)
{
    listaBuffer[idx].set_last_used(++tiempo_global);
}

void BufferPool::incrementar_pin_count(int idx)
{
    listaBuffer[idx].incrementar_pin_count();
}

void BufferPool::incrementar_hit()
{
    num_hit++;
}

void BufferPool::incrementar_miss()
{
    num_miss++;
}

void BufferPool::print()
{
    for (int i = 0; i < num_frames; i++)
    {
        //! clock
        //        if (i == clock_hand)
        if (politica == Politica::CLOCK)
            printf(i == clock_hand ? "*" : " ");
        listaBuffer[i].ver_atributos();
    }
}

void BufferPool::pin(int id)
{
    listaBuffer[id].pin();
}

void BufferPool::unpin(int id)
{
    // buscar_pagina_id(id);
    listaBuffer[buscar_pagina_id(id)].unpin();
}

//* ---------------------- Frame ----------------------
Frame::Frame()
{
}
Frame::Frame(int i)
{
    id = i;
    pagina = NULL;
    pin_count = 1;
    dirty_bit = false;
    is_pin = false;
    last_used = 0;
}
Frame::~Frame() {}
// destructor default
void Frame::ver_atributos()
{
    // comenzamos despues del frame Id
    if (pagina == NULL)
    {
        printf("|%d\t|-\t|---\t\t|---\t|\t-\t|\t---|\t-\t|----\t|\n", id);
        return;
    }
    printf("|%d\t|%d\t|%s\t\t|%s\t|\t%d\t|\t%d|\t%s\t|%s\t|\n", id,
           pagina->get_id(), dirty_bit ? "true" : "false", is_pin ? "true" : "false",
           pin_count, last_used, reference_bit ? "true" : "false", pagina->get_tipo_operacion());
}

int Frame::get_id()
{
    if (pagina != NULL)
        return pagina->get_id();
    else
        return -1;
}

void Frame::set_last_used(int time)
{
    last_used = time;
}

bool Frame::set_pagina(Page *p)
{
    if (!p->valido())
    {
        printf("[-] id no valido\n");
        return 0;
    }
    pagina = p;
    return 1;
}

void Frame::reset_frame()
{
    if (pagina != NULL)
    {
        delete pagina;
        pagina = NULL;
    }
    pin_count = 1;
    dirty_bit = false;
    is_pin = false;
    last_used = 0;
}

void Frame::incrementar_pin_count()
{
    pin_count++;
}

bool Frame::get_is_pin()
{
    return is_pin;
}

bool Frame::get_dirty_bit()
{
    return dirty_bit;
}

int Frame::get_last_used()
{
    return last_used;
}

void Frame::high_dirty_bit()
{
    dirty_bit = true;
}

void Frame::low_dirty_bit()
{
    dirty_bit = false;
}

string *Frame::get_puntero()
{
    if (pagina != NULL)
    {
        return &(pagina->contenido);
    }
    return NULL;
}

void Frame::pin()
{
    // printf("llegamos a pin frame\n");
    is_pin = true;
}

void Frame::unpin()
{
    // printf("llegamos a unpin frame\n");
    is_pin = false;
    // ver_atributos();
}

//* ---------------------- Page ----------------------
Page::Page(int id, Operacion op)
{
    this->page_id = id;
    this->tipo_operacion = op;
    contenido = "";
    for (int i = 0; i < discoUsado->tam_bloque; i++)
    {
        char ruta[20];
        // printf("buffer %s",buffer);
        if (!discoUsado->encontrarSector(ruta, id, i))
        {
            this->is_valido = false;
            return;
        }
        this->is_valido = true;
        contenido += discoUsado->leer(quitarEspacios(ruta));
    }
    printf("pagina inicializada con contenido\n");
}

Page::~Page() {}

int Page::get_id()
{
    return page_id;
}

bool Page::valido()
{
    return is_valido;
}

char *Page::get_tipo_operacion()
{
    switch (tipo_operacion)
    {
    case Operacion::Leer:
        return "Leer";
        break;

    case Operacion::Insertar:
        return "Modificar";
        break;

    case Operacion::Eliminar:
        return "Modificar";
        break;

    default:
        return "---";
        break;
    }
}

//* ---------------------- Generales----------------------
