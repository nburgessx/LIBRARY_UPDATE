#pragma once
#include <string>
#include <vector>
#include <exception>

// these are example functions demonstrating the supported return types
int getInt();
double getDouble();
char getChar();
bool getBool();
const char* getConstChar(); 
std::string getStdString();
const std::vector<int> getVectorOfInt();
const std::vector<std::vector <int> > getVectorOfVectorOfInt();
const std::vector<double> getVectorOfDouble();
const std::vector<std::vector <double> > getVectorOfVectorOfDouble();
const std::vector<std::string> getVectorOfString();
const std::vector<std::vector <std::string> > getVectorOfVectorOfString();
const std::string testVectorOfVectorOfDoubles(const std::vector<std::vector <std::string> >& in);
const int throwAnException() throw(std::exception);
const int throwAnIllegalArgumentException() throw(std::exception);
const double sumUp(const double inputArray[], const int sizeOfArray) throw(std::exception);
const double sumUp(const std::vector<double>& inputArray) throw(std::exception);



// testing an MLIB function that does not require static data
double doSomeFormula(const double z);

// should be called before using any MLIB related function
std::string setUpMLIB(const std::string& irPropsFullFilePath, const std::string& calendarFullFilePath);

// should be called before termination of client (or end of using MLIB functions)
const std::string tearDownMLIB();








