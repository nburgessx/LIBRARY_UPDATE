#pragma once

#include <boost/filesystem.hpp>

#include <memory>
#include <iostream>
#include <string>

#include "CoreEnumerations.h"

class AQLString;

namespace etrading
{
	class FolderConfig
	{
	public:
		static const AQLString* ir_prop_path();
		static const AQLString* calib_prop_path();
		static const AQLString* calendar_path();
		static const AQLString* cbschedule_path();

		static void set_ir_prop_path(const AQLString& s);
		static void set_calib_prop_path(const AQLString& s);
		static void set_calendar_path(const AQLString& s);
		static void set_cbschedule_path(const AQLString& s);

		//
		// Helper functions to help create configuration file path names
		//
		static boost::filesystem::path getGoogleTestInputPath();  // Returns AlgoQuantLib Google Test Folder path in the format of boost::filesystem::path
		static AQLString toPath( std::string environmentVariable, std::string suffix ); // Function to return the path stored by the an environment variable
		static const AQLString getCurrentFolder(); // Function returns the folder that the Excel add-in Vanilla.xll is running from
		static const AQLString createFilePath( const AQLString* folder, const AQLString* fileName );

		// Helper functions to set up the optional configuration
		static const AQLString* setupOptionalStartupConfig();
		static const AQLString* getOptionalConfigPath();

        static bool check_file_availability(const AQLString& file_path);

	protected:
		static std::shared_ptr<AQLString> ir_prop_path_;
		static std::shared_ptr<AQLString> calib_prop_path_;
		static std::shared_ptr<AQLString> calendar_path_;
		static std::shared_ptr<AQLString> cbschedule_path_;
		static std::shared_ptr<AQLString> optional_config_path_;

		static void read_config_file(const AQLString& key, std::shared_ptr<AQLString>& dest);
		
		// Helper function to set up the optional configuration
		static void deserializeObjectsForOptionalStartup(CachedObjectEnum requiredEnumType, const boost::filesystem::path& configDirectory, const AQLString& objectConfigFilePath);
	};

}
