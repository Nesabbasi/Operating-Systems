#include <iostream>
#include <unistd.h>
#include <fstream>
#include <chrono>
#include <pthread.h>

#define NUMBER_OF_THREADS 8
#define OUTPUT_FILE "output.bmp"

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;

#pragma pack(1)
#pragma once

char *fileBuffer;
int bufferSize;

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

unsigned char **red;
unsigned char **green;
unsigned char **blue;
int rows;
int cols;

void RGB_Allocate(unsigned char **&dude)
{
    dude = new unsigned char *[rows];
    for (int i = 0; i < rows; i++)
        dude[i] = new unsigned char[cols];
}

bool fillAndAllocate(char *&buffer, const char *Picture, int &rows, int &cols, int &bufferSize)
{
    std::ifstream file(Picture);

    if (file)
    {
        file.seekg(0, std::ios::end);
        std::streampos length = file.tellg();
        file.seekg(0, std::ios::beg);

        buffer = new char[length];
        file.read(&buffer[0], length);

        PBITMAPFILEHEADER file_header;
        PBITMAPINFOHEADER info_header;

        file_header = (PBITMAPFILEHEADER)(&buffer[0]);
        info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
        rows = info_header->biHeight;
        cols = info_header->biWidth;
        bufferSize = file_header->bfSize;
        return 1;
    }
    else
    {
        cout << "File" << Picture << " don't Exist!" << endl;
        return 0;
    }
}

void *diamondFilter(void *tid)
{
    long thread_id = (long)tid;
    float a1 = (float)(rows / 2) / (float)(cols / 2);
    float a2 = (float)(-rows / 2) / (float)(cols / 2);
    for (int i = thread_id * ((rows) / NUMBER_OF_THREADS); i < (thread_id + 1) * ((rows) / NUMBER_OF_THREADS); i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (j * a2 + (rows / 2) + rows == i || j * a1 - (rows / 2) == i || j * a2 + (rows / 2) == i || j * a1 + (rows / 2) == i)
            {
                red[i][j] = 255;
                blue[i][j] = 255;
                green[i][j] = 255;
            }
        }
    }
    pthread_exit(NULL);
}

void diamond()
{
    pthread_t threads[NUMBER_OF_THREADS];
    int return_code;
    for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
    {
        return_code = pthread_create(&threads[tid], NULL, diamondFilter, (void *)tid);

        if (return_code)
        {
            cout << "Error! pthread_create() Failed!" << return_code << endl;
            exit(-1);
        }
    }

    for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
    {
        return_code = pthread_join(threads[tid], NULL);
        if (return_code)
        {
            cout << "Error! pthread_join() Failed!" << return_code << endl;
            exit(-1);
        }
    }
}

void *convFilter(void *tid)
{
    long thread_id = (long)tid;
    int end;
    int vec[3][3] = {
        {-2, -1, 0},
        {-1, 1, 1},
        {0, 1, 2}};

    unsigned char **redTemp;
    unsigned char **greenTemp;
    unsigned char **blueTemp;
    RGB_Allocate(redTemp);
    RGB_Allocate(greenTemp);
    RGB_Allocate(blueTemp);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            redTemp[i][j] = red[i][j];

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            greenTemp[i][j] = green[i][j];

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            blueTemp[i][j] = blue[i][j];

    for (int i = thread_id * ((rows) / NUMBER_OF_THREADS); i < (thread_id + 1) * ((rows) / NUMBER_OF_THREADS); i++)
    {
        for (int j = 0; j < cols; j++)
        {
            int redConvolute = 0;
            int blueConvolute = 0;
            int greenConvolute = 0;
            for (int dx = -1; dx <= 1; dx++)
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    if (dx + i >= 0 && dy + j >= 0 && dx + i < rows && dy + j < cols)
                    {

                        redConvolute += int(redTemp[dx + i][dy + j] - '0') * vec[dx + 1][dy + 1];
                        blueConvolute += int(blueTemp[dx + i][dy + j] - '0') * vec[dx + 1][dy + 1];
                        greenConvolute += int(greenTemp[dx + i][dy + j] - '0') * vec[dx + 1][dy + 1];
                    }
                }
            }
            if (redConvolute > 255)
                redConvolute = 255;
            else if (redConvolute < 0)
                redConvolute = 0;
            if (blueConvolute > 255)
                blueConvolute = 255;
            else if (blueConvolute < 0)
                blueConvolute = 0;
            if (greenConvolute > 255)
                greenConvolute = 255;
            else if (greenConvolute < 0)
                greenConvolute = 0;
            red[i][j] = redConvolute;
            blue[i][j] = blueConvolute;
            green[i][j] = greenConvolute;
        }
    }
    pthread_exit(NULL);
}

