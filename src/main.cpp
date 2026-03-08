#include "calc.h"
//#include "input.h"

#include "options.h"

#ifndef PC
	APP_NAME("MineSnail")
	APP_DESCRIPTION("Minesweeper, use keys 3 to 9 or '<-' to start a new the game. github.com/SnailMath/CPMineSnail")
	APP_AUTHOR("SnailMath")
	APP_VERSION("1.0.0")
	#include <string.h>
	#include <sdk/os/input.h>
//#include <sdk/os/input.h>
#endif

#define VIEWSTEP 4 // 2^4=16 steps
#define PVIEW 16   // 2^4=16 steps
int scale=30; //32;//the current zoom level
int viewx=0;//the part of the field you see in the middle of the screen.
int viewy=0;
uint32_t randomInt;
int sw=9;//10; //the selected game size
int sh=9;//10;
int nrnotopen; //the number of fields the player needs to open to win ((sw*sh)-bamount)
bool stopped=false; //false if the game is running, true if lost or won
bool won=false; //if stopped=true and won=true the player won, if stopped=true and won=false the player has lost.
int bamount = 10;
int flagamount = 0;
int cmode = 1; //the mode (1 click open, 2 set flag, 3 set questionmark)
#define  BOMB   0b10000000
#define  FLAG   0b01000000
#define  OPEN   0b00100000
#define  QUEST  0b00010000
#define BAMOUNT 0b00001111
#define CLICKBOMB 15 //amount gets overwritten with this value when the bomb is clicked (to display the clicked bomb in red)
#define WRONGBOMB 14 //amount gets overwritten with this value when the game is lost and there are wrong flags.
const int fw=64; //the maximum game size
const int fh=64;
uint8_t field[fw*fh];
#define BARHEIGHT 50
void clearField();
void drawField();
void drawTile(int x, int y, uint8_t width, uint8_t flag);
void openTile(int x, int y, uint8_t width, uint8_t amount);
void showBomb(int x, int y, uint8_t w, uint8_t state);
uint32_t nextRand();
uint8_t  randUpTo(uint8_t max);
void generateField();
void rect(int x,int y,int w,int h, uint16_t c);
uint8_t cb(int x, int y); //CountBombs - return if this field is a bomb or not, returns 0 if field does not exist.
uint8_t co(int x, int y); //CheckOpen  - checks if this field is already open,  returns 0 if field does not exist
bool openField(int x, int y); //open this field and the neighbours.
void drawNumber(int x, int y, int w, uint8_t n);
void rect2(int x1, int y1, int x2, int y2, uint16_t c);
void revealBombs();
bool mouseToClick(int mousex, int mousey, uint8_t action); //converts coords and opens fields; returns false if ok and true if a bomb was clicked
void printnumber(int x, int y, int w, int l, int s, int number, uint16_t c); //x,y: coords, w: line width, l: line length, number: the number, s: space between digits
void printdigit(int x, int y, int w, int l, uint8_t digit, uint16_t c); //x,y: coords, w: line width, l: line length, digit: the digit
void printsegments(int x, int y, int w, int l, uint8_t segments, uint16_t c); //x,y: coords, w: line width, l: line length, segments: the segments, c: foreground color
void logo(int x,int y,int s,uint16_t c);

