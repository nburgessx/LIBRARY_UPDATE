/*
 * @brief			Methods that assist in library initialization
 *					It was ported over from InitializeMLibVanilla.h/.cpp.
 * @Created:		14 November 2016
 * @Author:			
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

//#pragma once

class LADataInstance;
class LAString;

//namespace etrading
//{
	void libSetUpETrading( LADataInstance* dataInstance, const bool checkIfCalendarLoaded = false );

	void setupCalendarETrading( const LAString* file_path = nullptr, const bool enableThrow = false );
	
	// load (reload) central bank schedule dates from given file
	void setupCBScheduleETrading( const LAString* file_path = nullptr, const bool enableThrow = false );
//}