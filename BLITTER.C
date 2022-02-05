#include<stdio.h>
#include<math.h>
#define bl_waypoint(a,b,c,d) (a)+(((b)-(a))*(d)/(c))

int screentop;
int screenbot;
int screenleft;
int screenright;

char *pal;
char *guru_table;
char *texshade_table;
char *filter_table;
char *texture;
char *output;

int shade_red,shade_green,shade_blue;

void set_texture(char *texture_ptr)
  {
  texture=texture_ptr;
  }

void set_environment(char *palette, int red, int green, int blue)
  {
  shade_red=red;
  shade_blue=blue;
  shade_green=green;
  pal=palette;
  }

void set_screen(char *location,int left,int top,int right,int bottom)
  {
  output=location;
  screenleft=left;
  screentop=top;
  screenright=right;
  screenbot=bottom;
  }

void generate_filter()
  {
  int x,y;
  float xdis,ydis;
  int dis1,dis2,dis3;
  char pixel;
  char inpix,outpixtr,outpixbl,outpixbr;

  filter_table=(char *) malloc(65536);

  for (y=0; y<128; y++)
    for (x=0; x<128; x++)
      {
      ydis=128-y;
      xdis=128-x;
      dis1=sqrt((xdis*xdis)+(ydis*ydis));

      if ((rand()%320)>dis1)
        pixel=0;
        else
        {
        xdis=0-x;
        ydis=128-y;
        dis3=sqrt((xdis*xdis)+(ydis*ydis))/2;

        xdis=128-x;
        ydis=0-y;
        dis2=sqrt((xdis*xdis)+(ydis*ydis))/2;

        if ((rand()%(dis2+dis3))>dis2)
          pixel=3;
          else
          pixel=1;
        }

      filter_table[y*256+x]=pixel;
      }

  for (y=0; y<128; y++)
    for (x=0; x<128; x++)
      {
      inpix=filter_table[y*256+x];
      switch(inpix)
        {
        case 3: outpixtr=3; outpixbl=4; outpixbr=4; break;
        case 1: outpixtr=2; outpixbl=1; outpixbr=2; break;
        case 0: outpixtr=0; outpixbl=0; outpixbr=0; break;
        }
      filter_table[y*256+(255-x)]=outpixtr;
      filter_table[((255-y)*256)+x]=outpixbl;
      filter_table[((255-y)*256)+(255-x)]=outpixbr;
      }
  }

void generate_shade_table()

  {
  unsigned char searchred,searchgreen,searchblue;
  int abs_dif,smallest_dif;

  int shade,col,current_col,smallest_dif_col;
  unsigned char *destination_offset;
  unsigned char *colptr;
  unsigned char *palptr;

  texshade_table=(char *) malloc(16384);
  destination_offset=texshade_table;

  for (shade=0; shade<64; shade++)
    {
    colptr=pal;
    for (col=0; col<256; col++)
      {
      searchred=bl_waypoint(*colptr,shade_red,64,shade);
      searchgreen=bl_waypoint(*(colptr+1),shade_green,64,shade);
      searchblue=bl_waypoint(*(colptr+2),shade_blue,64,shade);

      smallest_dif=768;
      palptr=pal;

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
      colptr+=3;
      }

    }
  }