void clearField(){
	int i = 0;
	while (i<fw*fh) {
		field[i]=0;
		i++;
	}
}
void drawField(){
	int posx= (-(scale*viewx)>>VIEWSTEP) +(width/2);
	int posy= (-(scale*viewy)>>VIEWSTEP) +(height/2);

	int y = 0;
	while (y<sh){
		int x = 0;
		while (x<sw) {
			int thisx = posx + x*scale;
			int thisy = posy + y*scale;
			if(thisx<width-0 && thisy<height-0 && thisx+scale>0 && thisy+scale>BARHEIGHT){ //only draw when visible
				//test what kind of square this is (opened or not)
				uint8_t thisfield = field[fw*y+x];
				if(!(thisfield&OPEN)) {
					//closed tile
					drawTile(thisx,thisy,scale,thisfield); //x-pos, y-pos, width
				}else{
					//open tile
					if((thisfield&BOMB)) { // if it is a bomb
						showBomb(thisx,thisy,scale,thisfield&BAMOUNT);
					}else{
						openTile(thisx,thisy,scale,thisfield&BAMOUNT);
					}
				}
			}
			x++;
		}
		y++;
	}
	//border
	int leftx  = posx + 0*scale-1;
	int topy   = posy + 0*scale-1;
	int rightx = posx + sw*scale;
	int bottomy= posy + sh*scale;
	if(topy   > 0    ){ int x = leftx>0?leftx:0; int max = rightx >width ?width :rightx ; while(x<max) setPixel(x++   ,topy   ,color(128,128,128)); }
	if(leftx  > 0    ){ int y = topy >0?topy :0; int max = bottomy>height?height:bottomy; while(y<max) setPixel(leftx ,y++    ,color(128,128,128)); }
	if(bottomy<height){ int x = leftx>0?leftx:0; int max = rightx >width ?width :rightx ; while(x<max) setPixel(x++   ,bottomy,color(128,128,128)); }
	if(rightx <width ){ int y = topy >0?topy :0; int max = bottomy>height?height:bottomy; while(y<max) setPixel(rightx,y++    ,color(128,128,128)); }

	//menu bar
	if(stopped){
		if(won){//the player has won
			rect2(0,0,width,BARHEIGHT,color(192,255,192));
		}else{//the player has lost
			rect2(0,0,width,BARHEIGHT,color(255,192,192));
		}
	}else{//the game is running
		rect2(0,0,width,BARHEIGHT,color(192,192,192));
	}
	rect2(0,BARHEIGHT,width,BARHEIGHT+2,color(128,128,128));
	rect(width-30-10,10,30,30,color(192,192,192));
	switch (cmode){
		case 1:
			drawTile(width-30-10,10,30,0);
			break;
		case 2:
			drawTile(width-30-10,10,30,FLAG);
			break;
		case 3:
			drawTile(width-30-10,10,30,QUEST);
			break;
	}

	rect(5,5,67,38,color(64,64,64));
	printnumber(10,10,5,11,20,bamount-flagamount,color(192,0,0));
	
	//printnumber(150,10,5,11,20,nrnotopen,color(192,0,0),color(100,64,64));

	//rect(80,13,162,24,0);
	logo(83,13,2,color(64,64,64));
	
	//crosshair
	//line(width/2-10,height/2-10,width/2+10,height/2+10,color(0,0,0));
	//line(width/2-10,height/2+10,width/2+10,height/2-10,color(0,0,0));
}
void logo(int x,int y,int s,uint16_t c){
	//M
	rect(  0*s+x,  0*s+y,  2*s, 12*s, c);
	rect(  2*s+x,  2*s+y,  2*s,  2*s, c);
	rect(  4*s+x,  4*s+y,  2*s,  2*s, c);
	rect(  6*s+x,  2*s+y,  2*s,  2*s, c);
	rect(  8*s+x,  0*s+y,  2*s, 12*s, c);
	x+= 12*s;
	//i
	rect(  0*s+x,  0*s+y,  2*s,  2*s, c);
	rect(  0*s+x,  4*s+y,  2*s,  8*s, c);
	x+= 4*s;
	//n
	rect(  0*s+x,  4*s+y,  2*s,  8*s, c);
	rect(  2*s+x,  4*s+y,  3*s,  2*s, c);
	rect(  5*s+x,  4*s+y,  2*s,  8*s, c);
	x+= 9*s;
	//e
	rect(  0*s+x,  4*s+y,  2*s,  8*s, c);
	rect(  2*s+x,  7*s+y,  1*s,  2*s, c);
	rect(  2*s+x, 10*s+y,  3*s,  2*s, c);
	rect(  2*s+x,  4*s+y,  3*s,  2*s, c);
	rect(  3*s+x,  4*s+y,  2*s,  5*s, c);
	x+= 7*s;
	/*
	//s
	rect(  0*s+x,  4*s+y,  2*s,  5*s, c);
	rect(  2*s+x,  4*s+y,  3*s,  2*s, c);
	rect(  0*s+x,  7*s+y,  5*s,  2*s, c);
	rect(  3*s+x,  9*s+y,  2*s,  1*s, c);
	rect(  0*s+x, 10*s+y,  5*s,  2*s, c);
	x+= 7*s;
	*/
	//SPACE
	x+= 4*s;
	//S
	rect(  0*s+x,  0*s+y,  6*s,  2*s, c);
	rect(  0*s+x,  2*s+y,  2*s,  3*s, c);
	rect(  0*s+x,  5*s+y,  6*s,  2*s, c);
	rect(  4*s+x,  7*s+y,  2*s,  3*s, c);
	rect(  0*s+x, 10*s+y,  6*s,  2*s, c);
	x+= 8*s;
	//n
	rect(  0*s+x,  4*s+y,  2*s,  8*s, c);
	rect(  2*s+x,  4*s+y,  3*s,  2*s, c);
	rect(  5*s+x,  4*s+y,  2*s,  8*s, c);
	x+= 9*s;
	//a
	rect(  0*s+x,  4*s+y,  5*s,  2*s, c);
	rect(  3*s+x,  6*s+y,  2*s,  6*s, c);
	rect(  0*s+x,  7*s+y,  2*s,  5*s, c);
	rect(  2*s+x,  7*s+y,  1*s,  2*s, c);
	rect(  2*s+x, 10*s+y,  1*s,  2*s, c);
	x+= 7*s;
	//i
	rect(  0*s+x,  0*s+y,  2*s,  2*s, c);
	rect(  0*s+x,  4*s+y,  2*s,  8*s, c);
	x+= 4*s;
	//l
	rect(  0*s+x,  0*s+y,  2*s, 12*s, c);
	x+= 4*s;
	//SPACE
	x+= 4*s;
	//1
	rect(  0*s+x,  2*s+y,  2*s,  2*s, c);
	rect(  2*s+x,  0*s+y,  2*s, 10*s, c);
	rect(  1*s+x, 10*s+y,  4*s,  2*s, c);
	x+= 7*s;
	//.
	rect(  0*s+x, 10*s+y,  2*s,  2*s, c);
	x+= 4*s;
	//0
	rect(  2*s+x,  0*s+y,  4*s,  2*s, c);
	rect(  0*s+x,  2*s+y,  2*s,  8*s, c);
	rect(  2*s+x, 10*s+y,  4*s,  2*s, c);
	rect(  6*s+x,  2*s+y,  2*s,  8*s, c);
	//x+=10*s;
	//rect(  0*s+x,  4*s+y,  0*s,  0*s, c);

}
void drawTile(int x, int y, uint8_t width, uint8_t thisfield){
	// x, y		position of the tile
	// width	the width and height of this tile
	// thisfield	info about this tile (flag or question mark)
	const uint16_t tlcolor = color(255,255,255);
	const uint16_t brcolor = color(128,128,128);

	//calculate border width at current tile width:
	int border=(width)>>3; // >>3 means divide by 8 (because 2^3=8)
	if (border==0) border=1;
	
	int i=0;
	//first 'border' lines are white with i dark at the end.
	while (i<border){
		int j=0; while (j<width-i) {
			setPixel(x+j,y+i, tlcolor);
			j++;
		}
		j=0; while (j<i) {
			setPixel(x+width-1-j,y+i, brcolor );
			j++;
		}
		i++;
	}
	//The next lines are white on the left, grey in the midle and dark on the right.
	//don't change i
	while (i<width-border){
		int j=0; 
		while (j<border) {
			setPixel(x+j,y+i, tlcolor );
			j++;
		}
		/*while (j<width-border) {
			setPixel(x+j,y+i, flag?0:color(192,192,192) );
			j++;
		}*/ 
		j=width-border;

		while (j<width) {
			setPixel(x+j,y+i, brcolor );
			j++;
		}
		i++;
	}	
	//Now the border at the bottom of the tile, similar to the top border.
	i=0;
	while (i<border){
		int j=0; while (j<i+1) {
			setPixel(x+j,y+width-1-i, tlcolor );
			j++;
		}
		while (j<width) {
			setPixel(x+j,y+width-1-i, brcolor );
			j++;
		}
		i++;
	}
	//Now add the flag or the questionmark if there is one...
	int w = width;
	//int w0  = 0;
	int w1  = (w* 1) >> 4;
	int w2 	 = (w* 2) >> 4;
	int w3 	 = (w* 3) >> 4;
	int w4  = (w* 4) >> 4;
	int w5  = (w* 5) >> 4;
	int w6  = (w* 6) >> 4;
	int w7  = (w* 7) >> 4;
	int w8  = (w* 8) >> 4;
	int w9  = (w* 9) >> 4;
	int w10 = (w*10) >> 4;
	int w11 = (w*11) >> 4;
	int w12 = (w*12) >> 4;
	int w13 = (w*13) >> 4;
	//int w14 = (w*14) >> 4;
	//int w15 = (w*15) >> 4;

	if (thisfield&FLAG){	
		rect(x+w4,y+w10,w8,w2,0);
		rect(x+w6,y+w9,w4,w2,0);
		rect(x+w8,y+w6,w1,w5,0);
		triangle(x+w9,y+w7,x+w9,y+w2,x+w4,y+w5,color(255,0,0),color(255,0,0));
		if((thisfield&BAMOUNT)==WRONGBOMB){
			triangle(x+w1,y,x,y+w1,x+w-w1,y+w,    color(200,0,0),color(200,0,0));
			triangle(x+w1,y,x+w-w1,y+w,x+w,y+w-w1,color(200,0,0),color(200,0,0));
			triangle(x+w1,y+w,x,y+w-w1,x+w,y+w1,color(200,0,0),color(200,0,0));
			triangle(x,y+w-w1,x+w,y+w1,x+w-w1,y,color(200,0,0),color(200,0,0));
		}
	} else if (thisfield&QUEST){
		rect2(x+w4 ,y+w4 ,x+w7 ,y+w6 ,0);
		rect2(x+w5 ,y+w3 ,x+w12,y+w5 ,0);
		rect2(x+w10,y+w4 ,x+w13,y+w6 ,0);
		rect2(x+w9 ,y+w6 ,x+w13,y+w7 ,0);
		rect2(x+w8 ,y+w7 ,x+w12,y+w8 ,0);
		rect2(x+w7 ,y+w8 ,x+w11,y+w9 ,0);
		rect2(x+w7 ,y+w9 ,x+w10,y+w10,0);
		rect2(x+w7 ,y+w11,x+w10,y+w13,0);
	}
	
}
void openTile(int x, int y, uint8_t width, uint8_t amount){
	const uint16_t bocolor = color(128,128,128);
	drawNumber(x, y, width, amount);

	int i=0;
	while (i<width){
		setPixel(x+i,y, bocolor );	
		i++;
	}/*
	i=0;
	while (i<width){ //note: this overlaps 1px with the next tile.
		setPixel(x+width,y+i, bocolor );	
		i++;
	}
	i=0;
	while (i<width){//note: this overlaps 1px with the next tile.	
		setPixel(x+i,y+width, bocolor );	
		i++;
	}*/
	i=0;
	while (i<width){
		setPixel(x,y+i, bocolor );	
		i++;
	}
}
void drawNumber(int x, int y, int w, uint8_t n){

	const uint16_t c[]={
		color(192,192,192), //0
		color(  0,  0,255), //1
		color(  0,128,  0), //2
		color(255,  0,  0), //3
		color(  0,  0,128), //4
		color(128,  0,  0), //5
		color(  0,128,128), //6
		color(  0,  0,  0), //7
		color(128,128,128)};//8

	//int w0  = 0;
	//int w1  = (w* 1) >> 4;
	//int w2  = (w* 2) >> 4;
	int w3  = (w* 3) >> 4;
	int w4  = (w* 4) >> 4;
	int w5  = (w* 5) >> 4;
	int w6  = (w* 6) >> 4;
	int w7  = (w* 7) >> 4;
	int w8  = (w* 8) >> 4;
	int w9  = (w* 9) >> 4;
	int w10 = (w*10) >> 4;
	int w11 = (w*11) >> 4;
	int w12 = (w*12) >> 4;
	int w13 = (w*13) >> 4;
	int w14 = (w*14) >> 4;
	//int w15 = (w*15) >> 4;

	if (n==1){
		rect2(x+w5 ,y+w11,x+w12 ,y+w13 ,c[n]);
		rect2(x+w7 ,y+w4 ,x+w10 ,y+w11 ,c[n]);
		rect2(x+w8 ,y+w3 ,x+w10 ,y+w4 ,c[n]);
		rect2(x+w6 ,y+w5 ,x+w7 ,y+w7 ,c[n]);
		rect2(x+w5 ,y+w6 ,x+w6 ,y+w7 ,c[n]);
	} else if (n==2) {
		rect2(x+w3 ,y+w4 ,x+w6 ,y+w6 ,c[n]);
		rect2(x+w4 ,y+w3 ,x+w12,y+w5 ,c[n]);
		rect2(x+w10,y+w4 ,x+w13,y+w7 ,c[n]);
		rect2(x+w8 ,y+w7 ,x+w12,y+w8 ,c[n]);
		rect2(x+w6 ,y+w8 ,x+w11,y+w9 ,c[n]);
		rect2(x+w4 ,y+w9 ,x+w9 ,y+w10,c[n]);
		rect2(x+w3 ,y+w10,x+w7 ,y+w11,c[n]);
		rect2(x+w3 ,y+w11,x+w13,y+w13,c[n]);
	} else if (n==3) {
		rect2(x+w3 ,y+w3 ,x+w12,y+w5 ,c[n]);
		rect2(x+w10,y+w4 ,x+w13,y+w7 ,c[n]);
		rect2(x+w6 ,y+w7 ,x+w12,y+w9 ,c[n]);
		rect2(x+w10,y+w9 ,x+w13,y+w12,c[n]);
		rect2(x+w3 ,y+w11,x+w12,y+w13,c[n]);
	} else if (n==4) {
		rect2(x+w5 ,y+w3 ,x+w8 ,y+w5 ,c[n]);
		rect2(x+w4 ,y+w5 ,x+w7 ,y+w7 ,c[n]);
		rect2(x+w3 ,y+w7 ,x+w14,y+w9 ,c[n]);
		rect2(x+w10,y+w3 ,x+w13,y+w13,c[n]);
	} else if (n==5) {
		rect2(x+w3 ,y+w3 ,x+w13,y+w5 ,c[n]);
		rect2(x+w3 ,y+w5 ,x+w6 ,y+w7 ,c[n]);
		rect2(x+w3 ,y+w7 ,x+w12,y+w9 ,c[n]);
		rect2(x+w10,y+w8 ,x+w13,y+w12,c[n]);
		rect2(x+w3 ,y+w11,x+w12,y+w13,c[n]);
	} else if (n==6) {
		rect2(x+w4 ,y+w3 ,x+w13,y+w5 ,c[n]);
		rect2(x+w3 ,y+w4 ,x+w6 ,y+w12,c[n]);
		rect2(x+w4 ,y+w11,x+w12,y+w13,c[n]);
		rect2(x+w10,y+w8 ,x+w13,y+w12,c[n]);
		rect2(x+w6 ,y+w7 ,x+w12,y+w9 ,c[n]);
	} else if (n==7) {
		rect2(x+w4 ,y+w3 ,x+w13,y+w5 ,c[n]);
		rect2(x+w10,y+w5 ,x+w13,y+w6 ,c[n]);
		rect2(x+w9,y+w6 ,x+w13,y+w7 ,c[n]);
		rect2(x+w8 ,y+w7 ,x+w12,y+w8 ,c[n]);
		rect2(x+w7 ,y+w8 ,x+w11,y+w9 ,c[n]);
		rect2(x+w7 ,y+w9 ,x+w10,y+w13,c[n]);
	} else if (n==8) {
		rect2(x+w3 ,y+w4 ,x+w6 ,y+w12,c[n]);
		rect2(x+w11,y+w4 ,x+w14,y+w12,c[n]);
		rect2(x+w4 ,y+w3 ,x+w13,y+w5 ,c[n]);
		rect2(x+w6 ,y+w7 ,x+w11,y+w9 ,c[n]);
		rect2(x+w4 ,y+w11,x+w13,y+w13,c[n]);
	}
}
void rect2(int x1, int y1, int x2, int y2, uint16_t c){
	int y=y1;
	while(y<y2){
		int x=x1;
		while(x<x2){
			setPixel(x,y,c);
			x++;
		}
		y++;
	}
}
void showBomb(int x, int y, uint8_t w, uint8_t state){
	if(state==CLICKBOMB){
		rect2(x,y,x+w,y+w,color(255,0,0));
	}
	openTile(x, y, w, 0);

	int w0  = w>>5;
	int w1  = w>>4;  
	int w2  = w>>3;  
	//int w3  = (w*3)>>4; 
	int w4  = w>>2;  
	int w5  = (w*5)>>4;
	int w6  = (w*3)>>3;
	//int w7  = (w*7)>>4;
	int w8  = w>>1;  
	//int w9  = (w*9)>>4;
	int w10 = (w*5)>>3;
	int w11 = (w*11)>>4;
	//int w12 = (w*6)>>3;
	//int w13 = 
	//int w16 = w;
	//int w14 = w16-w2;
	//int w15 = w16-w1;
	//the lines
	rect(x+w2+w0 ,y+w8-w0 ,w11 ,w1 ,0);
	rect(x+w8-w0 ,y+w2+w0 ,w1 ,w11 ,0);
	//the circle
	rect(x+w6 ,y+w4 ,w4 ,w2 ,0);
	rect(x+w6 ,y+w10,w4 ,w2 ,0);
	rect(x+w4 ,y+w6 ,w2 ,w4 ,0);
	rect(x+w10,y+w6 ,w2 ,w4 ,0);
	rect(x+w5 ,y+w5 ,w6 ,w6 ,0);
	//the reflection
	rect(x+w6 ,y+w6 ,w2 ,w2 ,0xffff);
	//the corners
	rect(x+w4 ,y+w4 ,w1 ,w1 ,0);
	rect(x+w11,y+w4 ,w1 ,w1 ,0);
	rect(x+w4 ,y+w11,w1 ,w1 ,0);
	rect(x+w11,y+w11,w1 ,w1 ,0);

	
}
void revealBombs(){
	int y = 0;
	while (y<sh){
		int x = 0;
		while (x<sw){
			uint8_t* f = &(field[fw*y+x]);
			//show all bombs that where not found
			if ( (*f & BOMB) && !(*f & FLAG)){
				*f |= OPEN;
			}
			//cross out all flags where there is no bomb
			if ((*f & FLAG) && !(*f & BOMB)){
				*f = (*f & (~BAMOUNT)) | WRONGBOMB ;
			}
				//leafe all bombs when there is a flag there as is.
			x++;
		}
		y++;
	}	
}
void generateField(){
	//place bombs
	int i = 0;
	while (i<bamount){
		int x = randUpTo(sw-1); // the left column is 0, the right one is sw-1, that makes sw columns.
		int y = randUpTo(sh-1);
		if (  !(field[fw*y+x] &  BOMB)) {
			field[fw*y+x] |= BOMB;
			//field[fw*y+x] |= OPEN;
			i++;
		}
	}
	//calculate numbers
	int y = 0;
	while (y<sh){
		int x = 0;
		while (x<sw){
			//calculate the neighbours for every piece 
			field[fw*y+x]|=
				cb(x-1,y-1)+
				cb(x  ,y-1)+
				cb(x+1,y-1)+
				cb(x+1,y  )+
				cb(x+1,y+1)+
				cb(x ,y+1)+
				cb(x-1,y+1)+
				cb(x-1,y  );
			x++;
		}
		y++;
	}
#ifdef SYMBOLTEST
	//test
	field[fw*0+0]=OPEN|1;
	field[fw*0+1]=OPEN|2;
	field[fw*0+2]=OPEN|3;
	field[fw*1+0]=OPEN|4;
	field[fw*1+1]=OPEN|5;
	field[fw*1+2]=OPEN|6;
	field[fw*2+0]=OPEN|7;
	field[fw*2+1]=OPEN|8;
	field[fw*2+2]=OPEN|0;
	field[fw*3+0]=QUEST;
	field[fw*3+1]=OPEN|BOMB;
	field[fw*3+2]=FLAG;
#endif

}
uint8_t cb(int x, int y){ //CountBombs - return if this field is a bomb or not, returns 0 if field does not exist.
	if(x>=0&&y>=0&&x<fw&&y<fh) return field[fw*y+x]&BOMB?1:0;
	return 0;
}
uint8_t co(int x, int y){ //CheckOpen  - checks if this field is already open,  returns 0 if field does not exist
	if(x>=0&&y>=0&&x<fw&&y<fh) return field[fw*y+x]&OPEN?1:0;
	return 0;
}
bool openField(int x, int y){ //open this field and the neighbours.
	if(x<0||y<0||x>=sw||y>=sh)
		return false;
	uint8_t thisfield = field[fw*y+x];
	if( (!(thisfield&OPEN)) && (!(thisfield&FLAG)) ) { //if this field is still closed and there is no flag on it
		field[fw*y+x]|=OPEN;
		if(thisfield&BOMB){
			//we hit a bomb
			return true;
		}else{
			nrnotopen--; //decrement the nr of fields the player has to open
			if((thisfield&BAMOUNT)==0){
			//if this field has no number on it (aka number 0) open the neighbouring fields as well.
				openField(x-1,y-1);
				openField(x  ,y-1);
				openField(x+1,y-1);
				openField(x+1,y  );
				openField(x+1,y+1);
				openField(x  ,y+1);
				openField(x-1,y+1);
				openField(x-1,y  );
			}
		}
	}
	return false;
}
uint8_t  randUpTo(uint8_t max){ //generates a tandom number between 0 and max
	uint16_t mask = 255;
	while ((mask>>1)>max) mask=mask>>1; //mask is one less than the lowest power of 2 higher than max. (as a bitmask)
	uint8_t i;
	do{
		i = nextRand() & mask;
	}while(i>max);
	return i;
}
//RNG inspired by https://github.com/Qqwy/SimpleRNG/blob/master/c/simple_rng.c
uint32_t nextRand(){
	randomInt^=randomInt<<13;
	randomInt^=randomInt>>17;
	randomInt^=randomInt<< 5;
	return randomInt;
}
void rect(int x,int y,int w,int h, uint16_t c){
	int j = 0;
	while (j<h){
		int i = 0;
		while (i<w){
			setPixel(x+i,y+j,c);	
			i++;
		}
		j++;
	}
}

