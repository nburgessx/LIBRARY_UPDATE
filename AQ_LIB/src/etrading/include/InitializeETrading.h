#pragma once

#include "AQLDataInstance.h"
#include "AQLString.h"
#include <memory>

// Forward Declaration
class CurveCalibrationData;

namespace etrading
{

	//
	// CLASS
	//    InitializeETrading
	//
	// PURPOSE
	//    To ensure initialisation of AlgoQuantLib DataInstance, Object Pool and Yield Curve Properties
	// 
	// SYNOPSIS
	//    #include "InitializeETrading.h"
	// 
	//    InitializeETrading& instance_ = instance();
	//    const DoubleArray res = etrading::LACurveCalibrationHelpers::outPutCurveFromYieldData( instance_.getDataInstance(), curveId, marketName );
	// 
	class InitializeETrading
	{
	public:
		
        // Singleton: Library Set-Up and Tear-down Methods
        static InitializeETrading& instance(const bool checkStaticDataLoaded = false, const bool checkIfCalendarLoaded = false); // Set-Up
        static void destroyInstance(); // Tear-Down

        // Re-initialisation (used by google tests)
		~InitializeETrading();

		// Accessors
		AQLDataInstance* dataInstance()  { return dataInstance_.get(); }
        CurveCalibrationData* ycStaticDataObject( const AQLString& curveCollection, const bool throwIfCurveDoesNotExist = true );

	protected:
		InitializeETrading(const bool checkStaticDataLoaded = false, const bool checkIfCalendarLoaded = false);  

	private:
		
        // Disable Copying & Assignment
        InitializeETrading(const InitializeETrading& rhs) = delete;
        InitializeETrading& operator=(const InitializeETrading& rhs) = delete;

        // Private Member Variables
		std::unique_ptr<AQLDataInstance> dataInstance_;
        static InitializeETrading* instance_;
	};
}

