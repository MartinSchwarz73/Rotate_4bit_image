#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <inttypes.h>
#include <tiffio.h>
#include <tiff.h>
#include <pthread.h>
#include <chrono>
#include <ctime>

using namespace std;

struct rotationParams_t {
    TIFF* tif;
    int angle;
    char* srcImg;
    char* origFile;
    short origWpx;
    short origHpx;
} ;

int rotate_cw90(TIFF* tif, char* srcImg, char* origFile, short origWpx, short origHpx);
int rotate_ccw90(TIFF* tif, char* srcImg, char* origFile, short origWpx, short origHpx);
int rotate_cw180(TIFF* tif, char* srcImg, char* origFile, short origWpx, short origHpx);

void *callRotateFn(void *params)
{
    rotationParams_t p = *(rotationParams_t *) params;
    
    switch (p.angle) {
        case 90: 
            std::cout << "Thread1 - 90° ... \n" << std::flush;
            rotate_cw90(p.tif, p.srcImg, p.origFile, p.origWpx, p.origHpx); 
            break;
        case 180: 
            std::cout << "Thread2 - 180° ...\n" << std::flush;
            rotate_cw180(p.tif, p.srcImg, p.origFile, p.origWpx, p.origHpx); 
            break;
        case 270: 
            std::cout << "Thread3 - 270° ...\n" << std::flush;
            rotate_ccw90(p.tif, p.srcImg, p.origFile, p.origWpx, p.origHpx); 
            break;
    }
}


int rotate_cw90(TIFF* tif, char* srcImg, char* origFile, short origWpx, short origHpx) 
{
    std::cout << std::endl << "Rotate_cw90() - start ... \n" << std::flush;
    
    char outFileName[30] = "cw90";
    char* rotImg; 
    int  oddW = 0, oddH = 0;
    short origW = (origWpx+1)/2;
    short rotW = (origHpx+1)/2;
    short rotH = origWpx;
    short myTemp = 0;
    int result = 0;
    
    oddW = origWpx % 2;
    oddH = origHpx % 2; 
    
    rotImg = (char *)malloc(rotW * rotH * sizeof(char));
    memset(rotImg, 0, rotW * rotH * sizeof(char));
    //printf("nova matice = %p\n", rotImg);
    //printf("srcTiff = %li \n", (rotImg + rotW * rotH) - (rotImg));
    //printf("Predano funkci: Orig: %d x %d, Rotated: %d x %d\n", origW, origH, rotW, rotH);
    
    if (rotImg) 
    {
        for(int rr=0; rr<=origHpx; rr=rr+2) 
        {
            for (int cc=0; cc<origW; cc++)
            {

                *(rotImg+(cc*2+1)*rotW - rr/2 - 1) = ((oddH && !rr) ? 0x00 : ((*(srcImg + cc + (rr-oddH)*origW) & 0xf0) >> 4)) | *(srcImg + cc + (rr-oddH+1)*origW) & 0xf0;
                
                if (!oddW || (cc+1)!=origW) *(rotImg+(cc*2+2)*rotW - rr/2 - 1) = ((oddH && !rr) ? 0x00 : ((*(srcImg + cc + (rr-oddH)*origW) & 0x0f))) | (*(srcImg + cc + (rr-oddH+1)*origW) & 0x0f) << 4;
                
            }
        }
    }
    
    strcat(outFileName,origFile);
    TIFF* outFile = TIFFOpen(outFileName, "w");
    
    if (outFile) 
    {
        TIFFSetField(outFile, TIFFTAG_IMAGEWIDTH, origHpx);  // set the width of the image
        TIFFSetField(outFile, TIFFTAG_IMAGELENGTH, origWpx);    // set the height of the image
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &myTemp);   // get number of channels per pixel
        TIFFSetField(outFile, TIFFTAG_SAMPLESPERPIXEL, myTemp);   // set number of channels per pixel
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &myTemp); 
        TIFFSetField(outFile, TIFFTAG_BITSPERSAMPLE, myTemp);    // set the size of the channels
        TIFFSetField(outFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
        TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &myTemp);
        TIFFSetField(outFile, TIFFTAG_PLANARCONFIG, myTemp);
        TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &myTemp);
        TIFFSetField(outFile, TIFFTAG_PHOTOMETRIC, myTemp);
        TIFFGetField(tif, TIFFTAG_COMPRESSION, &myTemp);    
        TIFFSetField(outFile, TIFFTAG_COMPRESSION, myTemp);    
        
        result = TIFFWriteEncodedStrip(outFile, 0, rotImg, rotW * rotH * sizeof(char));
        std::cout << "Rotate_cw90() - done. \n" << std::flush;
        if (result == -1) std::cout << std::endl << "Error[" << result << "]: Unable to write file " << outFileName << std::endl;
        TIFFClose(outFile);
    }
    free(rotImg);
    return result;
}
   

   
   
