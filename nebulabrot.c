// Dillon Giacoppo
// Program to generate the Buddahbrot

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_ORBITAL_LENGTH 100000
#define MIN_ORBITAL_LENGTH 10000
#define RED_CHANNEL_MAX 100000
#define GREEN_CHANNEL_MAX 35000
#define BLUE_CHANNEL_MAX 20000
#define SAMPLE_SIZE 1000
#define MAX_SQUARE_DIST 4
#define SCREEN_WIDTH 100
#define SCREEN_HEIGHT 100
#define RAND_RANGE 2
#define TICKER 100

#define TRUE 1 
#define FALSE 0



typedef struct _complex {
    double real;
    double imag;
} complex;

void initializeImageBuffer();
void renderImage();
void processPoints();
int orbitalLength(complex c);
void traceOrbit(complex c, int orbital_length);
complex newRandom(void);
int preIterate(complex z);
double randomPoint();
double modulusSquared(complex z);
complex square(complex z);
complex add(complex a, complex b);
double modulus(complex z);
double argument(complex z);
double distance(complex a, complex b);
void write_bmp();
unsigned int round_div(unsigned int dividend, unsigned int divisor);

static complex origin = {0,0};
unsigned char image_buffer[SCREEN_WIDTH][SCREEN_HEIGHT][3];
long long tempCounter[SCREEN_WIDTH][SCREEN_HEIGHT];
long long red_channel[SCREEN_WIDTH][SCREEN_HEIGHT];
unsigned char green_channel[SCREEN_WIDTH][SCREEN_HEIGHT];
unsigned char blue_channel[SCREEN_WIDTH][SCREEN_HEIGHT];


int main(int argc, char* argv[]){
   srand((unsigned)time(NULL));

   char filename[50] = "nebulabrot.bmp";

   initializeImageBuffer();
   printf("Calculating Orbital Trajectories....\n");
   processPoints();
   printf("Rendering Image....\n");
   renderImage();
   printf("Writing Image....\n");
   write_bmp(filename, SCREEN_WIDTH, SCREEN_HEIGHT);
   return EXIT_SUCCESS;
}

void initializeImageBuffer(){
   int x, y;
   for(y = 0; y < SCREEN_HEIGHT; y++){
      for(x = 0; x < SCREEN_WIDTH; x++){
         image_buffer[x][y][0] = 0;
         image_buffer[x][y][1] = 0;
         image_buffer[x][y][2] = 0;
      }
   }
}

void renderImage(){
   int x, y;
   unsigned char color;
   int red_max = 0, green_max = 0, blue_max = 0;

   for(y = 0; y < SCREEN_HEIGHT; y++){
      for(x = 0; x < SCREEN_WIDTH; x++){
         if(red_channel[x][y] > red_max){
            red_max = red_channel[x][y];
         }
      }
   }

   for(y = 0; y < SCREEN_HEIGHT; y++){
      for(x = 0; x < SCREEN_WIDTH; x++){
         if(green_channel[x][y] > green_max){
            green_max = green_channel[x][y];
         }
      }
   }  
    
   for(y = 0; y < SCREEN_HEIGHT; y++){
      for(x = 0; x < SCREEN_WIDTH; x++){
         if(blue_channel[x][y] > blue_max){
            blue_max = blue_channel[x][y];
         }
      }
   }


   for(y = 0; y < SCREEN_HEIGHT; y++){
      for(x = 0; x < SCREEN_WIDTH; x++){
         color = red_channel[x][y];
         color = 255*cbrt(color)/cbrt(red_max);
         image_buffer[x][y][0] = color;
      }
   }

   for(y = 0; y < SCREEN_HEIGHT; y++){
      for(x = 0; x < SCREEN_WIDTH; x++){
         color = green_channel[x][y];
         color = 255*cbrt(color)/cbrt(green_max);
         image_buffer[x][y][1] = color;
      }
   }

   for(y = 0; y < SCREEN_HEIGHT; y++){
      for(x = 0; x < SCREEN_WIDTH; x++){
         color = blue_channel[x][y];
         color = 255*cbrt(color)/cbrt(blue_max);
         image_buffer[x][y][2] = color;
      }
   }
}

