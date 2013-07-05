#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <string>
#include <vector>

/// <summary>
/// Main application class
/// </summary>
class Application 
{
private:
    /// <summary>
    /// Vector of all created window names
    /// </summary>
    std::vector<const std::string> _windowNames;

public:
    /// <summary>
    /// Initializes a new instance of the <see cref="Application"/> class.
    /// </summary>
    Application();
    
    /// <summary>
    /// Finalizes an instance of the <see cref="Application"/> class.
    /// </summary>
    ~Application() ;

    /// <summary>
    /// Runs this instance.
    /// </summary>
    void run();

    /// <summary>   Creates a window. </summary>
    ///
    /// <remarks>   Markus, 05.07.2013. </remarks>
    ///
    /// <param name="name"> [in] The name. </param>
    inline void createWindow(const char* name)
    {
        createWindow(std::string(name));
    }

    /// <summary>   Creates a window. </summary>
    ///
    /// <remarks>   Markus, 05.07.2013. </remarks>
    ///
    /// <param name="name"> [in] The name. </param>
    void createWindow(const std::string& name);
};

#endif