int rotate_ccw90(TIFF* tif, char* srcImg, char* origFile, short origWpx, short origHpx) 
{
    std::cout << std::endl << "Rotate_ccw90() - start ... \n" << std::flush;
    
    char outFileName[30] = "ccw90";
    char* rotImg; 
    int  oddW = 0, oddH = 0;
    short origW = (origWpx+1)/2;
    short rotW = (origHpx+1)/2;
    short rotH = origWpx;
    short myTemp = 0;
    int result = 0;
    
    oddW = origWpx % 2;
    oddH = origHpx % 2; 
    
    rotImg = (char *)malloc(rotW * rotH * sizeof(char));
    memset(rotImg, 0, rotW * rotH * sizeof(char));
    
    if (rotImg) 
    {
        for(int rr=0; rr<=origHpx; rr=rr+2) 
        {
            for (int cc=0; cc<origW; cc++)
            {
                
              *(rotImg+((origW-cc-oddW)*2)*rotW + rr/2) = ((oddH && rr==origHpx) ? 0x00 : *(srcImg + cc + (rr*origW)) & 0xf0) | (*(srcImg + cc + ((rr+1)*origW)) & 0xf0) >> 4;
                
                if (!oddW || (cc+1)!=origW) *(rotImg+((origW-cc-oddW)*2-1)*rotW + rr/2) = ((oddH && rr==origHpx) ? 0x00 : (*(srcImg + cc + (rr*origW)) & 0x0f) << 4) | *(srcImg + cc + (rr+1)*origW) & 0x0f;
                
            }
        }
    }
    
    strcat(outFileName,origFile);
    TIFF* outFile = TIFFOpen(outFileName, "w");
    
    if (outFile) 
    {
        TIFFSetField(outFile, TIFFTAG_IMAGEWIDTH, origHpx);  // set the width of the image
        TIFFSetField(outFile, TIFFTAG_IMAGELENGTH, origWpx);    // set the height of the image
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &myTemp);   // get number of channels per pixel
        TIFFSetField(outFile, TIFFTAG_SAMPLESPERPIXEL, myTemp);   // set number of channels per pixel
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &myTemp); 
        TIFFSetField(outFile, TIFFTAG_BITSPERSAMPLE, myTemp);    // set the size of the channels
        TIFFSetField(outFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
        TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &myTemp);
        TIFFSetField(outFile, TIFFTAG_PLANARCONFIG, myTemp);
        TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &myTemp);
        TIFFSetField(outFile, TIFFTAG_PHOTOMETRIC, myTemp);
        TIFFGetField(tif, TIFFTAG_COMPRESSION, &myTemp);    
        TIFFSetField(outFile, TIFFTAG_COMPRESSION, myTemp);    
        
        result = TIFFWriteEncodedStrip(outFile, 0, rotImg, rotW * rotH * sizeof(char));
        std::cout << "Rotate_ccw90() - done. \n" << std::flush;
        if (result == -1) std::cout << std::endl << "Error[" << result << "]: Unable to write file " << outFileName << std::endl;
        TIFFClose(outFile);
    }
    free(rotImg);
    return result;
}   
   

   
int rotate_cw180(TIFF* tif, char* srcImg, char* origFile, short origWpx, short origHpx) 
{
    std::cout << std::endl << "Rotate_cw180() - start ... \n" << std::flush;
    
    char outFileName[30] = "cw180";
    char* rotImg; 
    int  oddW = 0, oddH = 0;
    short origW = (origWpx+1)/2;
    short rotW = (origWpx+1)/2;
    short rotH = origHpx;
    short myTemp = 0;
    int result = 0;
    
    oddW = origWpx % 2;
    oddH = origHpx % 2;
    
    rotImg = (char *)malloc(rotW * rotH * sizeof(char));   
    
    if (rotImg) 
    {
        for(int rr=0; rr<origHpx; rr++) 
        {
            for (int cc=0; cc<origW; cc++)
            {
  
                *(rotImg+rr*origW+cc) = !oddW ? ((*(srcImg + (origHpx-1-rr)*origW-cc-1) & 0xf0) >> 4 | (*(srcImg + (origHpx-1-rr)*origW-cc-1) & 0x0f) << 4) : (((cc+1)<origW) ? (*(srcImg + (origHpx-rr)*origW-cc-1) & 0xf0 | *(srcImg + (origHpx-rr)*origW-cc-2) & 0x0f) : (0x00 | *(srcImg + (origHpx-rr)*origW-cc-1) & 0xf0));
                
            }
        }
    }
    
    strcat(outFileName,origFile);
    TIFF* outFile = TIFFOpen(outFileName, "w");
    
    if (outFile) 
    {
        TIFFSetField(outFile, TIFFTAG_IMAGEWIDTH, origWpx);  // set the width of the image
        TIFFSetField(outFile, TIFFTAG_IMAGELENGTH, origHpx);    // set the height of the image
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &myTemp);   // get number of channels per pixel
        TIFFSetField(outFile, TIFFTAG_SAMPLESPERPIXEL, myTemp);   // set number of channels per pixel
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &myTemp); 
        TIFFSetField(outFile, TIFFTAG_BITSPERSAMPLE, myTemp);    // set the size of the channels
        TIFFSetField(outFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
        TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &myTemp);
        TIFFSetField(outFile, TIFFTAG_PLANARCONFIG, myTemp);
        TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &myTemp);
        TIFFSetField(outFile, TIFFTAG_PHOTOMETRIC, myTemp);
        TIFFGetField(tif, TIFFTAG_COMPRESSION, &myTemp);    
        TIFFSetField(outFile, TIFFTAG_COMPRESSION, myTemp);    
        
        result = TIFFWriteEncodedStrip(outFile, 0, rotImg, rotW * rotH * sizeof(char));
        std::cout << "Rotate_cw180() - done. \n" << std::flush;
        if (result == -1) std::cout << std::endl << "Error[" << result << "]: Unable to write file " << outFileName << std::endl;
        TIFFClose(outFile);
    }
    free(rotImg);
    return result;
}
   

   
int main(int argc, char *argv[])
{

    auto start = std::chrono::system_clock::now();

    char fileFromCMD[30];
    strcpy(fileFromCMD, argv[1]); 
    
    
    TIFF* tifFile = TIFFOpen(fileFromCMD, "r");
    
    char* srcData;
    int result;
    short w = 0, h = 0;
    
  
    if (tifFile) {    
        std::cout << "START \n\n" << std::flush;
        TIFFGetField(tifFile, TIFFTAG_IMAGEWIDTH, &w);  // set the width of the image
        TIFFGetField(tifFile, TIFFTAG_IMAGELENGTH, &h);    // set the height of the image
        /*      TIFFGetField(tifFile, TIFFTAG_SAMPLESPERPIXEL, &sampleperpixel);   // set number of channels per pixel
         *        TIFFGetField(tifFile, TIFFTAG_BITSPERSAMPLE, &bitspersample);    // set the size of the channels
         *        TIFFGetField(tifFile, TIFFTAG_ORIENTATION, &imageorigin);    // set the origin of the image.
         *        //   Some other essential fields to set that you do not have to understand for now.
         *        TIFFGetField(tifFile, TIFFTAG_PLANARCONFIG, &planarcfg);
         *        TIFFGetField(tifFile, TIFFTAG_PHOTOMETRIC, &photometrick);
         *        TIFFGetField(tifFile, TIFFTAG_COMPRESSION, &compr);
         *        TIFFGetField(tifFile, TIFFTAG_STRIPBYTECOUNTS, &bc);
         *        TIFFGetField(tifFile, TIFFTAG_ROWSPERSTRIP, &rowsperim);
         *        
         */    
        srcData = (char *)_TIFFmalloc(((w+1)/2) * h * sizeof(char));
        TIFFReadEncodedStrip(tifFile, 0, srcData, -1);
        
        pthread_t thread1, thread2, thread3;
        int  iret1, iret2, iret3;
        rotationParams_t r90 {tifFile, 90, srcData, fileFromCMD, w, h};
        rotationParams_t r180 {tifFile, 180, srcData, fileFromCMD, w, h};
        rotationParams_t r270 {tifFile, 270, srcData, fileFromCMD, w, h};
            
        iret1 = pthread_create( &thread1, NULL, callRotateFn, (void*) &r90);
        iret2 = pthread_create( &thread2, NULL, callRotateFn, (void*) &r180);
        iret3 = pthread_create( &thread3, NULL, callRotateFn, (void*) &r270);      
        
        pthread_join( thread1, NULL);
        pthread_join( thread2, NULL); 
        pthread_join( thread3, NULL); 

        
        _TIFFfree(srcData);
        TIFFClose(tifFile);
        
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        std::cout << "Done. Elapsed time: " << elapsed_seconds.count() << "s\n";
    }
    exit(0);
}
