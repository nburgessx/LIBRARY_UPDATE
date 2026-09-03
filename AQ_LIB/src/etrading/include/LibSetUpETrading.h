//#pragma once

class AQLDataInstance;
class AQLString;

//namespace etrading
//{
	void libSetUpETrading( AQLDataInstance* dataInstance, const bool checkIfCalendarLoaded = false );

	void setupCalendarETrading( const AQLString* file_path = nullptr, const bool enableThrow = false );
	
	// load (reload) central bank schedule dates from given file
	void setupCBScheduleETrading( const AQLString* file_path = nullptr, const bool enableThrow = false );
//}