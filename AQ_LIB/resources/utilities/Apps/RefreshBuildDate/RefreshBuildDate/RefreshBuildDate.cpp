// A little utility to search for dates in an XLL and update them to today's date.

#include "stdafx.h"

#include <time.h>
#include <ctime>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>



void writeFile( const std::string& updatedDLLPath, const std::string& fileContent )
{
	const char* filename = updatedDLLPath.c_str();
	std::ofstream file( filename, std::ios::binary );
    if ( file )
    {
		std::cout << "Writing to new file" << std::endl;
		file << fileContent;
		file.close();
	}
}

// http://stackoverflow.com/questions/6487693/code-for-searching-for-a-std::string-in-a-binary-file


std::string readBinaryFile( const std::string& inputDLLPath )
{
	std::string fileContent;

	const char* filename = inputDLLPath.c_str();

	std::ifstream file( filename, std::ios::binary );
    if ( file )
    {
        file.seekg( 0, std::ios::end );
        size_t file_size = file.tellg();
        file.seekg( 0, std::ios::beg );
        fileContent.reserve( file_size );
        char buffer[16384];
        std::streamsize chars_read;

        while ( file.read( buffer, sizeof buffer), chars_read = file.gcount() )
            fileContent.append( buffer, chars_read );

		file.close();
	}
	else
	{
		std::cerr << "The specified file does not exist! You gave me filename: " << inputDLLPath << std::endl;
	}

	return fileContent;
}

std::string::size_type findStringInFileContent( const std::string& fileContent, const std::string& searchString, const std::string::size_type startingPosition )
{
	std::string::size_type found_at = std::string::npos;

	found_at = fileContent.find( searchString, startingPosition );
	if ( found_at != std::string::npos )
	{
		std::cout << "Found search string at: 0x" << std::hex << found_at << std::endl;
	}

	return found_at;
}

void replaceStringInFileContent( const std::string& buildDate, const std::string& todayDate, std::string& fileContent )
{
	std::string::size_type location = 0;

	location = findStringInFileContent( fileContent, buildDate, location );

	// The build date may occur more than once in the file
	// In fact we expect the build date to be compiled into the DLL exactly twice

	int count = 0;
	while( true )
	{
		location = fileContent.find( buildDate );

		if ( location == std::string::npos )
		{
			break;
		}
		std::cout << "Found Build Date at: 0x" << std::hex << location << std::endl;
		fileContent.replace( location, todayDate.size(), todayDate );
		count++;

		if (count > 4)
		{
			std::cerr << "#ERROR: Something went wrong. Found more than four instances of the build date in the DLL." << std::endl;
			std::cerr << "Are you sure you gave me an original DLL ? " << std::endl;
			exit(1);
		}
	}

	std::cout << "Replaced " << count << " instances of \"" << buildDate << "\" with \"" << todayDate << "\"" << std::endl;
}


/*
 * Search for the __DATE__ inside getExpireYearMonth() function inside meUtilities.cpp. This location is indicated
 * by the search string "Jan\0Feb\0Mar\0Apr\0", which is the compiled version of the if statement:
 * "if (mstr == "Jan") m = 1; else if ... "
 *
 * This search pattern works on XLLs up to 2 May 2017, after which the code in getExpireYearMonth() was refactored.
 * See below for a version2 search pattern.
 */
std::string findBuildDateInFileContent( const std::string& fileContent, const std::string& yearToSearchFor, std::string::size_type& location )
{
	location = findStringInFileContent( fileContent, yearToSearchFor, location );

	std::string potentialBuildDate;

	// Now read the build date which will be 12 bytes before the std::string match
	if ( location != std::string::npos )
	{
		/* Location will be the point where the year string starts.
		 * The actual date candidate will start 7 bytes before this
		 * 76543210
		 * MMM DD YYYY
		 * 0123456789A
		 */
		potentialBuildDate = fileContent.substr( location - 7, 11 );
	}
 
	return potentialBuildDate;
}



std::string getCurrentDateAsString()
{
	time_t current;
	time(&current);
	struct tm* timeinfo;
	timeinfo = localtime(&current);

	char  todayDate[20];

	//   0123456789A
	//   MMM DD YYYY
	//  "Feb 07 2017"
	std::strftime( todayDate, 20, "%b %d %Y", timeinfo );

	std::string currentDateAsString( todayDate );
	std::cout << "Today's date: " << currentDateAsString << std::endl;

	return currentDateAsString;
}

