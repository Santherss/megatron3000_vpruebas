#ifndef GENERALES
#define GENERALES

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

extern const char *palabras_reservadas[];
extern const char *palabras_salida[];
extern const char *palabras_adicionales[];
//    int n = sizeof(palabras_reservadas) / sizeof(palabras_reservadas[0]);

//tipos de datos? 

int tamano (char * str, char separador='\n');

bool compararTotal(char * a, char * b);

bool comparar(char * a, char * b);

int estaLiteral(char * str,const char **lista);

int estaParcial(char * str,const char **lista);

char *mayusculas(char *origen, char *destino);

int buscar(char * val, char * str);

void procesarLinea(char *str, char *linea[],char separador='#');

char * quitarEspacios(char *str);

const char* tipo_dato(char *campo);



#endif