void convolution()
{
    pthread_t threads[NUMBER_OF_THREADS];
    int return_code;
    for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
    {
        return_code = pthread_create(&threads[tid], NULL, convFilter, (void *)tid);

        if (return_code)
        {
            cout << "Error! pthread_create() Failed!" << return_code << endl;
            exit(-1);
        }
    }

    for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
    {
        return_code = pthread_join(threads[tid], NULL);
        if (return_code)
        {
            cout << "Error! pthread_join() Failed!" << return_code << endl;
            exit(-1);
        }
    }
}

void *flipFilter(void *tid)
{
    long thread_id = (long)tid;
    int row;
    int col;

    for (row = thread_id * ((rows) / NUMBER_OF_THREADS); row < (thread_id + 1) * ((rows) / NUMBER_OF_THREADS); row++)
    {
        for (col = 0; col < cols / 2; col++)
        {
            unsigned char temp = red[row][col];
            red[row][col] = red[row][cols - col - 1];
            red[row][cols - col - 1] = temp;
        }
    }

    for (row = thread_id * ((rows) / NUMBER_OF_THREADS); row < (thread_id + 1) * ((rows) / NUMBER_OF_THREADS); row++)
    {
        for (col = 0; col < cols / 2; col++)
        {
            unsigned temp = blue[row][col];
            blue[row][col] = blue[row][cols - col - 1];
            blue[row][cols - col - 1] = temp;
        }
    }

    for (row = thread_id * ((rows) / NUMBER_OF_THREADS); row < (thread_id + 1) * ((rows) / NUMBER_OF_THREADS); row++)
    {
        for (col = 0; col < cols / 2; col++)
        {
            unsigned temp = green[row][col];
            green[row][col] = green[row][cols - col - 1];
            green[row][cols - col - 1] = temp;
        }
    }
    pthread_exit(NULL);
}

void flipHorizontally()
{
    pthread_t threads[NUMBER_OF_THREADS];
    int return_code;
    for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
    {
        return_code = pthread_create(&threads[tid], NULL, flipFilter, (void *)tid);

        if (return_code)
        {
            cout << "Error! pthread_create() Failed!" << return_code << endl;
            exit(-1);
        }
    }

    for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
    {
        return_code = pthread_join(threads[tid], NULL);
        if (return_code)
        {
            cout << "Error! pthread_join() Failed!" << return_code << endl;
            exit(-1);
        }
    }
}

void *getPixlesFromBMP24(void *tid)
{
    long thread_id = (long)tid;
    int extra = cols % 4;
    int count = (thread_id * (rows / NUMBER_OF_THREADS)) * (cols * 3 + extra) + 1;

    for (int i = thread_id * ((rows) / NUMBER_OF_THREADS); i < (thread_id + 1) * ((rows) / NUMBER_OF_THREADS); i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
            for (int k = 0; k < 3; k++)
            {
                switch (k)
                {
                case 0:
                    red[i][j] = fileBuffer[bufferSize - count++];
                    break;
                case 1:
                    green[i][j] = fileBuffer[bufferSize - count++];
                    break;
                case 2:
                    blue[i][j] = fileBuffer[bufferSize - count++];
                    break;
                }
            }
    }
    pthread_exit(NULL);
}

