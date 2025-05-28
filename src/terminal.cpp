#include "generales.h"
#include "terminal.h"
#include "archivos.h"
#include "DiscoFisico.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>




bool campos_create_tabla(char *str, char *nombre_tabla, char *archivo, char *separador) {
    str += tamano((char *)"CREATE");
    while (*str == ' ') str++;

    int i = 0;
    while (*str && *str != ' ') nombre_tabla[i++] = *str++;
    nombre_tabla[i] = '\0';

    while (*str == ' ') str++;
    i = 0;
    while (*str && *str != ' ') archivo[i++] = *str++;
    archivo[i] = 0;

    while (*str == ' ') str++;
    if(*str =='t'){
        *separador= '\t';
    }
    else 
        *separador = *str ? *str : ',';
    return true;
}

bool crear_tabla(char *str, DiscoFisico *disk) {
    char nombre[32], archivo[64], sep;
    if (!campos_create_tabla(str, nombre, archivo, &sep)) {
        write(1, "ERROR: parámetros inválidos\n", 28);
        return false;
    }

    if (!agregar_a_esquema(disk, nombre,archivo,sep)) return false;

    // Buscar sector libre desde (0,0,0,1)
    unsigned int d = 0, p = 0, s = 1;
    int c = 0;
    return insertar_tabla(archivo, sep, disk, d, c, p, s,nombre);
}

bool procesar_select(char * str, char* mayus, DiscoFisico * mydisk) {
    int from_pos = buscar((char *)"FROM", mayus);
    if (from_pos == -1) {
        write(1, "Error: sintaxis incorrecta\n", 27);
        return true;
    }
    char *columnas_str = str + 6; // después de SELECT
    columnas_str[from_pos - 6] = '\0'; // terminamos columnas antes de FROM
    char *columnas = quitarEspacios(columnas_str);
    
    char *tabla_str = str + from_pos + 4;
    tabla_str = quitarEspacios(tabla_str);

    int pipe_pos = buscar((char *)"|", tabla_str);
    char *archivo_salida = NULL;
    if (pipe_pos != -1) {
        archivo_salida = quitarEspacios(tabla_str + pipe_pos + 1);
        tabla_str[pipe_pos] = '\0'; 
    }
    
    int where_pos = buscar((char *)"WHERE", mayus);
    char *condiciones = NULL;
    if (where_pos != -1) {
        condiciones = quitarEspacios(str + where_pos + 5);
        tabla_str[where_pos - from_pos - 4] = '\0';
    }
    
    char *tabla = quitarEspacios(tabla_str);
    
    if (buscarEsquema(tabla_str)){
        char linea[80];
        if (compararTotal(columnas,"*")){
            //printf("pp\n");
            //char * conteido=(char*) mydisk->leer(0,0,0,1).c_str();
            std::string conteido= mydisk->leer(0,0,0,1);
            //conteido[15] = '\0';
            //write(1,conteido.c_str(),tamano((char*)conteido.c_str()));            
            for(char c: conteido)
                if (c=='#'){
                    putchar(' ');
                    putchar('|');
                    putchar(' ');
                } else
                    putchar(c);
        }
        
    } else  
        write(1,"[-] No hay relacion en esquema\n",31);
    

    return true;
}




// -------------------CONSULTA-------------------

