// CreateFixedBond.h

/*
 * @brief			Method to create a fixed bond from a Google Test Input File
 * @Created:		6th March 2017
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "InitializeAQGoogleTest.h"
#include "ReadDataFile.h"
#include "BondFactory.h"
#include <string>

namespace google_test
{
    class CreateFixedBond : public virtual google_test::InitializeAQGoogleTest
    {
    public:
        etrading::BondPtr createFixedBondFromInputFile( const std::string& inputFile );
		etrading::BondPtr createFixedBondFromInputFileSingleLVB(const std::string& inputFile);
    
    protected:
        etrading::ReadDataFile::Load inputFile_;
    
    };
}
