
#include "stdafx.h"

#include "exposed_functions.h"
#include "TypeUtilities.h"
#include <stdexcept>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <memory>
#include <functional>


#include "MBTemplateType.h"  
#include "LADate.h"
#include "LAString.h"
#include "MMAnalyticFormula.h"
#include "MFirDateFuncUtility.h"
#include "MAService.h"
#include "MAExcelAddinWrap.h"
#include "MAExcelAddinConfig.h"
#include "MADataConst.h"
#include "MVirPlainVanillaVolatilityManager.h"
#include "LibSetUp.h"


// pitfalls for exposing MLIB functions to Java through SWIG:
// a.) do not return local variable addresses unless unavoidable (in which case consult SWIG to alter the interface for freeing up memory)
//     e.g. avoid things like "return myString.c_str();" or dynamically allocated const char * 
//	   If you do need to return a dynamically allocated object (and the client needs to do the cleanup)
//	   you will need to change the SWIG file (*.i), update the wrapper file and recompile


// these are implementation example functions demonstrating the supported return types
int getInt() {	return 1; }
double getDouble() { return 2.5; } 
char getChar() { return 'z'; };
bool getBool() { return false; };
const char* getConstChar() { return "Here you go!"; }  
std::string getStdString() { std::string theRealDeal = "The Real Deal"; return theRealDeal; }
const std::vector<int> getVectorOfInt() {
	std::vector<int> testVals;    // vector<int> testVals = { 2 ,4 ,5}; // only from VS 2015
	testVals.push_back(2);
	testVals.push_back(1);
	testVals.push_back(0);
	testVals.push_back(25);
	testVals.push_back(0);
	testVals.push_back(-25);
	testVals.push_back(-5);
	return testVals;
}
const std::vector<std::vector <int> > getVectorOfVectorOfInt() {
	std::vector<int> testVals;    
	testVals.push_back(2);
	testVals.push_back(1);
	testVals.push_back(0);
	testVals.push_back(25);
	testVals.push_back(0);
	testVals.push_back(-25);
	testVals.push_back(-5);
	std::vector<int> testVals2;    
	testVals2.push_back(20);
	testVals2.push_back(10);
	testVals2.push_back(40);
	testVals2.push_back(250);
	testVals2.push_back(9);
	testVals2.push_back(-125);
	testVals2.push_back(-253);
	std::vector<std::vector<int> > returnVecOfVec;
	returnVecOfVec.push_back(testVals2);
	returnVecOfVec.push_back(testVals);
	return returnVecOfVec;  // talk about copy overload...
}
const std::vector<double> getVectorOfDouble() {
	std::vector<double> testVals;    // vector<double> testVals = { 2.1 ,4.2 ,5.3}; // only from VS 2015
	testVals.push_back(2.01);
	testVals.push_back(1.56);
	testVals.push_back(0.46);
	testVals.push_back(25.89);
	testVals.push_back(0.0);
	testVals.push_back(-25.787);
	testVals.push_back(-5.9414);
	return testVals;
}
const std::vector<std::vector <double> > getVectorOfVectorOfDouble() {
	std::vector<double> testVals;    
	testVals.push_back(2.16);
	testVals.push_back(1.146);
	testVals.push_back(0.4967);
	testVals.push_back(25.97797);
	testVals.push_back(0.0);
	testVals.push_back(-25.47497);
	testVals.push_back(-5.1454);
	std::vector<double> testVals2;    
	testVals2.push_back(20.4964);
	testVals2.push_back(10.16);
	testVals2.push_back(40.987);
	testVals2.push_back(250.888);
	testVals2.push_back(9.63);
	testVals2.push_back(-125.14);
	testVals2.push_back(-253.25);
	std::vector<std::vector<double> > returnVecOfVec;
	returnVecOfVec.push_back(testVals);
	returnVecOfVec.push_back(testVals2);
	return returnVecOfVec;  // talk about copy overload...
}
const std::vector<std::string> getVectorOfString() {
	std::vector<std::string> testVals;    
	testVals.push_back("aa");
	testVals.push_back("B");
	testVals.push_back("C");
	testVals.push_back("D");
	testVals.push_back("E");
	testVals.push_back("ff");
	testVals.push_back("G");
	return testVals;
}
const std::vector<std::vector <std::string> > getVectorOfVectorOfString() {
	std::vector<std::string> testVals;    
	testVals.push_back("aa");
	testVals.push_back("B");
	testVals.push_back("C");
	testVals.push_back("D");
	testVals.push_back("E");
	testVals.push_back("ff");
	testVals.push_back("G");
	std::vector<std::string> testVals2;    
	testVals2.push_back("Z");
	testVals2.push_back("yyy");
	testVals2.push_back("X");
	testVals2.push_back("www");
	testVals2.push_back("V");
	testVals2.push_back("uuu");
	testVals2.push_back("Q");
	std::vector<std::vector<std::string> > returnVecOfVec;
	returnVecOfVec.push_back(testVals);
	returnVecOfVec.push_back(testVals2);
	return returnVecOfVec;  // talk about copy overload...
}
const std::string testVectorOfVectorOfDoubles(const std::vector<std::vector <std::string> >& in)
{
	return "Done";
}
const int throwAnException() throw(std::exception) {
	throw std::exception("ERROR: This exception text was generated inside the SWIG DLL through a std::exception");
	return 0;
}
const int throwAnIllegalArgumentException() throw(std::exception) {
	throw std::invalid_argument("ERROR: This exception text was generated inside the SWIG DLL through a std::invalid_argument");
	return 0;
};

