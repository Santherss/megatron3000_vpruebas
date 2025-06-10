#ifndef ARCHIVOS
#define ARCHIVOS

#include "DiscoFisico.h"
#include "generales.h"


bool insertar_tabla(int cantidad,char *archivo, char separador, DiscoFisico *disk, unsigned int &d, int &c, unsigned int &p, unsigned int &s, char *nombre_tabla, char modo, int * tamano=NULL);

bool agregar_a_esquema(DiscoFisico *disk, char *nombre_tabla,char * archivo,char sepa, char * cabecera);

bool avanzar(DiscoFisico *d, unsigned int &di, int &cara, unsigned int &pi, unsigned int &se);

int buscarEsquema(char * nombre,char* rel=NULL);

bool buscarRegistroRelacion (char * sector, char * nombre);

bool buscarSectorIndice(const char *sector, const char *nombre_seccion);

void extraerCampoLinea(char *linea, int indice, char *destino);
#endif