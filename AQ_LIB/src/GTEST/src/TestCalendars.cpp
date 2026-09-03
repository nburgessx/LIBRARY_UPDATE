#include "ExceptionMacros.h"
#include "TestHelperUtilities.h"
#include "DateUtilities.h"
#include "tryMeDate.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/split.hpp>

// Include: Google Test Library
#include <gTest/gTest.h>

namespace google_test
{
	const std::string fileName_MLIB_Calendar		= "Calendar.csv";
	const std::string fileName_MLIB_CalendarConfig	= "Calendar.conf";

	const std::string warningTenor					= "6M";
	const std::string errorTenor					= "12M";


	namespace
	{
		// Helper functions

		boost::filesystem::path getConfigFolderPath()
		{
			// Config data lives in the source tree at $(AQ)\resources\config.
			const char* environmentVariablePath = std::getenv( "AQ" );

			if ( environmentVariablePath == nullptr )
			{
				throw AQLCoreInvalidData("#Error: The 'AQ' environment variable has not been set.", __FILE__, __LINE__);
			}

			boost::filesystem::path resultPath = boost::filesystem::path(environmentVariablePath) / "resources" / "config";
			return resultPath;
		}

		std::string resolveFilename( const std::string& filename )
		{
			boost::filesystem::path p( filename.c_str() );

			// If a full file path is specified use it, otherwise use the pre-defined folder for the path as specified in etrading::FolderConfig
			if ( ! p.is_absolute() )
			{
				p = getConfigFolderPath() / p;
			}
			return p.string();
		}

		AQLDate loadCalendarConfig( const std::string& calendarConfigFile )
		{
			std::string resolvedfilename = resolveFilename(calendarConfigFile);

			std::ifstream fin;
			fin.open(resolvedfilename);
			if (!fin)
			{
				AQ_THROW( "Unable to locate Calendar.conf file: " + resolvedfilename );
			}

			std::string line;
			size_t line_num = 0;
			while (getline(fin, line))
			{
				line_num++;

				/* The format of each line in the calendar configuration file is Key,date
				* or it can be a comment, indicated by '#' at the start of the line
				*/
				std::string trimmedLine( line );
				boost::trim( trimmedLine );

				// skip over blank lines
				if ( trimmedLine.size() == 0 )
				{
					continue;
				}

				// skip over comments
				if ( trimmedLine[0] == '#' )
				{
					continue;
				}

				std::string delimiter(",");
				std::vector<std::string> tokens;
				boost::algorithm::split( tokens, trimmedLine, boost::algorithm::is_any_of( delimiter ));

				if (tokens.size() != 2)
				{
					AQ_THROW("Expected two columns of config data: key, value");
				}

				std::string key = tokens[0];
				std::string dateString = tokens[1];
				AQLDate lastCalendarUpdate = AQLDate( dateString.c_str() );
				fin.close();
				return lastCalendarUpdate;
				break;
			}
			fin.close();

			AQ_THROW("Did not find valid data inside the Calendar.conf file");
		}
	}


	TEST(Calendars, UNIT_Expiry_Test)
	{
		const AQLDate currentDate = etrading::getCurrentMLibDate();
		
		const AQLDate lastCalendarUpdate = loadCalendarConfig( fileName_MLIB_CalendarConfig );

		const std::string resolvedCalendarFile     = resolveFilename(fileName_MLIB_Calendar);
		const std::string resolvedCalendarConfFile = resolveFilename(fileName_MLIB_CalendarConfig);

		// Calculate the date beyond which the calendars are considered so old we issue an error
		const std::string dummyBusinessDayAdjust;
		const std::string dummyCalendar;
		const std::string dummyRollConvention;
		const AQLDate errorCutoffDate = validation::tryMeDateFromTenor( lastCalendarUpdate, errorTenor, dummyBusinessDayAdjust, dummyCalendar, dummyRollConvention );

		if ( currentDate > errorCutoffDate )
		{
			AQ_THROW( "According to config file: " + resolvedCalendarConfFile +
						", the calendar file: " + resolvedCalendarFile +
						" is more than " + errorTenor + 
						" old, Please regenerate it using the tool CDWCalendarUpdate.bat." );
		}

		const AQLDate warningCutoffDate = validation::tryMeDateFromTenor(lastCalendarUpdate, warningTenor, dummyBusinessDayAdjust, dummyCalendar, dummyRollConvention);

		if ( currentDate > warningCutoffDate )
		{
			GTEST_WARNING(  "According to config file: " + resolvedCalendarConfFile +
							", the calendar file: " + resolvedCalendarFile +
							" is more than " + warningTenor +
							" old. Please consider regenerating it using the tool CDWCalendarUpdate.bat.");
		}

	}

}
