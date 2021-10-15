#ifndef __LIB_BMP_H__
#define __LIB_BMP_H__

int bmpGetFileSize(int height, int width);
void generateBitmapImageToMemory (unsigned char* image, int height, int width, unsigned char* imageBuffer);
void generateBitmapImage (unsigned char* image, int height, int width, char* imageFileName);

#endif