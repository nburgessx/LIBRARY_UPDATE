#pragma once
#include <string>
#include <vector>
#include <exception>

//
// Important AlgoQuantLib Initialization Functions
//


// Initialize AlgoQuantLib - Should be called to initialize AlgoQuantLib before using the analytics
// ---------------------------------------------------------------------------------

//
// ONLY PYTHON AND R SUPPORT DEFAULT ARGUMENTS, JAVA AND CSHARP DO NOT
// -------------------------------------------------------------------
//

std::string setUpMLIB( const std::string& irPropsFullFilePath, const std::string& calendarFullFilePath );

// Alternative Name(s) for the setUpMLIB function
std::string setupMLIB( const std::string& irPropsFullFilePath, const std::string& calendarFullFilePath );
std::string initMLIB( const std::string& irPropsFullFilePath, const std::string& calendarFullFilePath );

// Support for null default arguments
std::string setUpMLIB();
std::string setupMLIB();
std::string initMLIB();


// Terminate AlgoQuantLib - Should be called before termination AlgoQuantLib anaytics by the client
// ---------------------------------------------------------------------------------
std::string tearDownMLIB();


//
// Echo Functions
//
// These functions echo the input and return the input as output. This is so we can test the AlgoQuantLib API
// ---------------------------------------------------------------------------------
//

int             echoInt( int i );
double          echoDouble( double d );
char            echoChar( char c );
bool            echoBool( bool b);
std::string     echoString( const std::string& s );

std::vector<int>                          echoVectorInt( const std::vector<int>& v );
std::vector<double>                       echoVectorDouble( const std::vector<double>& v );
std::vector<std::string>                  echoVectorString( const std::vector<std::string>& v );

std::vector<std::vector <int> >           echoMatrixInt( const std::vector<std::vector <int> >& m );
std::vector<std::vector <double> >        echoMatrixDouble( const std::vector<std::vector <double> >& m );
std::vector<std::vector <std::string> >   echoMatrixString( const std::vector<std::vector <std::string> >& m);


//
// Example Functions - These are example functions demonstrating the supported return types
// ---------------------------------------------------------------------------------
//

int             exampleInt();
double          exampleDouble();
char            exampleChar();
bool            exampleBool();
std::string     exampleString();

std::vector<int>                          exampleVectorInt();
std::vector<double>                       exampleVectorDouble();
std::vector<std::string>                  exampleVectorString();

std::vector<std::vector <int> >           exampleMatrixInt();
std::vector<std::vector <double> >        exampleMatrixDouble();
std::vector<std::vector <std::string> >   exampleMatrixString();


//
// LA Example Functions - Wrapped Above
// ---------------------------------------------------------------------------------
//

// These are example functions demonstrating the supported return types
int getInt();
double getDouble();
char getChar();
bool getBool();
std::string getStdString();

const std::vector<int> getVectorOfInt();
const std::vector<std::vector <int> > getVectorOfVectorOfInt();
const std::vector<double> getVectorOfDouble();
const std::vector<std::vector <double> > getVectorOfVectorOfDouble();
const std::vector<std::string> getVectorOfString();
const std::vector<std::vector <std::string> > getVectorOfVectorOfString();
const std::string testVectorOfVectorOfDoubles(const std::vector<std::vector <std::string> >& in);

const int throwAnException();
const int throwAnIllegalArgumentException();

const double sumUp(const double inputArray[], const int sizeOfArray);
const double sumUp(const std::vector<double>& inputArray);


// testing an AlgoQuantLib function that does not require static data
double doSomeFormula(const double z);


