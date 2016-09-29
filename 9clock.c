/*
  * 9clock.c: a basic graphical clock for plan 9
  * sean caron (scaron@umich.edu)
*/

/*
  * The paradigm here is that we must always include u.h
  * first and libc.h second, and anything else after that.
*/

#include <u.h>
#include <libc.h>
#include <draw.h>
#include <cursor.h>
#include <event.h>

int main(void);
void eresized(int new);

int main(void) {
	Point center, minend, hrend, o_minend, o_hrend;
	Tm *thetime;
	int xdim, ydim, fd;
	double mintheta, hrtheta;
	char *buttons[] = { "quit", 0 };
	Menu menu = { buttons };
	Mouse m;
	Event e;
	ulong timer, evtype;

	initdraw(nil, nil, "9clock");

	/*
	  * initialize event handler for mouse events only. we
	  * must call this subsequent to initdraw().
	*/

	einit(Emouse);

	/* we need to call this resize handler at least once */

	eresized(0);

	/*
	  * we need to set up this timer event because when you use
	  * the event() system call, it basically blocks until some sort
	  * of event occurs. we use the timer event in lieu of a real
	  * event occurring or else the clock would not update until
	  * the user moved the mouse or clicked a button.
	*/

	timer = etimer(0,10);

	/*
	  * initdraw() sets up some global variables,
	  *
	  * Display *display
	  * Image *screen
	  * Screen *_screen
	  * Font *font
	  *
	  * ZP is a constant point (0,0).
	*/

	/* resize the current window. */

	fd = open("/dev/wctl", OWRITE);

	fprint(fd, "resize -dx 200 -dy 200");

	close(fd);

	getwindow(display, Refnone);

	/* initialize some variables. */

	xdim = (screen->r.max.x - screen->r.min.x) / 2;
	ydim = (screen->r.max.y - screen->r.min.y) / 2;

	center.x = screen->r.min.x + xdim;
	center.y = screen->r.min.y + ydim;

	/* main program loop */

	while (1) {

		evtype = event(&e);

		/* look for mouse events */

		if (evtype == Emouse) {

			m = e.mouse;

			/* if left mouse button is clicked, show the menu */

			if (m.buttons == 4) {
			
				/* quit */

				if (emenuhit(3, &m, &menu) == 0) {
					closedisplay(display);
					exits(0);
				}

			}
		}

		/* use the faux timer event to update the clock if there is no real input */

		else if (event(&e) == timer) {

			/* draw outline of clock face. */
	
			ellipse(screen, center, xdim, ydim, 2, display->black, center);

			thetime = localtime(time(nil));

			/* correct 24 hour system time to 12 hour clock. */

			if (thetime->hour > 12) {
				thetime->hour = thetime->hour - 12;
			}

			/*
			  * 6 degrees = 0.104718 rads per minute on the clock face.
			  * 30 degrees = 0.523590 rads per hour on the clock face.
			  *
			  * we subtract the value from PI/2 to correct between standard
			  * form and clock form.
			*/

			mintheta = (PI/2.0) - (0.104718 * (double)thetime->min);
			hrtheta = (PI/2.0) - (0.523590 * (double)thetime->hour);

			/*
			  * calculate the endpoints for the minute and hour hands.
			  * we resize the window to be square so xdim and ydim are actually the same.
			  * we scale down the hour hand to make it shorter than the minute hand.
			*/

			minend.x = center.x + (double)xdim*cos(mintheta);
			minend.y = center.y - (double)ydim*sin(mintheta);

			hrend.x = center.x + (double)xdim*cos(hrtheta);
			hrend.y = center.y - (double)ydim*sin(hrtheta);

			/*
			  * if the time has changed since the last time we looped, get rid of the stale minute or
			  * hour hands.
			*/

			if ( (o_minend.x != minend.x) || (o_minend.y != minend.y) ) {
				line(screen, center, o_minend, Endsquare, Endarrow, 1, display->white, center);
			}

			if ( (o_hrend.x != hrend.x) || (o_hrend.y != hrend.y) ) {
				line(screen, center, o_hrend, Endsquare, Endarrow, 2, display->white, center);
			}

			/* draw the current minute and hour hands. */

			line(screen, center, minend, Endsquare, Endarrow, 1, display->black, center);

			line(screen, center, hrend, Endsquare, Endarrow, 2, display->black, center);

			/* save the minute and hour hand coordinates for comparison the next time around. */

			o_minend = minend;
			o_hrend = hrend;

			/* let some other processes get a word in edgewise. */

			sleep(1);

			/*
			  * the draw (2) manpage suggests that this is not required, but the graphics will not
			  * show up unless we use it.
			*/

			flushimage(display,1);
		}
	}	
}

/*
  * by convention we must declare an eresized() function to
  * handle the window resizing event.
*/

void eresized(int new) {

	/* just refresh the window for now. */

	getwindow(display, Refnone);
}