/*
 * @brief			Class that deals with library initialization
 *					It was ported over from InitializeMLibVanilla.h/.cpp.
 * @Created:		14 November 2016
 * @Author:			
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

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
	//    InitializeMLibETrading
	//
	// PURPOSE
	//    To ensure initialisation of MLib DataInstance, Object Pool and Yield Curve Properties
	// 
	// SYNOPSIS
	//    #include "InitializeMLibETrading.h"
	// 
	//    InitializeMLibETrading& instance_ = instance();
	//    const DoubleArray res = etrading::LACurveCalibrationHelpers::outPutCurveFromYieldData( instance_.getDataInstance(), curveId, marketName );
	// 
	class InitializeMLibETrading
	{
	public:
		
        // Singleton: Library Set-Up and Tear-down Methods
        static InitializeMLibETrading& instance(const bool checkStaticDataLoaded = false, const bool checkIfCalendarLoaded = false); // Set-Up
        static void destroyInstance(); // Tear-Down

        // Re-initialisation (used by google tests)
		~InitializeMLibETrading();

		// Accessors
		LADataInstance* dataInstance()  { return dataInstance_.get(); }
        CurveCalibrationData* ycStaticDataObject( const LAString& curveCollection, const bool throwIfCurveDoesNotExist = true );

	protected:
		InitializeMLibETrading(const bool checkStaticDataLoaded = false, const bool checkIfCalendarLoaded = false);  

	private:
		
        // Disable Copying & Assignment
        InitializeMLibETrading(const InitializeMLibETrading& rhs) = delete;
        InitializeMLibETrading& operator=(const InitializeMLibETrading& rhs) = delete;

        // Private Member Variables
		std::unique_ptr<LADataInstance> dataInstance_;
        static InitializeMLibETrading* instance_;
	};
}

