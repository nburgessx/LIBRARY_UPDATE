
#include "InitializeETrading.h"
#include "exposed_functions.h"
#include "TypeUtilities.h"
#include <stdexcept>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <memory>
#include <functional>


#include "AQLCoreTemplateType.h"  
#include "AQLDate.h"
#include "AQLString.h"
#include "AQLAnalyticFormula.h"
#include "AQLCoreDataService.h"
#include "AQLUpdateStaticDataManager.h"
#include "AQLDefinitions.h"
#include "AQLLinearRatesVolatilityManager.h"
#include "LibSetUpETrading.h"
#include "EnvironmentUtilities.h"
#include "tryAqToolsSetup.h"
#include "AQLCoreLockControl.h"
#include "OMPThreadManager.h"

//
// Note on Java API Code
// ------------------------------------------------------------------------------------------------------------
//

// pitfalls for exposing AlgoQuantLib functions to Java through SWIG:
// a.) do not return local variable addresses unless unavoidable (in which case consult SWIG to alter the interface for freeing up memory)
//     e.g. avoid things like "return myString.c_str();" or dynamically allocated const char * 
//	   If you do need to return a dynamically allocated object (and the client needs to do the cleanup)
//	   you will need to change the SWIG file (*.i), update the wrapper file and recompile


//
// Important AlgoQuantLib Initialization Functions
// ------------------------------------------------------------------------------------------------------------
//


// boost::mutex g_initialization_mutex; // not real thread safety (cfr. inititializeMLIB comments)
std::string setUpMLIB(const std::string& irPropsFullFilePath, const std::string& calendarFullFilePath)
{
    // Disable OMP Threading by Default for the Server APIs
    etrading::OMPThreadManager::getInstance().setIsOMPEnabled( false );

	std::string statusMsg;
	
	try
	{
		statusMsg = validation::trySetupMLIB( irPropsFullFilePath, calendarFullFilePath );
	}
	catch (AQLCoreError e)
	{
		statusMsg = "#Error: Unable to load configuration. Did you set the AlgoQuantLib environment variable? Error is: ";
		statusMsg += e.what();
	}

	return statusMsg;

}

//
// Alternative Name(s) for the setUpMLIB function
//

std::string setupMLIB( const std::string& irPropsFullFilePath, const std::string& calendarFullFilePath )
{
    return setUpMLIB( irPropsFullFilePath, calendarFullFilePath );
}

std::string initMLIB( const std::string& irPropsFullFilePath, const std::string& calendarFullFilePath )
{
    return setUpMLIB( irPropsFullFilePath, calendarFullFilePath );
}

// Support for null default arguments
std::string setUpMLIB() { return setUpMLIB( "","" ); }
std::string setupMLIB() { return setUpMLIB( "","" ); }
std::string initMLIB()  { return setUpMLIB( "","" ); }


// Terminate AlgoQuantLib - Should be called before termination AlgoQuantLib anaytics by the client
// --------------------------------------------------------------------------------

std::string tearDownMLIB()
{
	return validation::tryTearDownMLIB();
}


//
// Echo Functions - These functions echo the input i.e. input = output.
// This is so we can test that the Swig API represents fucntion arguments and types correctly
// ---------------------------------------------------------------------------------
//

int echoInt( int i )
{
    return i;
}

double echoDouble( double d )
{
    return d;
}

char echoChar( char c )
{
    return c;
}

bool echoBool( bool b)
{
    return b;
}

std::string echoString( const std::string& s )
{
    return s;
}

std::vector<int> echoVectorInt( const std::vector<int>& v )
{
    return v;
}

std::vector<double> echoVectorDouble( const std::vector<double>& v )
{
    return v;
}

std::vector<std::string> echoVectorString( const std::vector<std::string>& v )
{
    return v;
}

std::vector<std::vector <int> > echoMatrixInt( const std::vector<std::vector <int> >& m )
{
    return m;
}

std::vector<std::vector <double> > echoMatrixDouble( const std::vector<std::vector <double> >& m )
{
    return m;
}

std::vector<std::vector <std::string> > echoMatrixString( const std::vector<std::vector <std::string> >& m)
{
    return m;
}



//
// Example Functions - These are example functions demonstrating the supported return types
// ---------------------------------------------------------------------------------
//

int exampleInt()
{
    return 8;
}

double exampleDouble()
{
    return 8.8;
}

char exampleChar()
{
    return 'a';
}

bool exampleBool()
{
    return true;
}

