/*! @file
    @brief  Value print class
*/
//  2007, Mizuho International London.
#ifndef MAValuePrinter_h
#define MAValuePrinter_h

///X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MAValuePrinter.h
//
//  DESCRIPTION :      Value print class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif


#include <fstream>
#include <vector>
#include "LADefinitions.h"


class LADataInstance;
class LAObject;
class LAString;
struct MAIndexData;

//===================== Class Declare MAValuePrinter==================================
/*! 
    @brief Value print class 

   
*/
class MAValuePrinter
{
public:
	//==============================================================================
	// print value
	static int printValue(const std::vector<LAString> &mainTradeVec, 
		const std::vector<LAString> &fileNumVec, const std::vector<LADataInstance *> &rootVec, const std::vector<LAString> &ccyVec, LAString calc = MLIB_NO_DATA);

	// print fee value
	static int printFeeValue(const std::vector<LAString> &mainTradeVec,
		const std::vector<LAString> &fileNumVec, const std::vector<LADataInstance *> &rootVec);

	// ! print leg cashflow and option param
	static int printCF( const std::vector< LAString >& mainTradeVec,
						const std::vector< LAString >& fileNumVec,
						const std::vector< LADataInstance* >& rootVec,
						const std::vector< LAString >& ccyVec,
						LAString calc = MLIB_NO_DATA
					  );
	// ! print object dump
	static int printEntityDump(const std::vector< LAString >& fileNumVec,
                               const std::vector< LADataInstance* >& rootVec
                               );
	// ! print compounded rate
	static int printCompoundedRates(const LAObject& trade,
						            const LAString& fileNum,
									const LADataInstance* dataInstance,
									const LAString& ccy,
									std::ostream& dest,
									LAString calc = MLIB_NO_DATA);
	// ! print compounded rate
	static void printCompoundedRates(const int leg_num,
		                             const LAObject& trade,
									 const LAString& fileNum,
									 const LADataInstance* dataInstance,
									 const LAString& ccy,
									 std::ostream& dest,
									 LAString calc = MLIB_NO_DATA);


	static void changeSemiAnalyticResult(LADataInstance& dataInstance,const LAString& infileName, std::ofstream &file);

    static void printFxOptionMatrix(LADataInstance* dataInstance, const LAString& filenum);
private:
	//==============================================================================
	// print pv value
	static bool printPV(const LAString &name, const std::vector<const LAObject*> &tradeEntities, std::ofstream &file, const LAString &ccys, bool isDetail, bool isvanilla = false);
	//==============================================================================
	// print risk value
	static bool printRisk(const LAString &name, const LAString &riskName, const std::vector<const LAObject*> &tradeEntities, const LAObject &riske, std::ofstream &file, bool isBase = false);
	//==============================================================================
	// print data name
	static bool printAttr(LADataInstance *dataInstance, std::ofstream &file, const LAString &ccys, bool isDetail, bool isvanilla = false, bool isSummary = false);
	
	static bool printIMMFwdRisk(const LAString &name, const LAString &riskName, const std::vector<const LAObject*> &tradeEntities, const LAObject &riske, std::ofstream &file);

	static LAString outStr(double value);

	//print fee cash flow
	static void MAValuePrinter::printFeeCF(LAObjectHolder& tradeEntity, std::ofstream& outFile);

private:
	// constructor
	MAValuePrinter(void);
	// destructor
	~MAValuePrinter(void);
	// copy constructor
	MAValuePrinter(const MAValuePrinter &rhs);
	MAValuePrinter &operator=(const MAValuePrinter &rhs);


};

#endif