#pragma once

// Includes: This Library
#include "AQLCoreAppError.h"

//
// This class checks if start-up configuration files have been loaded using the Singleton Pattern.
//
class AQLStaticDataImport
{
private:
    
    // Private Constructor, Destructor, 
    AQLStaticDataImport() {};
    ~AQLStaticDataImport() {};
    
    // Private Copy Constructor & Assignment Operator
    AQLStaticDataImport( const AQLStaticDataImport & );
    AQLStaticDataImport & operator=( const AQLStaticDataImport & );
    
    bool isStaticDataLoaded_;

public:

    // The only way to create an instance of this class
    static AQLStaticDataImport & getInstance()
    {
        static AQLStaticDataImport theInstance;
        static AQLStaticDataImport* pInstance_ = &theInstance;
        return *pInstance_; 
    }

    bool getIsStaticDataLoaded() const									{ return isStaticDataLoaded_; }
    void setIsStaticDataLoaded( const bool & isStaticDataLoaded )		{ isStaticDataLoaded_ = isStaticDataLoaded; }
};

inline bool checkIfStaticDataLoaded( const bool & throwIfNotLoaded = true )
{
    const bool isLoaded = AQLStaticDataImport::getInstance().getIsStaticDataLoaded();
    if ( throwIfNotLoaded && ! isLoaded)
    {
        throw AQLCoreInvalidData( "#Error: Start-up Calendar- and Static Data Configuration Files have not been Loaded.", __FILE__, __LINE__ );
    }
    return isLoaded;
}
