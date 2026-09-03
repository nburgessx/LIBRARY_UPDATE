/*
 * @brief			Configuration methods that assist in addin setup
 *					It was ported over from LACurveProperties.h/.cpp	
 * @Created:		14 November 2016
 * @Author:			
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <boost/filesystem.hpp>

#include <memory>
#include <iostream>
#include <string>

#include "CoreEnumerations.h"

class LAString;

namespace etrading
{
	class FolderConfig
	{
	public:
		static const LAString* ir_prop_path();
		static const LAString* calib_prop_path();
		static const LAString* calendar_path();
		static const LAString* cbschedule_path();

		static void set_ir_prop_path(const LAString& s);
		static void set_calib_prop_path(const LAString& s);
		static void set_calendar_path(const LAString& s);
		static void set_cbschedule_path(const LAString& s);

		//
		// Helper functions to help create configuration file path names
		//
		static boost::filesystem::path getMLIBQEnvironmentVariableGoogleTestPath();  // Returns MLIBQ Google Test Folder path in the format of boost::filesystem::path
		static LAString toPath( std::string environmentVariable, std::string suffix ); // Function to return the path stored by the an environment variable
		static const LAString getCurrentFolder(); // Function returns the folder that the Excel add-in Vanilla.xll is running from
		static const LAString createFilePath( const LAString* folder, const LAString* fileName );

		// Helper functions to set up the optional configuration
		static const LAString* setupOptionalStartupConfig();
		static const LAString* getOptionalConfigPath();

        static bool check_file_availability(const LAString& file_path);

	protected:
		static std::shared_ptr<LAString> ir_prop_path_;
		static std::shared_ptr<LAString> calib_prop_path_;
		static std::shared_ptr<LAString> calendar_path_;
		static std::shared_ptr<LAString> cbschedule_path_;
		static std::shared_ptr<LAString> optional_config_path_;

		static void read_config_file(const LAString& key, std::shared_ptr<LAString>& dest);
		
		// Helper function to set up the optional configuration
		static void deserializeObjectsForOptionalStartup(CachedObjectEnum requiredEnumType, const boost::filesystem::path& configDirectory, const LAString& objectConfigFilePath);
	};

}
