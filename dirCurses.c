#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <curses.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define flechaArriba 0x1B5B41
#define flechaAbajo 0x1B5B42
#define flechaDerecha 0x1B5B43
#define flechaIzquierda 0x1B5B44
#define enter 10

/*** VARIABLES GLOBALES ***/
struct s_dir {
    int tipo;
    char *nombre;
} res[128];

int fd; // Archivo a leer

/*** DECLARACIÓN DE FUNCIONES ***/
int leeChar();
int leerDirectorio(char *cwd);
char *hazLinea(char *base, int dir);
char *mapFile(char *filePath);
//char *navegarDirectorios(char *cwdLogin, char *cwd, int i);

int main() {
   int j = 0, k = 0, c; 
   int i = 0; //Número de posición del elemento en la pantalla 
   int pos = 0; //Número de posición del elemento del directorio 
   int longDir = 0; //Número de elementos que tiene el directorio
   int maxPantalla = 10; //Número de elementos máximo que puedo ver en la pantalla
   int offset = 0; //A partir de qué directorio estoy viendo en pantalla
   char *cwd; //Cadena que guarda la ruta 

   initscr(); //Determina el tipo de la terminal
   raw();
   noecho(); //Inhabilita mostrar el caracter leido
   cbreak(); //Desabilita el line buffering 

    cwd = getcwd(NULL,0); //Obtener la ruta actual 
    longDir = leerDirectorio(cwd); //Obtener la longitud del directorio y su contenido 

   do {
      for (j=0; j < maxPantalla && (j + offset) < longDir ; j++) {
         if (j == i) {
           attron(A_REVERSE);
         }
         mvprintw(5+j,5,res[j+offset].nombre);
         attroff(A_REVERSE);
      }
      move(5+i,5);
      refresh();
      c = leeChar(); 
      
      switch(c) {
         case flechaArriba:
            if (i > 0 ){
              i += -1; 
            } else {
              if (offset > 0) {
                offset -= 1; 
              } else {
                offset = (longDir > maxPantalla) ? longDir - maxPantalla : 0;
                i = (longDir > maxPantalla) ? maxPantalla - 1 : longDir - 1 ; 
              }
              clear();
            }
            pos = offset + i; 
            break;
         case flechaAbajo:
            if(longDir <= maxPantalla){
              i = (pos == longDir-1) ? 0 : i + 1;
            } else {
                if (pos == longDir-1){ //Si se ha llegado al último elemento del directorio
                  offset = 0;
                  i = 0; 
                } else {
                  if(i == maxPantalla - 1){
                    offset += i;
                    i = 0;
                  } else{
                    i += 1;
                  }
                }
              clear();
            }
            pos = offset + i;
            break;
         case enter:
            offset = 0;         
            clear();
            if(strcmp("..", res[pos].nombre) == 0){ //Si es un directorio ".." regresa a un directorio padre
                char *aux = strrchr(cwd, '/'); //Encuetra el último caracter /
                *aux = 0; 
                longDir = leerDirectorio(cwd);
                i = 0; pos = 0;
                clear();
            } else if(res[pos].tipo == DT_DIR) { //Si es un directorio 
                strcat(cwd, "/");
                strcat(cwd,res[pos].nombre); //Otener el directorio al que se quiere mover
                longDir = leerDirectorio(cwd); //Obtener la longitud del directorio y su contenido 
                i = 0; pos = 0;
                clear();
            } else { //Si es un archivo
                clear();
                char *map = mapFile(res[pos].nombre);
                if (map == NULL) {
                  exit(EXIT_FAILURE);
                }       
                for(int k= 0; k<25; k++) {
                    // Haz linea, base y offset
                    char *l = hazLinea(map,k*16);
                    mvprintw(k,0,l);
                }
                refresh();

                int x = 9;
                int y = 0;

                do{
                  move(0+y,x);
                  c = leeChar();
                  switch(c){
                    case flechaArriba:
                      if(y > 0){ 
                        y -= 1;
                      }
                      break;
                    case flechaAbajo:
                      if(y < 24) {
                        y += 1;
                      }
                      break; 
                    case flechaDerecha:
                      if(x > 8 && x < 72) {
                        x = (x < 56) ? x + 3 : x + 1;
                      } else {
                        if (x > 60 && y < 24) {
                          y += 1; x = 9; 
                        }
                      }
                      break;
                    case flechaIzquierda:
                      if(x > 10) {
                        x = (x > 57) ? x - 1 : x - 3;
                      } 
                    break;
                  }
                } while(c!=24);
                close(fd);
                clear();
                i = 0; pos = 0;           
            }
            //cwd = navegarDirectorios(cwdLogin, cwd, i);
            break;
         default:
            // Nada
            break;
      }
      move(2,10);
      printw("i: %d  pos: %d  Offset %d Directorio actual: %s Leí %d", i, pos, offset, cwd, c);
   } while (c != 'q');

   endwin();   
   return 0;
}

