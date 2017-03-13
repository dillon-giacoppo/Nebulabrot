/*************************************************/
/*              NebulaBrot Generator             */
/*************************************************/

// Dillon Giacoppo
// Program to generate the Buddahbrot

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>

/*************************************************/
/*                Output Variables               */
/*************************************************/

// Image Dimensions in Pixels (Must Be Square)
#define WIDTH 2200
#define HEIGHT 2200

// Max and Min Iterations
#define MAX_ORBITAL_LENGTH 105
#define MIN_ORBITAL_LENGTH 100

// Sample Size 
// High sample size requires either max iterations to be decreased
// Or minimum iterations increase (i.e Range reduced)

#define MAX_SAMPLES 100000

// Choose Orbital Ranges for colour channels
// Float fault will occur if orbital range is not inclusive of any color range
#define RED_CHANNEL_MAX 100000
#define RED_CHANNEL_MIN 1
#define GREEN_CHANNEL_MAX 100000
#define GREEN_CHANNEL_MIN 1
#define BLUE_CHANNEL_MAX 100000
#define BLUE_CHANNEL_MIN 1

// Sets multiple for printouts to be displayed
#define TICKER 10000

/*************************************************/
/*               Static Definitions              */
/*************************************************/

#define RGB 3
#define CHANNELS 3
#define BYTES_PER_PIXEL 3
#define RAND_RANGE 2
#define MAX_SQUARE_DIST 4
#define TRUE 1 
#define FALSE 0
#define OPTIMIZE1
//#define OPTIMIZE2

#pragma pack(1)
struct BMPHeader
{
    char bfType[2];       /* "BM" */
    int bfSize;           /* Size of file in bytes */
    int bfReserved;       /* set to 0 */
    int bfOffBits;        /* Byte offset to actual bitmap data (= 54) */
    int biSize;           /* Size of BITMAPINFOHEADER, in bytes (= 40) */
    int biWidth;          /* Width of image, in pixels */
    int biHeight;         /* Height of images, in pixels */
    short biPlanes;       /* Number of planes in target device (set to 1) */
    short biBitCount;     /* Bits per pixel (24 in this case) */
    int biCompression;    /* Type of compression (0 if no compression) */
    int biSizeImage;      /* Image size, in bytes (0 if no compression) */
    int biXPelsPerMeter;  /* Resolution in pixels/meter of display device */
    int biYPelsPerMeter;  /* Resolution in pixels/meter of display device */
    int biClrUsed;        /* Number of colors in the color table (if 0, use 
                             maximum allowed by biBitCount) */
    int biClrImportant;   /* Number of important colors.  If 0, all colors 
                             are important */
};
#pragma pack(0)


typedef struct _complex {
   long double real;
   long double imag;
} complex;

complex randomCoord();
void processPoints();
int orbitalLength(complex c);
int checkExclusions(complex z);
long double modulusSquared(complex z);
complex square(complex z);
complex add(complex a, complex b);
void renderImage();
int write_bmp(const char* filename);
void orbitTrace(complex c, int orbital_length);


char bmp_image[WIDTH][HEIGHT][RGB];

long long hit_counter[WIDTH][HEIGHT][CHANNELS];


int main(int argc, char* argv[]){
   srand((unsigned)time(NULL));
   int timestamp = (unsigned)time(NULL);
   char filename[50];
   sprintf(filename, "%d", timestamp);
   strcat(filename, ".bmp");
   printf("Processing Points\n");
   processPoints();
   
   printf("Rendering Image\n");
   renderImage();
   printf("Saving To File\n");
   write_bmp(filename);
   return EXIT_SUCCESS;
}

complex randomCoord(){
   complex c;
   do{
      c.real = RAND_RANGE*(2.0 * (long double)((long double)rand()/(long double)RAND_MAX) - 1.0);
      c.imag = RAND_RANGE*(2.0 * (long double)((long double)rand()/(long double)RAND_MAX) - 1.0); 
   } while (checkExclusions(c) == FALSE);

   return c;
}

void processPoints(){
   complex c;
   int orbital_length;
   int samples = 0;
   printf("Searching for points...\n");
   while(samples < MAX_SAMPLES){
      c = randomCoord();
      orbital_length = orbitalLength(c);
      if(orbital_length < MAX_ORBITAL_LENGTH && orbital_length > MIN_ORBITAL_LENGTH){
         orbitTrace(c, orbital_length);
         samples++;
         if(samples%TICKER == 0){
            printf("%6d / %d\n", samples, MAX_SAMPLES);
         }
      }
   }
}

int orbitalLength(complex c){
   int orbital_length = 1;
   complex z = {0, 0};

   while (orbital_length <= MAX_ORBITAL_LENGTH){
      
      z = add(square(z), c);
      if(modulusSquared(z) > MAX_SQUARE_DIST){
         break;
      }
      orbital_length++;
   }

   return orbital_length;
}

void orbitTrace(complex c, int orbital_length){
   int orbital_step = 1;
   complex z = {0, 0};
   int x, y;
   float x_scale = WIDTH / 4;
   float x_offset = WIDTH / 2;
   float y_scale = HEIGHT / 4;
   float y_offset = HEIGHT / 2;

   while (orbital_step < MAX_ORBITAL_LENGTH){
      x = 0;
      y = 0;
      z = add(square(z), c);
      if(modulusSquared(z) > MAX_SQUARE_DIST){
         break;
      }
      x = (x_scale * z.real + x_offset)-1;
      y = (y_scale * z.imag + y_offset)-1;
      
      if(orbital_length < BLUE_CHANNEL_MAX){
         hit_counter[x][y][2] ++;
      } 

      if(orbital_length < GREEN_CHANNEL_MAX && orbital_length > GREEN_CHANNEL_MIN){
         hit_counter[x][y][1] ++;
      }

      if(orbital_length < RED_CHANNEL_MAX && orbital_length > RED_CHANNEL_MIN){
         hit_counter[x][y][0] ++;
      }

      orbital_step++;
   }
}

