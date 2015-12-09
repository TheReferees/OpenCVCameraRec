//
//  main.cpp
//  Image Processing
//
//  Created by Idean Labib on 6/20/15.
//  Copyright (c) 2015 Idean Labib. All rights reserved.
//


//STEP 1: THRESHOLDING

#include "jni.h"
#include "opencv/cv.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <sstream>
#include <android/log.h>

#define APPNAME "MyApp"
//#include "CImg.h"
//#define cimg_use_magick

//using namespace cimg_library;

int y_arr[256];
int u_arr[256];
int v_arr[256];

struct Color {
    std::string name = "";
    int r;
    int g;
    int b;
};

struct region {
    int minX;
    int maxX;
    int minY;
    int maxY;
    
    int mass;

    region() {
        mass = 1;
    }
    
    Color color;
    int colorBits;
};

std::vector<int> pixelColors;

Color rgbColors[8];

std::vector<region> blobs;
int blobsCount;

inline int get_max(int x, int y) {
    return x > y ? x : y;
}

inline int get_min(int x, int y) {
    return x < y ? x : y;
}

//Returns color for a pixel's value from the bitwise operations.
inline Color colorFromBits(int colorBits) {
    return rgbColors[(int) (log(colorBits) / log(2))];
}

//Sets the YUV arrays to the correct bits by looping through the colors.txt file.
//Values at YUV indexes within the range of color are given a bit.
bool setYUVColors() {
    FILE *in;
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "LOADING....");

    in = fopen("/mnt/sdcard/camerarec/colors.txt", "r");
    if (!in) {
        std::cout << "FILE NOT FOUND" << std::endl;
        return false;
    }
    
    char buf[256];
    
    int i = 0;
    while(fgets(buf, 256, in)) {
        int y1, y2, u1, u2, v1, v2;
        int n = sscanf(buf,"(%d:%d,%d:%d,%d:%d)",&y1,&y2,&u1,&u2,&v1,&v2);
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "SCANNED COLORS: %i, %i, %i, %i, %i, %i", y1, y2, u1, u2, v1, v2);

        /*YUVtoRGB(&r1, &g1, &b1);
        YUVtoRGB(&r2, &g2, &b2);*/
        
        if (n == 6) {
            //clip values to 0-256 and add 1 bit to the front of all values in range.
            y1 = get_max(y1, 0);
            y2 = get_min(y2, 256);
            for (int j = y1; j < y2; j++) {
                y_arr[j] |= (1 << i);
            }
            u1 = get_max(u1, 0);
            u2 = get_min(u2, 256);
            for (int j = u1; j < u2; j++) {
                u_arr[j] |= (1 << i);
            }
            v1 = get_max(v1, 0);
            v2 = get_min(v2, 256);
            for (int j = v1; j < v2; j++) {
                v_arr[j] |= (1 << i);
            }
        }
        i++;
    }
    return true;
}

//load the colors
bool loadColors() {
    for (int i = 0; i < 256; i++) {
        y_arr[i] = 0;
        u_arr[i] = 0;
        v_arr[i] = 0;
    }
    
    if (!setYUVColors())
        return false;
    
    //set rgb values for colors
    rgbColors[0].r = 255;
    rgbColors[0].g = 0;
    rgbColors[0].b = 0;
    rgbColors[0].name = "Red";
    
    return true;
}

/*Find regions of certain colors (regions contain one or more labelled groups)
 KERNEL:
 _____________
 | A | B | C |
 -------------
 | D | X |
 
 LabelTable: Index is the label number, value is another label it is touching (optional)
             Intention is to map labels that touch eachother so they all point to one region
 */
