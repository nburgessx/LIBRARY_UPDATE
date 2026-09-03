/*! @file
    @brief  Value print class
*/
//  2007, AlgoQuantHub.
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


class AQLDataInstance;
class AQLObject;
class AQLString;
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
	static int printValue(const std::vector<AQLString> &mainTradeVec, 
		const std::vector<AQLString> &fileNumVec, const std::vector<AQLDataInstance *> &rootVec, const std::vector<AQLString> &ccyVec, AQLString calc = AQ_NO_DATA);

	// print fee value
	static int printFeeValue(const std::vector<AQLString> &mainTradeVec,
		const std::vector<AQLString> &fileNumVec, const std::vector<AQLDataInstance *> &rootVec);

	// ! print leg cashflow and option param
	static int printCF( const std::vector< AQLString >& mainTradeVec,
						const std::vector< AQLString >& fileNumVec,
						const std::vector< AQLDataInstance* >& rootVec,
						const std::vector< AQLString >& ccyVec,
						AQLString calc = AQ_NO_DATA
					  );
	// ! print object dump
	static int printEntityDump(const std::vector< AQLString >& fileNumVec,
                               const std::vector< AQLDataInstance* >& rootVec
                               );
	// ! print compounded rate
	static int printCompoundedRates(const AQLObject& trade,
						            const AQLString& fileNum,
									const AQLDataInstance* dataInstance,
									const AQLString& ccy,
									std::ostream& dest,
									AQLString calc = AQ_NO_DATA);
	// ! print compounded rate
	static void printCompoundedRates(const int leg_num,
		                             const AQLObject& trade,
									 const AQLString& fileNum,
									 const AQLDataInstance* dataInstance,
									 const AQLString& ccy,
									 std::ostream& dest,
									 AQLString calc = AQ_NO_DATA);


	static void changeSemiAnalyticResult(AQLDataInstance& dataInstance,const AQLString& infileName, std::ofstream &file);

    static void printFxOptionMatrix(AQLDataInstance* dataInstance, const AQLString& filenum);
private:
	//==============================================================================
	// print pv value
	static bool printPV(const AQLString &name, const std::vector<const AQLObject*> &tradeEntities, std::ofstream &file, const AQLString &ccys, bool isDetail, bool isvanilla = false);
	//==============================================================================
	// print risk value
	static bool printRisk(const AQLString &name, const AQLString &riskName, const std::vector<const AQLObject*> &tradeEntities, const AQLObject &riske, std::ofstream &file, bool isBase = false);
	//==============================================================================
	// print data name
	static bool printAttr(AQLDataInstance *dataInstance, std::ofstream &file, const AQLString &ccys, bool isDetail, bool isvanilla = false, bool isSummary = false);
	
	static bool printIMMFwdRisk(const AQLString &name, const AQLString &riskName, const std::vector<const AQLObject*> &tradeEntities, const AQLObject &riske, std::ofstream &file);

	static AQLString outStr(double value);

	//print fee cash flow
	static void MAValuePrinter::printFeeCF(AQLObjectHolder& tradeEntity, std::ofstream& outFile);

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