bool mouseToClick(int mousex, int mousey, uint8_t action){ //returns false if ok and true if a bomb was clicked
	// mouseoverx = ((viewx / (float)PVIEW)) + ((mousex-(width >>1))/(float)scale) ;
	// mouseovery = ((viewy / (float)PVIEW)) + ((mousey-(height>>1))/(float)scale) ;
	// These equations are changed so no dividion has to be done (I don't want to mess with gcc and division...)
	int tempmouseoverx = (((( (viewx*scale))) + (((mousex-(width >>1))*PVIEW)) )>>VIEWSTEP);
	int tempmouseovery = (((( (viewy*scale))) + (((mousey-(height>>1))*PVIEW)) )>>VIEWSTEP);
	int mouseoverx = 0;
	int mouseovery = 0;
	if(tempmouseoverx<0||tempmouseovery<0)
		return false;
	while(tempmouseoverx>scale){
		mouseoverx++;
		tempmouseoverx-=scale; }
	while(tempmouseovery>scale){
		mouseovery++;
		tempmouseovery-=scale; }

	//printf("x%f y%d\n",tempmouseoverx,tempmouseovery);
	//
	if(mouseoverx>=sw||mouseovery>=sh)
		return false;
	switch (action) {
		case 1:
			if(!(field[fw*mouseovery+mouseoverx]&FLAG)){ //if there is already a flag
				if(openField(mouseoverx,mouseovery)){
					field[mouseoverx+(mouseovery*fw)]|=CLICKBOMB; //amount is set to CLICKBOMB to show that this bomb was clicked
					revealBombs();
					return true;
				}
			}
			break;
		case 2:
			if(!(field[fw*mouseovery+mouseoverx]&OPEN)){ //if there is already a flag
				if(field[fw*mouseovery+mouseoverx]&FLAG){ //if there is already a flag
					field[fw*mouseovery+mouseoverx]&=(~FLAG); //remove this flag
					flagamount--; //uncount flag
				}else{ //if there is not flag
					field[fw*mouseovery+mouseoverx]|=( FLAG); //set flag
					flagamount++; //count flag
				}
			}
			break;
		case 3:
			field[fw*mouseovery+mouseoverx]^=QUEST;
			if(field[fw*mouseovery+mouseoverx]&FLAG){ //if there is a flag
				field[fw*mouseovery+mouseoverx]&=(~FLAG); //remove this flag
				flagamount--; //uncount flag
			}
			break;
	}
	return false;

}

