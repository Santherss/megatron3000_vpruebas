#include "DiscoFisico.h"
#include "GeneralesFisico.h"
#include "generales.h"
#include "archivos.h"
#include <sys/mman.h>     // mmap, munmap, msync
#include <sys/stat.h>     // fstat
#include <fcntl.h>       // open
#include <filesystem>
#include <string.h>
#include <stdio.h>
namespace fs = std::filesystem;
//crea disco desde cero


bool discoInicializado(){
    return tamano((char*)base_base_ruta.c_str()) >= tamano((char*)ruta_base.c_str());
}



DiscoFisico::DiscoFisico() {
    discos=0;
    pistas=0;
    sectores=0;
    tam_sector=0;
};

std::string ruta_base = base_base_ruta;

void DiscoFisico::crear(char* nombre, unsigned int discos, unsigned int pistas, unsigned int sectores, unsigned int tam,unsigned int bloque){
    ruta_base = base_base_ruta + nombre;

    if (!ruta_base.empty() && ruta_base.size() > 10) {
        std::string comando = "rm -rf \"" + ruta_base + "\"";
        system(comando.c_str());
    }
    std::string comando_mkdir = "mkdir -p \"" + ruta_base + "\"";
    system(comando_mkdir.c_str());

    for (int d = 0; d < discos; d++) {
        for (int s = 0; s < 2; ++s) {
            for (int p = 0; p < pistas; ++p) {
                std::string ruta_pista = ruta_base + "/" + std::to_string(d) + "/" + std::to_string(s) + "/" + std::to_string(p);
                std::string comando_mkdir_pista = "mkdir -p \"" + ruta_pista + "\"";
                system(comando_mkdir_pista.c_str());

                for (int t = 0; t < sectores; ++t) {
                    std::string ruta_sector = ruta_pista + "/" + std::to_string(t);
                    FILE* archivo = fopen(ruta_sector.c_str(), "wb");
                    if (archivo) {
                        fclose(archivo);
                    }
                }
            }
        }
    }
   
    this->tam_sector = tam;
    this->discos = discos - 1;
    this->pistas = pistas - 1;
    this->sectores = sectores - 1;
    this->tam_bloque = bloque;
    
    std::string linea = std::to_string(this->discos) + "#" +
    std::to_string(this->pistas) + "#" +
    std::to_string(this->sectores) + "#" +
    std::to_string(this->tam_sector)+"#"+
    std::to_string(this->tam_bloque);
    
    if (modificar(linea, 0, 0, 0, 0)) {
        printf("[+] Disco creado...\n");
    } else {
        printf("[-] Error guardando disco creado...\n");
    }
}


bool DiscoFisico::inicializar(char * nombre){
    ruta_base = base_base_ruta + nombre;
    Sector my_sector (0);
    std::string contenido;
    if (my_sector.leer_sector(contenido,"0/0/0/0")){
        unsigned int* valores[] = { &discos, &pistas, &sectores, &tam_sector, &tam_bloque};
        size_t start = 0, end;
        for (int i = 0; i < 5; ++i) {
            end = contenido.find('#', start);
            string token;
            if(end == std::string::npos)
            token= contenido.substr(start);
            else 
            token= contenido.substr(start, end - start);
            //printf("tpcke %s", token.c_str());
            //printf("%s\n",ruta_base.c_str());
            *valores[i] = stoi(token);
            start = end + 1;
        }
        //this->tam_bloque = this->tam_sector;
        printf("[+] Disco inicializado: %s\n",ruta_base.c_str());
        return 1;
    } else{
        printf("[-] Error al inicializar disco\n");
        return 0;
    }
}


bool DiscoFisico::modificar(string str, unsigned int d, int cara, unsigned int p,unsigned int s){
    if (d>discos || !(cara==1 || cara==0) || p> pistas || s>sectores){
        printf("verifica la cantidad de discos, superficies, pistas y sectores\n");
        return 0;
    }
    
    Sector my_sector (tam_sector);
    if(my_sector.modificar_sector(str.c_str() , to_string(d)+"/"+to_string(cara)+"/"+to_string(p)+"/"+to_string(s))){
        return 1;
    } 
    return 0;
}

