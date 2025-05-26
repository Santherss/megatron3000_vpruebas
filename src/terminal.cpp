#include "generales.h"
#include "terminal.h"
#include "archivos.h"
#include "DiscoFisico.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>




bool campos_create(char *str, char *nombre_tabla, char *archivo, char *separador) {
    str += tamano((char *)"CREATE");
    while (*str == ' ') str++;

    int i = 0;
    while (*str && *str != ' ') nombre_tabla[i++] = *str++;
    nombre_tabla[i] = '\n';

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
    if (!campos_create(str, nombre, archivo, &sep)) {
        write(1, "ERROR: parámetros inválidos\n", 28);
        return false;
    }

    if (!agregar_a_esquema(disk, nombre,archivo,sep)) return false;

    // Buscar sector libre desde (0,0,0,1)
    unsigned int d = 0, p = 0, s = 1;
    int c = 0;
    return insertar_tabla(archivo, sep, disk, d, c, p, s);
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
        
        printf("select\n");
        
        int pos_select = buscar((char*)"SELECT", mayus);
        if (pos_select != 0) {
            write(1, "Comando no reconocido o mal formado\n", 36);
            return 1;
        }

        int pos_from = buscar((char*)"FROM", mayus);
        if (pos_from == -1) {
            write(1, "ERROR: falta FROM\n", 18);
            return 1;
        }

        // Extraer columnas
        char columnas[256] = {0};
        int i = 0;
        for (int j = pos_select + 6; j < pos_from && str[j]; ++j) {
            if (str[j] != ' ') columnas[i++] = str[j];
        }
        columnas[i] = 0;

        // Extraer tabla
        i = 0;
        int j = pos_from + 4;
        while (str[j] == ' ') ++j;

        char tabla[128] = {0};
        while (str[j] && str[j] != ' ' && str[j] != '\n' && str[j] != '|') {
            tabla[i++] = str[j++];
        }
        tabla[i] = 0;

        // Buscar WHERE si existe
        int pos_where = buscar((char*)"WHERE", mayus);
        char condicion[256] = {0};
        if (pos_where != -1) {
            i = 0;
            j = pos_where + 5;
            while (str[j] == ' ') ++j;
            while (str[j] && str[j] != '|') condicion[i++] = str[j++];
            condicion[i] = 0;
        }

        // Buscar archivo destino con '|'
        /* char archivo[128] = {0};
        char * ptr_pipe = (char*)strchr(str, '|');
        if (ptr_pipe) {
            ptr_pipe++; // Saltar el '|'
            while (*ptr_pipe == ' ') ++ptr_pipe;
            i = 0;
            while (*ptr_pipe && *ptr_pipe != '\n') archivo[i++] = *ptr_pipe++;
            archivo[i] = 0;
        } */
        char archivo[128] = {0};
        int pos_pipe = buscar((char *)"|", str);
        if (pos_pipe != -1) {
            char *p = str + pos_pipe + 1; // avanzar después del '|'
            while (*p == ' ') ++p; // saltar espacios
            int i = 0;
            while (*p && *p != '\n') {
                archivo[i++] = *p++;
            }
            archivo[i] = 0;
        }
        // Mostrar resultados parseados
        write(1, "CONSULTA SELECT DETECTADA\n", 26);
        write(1, "Columnas: ", 10); write(1, columnas, tamano(columnas)); write(1, "\n", 1);
        write(1, "Tabla: ", 7); write(1, tabla, tamano(tabla)); write(1, "\n", 1);
        if (pos_where != -1) {
            write(1, "Condición: ", 11); write(1, condicion, tamano(condicion)); write(1, "\n", 1);
        }
        if (pos_pipe) {
            write(1, "Archivo destino: ", 17); write(1, archivo, tamano(archivo)); write(1, "\n", 1);
        }



        return 1;
        //procesar_select()
    } 
    if (buscar((char*)palabras_reservadas[6],mayus)==0){ //insert <registro>
        printf("insert\n");
        
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