const double sumUp(const double inputArray[], const int sizeOfArray) throw(std::exception) {
	double resultOut = 0.0;
	//  sizeOfArray = (sizeof(inputArray)/sizeof(*inputArray)) ;   
	for(int i =0; i < sizeOfArray; i++) {
		resultOut +=  inputArray[i];
	}
	return resultOut;
}

const double sumUp(const std::vector<double>& inputArray) throw(std::exception) {
	double resultOut = 0.0;
	for(int i =0; i < inputArray.size(); i++) {
		resultOut +=  inputArray.at(i);
	}
	return resultOut;
}


double doSomeFormula(const double z) { return AnalyticFormulae::diffNormdist(z); }


// boost::mutex g_initialization_mutex; // not real thread safety (cfr. inititializeMLIB comments)

std::string setUpMLIB(const std::string& irPropsFullFilePath, const std::string& calendarFullFilePath) {

	try {

		// TODO: Move the mutexes down to the MLIB singleton on the Entity pool 
		// this fig leaf will NOT be thread safe unless MLIB itself is made thread safe
		// boost::lock_guard<boost::mutex> lock(g_initialization_mutex);

		// grab the reference this exists even if it does not hold anything... -> not really only if MLIB 
		if(g_root != nullptr) {
			tearDownMLIB();
		}

		MAService::setContext(CONTEXT_KEY_ISEXCELREQUEST, "FALSE");  // was set to true in previous statement
		MAService::setContext(CONTEXT_KEY_ISSETCURVEID, "TRUE");
		LAString calendarFullFileName(calendarFullFilePath.c_str());
		MAExcelAddinConfig::set_calendar_path(calendarFullFileName);
		LAString filepath(irPropsFullFilePath.c_str());
		MAExcelAddinConfig::set_ir_prop_path(filepath);
		MAExcelAddinWrap::setUpForIRServer();  // TODO: Stop making this lower layer refer to an interface (like Excel)
		g_root = vanilla::InitializeMLibVanilla::instance(false).root(); // should always return 
		MAExcelAddinWrap::setUpDefaultIRProperty(*g_root);
		// libSetUp(g_root);
		return "Initialized MLIB";
	}
	catch(MEError& e)
	{
		std::string errorMessageBack = "ERROR: Failed to set MLIB root - ";
		errorMessageBack += e.getMsg();
		return errorMessageBack;
	}
	catch(...)
	{
		return std::string("ERROR: Failed to set MLIB root - unknown error");
	}

	return "not implemented";
}

const std::string tearDownMLIB() {
	
	try 
	{
		if(g_root != nullptr) 
		{
			MAService::finalize();
			MVirPlainVanillaVolatilityManager::finalize();
			vanilla::InitializeMLibVanilla::destroyInstance();
		}
		g_root = nullptr; // never delete the observational pointer ...
		// if(g_root) delete g_root;  
		return std::string("Finalized MLIB");
	}	
	catch(MEError& e) 
	{
		std::string errorMessageBack = "ERROR: Failed to finalize MLIB properly - ";
		errorMessageBack += e.getMsg();
		return errorMessageBack;
	}
	catch(...) 
	{
		return std::string("ERROR: Failed to finalize MLIB properly MLIB root - unknown error");
	}
}