void DiscoFisico::reporte(){
    if(discoInicializado()){
        printf("[-]Error al hacer reporte: no hay disco seleccionado \n");
        return;   
    }
    uintmax_t tam_total = (discos+1) * 2 * (pistas+1) * (sectores+1) * tam_sector;
    uintmax_t tam_usado = 0;
    uintmax_t sector_usados = 0;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(ruta_base)) {
            if (fs::is_regular_file(entry)) {
                if(fs::file_size(entry)>0){
                    tam_usado += fs::file_size(entry);
                    sector_usados++;
                } 
            }
        }
    } catch (const fs::filesystem_error& e) {
        printf("Error al hacer reporte: %s\n", e.what());
        return;
    }
    printf("\n-------------Reporte-------------\n");
    printf("Disco: %s\n",ruta_base.c_str()+tamano((char*)base_base_ruta.c_str()));
    //write(1,leer(0,0,0,0).c_str(),leer(0,0,0,0).length());
    //printf("#%d",tam_bloque);
    printf("\nTamano total: %llu bytes\n", tam_total);
    printf("Tamano utilizado: %llu bytes\n", tam_usado);
    printf("Espacio libre: %llu bytes\n", tam_total - tam_usado);  
    
    printf("\n--------------------------\n");
    printf("platos: %llu | %llu en total, %llu bytes c/u\n",discos+1,discos+1,(pistas+1)*(sectores+1)*tam_sector);  
    printf("pistas: %llu  | %llu en total, %llu bytes c/u\n",pistas+1,(pistas+1)*2*(discos+1),(sectores+1)*tam_sector);  
    printf("sectores: %llu | %llu en total, %llu bytes c/u\n",sectores+1,(sectores+1)*(pistas+1)*(discos+1)*2, tam_sector);  
    printf("Bloque: %llu sectores, %llu bytes c/u\n", tam_bloque, tam_sector*tam_bloque);
    printf("\n--------------------------\n");
    printf("Bloques por pista: %llu\n", sectores+1);
    //printf("Bloques por platos: %llu\n", (sectores+1)/tam_bloque*(pistas+1));
    printf("Bloques por platos: %llu\n",sectores+1 );
    printf("sectores: %llu en total\n\t%llu vacios\n\t%llu llenos\n",(sectores+1)*(pistas+1)*(discos+1)*2, (sectores+1)*(pistas+1)*(discos+1)*2 +1-sector_usados,sector_usados);  
    
    
    
}

string DiscoFisico::leer(char * ruta){
    if(discoInicializado()){
        printf("[-]Error al leer: no hay disco seleccionado \n");
        return NULL; 
    }
    string contenido;
    Sector mysector (tam_sector);
    mysector.leer_sector(contenido,ruta);
    return contenido;
    
}

string DiscoFisico::leer(unsigned int d, int cara, unsigned int p,unsigned int s){
    if(discoInicializado()){
        printf("[-]Error al leer: no hay disco seleccionado \n");
        return NULL; 
    }
    if (d>discos || !(cara==1 || cara==0) || p> pistas || s>sectores){
        printf("[-] verifica la cantidad de discos, superficies, pistas y sectores para LEER\n");
        return "";
    }
    string contenido;
    Sector mysector (tam_sector);
    string ruta = std::to_string(d)+"/"+std::to_string(cara)+"/"+std::to_string(p)+"/"+std::to_string(s);
    mysector.leer_sector(contenido,ruta);
    return contenido;
    
}

bool DiscoFisico::escribir(char * str, unsigned int d, int cara, unsigned int p,unsigned int s,char*nombre){
    //printf("si en escribir\n");
    if(discoInicializado()){
        printf("[-]Error: no hay disco seleccionado \n");
        return 0; 
    }
    Sector my_sector (tam_sector);
    int i= 0;
    char ruta[tam_bloque][20];
    int idx;

    while (1) {
        bool todos_vacios = true;

        for (idx = 0; idx < tam_bloque; idx++) {
            if (!encontrarSector(ruta[idx], i, idx)){
                printf("[-]no encontro sector\n");
                return 0;
            }

            //printf("ruta %d -- %s\n", i, ruta[idx]);

            if (my_sector.esta_lleno(ruta[idx])) {
                todos_vacios = false; 
                break; 
            }
        }

        if (todos_vacios)
            break; 

        i++; 
    }


    //printf("22ruta %d -- %s\n",i,ruta[idx]);

/*     string ruta =  to_string(d)+"/"+to_string(cara)+"/"+to_string(p)+"/"+to_string(s);
    while(my_sector.esta_lleno(ruta)){
        //printf("p1\n");
        if(!avanzar(d,cara,p,s)){
            write(1,"Disco lleno\n",12);
            return false;
        }
        ruta = to_string(d)+"/"+to_string(cara)+"/"+to_string(p)+"/"+to_string(s);  
        //return false;

    } */
    //!------
    if (nombre){
        //if(!buscarRegistroRelacion((char*)ruta.c_str(),nombre))
        //printf("mandado a registrar enescribir disco %s\n",ruta.c_str());
        char str[20];
        sprintf(str, "%d", i);
        registrarRelacion(str,nombre);
    }
    //printf("mandado a escrito en %d\n",i);

    return insertar(str, tamano(str),(char*)ruta,nombre);
    //return my_sector.modificar_sector(str,ruta);
}

