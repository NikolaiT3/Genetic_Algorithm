//*****************************************************************************
// Team 7: Adam Guy, Nabir Dinani, Jonathan Kocmoud, Nicholas Warner 
// Date : 4 April 2015
// Subject : CSCE 315-504
// Assignment : Project 3
// Description : Graphically displays GA circuit search progress
//*****************************************************************************
/*
compile with: g++-4.7 $(fltk-config --use-images --compile) -std=c++11 
lineGraph.cpp ga.cpp FLTK_Hist.h -lfltk -lX11
*/

//Headers for graphical part using FLTK
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

//Headers for accessing necessary functinoalities 
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for fork, pipe, read, write
#include <sys/wait.h> // for wait() sys call
#include <time.h>

using namespace std;

#define BG_COLOR FL_WHITE
#define MIS_COLOR FL_GREEN
#define FIT_COLOR FL_BLACK
#define POP_COLOR FL_RED

static int pipefd;
static long FITNESS = 0;
static long MISSING_BITS = 0;
static long POPULATION = 0;

class lGraph : public Fl_Box {

	//overloading the virtual draw function, and making it do what we want 
    void draw() {
        // TELL BASE WIDGET TO DRAW ITS BACKGROUND
        Fl_Box::draw();
		
		int pointSpacing = 20,
			yAxis = (int) (x() + 80),
			xAxis = (int) (y() + 80),
			pointMaxH = 500;
		
		string s1, s2, s3;
		stringstream ss1, ss2, ss3;
		static deque<long> fit, mis, pop;
		
		long maxPop =1,
			maxFit =1,
			maxMis =1;

		// update data deques
		fit.push_back(FITNESS); 
		mis.push_back(MISSING_BITS);
		pop.push_back(POPULATION); 
		
		// keep only the most recent 25 generations
		if (fit.size() > 25) {
			fit.pop_front();
			pop.pop_front();
			mis.pop_front();
		}

		// find current largest FITNESS in the deque
		for (int i = 0; i < fit.size(); i++) {
			if (fit[i] > maxFit) maxFit = fit[i];
			if (pop[i] > maxPop) maxPop = pop[i];
			if (mis[i] > maxMis) maxMis = mis[i];
		}

		// display key and max values
		fl_font(FL_HELVETICA, 10);
		fl_color(FIT_COLOR);
		ss1 << maxFit;
		ss1 >> s1;
		fl_draw(s1.c_str(), 30, h(), 30, 200, FL_ALIGN_TOP);
		fl_draw("Current Scale", 30, h()-30, 30, 200, FL_ALIGN_TOP);
		fl_draw("AVERAGE FITNESS", yAxis-50, xAxis-50, 200, 15, FL_ALIGN_TOP);
		fl_color(MIS_COLOR);
		ss2 << maxMis;
		ss2 >> s2;
		fl_draw(s2.c_str(), 30, h()-10, 30, 200, FL_ALIGN_TOP);
		fl_draw("AVERAGE MISSING BITS", yAxis+150, xAxis-50, 200, 15, FL_ALIGN_TOP);
		fl_color(POP_COLOR);
		ss3 << maxPop;
		ss3 >> s3;
		fl_draw(s3.c_str(), 30, h()-20, 30, 200, FL_ALIGN_TOP);
		fl_draw("POPULATION", yAxis+300, xAxis-50, 200, 15, FL_ALIGN_TOP);
		
		// draw and label axises
		fl_color(FIT_COLOR);
		fl_font(FL_HELVETICA, 26);
		fl_draw("CIRCUIT GENERATIONS", yAxis+50, xAxis-30, 200, 15, FL_ALIGN_TOP);
		fl_line(yAxis, xAxis, w(), xAxis);
		fl_line(yAxis, xAxis, yAxis , h());
			
		if (fit.size() >1) { // can't draw lines with one point
			int x1 =0,
				x2 =0;
			fl_line_style(FL_SOLID);
			// draw lines on the graph
			for (int i = 0; i < fit.size()-1; i++) {
				x1 = yAxis + i*pointSpacing;
				x2 = x1 + pointSpacing;
				fl_color(FIT_COLOR);
				fl_line(x1, xAxis+(pointMaxH*fit[i]/maxFit), x2, xAxis+(pointMaxH*fit[i+1]/maxFit));
				fl_color(MIS_COLOR);
				fl_line(x1, xAxis+(pointMaxH*mis[i]/maxMis), x2, xAxis+(pointMaxH*mis[i+1]/maxMis));
				fl_color(POP_COLOR);
				fl_line(x1, xAxis+(pointMaxH*pop[i]/maxPop), x2, xAxis+(pointMaxH*pop[i+1]/maxPop));
				
			} // end for loop
		} // end if
    }

    static void Timer_CB(void *userdata) {
		
		lGraph *o = (lGraph*) userdata;
		static int readCheck, loopStop=0;
        readCheck =0;
		
		readCheck += read(pipefd, &POPULATION, sizeof(POPULATION));
		readCheck += read(pipefd, &FITNESS, sizeof(FITNESS));
		readCheck += read(pipefd, &MISSING_BITS, sizeof(MISSING_BITS));
		
		if ((POPULATION == 0 && MISSING_BITS == 0 && FITNESS ==0) ||
			(readCheck == 0)){
				loopStop++;
		}
		if (loopStop > 2){ // stop loop if no new data is read 
			Fl::remove_timeout(Timer_CB, userdata);
		}
		else{
			o->redraw();
			Fl::repeat_timeout(20, Timer_CB, userdata);
		}
    }
public:
    // CONSTRUCTOR
    lGraph(int X,int Y,int W,int H,const char*L=0) : Fl_Box(X,Y,W,H,L) {
        color(FL_BLACK);
		box(FL_FLAT_BOX);
        color(BG_COLOR);
        Fl::add_timeout(60, Timer_CB, (void*)this); // runs Timer_CB
    }
};

int graph() {
     
     Fl_Double_Window win(625, 625);
     lGraph line_graph(10, 10, win.w()-20, win.h()-20);
     win.show();
     return(Fl::run());
}
    

