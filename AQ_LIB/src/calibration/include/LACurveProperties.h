#pragma once

#include <memory>
#include <iostream>
#include <string>

class LAString;

class LACurveProperties{
public:
    static const LAString* ir_prop_path();
	static const LAString* calib_prop_path();
    static const LAString* calendar_path();
    static void set_ir_prop_path(const LAString& s);
	static void set_calib_prop_path(const LAString& s);
    static void set_calendar_path(const LAString& s);

	static LAString toPath(const LAString& environmentVariable, const LAString& suffix );

protected:
    static std::shared_ptr<LAString> ir_prop_path_;
	static std::shared_ptr<LAString> calib_prop_path_;
    static std::shared_ptr<LAString> calendar_path_;

    static void read_config_file(const LAString& key, std::shared_ptr<LAString>& dest);
    static bool check_file_availability(const LAString& file_path);

};

