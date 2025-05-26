#ifndef ARCHIVOS
#define ARCHIVOS

#include "DiscoFisico.h"
#include "generales.h"


bool insertar_tabla(char *archivo, char separador, DiscoFisico *disk, unsigned int &d, int &c, unsigned int &p, unsigned int &s);

bool agregar_a_esquema(DiscoFisico *disk, char *nombre_tabla,char * archivo,char sepa);

bool avanzar(DiscoFisico *d, unsigned int &di, int &cara, unsigned int &pi, unsigned int &se);

#endif