std::string exampleString()
{
    return std::string("String");
}

std::vector<int> exampleVectorInt()
{
    std::vector<int> result;

	result.push_back(2);
	result.push_back(1);
	result.push_back(0);
	result.push_back(25);
	result.push_back(0);
	result.push_back(-25);
	result.push_back(-5);
	
    return result;
}

std::vector<double> exampleVectorDouble()
{
    std::vector<double> result;

	result.push_back(2.01);
	result.push_back(1.56);
	result.push_back(0.46);
	result.push_back(25.89);
	result.push_back(0.0);
	result.push_back(-25.787);
	result.push_back(-5.9414);
		
    return result;
}

std::vector<std::string> exampleVectorString()
{
    std::vector<std::string> result;    
	
    result.push_back("aa");
	result.push_back("B");
	result.push_back("C");
	result.push_back("D");
	result.push_back("E");
	result.push_back("ff");
	result.push_back("G");
	
    return result;
}

std::vector<std::vector <int> > exampleMatrixInt()
{
    std::vector<int> column1;    
	column1.push_back(2);
	column1.push_back(1);
	column1.push_back(0);
	column1.push_back(25);
	column1.push_back(0);
	column1.push_back(-25);
	column1.push_back(-5);
	
    std::vector<int> column2;    
	column2.push_back(20);
	column2.push_back(10);
	column2.push_back(40);
	column2.push_back(250);
	column2.push_back(9);
	column2.push_back(-125);
	column2.push_back(-253);
	
    std::vector<std::vector<int> > resultMatrix;
	resultMatrix.push_back(column1);
	resultMatrix.push_back(column2);
	
    return resultMatrix;
}

std::vector<std::vector <double> > exampleMatrixDouble()
{
    std::vector<double> column1;    
	column1.push_back(2.16);
	column1.push_back(1.146);
	column1.push_back(0.4967);
	column1.push_back(25.97797);
	column1.push_back(0.0);
	column1.push_back(-25.47497);
	column1.push_back(-5.1454);

	std::vector<double> column2;    
	column2.push_back(20.4964);
	column2.push_back(10.16);
	column2.push_back(40.987);
	column2.push_back(250.888);
	column2.push_back(9.63);
	column2.push_back(-125.14);
	column2.push_back(-253.25);

	std::vector<std::vector<double> > resultMatrix;
	resultMatrix.push_back(column1);
	resultMatrix.push_back(column2);

	return resultMatrix;
}

std::vector<std::vector <std::string> > exampleMatrixString()
{
    std::vector<std::string> column1;    
	column1.push_back("aa");
	column1.push_back("B");
	column1.push_back("C");
	column1.push_back("D");
	column1.push_back("E");
	column1.push_back("ff");
	column1.push_back("G");

	std::vector<std::string> column2;    
	column2.push_back("Z");
	column2.push_back("yyy");
	column2.push_back("X");
	column2.push_back("www");
	column2.push_back("V");
	column2.push_back("uuu");
	column2.push_back("Q");

	std::vector<std::vector<std::string> > resultMatrix;
	resultMatrix.push_back(column1);
	resultMatrix.push_back(column2);
	
    return resultMatrix;
}


//
// LA Example Functions - Wrapped Above
// ------------------------------------------------------------------------------------------------------------
//

// these are implementation example functions demonstrating the supported return types
int getInt() {	return 1; }
double getDouble() { return 2.5; } 
char getChar() { return 'z'; };
bool getBool() { return false; };
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
const int throwAnException() {
	throw std::runtime_error("ERROR: This exception text was generated inside the SWIG DLL through a std::exception");
	return 0;
}
const int throwAnIllegalArgumentException() {
	throw std::invalid_argument("ERROR: This exception text was generated inside the SWIG DLL through a std::invalid_argument");
	return 0;
};

const double sumUp(const double inputArray[], const int sizeOfArray) {
	double resultOut = 0.0;
	//  sizeOfArray = (sizeof(inputArray)/sizeof(*inputArray)) ;   
	for(int i =0; i < sizeOfArray; i++) {
		resultOut +=  inputArray[i];
	}
	return resultOut;
}

const double sumUp(const std::vector<double>& inputArray) {
	double resultOut = 0.0;
	for(size_t i =0; i < inputArray.size(); i++) {
		resultOut +=  inputArray.at(i);
	}
	return resultOut;
}


double doSomeFormula(const double z) { return AnalyticFormulae::diffNormdist(z); }