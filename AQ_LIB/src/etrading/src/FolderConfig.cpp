/*
 * @brief			Configuration methods that assist in addin setup
 *					It was ported over from MAFolderConfig.h/.cpp	
 * @Created:		14 November 2016
 * @Author:			
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "FolderConfig.h"
#include "Environment.h"
#include "LAString.h"

#include <fstream>

#ifdef _MSC_VER
#include <Windows.h>
#endif

namespace
{
    // #1. Firstly we initialize the library config for legacy users for backwards compatibility
    // ====================================================================================================================

	// 	note that Path here is set to mean Path + File Name
    const LAString CONFIG_FILE_PATH( "D:\\appl\\irsvr_excel.conf" );
    const LAString DEFAULT_CBSCHEDULE_PATH( "S:\\mizuho\\fig\\fi_rstac\\mlibir\\configration\\CBSchedule.csv" );
    const LAString DEFAULT_CALENDAR_PATH( "S:\\mizuho\\fig\\fi_rstac\\mlibir\\configration\\Calendar.csv" );
    const LAString DEFAULT_IRPROP_PATH( "S:\\mizuho\\fig\\fi_rstac\\mlibir\\configration\\ir.properties" );
    const LAString DEFAULT_CALIBPROP_PATH( "S:\\mizuho\\fig\\fi_rstac\\mlibir\\configration\\calib.properties" );

    // #2. Initialize the library for Quants - This is only required and done if item #1 is not successful
    // ====================================================================================================================

    // Alternative File Path ( This is for Developers )
    // This requires the developer to set an Environment Variable "MLIBQ" pointing to the folder where
    // the library calendar and property files are kept. There should be no final backslash on the property path.
	//
	// NOTE: Use forward slash '/' as the directory separator, so that these paths work on Linux as well as Windows
	//
    const LAString CONFIG_FILE_PATH2 = etrading::FolderConfig::toPath( "MLIBQ", "/resource/config/irsvr_excel.conf" );
    const LAString DEFAULT_CBSCHEDULE_PATH2 = etrading::FolderConfig::toPath( "MLIBQ", "/resource/config/CBSchedule.csv" );
    const LAString DEFAULT_CALENDAR_PATH2 = etrading::FolderConfig::toPath( "MLIBQ", "/resource/config/Calendar.csv" );
    const LAString DEFAULT_CALIBPROP_PATH2 = etrading::FolderConfig::toPath( "MLIBQ", "/resource/config/calib.properties" );
    const LAString DEFAULT_IRPROP_PATH2 = etrading::FolderConfig::toPath( "MLIBQ", "/resource/config/ir.properties" );
    const LAString DEFAULT_IRPROP_PATH3 = etrading::FolderConfig::toPath( "MLIBQ", "/resource/config/ir.properties" );
	const LAString OPTIONAL_CONFIG_PATH = etrading::FolderConfig::toPath("MLIBQ", "/resource/config/startup.conf" );


    // #3. Initialize the library for Business Users - This is only required and done if item #1 and #2 not successful
    // ====================================================================================================================

    // Alternative File Path ( This is for Business Users and Excel )
    const LAString CONFIG_FILE_NAME( "/config/irsvr_excel.conf" );
    const LAString CBSCHEDULE_FILE_NAME( "/config/CBSchedule.csv" );
    const LAString CALENDAR_FILE_NAME( "/config/Calendar.csv" );
    const LAString CALIBPROP_FILE_NAME( "/config/calib.properties" );
    const LAString IRPROP_FILE_NAME( "/config/ir.properties" );
    const LAString IRPROP_FILE_NAME2( "/config/ir.properties" );
	const LAString OPTIONAL_CONFIG_PATH2 =( "/config/startup.conf" );

    // This is only for the Optional Config
	const bool reportErrors = false;
}

namespace etrading
{
	std::shared_ptr< LAString > FolderConfig::ir_prop_path_;
	std::shared_ptr< LAString > FolderConfig::calib_prop_path_;
	std::shared_ptr< LAString > FolderConfig::calendar_path_;
	std::shared_ptr< LAString > FolderConfig::cbschedule_path_;
	std::shared_ptr< LAString > FolderConfig::optional_config_path_;

	/* @brief			Function to set and return the MLIBQ Folder path to the Google Tests.
    *  @return			Returns Google Test Folder in the format of boost::filesystem::path 
    */
    boost::filesystem::path FolderConfig::getMLIBQEnvironmentVariableGoogleTestPath()
    {
        const char* environmentVariablePath = std::getenv( "MLIBQ" );
            
        if ( environmentVariablePath == nullptr )
        {
            throw LACoreInvalidData("#Error: The 'MLIBQ' environment variable has not been set.",__FILE__,__LINE__);
        }
            
        boost::filesystem::path resultPath = boost::filesystem::path( environmentVariablePath ) / "resource" / "test" / "inputs";
        return resultPath;
    }


	// Function to return the path stored by the an environment variable
	LAString FolderConfig::toPath( std::string environmentVariable, std::string suffix )
	{
		char* prefix = getenv( environmentVariable.c_str() );
		if ( prefix == NULL )
		{
			return LAString( "" );
		}
		return LAString( ( prefix + suffix ).c_str() );
	}

	
	//
	// Function to return the folder that the Excel add-in Vanilla.xll is running from
	//
	const LAString FolderConfig::getCurrentFolder()
	{
		try
		{
			LAString result = LAString();

// Only support this feature on Windows.
// On Linux we return an empty path for now.
#ifdef _MSC_VER
			char path[MAX_PATH];
			HMODULE hm = NULL;

			// Get a reference to the handle module to a local function specified below at (*)
			if ( !GetModuleHandleExA( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
									  GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
									  ( LPCSTR ) &getCurrentFolder, // the local funcion to reference (*) i.e. this function
									  &hm ) )
			{
				// If reference initialization fails return empty string
				return result;
			}

			// Get the full file path of the DLL calling the local function i.e. the file path of Vanilla.Xll
			GetModuleFileNameA( hm, path, sizeof( path ) );

			// Get the folder name from this file path
			std::string folder = path;
			folder = folder.substr( 0, folder.rfind( '\\' ) ); // if not found returns string:::npos i.e. end of string

			result = LAString( folder.c_str() );

#endif
			return result;
		}
		catch ( ... )
		{
			return LAString();
		}
	}

	const LAString FolderConfig::createFilePath( const LAString* folder, const LAString* fileName )
	{
		LAString result = LAString();
		LAString usedFolder = LAString();

		// Return null string if no fileName provided
		if ( fileName == NULL )
			return result;

		// If no folder provided use the current folder that the Excel Add-In Vanilla.xll was launched from.
		if ( folder == NULL )
			usedFolder = getCurrentFolder();
		else
			usedFolder = *folder;

		// If cannot get the current folder return null string
		if ( usedFolder == LAString() )
			return result;

		// Concatenate the folder and filepath
		LAString filepath = LAString( usedFolder + "\\" + *fileName );
		result = filepath;

		return result;
	}
	
	void FolderConfig::set_cbschedule_path( const LAString& s )
	{
		cbschedule_path_.reset( new LAString( s ) );
	}

	void FolderConfig::set_calendar_path( const LAString& s )
	{
		calendar_path_.reset( new LAString( s ) );
	}

	void FolderConfig::set_ir_prop_path( const LAString& s )
	{
		ir_prop_path_.reset( new LAString( s ) );
	}

	void FolderConfig::set_calib_prop_path( const LAString& s )
	{
		calib_prop_path_.reset( new LAString( s ) );
	}


	const LAString* FolderConfig::ir_prop_path()
	{
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();		
		read_config_file( "vnl.ir.properties", ir_prop_path_ );
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();

		ir_prop_path_.reset( new LAString( DEFAULT_IRPROP_PATH ) );
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();

		ir_prop_path_.reset( new LAString( createFilePath( NULL, &IRPROP_FILE_NAME ) ) );
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();

		ir_prop_path_.reset( new LAString( createFilePath( NULL, &IRPROP_FILE_NAME2 ) ) );
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();


		ir_prop_path_.reset( new LAString( DEFAULT_IRPROP_PATH2 ) );
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();

		ir_prop_path_.reset( new LAString( DEFAULT_IRPROP_PATH3 ) );
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();

		return NULL;
	}

	const LAString* FolderConfig::calib_prop_path()
	{
		if ( calib_prop_path_.get() != NULL && check_file_availability( *calib_prop_path_.get() ) ) return calib_prop_path_.get();
		read_config_file( "vnl.calib.properties", calib_prop_path_ );
		if ( calib_prop_path_.get() != NULL && check_file_availability( *calib_prop_path_.get() ) ) return calib_prop_path_.get();

		calib_prop_path_.reset( new LAString( DEFAULT_CALIBPROP_PATH ) );
		if ( calib_prop_path_.get() != NULL && check_file_availability( *calib_prop_path_.get() ) ) return calib_prop_path_.get();

		calib_prop_path_.reset( new LAString( createFilePath( NULL, &CALIBPROP_FILE_NAME ) ) );
		if ( calib_prop_path_.get() != NULL && check_file_availability( *calib_prop_path_.get() ) ) return calib_prop_path_.get();

		calib_prop_path_.reset( new LAString( DEFAULT_CALIBPROP_PATH2 ) );
		if ( calib_prop_path_.get() != NULL && check_file_availability( *calib_prop_path_.get() ) ) return calib_prop_path_.get();

		return NULL;
	}

	const LAString* FolderConfig::calendar_path()
	{
		if ( calendar_path_.get() != NULL && check_file_availability( *calendar_path_.get() ) ) return calendar_path_.get();
		read_config_file( "vnl.calendar", calendar_path_ );
		if ( calendar_path_.get() != NULL && check_file_availability( *calendar_path_.get() ) ) return calendar_path_.get();

		calendar_path_.reset( new LAString( DEFAULT_CALENDAR_PATH ) );
		if ( calendar_path_.get() != NULL && check_file_availability( *calendar_path_.get() ) ) return calendar_path_.get();

		LAString s = createFilePath( NULL, &CALENDAR_FILE_NAME );
		calendar_path_.reset( new LAString( createFilePath( NULL, &CALENDAR_FILE_NAME ) ) );
		if ( calendar_path_.get() != NULL && check_file_availability( *calendar_path_.get() ) ) return calendar_path_.get();

		calendar_path_.reset( new LAString( DEFAULT_CALENDAR_PATH2 ) );
		if ( calendar_path_.get() != NULL && check_file_availability( *calendar_path_.get() ) ) return calendar_path_.get();

		return NULL;
	}

	const LAString* FolderConfig::cbschedule_path()
	{
		if ( cbschedule_path_.get() != NULL && check_file_availability( *cbschedule_path_.get() ) ) return cbschedule_path_.get();

		cbschedule_path_.reset( new LAString( DEFAULT_CBSCHEDULE_PATH ) );
		if ( cbschedule_path_.get() != NULL && check_file_availability( *cbschedule_path_.get() ) ) return cbschedule_path_.get();

		LAString s = createFilePath( NULL, &CBSCHEDULE_FILE_NAME );
		cbschedule_path_.reset( new LAString( createFilePath( NULL, &CBSCHEDULE_FILE_NAME ) ) );
		if ( cbschedule_path_.get() != NULL && check_file_availability( *cbschedule_path_.get() ) ) return cbschedule_path_.get();

		cbschedule_path_.reset( new LAString( DEFAULT_CBSCHEDULE_PATH2 ) );
		if ( cbschedule_path_.get() != NULL && check_file_availability( *cbschedule_path_.get() ) ) return cbschedule_path_.get();

		return NULL;
	}

	void FolderConfig::read_config_file( const LAString& key, std::shared_ptr< LAString >& dest )
	{
		std::ifstream ifs( CONFIG_FILE_PATH.getCString() );
		if ( !ifs.is_open() )
			ifs.open( CONFIG_FILE_PATH2.getCString() ); // Alternative Config File Path for London
		if ( !ifs.is_open() )
			ifs.open( createFilePath( NULL, &CONFIG_FILE_NAME ).getCString() ); // Alternative Config File Path for London
		if ( !ifs.is_open() )
			return;

		std::string line;
		size_t sep_pos;
		const char sep = '=';
		std::string cur_key, key_;
		key_ = key.getCString();
		while ( std::getline( ifs, line ) )
		{
			sep_pos = line.find_first_of( sep );
			cur_key = line.substr( 0, sep_pos );
			if ( cur_key == key_ )
			{
				dest.reset( new LAString( line.substr( sep_pos + 1 ).c_str() ) );
				return;
			}
		}
	}

	bool FolderConfig::check_file_availability( const LAString& file_path )
	{
		std::ifstream ifs( file_path.getCString() );
		return ifs.is_open();
	}

	 /* @brief Checks for and loads an optional configuration file.
	 * The configuration file allows the pre-loading of saved JSON objects
	 * for example SWAP_GENERATORs
	 * The file contains one or more lines with the format:
	 * cachedObjectEnum,objectConfigFilePath
	 * cachedObjectEnum should be a recognized CacheObjectEnum.
	 * objectConfigFilePath should be a valid file containing a list of JSON files
	 */
	const LAString* FolderConfig::setupOptionalStartupConfig()
	{		
		const LAString* filepath =getOptionalConfigPath();
		if(filepath==NULL)
		{
			return NULL;	  // Optional Configuration not found. This is not considered an error
		}

		std::ifstream fin;
		fin.open(filepath->getCString());
		if (!fin)
		{
			if (reportErrors)
			{
				std::stringstream sst;
                sst << "#Error: Cannot open optional config file " << std::endl 
					<< *filepath;
				throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
			}
			else
			{
				return NULL;  // Cannot read the Optional Configuration file. Quietly return.
			}
		}

		// Construct the Configuration folder name using the Optional Configuration filepath as a base
		boost::filesystem::path fullPathToFile(filepath->getCString());
		boost::filesystem::path configDirectory = fullPathToFile.parent_path();

		std::string line;
		size_t line_num = 0;
		while (getline(fin, line))
		{
			line_num++;
			const char *c_line = line.c_str();
			LAString tmpstr(c_line);
			
			// The format of each line in the optional configuration file is: CachedObjectEnum,configFile
			char delimiter(',');
			unsigned int configFileColumnSize = 2;
			
			LAStringVector configItems = tmpstr.toToken( delimiter );
			if ( configItems.size() !=  configFileColumnSize )
			{
				if (reportErrors)
				{
					std::stringstream sst;
                    sst << "#Error: Optional properties file format is invalid" << std::endl
						<< "file : " << *filepath << std::endl
						<< "line : " << line_num << std::endl
						<< "contents : " << line
						<< "Expected format: ObjectEnum,FilenamePath";
					throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
				}
				else
				{
					continue;  // Incorrect line format. Quietly move on to the next line
				}
			}

			LAString& cachedObjectEnumString = configItems[0];
			LAString& configFile = configItems[1];
			CachedObjectEnum enumTypeToLoad;
			try
			{
				// Validate the specified cachedObjectEnumString
				enumTypeToLoad = etrading::toCachedObjectEnum(cachedObjectEnumString.getCString());
			}
			catch (ETradingException e)
			{
				if (reportErrors)
				{
					std::stringstream sst;
                    sst << "#Error: Optional properties file format contains error: Invalid ObjectEnum" << std::endl
						<< "file : " << *filepath << std::endl
						<< "line : " << line_num << std::endl
						<< "contents : " << line;
					throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
				}
				else
				{
					continue; // Invalid cachedObjectEnumString on this line. Quietly move on to the next line
				}
			}
			deserializeObjectsForOptionalStartup(enumTypeToLoad, configDirectory, configFile);
		}
		fin.close();
		return filepath;
	}

	/* @brief Search for the Optional Configuration file. 
	 * @param [out] The configuration file path (if resolved), otherwise NULL.
	 */
	 const LAString* FolderConfig::getOptionalConfigPath()
	{
        // First attempt: Load the config file using the %MLIBQ% environment variable; this is for developers
        optional_config_path_.reset( new LAString( OPTIONAL_CONFIG_PATH ) );
		if (optional_config_path_.get() != NULL && check_file_availability( *optional_config_path_.get() ) ) return optional_config_path_.get();

        // Second attempt: Load the config file using the current Excel Addin folder
		optional_config_path_.reset( new LAString( createFilePath( NULL, &OPTIONAL_CONFIG_PATH2 ) ) );
		if (optional_config_path_.get() != NULL && check_file_availability( *optional_config_path_.get() ) ) return optional_config_path_.get();

		return NULL;
	}

	/* @brief Deserializes all of the JSON files specified in the configFile.
	 * @param [in] requiredEnumType  The ObjectEnum we are expecting to deserialize
	 * @param [in] configDirectory   The Configuration folder containing the configFile and all JSON files
	 * @param [in] configFile        A config file which contains a list of JSON filenames to deserialize
	 */
	void FolderConfig::deserializeObjectsForOptionalStartup(CachedObjectEnum requiredEnumType, const boost::filesystem::path& configDirectory, const LAString& configFile)
	{
		boost::filesystem::path configFilePath = configDirectory / configFile.getCString();
		LAString configFileWithFolderPath(configFilePath.string().c_str());

		std::ifstream fin;
		fin.open(configFileWithFolderPath.getCString());
		if (!fin)
		{
			if (reportErrors)
			{
				std::stringstream sst;
				sst << "In Optional Config, cannot open configuration file " << std::endl << configFileWithFolderPath;
				throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
			}
			else
			{
				return;	  // Cannot load this configuration file. Quietly return
			}
		}

		std::string line;
		size_t line_num = 0;
		boost::filesystem::path jsonSubDirectory = configDirectory / toString(requiredEnumType);
		while (getline(fin, line))
		{
			line_num++;
			const char *jsonFile = line.c_str();
			boost::filesystem::path jsonFilePath = jsonSubDirectory / jsonFile;

			std::pair<std::string, CachedObjectEnum> cacheInfoOnDeserialization;
			try
			{
				// Deserialize the JSON file and populate the default environment cache.
				cacheInfoOnDeserialization = etrading::deSerializeFromJSON(etrading::serialize::FILE, jsonFilePath.string().c_str());
			}
			catch (ETradingException e)
			{
				if (reportErrors)
				{
					fin.close();
					std::stringstream sst;
					sst << "In Optional Config, error when attempting to deserialize an object. " << std::endl
						<< "Config file: " << configFile << std::endl
						<< "line : " << line_num << std::endl
						<< "contents : " << line << std::endl
						<< "JSON file path : " << jsonFilePath.string();
					throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
				}
				else
				{
					continue;   // Unable to deserialize this object. Quietly move on to the next line
				}
			}
			if (requiredEnumType != cacheInfoOnDeserialization.second && reportErrors)
			{
				fin.close();
				std::stringstream sst;
				sst << "In Optional Config, deserialized an object of the wrong type: " << std::endl
					<< "Expected Type: " << toString(requiredEnumType) << std::endl
					<< "Deserialised Type: " << toString(cacheInfoOnDeserialization.second) << std::endl
					<< "JSON file path: " << jsonFilePath.string();
				throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
			}
		}
		fin.close();
	}
}