void poly(int x1,int y1,int x2,int y2,int x3,int y3, unsigned char col)

  {
  int writes;
  int temp;
  int leftside,rightside;
  int truels,truers;
  int leftmove,rightmove;
  int x4;
  int segheight;
  unsigned int dcol;
  char *segend;
  char *linest;
  char *dest;
  char *fin;
  int onright;
  int onleft;

  onright=0;
  onleft=0;

  if (x1>319) onright++;
  if (x2>319) onright++;
  if (x3>319) onright++;

  if (x1<0) onleft++;
  if (x2<0) onleft++;
  if (x3<0) onleft++;

  if ((onright<3) && (onleft<3))
  {

  if (y1>=y2)
    {
    temp=y1;
    y1=y2;
    y2=temp;

    temp=x1;
    x1=x2;
    x2=temp;
    }

  if (y2>=y3)
    {
    temp=y2;
    y2=y3;
    y3=temp;

    temp=x2;
    x2=x3;
    x3=temp;
    }

  if (y1>=y2)
    {
    temp=y1;
    y1=y2;
    y2=temp;

    temp=x1;
    x1=x2;
    x2=temp;
    }

  dcol=0x01010101*col;
  rightside=leftside=(x1<<16);
  x4=x1+((x3-x1)+1)*((y2-y1)+1)/((y3-y1)+1);

  if (x2>x4)
    {
    temp=x4;
    x4=x2;
    x2=temp;
    }
  
  /* first section of poly */

  if ((y1<=screenbot) && (y2>=screentop))
    {
    segheight=(y2-y1)+1;
    leftmove=((x2-x1)<<16)/segheight;
    rightmove=((x4-x1)<<16)/segheight;

    if (y2>screenbot)
      {
      y2=screenbot;
      segheight=((y2-y1)+1);
      }

    if (y1<screentop)
      {
      leftside-=(leftmove*(y1-screentop));
      rightside-=(rightmove*(y1-screentop));
      y1=screentop;
      segheight=((y2-y1)+1);
      }

    linest=output+(y1*320);
    segend=output+(y2*320);

    while(linest<=segend)   
      {
      truels=leftside>>16;
      truers=rightside>>16;
 
      if ((truels<screenright) && (truers>screenleft))
        {
        if (truers>screenright)
          truers=screenright;

        if (truels<screenleft)
          truels=screenleft;

        dest=linest+truels;

        if ((truers-truels)>16)
          {
          fin=linest+(truels&0xFFFFFFFC)+4;
          while(dest<fin) 
            {
            *(char *)dest=dcol;
            dest++;
            } 
 
          writes=((linest+truers)-dest)>>2;
          for (;writes!=0; writes--)
            {
            *(int *)dest=dcol;
            dest+=4;
            } 

          }
   
        fin=linest+truers;
        while(dest<=fin)
          {
          *(char *)dest=dcol;
          dest++;
          }
        }

      leftside+=leftmove;
      rightside+=rightmove;
      linest+=320;
      }
    }
                   
  /* second section of poly */

  leftside=x2<<16;
  rightside=x4<<16;

  if ((y2<screenbot) && (y3>screentop))
    {
    segheight=(y3-y2)+1;
    leftmove=((x3-x2)<<16)/segheight;
    rightmove=((x3-x4)<<16)/segheight;

    if (y3>screenbot)
      {
      y3=screenbot;
      segheight=(y3-y2)+1;
      }

    if (y2<screentop)
      {
      leftside-=(leftmove*(y2-screentop));
      rightside-=(rightmove*(y2-screentop));
      y2=screentop;
      segheight=(y3-y2)+1;
      }

    linest=output+(y2*320);
    segend=output+(y3*320);

    while(linest<=segend)
      {
      truels=leftside>>16;
      truers=rightside>>16;

      if ((truels<=screenright) && (truers>=screenleft))
        {
        if (truers>screenright)
          truers=screenright;

        if (truels<screenleft)
          truels=screenleft;

        dest=linest+truels;

        if ((truers-truels)>16)
          {
          fin=linest+(truels&0xFFFFFFFC)+4;
          while(dest<fin) 
            {
            *(char *)dest=dcol;
            dest++;
            } 
   
          writes=((linest+truers)-dest)>>2;
          for (;writes!=0; writes--)
            {
            *(int *)dest=dcol;
            dest+=4;
            } 
          }

        fin=linest+truers;
        while(dest<=fin)
          {
          *(char *)dest=dcol;
          dest++;
          }
        }

      leftside+=leftmove;
      rightside+=rightmove;
      linest+=320;
      }

    }

  }
  }

