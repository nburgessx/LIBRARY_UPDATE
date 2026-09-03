// CreateFixedBond.cpp

/*
 * @brief			Method to create a fixed bond from a Google Test Input File
 * @Created:		6th March 2017
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "CreateFixedBond.h"
#include "LAString.h"

namespace google_test
{
    etrading::BondPtr CreateFixedBond::createFixedBondFromInputFile( const std::string& inputFile )
    {
        if ( inputFile.size() != 0 )
        {
            try
            {
                // Load the Google Test Input File
                LAString inputFileString            = LAString( inputFile.c_str() );
                inputFile_                          = etrading::ReadDataFile::Load( inputFileString );
                
                std::string bondObjectName          = inputFile_["bondObjectName"];
                LAStringMatrix bondLVB                = inputFile_["bondLVB"];
                LAStringMatrix scheduleLVB            = inputFile_["scheduleLVB"];
                bool validateKeys                   = inputFile_["validateKeys"];

                // Build the Bond Object
                etrading::BondPtr result            = etrading::createBond( bondObjectName, LabelValueBlock( bondLVB ), LabelValueBlock( scheduleLVB ), validateKeys );
               
               return result;
            }
            catch( const LACoreError& m )
            {
                std::cout <<  m.getMsg();
            }
            catch( const std::exception& e )
            {
                std::cout << e.what();
            }
        }
		return etrading::BondPtr();
    };

	etrading::BondPtr CreateFixedBond::createFixedBondFromInputFileSingleLVB(const std::string& inputFile)
	{
		if (inputFile.size() != 0)
		{
			try
			{
				// Load the Google Test Input File
				LAString inputFileString = LAString(inputFile.c_str());
				inputFile_ = etrading::ReadDataFile::Load(inputFileString);

				std::string bondObjectName = inputFile_["bondObjectName"];
				LAStringMatrix bondLVB = inputFile_["bondLVB"];
				bool validateKeys = inputFile_["validateKeys"];

				// Build the Bond Object
				etrading::BondPtr result = etrading::createBondFromSingleLVB(bondObjectName, LabelValueBlock(bondLVB), validateKeys);

				return result;
			}
			catch (const LACoreError& m)
			{
				std::cout << m.getMsg();
			}
			catch (const std::exception& e)
			{
				std::cout << e.what();
			}
		}
		return etrading::BondPtr();
	};
}