bool DiscoFisico::escribirBloque(char * str, unsigned int d, int cara, unsigned int p,unsigned int s,char*nombre, char modo, int * lista_tamanos){
    //printf("si en escribir\n");
    if(discoInicializado()){
        printf("[-]Error: no hay disco seleccionado \n");
        return 0; 
    }
    Sector my_sector (tam_sector);
    int i= 0;
    char ruta[tam_bloque][20];
    int idx;

    while (1) {
        bool todos_vacios = true;

        for (idx = 0; idx < tam_bloque; idx++) {
            if (!encontrarSector(ruta[idx], i, idx)){
                printf("[-]no encontro sector\n");
                return 0;
            }

            //printf("ruta %d -- %s\n", i, ruta[idx]);

            if (my_sector.esta_lleno(ruta[idx])) {
                todos_vacios = false; 
                break; 
            }
        }

        if (todos_vacios)
            break; 

        i++; 
    }

    //!------
    if (nombre){
        //if(!buscarRegistroRelacion((char*)ruta.c_str(),nombre))
        //printf("mandado a registrar enescribir disco %s\n",ruta.c_str());
        char str[20];
        sprintf(str, "%d", i);
        printf("registrando relacion...\n");
        registrarRelacion(str,nombre);
    }
//!modifique ruta por i
    return insertarBloque(str,i,nombre,modo,lista_tamanos);
    //return my_sector.modificar_sector(str,ruta);
}

/*
bool DiscoFisico::escribir(char * str, char * ruta ,char*nombre){
     //printf("si en escribir\n");
    if(discoInicializado()){
        printf("[-]Error: no hay disco seleccionado \n");
        return NULL; 
    }
    Sector my_sector (tam_sector);
    while(my_sector.esta_lleno(ruta)){
        //printf("p1\n");
        if(!avanzar(d,cara,p,s)){
                write(1,"Disco lleno\n",12);
                return false;
            }
        ruta = to_string(d)+"/"+to_string(cara)+"/"+to_string(p)+"/"+to_string(s);
        //return false;
    }
    if (nombre){
        //if(!buscarRegistroRelacion((char*)ruta.c_str(),nombre))
        //printf("mandado a registrar enescribir disco %s\n",ruta.c_str());
        registrarRelacion((char*)ruta.c_str(),nombre);
    }
    //printf("mandado a escrito en %s\n",ruta.c_str());

    return insertar(str, tamano(str),(char*)ruta.c_str(),nombre);
    //return my_sector.modificar_sector(str,ruta);
} */

bool DiscoFisico::encontrarSector(char * ruta,int id_bloque, int idx) const{
    if(idx>=tam_bloque){
        printf("[-]eror en indice de sector en bloque\n");
        return 0;
    }
    int total_posiciones = (pistas+1) * (sectores+1); 

    int posicion = id_bloque % total_posiciones;
    int grupo_en_posicion = id_bloque / total_posiciones;
    
    int pista = posicion / (sectores+1);
    int sector = posicion % (sectores+1);
    
    int inicio_disco_cara = grupo_en_posicion * tam_bloque;
    int sectores_mostrados = 0;

    //int i = idx;
    for (int i = 0; i < tam_bloque && sectores_mostrados < tam_bloque; i++) {
        int indice_disco_cara = inicio_disco_cara + i;
        int disco = indice_disco_cara / 2;
        int cara = indice_disco_cara % 2;
        
        if (disco < (discos+1) && cara < 2 && pista < (pistas +1)&& sector < (sectores+1)) {
            if(idx==i){
                //printf("ID: %d --Sector: %d/%d/%d/%d\n", id_bloque, disco, cara, pista, sector);
                sprintf(ruta, "%d/%d/%d/%d", disco, cara, pista, sector);
                return 1;
            }    
            sectores_mostrados++;
        } else {
            printf("error en encontrar sector\n");
            return 0; 
        }
    }
}


