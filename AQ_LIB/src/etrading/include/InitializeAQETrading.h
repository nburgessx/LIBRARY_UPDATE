#pragma once

#include "LADataInstance.h"
#include "LAString.h"
#include <memory>

// Forward Declaration
class CurveCalibrationData;

namespace etrading
{

	//
	// CLASS
	//    InitializeAQETrading
	//
	// PURPOSE
	//    To ensure initialisation of AlgoQuantLib DataInstance, Object Pool and Yield Curve Properties
	// 
	// SYNOPSIS
	//    #include "InitializeAQETrading.h"
	// 
	//    InitializeAQETrading& instance_ = instance();
	//    const DoubleArray res = etrading::LACurveCalibrationHelpers::outPutCurveFromYieldData( instance_.getDataInstance(), curveId, marketName );
	// 
	class InitializeAQETrading
	{
	public:
		
        // Singleton: Library Set-Up and Tear-down Methods
        static InitializeAQETrading& instance(const bool checkStaticDataLoaded = false, const bool checkIfCalendarLoaded = false); // Set-Up
        static void destroyInstance(); // Tear-Down

        // Re-initialisation (used by google tests)
		~InitializeAQETrading();

		// Accessors
		LADataInstance* dataInstance()  { return dataInstance_.get(); }
        CurveCalibrationData* ycStaticDataObject( const LAString& curveCollection, const bool throwIfCurveDoesNotExist = true );

	protected:
		InitializeAQETrading(const bool checkStaticDataLoaded = false, const bool checkIfCalendarLoaded = false);  

	private:
		
        // Disable Copying & Assignment
        InitializeAQETrading(const InitializeAQETrading& rhs) = delete;
        InitializeAQETrading& operator=(const InitializeAQETrading& rhs) = delete;

        // Private Member Variables
		std::unique_ptr<LADataInstance> dataInstance_;
        static InitializeAQETrading* instance_;
	};
}