void labelCells(cv::Mat& image, std::vector<unsigned int> labelBuffer, std::vector<int> * labelTable, std::vector<region> * regionsTable) {
    //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "LABEL1");
    int width = image.cols;
    int height = image.rows;
    //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "LABEL2");

    //Current label # to use
    int currLabel = 1;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "LABEL3");

            //current pixel number in one dimensional array
            int i = y * width + x;
            //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "LABEL4");

            int aLabel = (x > 0 && y > 0) ? labelBuffer[i - width - 1] : 0;
            int bLabel = (y > 0) ? labelBuffer[i - width] : 0;
            int cLabel = (x < width - 1 && y > 0) ? labelBuffer[i - width + 1] : 0;
            int dLabel = (x > 0) ? labelBuffer[i - 1] : 0;
            //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "LABEL5");

            //if the pixel matches a known color
            if (pixelColors[i] != 0) {
                //min: smallest label number of neighbors (A, B, C, D)
                int min = width * height + 1;
                //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "LABEL6");

                if (aLabel != 0 && (*regionsTable)[(*labelTable)[aLabel]].colorBits == pixelColors[i] && aLabel < min) min = aLabel;
                if (bLabel != 0 && (*regionsTable)[(*labelTable)[bLabel]].colorBits == pixelColors[i] && bLabel < min) min = bLabel;
                if (cLabel != 0 && (*regionsTable)[(*labelTable)[cLabel]].colorBits == pixelColors[i] && cLabel < min) min = cLabel;
                if (dLabel != 0 && (*regionsTable)[(*labelTable)[dLabel]].colorBits == pixelColors[i] && dLabel < min) min = dLabel;
                //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "LABEL7");

                //A, B, C, and D must be unlabeled since min is still the same value
                if (min == width * height + 1) {
                    //Give the cell the current label
                    labelBuffer[i] = currLabel;
                    //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "LABEL8");

                    //Add the new label number to the label table
                    labelTable->push_back((int) labelTable->size());
                    region r;
                    //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "LABEL9");

                    //Initialize values for region/labels
                    r.minX = x;
                    r.maxX = x;
                    r.minY = y;
                    r.maxY = y;
                    r.colorBits = pixelColors[i];
                    //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "LABEL10");

                    //add the region to the table of regions.
                    regionsTable->push_back(r);
                    
                    //increment so the next label number will be given out.
                    currLabel++;
                } else {
                    //give the cell the lowest label number of it's neighbors.
                    labelBuffer[i] = min;
                    
                    (*regionsTable)[min].maxY = y;
                    
                    (*regionsTable)[min].mass++;
                    
                    if (x > (*regionsTable)[min].maxX) (*regionsTable)[min].maxX = x;
                    if (x < (*regionsTable)[min].minX) (*regionsTable)[min].minX = x;
                    
                    //set labelTable value to smallest value of neighbors if the color is the same
                    if (aLabel != 0 && (*regionsTable)[min].colorBits == (*regionsTable)[(*labelTable)[aLabel]].colorBits) (*labelTable)[aLabel] = min;
                    if (bLabel != 0 && (*regionsTable)[min].colorBits == (*regionsTable)[(*labelTable)[bLabel]].colorBits) (*labelTable)[bLabel] = min;
                    if (cLabel != 0 && (*regionsTable)[min].colorBits == (*regionsTable)[(*labelTable)[cLabel]].colorBits) (*labelTable)[cLabel] = min;
                    if (dLabel != 0 && (*regionsTable)[min].colorBits == (*regionsTable)[(*labelTable)[dLabel]].colorBits) (*labelTable)[dLabel] = min;
                }
            }
            else {
                labelBuffer[i] = 0;
            }
        }
    }
}

void findRegions(cv::Mat& image) {
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "REGIONS0");

    int width = image.rows;
    int height = image.cols;
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "REGIONS1");

    //values correspond to indeces of regionsTable array (map of label to region)
    std::vector<int> labelTable;
	labelTable.push_back(0);
    std::vector<region> regionsTable;
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "REGIONS2: [%i, %i, %i]", width, height, width * height);

    //Contains the label number for each cell (0 for unlabelled)
    std::vector<unsigned int> labelBuffer;
    labelBuffer.resize(width * height);
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "REGIONS3");
    labelCells(image, labelBuffer, &labelTable, &regionsTable);
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "REGIONS4");

    for (int i = 1; i < (int) regionsTable.size() - 1; i++) {
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "REGIONS5");
        //if the label is not the smallest label number of the region
        if (labelTable[i] != i) {
            __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "REGIONS6");
            
            if (regionsTable[i].maxX > regionsTable[labelTable[i]].maxX) regionsTable[labelTable[i]].maxX = regionsTable[i].maxX;
            if (regionsTable[i].minX < regionsTable[labelTable[i]].minX) regionsTable[labelTable[i]].minX = regionsTable[i].minX;
            if (regionsTable[i].maxY > regionsTable[labelTable[i]].maxY) regionsTable[labelTable[i]].maxY = regionsTable[i].maxY;
            if (regionsTable[i].minY < regionsTable[labelTable[i]].minY) regionsTable[labelTable[i]].minY = regionsTable[i].minY;
        } else {
            __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "REGIONS8");
            //if mass is big enough include the region as an official blob. Otherwise exclude as background noise.
            if (regionsTable[i].mass > 50) blobs.push_back(regionsTable[i]);
        }
    }
}

