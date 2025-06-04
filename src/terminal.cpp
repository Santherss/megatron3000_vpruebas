#include "generales.h"
#include "terminal.h"
#include "archivos.h"
#include "DiscoFisico.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




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

bool evaluarCondicion(char *valorCampo, char *operador, char *valorCondicion, char *tipoDato) {
    //printf("[DEBUG] Campo: '%s', Operador: %s, Condicion: %s, Tipo: %s\n", valorCampo, operador, valorCondicion, tipoDato);

    if (compararTotal(tipoDato, "int")) {
        int campo = atoi(valorCampo);
        int condicion = atoi(valorCondicion);

        if (compararTotal(operador, "<")) return campo < condicion;
        if (compararTotal(operador, "<=")) return campo <= condicion;
        if (compararTotal(operador, ">")) return campo > condicion;
        if (compararTotal(operador, ">=")) return campo >= condicion;
        if (compararTotal(operador, "==")) return campo == condicion;

    } else if (compararTotal(tipoDato, "float")) {
        float campo = atof(valorCampo);
        float condicion = atof(valorCondicion);

        if (compararTotal(operador, "<")) return campo < condicion;
        if (compararTotal(operador, "<=")) return campo <= condicion;
        if (compararTotal(operador, ">")) return campo > condicion;
        if (compararTotal(operador, ">=")) return campo >= condicion;
        if (compararTotal(operador, "==")) return campo == condicion;

    } else if (compararTotal(tipoDato, "string")) {
        if (compararTotal(operador, "==")) return compararTotal(valorCampo, valorCondicion);
    }

    return false;
}

bool evaluarCondiciones(char *linea, char *condiciones, char campos[][50], char tipos[][10], int n_campos) {
    char copia[256];
    strcpy(copia, condiciones);

    char *ptr = copia;
    while (*ptr) {
        // Extraer campo
        char campo[50], operador[3], valor[50];
        int pos = 0;

        // Limpiar espacios al inicio
        while (*ptr && *ptr == ' ') ptr++;

        // Campo - extraer hasta encontrar operador
        while (*ptr && *ptr != '<' && *ptr != '>' && *ptr != '=' && *ptr != ' ') {
            campo[pos++] = *ptr++;
        }
        campo[pos] = '\0';

        // Limpiar espacios después del campo
        while (*ptr && *ptr == ' ') ptr++;

        // Operador
        pos = 0;
        if (*ptr == '<') {
            operador[pos++] = *ptr++;
            if (*ptr == '=') operador[pos++] = *ptr++;
        } else if (*ptr == '>') {
            operador[pos++] = *ptr++;
            if (*ptr == '=') operador[pos++] = *ptr++;
        } else if (*ptr == '=' && *(ptr+1) == '=') {
            operador[pos++] = *ptr++;
            operador[pos++] = *ptr++;
        }
        operador[pos] = '\0';

        // Limpiar espacios después del operador
        while (*ptr && *ptr == ' ') ptr++;

        // Valor - extraer hasta AND o final
        pos = 0;
        while (*ptr && *ptr != 'A' && strncmp(ptr, "AND", 3) != 0) {
            if (*ptr != ' ' || pos == 0 || valor[pos-1] != ' ') {
                valor[pos++] = *ptr;
            }
            ptr++;
        }
        valor[pos] = '\0';

        // Quitar espacios al final del valor
        while (pos > 0 && valor[pos-1] == ' ') {
            pos--;
            valor[pos] = '\0';
        }

        // Saltar "AND" si existe
        if (strncmp(ptr, "AND", 3) == 0) {
            ptr += 3;
            while (*ptr && *ptr == ' ') ptr++;
        }

        // Buscar tipo del campo y su índice
        char *tipoDato = NULL;
        int campo_idx = -1;
        for (int j = 0; j < n_campos; j++) {
            if (compararTotal(campos[j], campo)) {
                tipoDato = tipos[j];
                campo_idx = j;
                break;
            }
        }
        
        if (!tipoDato || campo_idx == -1) return false;

        // Extraer valor del campo usando separador '#'
        char valorCampo[100];
        int campo_actual = 0;
        int inicio = 0;
        int fin = 0;
        
        // Encontrar el campo en la posición campo_idx
        for (int k = 0; linea[k]; k++) {
            if (linea[k] == '#') {
                if (campo_actual == campo_idx) {
                    fin = k;
                    break;
                }
                campo_actual++;
                inicio = k + 1;
            }
        }
        
        // Si es el último campo
        if (campo_actual == campo_idx && fin == 0) {
            fin = strlen(linea);
        }
        
        // Copiar el valor del campo
        int len = fin - inicio;
        if (len >= 100) len = 99;
        strncpy(valorCampo, linea + inicio, len);
        valorCampo[len] = '\0';
        
        // Quitar espacios
        quitarEspacios(valorCampo);
        // Evaluar condición individual
        if (!evaluarCondicion(valorCampo, operador, valor, tipoDato)) return false;
    }

    return true;
}

