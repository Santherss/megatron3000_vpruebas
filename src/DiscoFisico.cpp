#include "DiscoFisico.h"
#include "GeneralesFisico.h"
#include "generales.h"
//#include "Sector.h"
#include <filesystem>
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

void DiscoFisico::crear(char* nombre, unsigned int discos, unsigned int pistas, unsigned int sectores, unsigned int tam){
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
    this->tam_bloque = this->tam_sector;
    
    std::string linea = std::to_string(this->discos) + "#" +
    std::to_string(this->pistas) + "#" +
    std::to_string(this->sectores) + "#" +
    std::to_string(this->tam_sector);
    
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
        unsigned int* valores[] = { &discos, &pistas, &sectores, &tam_sector };
        size_t start = 0, end;
        for (int i = 0; i < 4; ++i) {
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
        this->tam_bloque = this->tam_sector;
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
    try {
            for (const auto& entry : fs::recursive_directory_iterator(ruta_base)) {
                if (fs::is_regular_file(entry)) {
                    tam_usado += fs::file_size(entry); 
                }
            }
    } catch (const fs::filesystem_error& e) {
        printf("Error al hacer reporte: %s\n", e.what());
        return;
    }
    printf("\n-------------Reporte-------------\n");
    printf("Disco: %s\n",ruta_base.c_str()+tamano((char*)base_base_ruta.c_str()));
    write(1,leer(0,0,0,0).c_str(),leer(0,0,0,0).length());
    printf("#%d",tam_bloque);
    printf("\nTamano total: %llu bytes\n", tam_total);
    printf("Tamano utilizado: %llu bytes\n", tam_usado);
    printf("Espacio libre: %llu bytes\n", tam_total - tam_usado);  
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

bool DiscoFisico::escribir(string str, unsigned int d, int cara, unsigned int p,unsigned int s){
    if(discoInicializado()){
        printf("[-]Error: no hay disco seleccionado \n");
        return NULL; 
    }
    Sector my_sector (tam_sector);
    string ruta =  to_string(d)+"/"+to_string(cara)+"/"+to_string(p)+"/"+to_string(s);
    if(my_sector.esta_lleno(ruta)){
        printf("sector lleno: %s 42\n",ruta.c_str());
        return false;
    }
    else { 
        return my_sector.modificar_sector(str.c_str() ,ruta);
    }
    
}

bool DiscoFisico::insertar(char * str, int tam,unsigned int d, int cara, unsigned int p,unsigned int s){
    //Sector mySector (tam_sector);
    if(discoInicializado()){
        printf("[-]Error al leer: no hay disco seleccionado \n");
        return NULL; 
    }
    string ruta = to_string(d)+"/"+to_string(cara)+"/"+to_string(p)+"/"+to_string(s);
    
    while(fs::file_size(ruta_base+"/"+ruta)+tamano(str) > tam_sector){
        //write(1,"sector lleno ",13);
        //write(1, ruta.c_str(), ruta.length());
        //write(1,"\n",1);
        if(!avanzar(d,cara,p,s))
            return false;
        ruta = to_string(d)+"/"+to_string(cara)+"/"+to_string(p)+"/"+to_string(s);
    }
    
    FILE * archivo = fopen((ruta_base+"/"+ruta).c_str(),"a");
    if(!archivo){
        write(1,"Erro al insertar en ",20);
        return 0;
    }
    write(fileno(archivo),str,tam+1);
    printf("escrito en %s\n",ruta.c_str());
    return 1;

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


/* 


DiscoFisico::DiscoFisico(int id, int superficies, int pistas, int sectores, const std::string& base_path)
    : id(id), superficies(superficies), pistas(pistas), sectores(sectores), ruta_base(base_path) {}
    void DiscoFisico::inicializar() {

    
} */