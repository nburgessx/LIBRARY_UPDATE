#pragma once

// Includes: This Library
#include "LACoreAppError.h"

//
// This class checks if start-up configuration files have been loaded using the Singleton Pattern.
//
class LAStaticDataImport
{
private:
    
    // Private Constructor, Destructor, 
    LAStaticDataImport() {};
    ~LAStaticDataImport() {};
    
    // Private Copy Constructor & Assignment Operator
    LAStaticDataImport( const LAStaticDataImport & );
    LAStaticDataImport & operator=( const LAStaticDataImport & );
    
    bool isStaticDataLoaded_;

public:

    // The only way to create an instance of this class
    static LAStaticDataImport & getInstance()
    {
        static LAStaticDataImport theInstance;
        static LAStaticDataImport* pInstance_ = &theInstance;
        return *pInstance_; 
    }

    bool getIsStaticDataLoaded() const									{ return isStaticDataLoaded_; }
    void setIsStaticDataLoaded( const bool & isStaticDataLoaded )		{ isStaticDataLoaded_ = isStaticDataLoaded; }
};

inline bool checkIfStaticDataLoaded( const bool & throwIfNotLoaded = true )
{
    const bool isLoaded = LAStaticDataImport::getInstance().getIsStaticDataLoaded();
    if ( throwIfNotLoaded && ! isLoaded)
    {
        throw LACoreInvalidData( "#Error: Start-up Calendar- and Static Data Configuration Files have not been Loaded.", __FILE__, __LINE__ );
    }
    return isLoaded;
}
