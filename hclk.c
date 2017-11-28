#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

Display *dis;
int screen;
Window win, root;
GC gc;
unsigned long black, white;

int x11_fd;
fd_set in_fds;
struct timeval tv;

void curtime(int *h, int *m, int *s)
{
	time_t now;
	struct tm *tm;

	time(&now);
	tm = localtime(&now);
	*h = tm->tm_hour;
	*m = tm->tm_min;
	*s = tm->tm_sec;
	//printf("%02d:%02d:%02d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
}

void get_root_size(int *w, int *h)
{
	XWindowAttributes win_attr;
	XGetWindowAttributes(dis, root, &win_attr);
	*w = win_attr.width;
	*h = win_attr.height;
}


void get_window_size(int *w, int *h)
{
	XWindowAttributes win_attr;
	XGetWindowAttributes(dis, win, &win_attr);
	*w = win_attr.width;
	*h = win_attr.height;
}



void draw(Drawable dr, int ww, int wh)
{
	int H, M, S;
	int cw = ww/2, ch = wh/2;
	int i, x, y;
	const float PI = 3.14159265f;
	float alpha = 0.0f;

	XSetForeground(dis, gc, 0);
	for (i = 0; i < 12; ++i) {
		y = (short) ((ch - 50) * sinf( alpha )); y += ch;
		x = (short) ((ch - 50) * cosf( alpha )); x += cw;
		XFillRectangle(dis, dr, gc, x-3, y-3, 6, 6);
		alpha += PI / 6.0f;
	}

	curtime(&H, &M, &S);

	// HOUR
	alpha = 30.0f * (float) H;
	alpha += (float) M / 2.0f;
	alpha = (alpha - 90.0f) * PI / 180.0f;
	x = (short) ((ch / 2.5f) * cosf( alpha )); x += cw;
	y = (short) ((ch / 2.5f) * sinf( alpha )); y += ch;
	XSetLineAttributes(dis, gc, 16, LineSolid, CapRound, JoinRound);
	XDrawLine(dis, dr, gc, cw, ch, x, y);
	x = (short) (20.0f * cosf( alpha + PI )); x += cw;
	y = (short) (20.0f * sinf( alpha + PI )); y += ch;
	XDrawLine(dis, dr, gc, cw, ch, x, y);

	// MINUTE
	alpha = 6.0f * (float) M;
	alpha = (alpha - 90.0f) * PI / 180.0f;
	x = (short) ((ch / 1.4f) * cosf( alpha )); x += cw;
	y = (short) ((ch / 1.4f) * sinf( alpha )); y += ch;
	XSetLineAttributes(dis, gc, 10, LineSolid, CapRound, JoinRound);
	XDrawLine(dis, dr, gc, cw, ch, x, y);
	x = (short) (20.0f * cosf( alpha + PI )); x += cw;
	y = (short) (20.0f * sinf( alpha + PI )); y += ch;
	XDrawLine(dis, dr, gc, cw, ch, x, y);

	// SECOND
	alpha = 6.0f * (float) S;
	alpha = (alpha - 90.0f) * PI / 180.0f;
	x = (short) ((ch / 1.3f) * cosf( alpha )); x += cw;
	y = (short) ((ch / 1.3f) * sinf( alpha )); y += ch;
	XSetLineAttributes(dis, gc, 2, LineSolid, CapRound, JoinRound);
	XDrawLine(dis, dr, gc, cw, ch, x, y);
	x = (short) (20.0f * cosf( alpha + PI )); x += cw;
	y = (short) (20.0f * sinf( alpha + PI )); y += ch;
	XDrawLine(dis, dr, gc, cw, ch, x, y);

	XFlush(dis);
}

void draw_win()
{
	int w, h;
	int depth = DefaultDepth(dis, screen);
	Pixmap pixmap;

	get_window_size(&w, &h);
	pixmap = XCreatePixmap(dis, root, w, h, depth);

	XSetForeground(dis, gc, 0xffffff);
	//XSetBackground(dis, gc, 0xd0a010);
	XFillRectangle(dis, pixmap, gc, 0, 0, w, h);

	draw(pixmap, w, h);

	XCopyArea(dis, pixmap, win, gc, 0, 0, w, h, 0, 0);
	XFreePixmap(dis, pixmap);

	//XDrawImageString(dis, win, gc, 10, 10, "pixmap", 6);
}


void show_window()
{
	int rw, rh;

	XClearWindow(dis, win);
	get_root_size(&rw, &rh);
	XResizeWindow(dis, win, (int) (rw*0.9), (int) (rh*0.9));
	XMapRaised(dis, win);
	XMoveWindow(dis, win, (int) (rw*0.04), (int) (rh*0.04));
}


int main()
{
	XSetWindowAttributes attrs;
	XEvent event, nev;
	int keycode;
	int is_retriggered;
	unsigned modifier = Mod4Mask;

	dis = XOpenDisplay((char *)0);
	if (!dis) {
		fprintf(stderr, "Couldn't open X11 display\n");
		exit(1);
	}
	screen = DefaultScreen(dis);
	black = BlackPixel(dis,screen);
	white = WhitePixel(dis, screen);
	keycode = XKeysymToKeycode(dis,XK_Z);
	root = DefaultRootWindow(dis);

	win = XCreateWindow(dis, root, 0, 0, 100, 100, 0,
		DefaultDepth(dis, screen),
		InputOutput,
		DefaultVisual(dis, screen),
		0UL, &attrs);	// all defaults

	XSetStandardProperties(dis, win, "", "", None, NULL, 0, NULL);
	XSelectInput(dis, win, ExposureMask|KeyPressMask);
	XSelectInput(dis, root, KeyPressMask);

	gc = XCreateGC(dis, win, 0, 0);

	// Win+Z
	XGrabKey(dis, keycode, modifier,          root, False, GrabModeAsync, GrabModeAsync);
	XGrabKey(dis, keycode, modifier|Mod2Mask, root, False, GrabModeAsync, GrabModeAsync);

	while(1) {
		XNextEvent(dis, &event);

		if (event.type==Expose && event.xexpose.count==0) {
			draw_win();
		}

		if (event.type==KeyPress && event.xkey.keycode==keycode) {
			show_window();
		}

		if (event.type==KeyRelease && event.xkey.keycode==keycode) {

			is_retriggered = 0;

			if (XEventsQueued(dis, QueuedAfterReading)) {
				XPeekEvent(dis, &nev);

				if (nev.type == KeyPress && nev.xkey.time == event.xkey.time &&
					nev.xkey.keycode == event.xkey.keycode) {

					// delete retriggered KeyPress event
					XNextEvent(dis, &nev);
					is_retriggered = 1;
					draw_win();
				}
			}

			if ( !is_retriggered ) {
				XUnmapWindow(dis, win);
			}
		}
	}

	XUngrabKey(dis, keycode, modifier,          root);
	XUngrabKey(dis, keycode, modifier|Mod2Mask, root);

	XFreeGC(dis, gc);
	XDestroyWindow(dis, win);
	XCloseDisplay(dis);
	return 0;
}
