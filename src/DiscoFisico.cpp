#include "DiscoFisico.h"
#include "GeneralesFisico.h"
//#include "Sector.h"
#include <filesystem>
namespace fs = std::filesystem;
//crea disco desde cero

DiscoFisico::DiscoFisico() {
    discos=0;
    pistas=0;
    sectores=0;
    tam_sector=0;
};

std::string ruta_base = base_base_ruta;

void DiscoFisico::crear(char* nombre, unsigned int discos, unsigned int pistas, unsigned int sectores, unsigned int tam){
    ruta_base = base_base_ruta + nombre;

    std::error_code ec;
    for (const auto& entry : fs::recursive_directory_iterator(ruta_base, ec)) {
        fs::permissions(entry.path(),
                        fs::perms::owner_all,
                        fs::perm_options::replace, ec);
        fs::remove_all(entry, ec);
        if (ec) {
            printf("[ - ] Error eliminando: %s\n",ec.message().c_str());
        }
    }
    for (int d = 0; d < discos; d++){
        fs::create_directories(ruta_base + "/" + std::to_string(d));
        for (int s = 0; s < 2; ++s) {
            for (int p = 0; p < pistas; ++p) {
                std::string ruta_pista = ruta_base + "/"+ std::to_string(d) + "/" + std::to_string(s) + "/" + std::to_string(p);
                fs::create_directories(ruta_pista);
                for (int t = 0; t < sectores; ++t) {
                    std::string ruta_sector = ruta_pista + "/" + std::to_string(t) + "";
                    FILE* archivo = fopen(ruta_sector.c_str(), "wb");
                    if (archivo) {
                        fclose(archivo);
                    }

                }
            }
        }
    }
    this->tam_sector=tam-1;
    this->discos=discos-1;
    this->pistas=pistas-1;
    this->sectores=sectores-1;
    string linea = std::to_string(discos-1)+"#"+std::to_string(pistas-1)+"#"+std::to_string(sectores-1)+"#"+std::to_string(tam);
    if(modificar(linea,0,0,0,0)){
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
            printf("%s",contenido.c_str());
            string token;
            if(end == std::string::npos)
              token= contenido.substr(start);
            else 
            token= contenido.substr(start, end - start);
            //printf("tpcke %s", token.c_str());
            printf("%d",stoi(token));
            printf("%s\n",ruta_base.c_str());
            *valores[i] = stoi(token);
            start = end + 1;
        }
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
    printf("Tamano total disponible: %llu bytes\n", tam_total);
    printf("Tamano total utilizado: %llu bytes\n", tam_usado);
    printf("Espacio libre disponible: %llu bytes\n", tam_total - tam_usado);  
}

string DiscoFisico::leer(unsigned int d, int cara, unsigned int p,unsigned int s){
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
    Sector my_sector (tam_sector);
    string ruta =  to_string(d)+"\\"+to_string(cara)+"\\"+to_string(p)+"\\"+to_string(s);
    if(my_sector.esta_lleno(ruta))
        return false;
    else { 
        my_sector.modificar_sector(str.c_str() ,ruta);
        return 1;
    }
    
}


/* 


DiscoFisico::DiscoFisico(int id, int superficies, int pistas, int sectores, const std::string& base_path)
    : id(id), superficies(superficies), pistas(pistas), sectores(sectores), ruta_base(base_path) {}
    void DiscoFisico::inicializar() {

    
} */