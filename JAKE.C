#include<stdio.h>
#include<stdlib.h>
#include<dpmi.h>
#include"mouse.c"
#include"c:/dj/src/jlib/jlib.h"
#include"blitter.c"

/* JAKE Jon's Quake */

#define originx 160
#define originy 100
#define zoom    130
#define zbound  5

#define worldx 3000
#define worldz 3000

#define tilesize 25

#define FLOOR 0
#define XWALL 1
#define ZWALL 2


#define LOW 0
#define HIGH 1

typedef struct {
float x;
float y;
float z;
float newx;
float newy;
float newz;
int scrx;
int scry;
} vertex;

typedef struct {
unsigned int v1;
unsigned int v2;
unsigned int v3;
unsigned char tx1;
unsigned char ty1;
unsigned char tx2;
unsigned char ty2;
unsigned char tx3;
unsigned char ty3;
/* texture is inherited from object */
} textri;

typedef struct {
float  xpos;
float  ypos;
float  zpos;
int    numverts;
int    numtris;
vertex *oldverts;
textri *oldtris;
} object;

typedef struct {
object *top;
object *bottom;
object *left;
object *right;
object *front;
object *back;
float  topy;
float  bottomy;
float  leftx;
float  rightx;
float  frontz;
float  backz;
char   *toptex;
char   *bottomtex;
char   *lefttex;
char   *righttex;
char   *fronttex;
char   *backtex;
int    bricktype;
} brick;

brick bricklist[1024];

int player_xtile,player_ztile;

bitmap texture_base;
char mypal[768];

float sint[360];
float cost[360];

unsigned char ceilingmap[256][256]; 
unsigned char floormap[256][256];
char draworderx[1024];
char draworderz[1024];

int xrot,yrot,zrot;
float internal_xrot,internal_yrot,internal_zrot;

float xt,yt,zt;
float x,y,z;

float player_xpos,player_ypos,player_zpos;

int xmouse,ymouse,button_st;
int but=2;

int curpolys,maxpolys;
char buf[80];

float speed;

unsigned char horizon_colour;

int numbricks=0;
int brick_index;

void create_surface(object *floor,int surface_type,float x1,float z1,float x2,float z2,float depth)

  {
  int xland,zland;
  int vert1,vert2,vert3,vert4;
  int xdots,zdots;
  int xtiles,ztiles;
  float xtf,ztf;
  int polygen=0;

  textri *current_tri;
  vertex *current_vertex;

  float xtilesize,ztilesize;
  
  switch (surface_type) {
  case FLOOR: 
    floor->xpos=x1;
    floor->zpos=z1;
    floor->ypos=depth;
    break;
  case XWALL:
    floor->xpos=depth;
    floor->ypos=x1;
    floor->zpos=z1;
    break;
  case ZWALL:
    floor->xpos=x1;
    floor->ypos=z1;
    floor->zpos=depth;
    break;
    }

  xtf=(x2-x1)/32;
  xtiles=xtf+1;
  xtilesize=(x2-x1)/xtiles;

  ztf=(z2-z1)/32;
  ztiles=ztf+1;
  ztilesize=(z2-z1)/ztiles;

/*  printf("size: (x) %f (z) %f\n",(xtilesize*xtf),(ztilesize*ztf));
  printf("floats: (x) %f (z) %f\n",xtf,ztf);

  printf("xtiles: %d, ztiles: %d\n",xtiles,ztiles);
  printf("xtilesize: %f, ztilesize: %f\n",xtilesize,ztilesize);*/

  xdots=xtiles+1;
  zdots=ztiles+1;
  floor->numverts=xdots*zdots;
  floor->numtris=xtiles*ztiles*2;
/*  printf("Vertices: %d, Tris: %d\n",floor->numverts,floor->numtris);*/
  floor->oldverts=(vertex *) malloc(floor->numverts*sizeof(vertex));
  floor->oldtris=(textri *) malloc(floor->numtris*sizeof(textri)*2);

  current_vertex=floor->oldverts;
  for (zland=0; zland<zdots; zland++)
    for (xland=0; xland<xdots; xland++)
      {
      switch(surface_type) {
      case FLOOR:
        current_vertex->x=xland*xtilesize;
        current_vertex->y=0;
        current_vertex->z=zland*ztilesize;
        break;
      case XWALL:
        current_vertex->x=0;
        current_vertex->y=xland*xtilesize;
        current_vertex->z=zland*ztilesize;
        break;
      case ZWALL:
        current_vertex->x=xland*xtilesize;
        current_vertex->y=zland*ztilesize;
        current_vertex->z=0;
        break;
        }
      current_vertex++;
      }

  current_tri=floor->oldtris;
  for (zland=0; zland<ztiles; zland++)
    for (xland=0; xland<xtiles; xland++)
      {
      vert1=zland*xdots+xland;
      vert2=zland*xdots+xland+1;
      vert3=((zland+1)*xdots)+xland;
      vert4=((zland+1)*xdots)+xland+1;

      current_tri->v1=vert1;
      current_tri->v2=vert2;
      current_tri->v3=vert3;
      current_tri->tx1=xland*xtilesize;
      current_tri->ty1=zland*ztilesize;
      current_tri->tx2=(xland+1)*xtilesize;
      current_tri->ty2=zland*ztilesize;
      current_tri->tx3=xland*xtilesize;
      current_tri->ty3=(zland+1)*ztilesize;
      current_tri++;

      current_tri->v1=vert2;
      current_tri->v2=vert4;
      current_tri->v3=vert3;
      current_tri->tx1=(xland+1)*xtilesize;
      current_tri->ty1=zland*ztilesize;
      current_tri->tx2=(xland+1)*xtilesize;
      current_tri->ty2=(zland+1)*ztilesize;
      current_tri->tx3=xland*xtilesize;
      current_tri->ty3=(zland+1)*ztilesize-1;
      current_tri++;

      polygen+=2;
      }
/*  printf("polys generated: %d\n",polygen);*/
  }

