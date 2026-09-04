// CreateFixedBond.h

/*
 * @brief			Method to create a fixed bond from a Google Test Input File
 */

#pragma once

#include "InitializeGoogleTest.h"
#include "ReadDataFile.h"
#include "BondFactory.h"
#include <string>

namespace google_test
{
    class CreateFixedBond : public virtual google_test::InitializeGoogleTest
    {
    public:
        etrading::BondPtr createFixedBondFromInputFile( const std::string& inputFile );
		etrading::BondPtr createFixedBondFromInputFileSingleLVB(const std::string& inputFile);
    
    protected:
        etrading::ReadDataFile::Load inputFile_;
    
    };
}
