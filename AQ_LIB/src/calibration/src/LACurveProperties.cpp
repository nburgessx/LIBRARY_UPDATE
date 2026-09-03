#include "LACurveProperties.h"
#include "LACoreAppError.h"
#include "LAString.h"

#include <fstream>
#include <sstream>

std::shared_ptr<LAString> LACurveProperties::ir_prop_path_;
std::shared_ptr<LAString> LACurveProperties::calib_prop_path_;
std::shared_ptr<LAString> LACurveProperties::calendar_path_;


namespace
{
    const LAString CONFIG_FILE_PATH("d:\\appl\\irsvr_excel.conf");
    const LAString DEFAULT_CALENDAR_PATH("M:\\02_Project\\5858_mlibir\\configration\\Calendar.csv");
    const LAString DEFAULT_IRPROP_PATH("M:\\02_Project\\5858_mlibir\\configration\\ir.properties");
	const LAString DEFAULT_CALIBPROP_PATH("M:\\02_Project\\5858_mlibir\\configration\\calib.properties");
}


void LACurveProperties::set_calendar_path(const LAString& s)
{
    calendar_path_.reset(new LAString(s));
}

void LACurveProperties::set_ir_prop_path(const LAString& s)
{
    ir_prop_path_.reset(new LAString(s));
}

void LACurveProperties::set_calib_prop_path(const LAString& s)
{
    calib_prop_path_.reset(new LAString(s));
}


const LAString* LACurveProperties::ir_prop_path()
{
    if(ir_prop_path_.get() != NULL && check_file_availability(*ir_prop_path_.get())) return ir_prop_path_.get();
    read_config_file("vnl.ir.properties", ir_prop_path_);
    if(ir_prop_path_.get() != NULL && check_file_availability(*ir_prop_path_.get())) return ir_prop_path_.get();
	ir_prop_path_.reset(new LAString(LACurveProperties::toPath( "MLIBQ", "\\ir.properties" )));
	if(ir_prop_path_.get() != NULL && check_file_availability(*ir_prop_path_.get())) return ir_prop_path_.get();

    ir_prop_path_.reset(new LAString(DEFAULT_IRPROP_PATH));
    return check_file_availability(*ir_prop_path_.get()) ? ir_prop_path_.get() : NULL;
}

const LAString* LACurveProperties::calib_prop_path()
{
    if(calib_prop_path_.get() != NULL && check_file_availability(*calib_prop_path_.get())) return calib_prop_path_.get();
    read_config_file("vnl.calib.properties", calib_prop_path_);
    if(calib_prop_path_.get() != NULL && check_file_availability(*calib_prop_path_.get())) return calib_prop_path_.get();
	calib_prop_path_.reset(new LAString(LACurveProperties::toPath( "MLIBQ", "\\calib.properties" )));
	if(calib_prop_path_.get() != NULL && check_file_availability(*calib_prop_path_.get())) return calib_prop_path_.get();

    calib_prop_path_.reset(new LAString(DEFAULT_CALIBPROP_PATH));
    return check_file_availability(*calib_prop_path_.get()) ? calib_prop_path_.get() : NULL;
}

const LAString* LACurveProperties::calendar_path()
{
    if(calendar_path_.get() != NULL && check_file_availability(*calendar_path_.get())) return calendar_path_.get();
    read_config_file("vnl.calendar", calendar_path_);
    if(calendar_path_.get() != NULL && check_file_availability(*calendar_path_.get())) return calendar_path_.get();
	calendar_path_.reset(new LAString(LACurveProperties::toPath( "MLIBQ", "\\calendar.csv" )));
	if(calendar_path_.get() != NULL && check_file_availability(*calendar_path_.get())) return calendar_path_.get();

    calendar_path_.reset(new LAString(DEFAULT_CALENDAR_PATH));
    return check_file_availability(*calendar_path_.get()) ? calendar_path_.get() : NULL;
}

void LACurveProperties::read_config_file(const LAString& key, std::shared_ptr<LAString>& dest)
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
            dest.reset(new LAString(line.substr(sep_pos + 1).c_str()));
            return;
        }
	}
}

bool LACurveProperties::check_file_availability(const LAString& file_path)
{
    std::ifstream ifs(file_path.getCString());
    return ifs.is_open();
}

// Function to return the path stored by the an environment variable
LAString LACurveProperties::toPath(const LAString&  environmentVariable, const LAString&  suffix)
{
	char* prefix = getenv(environmentVariable.getCString());
	if(prefix == NULL){
		return LAString( "" );
	}
	return LAString((prefix + suffix ).getCString());
}
