#include <dos.h>

#define mouse_int 0x33
#define left_button   1
#define right_button  2
#define middle_button 4

int xdiff;

void mouse(int function_number,
	   int *bx,
	   int *cx,
	   int *dx)
{
	union REGS r;

	r.x.ax=function_number;
	r.x.bx=*bx;
	r.x.cx=*cx;
	r.x.dx=*dx;
	int86(mouse_int, &r, &r);
	*bx = r.x.bx;
	*cx = r.x.cx;
	*dx = r.x.dx;
}

int init_mouse(int *button)
{
	 int *ax;
         int doctor;

         mouse(0,ax,button,&doctor);
	 xdiff=640 /319;
	 return(*ax);
}

void display_mouse(void)
{
        int doctor;

        mouse(1,&doctor,&doctor,&doctor);
}

void hide_mouse(void)
{
        int doctor;
        mouse(2,&doctor,&doctor,&doctor);
}

void get_mouse_status(int *xmouse,int *ymouse,int *button_status)
{
	mouse(3,button_status,xmouse,ymouse);
	*xmouse = *xmouse / xdiff;
}


void set_mouse_pos(int x,int y)
{
        int doctor;

	x = x * xdiff;
        mouse(4,&doctor,&x,&y);
}



void mouse_limits(int xmin,int ymin,int xmax,int ymax)
{
	xmin = xmin*xdiff;
	xmax = xmax*xdiff;
        mouse(7,&xmin,&xmin,&xmax);
        mouse(8,&xmin,&ymin,&ymax);
}

