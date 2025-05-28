#include "archivos.h"


bool insertar_tabla(char *archivo, char separador, DiscoFisico *disk, unsigned int &d, int &c, unsigned int &p, unsigned int &s, char *nombre_tabla) {
    FILE *f = fopen(archivo, "r");
    if (!f) {
        write(1, "ERROR: no se pudo abrir archivo\n", 33);
        return false;
    }
    //!corregir crear bloque en base al tamaño 
    char linea[512];
    fgets(linea, sizeof(linea), f);
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
        string ruta = ruta_base + "/0/1/0/1";
        FILE * indice = fopen(ruta.c_str(),"r");
        //string nombre_sector = to_string(d)+"/"+to_string(c)+"/"+to_string(p)+"/"+to_string(s);

        char buffer[256];
        char sectorIndice[15];
        bool haySector = false;
        while (fgets(buffer, sizeof(buffer), indice)){
            //printf("insretar tabla:  %s -  nombre : %s\n",buffer+1,nombre_tabla);
            if(compararTotal(quitarEspacios(buffer+1),nombre_tabla)){
                while (fgets(buffer, sizeof(buffer), indice)and buffer[0]!='#'){
                    //printf("[+]encontreo sector en indice\n");
                    snprintf(sectorIndice, sizeof(sectorIndice), "%s",buffer);
                    haySector = true;
                }
                break;
            }
        }

        if (haySector){
            if (disk->insertar(linea,tamano(linea),quitarEspacios(sectorIndice),nombre_tabla)) {
                //write(1, "ERROR al insertar línea\n", 25);
                //break;
                continue;
            }
        } 
        if (!disk->escribir(linea, d, c, p, s,nombre_tabla)) {
            write(1, "ERROR al escribir línea en funci insertar tabla\n", 48);
            break;
        }
        /* if (!disk->insertar(linea, len, d, c, p, s,nombre_tabla)) {
            write(1, "ERROR al insertar línea\n", 25);
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
    //! cambiar forma de buscar
    if (buscarEsquema(nombre_tabla)) {
        write(1, "ERROR: tabla ya existe\n", 24);
        return false;
    }
    for (int i = 0; i < tamano(registro)+1; i++){
        registro[i] = registro[i]==sepa? '#': registro[i];
            
    }
    for (int i = 0; i < tamano(cabecera)+1; i++){
        cabecera[i] = cabecera[i]==sepa? '#': cabecera[i];
            
    }
    char cabecera_tipos[tamano(cabecera)+tamano(registro)+tamano(nombre_tabla)+2];
    int posCabecera=0;
    int posRegistro=0;
    int posSalida = 0;
    for (int i=0; i<tamano(nombre_tabla);i++){
        cabecera_tipos[posSalida++]=nombre_tabla[i];
        
    }
    //write(1,nombre_tabla,tamano(nombre_tabla)+1);
    cabecera_tipos[posSalida++]='#';
    
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
    
    //*agregamos en indices
    char nuevoNombre[tamano(nombre_tabla)+3];
    snprintf(nuevoNombre, sizeof(nuevoNombre), "#%s\n", nombre_tabla);
    disk->insertar(nuevoNombre,tamano(nuevoNombre,'\0')-1, 0, 1, 0,1);

    return disk->insertar(cabecera_tipos,tamano(cabecera_tipos), 0, 1, 0, 0);
}

int buscarEsquema(char * nombre){
    std::string ruta=(ruta_base+"/0/1/0/0");
    //printf("%s\n",(char*)ruta);
    FILE * esquema = fopen(ruta.c_str(),"r");
    char relacion[512];
    if(!esquema){
        write(1,ruta.c_str(),tamano((char*)ruta_base.c_str(),'\0'));
        write(1,"\n[-] Error al buscar relacion en esquema\n",41);
        return 0;
    }
    while (fgets(relacion, sizeof(relacion), esquema)){
        relacion[buscar("#",relacion)] = '\0';
        //printf("\n%s\n",relacion);
        if (compararTotal(relacion,nombre)){
            return 1;
        }
    }
    return 0;
}

bool buscarRegistroRelacion (char * sector, char * nombre){
    std::string ruta=(ruta_base+"/0/1/0/0");
    //printf("%s\n",(char*)ruta);
    FILE * RegistroRelacionaes = fopen(ruta.c_str(),"r");
    char linea[80];
    if(!RegistroRelacionaes){
        write(1,ruta.c_str(),tamano((char*)ruta_base.c_str(),'\0'));
        write(1,"\n[-] Error en RegistroRelacionaes\n",41);
        return 0;
    }
    while (fgets(linea, sizeof(linea), RegistroRelacionaes)){
        //linea[buscar("#",linea)] = '\0';
        //printf("\n%s\n",relacion);
        if (compararTotal(linea,nombre)){
            fgets(linea, sizeof(linea), RegistroRelacionaes);
            while (linea[0]=='#'){
                if(sector==linea)
                    return true;
            }
            return false;
        }
    }
    write(1,"no hay tabla en indice\n",23);
    return 0;
}

bool buscarSectorIndice(const char *sector, const char *nombre_seccion) {
    string ruta = ruta_base+"/0/1/0/1";

    FILE *archivo = fopen(ruta.c_str(), "r");
    if (!archivo) return false;

    char *linea = NULL;
    size_t len = 0;
    ssize_t read;
    bool en_seccion = false;
    char encabezado[256];
    snprintf(encabezado, sizeof(encabezado), "#%s", nombre_seccion);

    while ((read = getline(&linea, &len, archivo)) != -1) {
        quitarEspacios(linea);
        if (linea[0] == '#') {
            en_seccion = compararTotal(linea, encabezado);
            continue;
        }
        if (en_seccion && compararTotal(linea, (char*)sector)) {
            free(linea);
            fclose(archivo);
            return true;  // sector encontrado
        }
        if (en_seccion && linea[0] == '#') break;  // terminó la sección
    }

    if (linea) free(linea);
    fclose(archivo);
    return false;  // no encontrado
}