//set colors for blob for highlighting them.
void setColors() {
    for (int i = 0; i < blobs.size();i++) {
        blobs[i].color = colorFromBits(blobs[i].colorBits);
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "X: %i -> %i, Y: %i -> %i, Color: %s (%i, %i, %i), Mass: %i", blobs[i].minX, blobs[i].maxX, blobs[i].minY, blobs[i].maxY, blobs[i].color.name.c_str(), blobs[i].color.r, blobs[i].color.g, blobs[i].color.b, blobs[i].mass);
        //std::cout << "X: " << blobs[i].minX << " -> " << blobs[i].maxX << ", Y: " << blobs[i].minY << " -> " << blobs[i].maxY << ", Color: " << blobs[i].color.name << " (" << blobs[i].color.r << ", " << blobs[i].color.g << ", " << blobs[i].color.b << ")" << ", Mass: " << blobs[i].mass << std::endl;
    }
}

//merge two regions
region mergeBlobs(region first, region second) {
    region merged;
    
    merged.colorBits = first.colorBits;
    merged.minX = get_min(first.minX, second.minX);
    merged.minY = get_min(first.minY, second.minY);
    merged.maxX = get_max(first.maxX, second.maxX);
    merged.maxY = get_max(first.maxY, second.maxY);
    merged.mass = first.mass + second.mass;
    
    return merged;
}

bool checkDensity(region first, region second, int threshold) {
    double originalDensity = first.mass / ((first.maxX - first.minX) * (first.maxY - first.minY));
    double newDensity = (double) (first.mass + second.mass) / ((get_max(first.maxX, second.maxX) - get_min(first.minX, second.minX)) * (get_max(first.maxY, second.maxY) - get_min(first.minY, second.minY)));
    return newDensity - originalDensity >= 0 || originalDensity - newDensity < threshold;
}

//Merge blobs with similar densities into the same shape
void mergeDensities() {
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "MERGE0");
    for (int i = 0; i < (int) blobs.size() - 1;i++) {
        for (int j = i + 1; j < blobs.size(); j++) {
            __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "MERGE1");
            int first_x = (blobs[i].minX + blobs[i].maxX) / 2;
            int first_y = (blobs[i].minY + blobs[i].maxY) / 2;
            int second_x = (blobs[j].minX + blobs[j].maxX) / 2;
            int second_y = (blobs[j].minY + blobs[j].maxY) / 2;
            int maxDistanceX = first_x - blobs[i].minX + second_x - blobs[j].minX + 10;
            int maxDistanceY = first_y - blobs[i].minY + second_y - blobs[j].minY + 10;
            
            __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "MERGE2");

            //TODO: Check Distance!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Overlapping won't be same item........
            if (std::abs(first_x - second_x) < maxDistanceX && std::abs(first_y - second_y) < maxDistanceY && blobs[i].colorBits == blobs[j].colorBits && checkDensity(blobs[i], blobs[j], 1)) {
                blobs[i] = mergeBlobs(blobs[i], blobs[j]);
                blobs.erase(blobs.begin() + j);
            }
            __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "MERGE3");
        }
    }
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "MERGE4");
}