void getInput()
{
    pthread_t threads[NUMBER_OF_THREADS];
    int return_code;
    for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
    {
        return_code = pthread_create(&threads[tid], NULL, getPixlesFromBMP24, (void *)tid);

        if (return_code)
        {
            cout << "Error! pthread_create() Failed!" << return_code << endl;
            exit(-1);
        }
    }

    for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
    {
        return_code = pthread_join(threads[tid], NULL);
        if (return_code)
        {
            cout << "Error! pthread_join() Failed!" << return_code << endl;
            exit(-1);
        }
    }
}

void *writeOutBmp24(void *tid)
{
    std::ofstream write(OUTPUT_FILE);
    if (!write)
    {
        cout << "Failed to write " << OUTPUT_FILE << endl;
        exit(-1);
    }
    long thread_id = (long)tid;
    int extra = cols % 4;
    int count = (thread_id * (rows / NUMBER_OF_THREADS)) * (cols * 3 + extra) + 1;

    for (int i = thread_id * ((rows) / NUMBER_OF_THREADS); i < (thread_id + 1) * ((rows) / NUMBER_OF_THREADS); i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
            for (int k = 0; k < 3; k++)
            {
                switch (k)
                {
                case 0:
                    fileBuffer[bufferSize - count] = red[i][j];
                    // write red value in fileBuffer[bufferSize - count]
                    break;
                case 1:
                    fileBuffer[bufferSize - count] = green[i][j];
                    // write green value in fileBuffer[bufferSize - count]
                    break;
                case 2:
                    fileBuffer[bufferSize - count] = blue[i][j];
                    // write blue value in fileBuffer[bufferSize - count]
                    break;
                }
                count++;
            }
    }
    write.write(fileBuffer, bufferSize);
    pthread_exit(NULL);
}

void output()
{
    pthread_t threads[NUMBER_OF_THREADS];
    int return_code;
    for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
    {
        return_code = pthread_create(&threads[tid], NULL, writeOutBmp24, (void *)tid);

        if (return_code)
        {
            cout << "Error! pthread_create() Failed!" << return_code << endl;
            exit(-1);
        }
    }

    for (long tid = 0; tid < NUMBER_OF_THREADS; tid++)
    {
        return_code = pthread_join(threads[tid], NULL);
        if (return_code)
        {
            cout << "Error! pthread_join() Failed!" << return_code << endl;
            exit(-1);
        }
    }
}

int main(int argc, char *argv[])
{
    auto start = std::chrono::high_resolution_clock::now();
    char *fileName = argv[1];
    if (!fillAndAllocate(fileBuffer, fileName, rows, cols, bufferSize))
    {
        cout << "File read error" << endl;
        return 1;
    }
    RGB_Allocate(red);
    RGB_Allocate(green);
    RGB_Allocate(blue);
    auto startGetPixel = std::chrono::high_resolution_clock::now();
    getInput();
    auto endGetPixel = std::chrono::high_resolution_clock::now();
    cout << "GetPixlesFromBMP24 Time:" << std::chrono::duration_cast<std::chrono::milliseconds>(endGetPixel - startGetPixel).count() << " milliseconds" << endl;

    flipHorizontally();
    auto endFlip = std::chrono::high_resolution_clock::now();
    cout << "Flip Horizontally Time:" << std::chrono::duration_cast<std::chrono::milliseconds>(endFlip - endGetPixel).count() << " milliseconds" << endl;

    convolution();
    auto endConv = std::chrono::high_resolution_clock::now();
    cout << "Convolution Time:" << std::chrono::duration_cast<std::chrono::milliseconds>(endConv - endFlip).count() << " milliseconds" << endl;

    diamond();
    auto endDimond = std::chrono::high_resolution_clock::now();
    cout << "Dimond Time:" << std::chrono::duration_cast<std::chrono::milliseconds>(endDimond - endConv).count() << " milliseconds" << endl;

    output();
    auto endWriteOut = std::chrono::high_resolution_clock::now();
    cout << "WriteOutBmp24 Time:" << std::chrono::duration_cast<std::chrono::milliseconds>(endWriteOut - endDimond).count() << " milliseconds" << endl;

    auto end = std::chrono::high_resolution_clock::now();
    cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " milliseconds" << endl;
}