void printnumber(int x, int y, int w, int l, int s, int number, uint16_t c){ //x,y: coords, w: line width, l: line length, number: the number
	bool negative = false;
	if(number<0){
		number=0-number;
		printsegments(x,y,w,l,0b0000001,c);
		negative=true;
	}
	uint8_t hundreds = 0;
	uint8_t tens = 0;
	while (number>=100){ number-=100; hundreds++; }
	while (number>= 10){ number-= 10;     tens++; }
	if(!negative){
	printdigit(x    ,y,w,l,hundreds,c);}
	printdigit(x+s  ,y,w,l,tens    ,c);
	printdigit(x+s+s,y,w,l,number  ,c);

}
void printdigit(int x, int y, int w, int l, uint8_t digit, uint16_t c){ //x,y: coords, w: line width, l: line length, digit: the digit
	uint8_t segments;
	switch (digit){
		case 0:  segments=0b1111110; break;
		case 1:  segments=0b0110000; break;
		case 2:  segments=0b1101101; break;
		case 3:  segments=0b1111001; break;
		case 4:  segments=0b0110011; break;
		case 5:  segments=0b1011011; break;
		case 6:  segments=0b1011111; break;
		case 7:  segments=0b1110010; break;
		case 8:  segments=0b1111111; break;
		case 9:  segments=0b1111011; break;
		default: segments=0b0001000; break;
	}
	printsegments(x,y,w,l,segments,c);
}
void printsegments(int x, int y, int w, int l, uint8_t segments, uint16_t c){ //x,y: coords, w: line width, l: line length, segments: the segments, c: foreground color, d: background color
	if(segments&0b01000000) rect(x   ,y     ,l+w ,w   ,c); //a
	if(segments&0b00100000) rect(x+l ,y     ,w   ,l+w ,c); //b
	if(segments&0b00010000) rect(x+l ,y+l   ,w   ,l+w ,c); //c
	if(segments&0b00001000) rect(x   ,y+l+l ,l+w ,w   ,c); //d
	if(segments&0b00000100) rect(x   ,y+l   ,w   ,l+w ,c); //e
	if(segments&0b00000010) rect(x   ,y     ,w   ,l+w ,c); //f
	if(segments&0b00000001) rect(x   ,y+l   ,l+w ,w   ,c); //g
	//rect(x   ,y     ,l+w ,w   ,(segments&0b01000000)?c:d); //a
	//rect(x+l ,y     ,w   ,l+w ,(segments&0b00100000)?c:d); //b
	//rect(x+l ,y+l   ,w   ,l+w ,(segments&0b00010000)?c:d); //c
	//rect(x   ,y+l+l ,l+w ,w   ,(segments&0b00001000)?c:d); //d
	//rect(x   ,y+l   ,w   ,l+w ,(segments&0b00000100)?c:d); //e
	//rect(x   ,y     ,w   ,l+w ,(segments&0b00000010)?c:d); //f
	//rect(x   ,y+l   ,l+w ,w   ,(segments&0b00000001)?c:d); //g
}