// Función auxiliar para extraer campos de una línea
void extraerCampos(char *linea, char campos_extraidos[][100], int *num_campos) {
    *num_campos = 0;
    int campo_pos = 0;
    
    // Inicializar el primer campo
    campos_extraidos[*num_campos][0] = '\0';
    
    for (int i = 0; linea[i]; i++) {
        if (linea[i] == '#') {
            // Terminar el campo actual
            campos_extraidos[*num_campos][campo_pos] = '\0';
            quitarEspacios(campos_extraidos[*num_campos]);
            (*num_campos)++;
            campo_pos = 0;
            // Inicializar el siguiente campo
            if (*num_campos < 50) {
                campos_extraidos[*num_campos][0] = '\0';
            }
        } else {
            // Agregar carácter al campo actual
            if (campo_pos < 99) {
                campos_extraidos[*num_campos][campo_pos++] = linea[i];
            }
        }
    }
    
    // Último campo si no termina con #
    if (campo_pos > 0 || *num_campos == 0) {
        campos_extraidos[*num_campos][campo_pos] = '\0';
        quitarEspacios(campos_extraidos[*num_campos]);
        (*num_campos)++;
    }
}

bool procesar_select(char *str, char *mayus, DiscoFisico *mydisk) {
    int from_pos = buscar("FROM", mayus);
    if (from_pos == -1) {
        write(1, "Error: sintaxis incorrecta\n", 27);
        return true;
    }

    char *columnas_str = str + 6; // después de SELECT
    columnas_str[from_pos - 6] = '\0';
    char *columnas = quitarEspacios(columnas_str);

    // Separar columnas seleccionadas
    char columnasSeleccionadas[50][50];
    int n_seleccionadas = 0;
    if (!compararTotal(columnas, "*")) {
        // Hacer una copia para no modificar el original
        char columnas_copia[256];
        strcpy(columnas_copia, columnas);
        
        char *tok = strtok(columnas_copia, ",");
        while (tok && n_seleccionadas < 50) {
            // Quitar espacios al inicio y final
            while (*tok == ' ') tok++; // espacios al inicio
            char *end = tok + strlen(tok) - 1;
            while (end > tok && *end == ' ') *end-- = '\0'; // espacios al final
            
            strcpy(columnasSeleccionadas[n_seleccionadas++], tok);
            tok = strtok(NULL, ",");
        }
    }

    char *tabla_str = str + from_pos + 4;
    tabla_str = quitarEspacios(tabla_str);

    int pipe_pos = buscar("|", tabla_str);
    char *archivo_salida = NULL;
    if (pipe_pos != -1) {
        archivo_salida = quitarEspacios(tabla_str + pipe_pos + 1);
        tabla_str[pipe_pos] = '\0';
    }

    int where_pos = buscar("WHERE", mayus);
    char *condiciones = NULL;
    if (where_pos != -1) {
        // Encontrar WHERE en la cadena original y cortar tabla_str apropiadamente
        char *where_en_tabla = strstr(tabla_str, "WHERE");
        if (!where_en_tabla) {
            where_en_tabla = strstr(tabla_str, "where");
        }
        if (where_en_tabla) {
            *where_en_tabla = '\0';
        }
        condiciones = quitarEspacios(str + where_pos + 5);
    }

    tabla_str = quitarEspacios(tabla_str);

    char tabla[50];
    int tabla_len = strlen(tabla_str);
    if (tabla_len >= 50) tabla_len = 49;
    strncpy(tabla, tabla_str, tabla_len);
    tabla[tabla_len] = '\0';

    char relacion[250];
    if (!buscarEsquema(tabla, relacion)) {
        write(1, "[-] No hay relacion en esquema\n", 31);
        write(1, "para tabla:", 11);
        write(1, tabla, strlen(tabla));
        write(1, "\n", 1);
        return true;
    }

    char campos[50][50], tipos[50][10];
    int n_campos = 0, pos = buscar("#", relacion) + 1;

    while (relacion[pos]) {
        int i = 0;
        while (relacion[pos] != '#' && relacion[pos]) campos[n_campos][i++] = relacion[pos++];
        campos[n_campos][i] = '\0'; pos++;

        i = 0;
        while (relacion[pos] != '#' && relacion[pos]) tipos[n_campos][i++] = relacion[pos++];
        tipos[n_campos][i] = '\0'; pos++;

        n_campos++;
    }

    int indices_seleccionados[50];
    int n_indices = 0;

    if (n_seleccionadas > 0) {
        for (int i = 0; i < n_seleccionadas; i++) {
            for (int j = 0; j < n_campos; j++) {
                if (compararTotal(columnasSeleccionadas[i], campos[j])) {
                    indices_seleccionados[n_indices++] = j;
                    break;
                }
            }
        }
    } else {
        for (int j = 0; j < n_campos; j++) {
            indices_seleccionados[n_indices++] = j;
        }
    }

    // **NUEVA SECCIÓN: Imprimir cabecera de columnas**
    FILE *salida = archivo_salida ? fopen(archivo_salida, "w") : NULL;
    
    // Imprimir cabecera con las columnas seleccionadas
    bool primero = true;
    for (int k = 0; k < n_indices; k++) {
        int idx = indices_seleccionados[k];
        if (idx >= 0 && idx < n_campos) {
            if (!primero) {
                if (salida) fputc('|', salida);
                else putchar('|');
            }
            if (salida) fprintf(salida, "%s", campos[idx]);
            else printf("%s", campos[idx]);
            primero = false;
        }
    }
    if (salida) fputc('\n', salida);
    else putchar('\n');
    // **FIN DE NUEVA SECCIÓN**

    // Ruta de índice
    char ruta[256];
    strcpy(ruta, ruta_base.c_str());
    strcat(ruta, "/0/1/0/1");

    FILE *indice = fopen(ruta, "r");
    if (!indice) {
        write(1, "[-] No se pudo abrir el índice\n", 31);
        return true;
    }

    char buffer[256];
    bool imprimir = false;

    while (fgets(buffer, sizeof(buffer), indice)) {
        if (buffer[0] == '#' && compararTotal(quitarEspacios(buffer + 1), tabla)) {
            imprimir = true;
            continue;
        }
        if (imprimir && buffer[0] == '#') break;
        if (imprimir) {
            quitarEspacios(buffer);
            std::string datos_string = mydisk->leer(quitarEspacios(buffer));

            char datos[4096];
            strncpy(datos, datos_string.c_str(), sizeof(datos) - 1);
            datos[sizeof(datos) - 1] = '\0';

            char *linea_start = datos;
            char *linea_end;

            while ((linea_end = strchr(linea_start, '\n')) != NULL) {
                *linea_end = '\0';

                if (!condiciones || evaluarCondiciones(linea_start, condiciones, campos, tipos, n_campos)) {
                    // Extraer todos los campos de la línea
                    char campos_linea[50][100];
                    int num_campos_linea = 0;
                    extraerCampos(linea_start, campos_linea, &num_campos_linea);
                    
                    // Imprimir solo los campos seleccionados
                    bool primero = true;
                    for (int k = 0; k < n_indices; k++) {
                        int idx = indices_seleccionados[k];
                        if (idx >= 0 && idx < num_campos_linea) {
                            if (!primero) {
                                putchar('|');
                                if (salida) fputc('|', salida);
                            }
                            printf("%s", campos_linea[idx]);
                            if (salida) fprintf(salida, "%s", campos_linea[idx]);
                            primero = false;
                        }
                    }
                    putchar('\n');
                    if (salida) fputc('\n', salida);
                }

                linea_start = linea_end + 1;
            }

            // Procesar última línea si no termina con \n
            if (*linea_start) {
                if (!condiciones || evaluarCondiciones(linea_start, condiciones, campos, tipos, n_campos)) {
                    // Extraer todos los campos de la línea
                    char campos_linea[50][100];
                    int num_campos_linea = 0;
                    extraerCampos(linea_start, campos_linea, &num_campos_linea);
                    
                    // Imprimir solo los campos seleccionados
                    bool primero = true;
                    for (int k = 0; k < n_indices; k++) {
                        int idx = indices_seleccionados[k];
                        if (idx < num_campos_linea) {
                            if (!primero) {
                                if (salida) fputc('|', salida);
                                else putchar('|');
                            }
                            if (salida) fprintf(salida, "%s", campos_linea[idx]);
                            else printf("%s", campos_linea[idx]);
                            primero = false;
                        }
                    }
                    if (salida) fputc('\n', salida);
                    else putchar('\n');
                }
            }
        }
    }

    if (salida){
        fclose(salida);
        if (!agregar_a_esquema(mydisk, archivo_salida,archivo_salida,'|')) return false;
        // Buscar sector libre desde (0,0,0,1)
        unsigned int d = 0, p = 0, s = 1;
        int c = 0;
        insertar_tabla(archivo_salida, '|', mydisk, d, c, p, s,archivo_salida);
    };
    fclose(indice);

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

        unsigned int discos,pistas,sectores,tam,bloque;
        write(1,"\nPlatos: ",9);
        scanf("%u", &discos);
        write(1,"\nPistas: ",9);
        scanf("%u", &pistas);
        write(1,"\nSectores: ",11);
        scanf("%u", &sectores);
        write(1,"\nTamano del sector: ",20);
        scanf("%u", &tam);
        write(1,"\nTamano de bloque: ",19);
        scanf("%u", &bloque);
        getchar();

        //printf("crear discoo\n");
        mydisk->crear(quitarEspacios(str+tamano((char *) palabras_adicionales[1])),discos,pistas,sectores,tam,bloque);

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

    //! *******
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

    /* if(buscar("SET-BLOQUE", mayus)==0){ //setear el bloque  
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
    } */


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
