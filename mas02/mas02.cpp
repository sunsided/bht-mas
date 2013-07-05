#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>

#include "Application.h"

/// <summary>
/// Main entry point
/// </summary>
/// <returns>int.</returns>
int main(void) 
{
    using namespace std;

    try 
    {
        unique_ptr<Application> application(new Application());
        application->run();
    }
    catch(exception& e)
    {
        cerr << "Ein Fehler ist aufgetreten: " << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        cerr << "Ein unbekannter Fehler ist aufgetreten." << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
