// Dillon Giacoppo
// Program to generate the Buddahbrot

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>

#define WIDTH 900
#define HEIGHT 900
#define RGB 3
#define CHANNELS 3
#define BYTES_PER_PIXEL 3

#define MAX_ORBITAL_LENGTH 8000
#define MIN_ORBITAL_LENGTH 500
#define MAX_SAMPLES 1000
#define RED_CHANNEL_MAX 8000
#define RED_CHANNEL_MIN 2000
#define GREEN_CHANNEL_MAX 5000
#define GREEN_CHANNEL_MIN 1300
#define BLUE_CHANNEL_MAX 2000
#define BLUE_CHANNEL_MIN 1000




#define RAND_RANGE 2
#define MAX_SQUARE_DIST 4
#define TRUE 1 
#define FALSE 0

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
int orbitalLength(complex c, int max_depth);
int checkExclusions(complex z);
long double modulusSquared(complex z);
complex square(complex z);
complex add(complex a, complex b);
void renderImage();
int write_bmp(const char* filename);


char bmp_image[WIDTH][HEIGHT][RGB];

long long hit_counter[WIDTH][HEIGHT][CHANNELS];
int temp_counter[WIDTH][HEIGHT];


int main(int argc, char* argv[]){
   srand((unsigned)time(NULL));
   char filename[50] = "brot.bmp";
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
   int x, y;
   int samples = 0;
   printf("Searching for points...\n");
   while(samples < MAX_SAMPLES){
      c = randomCoord();
      orbital_length = orbitalLength(c, MAX_ORBITAL_LENGTH);
      if(orbital_length < MAX_ORBITAL_LENGTH && orbital_length > MIN_ORBITAL_LENGTH){
         if(orbital_length < BLUE_CHANNEL_MAX){
            for(y = 0; y < HEIGHT; y++){
               for(x = 0; x < WIDTH; x++){
                  hit_counter[x][y][2] += temp_counter[x][y];
               }
            }
         } 
         if(orbital_length < GREEN_CHANNEL_MAX && orbital_length > GREEN_CHANNEL_MIN){
            for(y = 0; y < HEIGHT; y++){
               for(x = 0; x < WIDTH; x++){
                  hit_counter[x][y][1] += temp_counter[x][y];
               }
            }
         }

         if(orbital_length < RED_CHANNEL_MAX && orbital_length > RED_CHANNEL_MIN){
            for(y = 0; y < HEIGHT; y++){
               for(x = 0; x < WIDTH; x++){
                  hit_counter[x][y][0] += temp_counter[x][y];
               }
            }
         }

      
         samples++;
         printf("%4d : [%11.19Lf , %11.19Lf] with %5d\n", samples, c.real, c.imag, orbital_length);
      }
   }
}

int orbitalLength(complex c, int max_depth){
   int orbital_length = 1;
   complex z = {0, 0};
   int x, y;
   float x_scale = WIDTH / 4;
   float x_offset = WIDTH / 2;
   float y_scale = HEIGHT / 4;
   float y_offset = HEIGHT / 2;

   int a = 0, b = 0;

   while( a < HEIGHT){ 
      b = 0;
      while(b < WIDTH){
         temp_counter[b][a] = 0;
         b++;
      }
      a++;
   }

   while (orbital_length < max_depth){
      x = 0;
      y = 0;
      z = add(square(z), c);
      if(modulusSquared(z) > MAX_SQUARE_DIST){
         break;
      }
      x = (x_scale * z.real + x_offset)-1;
      y = (y_scale * z.imag + y_offset)-1;
      temp_counter[x][y] ++;
      orbital_length++;
   }

   return orbital_length;
}

int checkExclusions(complex z){
   int cardioid = 0, bulb, to_iterate;
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

   return to_iterate;
}

void renderImage(){
   int x, y;
   unsigned char color;
   int red_max = 0, green_max = 0, blue_max = 0;

   for(y = 0; y < HEIGHT; y++){
      for(x = 0; x < WIDTH; x++){
         if(hit_counter[x][y][0] > red_max){
            red_max = hit_counter[x][y][0];
         }
      }
   }

   for(y = 0; y < HEIGHT; y++){
      for(x = 0; x < WIDTH; x++){
         if(hit_counter[x][y][1] > green_max){
            green_max = hit_counter[x][y][1];
         }
      }
   }  
    
   for(y = 0; y < HEIGHT; y++){
      for(x = 0; x < WIDTH; x++){
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

   file = fopen (filename, "wb");
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