int procesar_consulta(char * str, DiscoFisico * mydisk){
    //printf("%s",str);
    char mayus[tamano(str)];
    mayusculas(str,mayus);
    //printf("%s",mayus);
    /* int i=0;
    do{
        write(1,&mayus[i],1);
        write(1,"*",1);
    } while (mayus[i++]!='\n');
    write(1,"\n",1);  */
    if (estaLiteral(mayus,palabras_salida)){ //salir
        printf("Saliendo...\n");
        return 0;
    }      
    
    if (buscar((char*)palabras_adicionales[2],mayus)==0){ // SELECT-DISCO
        //printf("%s",quitarEspacios(str+tamano((char *) palabras_adicionales[2])));
        mydisk->inicializar(quitarEspacios(str+tamano((char *) palabras_adicionales[2])));
        //printf("selecionardiscosss\n");
        return 1;
    }

    if (buscar((char*)palabras_adicionales[1],mayus)==0){ // createdisco
        //printf("%s",quitarEspacios(str+tamano((char *) palabras_adicionales[1])));
        if(tamano(quitarEspacios(str+tamano((char *) palabras_adicionales[1])))<1){
            write(1,"nombre requerido\n",18);
            return 1;
        }

        unsigned int discos,pistas,sectores,tam;
        write(1,"\nPlatos: ",9);
        scanf("%u", &discos);
        write(1,"\nPistas: ",9);
        scanf("%u", &pistas);
        write(1,"\nSectores: ",11);
        scanf("%u", &sectores);
        write(1,"\nTamano del sector: ",20);
        scanf("%u", &tam);
        getchar();

        //printf("crear discoo\n");
        mydisk->crear(quitarEspacios(str+tamano((char *) palabras_adicionales[1])),discos,pistas,sectores,tam);

        return 1;
    }
    
    if (compararTotal((char*)palabras_reservadas[0],mayus)){ //HELP
        write(1,"Primeros crea o seleciona un disco con create-disco O select-disco\n",45);
        write(1,"SELECT <columnas> FROM <tabla> <WHERE condiciones> <| nombre>\n",62);
        return 1;
    }
    
    if(buscar((char *)palabras_reservadas[5],mayus)==0){ //CREATE tabla archivo
        crear_tabla(str, mydisk);
        return 1;
    }

    if(buscar((char *)palabras_reservadas[1],mayus)==0){ //SELECT
        //printf("%s\n",ruta_base.c_str());

        procesar_select(str,mayus, mydisk);
        return 1;
        
        //procesar_select()
    } 
    if (buscar((char*)palabras_reservadas[6],mayus)==0){ //insert <registro>
        printf("insert\n");
        
        return 1;
    }

    if (buscar("ESQUEMA",mayus)==0){ // esquema, para ver nuestras relaciones
        write(1,(char*)(mydisk->leer(0,1,0,0)).c_str(),tamano((char*)mydisk->leer(0,1,0,0).c_str(),'\0'));
        return 1;   
    }
    

    if (buscar((char*)palabras_adicionales[0],mayus)==0){ //reporte
        mydisk->reporte();
        //printf("reporanrrr\n");
        return 1;
    }

    if(buscar("SET-BLOQUE", mayus)==0){ //setear el bloque  
        if(discoInicializado()){
            printf("[-]Error: no hay disco seleccionado \n");
            return 1; 
        }
        int tam;
        write(1,"tamano de bloque: ",18);
        scanf("%u", &tam);
        getchar();
        if( tam%mydisk->tam_sector==0){ //es mutliplo
            write(1,"[+] Tamano de bloque actualizado\n",33);
            mydisk->tam_bloque=tam;
        }else
            write(1,"[-] tamano de bloque no correcto. Usa reporte\n",46);
        
        return 1;
    }


    write(1,"Consulta invalida usa HELP\n",27);
    return 1;
}


// -------------------TERMINAL-------------------
void terminal(){
    write(1,"Welcom to MEGATRON3000!!!\n->  ",30);

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    bool continuar = 1;
    bool disco = false;
    DiscoFisico * myDisk = new DiscoFisico();
    while (continuar){
        nread = getline(&line, &len, stdin);
        if (nread == -1) {  // EOF o error: salir del ciclo
            free(line);
            break;
        }
        ////printf("len=%d, line='%s'\n", sizeof(line), line);
        continuar = procesar_consulta(quitarEspacios(line), myDisk);
        free(line);
        line = NULL;
        write(1, "\n--------------------\n->  ", 26);
    }
    if (discoInicializado()){
        myDisk->reporte();
    }
    
    //reporte de disco
}
