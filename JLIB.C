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


typedef struct {
unsigned char *location;
unsigned short x_size;
unsigned short y_size;
} bitmap;

bitmap screen; /* the screen is merely a bitmap of size 320x200 */
bitmap alpha;

char *usedinblit;
int blitgap;
char *gurutable;

#define waypoint(a,b,c,d) (a)+(((b)-(a))*(d)/(c))

void sincostable(int *array,
                 int size,
                 int elements,
                 int type)

     {
     unsigned int count;
     float rot_value;
     rot_value=pi;

     for (count=0;count<elements;count++)
       {
       if (type==SIN)
         array[count]=sin(rot_value)*size;

       if (type==COS)
         array[count]=cos(rot_value)*size;

       rot_value+=(fullcircle/elements);
       }
    }

void clearscreen(bitmap *pic)

  {
  memset(pic->location,0,pic->x_size*pic->y_size);
  }

void newbitmap(bitmap *pic,unsigned short x_size,unsigned short y_size)
  {
  pic->location=(char *) malloc(x_size*y_size);
  pic->x_size=x_size;
  pic->y_size=y_size;
  }

void opengraphics(void)
  {
  union REGS TempRegs;
  newbitmap(&screen,320,200);
  clearscreen(&screen);

#ifdef __DJGPP__
  TempRegs.x.ax=0x13;
  _int86(0x10,&TempRegs,&TempRegs);
#endif
#ifdef __WATCOMC__
  TempRegs.w.ax=0x13;
  int386(0x10,&TempRegs,&TempRegs);
#endif

  }

void closegraphics(void)
  {
  union REGS TempRegs;

  printf("internal graphics library/");

#ifdef __DJGPP__
  TempRegs.x.ax=0x03;
  _int86(0x10,&TempRegs,&TempRegs);
  printf("djgpp");
#endif

#ifdef __WATCOMC__
  TempRegs.w.ax=0x03;
  int386(0x10,&TempRegs,&TempRegs);
  printf("watcom");  
#endif

  printf(" v1.0b\n");
  printf("compiled %s, %s\n",__TIME__,__DATE__);
  }

long filesize(FILE *stream)
   {
   long curpos, length;

   curpos = ftell(stream);
   fseek(stream, 0L, SEEK_END);
   length = ftell(stream);
   fseek(stream, curpos, SEEK_SET);
   return length;
   }

void setcol(unsigned short col, unsigned char r,unsigned char g,unsigned char b)
  {
  outportb(0x3c8,col);
  outportb(0x3c9,r);
  outportb(0x3c9,g);
  outportb(0x3c9,b);
  }

int unpackedsize(char *filename)

  {
  FILE *pcxfile;
  unsigned short xmin,ymin,xmax,ymax,xs,ys;

  pcxfile=fopen(filename, "rb");

  fseek(pcxfile,4,0);
  fread(&xmin,2,1,pcxfile);
  fread(&ymin,2,1,pcxfile);
  fread(&xmax,2,1,pcxfile);
  fread(&ymax,2,1,pcxfile);

  fclose(pcxfile);

  xs=(xmax-xmin)+1;
  ys=(ymax-ymin)+1;

  return(xs*ys);
  }

void loadpcx(char *filename,
             bitmap *data,
             char *pal_location)

  {
  FILE *pcxfile;
  unsigned char type;
  unsigned char col;
  unsigned char pixels;
  unsigned char *buffer;

  unsigned char *input;
  unsigned char *endpos;
  int datasize;
  unsigned short xmin,ymin,xmax,ymax;
  char *output;

  pcxfile=fopen(filename, "rb");
  datasize=filesize(pcxfile)-(128+768);
  buffer = (char *) malloc(datasize);

  fseek(pcxfile,4,0);
  fread(&xmin,2,1,pcxfile);
  fread(&ymin,2,1,pcxfile);
  fread(&xmax,2,1,pcxfile);
  fread(&ymax,2,1,pcxfile);

  data->x_size=(xmax-xmin)+1;
  data->y_size=(ymax-ymin)+1;

  fseek(pcxfile,128,0);
  fread(buffer,datasize,1,pcxfile);
  endpos=(buffer+datasize);

  if (pal_location!=NULL)
    {
    fseek(pcxfile,(filesize(pcxfile)-768),0);
    fread(pal_location,768,1,pcxfile);
    }

  fclose(pcxfile);

  input=buffer;
  output = data->location;
   
  type=*input++;
  while (input<=endpos)
    {
    if (type>191)
      {
      col=*input++;
      for (pixels=type & 63;pixels--;pixels)
        *output++=col;
      }
    else
      *output++=type;
    type=*input++;
    }

  free(buffer);
  }

