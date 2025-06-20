#include "generales.h"
#include "terminal.h"
#include "archivos.h"
#include "DiscoFisico.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <filesystem>
#include "BufferManager.h"

BufferManager * bufferManager;

namespace fs = std::filesystem;


bool campos_create_tabla(int tam,char *str, char *nombre_tabla, char *archivo, char *separador, char * modo) {
    //str += tamano((char *)"CREATE");
    str+=tam;
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
    } else 
        *separador = *str ? *str : ',';

    str++;
    while (*str == ' ') str++; 
    if (*str) {
        *modo = *str; 
    } else {
        printf("modo defecto -> Fijo\n");
        *modo = 'F'; 
    }
    return true;
}

bool crear_tabla(char *str, DiscoFisico *disk) {
    char nombre[32], archivo[64], sep, modo;
    if (!campos_create_tabla(tamano("CREATE"),str, nombre, archivo, &sep, &modo)|| !(modo=='V' || modo =='F' || modo=='f' || modo=='v')) {
        write(1, "erorr parametros invalidos\n", 28);
        return false;
    }
    printf("nombre %s archivo %s separador %c modo %c*\n",nombre,archivo,sep,modo);
    
    char cabecera[512];
    if (!agregar_a_esquema(disk, nombre,archivo,sep, cabecera)) return false;
    int tamanos[20];
    int i=0;
    int idx=1;
    //printf("cabecera %s\n",cabecera);
    
    while(i<tamano(cabecera)){
        while((cabecera+i) and *(cabecera+i)!='#')
            putchar(*(cabecera+i++));
        printf(": ");
        i++;
        scanf("%d", &tamanos[idx++]);
        //putchar('\n');
    }
    getchar();
    tamanos[0]=idx;
    // Buscar sector libre desde (0,0,0,1)
    unsigned int d = 0, p = 0, s = 1;
    int c = 0;
    for (int i = 0; i < idx; i++){
        printf("lista tam %d - %d\n",i,tamanos[i]);
    }
    return insertar_tabla(-1,archivo, sep, disk, d, c, p, s,nombre, modo, tamanos);
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
//!----------------------

bool  procesar_select(char *str, char *mayus, DiscoFisico *mydisk) {
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
    //printf("numero de campos %d\n",n_campos);
    if(n_indices<1)
        return 0;
    // **NUEVA SECCIÓN: Imprimir cabecera de columnas**
    FILE *salida = archivo_salida ? fopen(archivo_salida, "w") : NULL;
    
    // Imprimir cabecera con las columnas seleccionadas
    bool primero = true;
    for (int k = 0; k < n_indices; k++) {
        int idx = indices_seleccionados[k];
        if (idx >= 0 && idx < n_campos) {
            if (!primero) {
                if (salida) fputc('|', salida);
                putchar('|');
            }
            if (salida) fprintf(salida, "%s", campos[idx]);
            printf("%s", campos[idx]);
            primero = false;
        }
    }
    if (salida) fputc('\n', salida);
    putchar('\n');

    // Ruta de índice
    char ruta[256];
    strcpy(ruta, ruta_base.c_str());
    strcat(ruta, "/0/1/0/1");

    FILE *indice = fopen(ruta, "r");
    if (!indice) {
        write(1, "[-] No se pudo abrir el índice\n", 31);
        return true;
    }

    char buffer[25];
    bool imprimir = false;

    int * tamanos =new int[20];
    //tamanos[0]=0;
    int iTamanos=1;
    while (fgets(buffer, sizeof(buffer), indice)) {
        if (buffer[0] == '#' && compararTotal(quitarEspacios(buffer + 1), tabla)) {
            imprimir = true;
            continue;
        }
        if (imprimir && buffer[0] == '#') break;
        if (imprimir) {
            quitarEspacios(buffer);
            //for (int i=0;i<mydisk->tam_bloque;i++){
                char ruta[20];
                //printf("buffer %s",buffer);
                //mydisk->encontrarSector(ruta,stoi(buffer),i);
                //std::string datos_string = mydisk->leer(quitarEspacios(ruta));
                std::string *datos_string = bufferManager->acceder(stoi(buffer), Operacion::Leer);
                char datos[4096];
                //*datos_string+="#";
                std::string datos_con_extra = *datos_string + "#"; 
                strncpy(datos, datos_con_extra.c_str(), sizeof(datos) - 1);
                datos[sizeof(datos) - 1] = '\0';

                char *linea_start = datos;
                char *linea_end;
                bool cabecera = true;
                
                while ((linea_end = strchr(linea_start, '\n')) != NULL) {
                    *linea_end = '\0';
                    if (!condiciones || evaluarCondiciones(linea_start, condiciones, campos, tipos, n_campos)) {
                        // Extraer todos los campos de la línea
                        char campos_linea[50][100];
                        int num_campos_linea = 0;
                        extraerCampos(linea_start, campos_linea, &num_campos_linea);
                        
                        // Imprimir solo los campos seleccionados
                        bool primero = true;
                        //tamanos[iTamanos]=0;
                        for (int k = 0; k < n_indices; k++) {
                            int idx = indices_seleccionados[k];
                            if (idx >= 0 && idx < num_campos_linea) {
                                if(cabecera||campos_linea[idx][0]=='-'){
                                    cabecera=true;
                                    break;
                                }
                                if (!primero) {
                                    putchar('|');
                                    if (salida) fputc('|', salida);
                                }
                                int temp =tamano(campos_linea[idx]);
                                tamanos[k+1]= temp;
                                //printf("tamanos %d -%d\n",tamano(campos_linea[idx]),tamanos[k+1]);
                                //iTamanos++;
                                /* if(campos_linea[idx][0]=='-'){
                                    cabecera=true;
                                    break;
                                }  */
                                for (int i = 0; i < tamano(campos_linea[idx]); i++){
                                    if(campos_linea[idx][i]!='~'){
                                        putchar(campos_linea[idx][i]);
                                        if(salida) fputc(campos_linea[idx][i],salida);
                                    }

                                }
                               
                                primero = false;
                            }
                        }
                        if(!cabecera){
                            putchar('\n');
                            if (salida) fputc('\n', salida);
                        }else
                            cabecera=false;
                    }
                    linea_start = linea_end + 1;
                }
                //printf("****unpin %d\n",stoi(buffer));
                bufferManager->unpin(stoi(buffer));
            //}
        }
    }

    if (salida){
        fclose(salida);
        char nulle[250];
        if (!agregar_a_esquema(mydisk, archivo_salida,archivo_salida,'|',nulle)) return false;
        // Buscar sector libre desde (0,0,0,1)
        unsigned int d = 0, p = 0, s = 1;
        int c = 0;
        iTamanos = n_indices+1;
        tamanos[0]=iTamanos;
        for (int rr = 0; rr < iTamanos; rr++){
            printf("lista %d - %d\n",rr,tamanos[rr]);
        }
        insertar_tabla(-1,archivo_salida, '|', mydisk, d, c, p, s,archivo_salida,'F',tamanos);
        delete tamanos;
    }
    fclose(indice);

    return true;
}


bool insertarArchivo(char * str, DiscoFisico * mydis){
    char nombre[32], archivo[64], sep, modo;
    char cantidad[5];
    str+=tamano("insert");
    while (*str == ' ') str++;

    int i=0;
    //while(*str && *str != ' '){
    while(!isalpha(*str) and *str != ' '){
        cantidad[i++]=*str++;
    }
    cantidad[i]='\0';

    if (!campos_create_tabla(0,str, nombre, archivo, &sep,&modo)) {
        write(1, "ERROR: parámetros inválidos\n", 28);
        return false;
    }
    printf("nombre *%s*\n",nombre);
    printf("archivo *%s*\n",archivo);
    printf("separador *%c*\n",sep);
    printf("cantidad *%s*\n",cantidad);
    printf("cantidad *%d*\n",stoi(cantidad)); 
    unsigned int d = 0, p = 0, s = 1;
    int c = 0;
    char null[500];
    if(!buscarEsquema(nombre,null)){
        printf("[-]No existe tabla(%s) con ese nombre\n",nombre);
        return 0;
    }
    return insertar_tabla(stoi(cantidad),archivo, sep, mydis, d, c, p, s,nombre, 'F');
}


bool procesar_eliminar(char * str, DiscoFisico * mydisc){
    char mayus [tamano (str)];
    mayusculas(str,mayus); 
    
    //delete from tabla where adsfa

    int pos_from = buscar("FROM", mayus);
    int pos_where = buscar("WHERE",mayus);
    if(pos_from ==-1 || pos_where==-1){
        printf("[-] error sintaxis incorrecta\n");
        return false;
    }
    char * nombre = str + pos_from+ tamano("from");
    *(str + pos_where) = '\0';
    nombre = quitarEspacios(nombre);
    char * condicion = str + pos_where + tamano("where");
    condicion = quitarEspacios(condicion);
    printf("nombre *%s*\n", nombre);
    printf("condicon *%s*\n", condicion);
    
    // Obtener esquema de la tabla
    char relacion[250];
    if (!buscarEsquema(nombre, relacion)) {
        printf("[-] No hay relacion en esquema para tabla: %s\n", nombre);
        return false;
    }

    // Extraer campos y tipos del esquema
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
    
    //----
    char ruta[256];
    strcpy(ruta, ruta_base.c_str());
    strcat(ruta, "/0/1/0/1");

    FILE *indice = fopen(ruta, "r");
    if (!indice) {
        write(1, "[-] No se pudo abrir el índice\n", 31);
        return true;
    }

    char buffer[256];
    bool encontrado = false;
    char ruta_datos[20];

    while (fgets(buffer, sizeof(buffer), indice)) {
        if (buffer[0] == '#' && compararTotal(quitarEspacios(buffer + 1), nombre)) {
            encontrado = true;
            continue;
        }
        if (encontrado && buffer[0] == '#') break;
        
        if (encontrado) {
            quitarEspacios(buffer);
            //?for (int i = 0; i < mydisc->tam_bloque; i++) {
                //if(!mydisc->encontrarSector(ruta_datos, stoi(buffer), i))
                  //  break;
                //std::string datos_string = mydisc->leer(quitarEspacios(ruta_datos));
                std::string *datos_string = bufferManager->acceder(stoi(buffer),Operacion::Eliminar);

                char datos[4096];
                //datos_string += "#";
                //strncpy(datos, datos_string.c_str(), sizeof(datos) - 1);
                std::string datos_con_extra = *datos_string + "#"; 
                strncpy(datos, datos_con_extra.c_str(), sizeof(datos) - 1);
                datos[sizeof(datos) - 1] = '\0';
                char *linea_start = datos;
                char *linea_end;
                std::string datos_modificados = "";

                while ((linea_end = strchr(linea_start, '\n')) != NULL) {
                    *linea_end = '\0';

                    if (campos[0][0]!='-' and evaluarCondiciones(linea_start, condicion, campos, tipos, n_campos)) {
                        // Marcar registro como eliminado sobrescribiendo el primer carácter con '-'
                        printf("elimando en %s\n",ruta_datos);
                        bufferManager->high_dirty_bit(stoi(buffer));
                        std::string linea_eliminada = "-";
                        linea_eliminada += (linea_start + 1);  // Agregar el resto de la línea
                        datos_modificados += linea_eliminada + "\n";
                    } else {
                        // Mantener la línea sin cambios
                        datos_modificados += linea_start;
                        datos_modificados += "\n";
                    }

                    linea_start = linea_end + 1;
                }
                *datos_string = datos_modificados;
                printf("--------------------\n%s",datos_string->c_str());
                // Escribir los datos modificados usando FILE
                /*
                char ruta_completa[512];
                strcpy(ruta_completa, ruta_base.c_str());
                strcat(ruta_completa, "/");
                strcat(ruta_completa, ruta_datos);
                FILE *archivo = fopen(ruta_completa, "w");
                if (archivo) {
                    fputs(datos_modificados.c_str(), archivo);
                    fclose(archivo);
                    mydisc->actualizarCabeceraFija(ruta_completa);
                }*/
                //printf("pp\n");
            //?}
            bufferManager->unpin(stoi(buffer));
        }
    }
    printf("registros eliminados\n");
    fclose(indice);
    return true;
}


// -------------------CONSULTA-------------------

int procesar_consulta(char * str, DiscoFisico * mydisk){
    char mayus[tamano(str)];
    mayusculas(str,mayus);

    if (estaLiteral(mayus,palabras_salida)){ //salir
        mydisk->reporte();
        printf("\n[+] Saliendo...\n");
        if(bufferManager)
            delete bufferManager;
        return 0;
    }      
    
    if (buscar((char*)palabras_adicionales[2],mayus)==0){ // SELECT-DISCO
        //printf("%s",quitarEspacios(str+tamano((char *) palabras_adicionales[2])));
        if( mydisk->inicializar(quitarEspacios(str+tamano((char *) palabras_adicionales[2]))))
            bufferManager = new BufferManager(4,mydisk);

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
    
    if(buscar((char *)palabras_reservadas[5],mayus)==0){ //CREATE tabla archivo separador
        crear_tabla(str, mydisk);
        return 1;
    }

    if(buscar((char *)palabras_reservadas[1],mayus)==0){ //SELECT
        //printf("%s\n",ruta_base.c_str());
        
        procesar_select(str,mayus, mydisk);
        return 1;
        
        //procesar_select()
    } 
    //! *******
    
    if (buscar((char*)palabras_reservadas[6],mayus)==0){ //insert 32 tabla Arhchivo.txt separador
        printf("insert\n");
        insertarArchivo(str,mydisk);
        
        return 1;
    }

    if (buscar("ESQUEMA",mayus)==0){ // esquema, para ver nuestras relaciones
        if (discoInicializado()){
            printf("Disco no inicializado\n");
            return 1;
        }
        write(1,(char*)(mydisk->leer(0,1,0,0)).c_str(),tamano((char*)mydisk->leer(0,1,0,0).c_str(),'\0'));
        return 1;   
    }
    
    if (buscar((char*)palabras_adicionales[0],mayus)==0){ //reporte
        mydisk->reporte();
        //printf("reporanrrr\n");
        return 1;
    }
    
    //! *******

    if (buscar("DELETE",mayus)==0){ //delete from <tabla> where <id> ==5
        printf("elimando...\n");
        procesar_eliminar(str,mydisk);
        return 1;
    }
    
    if(buscar("SET-BUFFER", mayus)==0){ //setear el bloque  
        if (discoInicializado()){
            printf("Disco no inicializado\n");
            return 1;
        }
        int tam;
        write(1,"tamano de buffer: ",18);
        scanf("%u", &tam);
        getchar();
        if(bufferManager)
            delete bufferManager;
        bufferManager = new BufferManager(tam,mydisk);

        return 1;
    } 

    if (buscar("INDICE",mayus)==0){
        if (discoInicializado()){
            printf("Disco no inicializado\n");
            return 1;
        }
        write(1,(char*)(mydisk->leer(0,1,0,1)).c_str(),tamano((char*)mydisk->leer(0,1,0,1).c_str(),'\0'));
        return 1; 
    }

    if(buscar("BLOQUE-BUFFER-PIN",mayus)==0){
        char id[10] = {0}; 
        int idx=0;
        str = str + tamano("bloque-buffer-pin");
        while(*str == ' ') str++; 
        while (*(str+idx) && *(str+idx) != '\n' && *(str+idx) != ' '){ 
            id[idx] = *(str+idx); 
            idx++;
        }
        id[idx] = '\0'; 
        printf("*****%s\n",id);
        int num_id = stoi(id);
        str+=idx;
        while(*str == ' ') str++; 
        char c= *str;
        printf("oeracion %d -- %c\n",num_id,c);
        Operacion ope;
        if (c=='L')
            ope = Operacion::Leer;
        else if (c=='W')
            ope= Operacion::Eliminar;
        else{
            printf("ingrese operacion correcta\n");
            return 1;            
        }
        string* resultado= bufferManager->acceder(num_id,ope);
        if (resultado==NULL){
            return 1;
        }
        
        printf("%s", resultado->c_str());
        //bufferManager->unpin(num_id);
        printf("PIN PERMANENETE\n");
        return 1;
    }

    if(buscar("BLOQUE-BUFFER-UNPIN",mayus)==0){
        char id[10] = {0}; 
        int idx=0;
        str = str + tamano("bloque-buffer-UNpin");
        while(*str == ' ') str++; 
        while (*(str+idx) && *(str+idx) != '\n' && *(str+idx) != ' '){ 
            id[idx] = *(str+idx); 
            idx++;
        }
        id[idx] = '\0'; 
        int num_id = stoi(id);
        str+=idx;
        while(*str == ' ') str++; 
        char c= *str;
        printf("oeracion %d -- %c\n",num_id,c);
        Operacion ope;
        if (c=='L')
            ope = Operacion::Leer;
        else if (c=='W')
            ope= Operacion::Eliminar;
        else{
            printf("ingrese operacion correcta\n");
            return 1;            
        }
        printf("UNPIN del permaneten\n");

        //printf("%s", bufferManager->acceder(num_id,ope)->c_str());
        bufferManager->unpin(num_id);
        return 1;
    }

    if(buscar("BLOQUE-BUFFER",mayus)==0){
        char id[10] = {0}; 
        int idx=0;
        str = str + tamano("bloque-buffer");
        while(*str == ' ') str++; 
        while (*(str+idx) && *(str+idx) != '\n' && *(str+idx) != ' '){ 
            id[idx] = *(str+idx); 
            idx++;
        }
        id[idx] = '\0'; 
        int num_id = stoi(id);
        str+=idx;
        while(*str == ' ') str++; 
        char c= *str;
        printf("oeracion con desfija automatico %d -- %c\n",num_id,c);
        Operacion ope;
        if (c=='L')
            ope = Operacion::Leer;
        else if (c=='W')
            ope= Operacion::Eliminar;
        else{
            printf("ingrese operacion correcta\n");
            return 1;            
        }
        
        string* resultado= bufferManager->acceder(num_id,ope);
        if (resultado==NULL){
            return 1;
        }
        
        printf("%s", resultado->c_str());
        bufferManager->unpin(num_id);
        return 1;
    }

    if(buscar("BLOQUE-DISCO",mayus)==0){
        char id[10] = {0}; 
        int idx=0;
        str = str + tamano("bloque-DISCO");
        while(*str == ' ') str++; 
        while (*(str+idx) && *(str+idx) != '\n' && *(str+idx) != ' '){ 
            id[idx] = *(str+idx); 
            idx++;
        }
        id[idx] = '\0'; 
        int num_id = stoi(id);
        //printf("id %d\n", num_id);
        char ruta[20];
        for(int ii = 0; ii < mydisk->tam_bloque;ii++){
            if(!mydisk->encontrarSector(ruta,num_id,ii)) return false;
            printf("--sector: %s\n%s\n",ruta, mydisk->leer(ruta).c_str());
        }
        
        return 1;
    }
    
    if(buscar("MOSTRAR",mayus)==0){
        char ruta[20];
        int num_id=0;
        while(1){
            int capacidad=0;
            for(int ii = 0; ii < mydisk->tam_bloque;ii++){
                if(!mydisk->encontrarSector(ruta,num_id,ii)) return 1;
                capacidad+= fs::file_size(ruta_base+"/"+ ruta);
            }
            printf("bloque %d - capacidad total %d - capacidad usada %d\n",num_id, mydisk->tam_bloque*mydisk->tam_sector,capacidad);
            num_id++;
        }
        return 1;
    }

    if(buscar("BUFFER",mayus)==0){
        bufferManager->ver_tabla();
        return 1;
    }

    if(buscar("GUARDAR",mayus)==0){
         char id[10] = {0}; 
        int idx=0;
        str = str + tamano("GUARDAR");
        while(*str == ' ') str++; 
        while (*(str+idx) && *(str+idx) != '\n' && *(str+idx) != ' '){ 
            id[idx] = *(str+idx); 
            idx++;
        }
        id[idx] = '\0'; 
        int num_id = stoi(id);
        bufferManager->guardar(num_id);
        return 1;
    }

    if(buscar("ELIMINAR",mayus)==0){
         char id[10] = {0}; 
        int idx=0;
        str = str + tamano("ELIMINAR");
        while(*str == ' ') str++; 
        while (*(str+idx) && *(str+idx) != '\n' && *(str+idx) != ' '){ 
            id[idx] = *(str+idx); 
            idx++;
        }
        id[idx] = '\0'; 
        int num_id = stoi(id);
        bufferManager->eliminar(num_id);
        printf("[+] pagina %d elimanda",num_id);
        return 1;
    }

    
    //if(buscar("TREE",mayus)==0){}

    write(1,"Consulta invalida usa HELP\n",27);
    return 1;
}


// -------------------TERMINAL-------------------
void terminal(){
    
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    bool continuar = 1;
    bool disco = false;
    DiscoFisico * myDisk = new DiscoFisico();
    if(myDisk->inicializar("default"))
        bufferManager = new BufferManager(4,myDisk);
    //myDisk->crear("d",3,3,3,500,3);
    //write(1,"\n",1);
    write(1,"Welcom to MEGATRON3000!!!\n->  ",30);
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

    
    //reporte de disco
}