void polyfst(int x1,int y1,int x2,int y2,int x3,int y3,unsigned char tx1,
                                                         unsigned char ty1,
                                                         unsigned char tx2,
                                                         unsigned char ty2,
                                                         unsigned char tx3,
                                                         unsigned char ty3,int b)

  {
  int texx,texy,texxmov,texymov; /* movement on x axis */
  int ctx,cty,ctxmov,ctymov;     /* movement on y axis */
  int cliplength;

  int temp;
  int leftside,rightside;
  int truels,truers;
  int leftmove,rightmove;
  int x4,y4,tx4,ty4;
  int segheight;
  char *segend;
  char *linest;
  char *dest;
  char *fin;

  int onright;
  int onleft;
  char psel;

  char *shady;

  shady=&texshade_table[b*256];

  onright=0;
  onleft=0;

  if (x1>319) onright++;
  if (x2>319) onright++;
  if (x3>319) onright++;

  if (x1<0) onleft++;
  if (x2<0) onleft++;
  if (x3<0) onleft++;

  if ((onright<3) && (onleft<3))
  {

  if (y1>y2)
    {
    temp=y1;
    y1=y2;
    y2=temp;

    temp=x1;
    x1=x2;
    x2=temp;

    temp=tx1;
    tx1=tx2;
    tx2=temp;

    temp=ty1;
    ty1=ty2;
    ty2=temp;
    }

  if (y2>y3)
    {
    temp=y2;
    y2=y3;
    y3=temp;

    temp=x2;
    x2=x3;
    x3=temp;

    temp=tx3;
    tx3=tx2;
    tx2=temp;

    temp=ty3;
    ty3=ty2;
    ty2=temp;
    }

  if (y1>y2)
    {
    temp=y1;
    y1=y2;
    y2=temp;

    temp=x1;
    x1=x2;
    x2=temp;

    temp=tx1;
    tx1=tx2;
    tx2=temp;

    temp=ty1;
    ty1=ty2;
    ty2=temp;
    }

  rightside=leftside=(x1<<16);

  x4=x1+((x3-x1)+1)*((y2-y1)+1)/((y3-y1)+1);
  y4=y2;

  tx4=tx1+((tx3-tx1)*((y2-y1)+1)/((y3-y1)+1));
  ty4=ty1+((ty3-ty1)*((y2-y1)+1)/((y3-y1)+1));

  if (x2>x4)
    {
    temp=x4;
    x4=x2;
    x2=temp;

    temp=tx4;
    tx4=tx2;
    tx2=temp;

    temp=ty4;
    ty4=ty2;
    ty2=temp;
    }

  segheight=((y2-y1)+1);

  texxmov=((tx4-tx2)<<8)/((x4-x2)+1);
  texymov=((ty4-ty2)<<8)/((x4-x2)+1);

  /* first section of poly */

  leftmove=((x2-x1)<<16)/segheight;
  rightmove=((x4-x1)<<16)/segheight;
  ctxmov=((tx2-tx1)<<8)/segheight;
  ctymov=((ty2-ty1)<<8)/segheight;
  ctx=tx1<<8;
  cty=ty1<<8;

  if ((y1<screenbot) && (y2>screentop))
    {
    if (y2>screenbot)
      {
      y2=screenbot;
      segheight=((y2-y1)+1);
      }

    if (y1<screentop)
      {
      cliplength=y1-screentop;
      leftside-=leftmove*cliplength;
      rightside-=rightmove*cliplength;
      ctx-=ctxmov*cliplength;
      cty-=ctymov*cliplength;
      y1=screentop;
      segheight=((y2-y1)+1);
      }

    linest=output+(y1*320);
    segend=output+(y2*320);

    while(linest<=segend)   
      {
      truels=leftside>>16;
      truers=rightside>>16;
      texx=ctx;
      texy=cty;

      if ((truels<screenright) && (truers>screenleft))
        {
        if (truers>screenright)
          truers=screenright;

        if (truels<screenleft)
          {
          texx-=(texxmov*(truels-screenleft));
          texy-=(texymov*(truels-screenleft));
          truels=screenleft;
          }

        dest=linest+truels;

        fin=linest+truers;
        while(dest<=fin)
          {
          psel=filter_table[(texx&255)+((texy&255)*256)];
          switch(psel)
            {
            case 0: *dest=shady[texture[(texy&65280)+(texx>>8)]]; break;
            case 1: *dest=shady[texture[(texy&65280)+(texx>>8)-1]]; break;
            case 2: *dest=shady[texture[(texy&65280)+(texx>>8)+1]]; break;
            case 3: *dest=shady[texture[(texy&65280)+(texx>>8)-256]]; break;
            case 4: *dest=shady[texture[(texy&65280)+(texx>>8)+256]]; break;
            }
          texy+=texymov;
          texx+=texxmov;
          dest++;
          }

        }

      leftside+=leftmove;
      rightside+=rightmove;
      ctx+=ctxmov;
      cty+=ctymov;
      linest+=320;
      }
    }

  /* second section of poly */

  leftside=x2<<16;
  rightside=x4<<16;
  ctx=tx2<<8;
  cty=ty2<<8;

  segheight=((y3-y2)+1);
  leftmove=((x3-x2)<<16)/segheight;
  rightmove=((x3-x4)<<16)/segheight;
  ctxmov=((tx3-tx2)<<8)/segheight;
  ctymov=((ty3-ty2)<<8)/segheight;

  if ((y2<screenbot) && (y3>screentop))
    {
    if (y3>screenbot)
      {
      y3=screenbot;
      segheight=((y3-y2)+1);
      }

    if (y2<screentop)
      {
      cliplength=y2-screentop;
      leftside-=leftmove*cliplength;
      rightside-=rightmove*cliplength;
      ctx-=ctxmov*cliplength;
      cty-=ctymov*cliplength;
      y2=screentop;
      segheight=((y3-y2)+1);
      }

    linest=output+(y2*320);
    segend=output+(y3*320);

    while(linest<segend)
      {
      truels=leftside>>16;
      truers=rightside>>16;
      texx=ctx;
      texy=cty;

      if ((truels<screenright) && (truers>screenleft))
        {
        if (truers>screenright)
          truers=screenright;

        if (truels<screenleft)
          {
          texx-=(texxmov*(truels-screenleft));
          texy-=(texymov*(truels-screenleft));
          truels=screenleft;
          }

        dest=linest+truels;
  
        fin=linest+truers;
        while(dest<=fin)                   
          {
          psel=filter_table[(texx&255)+((texy&255)*256)];
          switch(psel)
            {
            case 0: *dest=shady[texture[(texy&65280)+(texx>>8)]]; break;
            case 1: *dest=shady[texture[(texy&65280)+(texx>>8)-1]]; break;
            case 2: *dest=shady[texture[(texy&65280)+(texx>>8)+1]]; break;
            case 3: *dest=shady[texture[(texy&65280)+(texx>>8)-256]]; break;
            case 4: *dest=shady[texture[(texy&65280)+(texx>>8)+256]]; break;
            }
          texy+=texymov;
          texx+=texxmov;
          dest++;
          }
        }

      leftside+=leftmove;
      rightside+=rightmove;
      ctx+=ctxmov;
      cty+=ctymov;
      linest+=320;
      }
    }
  }
  }

