//  2008, Mizuho International London.

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceHWCalibration.h"
#include "LAObject.h"
#include "LADataProcedure.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAObjectPool.h"
#include "LACoreTemplateType.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataFunction.h"
#include "LABasic.h"
#include "LAAlgorithm.h"
#include "LAMathDateCalculations.h"
#include "LAPriceCFGenUtility.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAMathValuableEntity.h"

#include "LAModelDynamicsHW1FCurve.h"
#include "LAPriceTargetFunction.h"
#include "LAPriceLSTargetFunction.h"
#include "LAPriceCashFlowGenerator.h"
#include <algorithm>

using namespace std;

class LAMathJamshidianSwaptionDataProvider;
class IRCalibLSTool
{
public:
	// constructor
	explicit IRCalibLSTool(LADataDoubles* pAttr) : mpAttr(pAttr) {;}
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
	LADataDoubles* mpAttr; // pointer to LADataDoubles 
};



///////////////////////////////////////////////////////////////////////
/*!
    @brief default constructor
*/
LAPriceHWCalibration::LAPriceHWCalibration()
: LACoreProcedure()
{
}
/*!
    @brief destructor

*/
LAPriceHWCalibration::~LAPriceHWCalibration()
{
}
/*!
    @brief  Check this class ID is the same or not

	@param[in] id function ID
	@return true or false
*/
bool
LAPriceHWCalibration::isTypeOf(function_t id) const
{
	return (id == FN_IR_HWCALIBRATION ? true :
						LACoreProcedure::isTypeOf(id));
}
/*!
    @brief  Copy this class

	@return pointer to copied object
*/
LACoreFunctionBase*		
LAPriceHWCalibration::clone() const
{
    try 
	{
        return new LAPriceHWCalibration();
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief Retern class type

	@return class type
*/
function_t			
LAPriceHWCalibration::getType() const
{
	return FN_IR_HWCALIBRATION;
}

/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceHWCalibration::registerData(LAPriceDataManager& dm) const
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
LAPriceHWCalibration::calibrateModel(const LADate& basedate, 
										LAObject& object, 
										const LADataProcedure& att) const
{
	
	(void)basedate; (void)att; 
	
	LADataHolder* dh;
	dh = &(object.getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL));
	const LADataMultiReference& attrdata = dynamic_cast<LADataMultiReference&>(dh->get());
	std::vector<LAObject*> data;
	for(unsigned int i=0;i<attrdata.getSize();i++)
		data.push_back(&attrdata.get(i).get());
	std::vector<LAObject*>::iterator it = data.begin();
	//sort about option maturity
	sort(data.begin(), data.end(), Comp_term());

	//set up calibration object matrix 
	std::vector< std::vector<LAObject*> > datamat(data.size());
	dh = &(data[0]->getData(IR_CALIBRATION_DATA_OPTIONMATURITY,ISNOTNULL));
	datamat[0].push_back(data[0]);
	LAString tenorstr1 = dynamic_cast<LADataString &>(dh->get()).get();
	tenorstr1.toUpper();
	LAString tenorstr2;
	LAStringVector resultmaturity(1,tenorstr1);
	unsigned int j = 0;
	for(unsigned int i=1; i< attrdata.getSize();i++)
	{
		dh = &(data[i]->getData(IR_CALIBRATION_DATA_OPTIONMATURITY,ISNOTNULL));
		tenorstr2 = dynamic_cast<LADataString &>(dh->get()).get();
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
	LADate asof = dynamic_cast<LADataDate &>(dh->get()).get();

	dh = &(object.getData(PRICING_DATA_SDEINFO,ISNOTNULL));
	LAObject& vole = dynamic_cast<LADataReference&>(dh->get()).get().get();

	double rstar=0.01;
	unsigned int startpos = 0;
	DoubleVector resultconvergvec(calibSize,0.0);
	DoubleVector resultvolvec(calibSize,0.0);
	//set up calibration method
	for(unsigned int i=0;i<calibSize;i++)
	{
		// Least Square Function
		LAPriceLSTargetFunction<IRCalibLSTool> method;
		method.setBaseDate(asof);

		// Target variable - calibvariables
		dh = &(object.getData(PRICING_DATA_SDEINFO,ISNOTNULL));
		LAObject& models = dynamic_cast<LADataReference&>(dh->get()).get().get();
		dh = &(models.getData(PRICING_DATA_CALIBVARIABLES,ISNOTNULL));
		IRCalibLSTool tool(&dynamic_cast<LADataDoubles &>(dh->get()));
		method.setVariable(tool);

		unsigned int LSsize = datamat[i].size();
		LAMathObjectValue    temp_entity(object.getDataInstance() );
        vector< pair<LAMathObjectValue*, LAMathObjectValue*> > target(LSsize);
		//set target
		for(unsigned j=0;j<LSsize;j++)
		{
			//set startpos
			datamat[i][j]->remove(PRICING_DATA_STARTPOSITION);
			datamat[i][j]->add(PRICING_DATA_STARTPOSITION,new LADataInt(startpos));
			datamat[i][j]->remove(PRICING_DATA_INITIALRSTAR);
			datamat[i][j]->add(PRICING_DATA_INITIALRSTAR, new LADataDouble(rstar));
			//set target
			target[j].first = dynamic_cast<LAMathObjectValue*>(datamat[i][j]);
			target[j].second = &temp_entity;
		}
		method.set(target);

		LAOptimumBrent brent;
		DoubleArray x(1,0.001);
		double valconvergence = brent.findMinimum(method, x);

		dh = &(datamat[i][0]->getData(CALIBRATION_DATA_VALUE,ISNOTNULL));
		LADataValuation& attrval = dynamic_cast<LADataValuation&>(dh->get());
		const LACoreValuation& valuation = attrval.getMethod();
		//set next startpos
		startpos = dynamic_cast<const LAMathJamshidianSwaption &>(valuation).getNextPos(attrval);
		//set next rstar
		rstar =  dynamic_cast<const LAMathJamshidianSwaption &>(valuation).getNextRstar(attrval);
		//set next volresult
		DoubleVector volresult = dynamic_cast<const LAMathJamshidianSwaption &>(valuation).getVolatilityResult(attrval);
		if(calibSize-1==i)
		{
			double vollast = volresult[startpos-1];
			unsigned int size = volresult.size()-startpos;
			volresult.erase(volresult.begin()+startpos,volresult.end());
			volresult.insert(volresult.begin()+startpos,size,vollast);
		}
		dynamic_cast<LADataDoubles&>(vole.getData(PRICING_DATA_CALIBVOL_T,ISNOTNULL).get()).set(volresult);

		for(unsigned int j=0;j<LSsize;j++)
		{
			dh = &(datamat[i][j]->getData(CALIBRATION_DATA_VALUE,ISNOTNULL));
			LADataValuation& attrval2 = dynamic_cast<LADataValuation&>(dh->get());
			const LACoreValuation& valuation2 = attrval2.getMethod();
			datamat[i][j]->remove("OptionPremiumResult");
			double simprem = dynamic_cast<const LAMathJamshidianSwaption &>(valuation2).getSimPrem(attrval2);
			datamat[i][j]->add("OptionPremiumResult",new LADataDouble(simprem));

		}
		
		resultconvergvec[i] = valconvergence;
		resultvolvec[i] = x[0];
	}

	//make result object;
	vole.remove("ResultVolatility");
	vole.add("ResultVolatility",new LADataDoubles(resultvolvec));

	vole.remove("ResultConvergenceValue");
	vole.add("ResultConvergenceValue", new LADataDoubles(resultconvergvec));

	vole.remove("ResultMaturity");
	vole.add("ResultMaturity",new LADataStrings(resultmaturity));
}
