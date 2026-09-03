//  2008, Mizuho International London.
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceFXDisplacedDiffusionCalibration.h"
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
#include "LAStepInterpolation.h"
#include "LADataMatrix.h"
#include "LAMathValuableEntity.h"

#include "LAPriceTargetFunction.h"
#include "LAPriceLSTargetFunction.h"
#include "LAPriceCashFlowGenerator.h"
#include "LAOptimumBFGS.h"
#include <algorithm>
#include <float.h>

using namespace std;

//const unsigned int  BFGSMAXNUM = 30;
const unsigned int  BFGSMAXNUM = 60;
const double BFGSPRECISION = 1E-8;

class LAMathJamshidianSwaptionDataProvider;
class FXDDCalibLSTool
{
public:
	// constructor
	explicit FXDDCalibLSTool(LADataDoubles* pAttr) : mpAttr(pAttr) {;}
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
LAPriceFXDisplacedDiffusionCalibration::LAPriceFXDisplacedDiffusionCalibration()
: LACoreProcedure()
{
}

/*!
    @brief copy constructor
*/
LAPriceFXDisplacedDiffusionCalibration::LAPriceFXDisplacedDiffusionCalibration(const LAPriceFXDisplacedDiffusionCalibration& p)
: LACoreProcedure(p)
{

}

/*!
    @brief destructor

*/
LAPriceFXDisplacedDiffusionCalibration::~LAPriceFXDisplacedDiffusionCalibration()
{
}
/*!
    @brief  Check this class ID is the same or not

	@param[in] id function ID
	@return true or false
*/
bool
LAPriceFXDisplacedDiffusionCalibration::isTypeOf(function_t id) const
{
	return (id == FN_IR_FXDISPLACEDDIFFUSIONCALIBRATION ? true :
						LACoreProcedure::isTypeOf(id));
}
/*!
    @brief  Copy this class

	@return pointer to copied object
*/
LACoreFunctionBase*		
LAPriceFXDisplacedDiffusionCalibration::clone() const
{
    try 
	{
        return new LAPriceFXDisplacedDiffusionCalibration();
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
LAPriceFXDisplacedDiffusionCalibration::getType() const
{
	return FN_IR_FXDISPLACEDDIFFUSIONCALIBRATION;
}

/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceFXDisplacedDiffusionCalibration::registerData(LAPriceDataManager& dm) const
{
	dm.setData(PRICING_DATA_CALIBRATORENGINE,		DATA_PROCEDURE);
	dm.setData(CALIBRATION_DATA_CALIBRATIONDATA,				DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_SDEINFO,				DATA_REFERENCE);
	dm.setData(PRICING_DATA_BETACAP,				DATA_DOUBLE);
	dm.setData(PRICING_DATA_BETAFLOOR,				DATA_DOUBLE);
	dm.setData(PRICING_DATA_DOMESTICCURVE,			DATA_REFERENCE);
	dm.setData(PRICING_DATA_FOREIGNCURVE,			DATA_REFERENCE);
	dm.setData(PRICING_DATA_ISUSEDF2FORDOMESTIC,	DATA_BOOL);
	dm.setData(PRICING_DATA_ISUSEDF2FORFOREIGN,	DATA_BOOL);
	dm.setData(PRICING_DATA_SPOTFX,				DATA_DOUBLE);
	dm.setData(PRICING_DATA_DOMESTICIRMODEL,		DATA_REFERENCE);
	dm.setData(PRICING_DATA_FOREIGNIRMODEL,		DATA_REFERENCE);
	dm.setData(PRICING_DATA_MARKETTERMS,			DATA_DOUBLES);
	dm.setData(PRICING_DATA_MARKETVOLDD,			DATA_DOUBLES);
	dm.setData(PRICING_DATA_MARKETSKEWDD,			DATA_DOUBLES);
	dm.setData(PRICING_DATA_AVERAGEMARKETVOLDD,	DATA_DOUBLES);
	dm.setData(PRICING_DATA_AVERAGEMARKETSKEWDD,	DATA_DOUBLES);

}

/*!
	@brief Generate CashFlow

	@param[in] basedate basedate
	@param[in,out] leg evaluated Object 
	@param[in] att Data with estimation procedure class 

	@note basedate is not used in estimation
*/
void	            
LAPriceFXDisplacedDiffusionCalibration::calibrateModel(const LADate& basedate, 
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
	////asof 
	dh = &(object.getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL));
	LADate asof = dynamic_cast<LADataDate &>(dh->get()).get();
	//vole object
	dh = &(object.getData(PRICING_DATA_SDEINFO,ISNOTNULL));
	LAObject& vole = dynamic_cast<LADataReference&>(dh->get()).get().get();
	//get betacap and betafloor
	double bcap = DBL_MAX;
	double bfloor = DBL_MIN;
	dh = &(object.getData(PRICING_DATA_BETACAP,NOCHECK));
	if(dh->isDefined())
		bcap = dynamic_cast<LADataDouble &>(dh->get()).get();
	dh = &(object.getData(PRICING_DATA_BETAFLOOR,NOCHECK));
	if(dh->isDefined())
		bfloor = dynamic_cast<LADataDouble &>(dh->get()).get();
	//making data for for grid calibration
	makeAttributeForGridEntity(basedate,object,att);

	unsigned int startpos = 0;
	unsigned int calibSize = attrdata.getSize();
	DoubleVector converg(calibSize,0.0),resultvol(calibSize,0.0),resultbeta(calibSize,0.0),
					resultvolTm(calibSize,0.0),resultbetaTm(calibSize,0.0);
	DoubleMatrix marketprem(5,DoubleVector(calibSize,0.0));
	LAStringVector resultmatu(calibSize);
	for(unsigned int i=0;i<calibSize;i++)
	{
		// Least Square Function
		LAPriceLSTargetFunction<FXDDCalibLSTool> method;
		method.setBaseDate(asof);
		// Target variable - calibvariables
		dh = &(vole.getData(PRICING_DATA_CALIBVARIABLES,ISNOTNULL));
		FXDDCalibLSTool tool(&dynamic_cast<LADataDoubles &>(dh->get()));
		method.setVariable(tool);

		unsigned int LSsize = 1;
		LAMathObjectValue    temp_entity(object.getDataInstance() );
        vector< pair<LAMathObjectValue*, LAMathObjectValue*> > target(LSsize);
		
		//set startpos
		data[i]->remove(PRICING_DATA_STARTPOSITION);
		data[i]->add(PRICING_DATA_STARTPOSITION,new LADataInt(startpos));
		//set target
		target[0].first = dynamic_cast<LAMathObjectValue*>(data[i]);
		target[0].second = &temp_entity;
		method.set(target);
		
		//set optimum condition
		LAOptimumBFGS bfgs;
		LAOptimumOption& opt = bfgs.getOption();
		opt.mPrecision = BFGSPRECISION;
		opt.mIterMax = BFGSMAXNUM;
		opt.mException = false;
		opt.mBoundary = true;
		DoubleVector highb(2,DBL_MAX),lowb(2,DBL_MIN);
		//set vol cap;
		lowb[0] = 0.0;
		//set beta floor and cap;
		lowb[1] = bfloor;
		highb[1] = bcap;
		opt.mBoundLow = lowb;
		opt.mBoundHigh = highb;

		//initialval
		dh = &(data[i]->getData(PRICING_DATA_VOLATILITYS,ISNOTNULL));
		double atm = dynamic_cast<const LADataDoubles &>(dh->get()).get()[0];
		dh = &(data[i]->getData(PRICING_DATA_TERMBETA,ISNOTNULL));
		double termbeta = dynamic_cast<LADataDouble &>(dh->get()).get();
		DoubleVector x(2); x[0] = atm; x[1] = termbeta;
		double eps = 1E-3;
		if(termbeta < bfloor)
			x[1] = bfloor + eps;
		if(termbeta > bcap)
			x[1] = bcap -eps;

		double error = 0.0;
		//start optimization
		try
		{
			error = bfgs.findMinimum(method,x);
		}
		catch(LACoreError &err)
		{
			
			LAString attr = "FX Calibration, findMinimum error in BFGS. Calib num = " + LAString(static_cast<int>(i)) + ". [" + LAString(__FILE__) + ":" + LAString(__LINE__) + "]";
			LAString add = "\n";
			for (unsigned int i = 0;i < err.getSize();++i)
			{
				add += "<---";
				LAString tmp = LAString(err.getMsg(i)) + "[" + LAString(err.getFile(i)) + ":" + LAString(err.getLine(i)) + "]";
				attr += add + tmp;
			}
			//regist 
			LADataHolder &dh = object.getData(CALIBRATION_DATA_ERRORMESSAGES, NOCHECK);
			if (dh.isDefined() && !dh.isNull())
			{
				dynamic_cast<LADataStrings &>(dh.get()).push_back(attr);
			}
			else
			{
				LAStringVector msgs(1, attr);
				object.add(CALIBRATION_DATA_ERRORMESSAGES, new LADataStrings(msgs));
			}
		}

		dh = &(data[i]->getData(CALIBRATION_DATA_VALUE,ISNOTNULL));
		LADataValuation& attrval = dynamic_cast<LADataValuation&>(dh->get());
		const LACoreValuation& valuation = attrval.getMethod();
		//set next startpos and endpos
		startpos = dynamic_cast<const LAMathAntonovFXOption &>(valuation).getNextPos(attrval);
		data[i]->remove(PRICING_DATA_ENDPOSITION);
		data[i]->add(PRICING_DATA_ENDPOSITION,new LADataInt(startpos));
		//set next volresult
		DoubleVector volresult = dynamic_cast<const LAMathAntonovFXOption &>(valuation).getVolatilityResult(attrval);
		//set nextt begagrids
		DoubleVector betaresult = dynamic_cast<const LAMathAntonovFXOption &>(valuation).getBetaResult(attrval);
		if(calibSize-1==i)
		{
			double vollast = volresult[startpos-1];
			unsigned int size = volresult.size()-startpos;
			volresult.erase(volresult.begin()+startpos,volresult.end());
			volresult.insert(volresult.begin()+startpos,size,vollast);

			double betalast = betaresult[startpos-1];
			betaresult.erase(betaresult.begin()+startpos,betaresult.end());
			betaresult.insert(betaresult.begin()+startpos,size,betalast);
		}
		dynamic_cast<LADataDoubles&>(vole.getData(PRICING_DATA_FXVOLGRIDS,ISNOTNULL).get()).set(volresult);
		dynamic_cast<LADataDoubles&>(vole.getData(PRICING_DATA_FXBETAGRIDS,ISNOTNULL).get()).set(betaresult);
	
		double btm = dynamic_cast<const LAMathAntonovFXOption &>(valuation).getBetaTM(attrval);
		btm += (btm==0.0) ? 1E-7 : 0.0;
		double vtm = dynamic_cast<const LAMathAntonovFXOption &>(valuation).getVTM(attrval);
		double tm = dynamic_cast<const LAMathAntonovFXOption &>(valuation).getTM(attrval);
		if(tm==0.0)
			throw LACoreInvalidData("OptionMaturity 0.0 is prohibited",__FILE__,__LINE__); 
		//get volTm and betaTm 
		resultvolTm[i] = LAMath::sqrt(vtm/btm/btm/tm);
		resultbetaTm[i] = btm;
		//errorvalu
		converg[i] = error;
		//tenor
		dh = &(data[i]->getData(IR_CALIBRATION_DATA_OPTIONMATURITY,ISNOTNULL));
		resultmatu[i] = dynamic_cast<LADataString &>(dh->get()).get();

		dh = &(data[i]->getData(PRICING_DATA_STRIKES,ISNOTNULL));
		const DoubleVector& strikes = dynamic_cast<const LADataDoubles &>(dh->get()).get();
		dh = &(data[i]->getData(PRICING_DATA_STRIKESTRINGS,ISNOTNULL));
		const LAStringVector& deltastrs = dynamic_cast<const LADataStrings &>(dh->get()).get();
		unsigned int opSize = deltastrs.size();
		//check opaSize;
		if(marketprem.size() != opSize)
			throw LACoreInvalidData("Number of strikes should be the same in FXDisplacedDiffusionCalibration",__FILE__,__LINE__); 

		dh = &(data[i]->getData(PRICING_DATA_ISCALLS,ISNOTNULL));
		const BoolVector& iscalls = dynamic_cast<LADataBools &>(dh->get()).get();

		double FxTm= dynamic_cast<const LAMathAntonovFXOption &>(valuation).getFxTM(attrval);
		double PdTm = dynamic_cast<const LAMathAntonovFXOption &>(valuation).getPTM(attrval);
		for(unsigned int j = 0;j<opSize;j++)
		{
			if(iscalls[j])
				marketprem[j][i] = 
					dynamic_cast<const LAMathAntonovFXOption &>(valuation).DisplayDiffCall(btm,vtm,strikes[j],FxTm,PdTm);
			else
				marketprem[j][i] = 
					dynamic_cast<const LAMathAntonovFXOption &>(valuation).DisplayDiffPut(btm,vtm,strikes[j],FxTm,PdTm);
		}

	}

	//make result object;
	vole.remove("ResultVolatility");
	vole.add("ResultVolatility",new LADataDoubles(resultvolTm));

	vole.remove("ResultBeta");
	vole.add("ResultBeta",new LADataDoubles(resultbetaTm));

	vole.remove("ResultConvergenceValue");
	vole.add("ResultConvergenceValue", new LADataDoubles(converg));

	vole.remove("ResultMaturity");
	vole.add("ResultMaturity",new LADataStrings(resultmatu));
	
	//this is representive 
	dh = &(data[0]->getData(PRICING_DATA_STRIKESTRINGS,ISNOTNULL));
	const LAStringVector& deltastrs = dynamic_cast<const LADataStrings &>(dh->get()).get();
	for(unsigned int j=0;j<deltastrs.size();j++)
	{
		LAString key = deltastrs[j];
		key+="Strike";
		vole.remove(key);
		vole.add(key,new LADataDoubles(marketprem[j]));
	}

}

void
LAPriceFXDisplacedDiffusionCalibration::makeAttributeForGridEntity(const LADate& basedate,
															LAObject& object,
															const LADataProcedure& att) const
{
	LADataHolder* dh;
	dh = &(object.getData(PRICING_DATA_SDEINFO,ISNOTNULL));
	LAObject& vole = dynamic_cast<LADataReference&>(dh->get()).get().get();

	dh = &(vole.getData(PRICING_DATA_FXTIMEGRIDS,ISNOTNULL));
	const DoubleVector& timegrids = dynamic_cast<const LADataDoubles &>(dh->get()).get();
	
	unsigned int M = timegrids.size();

	//get ForwardFXVector
	dh = &(object.getData(PRICING_DATA_DOMESTICCURVE,ISNOTNULL));
	LAObject& dcurve = dynamic_cast<LADataReference &>(dh->get()).get().get();
	dh = &(object.getData(PRICING_DATA_ISUSEDF2FORDOMESTIC,ISNOTNULL));
	bool isdf2dom = dynamic_cast<LADataBool &>(dh->get()).get();
	dh = &(dcurve.getData(CALIBRATION_DATA_TERMS,ISNOTNULL));
	const DoubleVector& dcterm = dynamic_cast<const LADataDoubles &>(dh->get()).get();
	if(isdf2dom)
		dh = &(dcurve.getData(IR_CALIBRATION_DATA_DFS2,ISNOTNULL));
	else
		dh = &(dcurve.getData(IR_CALIBRATION_DATA_DFS,ISNOTNULL));
	const  DoubleVector& dcp = dynamic_cast<const LADataDoubles &>(dh->get()).get();
	
	LASplineInterpolation dcinter;
	dcinter.set(dcterm,dcp);
	DoubleVector Pd(M,0.0);
	for(unsigned int i=0;i<M;i++)
		Pd[i] = dcinter.value(timegrids[i]);
	
	dh = &(object.getData(PRICING_DATA_FOREIGNCURVE,ISNOTNULL));
	LAObject& fcurve = dynamic_cast<LADataReference &>(dh->get()).get().get();
	dh = &(object.getData(PRICING_DATA_ISUSEDF2FORFOREIGN,ISNOTNULL));
	bool isdf2for = dynamic_cast<LADataBool &>(dh->get()).get();
	dh = &(fcurve.getData(CALIBRATION_DATA_TERMS,ISNOTNULL));
	const DoubleVector& dfterm = dynamic_cast<const LADataDoubles &>(dh->get()).get();
	if(isdf2for)
		dh = &(fcurve.getData(IR_CALIBRATION_DATA_DFS2,ISNOTNULL));
	else
		dh = &(fcurve.getData(IR_CALIBRATION_DATA_DFS,ISNOTNULL));
	const DoubleVector& dcf = dynamic_cast<const LADataDoubles &>(dh->get()).get();
	
	LASplineInterpolation dfinter;
	dfinter.set(dfterm,dcf);
	DoubleVector Pf(M,0.0);
	for(unsigned int i=0;i<M;i++)
		Pf[i] = dfinter.value(timegrids[i]);

	DoubleVector fx(M,0.0);
	dh = &(object.getData(PRICING_DATA_SPOTFX,ISNOTNULL));
	double spotfx = dynamic_cast<LADataDouble &>(dh->get()).get();
	for(unsigned int i=0;i<M;i++)
		fx[i] = spotfx*Pf[i]/LAMath::max(Pd[i],1E-10);

	//getDomestic HW P Volatiliy
	dh = &(object.getData(PRICING_DATA_DOMESTICIRMODEL,ISNOTNULL));
	LAObject& hwd = dynamic_cast<LADataReference &>(dh->get()).get().get();
	dh = &(hwd.getData(PRICING_DATA_CALIBCANONICAL_T,ISNOTNULL));
	const DoubleVector& hwtermd = dynamic_cast<const LADataDoubles &>(dh->get()).get();
	dh = &(hwd.getData(PRICING_DATA_CALIBVOL_T,ISNOTNULL));
	const DoubleVector& hwvold = dynamic_cast<const LADataDoubles &>(dh->get()).get();
	dh = &(hwd.getData(PRICING_DATA_CALIBMEANREV_T,ISNOTNULL));
	double ad0 = dynamic_cast<LADataDoubles &>(dh->get()).get()[0];
	if(ad0<=0.0)
		throw LACoreInvalidData("Domestic mean reversion should be positive",__FILE__,__LINE__);
	
	DoubleVector irdv(M,0.0); DoubleMatrix Pdvolmat(M,irdv);
	LAStepInterpolation inter;
	inter.set(hwtermd,hwvold);
	for(unsigned int i=0;i<M;i++)
		irdv[i] = inter.value(timegrids[i]);

	for(unsigned int i=0;i<M;i++)
	{
		double t = timegrids[i];
		double vol = irdv[i];
		for(unsigned int j=0;j<M;j++)
		{
			if(i>j)
				continue;
			double T = timegrids[j];
			Pdvolmat[i][j] = -(1.0-LAMath::exp(-ad0*(T-t)))/ad0*vol;
		}
	}

	//getForeign HW P Volatiliy
	dh = &(object.getData(PRICING_DATA_FOREIGNIRMODEL,ISNOTNULL));
	LAObject& hwf = dynamic_cast<LADataReference &>(dh->get()).get().get();
	dh = &(hwf.getData(PRICING_DATA_CALIBCANONICAL_T,ISNOTNULL));
	const DoubleVector& hwtermf = dynamic_cast<const LADataDoubles &>(dh->get()).get();
	dh = &(hwf.getData(PRICING_DATA_CALIBVOL_T,ISNOTNULL));
	const DoubleVector& hwvolf = dynamic_cast<const LADataDoubles &>(dh->get()).get();
	dh = &(hwf.getData(PRICING_DATA_CALIBMEANREV_T,ISNOTNULL));
	double af0 = dynamic_cast<LADataDoubles &>(dh->get()).get()[0];
	if(af0<=0.0)
		throw LACoreInvalidData("Foreign mean reversion should be positive",__FILE__,__LINE__);

	DoubleVector irfv(M,0.0); DoubleMatrix Pfvolmat(M,irdv);
	LAStepInterpolation interf;
	interf.set(hwtermf,hwvolf);
	for(unsigned int i=0;i<M;i++)
		irfv[i] = interf.value(timegrids[i]);

	for(unsigned int i=0;i<M;i++)
	{
		double t = timegrids[i];
		double vol = irfv[i];
		for(unsigned int j=0;j<M;j++)
		{
			if(i>j)
				continue;
			double T = timegrids[j];
			Pfvolmat[i][j] = -(1.0-LAMath::exp(-af0*(T-t)))/af0*vol;
		}
	}

	//get bMat;
	DoubleMatrix bMat(M,DoubleVector(M,0.0));

	dh = &(object.getData(PRICING_DATA_SDECORRELATIONS,ISNOTNULL));
	const DoubleVector& cor = dynamic_cast<const LADataDoubles &>(dh->get()).get();
	double covdf = cor[0]; double covdfx = cor[1]; double covffx = cor[2];
	for(unsigned int i=0;i<M;i++)
	{
		for(unsigned int j=0;j<M;j++)
		{
			if(i>j)
				continue;
			double vold = Pdvolmat[i][j];
			double volf = Pfvolmat[i][j];
			bMat[i][j] = -2.0*covdfx * vold + 2.0 * covffx * volf;
		}
	}

	//get aVector
	DoubleMatrix aMat(M,DoubleVector(M,0.0));
	for(unsigned int i=0;i<M;i++)
	{
		for(unsigned int j=0;j<M;j++)
		{
			if(i>j)
				continue;
			double vd = Pdvolmat[i][j];
			double vf = Pfvolmat[i][j];
			aMat[i][j] = vd * vd + vf * vf - 2.0 * covdf * vd * vf;
		}
	}

	//set Fx
	dh = &(vole.getData(PRICING_DATA_FXFORWARDGRIDS,NOCHECK));
	if(dh->isDefined())
		dynamic_cast<LADataDoubles &>(dh->get()).set(fx);
	else
		vole.add(PRICING_DATA_FXFORWARDGRIDS,new LADataDoubles(fx));
	
	//set Pvolatility d
	dh = &(vole.getData(PRICING_DATA_IRDOMESTICBONDVOLATILITY,NOCHECK));
	if(dh->isDefined())
		dynamic_cast<LADataDoubleMatrix &>(dh->get()).set(Pdvolmat);
	else
		vole.add(PRICING_DATA_IRDOMESTICBONDVOLATILITY,new LADataDoubleMatrix(Pdvolmat));

	//set Pvolatility f
	dh = &(vole.getData(PRICING_DATA_IRFOREIGNBONDVOLATILITY,NOCHECK));
	if(dh->isDefined())
		dynamic_cast<LADataDoubleMatrix &>(dh->get()).set(Pfvolmat);
	else
		vole.add(PRICING_DATA_IRFOREIGNBONDVOLATILITY,new LADataDoubleMatrix(Pfvolmat));

	//set bMatrix
	dh = &(vole.getData(PRICING_DATA_BMATRIX,NOCHECK));
	if(dh->isDefined())
		dynamic_cast<LADataDoubleMatrix &>(dh->get()).set(bMat);
	else
		vole.add(PRICING_DATA_BMATRIX,new LADataDoubleMatrix(bMat));

	//set aMatrix
	dh = &(vole.getData(PRICING_DATA_AMATRIX,NOCHECK));
	if(dh->isDefined())
		dynamic_cast<LADataDoubleMatrix &>(dh->get()).set(aMat);
	else
		vole.add(PRICING_DATA_AMATRIX,new LADataDoubleMatrix(aMat));

	//set SDECorrelations
	dh = &(vole.getData(PRICING_DATA_SDECORRELATIONS,NOCHECK));
	if(dh->isDefined())
		dynamic_cast<LADataDoubles &>(dh->get()).set(cor);
	else
		vole.add(PRICING_DATA_SDECORRELATIONS,new LADataDoubles(cor));

	//set domestic DiscountFactor
	dh = &(vole.getData(PRICING_DATA_DOMESTICDFS,NOCHECK));
	if(dh->isDefined())
		dynamic_cast<LADataDoubles &>(dh->get()).set(Pd);
	else
		vole.add(PRICING_DATA_DOMESTICDFS,new LADataDoubles(Pd));

	//set Foreign DiscountFactor
	dh = &(vole.getData(PRICING_DATA_FOREIGNDFS,NOCHECK));
	if(dh->isDefined())
		dynamic_cast<LADataDoubles &>(dh->get()).set(Pf);
	else
		vole.add(PRICING_DATA_FOREIGNDFS,new LADataDoubles(Pf));
}