void create_brick(float x1,float y1,float z1,float x2,float y2,float z2,
                  int newlefttex,int newtoptex,int newbacktex, int newrighttex, int newbottomtex, int newfronttex,int type)
  {
  brick *newbrick;

  newbrick=&bricklist[numbricks];

  newbrick->bricktype=type;
  newbrick->topy=y1;
  newbrick->bottomy=y2;
  newbrick->leftx=x1;
  newbrick->rightx=x2;
  newbrick->frontz=z1;
  newbrick->backz=z2;

  if (newtoptex!=-1)
    {
    newbrick->top=(object *) malloc(sizeof(object));
    create_surface(newbrick->top,FLOOR,x1,z1,x2,z2,y1);
    newbrick->toptex=texture_base.location+(newtoptex*65536);
    }
    else
    newbrick->top=NULL;

  if (newbottomtex!=-1)
    {
    newbrick->bottom=(object *) malloc(sizeof(object));
    create_surface(newbrick->bottom,FLOOR,x1,z1,x2,z2,y2);
    newbrick->bottomtex=texture_base.location+(newbottomtex*65536);
    }
    else
    newbrick->bottom=NULL;

  if (newlefttex!=-1)
    {
    newbrick->left=(object *) malloc(sizeof(object));
    create_surface(newbrick->left,XWALL,y1,z1,y2,z2,x1);
    newbrick->lefttex=texture_base.location+(newlefttex*65536);
    }
    else
    newbrick->left=NULL;

  if (newrighttex!=-1)
    {  
    newbrick->right=(object *) malloc(sizeof(object));
    create_surface(newbrick->right,XWALL,y1,z1,y2,z2,x2);
    newbrick->righttex=texture_base.location+(newrighttex*65536);
    }
    else
    newbrick->right=NULL;

  if (newfronttex!=-1)
    {
    newbrick->front=(object *) malloc(sizeof(object));
    create_surface(newbrick->front,ZWALL,x1,y1,x2,y2,z1);
    newbrick->fronttex=texture_base.location+(newfronttex*65536);
    }
    else
    newbrick->front=NULL;

  if (newbacktex!=-1)
    {
    newbrick->back=(object *) malloc(sizeof(object));
    create_surface(newbrick->back,ZWALL,x1,y1,x2,y2,z2);
    newbrick->backtex=texture_base.location+(newbacktex*65536);
    }
    else
    newbrick->back=NULL;

  numbricks++;
  }


void create_floor_map()
  {
  unsigned char bricknum;
  float centrex;
  float centrez;
  brick* current_brick;
  int placex;
  int placez;
  unsigned char brickat;

  memset(floormap,255,65536);
  memset(ceilingmap,255,65536);

  current_brick=bricklist;
  for (bricknum=0; bricknum<numbricks; bricknum++)
    {
    current_brick=&bricklist[bricknum];

    centrez=(current_brick->frontz+current_brick->backz)/2;
    centrex=(current_brick->rightx+current_brick->leftx)/2;
    placez=centrez/tilesize;
    placex=centrex/tilesize;

    if (current_brick->bricktype==LOW)
      {
      brickat=floormap[placez][placex];
      if (brickat!=255) /* 255 = unoccupied square */
        printf("floor position conflict at %d %d! (blocks %d and %d)\n",placez,placex,bricknum,brickat);
        else
        floormap[placez][placex]=bricknum;
      }
      else
      {
      brickat=ceilingmap[placez][placex];
      if (brickat!=255) /* 255 = unoccupied square */
        printf("ceiling position conflict at %d %d! (blocks %d and %d)\n",placez,placex,bricknum,brickat);
        else
        ceilingmap[placez][placex]=bricknum;
      }

    }

  }