int leeChar() {
  int chars[5];
  int ch,i=0;
  nodelay(stdscr, TRUE);
  while((ch = getch()) == ERR); // Espera activa 
  ungetch(ch);
  while((ch = getch()) != ERR) {
    chars[i++]=ch;
  }
  //convierte a numero con todo lo leido
  int res=0;
  for(int j=0;j<i;j++) {
    res <<=8;
    res |= chars[j];
  }
  return res;
}

int leerDirectorio(char *cwd){
    DIR *dir = opendir(cwd);
    struct dirent *dp;
    int i = 0; 

    while((dp=readdir(dir)) != NULL) {
      res[i].tipo = dp->d_type;
      res[i].nombre=dp->d_name;
      i++;
   }
    closedir(dir);
   return i;
}

char *hazLinea(char *base, int dir) {
	char linea[100]; // La linea es mas pequeña
	int o=0;
	// Muestra 16 caracteres por cada linea
	o += sprintf(linea,"%08x ",dir); // offset en hexadecimal
	for(int i=0; i < 4; i++) {
		unsigned char a,b,c,d;
		a = base[dir+4*i+0];
		b = base[dir+4*i+1];
		c = base[dir+4*i+2];
		d = base[dir+4*i+3];
		o += sprintf(&linea[o],"%02x %02x %02x %02x ", a, b, c, d);
	}
	for(int i=0; i < 16; i++) {
		if (isprint(base[dir+i])) {
			o += sprintf(&linea[o],"%c",base[dir+i]);
		}
		else {
			o += sprintf(&linea[o],".");
		}
	}
	sprintf(&linea[o],"\n");

	return(strdup(linea));
}

char *mapFile(char *filePath) {
    //Abrir archivo 
    fd = open(filePath, O_RDONLY);
    if (fd == -1) {
    	perror("Error abriendo el archivo");
	    return(NULL);
    }
    //Mapea el archivo 
    struct stat st;
    fstat(fd,&st);
    int fs = st.st_size;

    char *map = mmap(0, fs, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
    	close(fd);
	    perror("Error mapeando el archivo");
	    return(NULL);
    }
  return map;
}

/*char *navegarDirectorios(char *cwdLogin, char *cwd, int i){
  char cwdDirAnt[256];
  int login, longDir, prueba = 1; 

  system("clear");
  //Si te encuentras en el dir login no hagas nada
  if(strcmp(cwd,cwdLogin) == 0){ 
      login = 1;
  } else {
    login = 0;
  }
  //Si es un directorio "." regresa al directorio de login
  if(strcmp(".", res[i].nombre) == 0){  
      if(!login) {
          strcpy(cwd,cwdLogin);
          longDir = leerDirectorio(cwd);
      }
  //Si es un directorio ".." regresa a un directorio superior
  } else if(strcmp("..", res[i].nombre) == 0){ 
      if(!login) {
          longDir = leerDirectorio(cwdDirAnt);
      }
  } else { 
      if(!prueba) { //Si es un archivo 
          system("clear");
          char *map = mapFile(res[i].nombre);
          if (map == NULL) {
          exit(EXIT_FAILURE);
          }       
          for(int i= 0; i<25; i++) {
              // Haz linea, base y offset
              char *l = hazLinea(map,i*16);
              mvprintw(i,0,l);
          }
          refresh();
          close(fd);
      } else { //Si es un directorio
          strcpy(cwdDirAnt, cwd);
          strcat(cwd, "/");
          strcat(cwd,res[i].nombre); //Otener el directorio al que se quiere mover
          longDir = leerDirectorio(cwd); //Obtener la longitud del directorio y su contenido 
      }
  }
  return cwd;
}*/