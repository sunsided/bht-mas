#pragma warning(push) // Disable deprecation
#pragma warning(disable: 4996) // Disable deprecation

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include "OpenCvImage.h"

#pragma warning(pop) // enable deprecation

#include "Application.h"

using namespace std;

/// <summary>
/// Initializes a new instance of the <see cref="Application"/> class.
/// </summary>
Application::Application() {}
    
/// <summary>
/// Finalizes an instance of the <see cref="Application"/> class.
/// </summary>
Application::~Application() 
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
void Application::createWindow(const string& name)
{
    _windowNames.push_back(name);
    cvNamedWindow(name.c_str(), CV_WINDOW_AUTOSIZE);
}

/// <summary>
/// Runs this instance.
/// </summary>
void Application::run()
{
    cvWaitKey(0);
}