// suma(lista)  espaciolibre indice eliminado
bool DiscoFisico::actualizarCabeceraFija(char * ruta){
    FILE * archivo = fopen(ruta,"r+");
    if(!archivo){
        printf("error en abrir archivo para actualizar cabecera\n");
        return 0;
    }
    printf("actualizando %s\n",ruta);
    //int espacio = tam_sector;
    int espacio = tam_sector;
    char linea[250];
    char newline[11]="000999000";
    if (fgets(linea, sizeof(linea),archivo)==NULL){
        fseek(archivo, 0, SEEK_SET);
        fputs(newline,archivo);
        fputs("\n",archivo);
        fclose(archivo);
        return 1;
    }
    //printf("cabecera %s\n",linea);
    //agrupamos de 3
    char campos[3][3];
    int campo_tamano = -1;
    //resta cabecera (si tiene)
    espacio=espacio-9;
/*     if(!buscar("#",linea)){
        int k=0;
        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 3; j++){
                campos[i][j]=linea[k++];
            }
            printf("campo %d %s\n",i,campos[i]);
        } 
        campo_tamano =stoi(campos[0]);
    } else { */
    if(fgets(linea, sizeof(linea),archivo)){
        campo_tamano=tamano(linea);
        if(linea[0]!='-')
            espacio-=tamano(linea,'\n');
        else{
            fseek(archivo, -tamano(linea), SEEK_CUR);
            fputs(to_string(1).c_str(),archivo);
            fseek(archivo, +tamano(linea)-1, SEEK_CUR);

        }
    }
/*     else {
        int k=0;
        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 3; j++){
                campos[i][j]=linea[k++];
            }
            printf("campo %d %s\n",i,campos[i]);
        } 
        //campo_tamano =stoi(campos[0]);
    } */
    //}
    bool encontrado= true;
    int conta=1;
    while(fgets(linea, sizeof(linea),archivo)){
        //printf("%s\n",linea);
        if(linea[0]!='-')
            espacio-=tamano(linea,'\n');
        else{
            fseek(archivo, -tamano(linea), SEEK_CUR);
            fputs(to_string(conta).c_str(),archivo);
            fseek(archivo, +tamano(linea)-tamano((char*)to_string(conta).c_str()), SEEK_CUR);
        }
        conta++;
    }
    espacio/=++campo_tamano;
    if(espacio>999)
        espacio=999;
    if(campo_tamano>999)
        campo_tamano=999;
        
    fseek(archivo, 0, SEEK_SET);
    snprintf(newline, sizeof(newline), "%03d%03d%03d\n", campo_tamano, espacio,conta);
    fputs(newline,archivo);
    fclose(archivo);


    return 1;
}
bool DiscoFisico::insertarBloque(char * linea,int id_bloque,char * nombre, char modo, int * lista_tamanos){
    char ruta[20];
    char newLine[300];
    int i= 0, n=0;
    int idx=1;
    //printf("1funcinsertando %s\n", linea);

    /* for (int i = 0; i < lista_tamanos[0]; i++){
        printf("22listaauto %d - %d\n",i,lista_tamanos[i]);
    } */
    while(idx<lista_tamanos[0]){
        int tam = 0;
        while(i< tamano(linea) and *(linea+i)!='#' and *(linea+i)!='\n'){
            newLine[n]=linea[i];
            //putchar(linea[i]);
            tam++;
            i++;
            n++;
        }
        //putchar('\n');
        //printf("nuevalinea %s\n",newLine);
        //printf("i %d\nn %d\nidx %d\ntam %d\nlista_tam %d\n",i,n,idx,tam,lista_tamanos[idx]);
        if(tam>lista_tamanos[idx]){
            printf("[-]campo %d mas grande q el maximo %d - %d\n",idx,tam,lista_tamanos[idx]);
            return false;
        }
        for (int  j = tam; j < lista_tamanos[idx]; j++){
            newLine[n]='~';
            n++;
        }
        newLine[n]=linea[i];
        i++;
        n++;
        idx++;
    }
    newLine[n]='\0';
    //printf("nueva linea: \n %s\n",newLine);
    for(int ii = 0; ii < tam_bloque;ii++){
        if(!encontrarSector(ruta,id_bloque,ii)) return false;
        if(modo=='F'||modo=='f'){
            //printf("mandado a escribir\n");
            //printf("funcinsertando %s\n", newLine);
            if(insertarFijo(newLine,ruta,nombre,lista_tamanos))
                return 1;
            //return 1;
        }
        //if(insertar(linea,tamano(linea),ruta,nombre))
        //    return 1;
    }
    printf("[-] no se pudo insertar en el bloque %d\n",id_bloque);
    return 0;
     escribir(linea,0,0,0,0,nombre);
}

