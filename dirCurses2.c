#include <stdio.h>
#include <dirent.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <ctype.h>

int fd;
int fs;
int lineas;
int countModif = 0;

char *hazLinea(char *base, unsigned long long dir) {
	// mvprintw(27,0,"%d", lineas);
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
    /* Abre archivo */
    fd = open(filePath, O_RDWR);
    if (fd == -1) {
    	perror("Error abriendo el archivo");
	    return(NULL);
    }

    /* Mapea archivo */
    struct stat st;
    fstat(fd,&st);
    fs = st.st_size;
		lineas = fs / 16;

    char *map = mmap(NULL, fs, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
    	close(fd);
	    perror("Error mapeando el archivo");
	    return(NULL);
    }
		// close(fd);

  return map;
}

int main() {
   struct s_dir {
	   int tipo;
	   char *nombre;
   } res[128];
   struct s_dir temp;
   struct s_dir cur;

   int highlight = 1;
   int choice = 0;
   int c;

   long size;
   char *buf;
   char *ptr;
	 char *ptr2;

	 long l;

	 int maxPantalla = 10;
	 int offset = 0;

   size = pathconf(".", _PC_PATH_MAX);

   initscr();
 	 clear();
 	 noecho();
   cbreak();
   keypad(stdscr, TRUE);

   DIR *dir = opendir(".");
   if ((buf = (char *)malloc((size_t)size)) != NULL)
       ptr = getcwd(buf, (size_t)size);
	 if ((buf = (char *)malloc((size_t)size)) != NULL)
 			ptr2 = getcwd(buf, (size_t)size);
   struct dirent *dp;
   int i=0;
   while((dp=readdir(dir)) != NULL) {
      res[i].tipo = dp->d_type;
      res[i].nombre=dp->d_name;
      i++;
   }
   int cant = i;
   for(int j=0;j<maxPantalla;j++) {
		 if(j == i) break;
     refresh();
      for(int k=j+1;k<i;k++){
         if(strcmp(res[j].nombre,res[k].nombre)>0){
            temp= res[j];
            res[j]=res[k];
            res[k]=temp;
         }
      }
    }
		int count = 0;
   for(int j=offset; j<maxPantalla+offset; j++) {
		 if(j >= i) break;
     if(highlight == count + 1) {
       attron(A_REVERSE);
       mvprintw(3+count, 0, "%c %s", res[j].tipo == DT_DIR ? 'D' :'F',res[j].nombre);
			 clrtoeol();
       attroff(A_REVERSE);
     } else {
       mvprintw(3+count, 0, "%c %s", res[j].tipo == DT_DIR ? 'D' :'F',res[j].nombre);
			 clrtoeol();
     }
     mvprintw(1, 5, "Estoy en %d: Lei %s",highlight,res[highlight-1].nombre);
     clrtoeol();
     refresh();
		 count++;
   }
   do {
     c = getch();
     switch(c)
 		{	case KEY_UP:
 				if(highlight == 1){ //Al inicio de la lista?
           if (offset == 0){
             if (cant > maxPantalla) { //Mas elementos que el max?
             highlight = maxPantalla;
             offset = cant - maxPantalla;
             } else { // Dentro del max
               highlight = cant;
             }
           } else {
             --offset;
           }
        }
 				else
 					--highlight;
 				break;
 			case KEY_DOWN:
 				if((highlight+offset) == i){
 					highlight = 1;
					offset = 0;
 				}else {
					if(highlight == maxPantalla) {
						offset++;
					} else {
							++highlight;
					}
				}
 				break;
 			case 10:
 				cur = res[highlight+offset-1];
        if(cur.tipo == DT_DIR) {
          if(strcmp(cur.nombre, "..") == 0) {
            char *aux = strrchr(ptr, '/'); //Encuetra el último caracter "/"
            if(aux != ptr) {
              *aux = '\0';
            } else {
              *(aux+1) = '\0';
            }
              // char *ptr2;
              // // printw("%s", ptr);
              // int slashCount = 0;
              // int slashCount2 = 0;
              // for(int i = 0; i < strlen(ptr); i++) if(ptr[i] == '/') slashCount++;
              // // if(slashCount==1) {
              // //   sprintf(ptr, "/");
              // //   break;
              // // }
              // // printw("%d", slashCount);
              // for(int i = 0; i < strlen(ptr); i++) {
              //   if(ptr[i] == '/') slashCount2++;
              //   if(slashCount2 == slashCount) {
              //     ptr[i] = slashCount != 1 ? '\0' : '/';
              //     break;
              //   }
              // }
            // printw("\t%s", ptr);
          }  else {
            sprintf(ptr, "%s/%s", ptr, cur.nombre);
          }
          printw("%s", ptr);
          closedir(dir);
          dir = opendir(ptr);
          i = 0;
          highlight = 1;
        } else {
          /* Lee archivo */
          clear();
          refresh();
          sprintf(ptr2, "%s/%s", ptr, cur.nombre);
          char *map = mapFile(ptr2);
					countModif=0;
          if (map == NULL) {
            exit(EXIT_FAILURE);
            }

      		int c2;
      		int x = 0;
      		int y = 0;
      		int px = 0;
					int offsetArch = 0;
      		do {
						for(int i= 0; i<25; i++) {
							// Haz linea, base y offset
							char *l = hazLinea(map,(unsigned long long)(i+offsetArch)*16);
							mvprintw(i,0,l);
						}
						mvprintw(27,0,"X: %d Y: %d FS: %d Mod: %d", x, y+offsetArch, fs, countModif*sizeof(char));
						refresh();
      			px = (x<16) ? x*3 : 32+x;
      			move(0+y, 9+px);
      			c2 = getch();
      			switch(c2) {
      				case KEY_UP:
								if(y == 0) {
									if(offsetArch != 0) {
										offsetArch--;
									}
								} else {
									y-=1;
								}
      					break;
      				case KEY_DOWN:
								// mvprintw(27,0,"Y: %d, O: %d, Lin: %d", y, offsetArch, lineas);
								if(y == 24) {
									if((y+offsetArch) >= lineas) {
										// mvprintw(27,0,"EOF");
									} else {
										offsetArch++;
									}
								} else {
									y+=1;
								}
      					break;
      				case KEY_LEFT:
      					if(x > 0) {
                  x-=1;
                }
      					break;
      				case KEY_RIGHT:
      					if (x < 31) {
                  x+=1;
                } else {
                  x = 0; y += 1;
                }
      					break;
              case KEY_DC:
                memcpy(&map[(offsetArch+y)*16+x], &map[(offsetArch+y)*16+x+1],fs-(offsetArch+y)*16+x);
                break;
							default:
								if(c2 == 24) break;
								if(x >= 0 && x <= 15) {
									// char c3 = getch();
									char n3 = tolower(c2);
									if((n3 >= '0' && n3 <= '9') || (n3 >= 'a' && n3 <= 'f')) {
										char c4 = getch();
										char n4 = tolower(c4);
										if((n4 >= '0' && n4 <= '9') || (n4 >= 'a' && n4 <= 'f')) {
											char hd[3];
											hd[0] = n3;
											hd[1] = n4;
											hd[2] = 0;
											l = strtol(hd, NULL, 16);
											map[(offsetArch+y)*16+x] = l;
											countModif++;
											for(int i= 0; i<25; i++) {
												// Haz linea, base y offset
												char *lin = hazLinea(map,(unsigned long long)(i+offsetArch)*16);
												mvprintw(i,0,lin);
											}
											// mvprintw(27,0,"X: %d Y: %d", x, y+offsetArch);
											refresh();
										} else {
											move(28,10);
											addstr("Caracter inválido");
										}
									} else {
										move(28,10);
										addstr("Caracter inválido");
									}
								} else {
									mvprintw(28,0,"%c", c2);
									// char c3 = getch();
									// map[(offsetArch+y)*16+x] = c2;
									map[(offsetArch+y-1)*16+x] = c2;
									// if(isprint(c2))
										// sprintf(&map[(offsetArch + y-1) * 16 + x], "%c", c2);
									// } else {
										// sprintf(&map[(offsetArch + y-1) * 16 + x], "%c", '.');
									// }
									countModif++;
								}
								break;
							}
      		} while(c2!=24);
          // leeChar();
          // endwin();

          if (munmap(map, fs+(countModif*sizeof(char))) == -1) {
            perror("Error al desmapear");
						exit(1);
          }
          close(fd);
        }
				highlight = 1;
				offset = 0;
				c = 0;
        clear();
        refresh();
 				break;
 		}
    while((dp=readdir(dir)) != NULL) {
       res[i].tipo = dp->d_type;
       res[i].nombre=dp->d_name;
       i++;
       cant = i;
    }
    for(int j=0;j<i;j++) {
       for(int k=j+1;k<i;k++){
          if(strcmp(res[j].nombre,res[k].nombre)>0){
             temp= res[j];
             res[j]=res[k];
             res[k]=temp;
          }
       }
     }
		 count = 0;
		 for(int j=offset; j<maxPantalla+offset; j++) {
  		 if(j >= i) break;
       if(highlight == count + 1) {
         attron(A_REVERSE);
         mvprintw(3+count, 0, "%c %s", res[j].tipo == DT_DIR ? 'D' :'F',res[j].nombre);
				 clrtoeol();
         attroff(A_REVERSE);
       } else {
         mvprintw(3+count, 0, "%c %s", res[j].tipo == DT_DIR ? 'D' :'F',res[j].nombre);
				 clrtoeol();
       }
       mvprintw(1, 5, "Estoy en %d: Off: %d Max: %d Lei %s ",highlight, offset, cant, res[highlight-1].nombre);
       clrtoeol();
       refresh();
  		 count++;
     }
  } while(c != 'q');
  // clrtoeol();
  endwin();
   closedir(dir);
   return 0;
}
