#include "generales.h"
#include <ctype.h>

const char *palabras_reservadas[] = {"HELP","SELECT","FROM","WHERE","|","CREATE","INSERT",NULL};
const char *palabras_adicionales[] = {"REPORTE","CREATE-DISCO","SELECT-DISCO",NULL};
const char *palabras_salida[] = {"QUIT","EXIT","Q",NULL};

//unsigned char tamano()

/* int buscar_espacio(char * str, unsigned char inicio){

} */

int tamano(char * str, char separador){
    int i=0;
    while (str[i] && str[i]!=separador){
        i++;
    }
    return i;
}

bool compararTotal(char * a, char * b){
/*     printf("a: %s - %d\n",a,tamano(a));
    printf("b: %s - %d\n",b,tamano(b)); */
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return *a == *b;
}

bool comparar(char * a, char * b){ //B es siempre mayus
    mayusculas(a,a);
    return compararTotal(a,b);
}

//! para usar restar -1
int estaLiteral(char * str,const char **lista){
    for (int i = 0; lista[i]; ++i) {
        //printf("\n%s - %s",str, lista[i]);
        if (compararTotal(str,(char*)lista[i])) {
            return i+1; // 
        }
    }
    return 0;
}

int estaParcial(char * str,const char **lista){
    for (int i = 0; lista[i]; ++i) {
        //printf("\n%s - %s",str, lista[i]);
        if (comparar(str,(char*)lista[i])) {
            return i+1; // 
        }
    }
    return 0;
}

char *mayusculas(char *origen, char *destino) {
    if (!origen || !destino) return NULL;
    unsigned char i;
    for ( i = 0; origen[i]; ++i) {
        destino[i] = (origen[i] >= 'a' && origen[i] <= 'z') ? origen[i] - 32 : origen[i];
    }
    destino[i] = '\0';
    return destino;
}

int buscar(char * val, char * str){
    if (!val || !str) return -1;

    for (int i = 0; str[i]; ++i) {
        int j = 0;
        while (val[j] && str[i + j] == val[j]) ++j;
        if (!val[j]) return i;
    }

    return -1;
}

void procesarLinea(char *str, char *linea[], char separador) {
    int i = 0;
    char *p = str;

    while (*p) {
        while (*p == ' ') p++;

        char *inicio = p;
        char *fin = p;

        if (*p == '"') {
            p++;             
            inicio = p;
            while (*p && *p != '"') p++;
            fin = p;
            if (*p == '"') p++; 
        } else {
            inicio = p;
            while (*p && *p != separador) {
                if (*p != ' ') fin = p + 1;
                p++;
            }
        }

        int len = fin - inicio;
        if (len > 0) {
            linea[i] = (char *)malloc(len + 1);
            if (linea[i]) {
                for (int j = 0; j < len; j++)
                    linea[i][j] = inicio[j];
                linea[i][len] = '\0';
                i++;
            }
        }

        while (*p == ' ' || *p == separador) p++;
    }

    linea[i] = NULL;
}

char * quitarEspacios(char *str) {
    if (!str) return NULL; 
    
    while (*str == ' ') ++str;
    
    char *inicio = str;
    char *dest = str;
    while (*str && *str!='\0') *dest++ = *str++;
    
    *dest = '\0';
    dest--;
    while (dest >= inicio && (*dest == ' '||*dest == '\n')) {
        *dest-- = '\0';
    }
    return inicio;
}

/* char * concatenar(char * a, char * b, char * res){

}
 */
const char* tipo_dato(char *campo) {
    if (!campo || campo[0] == '\0') return "string";  // vacío se trata como string

    int i = 0;
    bool tienePunto = false;

    while (campo[i]) {
        if (isdigit(campo[i])) {
            i++;
        } else if (campo[i] == '.' && !tienePunto) {
            tienePunto = true;
            i++;
        } else {
            return "string";  // cualquier otra cosa => string
        }
    }

    return tienePunto ? "float" : "int";
}


void printCabecera(char * str, int pos){
    for (char i = pos; i < tamano(str)-1; i++){
        if (str[i]=='#'){
            putchar(' ');
            putchar('|');
            putchar(' ');
        } else
            putchar(str[i]);
    }
    putchar('\n');
}

char *tipoDato(char *valor) {
    int i = 0, punto = 0;
    if (valor[0] == '-' || valor[0] == '+') i++;
    for (; valor[i]; i++) {
        if (valor[i] == '.') {
            if (++punto > 1) return (char *)"string";
        } else if (valor[i] < '0' || valor[i] > '9') {
            return (char *)"string";
        }
    }
    if (punto == 1) return (char *)"float";
    return (char *)"int";
}
