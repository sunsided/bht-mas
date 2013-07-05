#include <algorithm> 
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

#pragma warning(disable: 4996) // Disable deprecation

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#include "OpenCvImage.h"

void invert_8u(IplImagePtr& img);
void transform(IplImagePtr& img, double angle, double scale);

/// <summary>
/// Main application class
/// </summary>
class Application 
{
private:
    /// <summary>
    /// Vector of all created window names
    /// </summary>
    vector<const string> _windowNames;

public:
    /// <summary>
    /// Initializes a new instance of the <see cref="Application"/> class.
    /// </summary>
    Application() {}
    
    /// <summary>
    /// Finalizes an instance of the <see cref="Application"/> class.
    /// </summary>
    ~Application() 
    {
        // close all windows
        for(string& name: _windowNames)
        {
            cvDestroyWindow(name.c_str());
        }

        // purge window names
        _windowNames.clear();
    }

    /// <summary>   Creates a window. </summary>
    ///
    /// <remarks>   Markus, 05.07.2013. </remarks>
    ///
    /// <param name="name"> [in,out] The name. </param>
    void createWindow(const char* name)
    {
        createWindow(string(name));
    }

    /// <summary>   Creates a window. </summary>
    ///
    /// <remarks>   Markus, 05.07.2013. </remarks>
    ///
    /// <param name="name"> [in,out] The name. </param>
    void createWindow(const string& name)
    {
        _windowNames.push_back(name);
        cvNamedWindow(name.c_str(), CV_WINDOW_AUTOSIZE);
    }
};

int main(void) 
{
    unique_ptr<Application> application(new Application());
    application->createWindow("trololo");
    cvWaitKey(0);

    /*
    // Fenster erzeugen
    cvNamedWindow("original image", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("processed image", CV_WINDOW_AUTOSIZE);

    // Bild laden & anzeigen
    IplImagePtr img1(cvLoadImage("./images/lena.jpg", CV_LOAD_IMAGE_GRAYSCALE));
    if(img1.get() == NULL) {
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
    IplImagePtr img2(cvCreateImage(cvSize(width, height), depth, channels));
    if(img1.get() == NULL) {
        cerr << "Could not create working copy" << endl;
        exit(0);
    };

    // naive image copy
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
    invert_8u(img2);
    transform(img2, 45.0, 1.5);

    // Region of Interest / Addition
    cvSetImageROI(img2.get(), cvRect(40,10,100,100));
    cvAddS(img2.get(), cvScalar(50), img2.get());
    cvResetImageROI(img2.get());

    // Hellste und dunkelste Werte ermitteln
    double min, max;
    cvMinMaxLoc(img1.get(), &min, &max, NULL, NULL, NULL);
    cout << "Werte:     " << to_string(static_cast<uint_fast8_t>(min)) << ".." << to_string(static_cast<uint_fast8_t>(max)) << endl;

    // Bild anzeigen und speichern
    cvShowImage("original image", img1.get());
    cvShowImage("processed image", img2.get());
    cvSaveImage("./mas01_lena_modified.jpg", img1.get());

    // Freigeben	
    cvWaitKey(0);
    cvDestroyWindow("original image");
    cvDestroyWindow("processed image");
    */

    return 0;
}

// invertiert das Bild in einem bestimmten Bereich
void invert_8u(IplImagePtr& img) 
{
    uint_fast16_t width 	= img->width;
    uint_fast16_t left 	= width / 6;
    uint_fast16_t right 	= width - left;

    uint_fast16_t height 	= img->height;
    uint_fast16_t top 	= height / 6;
    uint_fast16_t bottom 	= height - top;

    uint8_t* imageData	= reinterpret_cast<uint8_t*>(img->imageData);
    uint_fast16_t widthStep = img->widthStep;

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
void transform(IplImagePtr& img, double angle, double scale)
{
    // Zielbild erzeugen
    IplImage *dst = cvCloneImage(img.get());
    // printf("Dimension': %d x %d\n", dst->width, dst->height);
    // printf("Kanäle':    %d x %d bit\n", dst->nChannels, dst->depth);

    // Matrix erzeugen
    CvMat* 		rot_mat = cvCreateMat(2, 3, CV_32FC1); // 32bit, float, 1 Kanal
    CvPoint2D32f 	center = cvPoint2D32f((img)->width / 2, (img)->height / 2);
    cv2DRotationMatrix(center, angle, scale, rot_mat);

    // Transformieren
    cvWarpAffine(img.get(), dst, rot_mat);
    cvCopy(dst, img.get());

    // Speicher freigeben
    cvReleaseImage(&dst);
    cvReleaseMat(&rot_mat);
}