void polyst(int x1,int y1,int x2,int y2,int x3,int y3,unsigned char tx1,
                                                         unsigned char ty1,
                                                         unsigned char tx2,
                                                         unsigned char ty2,
                                                         unsigned char tx3,
                                                         unsigned char ty3,int b)

  {
  int texx,texy,texxmov,texymov; /* movement on x axis */
  int ctx,cty,ctxmov,ctymov;     /* movement on y axis */
  int cliplength;

  int temp;
  int leftside,rightside;
  int truels,truers;
  int leftmove,rightmove;
  int x4,y4,tx4,ty4;
  int segheight;
  char *segend;
  char *linest;
  char *dest;
  char *fin;

  int onright;
  int onleft;

  char *shady;

  shady=&texshade_table[b*256];

  onright=0;
  onleft=0;

  if (x1>319) onright++;
  if (x2>319) onright++;
  if (x3>319) onright++;

  if (x1<0) onleft++;
  if (x2<0) onleft++;
  if (x3<0) onleft++;

  if ((onright<3) && (onleft<3))
  {

  if (y1>y2)
    {
    temp=y1;
    y1=y2;
    y2=temp;

    temp=x1;
    x1=x2;
    x2=temp;

    temp=tx1;
    tx1=tx2;
    tx2=temp;

    temp=ty1;
    ty1=ty2;
    ty2=temp;
    }

  if (y2>y3)
    {
    temp=y2;
    y2=y3;
    y3=temp;

    temp=x2;
    x2=x3;
    x3=temp;

    temp=tx3;
    tx3=tx2;
    tx2=temp;

    temp=ty3;
    ty3=ty2;
    ty2=temp;
    }

  if (y1>y2)
    {
    temp=y1;
    y1=y2;
    y2=temp;

    temp=x1;
    x1=x2;
    x2=temp;

    temp=tx1;
    tx1=tx2;
    tx2=temp;

    temp=ty1;
    ty1=ty2;
    ty2=temp;
    }

  rightside=leftside=(x1<<16);

  x4=x1+((x3-x1)+1)*((y2-y1)+1)/((y3-y1)+1);
  y4=y2;

  tx4=tx1+((tx3-tx1)*((y2-y1)+1)/((y3-y1)+1));
  ty4=ty1+((ty3-ty1)*((y2-y1)+1)/((y3-y1)+1));

  if (x2>x4)
    {
    temp=x4;
    x4=x2;
    x2=temp;

    temp=tx4;
    tx4=tx2;
    tx2=temp;

    temp=ty4;
    ty4=ty2;
    ty2=temp;
    }

  segheight=((y2-y1)+1);

  texxmov=((tx4-tx2)<<8)/((x4-x2)+1);
  texymov=((ty4-ty2)<<8)/((x4-x2)+1);

  /* first section of poly */

  leftmove=((x2-x1)<<16)/segheight;
  rightmove=((x4-x1)<<16)/segheight;
  ctxmov=((tx2-tx1)<<8)/segheight;
  ctymov=((ty2-ty1)<<8)/segheight;
  ctx=tx1<<8;
  cty=ty1<<8;

  if ((y1<screenbot) && (y2>screentop))
    {
    if (y2>screenbot)
      {
      y2=screenbot;
      segheight=((y2-y1)+1);
      }

    if (y1<screentop)
      {
      cliplength=y1-screentop;
      leftside-=leftmove*cliplength;
      rightside-=rightmove*cliplength;
      ctx-=ctxmov*cliplength;
      cty-=ctymov*cliplength;
      y1=screentop;
      segheight=((y2-y1)+1);
      }

    linest=output+(y1*320);
    segend=output+(y2*320);

    while(linest<=segend)   
      {
      truels=leftside>>16;
      truers=rightside>>16;
      texx=ctx;
      texy=cty;

      if ((truels<screenright) && (truers>screenleft))
        {
        if (truers>screenright)
          truers=screenright;

        if (truels<screenleft)
          {
          texx-=(texxmov*(truels-screenleft));
          texy-=(texymov*(truels-screenleft));
          truels=screenleft;
          }

        dest=linest+truels;

        fin=linest+truers;
        while(dest<=fin)
          {
          *dest=shady[texture[(texy&65280)+(texx>>8)]];
          texy+=texymov;
          texx+=texxmov;
          dest++;
          }

        }

      leftside+=leftmove;
      rightside+=rightmove;
      ctx+=ctxmov;
      cty+=ctymov;
      linest+=320;
      }
    }

  /* second section of poly */

  leftside=x2<<16;
  rightside=x4<<16;
  ctx=tx2<<8;
  cty=ty2<<8;

  segheight=((y3-y2)+1);
  leftmove=((x3-x2)<<16)/segheight;
  rightmove=((x3-x4)<<16)/segheight;
  ctxmov=((tx3-tx2)<<8)/segheight;
  ctymov=((ty3-ty2)<<8)/segheight;

  if ((y2<screenbot) && (y3>screentop))
    {
    if (y3>screenbot)
      {
      y3=screenbot;
      segheight=((y3-y2)+1);
      }

    if (y2<screentop)
      {
      cliplength=y2-screentop;
      leftside-=leftmove*cliplength;
      rightside-=rightmove*cliplength;
      ctx-=ctxmov*cliplength;
      cty-=ctymov*cliplength;
      y2=screentop;
      segheight=((y3-y2)+1);
      }

    linest=output+(y2*320);
    segend=output+(y3*320);

    while(linest<segend)
      {
      truels=leftside>>16;
      truers=rightside>>16;
      texx=ctx;
      texy=cty;

      if ((truels<screenright) && (truers>screenleft))
        {
        if (truers>screenright)
          truers=screenright;

        if (truels<screenleft)
          {
          texx-=(texxmov*(truels-screenleft));
          texy-=(texymov*(truels-screenleft));
          truels=screenleft;
          }

        dest=linest+truels;
  
        fin=linest+truers;
        while(dest<=fin)                   
          {
          *dest=shady[texture[(texy&65280)+(texx>>8)]];
          texy+=texymov;
          texx+=texxmov;
          dest++;
          }
        }

      leftside+=leftmove;
      rightside+=rightmove;
      ctx+=ctxmov;
      cty+=ctymov;
      linest+=320;
      }
    }
  }
  }

