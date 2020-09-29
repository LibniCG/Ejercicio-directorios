#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <curses.h>
#include <string.h>

struct s_dir {
    int tipo;
    char *nombre;
} res[128];

/*** DECLARACIÓN DE FUNCIONES ***/
int leeChar();
int leerDirectorio(char *cwd);

int main() {
   int i = 0, j = 0, longDir = 0, c;
   char *cwd, *cwdLogin;

   initscr(); //Determina el tipo de la terminal
   raw();
   noecho(); //Inhabilita mostrar el caracter leido
   cbreak(); //Desabilita el line buffering 

    cwdLogin = getcwd(NULL,0); //Obtener el directorio de login
    cwd = getcwd(NULL,0); //Obtener la ruta actual 
    longDir = leerDirectorio(cwd); //Obtener la longitud del directorio y su contenido 
    i = longDir;

   do {
      for (j=0; j < longDir ; j++) {
         if (j == i) {
           attron(A_REVERSE);
         }
         mvprintw(5+j,5,res[j].nombre);
         attroff(A_REVERSE);
      }
      move(5+i,5);
      refresh();
      c = leeChar();
      
      switch(c) {
         case 0x1B5B41:
            i = (i>0) ? i - 1 : longDir;
            break;
         case 0x1B5B42:
            i = (i<longDir) ? i + 1 : 0;
            break;
         case 10:
            system("clear");
            /*if(strcmp(cwd,cwdLogin) != 0){ //Si te encuentras en el dir login no hagas nada, si no, muévete al dir login*/
            if(strcmp(".", res[i].nombre) == 0){ //Si es un directorio "." regresa al directorio de login 
                strcpy(cwd, cwdLogin);
                longDir = leerDirectorio(cwd);
            } else if(strcmp(".." res[i.nombre]) == 0){ //Si es un directorio ".." regresa a un directorio superior
                
            } else { 
                strcat(cwd, "/");
                strcat(cwd,res[i].nombre); //Otener el directorio al que se quiere mover
                longDir = leerDirectorio(cwd); //Obtener la longitud del directorio y su contenido 
            }
            break;
         default:
            // Nothing 
            break;
      }
      move(2,10);
      printw("Selección actual: %d  Directorio actual: %s Leí %d", i, cwd, c);
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