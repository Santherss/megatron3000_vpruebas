#ifndef SECTOR_H
#define SECTOR_H
#include "GeneralesFisico.h"

class Sector{
private:
    //FILE * file_sector;
    //std::string ruta;
    //unsigned int index;
    //! char delimitador;
    unsigned int tamano;
    //funcion interna para buscar registro
    // busca v en las columna campo
    //retorna el puntero al ID(inicio del registro)
    //string buscar(std::string &valor, unsigned int idx_campo); 
    //bool datos_llenos();
public:
    Sector(unsigned int tam);
    //Sector(std::string r, unsigned int i);
    ~Sector();
       // bool actualizar_index(std::string &r, unsigned int index);
    //confirma si se puedo escribir
bool modificar_sector(const char * str, std::string rut);
      //  bool modificar(std::string v_buscado, std::string * c_bucado, std::string * v_nuevo, std::string * c_nuevo);
       // void escribir(std::string str);
    //void escribir(FILE * archivo_entrada);
    //verifica el primer id

    //!
bool esta_lleno(std::string rut);

bool leer_sector(std::string& str, std::string rut);
        void leer(char const  * v_buscado, char const * c_buscado, char const * c_return);
};


#endif