void create_bitmap(bitmap *source_pic,
                   bitmap *dest_pic,  
                   int x1,
                   int y1,
                   int x2,
                   int y2)
  {
  unsigned short dest_width;
  unsigned short dest_height;
  unsigned char *output;
  unsigned char *input;
  int yindex,xindex;

  dest_width=(x2-x1)+1;
  dest_height=(y2-y1)+1;

  dest_pic->location=(char *) malloc(dest_width*dest_height);
  dest_pic->x_size=dest_width;
  dest_pic->y_size=dest_height;

  output=dest_pic->location;
  input=source_pic->location+x1+(y1*source_pic->x_size);

  for (yindex=dest_height;yindex>0;yindex--)
    {
    for (xindex=dest_width;xindex>0;xindex--)
      {
      *output=*input;
      output++;
      input++;
      }
    input+=(source_pic->x_size-dest_width);
    }
  }

void screen_copy(bitmap *src, bitmap *dest, int sx1, int sy1, int sx2, int sy2, int dx, int dy)
  {
  char *sp;
  char *dp;
  int yi,xs;

  xs = (sx2-sx1)+1;
  yi = (sy2-sy1)+1;
  sp = src->location+(src->x_size*sy1)+sx1;
  dp = dest->location+(dest->x_size*dy)+dx;

  for (; yi>0; yi--)
    {
    memcpy(dp,sp,xs);
    sp+=src->x_size;
    dp+=dest->x_size;
    }
  }


void copyscreen(bitmap *source_pic,
                bitmap *dest_pic)

  {
  if ((source_pic->x_size==dest_pic->x_size) &&
      (source_pic->y_size==dest_pic->y_size))
        memcpy(dest_pic->location,source_pic->location,source_pic->x_size*source_pic->y_size);
  }

void showpal(char *palette)

  {
  unsigned short col;
  char *pos;

  pos=palette;

  for (col=0; col<256; col++)
    {
    setcol(col,*pos>>2,*(pos+1)>>2,*(pos+2)>>2);
    pos+=3;
    }
  }

void loadfont(char *filename)

  {
  alpha.location=(char *) malloc(32768);
  loadpcx(filename,&alpha,NULL);
  }

void putpixel(unsigned int x, unsigned int y, char col)
   {
   if ((x<=319) && (y<=199))
     screen.location[(y*320)+x]=col;
   }

void write(int x,int y,unsigned char *string)

  {
  unsigned char *start;    /* Start pos of each letter on screen */
  unsigned char *current;  /* screen pointer used while drawing */
  unsigned char *src;      /* the letter bitmap data */
  unsigned char xd,yd;     /* also used while drawing */

  start=screen.location+(y*320)+x;

  while (*string!=0)
    {
    current=start;
    src=(alpha.location+(*string<<3));
    for (yd=0;yd<8;yd++)
      {
      for (xd=0;xd<8;xd++)
        {
        if (*src!=0)
          *current=*src;
        src++;
        current++;
        }
      src+=(2048-8);
      current+=(320-8);
      }
    start+=8;
    string++;
    }
  }

#ifdef __DJGPP__

