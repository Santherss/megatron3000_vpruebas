#include "generales.h"
#include "terminal.h"
#include "archivos.h"
#include "DiscoFisico.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>



// -------------------CONSULTA-------------------

int procesar_consulta(char * str, DiscoFisico * mydisk){
    char mayus[tamano(str)];
    mayusculas(str,mayus);
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
        printf("%s",quitarEspacios(str+tamano((char *) palabras_adicionales[2])));
        mydisk->inicializar(quitarEspacios(str+tamano((char *) palabras_adicionales[2])));
        //printf("selecionardiscosss\n");
        return 1;
    }
    if (buscar((char*)palabras_adicionales[1],mayus)==0){ // createdisco
        printf("%s",quitarEspacios(str+tamano((char *) palabras_adicionales[1])));
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
        printf("crear discoo\n");
        mydisk->crear(quitarEspacios(str+tamano((char *) palabras_adicionales[1])),discos,pistas,sectores,tam);

        return 1;
    }
    
    if (compararTotal((char*)palabras_reservadas[0],mayus)){ //HELP
        write(1,"SELECT <columnas> FROM <tabla> <WHERE condiciones> <| nombre>\n",62);
        return 1;
    }
    if(buscar((char *)palabras_reservadas[5],mayus)==0){ //CREATE tabla
        printf("Creando...\n");
        FILE * esquema;
        return 1;
        //esquema= fopen())
    }

    if(buscar((char *)palabras_reservadas[1],mayus)==0){ //SELECT
        
        printf("select\n");
        return 1;
        //procesar_select()
    } 
    if (buscar((char*)palabras_reservadas[6],mayus)==0){ //insert
        printf("insert\n");
        return 1;
    }
    if (buscar((char*)palabras_adicionales[0],mayus)==0){ //reporte
        mydisk->reporte();
        printf("reporanrrr\n");
        return 1;
    }


    write(1,"Consulta invalida, usa HELP\n",28);
    return 1;
}


// -------------------TERMINAL-------------------
void terminal(){
    write(1,"Welcom to MEGATRON3000!!!\n",26);

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
        continuar = procesar_consulta(line, myDisk);
        free(line);
        line = NULL;
        write(1, "\n--------------------\n", 22);
    }
    //reporte de disco
}
