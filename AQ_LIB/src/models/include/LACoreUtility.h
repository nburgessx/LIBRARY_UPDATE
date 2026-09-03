#ifndef LACoreUtility_h
#define LACoreUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LADataInstance.h"
#include "LADate.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include <map>
#include "ConstantDeclarations.h"
#include "LADataVector.h"
#include "LADataCSVFileLoader.h"

class LAString;
class LAObject;
class LAPriceDataSlidingRule;
class LAPriceDataCalendar;

#define searchbyrow(mat,str,c,iserror)(LACoreUtility::findElement(mat,str,0,c,true,iserror))
#define searchbycol(mat,str,r,iserror)(LACoreUtility::findElement(mat,str,r,0,false,iserror))
#define searchvecbyrow(mat,str,iserror)(LACoreUtility::findVector(mat,str,true,iserror))
#define searchvecbycol(mat,str,iserror)(LACoreUtility::findVector(mat,str,false,iserror))

class LACoreUtility
{
public:
	// find vector
	static LAStringVector findVector(LAStringMatrix& mat, const LAString& input,bool ischangeroworcol, bool isemptyerror);
	// find element
	static LAString findElement(LAStringMatrix& mat, const LAString& input, const int row, const int col, 
								bool ischangeroworcol, bool isemptyerror);
	// find rows number
	static const int  findRowsNumber(LAStringMatrix& defmat, const LAString& input);
	// find cols number
	static const int  findColsNumber(LAStringMatrix& defmat, const LAString& input);
	// removequotation2
	static void removeQuotation2(LAString &input);
	// removequotation
	static LAString removeQuotation(const LAString & input);
	// removequotation
	static LAStringVector removeQuotation(const LAStringVector& inputvec);
	// removequotation
	static void removeQuotation(LAStringVector& inputvec);
	// xll dump memory
	static void xlDumpMemory(LADataInstance* dataInstance, const LAString& file);
	// find object
	static std::vector<const LAObject*> findEntity(LADataInstance* dataInstance, const LAString& keyname);
	// data load
	static void dataLoad(const LAString& fname, LADataInstance &dataInstance, bool isolddataclear, bool iscompletedependency);
	// change double from string
	static DoubleArray changeDoubleFromString(const LAStringVector& strvec);
    static DoubleMatrix changeDoubleFromString(const LAStringMatrix& strMat);
    // change date from string
	static DateVector changeDateFromString(const LAStringVector& strvec);

private:
	LACoreUtility(void);
	~LACoreUtility(void);
	LACoreUtility(const LACoreUtility &rhs);
	LACoreUtility &operator=(const LACoreUtility &rhs);
};


#endif
