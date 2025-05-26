#include "archivos.h"


bool insertar_tabla(char *archivo, char separador, DiscoFisico *disk, unsigned int &d, int &c, unsigned int &p, unsigned int &s) {
    FILE *f = fopen(archivo, "r");
    if (!f) {
        write(1, "ERROR: no se pudo abrir archivo\n", 33);
        return false;
    }
    //!corregir crear bloque en base al tamaño 
    char linea[512];
/*     char cabecera[512];
    fgets(cabecera, sizeof(cabecera), f);
    fgets(linea, sizeof(linea), f);
    if (!agregar_a_esquema(disk,cabecera,linea)){
        write(1,"[-] Error en agregar cabecera\n",30);
        return false;
    }
    fseek(f, -tamano(linea, '\0'), SEEK_CUR); */
    while (fgets(linea, sizeof(linea), f)) {
        int len = tamano(linea);
        if ((unsigned)len >= disk->tam_sector) {
            write(1, "ERROR: línea muy larga\n", 24);
            continue;
        }

        for (int i = 0; i < len+1; i++){
            linea[i] = linea[i]==separador? '#': linea[i];
            if (linea[i]=='\0'){
                linea[i]='\n';
            }
            
        }
        
        if (!disk->insertar(linea, len, d, c, p, s)) {
            write(1, "ERROR al insertar línea\n", 25);
            break;
        }
       /*  if (!avanzar(disk, d, c, p, s)) {
            write(1, "Disco lleno\n", 12);
            break;
        } */
    }

    fclose(f);
    return true;
}


bool agregar_a_esquema(DiscoFisico *disk, char *nombre_tabla, char *  archivo,char sepa) {
    FILE *f = fopen(archivo, "r");

    char cabecera[512];
    char registro[512];
    fgets(cabecera, sizeof(cabecera), f);
    fgets(registro, sizeof(registro), f);

    std::string contenido = disk->leer(0, 1, 0, 0);
    if (buscar(nombre_tabla, (char *)contenido.c_str()) != -1) {
        write(1, "ERROR: tabla ya existe\n", 24);
        return false;
    }
/* 
    char entrada[64];
    int len = tamano((char *)contenido.c_str()) + tamano(nombre_tabla) + 2;
    if (len >= (int)disk->tam_sector) {
        write(1, "ERROR: esquema lleno\n", 22);
        return false;
    }

    snprintf(entrada, sizeof(entrada), "%s\n", nombre_tabla);
    contenido += entrada; */
    for (int i = 0; i < tamano(registro)+1; i++){
        registro[i] = registro[i]==sepa? '#': registro[i];
            
    }
    for (int i = 0; i < tamano(cabecera)+1; i++){
        cabecera[i] = cabecera[i]==sepa? '#': cabecera[i];
            
    }
    write(1,nombre_tabla,tamano(nombre_tabla)+1);
    char cabecera_tipos[tamano(cabecera)+tamano(registro)];
    int posCabecera=0;
    int posRegistro=0;
    int posSalida = 0;

    while (1) {
        int finCab = buscar((char *)"#", &cabecera[posCabecera]);
        int finReg = buscar((char *)"#", &registro[posRegistro]);

        if (finCab == -1 || finReg == -1) break;

        for (int i = 0; i < finCab; ++i)
            cabecera_tipos[posSalida++] = cabecera[posCabecera + i];
        
        cabecera_tipos[posSalida++] = '#';

        char tempCampo[64];
        int k = 0;
        for (int i = 0; i < finReg && k < 63; ++i)
            tempCampo[k++] = registro[posRegistro + i];
        tempCampo[k] = '\0';

        const char* tipo = tipo_dato(tempCampo);

        for (int i = 0; tipo[i]; ++i)
            cabecera_tipos[posSalida++] = tipo[i];

        cabecera_tipos[posSalida++] = '#';

        posCabecera += finCab + 1;
        posRegistro += finReg + 1;
    }

    // Terminar con '\0'
    if (posSalida > 0) posSalida--; // eliminar '#' final extra
    cabecera_tipos[posSalida] = '\n';
    
    return disk->insertar(cabecera_tipos,tamano(cabecera_tipos), 0, 1, 0, 0);
}


