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