
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLPriceHWCalibration.h"
#include "AQLObject.h"
#include "AQLDataProcedure.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLObjectPool.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataFunction.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "AQLDateCalculations.h"
#include "AQLPriceCFGenUtility.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLMathValuableEntity.h"

#include "AQLModelDynamicsHW1FCurve.h"
#include "AQLPriceTargetFunction.h"
#include "AQLPriceLSTargetFunction.h"
#include "AQLPriceCashFlowGenerator.h"
#include <algorithm>

using namespace std;

class AQLMathJamshidianSwaptionDataProvider;
class IRCalibLSTool
{
public:
	// constructor
	explicit IRCalibLSTool(AQLDataDoubles* pAttr) : mpAttr(pAttr) {;}
	/*!
		@brief set up parameter for function pointed by a member variable
	*/
	void operator = (const DoubleArray& param) const
	{
		mpAttr->set(param);
	};
    void operator = (const double param) const
    {
        mpAttr->set(param, 0);
    };
	AQLDataDoubles* mpAttr; // pointer to AQLDataDoubles 
};



///////////////////////////////////////////////////////////////////////
/*!
    @brief default constructor
*/
AQLPriceHWCalibration::AQLPriceHWCalibration()
: AQLCoreProcedure()
{
}
/*!
    @brief destructor

*/
AQLPriceHWCalibration::~AQLPriceHWCalibration()
{
}
/*!
    @brief  Check this class ID is the same or not

	@param[in] id function ID
	@return true or false
*/
bool
AQLPriceHWCalibration::isTypeOf(function_t id) const
{
	return (id == FN_IR_HWCALIBRATION ? true :
						AQLCoreProcedure::isTypeOf(id));
}
/*!
    @brief  Copy this class

	@return pointer to copied object
*/
AQLCoreFunctionBase*		
AQLPriceHWCalibration::clone() const
{
    try 
	{
        return new AQLPriceHWCalibration();
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief Retern class type

	@return class type
*/
function_t			
AQLPriceHWCalibration::getType() const
{
	return FN_IR_HWCALIBRATION;
}

/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLPriceHWCalibration::registerData(AQLPriceDataManager& dm) const
{
	dm.setData(PRICING_DATA_CALIBRATORENGINE,		DATA_PROCEDURE);
	dm.setData(CALIBRATION_DATA_CALIBRATIONDATA,				DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_SDEINFO,				DATA_REFERENCE);

}

/*!
	@brief Generate CashFlow

	@param[in] basedate basedate
	@param[in,out] leg evaluated Object 
	@param[in] att Data with estimation procedure class 

	@note basedate is not used in estimation
*/
void	            
AQLPriceHWCalibration::calibrateModel(const AQLDate& basedate, 
										AQLObject& object, 
										const AQLDataProcedure& att) const
{
	
	(void)basedate; (void)att; 
	
	AQLDataHolder* dh;
	dh = &(object.getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL));
	const AQLDataMultiReference& attrdata = dynamic_cast<AQLDataMultiReference&>(dh->get());
	std::vector<AQLObject*> data;
	for(unsigned int i=0;i<attrdata.getSize();i++)
		data.push_back(&attrdata.get(i).get());
	std::vector<AQLObject*>::iterator it = data.begin();
	//sort about option maturity
	sort(data.begin(), data.end(), Comp_term());

	//set up calibration object matrix 
	std::vector< std::vector<AQLObject*> > datamat(data.size());
	dh = &(data[0]->getData(IR_CALIBRATION_DATA_OPTIONMATURITY,ISNOTNULL));
	datamat[0].push_back(data[0]);
	AQLString tenorstr1 = dynamic_cast<AQLDataString &>(dh->get()).get();
	tenorstr1.toUpper();
	AQLString tenorstr2;
	AQLStringVector resultmaturity(1,tenorstr1);
	unsigned int j = 0;
	for(unsigned int i=1; i< attrdata.getSize();i++)
	{
		dh = &(data[i]->getData(IR_CALIBRATION_DATA_OPTIONMATURITY,ISNOTNULL));
		tenorstr2 = dynamic_cast<AQLDataString &>(dh->get()).get();
		tenorstr2.toUpper();
		if(tenorstr1 == tenorstr2)
		{
			datamat[j].push_back(data[i]);
		}
		else
		{
			datamat[++j].push_back(data[i]);
			resultmaturity.push_back(tenorstr2);
		}
		tenorstr1 = tenorstr2;
	}
	datamat.resize(j+1);
	//sort about swaptenor
	unsigned int calibSize = datamat.size();
	for(unsigned int i =0;i<calibSize;i++)
	{
		it = datamat[i].begin();
		sort(datamat[i].begin(), datamat[i].end(), Comp_swaptenor());
	}

	//asof 
	dh = &(object.getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL));
	AQLDate asof = dynamic_cast<AQLDataDate &>(dh->get()).get();

	dh = &(object.getData(PRICING_DATA_SDEINFO,ISNOTNULL));
	AQLObject& vole = dynamic_cast<AQLDataReference&>(dh->get()).get().get();

	double rstar=0.01;
	unsigned int startpos = 0;
	DoubleVector resultconvergvec(calibSize,0.0);
	DoubleVector resultvolvec(calibSize,0.0);
	//set up calibration method
	for(unsigned int i=0;i<calibSize;i++)
	{
		// Least Square Function
		AQLPriceLSTargetFunction<IRCalibLSTool> method;
		method.setBaseDate(asof);

		// Target variable - calibvariables
		dh = &(object.getData(PRICING_DATA_SDEINFO,ISNOTNULL));
		AQLObject& models = dynamic_cast<AQLDataReference&>(dh->get()).get().get();
		dh = &(models.getData(PRICING_DATA_CALIBVARIABLES,ISNOTNULL));
		IRCalibLSTool tool(&dynamic_cast<AQLDataDoubles &>(dh->get()));
		method.setVariable(tool);

		unsigned int LSsize = datamat[i].size();
		AQLMathObjectValue    temp_entity(object.getDataInstance() );
        vector< pair<AQLMathObjectValue*, AQLMathObjectValue*> > target(LSsize);
		//set target
		for(unsigned j=0;j<LSsize;j++)
		{
			//set startpos
			datamat[i][j]->remove(PRICING_DATA_STARTPOSITION);
			datamat[i][j]->add(PRICING_DATA_STARTPOSITION,new AQLDataInt(startpos));
			datamat[i][j]->remove(PRICING_DATA_INITIALRSTAR);
			datamat[i][j]->add(PRICING_DATA_INITIALRSTAR, new AQLDataDouble(rstar));
			//set target
			target[j].first = dynamic_cast<AQLMathObjectValue*>(datamat[i][j]);
			target[j].second = &temp_entity;
		}
		method.set(target);

		AQLOptimumBrent brent;
		DoubleArray x(1,0.001);
		double valconvergence = brent.findMinimum(method, x);

		dh = &(datamat[i][0]->getData(CALIBRATION_DATA_VALUE,ISNOTNULL));
		AQLDataValuation& attrval = dynamic_cast<AQLDataValuation&>(dh->get());
		const AQLCoreValuation& valuation = attrval.getMethod();
		//set next startpos
		startpos = dynamic_cast<const AQLMathJamshidianSwaption &>(valuation).getNextPos(attrval);
		//set next rstar
		rstar =  dynamic_cast<const AQLMathJamshidianSwaption &>(valuation).getNextRstar(attrval);
		//set next volresult
		DoubleVector volresult = dynamic_cast<const AQLMathJamshidianSwaption &>(valuation).getVolatilityResult(attrval);
		if(calibSize-1==i)
		{
			double vollast = volresult[startpos-1];
			unsigned int size = volresult.size()-startpos;
			volresult.erase(volresult.begin()+startpos,volresult.end());
			volresult.insert(volresult.begin()+startpos,size,vollast);
		}
		dynamic_cast<AQLDataDoubles&>(vole.getData(PRICING_DATA_CALIBVOL_T,ISNOTNULL).get()).set(volresult);

		for(unsigned int j=0;j<LSsize;j++)
		{
			dh = &(datamat[i][j]->getData(CALIBRATION_DATA_VALUE,ISNOTNULL));
			AQLDataValuation& attrval2 = dynamic_cast<AQLDataValuation&>(dh->get());
			const AQLCoreValuation& valuation2 = attrval2.getMethod();
			datamat[i][j]->remove("OptionPremiumResult");
			double simprem = dynamic_cast<const AQLMathJamshidianSwaption &>(valuation2).getSimPrem(attrval2);
			datamat[i][j]->add("OptionPremiumResult",new AQLDataDouble(simprem));

		}
		
		resultconvergvec[i] = valconvergence;
		resultvolvec[i] = x[0];
	}

	//make result object;
	vole.remove("ResultVolatility");
	vole.add("ResultVolatility",new AQLDataDoubles(resultvolvec));

	vole.remove("ResultConvergenceValue");
	vole.add("ResultConvergenceValue", new AQLDataDoubles(resultconvergvec));

	vole.remove("ResultMaturity");
	vole.add("ResultMaturity",new AQLDataStrings(resultmaturity));
}