void processPoints(){
   complex c;
   int i = 0;
   int orbital_length;
   while(i < SAMPLE_SIZE){
      c = newRandom();
      orbital_length = orbitalLength(c);
      if(orbital_length < MAX_ORBITAL_LENGTH  && orbital_length > MIN_ORBITAL_LENGTH){
        printf("%f , %f\t%d\n",c.real,c.imag,orbital_length);
        traceOrbit(c, orbital_length);
        i++;
        printf("Calculated %d points\n", i);
      }
      if(i%TICKER == 0){
        // printf("Calculated %d points\n", i);
      }
   }
}

int orbitalLength(complex c){
  complex z = {0, 0};
  int orbital_length = 0;
  int x,y;

  int a = 0, b= 0;
  while(a < SCREEN_HEIGHT){
    while(b < SCREEN_WIDTH){
      tempCounter[b][a] = 0;
      b++;
    }
    a++;
  }

  while (modulusSquared(z) < MAX_SQUARE_DIST && orbital_length < MAX_ORBITAL_LENGTH) {
      z = add(square(z), c);
      x = z.real - SCREEN_WIDTH / 2 * 0.5 + 0.5 / 2;
      y = z.imag - SCREEN_HEIGHT / 2 * 0.5 + 0.5 / 2;
      tempCounter[x][y]++;
      orbital_length++;
  }

  return orbital_length;
}

void traceOrbit(complex c, int orbital_length){
   int y, x;
   for(y = 0; y < SCREEN_HEIGHT; y ++){
      for(x = 0; x < SCREEN_WIDTH; x++){
         if(orbital_length <= BLUE_CHANNEL_MAX){
            blue_channel[x][y] = tempCounter[x][y];
            //printf("Blue counter was incremented!\n");
         } else if(orbital_length <= GREEN_CHANNEL_MAX){
            green_channel[x][y] = tempCounter[x][y];
         } else  {
            red_channel[x][y] = tempCounter[x][y];
         }

      }
   }
}

complex newRandom(){
   int iterate;
   double x, y;
   complex z;
   while(1)
   {
      x = randomPoint();
      y = randomPoint();
      z.real = x;
      z.imag = y;
      if(distance(z, origin) < 4){ 
         iterate = preIterate(z);
         if(iterate == TRUE){
            return z;
         }
      }
   }
}

int preIterate(complex z){
   int cardioid = 0, bulb, iterate;
   double p = sqrt(pow((z.real - 0.25), 2) + pow(z.imag, 2));

   if(z.real < p - 2 * pow(p, 2) + 0.25){
      cardioid = TRUE;
   }
   if(pow((z.real+1), 2) + pow(z.imag,2) < (1/16)){
      bulb = TRUE;
   }
   if(cardioid == TRUE || bulb == TRUE){
      iterate = FALSE;
   } else {
      iterate = TRUE;
   }

   return iterate;
}

double randomPoint(){
   double point;
   point = RAND_RANGE*(2.0f * ((float)rand()/(float)RAND_MAX) - 1); 
   return point;
}

double modulusSquared(complex z){
    double x = z.real;
    double y = z.imag;

    return x * x + y * y;
}

complex square(complex z){
    complex result;
    double x = z.real;
    double y = z.imag;

    result.real = x * x - y * y;
    result.imag = 2 * x * y;

    return result;
}

complex add(complex a, complex b){
    complex result;

    result.real = a.real + b.real;
    result.imag = a.imag + b.imag;

    return result;
}

double modulus(complex z){
   double modulus = sqrt(modulusSquared(z));
   return modulus;
}

double argument(complex z){
   double argument = atan(z.imag/z.real);
    return argument;
}

double distance(complex a, complex b){
   b.real = -b.real;
   b.imag = -b.imag;

   double distance = modulus(add(a, b));

   return distance;
}

void write_bmp(){
  int a = 0, b= 0;
  while(a < SCREEN_HEIGHT){
    while(b < SCREEN_WIDTH){
      printf("%llu\t", tempCounter[b][a]);
      b++;
    }
    putchar('\n');
    a++;
  }

}

unsigned int round_div(unsigned int dividend, unsigned int divisor){
    return (dividend + (divisor / 2)) / divisor;
}