//Detect the colors by confirming the color is within the ranges in the Y, U, and V arrays.
void detectColors(cv::Mat& image) {
    /*std::vector<char> colors;
    colors.resize(image.cols * image.rows * 4);*/
    //int i = 0;
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            int color_y = image.at<cv::Vec3b>(y, x)[0];
            int color_u = image.at<cv::Vec3b>(y, x)[2];
            int color_v = image.at<cv::Vec3b>(y, x)[1];
            /*colors[i] = (char) color_r;
            colors[i + 1] = (char) color_g;
            colors[i + 2] = (char) color_b;
            colors[i + 3] = (char) 255;
            i += 4;*/

            int colorBits = (y_arr[color_y] & u_arr[color_u]) & v_arr[color_v];
            
            pixelColors.push_back(colorBits);
        }
    }
}


//Detect the colors, group regions and return the highlighted result.
/*template<typename T> CImg<T> changeImage(CImg<T> image) {
    CImg<T> changed(image.width(), image.height(), 1, 3, 0);
    CImg<T> converted = image.get_RGBtoYCbCr();
    
    changed = image;

    detectColors(converted);
    
    findRegions(image);
    
    mergeDensities();
    
    setColors();
    
    for (int i = 0; i < blobs.size();i++) {
        int minX = blobs[i].minX;
        int maxX = blobs[i].maxX;
        int minY = blobs[i].minY;
        int maxY = blobs[i].maxY;
        
        for (int x = minX; x <= maxX;x++) {
            changed(x, minY, 0) = blobs[i].color.r;
            changed(x, minY, 1) = blobs[i].color.g;
            changed(x, minY, 2) = blobs[i].color.b;
            
            changed(x, maxY, 0) = blobs[i].color.r;
            changed(x, maxY, 1) = blobs[i].color.g;
            changed(x, maxY, 2) = blobs[i].color.b;
            
            for (int y = minY; y <= maxY; y++) {
                changed(minX, y, 0) = blobs[i].color.r;
                changed(minX, y, 1) = blobs[i].color.g;
                changed(minX, y, 2) = blobs[i].color.b;
                
                changed(maxX, y, 0) = blobs[i].color.r;
                changed(maxX, y, 1) = blobs[i].color.g;
                changed(maxX, y, 2) = blobs[i].color.b;
            }
        }
    }
    
    return changed;
} */

//Create image with borders around blobs
std::vector<char> createTestImage(cv::Mat& image) {
    std::vector<char> testImage;
    long byteSize = image.cols * image.rows * 4;
    testImage.resize(byteSize);
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "TESTIMAGE0");
    int i = 0;
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            int color_r = image.at<cv::Vec4b>(y, x)[0];
            int color_g = image.at<cv::Vec4b>(y, x)[1];
            int color_b = image.at<cv::Vec4b>(y, x)[2];
            testImage[i] = (char) color_r;
            testImage[i + 1] = (char) color_g;
            testImage[i + 2] = (char) color_b;
            testImage[i + 3] = (char) 255;
            i += 4;
        }
    }
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "TESTIMAGE4: %i", blobs.size());
    
    for (int i = 0; i < blobs.size(); i++) {
        int minX = blobs[i].minX;
        int maxX = blobs[i].maxX;
        int minY = blobs[i].minY;
        int maxY = blobs[i].maxY;
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "TESTIMAGE5");
        for (int x = minX; x <= maxX; x++) {
            //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "TESTIMAGE6");
            testImage[3*(minY * image.cols + x)] = blobs[i].color.r;
            testImage[3*(minY * image.cols + x) + 1] = blobs[i].color.g;
            testImage[3*(minY * image.cols + x) + 2] = blobs[i].color.b;

            testImage[3*(maxY * image.cols + x) + 0] = blobs[i].color.r;
            testImage[3*(minY * image.cols + x) + 1] = blobs[i].color.g;
            testImage[3*(minY * image.cols + x) + 2] = blobs[i].color.b;
            //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "TESTIMAGE7");
        }
        for (int y = minY; y <= maxY; y++) {
            //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "TESTIMAGE8");
            testImage[3*(y * image.cols + minX)] = blobs[i].color.r;
            testImage[3*(y * image.cols + minX) + 1] = blobs[i].color.g;
            testImage[3*(y * image.cols + minX) + 2] = blobs[i].color.b;
            //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "TESTIMAGE9");
            testImage[3*(y * image.cols + maxX) + 0] = blobs[i].color.r;
            testImage[3*(y * image.cols + maxX) + 1] = blobs[i].color.g;
            testImage[3*(y * image.cols + maxX) + 2] = blobs[i].color.b;
        }
    }
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "TESTIMAGE10");
    return testImage;
}