void polyft(int x1,int y1,int x2,int y2,int x3,int y3,unsigned char tx1,
                                                         unsigned char ty1,
                                                         unsigned char tx2,
                                                         unsigned char ty2,
                                                         unsigned char tx3,
                                                         unsigned char ty3)

  {
  int texx,texy,texxmov,texymov; /* movement on x axis */
  int ctx,cty,ctxmov,ctymov;     /* movement on y axis */
  int cliplength;

  int temp;
  int leftside,rightside;
  int truels,truers;
  int leftmove,rightmove;
  int x4,y4,tx4,ty4;
  int segheight;
  char *segend;
  char *linest;
  char *dest;
  char *fin;

  int onright;
  int onleft;
  char psel;

  onright=0;
  onleft=0;

  if (x1>319) onright++;
  if (x2>319) onright++;
  if (x3>319) onright++;

  if (x1<0) onleft++;
  if (x2<0) onleft++;
  if (x3<0) onleft++;

  if ((onright<3) && (onleft<3))
  {

  if (y1>y2)
    {
    temp=y1;
    y1=y2;
    y2=temp;

    temp=x1;
    x1=x2;
    x2=temp;

    temp=tx1;
    tx1=tx2;
    tx2=temp;

    temp=ty1;
    ty1=ty2;
    ty2=temp;
    }

  if (y2>y3)
    {
    temp=y2;
    y2=y3;
    y3=temp;

    temp=x2;
    x2=x3;
    x3=temp;

    temp=tx3;
    tx3=tx2;
    tx2=temp;

    temp=ty3;
    ty3=ty2;
    ty2=temp;
    }

  if (y1>y2)
    {
    temp=y1;
    y1=y2;
    y2=temp;

    temp=x1;
    x1=x2;
    x2=temp;

    temp=tx1;
    tx1=tx2;
    tx2=temp;

    temp=ty1;
    ty1=ty2;
    ty2=temp;
    }

  rightside=leftside=(x1<<16);

  x4=x1+((x3-x1)+1)*((y2-y1)+1)/((y3-y1)+1);
  y4=y2;

  tx4=tx1+((tx3-tx1)*((y2-y1)+1)/((y3-y1)+1));
  ty4=ty1+((ty3-ty1)*((y2-y1)+1)/((y3-y1)+1));

  if (x2>x4)
    {
    temp=x4;
    x4=x2;
    x2=temp;

    temp=tx4;
    tx4=tx2;
    tx2=temp;

    temp=ty4;
    ty4=ty2;
    ty2=temp;
    }

  segheight=((y2-y1)+1);

  texxmov=((tx4-tx2)<<8)/((x4-x2)+1);
  texymov=((ty4-ty2)<<8)/((x4-x2)+1);

  /* first section of poly */

  leftmove=((x2-x1)<<16)/segheight;
  rightmove=((x4-x1)<<16)/segheight;
  ctxmov=((tx2-tx1)<<8)/segheight;
  ctymov=((ty2-ty1)<<8)/segheight;
  ctx=tx1<<8;
  cty=ty1<<8;

  if ((y1<screenbot) && (y2>screentop))
    {
    if (y2>screenbot)
      {
      y2=screenbot;
      segheight=((y2-y1)+1);
      }

    if (y1<screentop)
      {
      cliplength=y1-screentop;
      leftside-=leftmove*cliplength;
      rightside-=rightmove*cliplength;
      ctx-=ctxmov*cliplength;
      cty-=ctymov*cliplength;
      y1=screentop;
      segheight=((y2-y1)+1);
      }

    linest=output+(y1*320);
    segend=output+(y2*320);

    while(linest<=segend)   
      {
      truels=leftside>>16;
      truers=rightside>>16;
      texx=ctx;
      texy=cty;

      if ((truels<screenright) && (truers>screenleft))
        {
        if (truers>screenright)
          truers=screenright;

        if (truels<screenleft)
          {
          texx-=(texxmov*(truels-screenleft));
          texy-=(texymov*(truels-screenleft));
          truels=screenleft;
          }

        dest=linest+truels;

        fin=linest+truers;
        while(dest<=fin)
          {
          psel=filter_table[(texx&255)+((texy&255)*256)];
          switch(psel)
            {
            case 0: *dest=texture[(texy&65280)+(texx>>8)]; break;
            case 1: *dest=texture[(texy&65280)+(texx>>8)-1]; break;
            case 2: *dest=texture[(texy&65280)+(texx>>8)+1]; break;
            case 3: *dest=texture[(texy&65280)+(texx>>8)-256]; break;
            case 4: *dest=texture[(texy&65280)+(texx>>8)+256]; break;
            }
          texy+=texymov;
          texx+=texxmov;
          dest++;
          }

        }

      leftside+=leftmove;
      rightside+=rightmove;
      ctx+=ctxmov;
      cty+=ctymov;
      linest+=320;
      }
    }

  /* second section of poly */

  leftside=x2<<16;
  rightside=x4<<16;
  ctx=tx2<<8;
  cty=ty2<<8;

  segheight=((y3-y2)+1);
  leftmove=((x3-x2)<<16)/segheight;
  rightmove=((x3-x4)<<16)/segheight;
  ctxmov=((tx3-tx2)<<8)/segheight;
  ctymov=((ty3-ty2)<<8)/segheight;

  if ((y2<screenbot) && (y3>screentop))
    {
    if (y3>screenbot)
      {
      y3=screenbot;
      segheight=((y3-y2)+1);
      }

    if (y2<screentop)
      {
      cliplength=y2-screentop;
      leftside-=leftmove*cliplength;
      rightside-=rightmove*cliplength;
      ctx-=ctxmov*cliplength;
      cty-=ctymov*cliplength;
      y2=screentop;
      segheight=((y3-y2)+1);
      }

    linest=output+(y2*320);
    segend=output+(y3*320);

    while(linest<segend)
      {
      truels=leftside>>16;
      truers=rightside>>16;
      texx=ctx;
      texy=cty;

      if ((truels<screenright) && (truers>screenleft))
        {
        if (truers>screenright)
          truers=screenright;

        if (truels<screenleft)
          {
          texx-=(texxmov*(truels-screenleft));
          texy-=(texymov*(truels-screenleft));
          truels=screenleft;
          }

        dest=linest+truels;
  
        fin=linest+truers;
        while(dest<=fin)                   
          {
          psel=filter_table[(texx&255)+((texy&255)*256)];
          switch(psel)
            {
            case 0: *dest=texture[(texy&65280)+(texx>>8)]; break;
            case 1: *dest=texture[(texy&65280)+(texx>>8)-1]; break;
            case 2: *dest=texture[(texy&65280)+(texx>>8)+1]; break;
            case 3: *dest=texture[(texy&65280)+(texx>>8)-256]; break;
            case 4: *dest=texture[(texy&65280)+(texx>>8)+256]; break;
            }
          texy+=texymov;
          texx+=texxmov;
          dest++;
          }
        }

      leftside+=leftmove;
      rightside+=rightmove;
      ctx+=ctxmov;
      cty+=ctymov;
      linest+=320;
      }
    }
  }
  }

