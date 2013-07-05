#include <iostream>
#include <cstdint>
#include <algorithm> 

using namespace std;

#pragma warning(disable: 4996) // Disable deprecation

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

// gcc -I/usr/include/opencv -lcv -lhighgui -lstdc++  main01.c -o main01

void invert_8u(IplImage** img);
void transform(IplImage** img, double angle, double scale);

int main(void) 
{
    // Bild laden & anzeigen
    IplImage *img1;
    cvNamedWindow("original image", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("processed image", CV_WINDOW_AUTOSIZE);

    if((img1 = cvLoadImage("./images/lena.jpg", CV_LOAD_IMAGE_GRAYSCALE)) == NULL) {
        cerr << "Could not open image file" << endl;
        exit(0);
    };

    // Daten ausgeben
    auto width = img1->width;
    auto height = img1->height;
    auto widthStep = img1->widthStep;
    auto channels = img1->nChannels;
    auto depth = img1->depth;
    cout << "Dimension: "  << to_string(width) << "x" << to_string(height) << endl
         << "Kanäle:    "  << to_string(channels) << "x" << to_string(depth) << endl;

    // Bild kopieren
    IplImage *img2 = cvCreateImage(cvSize(width, height), depth, channels);
    for (auto y=0; y<height; ++y) 
    {
        auto line = y*widthStep;
        for (auto x=0; x<width; ++x) 
        {
            auto pixel = line + x;
            for (auto c=0; c<channels; ++c) 
            {
                img2->imageData[pixel + c] = img1->imageData[pixel + c];
            }
        }
    }

    // Bild invertieren und rotieren
    invert_8u(&img2);
    transform(&img2, 45.0, 1.5);

    // Region of Interest / Addition
    cvSetImageROI(img2, cvRect(40,10,100,100));
    cvAddS(img2, cvScalar(50), img2);
    cvResetImageROI(img2);

    // Hellste und dunkelste Werte ermitteln
    double min, max;
    cvMinMaxLoc(img1, &min, &max, NULL, NULL, NULL);
    cout << "Werte:     " << to_string(static_cast<uint_fast8_t>(min)) << ".." << to_string(static_cast<uint_fast8_t>(max)) << endl;

    // Bild anzeigen und speichern
    cvShowImage("original image", img1);
    cvShowImage("processed image", img2);
    cvSaveImage("./mas01_lena_modified.jpg", img1);

    // Freigeben	
    cvWaitKey(0);
    cvReleaseImage(&img2);
    cvReleaseImage(&img1);
    cvDestroyWindow("original image");
    cvDestroyWindow("processed image");
    return 0;
}

// invertiert das Bild in einem bestimmten Bereich
void invert_8u(IplImage** img) 
{
    uint_fast16_t width 	= (*img)->width;
    uint_fast16_t left 	= width / 6;
    uint_fast16_t right 	= width - left;

    uint_fast16_t height 	= (*img)->height;
    uint_fast16_t top 	= height / 6;
    uint_fast16_t bottom 	= height - top;

    uint8_t* imageData	= reinterpret_cast<uint8_t*>((*img)->imageData);
    uint_fast16_t widthStep = (*img)->widthStep;

    // Zeilen durchlaufen
    for (uint_fast16_t y=top; y < bottom; ++y) 
    {
        uint_fast8_t* ptr = static_cast<uint_fast8_t*>(imageData + y * widthStep);
        
        // Spalten durchlaufen
        for (uint_fast16_t x=left; x < right; ++x) 
        {
            uint_fast8_t pixel = ptr[x];

            // Invertieren und Helligkeit erhöhen
            ptr[x] = std::min(255, 300-pixel);
        }
    }
}

// führt eine affine Transformation aus
void transform(IplImage** img, double angle, double scale)
{
    // Zielbild erzeugen
    IplImage *dst = cvCloneImage(*img);
    // printf("Dimension': %d x %d\n", dst->width, dst->height);
    // printf("Kanäle':    %d x %d bit\n", dst->nChannels, dst->depth);

    // Matrix erzeugen
    CvMat* 		rot_mat = cvCreateMat(2, 3, CV_32FC1); // 32bit, float, 1 Kanal
    CvPoint2D32f 	center = cvPoint2D32f((*img)->width / 2, (*img)->height / 2);
    cv2DRotationMatrix(center, angle, scale, rot_mat);

    // Transformieren
    cvWarpAffine(*img, dst, rot_mat);
    cvCopy(dst, *img);

    // Speicher freigeben
    cvReleaseImage(&dst);
    cvReleaseMat(&rot_mat);
}