int checkExclusions(complex z){
   int to_iterate = TRUE;
   #ifdef OPTIMIZE1
      int cardioid = 0, bulb;
      double p = sqrt(pow((z.real - 0.25), 2) + pow(z.imag, 2));

      if(z.real < p - 2 * pow(p, 2) + 0.25){
         cardioid = TRUE;
      }
      if(pow((z.real+1), 2) + pow(z.imag,2) < (1/16)){
         bulb = TRUE;
      }

      if(cardioid == TRUE || bulb == TRUE){
         to_iterate = FALSE;
      } else {
         to_iterate = TRUE;
      }
   #endif
   #ifdef OPTIMIZE2
      if(
            (z.real >  -1.2 && z.real <=  -1.1 && z.imag >  -0.1 && z.imag  < 0.1)
         || (z.real >  -1.1 && z.real <=  -0.9 && z.imag >  -0.2 && z.imag < 0.2)
         || (z.real >  -0.9 && z.real <=  -0.8 && z.imag >  -0.1 && z.imag < 0.1)
         || (z.real > -0.69 && z.real <= -0.61 && z.imag >  -0.2 && z.imag < 0.2)
         || (z.real > -0.61 && z.real <=  -0.5 && z.imag > -0.37 && z.imag < 0.37)
         || (z.real >  -0.5 && z.real <= -0.39 && z.imag > -0.48 && z.imag < 0.48)
         || (z.real > -0.39 && z.real <=  0.14 && z.imag > -0.55 && z.imag < 0.55)
         || (z.real >  0.14 && z.real <   0.29 && z.imag > -0.42 && z.imag < -0.07)
         || (z.real >  0.14 && z.real <   0.29 && z.imag >  0.07 && z.imag < 0.42)
      ) 
      {
         to_iterate = FALSE;
      }
   #endif
   return to_iterate;
}

void renderImage(){
   int x, y;
   unsigned char color;
   long red_max = 0, green_max = 0, blue_max = 0;

   for(y = 0; y < HEIGHT; y++){
      for(x = 0; x < WIDTH; x++){
         if(hit_counter[x][y][0] > red_max){
            red_max = hit_counter[x][y][0];
         }
         if(hit_counter[x][y][1] > green_max){
            green_max = hit_counter[x][y][1];
         }
         if(hit_counter[x][y][2] > blue_max){
            blue_max = hit_counter[x][y][2];
         }
      }
   }

   
   for(y = 0; y < HEIGHT; y++){
      for(x = 0; x < WIDTH; x++){
         color = hit_counter[x][y][0];
         color = 255*cbrt(color)/cbrt(red_max);
         bmp_image[x][y][0] = color;
      }
   }

   for(y = 0; y < HEIGHT; y++){
      for(x = 0; x < WIDTH; x++){
         color = hit_counter[x][y][1];
         color = 255*cbrt(color)/cbrt(green_max);
         bmp_image[x][y][1] = color;
      }
   }

   for(y = 0; y < HEIGHT; y++){
      for(x = 0; x < WIDTH; x++){
         color = hit_counter[x][y][2];
         color = 255*cbrt(color)/cbrt(blue_max);
         bmp_image[x][y][2] = color;
      }
   }
   printf("Render Complete\n");
}

long double modulusSquared(complex z){
   long double x = z.real;
   long double y = z.imag;

   return x * x + y * y;
}

complex square(complex z){
   complex result;
   long double x = z.real;
   long double y = z.imag;

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

int write_bmp(const char* filename){
   printf("Begin Save\n");
   int y = 0, x = 0;
  
   FILE *file;
   struct BMPHeader bmph;

   /* The length of each line must be a multiple of 4 bytes */

   int bytesPerLine = (3 * (WIDTH + 1) / 4) * 4;

   bmph.bfType[0] = 'B';
   bmph.bfType[1] = 'M';
   bmph.bfOffBits = 54;
   bmph.bfSize = bmph.bfOffBits + bytesPerLine * HEIGHT;
   bmph.bfReserved = 0;
   bmph.biSize = 40;
   bmph.biWidth = WIDTH;
   bmph.biHeight = HEIGHT;
   bmph.biPlanes = 1;
   bmph.biBitCount = 24;
   bmph.biCompression = 0;
   bmph.biSizeImage = bytesPerLine * HEIGHT;
   bmph.biXPelsPerMeter = 0;
   bmph.biYPelsPerMeter = 0;
   bmph.biClrUsed = 0;       
   bmph.biClrImportant = 0; 
   printf("BMP HEADER SET\n");
   file = fopen (filename, "wb");
   printf("BMP/OPENED/CREATED\n");
   if (file == NULL) return(0);
   fwrite(&bmph, sizeof(bmph), 1, file);
   printf("Printed BMP Header\n");
   char bmpColorTable[WIDTH * HEIGHT * BYTES_PER_PIXEL];
   while(y < HEIGHT){
      x = 0;
      while(x < WIDTH){
         int bmpPxOffset = BYTES_PER_PIXEL * (y * HEIGHT + x);
         bmpColorTable[bmpPxOffset    ] = bmp_image[x][y][2];
         bmpColorTable[bmpPxOffset + 1] = bmp_image[x][y][1];
         bmpColorTable[bmpPxOffset + 2] = bmp_image[x][y][0];
         x++;
      }
      y++;
   }  
   fwrite(&bmpColorTable, sizeof(bmpColorTable), 1, file);
   printf("Printed BMP image color table\n");
   fclose(file);

   return(1);
}




