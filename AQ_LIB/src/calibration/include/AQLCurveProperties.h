#pragma once

#include <memory>
#include <iostream>
#include <string>

class AQLString;

class AQLCurveProperties{
public:
    static const AQLString* ir_prop_path();
	static const AQLString* calib_prop_path();
    static const AQLString* calendar_path();
    static void set_ir_prop_path(const AQLString& s);
	static void set_calib_prop_path(const AQLString& s);
    static void set_calendar_path(const AQLString& s);

	static AQLString toPath(const AQLString& environmentVariable, const AQLString& suffix );

protected:
    static std::shared_ptr<AQLString> ir_prop_path_;
	static std::shared_ptr<AQLString> calib_prop_path_;
    static std::shared_ptr<AQLString> calendar_path_;

    static void read_config_file(const AQLString& key, std::shared_ptr<AQLString>& dest);
    static bool check_file_availability(const AQLString& file_path);

};