bool DiscoFisico::insertarFijo(char * str, char * ruta,char*nombre, int * lista_tamanos){
    if(discoInicializado()){
        printf("[-]Error al leer: no hay disco seleccionado \n");
        return 0; 
    }
    //printf("funci insertar %s\n",ruta);
    string rutta=ruta_base+"/"+ruta;
    /* if(fs::file_size(ruta_base+"/"+ruta)+tamano(str) > tam_sector){
        //printf("%s sin espacio\n",ruta);
        return false;
        } */
    actualizarCabeceraFija((char*)rutta.c_str());
    FILE * tmp=fopen(rutta.c_str(),"r");
    char linea[200];
    if(!fgets(linea,sizeof(linea),tmp)){
        printf("error en acceder a cabecera\n");
        return 0;
    }
    fclose(tmp);
    printf("cabecera %s\n",linea);
    linea[6]='\0';
    printf("espacio %d\n",stoi(linea+4));
    if(stoi(linea+4)==0){
        printf("no hay espacio segun cabecera\n");
        return false;
    }

    string ruta_escribir = ruta_base+"/"+ruta;
    FILE * archivo = fopen(ruta_escribir.c_str(),"r+");
    if(!archivo){
        write(1,"Erro al insertar en ",20);
        fclose(archivo);
        return 0;
    }
    while(fgets(linea,sizeof(linea),archivo))
        if (linea[0]=='-'){
            fseek(archivo, -tamano(linea), SEEK_CUR);
            fputs(str,archivo);
            fclose(archivo);
            actualizarCabeceraFija((char*)ruta_escribir.c_str());
            printf("[+]registro fijo escrito en uno elimnado %s\n",ruta);
            return 1;
        }
        
        //printf("3insertando %s\n",str);
    fputs(str,archivo);
    //write(fileno(archivo),str,tamano(str)+1);
    //printf("%s\n",str);
    fclose(archivo);
    actualizarCabeceraFija((char*)ruta_escribir.c_str());
    printf("[+]registro fijo escrito en %s\n",ruta);
    return 1;

}

bool DiscoFisico::insertar(char * str, int tam, char * ruta,char*nombre){
    if(discoInicializado()){
        printf("[-]Error al leer: no hay disco seleccionado \n");
        return NULL; 
    }
    //printf("funci insertar %s\n",ruta);

    if(fs::file_size(ruta_base+"/"+ruta)+tamano(str) > tam_sector){
        printf("no hay espacio\n");
        return false;
    }
    /* while(fs::file_size(ruta_base+"/"+ruta)+tamano(str) > tam_sector){
        //write(1, ruta.c_str(), ruta.length());
        //write(1,"\n",1);
        if(!avanzar(d,cara,p,s)){
            write(1,"Disco lleno\n",12);
            return false;
        }
        ruta = to_string(d)+"/"+to_string(cara)+"/"+to_string(p)+"/"+to_string(s);
    } */
    string ruta_escribir = ruta_base+"/"+ruta;
    //char * ruta_escribir= (char*)(ruta_base+"/"+ruta).c_str();
    FILE * archivo = fopen(ruta_escribir.c_str(),"a");
    //FILE * archivo = fopen((ruta_base+"/"+ruta).c_str(),"a");
    if(!archivo){
        write(1,"Erro al insertar en ",20);
        fclose(archivo);
        return 0;
    }
    write(fileno(archivo),str,tam+1);
    /* if (nombre){
        //if(!buscarRegistroRelacion((char*)ruta.c_str(),nombre))
        registrarRelacion((char*)ruta,nombre);
    } */
    
    printf("escrito en %s\n",ruta);
    fclose(archivo);
    return 1;

}