void bar(int x1,
         int y1,
         int x2,
         int y2,
         unsigned char col)

  {
  int temp;

  if (x1>x2)
    {
    temp=x1;
    x1=x2;
    x2=temp;
    }

  if (y1>y2)
    {
    temp=y1;
    y1=y2;
    y2=temp;
    }

  if (x1<0)
    x1=0;
  if (y1<0)
    y1=0;
  if (x2<0)
    x2=0;
  if (y2<0)
    y2=0;

  if (x1>319)
    x1=319;
  if (y1>319)
    y1=319;
  if (x2>319)
    x2=319;
  if (y2>319)
    y2=319;

  asm volatile("
                line:              \n\t
                movl  %%esi,%%ecx  \n\t
                rep                \n\t
                stosb              \n\t
                addl  %%ebx,%%edi  \n\t
                decl  %%edx        \n\t
                jnz   line         \n\t
               " :
                 : "a" (col), "S" ((x2-x1)+1), "D" (screen.location+x1+(y1*320)),
                   "b" (319-(x2-x1)), "d" (y2-y1+1)
                 : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi" );

  }

#endif


void gurupoly(int x1,int y1,int x2,int y2,int x3,int y3, unsigned char col,char shd1,char shd2,char shd3)

  {
  int temp;
  unsigned char tempb;
  int leftside,rightside;
  int leftmove,rightmove;
  int shadex,shadexmove;
  int shadey,shadeymove;
  int x4;
  char shd4;
  int segheight;
  char *segend;
  char *linest;
  char *dest;
  char *fin;
  unsigned char *thiscol;

  if (y1>y2)
    {
    temp=y1;
    y1=y2;
    y2=temp;

    temp=x1;
    x1=x2;
    x2=temp;

    tempb=shd1;
    shd1=shd2;
    shd2=tempb;
    }

  if (y2>y3)
    {
    temp=y2;
    y2=y3;
    y3=temp;

    temp=x2;
    x2=x3;
    x3=temp;

    tempb=shd2;
    shd2=shd3;
    shd3=tempb;
    }

  if (y1>y2)
    {
    temp=y1;
    y1=y2;
    y2=temp;

    temp=x1;
    x1=x2;
    x2=temp;

    tempb=shd1;
    shd1=shd2;
    shd2=tempb;
    }


  thiscol=gurutable+(col*64);

  rightside=leftside=(x1<<16);
  x4=x1+((x3-x1)+1)*((y2-y1)+1)/((y3-y1)+1);
  shd4=shd1+((shd3-shd1)+1)*((y2-y1)+1)/((y3-y1)+1);

  if (x2>x4)
    {
    temp=x4;
    x4=x2;
    x2=temp;
    tempb=shd4;
    shd4=shd2;
    shd2=tempb;
    }

  shadexmove=((shd4-shd2)<<8)/((x4-x2)+1);

  /* first section of poly */

  if (y1!=y2)
    {
    shadeymove=((shd2-shd1)<<8)/(y2-y1+1);
    shadey=(shd1<<8);

    segheight=(y2-y1+1);
    leftmove=((x2-x1)<<16)/segheight;
    rightmove=((x4-x1)<<16)/segheight;

    linest=screen.location+(y1*320);
    segend=screen.location+(y2*320);

    while(linest<=segend)
      {
      dest=linest+(leftside>>16);
      fin=linest+(rightside>>16);
      shadex=shadey;

      while (dest<=fin)
        {
        *dest=thiscol[shadex>>8];
        dest++;
        shadex+=shadexmove;
        }

      leftside+=leftmove;
      rightside+=rightmove;
      linest+=320;
      shadey+=shadeymove;
      }
    }

  /* second section of poly */

  if (y2!=y3)
    {
    leftside=(x2<<16);
    rightside=(x4<<16);
    shadeymove=((shd3-shd2)<<8)/(y3-y2+1);
    shadey=(shd2<<8);

    segheight=(y3-y2+1);
    leftmove=((x3-x2)<<16)/segheight;
    rightmove=((x3-x4)<<16)/segheight;

    linest=screen.location+(y2*320);
    segend=screen.location+(y3*320);

    while(linest<=segend)
      {
      dest=linest+(leftside>>16);
      fin=linest+(rightside>>16);
      shadex=shadey;

      while (dest<=fin)
        {
        *dest=thiscol[shadex>>8];
        dest++;
        shadex+=shadexmove;
        }

      leftside+=leftmove;
      rightside+=rightmove;
      linest+=320;
      shadey+=shadeymove;
      }
    } 
  }


void colourmix_table(char *table, char *palette)

  {
  unsigned char searchred,searchgreen,searchblue;
  int abs_dif,smallest_dif;

  unsigned char col1,col2,current_col,smallest_dif_col;
  unsigned char *destination_offset;
  unsigned char *col1ptr;
  unsigned char *col2ptr;
  unsigned char *palptr;

  destination_offset=table;

  col1ptr=palette;
  col1=0;
  do
    {
    col2=0;
    col2ptr=palette;
    do
      {
      searchred=*col2ptr+((*col1ptr-*col2ptr)>>1);
      searchgreen=*(col2ptr+1)+((*(col1ptr+1)-*(col2ptr+1))>>1); 
      searchblue=*(col2ptr+2)+((*(col1ptr+2)-*(col2ptr+2))>>1);
      col2ptr+=3;

      smallest_dif=768;
      palptr=palette;
      current_col=0;
      do
        {
        abs_dif=abs(* palptr   -searchred)+
                abs(*(palptr+1)-searchgreen)+
                abs(*(palptr+2)-searchblue);
        palptr+=3;

        if (abs_dif<smallest_dif) 
          {
          smallest_dif=abs_dif;
          smallest_dif_col=current_col;
          }

        current_col++;
        } while (current_col!=0);
      *destination_offset=smallest_dif_col;
      destination_offset++;
      col2++;
      } while (col2!=0);
    col1ptr+=3;
    col1++;
    } while (col1!=0);
  }

void shade_table(char *table, char *palette,int dis_red,int dis_green,int dis_blue)

  {
  unsigned char searchred,searchgreen,searchblue;
  int abs_dif,smallest_dif;

  int shade,col,current_col,smallest_dif_col;
  unsigned char *destination_offset;
  unsigned char *colptr;
  unsigned char *palptr;

  destination_offset=table;

  colptr=palette;
  for (col=0; col<256; col++)
    {
    for (shade=0; shade<64; shade++)
      {
      searchred=waypoint(*colptr,dis_red,64,shade);
      searchgreen=waypoint(*(colptr+1),dis_green,64,shade);
      searchblue=waypoint(*(colptr+2),dis_blue,64,shade);

      smallest_dif=768;
      palptr=palette;

      for (current_col=0; current_col<256; current_col++)
        {
        abs_dif=abs(* palptr   -searchred)+
                abs(*(palptr+1)-searchgreen)+
                abs(*(palptr+2)-searchblue);
        palptr+=3;

        if (abs_dif<smallest_dif) 
          {
          smallest_dif=abs_dif;
          smallest_dif_col=current_col;
          }
        } 

      *destination_offset=smallest_dif_col;
      destination_offset++;
      }
    colptr+=3;
    }
  }

#ifdef __DJGPP__

char rtblit(int x,int y,bitmap *sprite)

  {
  int xs;
  int ys;
  char *loc;

  xs=sprite->x_size;
  ys=sprite->y_size;
  loc=sprite->location;

  if ((x+xs>0) && (y+ys>0) && (x<320) && (y<200))
    {

    blitgap=0;

    if ((y+ys)>199) 
      ys=(200-y);
   
    if (y<0)       
      {
      loc-=(xs*y);
      ys+=y;
      y=0;
      }

    if ((x+xs)>319)
      {
      xs=(320-x);
      blitgap=(sprite->x_size-xs);
      }

    if (x<0)
      {
      blitgap=-x;
      loc-=x;
      xs+=x;
      x=0;
      }

    asm volatile("
   
    pushl %%ebp              \n\t
    mov %%ecx,%%ebp          \n\t
    
    nextline2:               \n\t
    
    mov %%ebp,%%ecx          \n\t
    nextbyte2:               \n\t
    movb (%%esi),%%al        \n\t

    cmpb $0,%%al             \n\t
    je   next2               \n\t
    movb %%al,(%%edi)        \n\t
    next2:                   \n\t

    incl %%edi               \n\t
    incl %%esi               \n\t
    decl %%ecx               \n\t
    jnz nextbyte2            \n\t

    addl _blitgap,%%esi      \n\t
    addl %%edx,%%edi         \n\t
    decl %%ebx               \n\t
    jnz nextline2            \n\t

    popl %%ebp
    "
       :
       : "S" (loc),
         "D" (screen.location+x+(y*320)),
         "a" (blitgap),
         "c" (xs),
         "d" (320-xs),
         "b" (ys)
       : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi" );

      return(1);
      }
    return(0);
    }


#endif
#undef SIN
#undef COS
