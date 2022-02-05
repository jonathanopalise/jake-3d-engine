/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป*/
/*บ                            jlib32                                     บ*/
/*บ graphics library for djgpp/watcom                                     บ*/
/*บ                                                                       บ*/
/*บ                                 copyright (c) Jonathan Thomas 1996/97 บ*/
/*ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/

#include<stdio.h>
#include<math.h>
#include<dos.h>
#include<string.h>

#ifdef __DJGPP__
#include<go32.h>
#define copyscreenbuffer(); _dosmemputl(screen.location,16000,0xA0000);
#endif

#ifdef __WATCOMC__
#include<i86.h>
#include<stdlib.h>
#include<conio.h>
#define copyscreenbuffer(); memcpy(0xA0000,screen.location,64000);
#define outportb outp
#endif

#define pi 3.1415927
#define fullcircle 3.1415927*2
#define SIN 0
#define COS 1

#define waypoint(a,b,c,d) (a)+(((b)-(a))/(c)*(d))

typedef struct {
unsigned char *location;
unsigned short x_size;
unsigned short y_size;
} bitmap;

bitmap screen; /* the screen is merely a bitmap of size 320x200 */
bitmap alpha;

void sincostable(int *array,int size,int elements,int type);
void clearscreen(bitmap *pic);
void opengraphics(void);
void closegraphics(void);
long filesize(FILE *stream);
void setcol(unsigned short Col, unsigned char R,unsigned char G,unsigned char B);
int  unpackedsize(char *filename);
void loadpcx(char *filename,bitmap *data, char *pal_location);
void create_bitmap(bitmap *source_pic,bitmap *dest_pic,int x1,int y1,int x2,int y2);
void screen_copy(bitmap *src, bitmap *dest, int sx1, int sy1, int sx2, int sy2, int dx, int dy);
void copyscreen(bitmap *source_pic,bitmap *dest_pic);
void showpal(char *palette);
void loadfont(char *filename);
void putpixel(unsigned int x, unsigned int y, char col);
void write(int x,int y,unsigned char *string);
void bar(int x1,int y1,int x2,int y2,unsigned char col);
void gurupoly(int x1,int y1,int x2,int y2,int x3,int y3, unsigned char col,char shd1,char shd2,char shd3);
void colourmix_table(char *table, char *palette);
void shade_table(char *table, char *palette,char dis_red,char dis_green,char dis_blue);
char rtblit(int x,int y,bitmap *sprite);
