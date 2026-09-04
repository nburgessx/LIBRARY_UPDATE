#include "AQLCurveProperties.h"
#include "AQLCoreAppError.h"
#include "AQLString.h"

#include <fstream>
#include <sstream>

std::shared_ptr<AQLString> AQLCurveProperties::ir_prop_path_;
std::shared_ptr<AQLString> AQLCurveProperties::calib_prop_path_;
std::shared_ptr<AQLString> AQLCurveProperties::calendar_path_;


namespace
{
    const AQLString CONFIG_FILE_PATH(".\\config\\irsvr_excel.conf");
    const AQLString DEFAULT_CALENDAR_PATH(".\\config\\Calendar.csv");
    const AQLString DEFAULT_IRPROP_PATH(".\\config\\ir.properties");
	const AQLString DEFAULT_CALIBPROP_PATH(".\\config\\calib.properties");
}


void AQLCurveProperties::set_calendar_path(const AQLString& s)
{
    calendar_path_.reset(new AQLString(s));
}

void AQLCurveProperties::set_ir_prop_path(const AQLString& s)
{
    ir_prop_path_.reset(new AQLString(s));
}

void AQLCurveProperties::set_calib_prop_path(const AQLString& s)
{
    calib_prop_path_.reset(new AQLString(s));
}


const AQLString* AQLCurveProperties::ir_prop_path()
{
    if(ir_prop_path_.get() != NULL && check_file_availability(*ir_prop_path_.get())) return ir_prop_path_.get();
    read_config_file("vnl.ir.properties", ir_prop_path_);
    if(ir_prop_path_.get() != NULL && check_file_availability(*ir_prop_path_.get())) return ir_prop_path_.get();
	ir_prop_path_.reset(new AQLString(AQLCurveProperties::toPath( "AQ", "\\resources\\config\\ir.properties" )));
	if(ir_prop_path_.get() != NULL && check_file_availability(*ir_prop_path_.get())) return ir_prop_path_.get();

    ir_prop_path_.reset(new AQLString(DEFAULT_IRPROP_PATH));
    return check_file_availability(*ir_prop_path_.get()) ? ir_prop_path_.get() : NULL;
}

const AQLString* AQLCurveProperties::calib_prop_path()
{
    if(calib_prop_path_.get() != NULL && check_file_availability(*calib_prop_path_.get())) return calib_prop_path_.get();
    read_config_file("vnl.calib.properties", calib_prop_path_);
    if(calib_prop_path_.get() != NULL && check_file_availability(*calib_prop_path_.get())) return calib_prop_path_.get();
	calib_prop_path_.reset(new AQLString(AQLCurveProperties::toPath( "AQ", "\\resources\\config\\calib.properties" )));
	if(calib_prop_path_.get() != NULL && check_file_availability(*calib_prop_path_.get())) return calib_prop_path_.get();

    calib_prop_path_.reset(new AQLString(DEFAULT_CALIBPROP_PATH));
    return check_file_availability(*calib_prop_path_.get()) ? calib_prop_path_.get() : NULL;
}

const AQLString* AQLCurveProperties::calendar_path()
{
    if(calendar_path_.get() != NULL && check_file_availability(*calendar_path_.get())) return calendar_path_.get();
    read_config_file("vnl.calendar", calendar_path_);
    if(calendar_path_.get() != NULL && check_file_availability(*calendar_path_.get())) return calendar_path_.get();
	calendar_path_.reset(new AQLString(AQLCurveProperties::toPath( "AQ", "\\resources\\config\\Calendar.csv" )));
	if(calendar_path_.get() != NULL && check_file_availability(*calendar_path_.get())) return calendar_path_.get();

    calendar_path_.reset(new AQLString(DEFAULT_CALENDAR_PATH));
    return check_file_availability(*calendar_path_.get()) ? calendar_path_.get() : NULL;
}

void AQLCurveProperties::read_config_file(const AQLString& key, std::shared_ptr<AQLString>& dest)
{
    std::ifstream ifs(CONFIG_FILE_PATH.getCString());
    if(!ifs) return;

	std::string line;
	size_t sep_pos;
	const char sep = '=';
	std::string cur_key, key_;
    key_ = key.getCString();
	while(std::getline(ifs, line)){
		sep_pos = line.find_first_of(sep);
		cur_key = line.substr(0, sep_pos);
        if(cur_key == key_){
            dest.reset(new AQLString(line.substr(sep_pos + 1).c_str()));
            return;
        }
	}
}

bool AQLCurveProperties::check_file_availability(const AQLString& file_path)
{
    std::ifstream ifs(file_path.getCString());
    return ifs.is_open();
}

// Function to return the path stored by the an environment variable
AQLString AQLCurveProperties::toPath(const AQLString&  environmentVariable, const AQLString&  suffix)
{
	char* prefix = getenv(environmentVariable.getCString());
	if(prefix == NULL){
		return AQLString( "" );
	}
	return AQLString((prefix + suffix ).getCString());
}
