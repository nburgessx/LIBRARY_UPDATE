#ifndef AQLCoreUtility_h
#define AQLCoreUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDataInstance.h"
#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include <map>
#include "ConstantDeclarations.h"
#include "AQLDataVector.h"
#include "AQLDataCSVFileLoader.h"

class AQLString;
class AQLObject;
class AQLPriceDataSlidingRule;
class AQLPriceDataCalendar;

#define searchbyrow(mat,str,c,iserror)(AQLCoreUtility::findElement(mat,str,0,c,true,iserror))
#define searchbycol(mat,str,r,iserror)(AQLCoreUtility::findElement(mat,str,r,0,false,iserror))
#define searchvecbyrow(mat,str,iserror)(AQLCoreUtility::findVector(mat,str,true,iserror))
#define searchvecbycol(mat,str,iserror)(AQLCoreUtility::findVector(mat,str,false,iserror))

class AQLCoreUtility
{
public:
	// find vector
	static AQLStringVector findVector(AQLStringMatrix& mat, const AQLString& input,bool ischangeroworcol, bool isemptyerror);
	// find element
	static AQLString findElement(AQLStringMatrix& mat, const AQLString& input, const int row, const int col, 
								bool ischangeroworcol, bool isemptyerror);
	// find rows number
	static const int  findRowsNumber(AQLStringMatrix& defmat, const AQLString& input);
	// find cols number
	static const int  findColsNumber(AQLStringMatrix& defmat, const AQLString& input);
	// removequotation2
	static void removeQuotation2(AQLString &input);
	// removequotation
	static AQLString removeQuotation(const AQLString & input);
	// removequotation
	static AQLStringVector removeQuotation(const AQLStringVector& inputvec);
	// removequotation
	static void removeQuotation(AQLStringVector& inputvec);
	// xll dump memory
	static void xlDumpMemory(AQLDataInstance* dataInstance, const AQLString& file);
	// find object
	static std::vector<const AQLObject*> findEntity(AQLDataInstance* dataInstance, const AQLString& keyname);
	// data load
	static void dataLoad(const AQLString& fname, AQLDataInstance &dataInstance, bool isolddataclear, bool iscompletedependency);
	// change double from string
	static DoubleArray changeDoubleFromString(const AQLStringVector& strvec);
    static DoubleMatrix changeDoubleFromString(const AQLStringMatrix& strMat);
    // change date from string
	static DateVector changeDateFromString(const AQLStringVector& strvec);

private:
	AQLCoreUtility(void);
	~AQLCoreUtility(void);
	AQLCoreUtility(const AQLCoreUtility &rhs);
	AQLCoreUtility &operator=(const AQLCoreUtility &rhs);
};


#endif