cv::Mat RGBtoYUV(cv::Mat& image) {
    cv::Mat converted = image.clone();
    cv::cvtColor(image, converted, CV_BGR2YCrCb, 3);
    return converted;
}

//Get the blobs for the Java
std::vector<char> getBlobs(cv::Mat& image) {
    cv::Mat converted = RGBtoYUV(image);
    loadColors();
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "HERE0");
    detectColors(converted);
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "HERE1");

    findRegions(converted);
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "HERE2");

    mergeDensities();
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "HERE3");

    setColors();
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "HERE4");
    
    std::vector<char> byteArray = createTestImage(image);
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "HERE5");
    return byteArray;
}

//returns array of arrays which are in this format: [[minX, maxX, minY, maxY, mass, colorBits]]
extern "C" {
    JNIEXPORT jobjectArray JNICALL Java_com_thereferees_opencvcamerarec_MainActivity_findObjects( JNIEnv* env, jobject, jlong addrImage) {
        cv::Mat* image = (cv::Mat*)addrImage;
        getBlobs(*image);

        jintArray temp = env->NewIntArray(0);
        jobjectArray returnedObjects = env->NewObjectArray(blobs.size(), env->GetObjectClass(temp), NULL);


        for (int i = 0; i < blobs.size(); i++) {
            region* b = &blobs[i];

            jintArray blob = env->NewIntArray(6);
            jint blobArray[6] = {b->minX, b->maxX, b->minY, b->maxY, b->mass, b->colorBits};

            env->SetIntArrayRegion(blob, 0, 6, blobArray);
            env->SetObjectArrayElement(returnedObjects, i, blob);
            __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "RETURNED: [%i, %i, %i, %i, %i, %i]", b->minX, b->maxX, b->minY, b->maxY, b->mass, b->colorBits);
        }

        return returnedObjects;
    }
    
    JNIEXPORT jbyteArray JNICALL Java_com_thereferees_opencvcamerarec_MainActivity_testObjects( JNIEnv* env, jobject, jlong addrImage) {
        cv::Mat* image = (cv::Mat*)addrImage;
        std::vector<char> charArray = getBlobs(*image);
        
        jbyte* byteArray = new jbyte[image->total() * 4];
        for (int i = 0; i < image->total() * 4; i++) {
            byteArray[i] = ((charArray)[i]);
        }
        jbyteArray pixelArray = env->NewByteArray(image->total() * 4);
        env->SetByteArrayRegion(pixelArray, 0, image->total() * 4, byteArray);
        delete [] byteArray;
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "DONE!!!!! %i", image->total());
        return pixelArray;
    }

    /*JNIEXPORT jbyteArray JNICALL Java_com_thereferees_opencvcamerarec_MainActivity_detectColors( JNIEnv* env, jobject, jlong addrImage) {
        cv::Mat* image = (cv::Mat*)addrImage;
        detectColors(*image);

        jbyte* byteArray = new jbyte[image->total() * 4];
        for (int i = 0; i < image->total() * 4; i++) {
            byteArray[i] = ((colors)[i]);
        }
        jbyteArray colorArray = env->NewByteArray(image->total() * 4);
        env->SetByteArrayRegion(colorArray, 0, image->total() * 4, byteArray);
        delete [] byteArray;
        return colorArray;
    }*/
}

int main(){
    /*Mat mat;
    long addrImage = &mat;
    cv::Mat* image = (cv::Mat*)addrImage;
     
    loadColors();

    getBlobs(*image); */
    return 0;
}
/*int main(int argc, const char * argv[]) {
    cimg::imagemagick_path("/usr/local/bin/convert");
    
    if (!loadColors())
        return 1;
    
    CImg<double> image("hydrant.jpg");
    CImgDisplay main_disp(image,"Original",0);
    
    CImg<double> colors = changeImage(image);
    CImgDisplay colors_disp(colors,"Original",0);
    
    while (!main_disp.is_closed()) {
        main_disp.wait();
    } 
    
    return 0;
} */