//The acutal main
void main2(){
	//set the RNG seed
	{
#ifdef PC
		randomInt=random();
#else
		uint8_t* point1 = (uint8_t*)0xA413FEC0; //this is the 64Hz counter. Take it as a rng seed.
		uint8_t* point2 = (uint8_t*)0xA413FEC2; //this is the seconds counter. Take it as a rng seed.
		uint8_t* point3 = (uint8_t*)0xA413FEC4; //this is the seconds counter. Take it as a rng seed.
		randomInt= (*point1 & 0b00111111) | (*point2<<6) | (*point3<<14); 
#endif
		if (randomInt==0) randomInt=1;
	}
	
	int k_keydown=0;

	bool mousedown = 0;
	int  mousex    = 0;
	int  mousey    = 0;
	int  clickx    = 0;
	int  clicky    = 0;
	int  lastx     = 0;
	int  lasty     = 0;

	bool closed=false; //if the program gets closed
	while(!closed){
	
		//prepare new game
		clearField();
		generateField();
	
		bool active=true; //if a game is currently running
		stopped=false;
		won=false;
		flagamount=0;
		nrnotopen = ((sw*sh)-bamount); //set the amount of open fields
		//center the game
		viewx=sw<<(VIEWSTEP-1);
		viewy=sh<<(VIEWSTEP-1);
		int temp = width;
		scale=-1;
		while(temp>0){
			temp-=(sw+1);
			scale++;
		}
		//scale=width/(sw+1);
		while (active){
			//printf("x:%d y:%d s:%d\n",tempmouseoverx,tempmouseovery,scale);
			fillScreen(color(0xc0,0xc0,0xc0));	
			drawField();
			LCD_Refresh();

			//New Input Routine
			#ifdef PC //do this on the pc
				SDL_Event event;
				while(SDL_PollEvent(&event)){
			#else
				struct Input_Event event __attribute__((aligned(4)));
				memset(&event, 0, sizeof(event));
				//int loop_i = 1; // exit the loop even if ther are still touch events so the dragging is more fluid
				/*do*/{ 
					GetInput(&event, 0, 0x10);
			#endif
				#ifdef PC
					if (event.type==SDL_QUIT){
						closed=true; active=false;
					}
				#endif
				#ifdef PC
					if (event.type==SDL_KEYUP){
				#else
					if (event.type==EVENT_KEY&&event.data.key.direction==KEY_RELEASED){
				#endif
						k_keydown=0;
					}
				#ifdef PC
					if (event.type==SDL_KEYDOWN){
					//case SDL_KEYUP:
						switch(event.key.keysym.sym) {
				#else
					if (event.type==EVENT_KEY&&event.data.key.direction==KEY_PRESSED){
						switch((int)event.data.key.keyCode) {
				#endif
							case K_QUIT:
							#ifdef K_QUIT2
							case K_QUIT2:
							#endif
								closed=true;
								active=false;
								break;
	
							case K_NEW:
							#ifdef K_NEW2
							case K_NEW2:
							#endif
								active=false;
								break;
	
							case K_ZOOM_IN:
							#ifdef K_ZOOM_IN2
							case K_ZOOM_IN2:
							#endif	
								k_keydown=1;
								break;
	
							case K_ZOOM_OUT:
							#ifdef K_ZOOM_OUT2
							case K_ZOOM_OUT2:
							#endif
								k_keydown=2;
								break;
	
							case K_MODE_CLICK:
							#ifdef K_MODE_CLICK2
							case K_MODE_CLICK2:
							#endif
								cmode=1;
								break;
	
							case K_MODE_FLAG:
							#ifdef K_MODE_FLAG2
							case K_MODE_FLAG2:
							#endif
								cmode=2;
								break;
	
							case K_MODE_QUEST:
							#ifdef K_MODE_QUEST2
							case K_MODE_QUEST2:
							#endif
								cmode=3;
								break;
	
							case K_7: sw= 9;sh= 9;bamount= 10;active=false;break;
							case K_8: sw=16;sh=16;bamount= 40;active=false;break;
							case K_9: sw=16;sh=30;bamount= 99;active=false;break;
							case K_4: sw=10;sh=10;bamount= 10;active=false;break;
							case K_5: sw=32;sh=32;bamount=202;active=false;break;
							case K_6: sw=64;sh=64;bamount=800;active=false;break;
							case K_3: sw=64;sh=64;bamount=300;active=false;break;
						}	
					}
				#ifdef PC
					if (event.type==SDL_MOUSEMOTION){
						mousex = event.motion.x;
						mousey = event.motion.y;
				#else
					if (event.type==EVENT_TOUCH){
						mousex = event.data.touch_single.p1_x;
						mousey = event.data.touch_single.p1_y;
				#endif
						//when the pen moves $step pixels the field should move $s2 pixels.
						int s1   = 0;
						int s2;
						int step = 0;
						while (step==0){
							step = scale >> (VIEWSTEP-s1);
							s2 = 1 << s1;
							s1++;
							//if (step==0) step=1;
						}
						if (mousedown) {
							//printf("mousex:%3d lastx:%3d step:%2d s2:%2d\n",mousex,lastx,step,s2);
							while (mousex>lastx+step){ lastx+=step; viewx-=s2; }
							while (mousex<lastx-step){ lastx-=step; viewx+=s2; }
							while (mousey>lasty+step){ lasty+=step; viewy-=s2; }
							while (mousey<lasty-step){ lasty-=step; viewy+=s2; }
						}
					}
				#ifdef PC
					if (event.type==SDL_MOUSEBUTTONDOWN){
				#else
					if (event.type==EVENT_TOUCH&&event.data.touch_single.direction==TOUCH_DOWN){
				#endif
						mousedown = 1;
						clickx    = mousex;
						clicky    = mousey;
						lastx     = mousex;
						lasty     = mousey;
					}
				#ifdef PC
					if (event.type==SDL_MOUSEBUTTONUP){
				#else
					if (event.type==EVENT_TOUCH&&event.data.touch_single.direction==TOUCH_UP){
				#endif
						mousedown = 0;
						if(clickx==mousex&&clicky==mousey){ //when the cursor didn't move it was a click
							if(mousey<BARHEIGHT){ //if the menu bar at the top was clicked
								if (cmode==1){
									cmode=2;
								}else{
									cmode=1;
								}
							}else{//if anything but the menu bar was clicked
								if(!stopped) { //only click if the game is not stopped
									stopped = mouseToClick( mousex, mousey, cmode); //returns false if ok and true if a bomb was clicked
								}
							}
						}
					}
	
					//do the zoom if zoom keys are still pressed...
					if (k_keydown==1){	
						if (scale<128)scale+=1;
					}
					if (k_keydown==2){
						if (scale>4)  scale-=1;
					}
	
					//correct the position so the field does not get lost...
					if (viewx>=sw<<VIEWSTEP) { viewx=(sw<<VIEWSTEP)-1; }
					if (viewy>=sh<<VIEWSTEP) { viewy=(sh<<VIEWSTEP)-1; }
					if (viewx<0)   { viewx=0; }
					if (viewy<0)   { viewy=0; }
			#ifdef PC
				}
			#else
					//if(!--loop_i) break;
				}//while(event.type==EVENT_TOUCH&&event.data.touch_single.direction==TOUCH_HOLD_DRAG);
			#endif

			//check if the game was won
			if(nrnotopen==0){
				stopped=true;
				won=true;
			}

		}
	
	
	}
}
