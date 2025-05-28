#ifndef DISCOFISICO_H
#define DISCOFISICO_H
#include "Sector.h"
#include "GeneralesFisico.h"
using namespace std;
/* class Disco;
class Superficie;
class Pista; */

class DiscoFisico{
private:
    //cantidad de discos, pistas, sectores
    //superficie se deduce
    //Sector my_sector;
public:
    unsigned int discos,pistas,sectores, tam_sector;
    int tam_bloque;
    DiscoFisico(); 
    void crear(char* nombre, unsigned int discos, unsigned int pistas, unsigned int sectores, unsigned int tam);
    bool inicializar(char * nombre);
    
    bool modificar(string str, unsigned int d, int cara, unsigned int p,unsigned int s);
    std::string leer(unsigned int d, int cara, unsigned int p,unsigned int s);
    void reporte();
    bool escribir(char * str, unsigned int d, int cara, unsigned int p,unsigned int s, char* nombre=NULL);
    bool escribir(FILE * archivo, unsigned int d, int cara, unsigned int p,unsigned int s);
    bool insertar(char * str, int tam, char * ruta,char*nombre);
    bool insertar(char * str,int tam, unsigned int d, int cara, unsigned int p,unsigned int s,char*nombre=NULL);
    //por chunks o bloques
    bool avanzar(unsigned int &di, int &cara, unsigned int &pi, unsigned int &se);
    void registrarRelacion(char * sector, char * nombre); //"indexacion" de los sectores a una relacion

    /* void escribir_manual();
    void leer_manual();
    void modificar_manual(); */

};

bool discoInicializado();

#endif