bool DiscoFisico::insertar(char * str, int tam,unsigned int d, int cara, unsigned int p,unsigned int s,char*nombre){
    printf("***************\n");
    string ruta = to_string(d)+"/"+to_string(cara)+"/"+to_string(p)+"/"+to_string(s);
    while(fs::file_size(ruta_base+"/"+ruta)+tamano(str) > tam_sector ){
        //write(1, ruta.c_str(), ruta.length());
        //write(1,"\n",1);
        if(!avanzar(d,cara,p,s)){
            write(1,"Disco lleno\n",12);
            return false;
        }
        ruta = to_string(d)+"/"+to_string(cara)+"/"+to_string(p)+"/"+to_string(s);
    }
    return insertar(str,tam,(char*)ruta.c_str(),nombre);
}

bool DiscoFisico::avanzar(unsigned int &di, int &cara, unsigned int &pi, unsigned int &se) {
    se++;
    if (se > this->sectores) {
        se = 0; pi++;
        if (pi > this->pistas) {
            pi = 0; cara++;
            if (cara > 1) {
                cara = 0; di++;
                if (di > this->discos) return false;
            }
        }
    }
    return true;
}


void DiscoFisico::registrarRelacion(char *sector, char *nombre) {
    string ruta = ruta_base + "/0/1/0/1";
    if (buscarSectorIndice(sector, nombre)) {
        //printf("Sector ya existe en la sección.\n");
        return;
    }

    int fd = open(ruta.c_str(), O_RDWR);
    if (fd < 0) {
        perror("No se pudo abrir el archivo");
        return;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat falló");
        close(fd);
        return;
    }

    size_t size = st.st_size;
    if (size == 0) {
        // El archivo está vacío, insertar sección y sector directamente
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "#%s\n%s\n", nombre, sector);
        write(fd, buffer, tamano(buffer, '\0'));
        close(fd);
        //printf("Archivo vacío. Sección y sector insertados.\n");
        return;
    }

    char *map = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap falló");
        close(fd);
        return;
    }

    char *ptr = map;
    char *fin = map + size;
    char *inicio_seccion = NULL;
    char *fin_seccion = NULL;

    // Buscar sección
    while (ptr < fin) {
        if (*ptr == '#') {
            char *line_start = ptr;
            while (ptr < fin && *ptr != '\n') ptr++;
            int len = ptr - line_start;
            char temp[256] = {0};
            memcpy(temp, line_start, len < 255 ? len : 255);
            quitarEspacios(temp);

            if (compararTotal(temp + 1, (char *)nombre)) {  // +1 para ignorar '#'
                inicio_seccion = line_start;
                break;
            }
            ptr++;
        } else {
            while (ptr < fin && *ptr != '\n') ptr++;
            ptr++;
        }
    }

    // Encontrar fin de la sección
    if (inicio_seccion) {
        ptr = inicio_seccion;
        while (ptr < fin) {
            if (*ptr == '#' && ptr != inicio_seccion) {
                fin_seccion = ptr;
                break;
            }
            ptr++;
        }
        if (!fin_seccion) fin_seccion = fin;
    }

    // Preparar contenido a insertar
    char buffer[512];
    size_t insercion_len;
    off_t offset_insercion;

    if (inicio_seccion) {
        snprintf(buffer, sizeof(buffer), "%s\n", sector);
        insercion_len = tamano(buffer, '\0');
        offset_insercion = fin_seccion - map;
    } else {
        snprintf(buffer, sizeof(buffer), "\n#%s\n%s\n", nombre, sector);
        insercion_len = tamano(buffer, '\0');
        offset_insercion = size;
    }

    // Extender archivo
    ftruncate(fd, size + insercion_len);
    munmap(map, size);

    // Remapear y mover contenido
    map = (char*)mmap(NULL, size + insercion_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap remap falló");
        close(fd);
        return;
    }

    memmove(map + offset_insercion + insercion_len, map + offset_insercion, size - offset_insercion);
    memcpy(map + offset_insercion, buffer, insercion_len);

    msync(map, size + insercion_len, MS_SYNC);
    munmap(map, size + insercion_len);
    close(fd);
    //printf("Sector agregado en índice.\n");
}