void polyt(int x1,int y1,int x2,int y2,int x3,int y3,unsigned char tx1,
                                                         unsigned char ty1,
                                                         unsigned char tx2,
                                                         unsigned char ty2,
                                                         unsigned char tx3,
                                                         unsigned char ty3)

  {
  int texx,texy,texxmov,texymov; /* movement on x axis */
  int ctx,cty,ctxmov,ctymov;     /* movement on y axis */
  int cliplength;

  int temp;
  int leftside,rightside;
  int truels,truers;
  int leftmove,rightmove;
  int x4,y4,tx4,ty4;
  int segheight;
  char *segend;
  char *linest;
  char *dest;
  char *fin;

  int onright;
  int onleft;

  onright=0;
  onleft=0;

  if (x1>319) onright++;
  if (x2>319) onright++;
  if (x3>319) onright++;

  if (x1<0) onleft++;
  if (x2<0) onleft++;
  if (x3<0) onleft++;

  if ((onright<3) && (onleft<3))
  {

  if (y1>y2)
    {
    temp=y1;
    y1=y2;
    y2=temp;

    temp=x1;
    x1=x2;
    x2=temp;

    temp=tx1;
    tx1=tx2;
    tx2=temp;

    temp=ty1;
    ty1=ty2;
    ty2=temp;
    }

  if (y2>y3)
    {
    temp=y2;
    y2=y3;
    y3=temp;

    temp=x2;
    x2=x3;
    x3=temp;

    temp=tx3;
    tx3=tx2;
    tx2=temp;

    temp=ty3;
    ty3=ty2;
    ty2=temp;
    }

  if (y1>y2)
    {
    temp=y1;
    y1=y2;
    y2=temp;

    temp=x1;
    x1=x2;
    x2=temp;

    temp=tx1;
    tx1=tx2;
    tx2=temp;

    temp=ty1;
    ty1=ty2;
    ty2=temp;
    }

  rightside=leftside=(x1<<16);

  x4=x1+((x3-x1)+1)*((y2-y1)+1)/((y3-y1)+1);
  y4=y2;

  tx4=tx1+((tx3-tx1)*((y2-y1)+1)/((y3-y1)+1));
  ty4=ty1+((ty3-ty1)*((y2-y1)+1)/((y3-y1)+1));

  if (x2>x4)
    {
    temp=x4;
    x4=x2;
    x2=temp;

    temp=tx4;
    tx4=tx2;
    tx2=temp;

    temp=ty4;
    ty4=ty2;
    ty2=temp;
    }

  segheight=((y2-y1)+1);

  texxmov=((tx4-tx2)<<8)/((x4-x2)+1);
  texymov=((ty4-ty2)<<8)/((x4-x2)+1);

  /* first section of poly */

  leftmove=((x2-x1)<<16)/segheight;
  rightmove=((x4-x1)<<16)/segheight;
  ctxmov=((tx2-tx1)<<8)/segheight;
  ctymov=((ty2-ty1)<<8)/segheight;
  ctx=tx1<<8;
  cty=ty1<<8;

  if ((y1<screenbot) && (y2>screentop))
    {
    if (y2>screenbot)
      {
      y2=screenbot;
      segheight=((y2-y1)+1);
      }

    if (y1<screentop)
      {
      cliplength=y1-screentop;
      leftside-=leftmove*cliplength;
      rightside-=rightmove*cliplength;
      ctx-=ctxmov*cliplength;
      cty-=ctymov*cliplength;
      y1=screentop;
      segheight=((y2-y1)+1);
      }

    linest=output+(y1*320);
    segend=output+(y2*320);

    while(linest<=segend)   
      {
      truels=leftside>>16;
      truers=rightside>>16;
      texx=ctx;
      texy=cty;

      if ((truels<screenright) && (truers>screenleft))
        {
        if (truers>screenright)
          truers=screenright;

        if (truels<screenleft)
          {
          texx-=(texxmov*(truels-screenleft));
          texy-=(texymov*(truels-screenleft));
          truels=screenleft;
          }

        dest=linest+truels;

        fin=linest+truers;
        while(dest<=fin)
          {
          *dest=texture[(texy&65280)+(texx>>8)];
          texy+=texymov;
          texx+=texxmov;
          dest++;
          }

        }

      leftside+=leftmove;
      rightside+=rightmove;
      ctx+=ctxmov;
      cty+=ctymov;
      linest+=320;
      }
    }

  /* second section of poly */

  leftside=x2<<16;
  rightside=x4<<16;
  ctx=tx2<<8;
  cty=ty2<<8;

  segheight=((y3-y2)+1);
  leftmove=((x3-x2)<<16)/segheight;
  rightmove=((x3-x4)<<16)/segheight;
  ctxmov=((tx3-tx2)<<8)/segheight;
  ctymov=((ty3-ty2)<<8)/segheight;

  if ((y2<screenbot) && (y3>screentop))
    {
    if (y3>screenbot)
      {
      y3=screenbot;
      segheight=((y3-y2)+1);
      }

    if (y2<screentop)
      {
      cliplength=y2-screentop;
      leftside-=leftmove*cliplength;
      rightside-=rightmove*cliplength;
      ctx-=ctxmov*cliplength;
      cty-=ctymov*cliplength;
      y2=screentop;
      segheight=((y3-y2)+1);
      }

    linest=output+(y2*320);
    segend=output+(y3*320);

    while(linest<segend)
      {
      truels=leftside>>16;
      truers=rightside>>16;
      texx=ctx;
      texy=cty;

      if ((truels<screenright) && (truers>screenleft))
        {
        if (truers>screenright)
          truers=screenright;

        if (truels<screenleft)
          {
          texx-=(texxmov*(truels-screenleft));
          texy-=(texymov*(truels-screenleft));
          truels=screenleft;
          }

        dest=linest+truels;
  
        fin=linest+truers;
        while(dest<=fin)                   
          {
          *dest=texture[(texy&65280)+(texx>>8)];
          texy+=texymov;
          texx+=texxmov;
          dest++;
          }
        }

      leftside+=leftmove;
      rightside+=rightmove;
      ctx+=ctxmov;
      cty+=ctymov;
      linest+=320;
      }
    }
  }
  }
