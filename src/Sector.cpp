#include "Sector.h"
#include "GeneralesFisico.h"

using namespace std;
//devuelve la linea

Sector::Sector(unsigned int tam):tamano(tam){};

Sector::~Sector(){
 /*    if (file_sector)
        fclose(file_sector);  */   
}

/* string Sector::buscar(string& valor, unsigned int idx_campo){
    if (file_sector){
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file_sector)) {
            printf("Línea: %s", buffer);
        }
    }
} */
/* 
bool Sector::actualizar_index(string& r, unsigned int index){
    if(file_sector)
        fclose(file_sector);
    fopen((ruta_base + r+"\\s"+to_string(index)).c_str(), "r+" );
    return file_sector;
}; */

bool modificar(char const * v_buscado, char const * c_bucado, char const * v_nuevo, char const * c_nuevo){

}

bool Sector::modificar_sector(const char * str, string rut){
    rut = ruta_base + "/"+ rut;
    const char * ruta = rut.c_str();
    FILE * file_sector = fopen(ruta,"r");
    if (!file_sector) {
        printf("[-] No existe sector %s\n", ruta);
        return 0;
    }
    fclose(file_sector);

    file_sector = fopen(ruta, "w");
    if (!file_sector) {
        printf("[-] Error para escribir sector %s", ruta);
        return 0;
    }
    fprintf(file_sector,str);
    fclose(file_sector);
    //printf("[+] Se escribio en %s\n", ruta);
    return 1;
}

bool Sector::leer_sector(string& str, string rut){
    rut = ruta_base + "/"+ rut;
    const char * ruta = rut.c_str();
    FILE * file_sector = fopen(ruta,"r");
    if (!file_sector) {
        printf("[-] No existe sector %s\n", ruta);
        return 0;
    }
    string contenido;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file_sector)) {
        contenido += buffer;
    }
    str= contenido;
    fclose(file_sector);
    return 1;
}

bool Sector::esta_lleno(string rut){
    rut = ruta_base + "/"+ rut;
    const char * ruta = rut.c_str();
    FILE * file_sector = fopen(ruta,"rb");
    if (!file_sector) {
        printf("[-] No existe sector %s\n", ruta);
        return 0;
    }
    int c = fgetc(file_sector);  // Leer un solo byte
    fclose(file_sector);

    return c != EOF; 
}

/* 
void Sector::escribir(string str){
    

} */

/* void Sector::escribir(FILE * archivo_entrada){
    char buffer[4096];
    size_t leidos;
    while ((leidos = fread(buffer, 1, sizeof(buffer), file_sector)) > 0) {
        // Procesar bloque
        fwrite(buffer, 1, leidos, stdout); // ejemplo: imprimir al stdout
    }

} */

//verifica el primer id
//void leer(char const  * v_buscado, char const * c_buscado, char const * c_return);