void sincostable2(float *array,
                  float size,
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


void project(object *current_object)
  {
  vertex *current_vertex;
  float z;
  int vertex_number;

  current_vertex=current_object->oldverts;

  for (vertex_number=current_object->numverts; vertex_number>0; vertex_number--)
    {
    z=current_vertex->newz/zoom;
    current_vertex->scrx=current_vertex->newx/z;
    current_vertex->scry=current_vertex->newy/z;
    current_vertex++;
    }
  }

void transform_and_rotate(object *current_object)
  {
  int vertex_number;
  vertex *current_old_vertex;

  float xposadd;
  float yposadd;
  float zposadd;

  xposadd=current_object->xpos-player_xpos;
  yposadd=current_object->ypos-player_ypos;
  zposadd=current_object->zpos-player_zpos;

  current_old_vertex=current_object->oldverts;

  for (vertex_number=current_object->numverts; vertex_number>0; vertex_number--)
    {
    x=current_old_vertex->x;
    y=current_old_vertex->y;
    z=current_old_vertex->z;

    x+=xposadd;
    y+=yposadd;
    z+=zposadd;

    xt = x*cost[yrot] - z*sint[yrot];
    zt = x*sint[yrot] + z*cost[yrot];
    x = xt;
    z = zt;

    yt = y*cost[xrot] - z*sint[xrot];
    zt = y*sint[xrot] + z*cost[xrot];
    y = yt;
    z = zt;

    xt = x*cost[zrot] - y*sint[zrot];
    yt = x*sint[zrot] + y*cost[zrot];
    x = xt;
    y = yt;

    current_old_vertex->newx=x;
    current_old_vertex->newy=y;
    current_old_vertex->newz=z;

    current_old_vertex++;
    }
  }

display_surface(object *current_surface)
  {
  textri *current_tri;
  int trinum;
  vertex *vertex_list;
  int intersections;

  vertex tempv1,tempv2;
  vertex *cv1;
  vertex *cv2;

  float temptx1,temptx2,tempty1,tempty2;
  float avgz;

  transform_and_rotate(current_surface);
  project(current_surface);

  current_tri=current_surface->oldtris;
  vertex_list=current_surface->oldverts;

  for (trinum=0; trinum<current_surface->numtris; trinum++)
    {
    avgz=(vertex_list[current_tri->v1].newz+
         vertex_list[current_tri->v2].newz+
         vertex_list[current_tri->v3].newz)/3;

    intersections=0;
    if (vertex_list[current_tri->v1].newz<zbound) intersections|=1; /* 001 */
    if (vertex_list[current_tri->v2].newz<zbound) intersections|=2; /* 010 */
    if (vertex_list[current_tri->v3].newz<zbound) intersections|=4; /* 100 */
    if (avgz>75) intersections=-1;
    if (avgz>(5*64)) intersections=-2;

    switch(intersections) {

    case -1:
      polyst(originx+vertex_list[current_tri->v1].scrx,originy+vertex_list[current_tri->v1].scry,
               originx+vertex_list[current_tri->v2].scrx,originy+vertex_list[current_tri->v2].scry,
               originx+vertex_list[current_tri->v3].scrx,originy+vertex_list[current_tri->v3].scry,
               current_tri->tx1,current_tri->ty1,
               current_tri->tx2,current_tri->ty2,
               current_tri->tx3,current_tri->ty3,avgz/5);
      curpolys++;
      break;

    case 0:
      polyfst(originx+vertex_list[current_tri->v1].scrx,originy+vertex_list[current_tri->v1].scry,
               originx+vertex_list[current_tri->v2].scrx,originy+vertex_list[current_tri->v2].scry,
               originx+vertex_list[current_tri->v3].scrx,originy+vertex_list[current_tri->v3].scry,
               current_tri->tx1,current_tri->ty1,
               current_tri->tx2,current_tri->ty2,
               current_tri->tx3,current_tri->ty3,avgz/5);
      curpolys++;
      break;

    case 1:
      cv1=&vertex_list[current_tri->v2];
      cv2=&vertex_list[current_tri->v1];
      tempv1.newz=zbound;
      tempv1.newx=waypoint(cv1->newx,cv2->newx,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv1.newy=waypoint(cv1->newy,cv2->newy,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv1.scrx=tempv1.newx/(tempv1.newz/zoom);
      tempv1.scry=tempv1.newy/(tempv1.newz/zoom);
      temptx1=waypoint(current_tri->tx2,current_tri->tx1,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempty1=waypoint(current_tri->ty2,current_tri->ty1,cv1->newz-cv2->newz,cv1->newz-zbound);

      cv1=&vertex_list[current_tri->v3];
      cv2=&vertex_list[current_tri->v1];
      tempv2.newz=zbound;
      tempv2.newx=waypoint(cv1->newx,cv2->newx,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv2.newy=waypoint(cv1->newy,cv2->newy,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv2.scrx=tempv2.newx/(tempv2.newz/zoom);
      tempv2.scry=tempv2.newy/(tempv2.newz/zoom);
      temptx2=waypoint(current_tri->tx3,current_tri->tx1,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempty2=waypoint(current_tri->ty3,current_tri->ty1,cv1->newz-cv2->newz,cv1->newz-zbound);

      polyfst(originx+vertex_list[current_tri->v2].scrx,originy+vertex_list[current_tri->v2].scry,
               originx+vertex_list[current_tri->v3].scrx,originy+vertex_list[current_tri->v3].scry,
               originx+tempv1.scrx,originy+tempv1.scry,
               current_tri->tx2,current_tri->ty2,
               current_tri->tx3,current_tri->ty3,
               temptx1,tempty1,avgz/5);

      polyfst(originx+vertex_list[current_tri->v3].scrx,originy+vertex_list[current_tri->v3].scry,
               originx+tempv2.scrx,originy+tempv2.scry,
               originx+tempv1.scrx,originy+tempv1.scry,
               current_tri->tx3,current_tri->ty3,
               temptx1,tempty1,
               temptx2,tempty2,avgz/5);

      curpolys+=2;
      break;


    case 2:
      cv1=&vertex_list[current_tri->v3];
      cv2=&vertex_list[current_tri->v2];
      tempv1.newz=zbound;
      tempv1.newx=waypoint(cv1->newx,cv2->newx,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv1.newy=waypoint(cv1->newy,cv2->newy,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv1.scrx=tempv1.newx/(tempv1.newz/zoom);
      tempv1.scry=tempv1.newy/(tempv1.newz/zoom);
      temptx1=waypoint(current_tri->tx3,current_tri->tx2,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempty1=waypoint(current_tri->ty3,current_tri->ty2,cv1->newz-cv2->newz,cv1->newz-zbound);

      cv1=&vertex_list[current_tri->v1];
      cv2=&vertex_list[current_tri->v2];
      tempv2.newz=zbound;
      tempv2.newx=waypoint(cv1->newx,cv2->newx,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv2.newy=waypoint(cv1->newy,cv2->newy,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv2.scrx=tempv2.newx/(tempv2.newz/zoom);
      tempv2.scry=tempv2.newy/(tempv2.newz/zoom);
      temptx2=waypoint(current_tri->tx1,current_tri->tx2,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempty2=waypoint(current_tri->ty1,current_tri->ty2,cv1->newz-cv2->newz,cv1->newz-zbound);

      polyfst(originx+vertex_list[current_tri->v3].scrx,originy+vertex_list[current_tri->v3].scry,
               originx+vertex_list[current_tri->v1].scrx,originy+vertex_list[current_tri->v1].scry,
               originx+tempv1.scrx,originy+tempv1.scry,
               current_tri->tx3,current_tri->ty3,
               current_tri->tx1,current_tri->ty1,
               temptx1,tempty1,avgz/5);

      polyfst(originx+vertex_list[current_tri->v1].scrx,originy+vertex_list[current_tri->v1].scry,
               originx+tempv1.scrx,originy+tempv1.scry,
               originx+tempv2.scrx,originy+tempv2.scry,
               current_tri->tx1,current_tri->ty1,
               temptx1,tempty1,
               temptx2,tempty2,avgz/5);

      curpolys+=2;
      break;

    case 4:
      cv1=&vertex_list[current_tri->v1];
      cv2=&vertex_list[current_tri->v3];
      tempv1.newz=zbound;
      tempv1.newx=waypoint(cv1->newx,cv2->newx,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv1.newy=waypoint(cv1->newy,cv2->newy,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv1.scrx=tempv1.newx/(tempv1.newz/zoom);
      tempv1.scry=tempv1.newy/(tempv1.newz/zoom);
      temptx1=waypoint(current_tri->tx1,current_tri->tx3,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempty1=waypoint(current_tri->ty1,current_tri->ty3,cv1->newz-cv2->newz,cv1->newz-zbound);

      cv1=&vertex_list[current_tri->v2];
      cv2=&vertex_list[current_tri->v3];
      tempv2.newz=zbound;
      tempv2.newx=waypoint(cv1->newx,cv2->newx,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv2.newy=waypoint(cv1->newy,cv2->newy,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv2.scrx=tempv2.newx/(tempv2.newz/zoom);
      tempv2.scry=tempv2.newy/(tempv2.newz/zoom);
      temptx2=waypoint(current_tri->tx2,current_tri->tx3,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempty2=waypoint(current_tri->ty2,current_tri->ty3,cv1->newz-cv2->newz,cv1->newz-zbound);

      polyfst(originx+vertex_list[current_tri->v1].scrx,originy+vertex_list[current_tri->v1].scry,
               originx+vertex_list[current_tri->v2].scrx,originy+vertex_list[current_tri->v2].scry,
               originx+tempv1.scrx,originy+tempv1.scry,
               current_tri->tx1,current_tri->ty1,
               current_tri->tx2,current_tri->ty2,
               temptx1,tempty1,avgz/5);

      polyfst(originx+vertex_list[current_tri->v2].scrx,originy+vertex_list[current_tri->v2].scry,
               originx+tempv1.scrx,originy+tempv1.scry,
               originx+tempv2.scrx,originy+tempv2.scry,
               current_tri->tx2,current_tri->ty2,
               temptx1,tempty1,
               temptx2,tempty2,avgz/5);

      curpolys+=2;
      break;

    case 3:
      cv1=&vertex_list[current_tri->v3];
      cv2=&vertex_list[current_tri->v2];
      tempv1.newz=zbound;
      tempv1.newx=waypoint(cv1->newx,cv2->newx,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv1.newy=waypoint(cv1->newy,cv2->newy,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv1.scrx=tempv1.newx/(tempv1.newz/zoom);
      tempv1.scry=tempv1.newy/(tempv1.newz/zoom);
      temptx1=waypoint(current_tri->tx3,current_tri->tx2,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempty1=waypoint(current_tri->ty3,current_tri->ty2,cv1->newz-cv2->newz,cv1->newz-zbound);

      cv1=&vertex_list[current_tri->v3];
      cv2=&vertex_list[current_tri->v1];
      tempv2.newz=zbound;
      tempv2.newx=waypoint(cv1->newx,cv2->newx,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv2.newy=waypoint(cv1->newy,cv2->newy,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv2.scrx=tempv2.newx/(tempv2.newz/zoom);
      tempv2.scry=tempv2.newy/(tempv2.newz/zoom);
      temptx2=waypoint(current_tri->tx3,current_tri->tx1,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempty2=waypoint(current_tri->ty3,current_tri->ty1,cv1->newz-cv2->newz,cv1->newz-zbound);

      polyfst(originx+vertex_list[current_tri->v3].scrx,originy+vertex_list[current_tri->v3].scry,
               originx+tempv1.scrx,originy+tempv1.scry,
               originx+tempv2.scrx,originy+tempv2.scry,
               current_tri->tx3,current_tri->ty3,
               temptx1,tempty1,
               temptx2,tempty2,avgz/5);

      curpolys++;
      break;

    case 5:
      cv1=&vertex_list[current_tri->v2];
      cv2=&vertex_list[current_tri->v3];
      tempv1.newz=zbound;
      tempv1.newx=waypoint(cv1->newx,cv2->newx,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv1.newy=waypoint(cv1->newy,cv2->newy,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv1.scrx=tempv1.newx/(tempv1.newz/zoom);
      tempv1.scry=tempv1.newy/(tempv1.newz/zoom);
      temptx1=waypoint(current_tri->tx2,current_tri->tx3,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempty1=waypoint(current_tri->ty2,current_tri->ty3,cv1->newz-cv2->newz,cv1->newz-zbound);

      cv1=&vertex_list[current_tri->v2];
      cv2=&vertex_list[current_tri->v1];
      tempv2.newz=zbound;
      tempv2.newx=waypoint(cv1->newx,cv2->newx,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv2.newy=waypoint(cv1->newy,cv2->newy,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv2.scrx=tempv2.newx/(tempv2.newz/zoom);
      tempv2.scry=tempv2.newy/(tempv2.newz/zoom);
      temptx2=waypoint(current_tri->tx2,current_tri->tx1,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempty2=waypoint(current_tri->ty2,current_tri->ty1,cv1->newz-cv2->newz,cv1->newz-zbound);

      polyfst(originx+vertex_list[current_tri->v2].scrx,originy+vertex_list[current_tri->v2].scry,
               originx+tempv2.scrx,originy+tempv2.scry,
               originx+tempv1.scrx,originy+tempv1.scry,
               current_tri->tx2,current_tri->ty2,
               temptx2,tempty2,
               temptx1,tempty1,avgz/5);

      curpolys++;
      break;

    case 6:
      cv1=&vertex_list[current_tri->v1];
      cv2=&vertex_list[current_tri->v3];
      tempv1.newz=zbound;
      tempv1.newx=waypoint(cv1->newx,cv2->newx,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv1.newy=waypoint(cv1->newy,cv2->newy,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv1.scrx=tempv1.newx/(tempv1.newz/zoom);
      tempv1.scry=tempv1.newy/(tempv1.newz/zoom);
      temptx1=waypoint(current_tri->tx1,current_tri->tx3,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempty1=waypoint(current_tri->ty1,current_tri->ty3,cv1->newz-cv2->newz,cv1->newz-zbound);

      cv1=&vertex_list[current_tri->v1];
      cv2=&vertex_list[current_tri->v2];
      tempv2.newz=zbound;
      tempv2.newx=waypoint(cv1->newx,cv2->newx,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv2.newy=waypoint(cv1->newy,cv2->newy,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempv2.scrx=tempv2.newx/(tempv2.newz/zoom);
      tempv2.scry=tempv2.newy/(tempv2.newz/zoom);
      temptx2=waypoint(current_tri->tx1,current_tri->tx2,cv1->newz-cv2->newz,cv1->newz-zbound);
      tempty2=waypoint(current_tri->ty1,current_tri->ty2,cv1->newz-cv2->newz,cv1->newz-zbound);

      polyfst(originx+vertex_list[current_tri->v1].scrx,originy+vertex_list[current_tri->v1].scry,
               originx+tempv1.scrx,originy+tempv1.scry,
               originx+tempv2.scrx,originy+tempv2.scry,
               current_tri->tx1,current_tri->ty1,
               temptx1,tempty1,
               temptx2,tempty2,avgz/5);

      curpolys++;
      break;
      }
      current_tri++;
    }

  }


void draw_brick(brick *drawbrick)
  {
  if ((player_ypos<drawbrick->topy) && (drawbrick->toptex!=NULL))
    {
    set_texture(drawbrick->toptex);
    display_surface(drawbrick->top);
    }

  if ((player_ypos>drawbrick->bottomy) && (drawbrick->bottomtex!=NULL))
    {
    set_texture(drawbrick->bottomtex);
    display_surface(drawbrick->bottom);
    }

  if ((player_xpos<drawbrick->leftx) && (drawbrick->lefttex!=NULL))
    {
    set_texture(drawbrick->lefttex);
    display_surface(drawbrick->left);
    }

  if ((player_xpos>drawbrick->rightx) && (drawbrick->righttex!=NULL))
    {
    set_texture(drawbrick->righttex);
    display_surface(drawbrick->right);
    }

  if ((player_zpos<drawbrick->frontz) && (drawbrick->fronttex!=NULL))
    {
    set_texture(drawbrick->fronttex);
    display_surface(drawbrick->front);
    }

  if ((player_zpos>drawbrick->backz) && (drawbrick->backtex!=NULL))
    {
    set_texture(drawbrick->backtex);
    display_surface(drawbrick->back);
    }
  }

void precalc_view()
  {
  int xb,zb;
  float fxd,fzd;
  int xbm,zbm;
  int xpadd,zpadd;
  int drawindex;
  int outer,inner;
  int temp;
  float tempf;

  float *distance_table;

  printf("precalc view...\n");

  distance_table=(float *) malloc(1024*sizeof(float));

  printf("  step 1:\n");

  drawindex=0;
  for (zb=-16; zb<16; zb++)
    for (xb=-16; xb<16; xb++)
      {
      draworderz[drawindex]=zb;
      draworderx[drawindex]=xb;
      xbm=xb*50+12;
      zbm=zb*50+12;
      distance_table[drawindex]=sqrt((xbm*xbm)+(zbm*zbm));
      drawindex++;
      }

  printf("%d generated\n",drawindex);

  printf("  step 2:\n");

  for (outer=0; outer<1023; outer++)
    for (inner=0; inner<1023; inner++)
      {
      if (distance_table[inner]<distance_table[inner+1])
        {
        temp=draworderx[inner];
        draworderx[inner]=draworderx[inner+1];
        draworderx[inner+1]=temp;

        temp=draworderz[inner];
        draworderz[inner]=draworderz[inner+1];
        draworderz[inner+1]=temp;

        tempf=distance_table[inner];
        distance_table[inner]=distance_table[inner+1];
        distance_table[inner+1]=tempf;
        }
      }
  }

void draw_view()
  {
  int xdif,zdif;
  int check;

  int bricknum;

  for (check=0; check<1024; check++)
    {
    xdif=draworderx[check];
    zdif=draworderz[check];

    bricknum=floormap[player_ztile+zdif][player_xtile+xdif];
    if (bricknum!=255)
      draw_brick(&bricklist[bricknum]);

    bricknum=ceilingmap[player_ztile+zdif][player_xtile+xdif];              
    if (bricknum!=255)
      draw_brick(&bricklist[bricknum]);

    }
  }


void read_mouse_controls()
  {
  get_mouse_status(&xmouse,&ymouse,&button_st);

  if (speed>0)
    speed-=0.2;
  if (speed<0)
    speed+=0.2;

  if (button_st==1)
    speed=4;
  if (button_st==2)
    speed=-4;

  internal_yrot+=(float)(xmouse-160)/20;
  if (internal_yrot<0)
    internal_yrot+=360;
  if (internal_yrot>359)
    internal_yrot-=360;

  yrot=internal_yrot;
  }

main()

  {
  object **obj_ptr;
  __dpmi_free_mem_info meminfo;
  unsigned int freemem,memused;
  struct time t;
  int fps,frames_counted,old_sec,pps;

  clrscr();

  player_xpos=3000;
  player_zpos=3000;
  player_ypos=-50;

  __dpmi_get_free_memory_information(&meminfo);
  freemem=meminfo.largest_available_free_block_in_bytes;

  printf("JAKE - Powered by VELOCITY32 3D Blitter\n");
  printf("EXE: %s, %s\n",__TIME__,__DATE__);
  printf("Copyright (C) Jonathan Thomas 1996-1997\n");
  printf("Freemem: %dK\n",freemem/1024);
  printf("-----------------------------------------\n");

  printf("system startup...\n");

  texture_base.location=(char *) malloc(unpackedsize("textures.pcx"));
  loadpcx("textures.pcx",&texture_base,mypal);

  sincostable2(sint,1,360,SIN);
  sincostable2(cost,1,360,COS);

  create_brick(-80+worldx,-220,-80+worldz,80+worldx,-210,80+worldz,-1,-1,-1,-1,4,-1,HIGH);        /* ceiling */
  create_brick(-120+worldx,-210,-80+worldz,-80+worldx,-200,80+worldz,-1,-1,-1,3,4,-1,HIGH);       /* surround ceiling 1 */
  create_brick(80+worldx,-210,-80+worldz,120+worldx,-200,80+worldz,0,-1,-1,-1,4,-1,HIGH);         /* 2 */
  create_brick(-80+worldx,-210,-120+worldz,80+worldx,-200,-80+worldz,-1,-1,2,-1,4,-1,HIGH);       /* 3 */
  create_brick(-80+worldx,-210,80+worldz,80+worldx,-200,120+worldz,-1,-1,-1,-1,4,5,HIGH);         /* 4 */

  create_brick(-80+worldx,10,-80+worldz,80+worldx,20,80+worldz,-1,1,-1,-1,-1,-1,LOW);        /* center floor */
  create_brick(-120+worldx,0,-80+worldz,-80+worldx,10,80+worldz,-1,1,-1,3,-1,-1,LOW);       /* surround floor 1 */
  create_brick(80+worldx,0,-80+worldz,120+worldx,10,80+worldz,0,1,-1,-1,-1,-1,LOW);         /* 2 */
  create_brick(-80+worldx,0,-120+worldz,80+worldx,10,-80+worldz,-1,1,2,-1,-1,-1,LOW);       /* 3 */
  create_brick(-80+worldx,0,80+worldz,80+worldx,10,120+worldz,-1,1,-1,-1,-1,5,LOW);         /* 4 */

  create_brick(-120+worldx,-200,-120+worldz,-80+worldx,0,-80+worldz,-1,-1,2,3,-1,-1,LOW);   /* pole thang 1 */
  create_brick(80+worldx,-200,-120+worldz,120+worldx,0,-80+worldz,0,-1,2,-1,-1,-1,LOW);     /* 2 */
  create_brick(-120+worldx,-200,80+worldz,-80+worldx,0,120+worldz,-1,-1,-1,3,-1,5,LOW);     /* 3 */
  create_brick(80+worldx,-200,80+worldz,120+worldx,0,120+worldz,0,-1,-1,-1,-1,5,LOW);       /* 4 */

  create_brick(20+worldx,-200,-180+worldz,80+worldx,-100,-120+worldz,-1,-1,2,-1,4,-1,HIGH);   /* above door */
  create_brick(20+worldx,0,-180+worldz,80+worldx,10,-120+worldz,-1,1,-1,-1,-1,-1,LOW);   /* below door */
  create_brick(-80+worldx,-200,-180+worldz,20+worldx,-100,-120+worldz,-1,-1,2,-1,-1,-1,HIGH);    /* rest of wall (top) */
  create_brick(-80+worldx,-100,-180+worldz,20+worldx,0,-120+worldz,-1,-1,2,3,-1,5,LOW);    /* rest of wall (bottom) */
  create_brick(80+worldx,-100,-260+worldz,90+worldx,0,-120+worldz,0,-1,-1,-1,-1,-1,LOW);   /* wall by door */

  create_brick(-80+worldx,-200,120+worldz,80+worldx,0,130+worldz,-1,-1,-1,-1,-1,5,LOW);      /* wall */

  create_brick(120+worldx,-200,-80+worldz,130+worldx,0,80+worldz,0,-1,-1,-1,-1,-1,LOW);      /* wall */
  create_brick(-130+worldx,-200,-80+worldz,-120+worldx,0,80+worldz,-1,-1,-1,3,-1,-1,LOW);      /* wall */

  create_brick(20+worldx,0,-260+worldz,80+worldx,10,-180+worldz,-1,1,-1,-1,-1,-1,LOW); /* outside path (1) */
  create_brick(-80+worldx,0,-260+worldz,20+worldx,10,-180+worldz,-1,1,-1,-1,-1,-1,LOW); /* outside path (2) */
  create_brick(-380+worldx,0,-260+worldz,-80+worldx,10,-180+worldz,-1,1,-1,-1,-1,-1,LOW); /* towards room 2: */
  create_brick(-380+worldx,-150,-260+worldz,-80+worldx,-100,-180+worldz,-1,-1,-1,-1,4,-1,HIGH); /* ceiling */
  create_brick(-380+worldx,-100,-180+worldz,-80+worldx,0,-120+worldz,-1,-1,-1,-1,-1,5,LOW);  /* side wall */
  create_brick(-380+worldx,-100,-340+worldz,-80+worldx,0,-260+worldz,-1,-1,2,3,-1,-1,LOW);  /* other side wall */

  create_brick(-80+worldx,0,-340+worldz,20+worldx,10,-260+worldz,-1,1,-1,-1,-1,5,LOW);
  create_brick(-80+worldx,10,-400+worldz,20+worldx,20,-340+worldz,-1,1,-1,-1,-1,5,LOW); /* 5 steps */
  create_brick(-80+worldx,20,-460+worldz,20+worldx,30,-400+worldz,-1,1,-1,-1,-1,5,LOW);
  create_brick(-80+worldx,30,-520+worldz,20+worldx,40,-460+worldz,-1,1,-1,-1,-1,5,LOW);
  create_brick(-80+worldx,40,-580+worldz,20+worldx,50,-520+worldz,-1,1,-1,-1,-1,-1,LOW);

  create_brick(-80+worldx,-100,-580+worldz,20+worldx,-70,-460+worldz,-1,-1,2,-1,1,-1,HIGH); /* steps ceiling */
  create_brick(-80+worldx,-110,-460+worldz,20+worldx,-100,-260+worldz,-1,-1,-1,-1,1,-1,HIGH); /* steps ceiling 2 */
  create_brick(-80+worldx,-120,-580+worldz,20+worldx,-70,-560+worldz,-1,-1,-1,-1,-1,5,HIGH); /* steps ceiling 3 */   

  create_brick(-90+worldx,-100,-400+worldz,-80+worldx,10,-340+worldz,-1,-1,-1,3,-1,-1,LOW);  /* stairs wall right */
  create_brick(-90+worldx,-100,-460+worldz,-80+worldx,20,-400+worldz,-1,-1,-1,3,-1,-1,LOW);  /* stairs wall right */
  create_brick(-90+worldx,-70,-520+worldz,-80+worldx,30,-460+worldz,-1,-1,-1,3,-1,-1,LOW);  /* stairs wall right */
  create_brick(-90+worldx,-70,-580+worldz,-80+worldx,40,-520+worldz,-1,-1,-1,3,-1,-1,LOW);  /* stairs wall right */

  create_brick(20+worldx,-100,-340+worldz,80+worldx,0,-260+worldz,0,-1,1,-1,-1,-1,LOW);  /* stairs wall left and corner*/
  create_brick(20+worldx,-100,-400+worldz,30+worldx,10,-340+worldz,0,-1,-1,-1,-1,-1,LOW);  /* stairs wall left */
  create_brick(20+worldx,-100,-460+worldz,30+worldx,20,-400+worldz,0,-1,-1,-1,-1,-1,LOW);  /* stairs wall left */
  create_brick(20+worldx,-70,-520+worldz,30+worldx,30,-460+worldz,0,-1,-1,-1,-1,-1,LOW);  /* stairs wall left */
  create_brick(20+worldx,-70,-580+worldz,30+worldx,40,-520+worldz,0,-1,-1,-1,-1,-1,LOW);  /* stairs wall left */

  /* now the complicated room */

  create_brick(-100+worldx,-120,-780+worldz,-80+worldx,40,-760+worldz,0,-1,2,3,-1,5,LOW); /* poles */
  create_brick(20+worldx,-120,-780+worldz,40+worldx,40,-760+worldz,0,-1,2,3,-1,5,LOW);
  create_brick(-100+worldx,-120,-660+worldz,-80+worldx,40,-640+worldz,0,-1,2,3,-1,5,LOW);
  create_brick(20+worldx,-120,-660+worldz,40+worldx,40,-640+worldz,0,-1,2,3,-1,5,LOW);

  create_brick(-80+worldx,40,-780+worldz,20+worldx,350,-760+worldz,-1,1,2,-1,4,-1,LOW); /* edges of pit */
  create_brick(-80+worldx,40,-660+worldz,20+worldx,350,-640+worldz,-1,1,-1,-1,4,5,LOW);
  create_brick(20+worldx,40,-760+worldz,40+worldx,350,-660+worldz,0,1,-1,-1,4,-1,LOW);
  create_brick(-100+worldx,40,-760+worldz,-80+worldx,350,-660+worldz,-1,1,-1,3,4,-1,LOW);

  create_brick(-100+worldx,40,-640+worldz,40+worldx,350,-580+worldz,-1,1,-1,-1,-1,-1,LOW); /* floor (front+back) */
  create_brick(-100+worldx,40,-900+worldz,40+worldx,350,-780+worldz,-1,1,-1,-1,-1,-1,LOW);

  create_brick(-180+worldx,40,-660+worldz,-100+worldx,350,-580+worldz,-1,1,-1,-1,-1,-1,LOW);
  create_brick(-180+worldx,40,-780+worldz,-100+worldx,350,-660+worldz,-1,1,-1,-1,-1,-1,LOW); /* right segments */
  create_brick(-180+worldx,40,-900+worldz,-100+worldx,350,-780+worldz,-1,1,-1,-1,-1,-1,LOW); 

  create_brick(40+worldx,40,-660+worldz,120+worldx,350,-580+worldz,-1,1,-1,-1,-1,-1,LOW);
  create_brick(40+worldx,40,-780+worldz,120+worldx,350,-660+worldz,-1,1,-1,-1,-1,-1,LOW); /* left segments */
  create_brick(40+worldx,40,-900+worldz,120+worldx,350,-780+worldz,-1,1,-1,-1,-1,-1,LOW); 

  /* ceiling */

  create_brick(-80+worldx,-130,-760+worldz,20+worldx,-120,-660+worldz,-1,1,-1,-1,1,-1,HIGH); 

  create_brick(-80+worldx,-130,-780+worldz,20+worldx,-120,-760+worldz,-1,-1,-1,-1,1,-1,HIGH); /* edges of pit */
  create_brick(-80+worldx,-130,-660+worldz,20+worldx,-120,-640+worldz,-1,-1,-1,-1,1,-1,HIGH);
  create_brick(20+worldx,-130,-760+worldz,40+worldx,-120,-660+worldz,-1,-1,-1,-1,1,-1,HIGH);
  create_brick(-100+worldx,-130,-760+worldz,-80+worldx,-120,-660+worldz,-1,-1,-1,-1,1,-1,HIGH);

  create_brick(-100+worldx,-130,-640+worldz,40+worldx,-120,-580+worldz,-1,-1,-1,-1,1,-1,HIGH); /* floor (front+back) */
  create_brick(-100+worldx,-130,-900+worldz,40+worldx,-120,-780+worldz,-1,-1,-1,-1,1,-1,HIGH);

  create_brick(-180+worldx,-130,-660+worldz,-100+worldx,-120,-580+worldz,-1,-1,-1,-1,1,-1,HIGH);
  create_brick(-180+worldx,-130,-780+worldz,-100+worldx,-120,-660+worldz,-1,-1,-1,-1,1,-1,HIGH); /* right segments */
  create_brick(-180+worldx,-130,-900+worldz,-100+worldx,-120,-780+worldz,-1,-1,-1,-1,1,-1,HIGH); 

  create_brick(40+worldx,-130,-660+worldz,120+worldx,-120,-580+worldz,-1,-1,-1,-1,1,-1,HIGH);
  create_brick(40+worldx,-130,-780+worldz,120+worldx,-120,-660+worldz,-1,-1,-1,-1,1,-1,HIGH); /* left segments */
  create_brick(40+worldx,-130,-900+worldz,120+worldx,-120,-780+worldz,-1,-1,-1,-1,1,-1,HIGH); 

  create_brick(-180+worldx,-120,-950+worldz,120+worldx,40,-900+worldz,0,-1,2,3,-1,5,LOW); /* back wall */
  create_brick(-230+worldx,-120,-900+worldz,-180+worldx,40,-580+worldz,0,-1,-1,3,-1,-1,LOW); 
  create_brick(120+worldx,-120,-900+worldz,170+worldx,40,-580+worldz,0,-1,-1,3,-1,-1,LOW); 
  create_brick(20+worldx,-120,-580+worldz,120+worldx,40,-560+worldz,-1,-1,-1,-1,-1,5,HIGH); /* walls by door */   
  create_brick(-180+worldx,-120,-580+worldz,-80+worldx,40,-560+worldz,-1,-1,-1,-1,-1,5,HIGH); /* walls by door */   

  create_brick(-80+worldx,-150,-260+worldz,80+worldx,-100,-180+worldz,-1,-1,-1,-1,4,-1,HIGH); /* ceiling thru first door */

  create_floor_map();
  precalc_view();

  __dpmi_get_free_memory_information(&meminfo);
  printf("free mem: %dK\n",meminfo.largest_available_free_block_in_bytes/1024);

  printf("Press Return...\n");
  getc(stdin);

  opengraphics();

  set_environment(mypal,80,80,80);
  set_screen(screen.location,0,0,319,199);
  generate_filter();
  generate_shade_table();

  showpal(mypal);

  init_mouse(&but);
  loadfont("alpha2.pcx");

  horizon_colour=texshade_table[16383];

  while(!kbhit())
    {
    memset(screen.location,horizon_colour,64000);
    read_mouse_controls();

    player_xpos+=sint[359-yrot]*speed;
    player_zpos-=cost[359-yrot]*speed;
    player_ypos+=sint[xrot]*speed;

    player_xtile=player_xpos/tilesize;
    player_ztile=player_zpos/tilesize;

    xrot=270+ymouse;
    if (xrot>359)
      xrot-=360;

    draw_view();

    if (curpolys>maxpolys)
      maxpolys=curpolys;
     
    gettime(&t);
    if (t.ti_sec!=old_sec)
      {
      fps=frames_counted;
      frames_counted=0;
      pps=curpolys;
      curpolys=0;
      }
    old_sec=t.ti_sec;

    bar(0,0,50,40,0);
    sprintf(buf,"%d",fps);
    write(5,5,buf);
    sprintf(buf,"%d",yrot);
    write(5,15,buf);

    sprintf(buf,"%d",pps);
    write(5,25,buf);
/*    sprintf(buf,"x%d z%d",player_xtile,player_ztile);
    write(5,25,buf);*/

    frames_counted++;
    copyscreenbuffer();
    }

  closegraphics();
  printf("-----------------------------------------\n");
  printf("            JAKE 3D indoor engine\n");
  printf("EXE: %s, %s\n",__TIME__,__DATE__);
  printf("Copyright (C) Jonathan Thomas 1996-1997\n");
  printf("-----------------------------------------\n");
  }