void findCandidateYearAndUpdateBuildDates( std::string& fileContent,  const std::string& yearToSearchFor, std::string::size_type& location )
{

	// Keep searching until we find a valid builddate, or reach end of file
	std::string potentialBuildDate = "";
	bool validBuildDate = false;
	while (! validBuildDate )
	{
		potentialBuildDate = findBuildDateInFileContent( fileContent, yearToSearchFor, location );

		if ( potentialBuildDate.empty() )
		{
			std::cerr << "Did not find the year " << yearToSearchFor << " in the file. " << std::endl;
			return;
		}

		std::cout << "Located a BuildDate candidate based on year string: " << potentialBuildDate << std::endl;

		// Perform some basic sanity checking on the build date.
		// We are expecting a date in the format:  "MMM DD YYYY"  or "MMM  D YYYY"
		// i.e. 3 tokens separated by 2 spaces.
		// So let's split the string into tokens

		// This tokeniser from stackoverflow:
		// http://stackoverflow.com/questions/236129/split-a-string-in-c
		std::vector<std::string> tokens;
		std::istringstream iss( potentialBuildDate );
		std::copy( std::istream_iterator<std::string>( iss ),
					std::istream_iterator<std::string>(),
					std::back_inserter( tokens ));

		if ( ( tokens.size() == 3 )  /* MMM DD YYYY */
			&& ( tokens[0].size() == 3 ) /* month string */
			&& ( tokens[1].size() == 2 || tokens[1].size() == 1 )  /* day string */
			&& ( tokens[2].size() == 4 ) /* year string */ )
		{
			std::cerr << "BuildDate looks valid. Will attempt to replace." << std::endl;
			validBuildDate = true;
		}
		else
		{
			std::cerr << "Candidate build date is not in the expected format MMM DD YYYY" << std::endl;
			// Search again, starting a bit further along in the file
			location += 50;
		}
	}

	// Change the buildDate to be today, so that the user gets another 3M

	std::string todayDate = getCurrentDateAsString();

	if (potentialBuildDate == todayDate )
	{
		// BuildDate is already up-to-date
		// Possibly the user gave us a fresh XLL to work on, or the BuilDate for this LicenseCheck has already been updated in a previous pass.
		std::cout << "Build Date already up to date. Nothing to do." << std::endl;
	}
	else
	{
		replaceStringInFileContent( potentialBuildDate, todayDate, fileContent );
	}

}

int main(int argc, char* argv[])
{
	std::cout << "*** GodzillaVsMLIB Build-Date Tool ***" << std::endl;

	if ( argc != 2 )
	{
		std::cerr << "Usage: GodzillaVSMLIB.exe  path_to_MLIB_xll" << std::endl;
		std::cerr << "Wrong number of command line arguments. Was expecting 1, but was given " << (argc -1) << std::endl;
		exit(1);
	}

	/*
		The overall strategy:

		1. Read the input XLL path from the command line
		2. Read in the input XLL, modify the buildDate, write out to a copy  .xll.new
		3. Move the input XLL path to a backup  .BAK
		4. Move the filename.xll.new to filename.xll

	*/

	std::string inputDLLPath( argv[1] );
	std::string updatedDLLPath ( inputDLLPath );
	updatedDLLPath += ".new";

	std::string fileContent = readBinaryFile( inputDLLPath );

	// Look for the license check in the binary file, starting at the beginning
	// The location parameter will be updated to the location of the License check code.
	std::string::size_type location = 0;

	//   0123456789A
	//   MMM DD YYYY
	const std::string todayDate = getCurrentDateAsString();

	const size_t yearStartOffset = 7;
	const size_t yearLength       = 4;
	const std::string yearStr   = todayDate.substr( yearStartOffset, yearLength );
	const int baseYear = std::stoi( yearStr );

	// Go back up to 5 years
	for ( int i = 0, year = baseYear; i < 5; i++, year-- )
	{
		const std::string yearToSearchFor = std::to_string( year );
		std::cout << "Searching for build-date in year: " << yearToSearchFor << std::endl;

		location = 0;
		findCandidateYearAndUpdateBuildDates( fileContent, yearToSearchFor, location );

		// Now search for the licenseCheck code again: 
		// There is an etrading license check and a vanilla license check.
		// These checks are from different source/object files, and so may contain different build dates.
		std::cout << "Searching for additional build-date in year: " << yearToSearchFor << std::endl;

		location += 50;  // Move past the previous licenseCheck
		findCandidateYearAndUpdateBuildDates( fileContent, yearToSearchFor, location );
	}

	// Finally, write out the updated fileContent
	writeFile( updatedDLLPath, fileContent );

	std::cout << "Making a backup of the original XLL... ";

	std::string backupCommand = "move \"" + inputDLLPath + "\" \"" + inputDLLPath + "\".BAK";
	//std::cout << "About to run command: " << backupCommand << std::endl;
	system( backupCommand.c_str() );

	std::cout << "Moving the new XLL into position...    ";
	std::string renameCommand = "move \"" + updatedDLLPath + "\" \"" + inputDLLPath + "\"";
	system( renameCommand.c_str() );

	return 0;
}

