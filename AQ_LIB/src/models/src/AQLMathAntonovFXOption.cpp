#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathAntonovFXOption.h"
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
#include "AQLDataReference.h"
#include "AQLPriceDataFunction.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "AQLDateCalculations.h"
#include "AQLPriceCFGenUtility.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLDataMatrix.h"
#include "AQLModelDynamicsHW1FCurve.h"

#include "AQLCoreComponentManager.h"
#include "AQLMathFXVanillaFuncUtility.h"
#include "AQLMathIRVanillaFuncUtility.h"

using namespace std;

AQLMathAntonovFXOption::AQLMathAntonovFXOption()
: AQLCoreValuation()
{}

AQLMathAntonovFXOption::~AQLMathAntonovFXOption()
{
}

AQLMathAntonovFXOption::AQLMathAntonovFXOption(const AQLMathAntonovFXOption& v)
: AQLCoreValuation(v)
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
AQLMathAntonovFXOption::getType() const
{
	return FN_ANTONOVFXOPTIOIN;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLMathAntonovFXOption::registerData(AQLPriceDataManager& dm) const
{
	dm.setData(PRICING_DATA_ENDPOSITION,					DATA_INT);
	dm.setData(PRICING_DATA_STRIKES,						DATA_DOUBLES);
	dm.setData(PRICING_DATA_OPTIONPREMIUMS,				DATA_DOUBLES);
	dm.setData(PRICING_DATA_WEIGHTS,						DATA_DOUBLES);
	dm.setData(PRICING_DATA_ISCALLS,						DATA_BOOLS);
	dm.setData(PRICING_DATA_VOLATILITYS,					DATA_DOUBLES);
	dm.setData(PRICING_DATA_ISFITATM,						DATA_BOOL);
	dm.setData(PRICING_DATA_ISMAKECALIBDATAFROMVOLATILITY,	DATA_BOOL);
	dm.setData(PRICING_DATA_ISDELTANEUTRAL,				DATA_BOOL);
	dm.setData(PRICING_DATA_TERMBETA,						DATA_DOUBLE);
	dm.setData(PRICING_DATA_STRIKESTRINGS,					DATA_STRINGS);
	dm.setData(PRICING_DATA_SDECORRELATIONS,				DATA_DOUBLES);
	dm.setData(PRICING_DATA_FXTIMEGRIDS,					DATA_DOUBLES);
	dm.setData(PRICING_DATA_FXFORWARDGRIDS,				DATA_DOUBLES);
	dm.setData(PRICING_DATA_FXBETAGRIDS,					DATA_DOUBLES);
	dm.setData(PRICING_DATA_FXVOLGRIDS,					DATA_DOUBLES);
	dm.setData(PRICING_DATA_SPOTORFWD,						DATA_STRING);
	dm.setData(PRICING_DATA_DOMESTICDFS,					DATA_DOUBLES);
	dm.setData(PRICING_DATA_FOREIGNDFS,					DATA_DOUBLES);
	dm.setData(PRICING_DATA_IRDOMESTICBONDVOLATILITY,		DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_IRFOREIGNBONDVOLATILITY,		DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_BMATRIX,						DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_AMATRIX,						DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_DOMESTICDF,					DATA_DOUBLE);
	dm.setData(PRICING_DATA_FOREIGNDF,						DATA_DOUBLE);
	dm.setData(PRICING_DATA_FXTERM,						DATA_DOUBLE);
	dm.setData(PRICING_DATA_FORWARDFX,						DATA_DOUBLE);
	//furuya
	dm.setData(PRICING_DATA_FXTHETAGRIDS,					DATA_DOUBLES);
	dm.setData(PRICING_DATA_FXKAPPAGRIDS,					DATA_DOUBLES);
	dm.setData(PRICING_DATA_FXEPSILONGRIDS,				DATA_DOUBLES);
	dm.setData(PRICING_DATA_BOUNDARY_RANGE,				DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_INITIALRHO,					DATA_DOUBLE);

}

AQLCoreFunctionBase*
AQLMathAntonovFXOption::clone() const
{
    try 
	{
		return new AQLMathAntonovFXOption(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
	@brief value trade

	@param[in] basedate evaluate day
	@param[in,out] object trade object object(reference to AQLMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return swaption prem
	
*/
double
AQLMathAntonovFXOption::value(const AQLDate& basedate, AQLObject& object,
					const AQLDataValuation& att) const
{
	AQLDataHolder* dh;
	AQLMathAntonovFXOptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		dataProvider = dynamic_cast<AQLMathAntonovFXOptionDataProvider*>(setUpDataProvider(basedate, object, att));
	else
		dataProvider = &dynamic_cast<AQLMathAntonovFXOptionDataProvider&>(att.getDataProvider());

	//set variables
	dh = &(object.getData(PRICING_DATA_MODELPARAM,ISNOTNULL));
	AQLObject& models= dynamic_cast<AQLDataReference&>(dh->get()).get().get();
	dh = &(models.getData(PRICING_DATA_CALIBVARIABLES,ISNOTNULL));
	double varV = dynamic_cast<AQLDataDoubles&>(dh->get()).get()[0];
	double varB = dynamic_cast<AQLDataDoubles&>(dh->get()).get()[1];
	double T = dataProvider->mTm;

	dataProvider->mUpdateIntegral1flag = true;
	dataProvider->mUpdateABCflag = true;
	
	//virtual method set param mVGrids, mSGrids, mBetaGrids
	setCalibParam(dataProvider,varV,varB);
	
	dataProvider->reset(varV,varB,dataProvider->mSpos,dataProvider->mEpos);
	dataProvider->mBetaTm = getBetaTM(T,att);
	dataProvider->mVolTm = getVTM(T,att);

	double fx = dataProvider->mFxTm;
	double P = dataProvider->mPTm;
	
	double ret =0.0;
	double prem = 0.0;
	for(int i = dataProvider->mSmileNum -1;i>=0;i--)
	{
		prem = (dataProvider->mIsCallVec[i]) ? DisplayDiffCall(dataProvider->mBetaTm,dataProvider->mVolTm,dataProvider->mStrikeVec[i],fx,P) :
			DisplayDiffPut(dataProvider->mBetaTm,dataProvider->mVolTm,dataProvider->mStrikeVec[i],fx,P);
		
		ret+= (prem - dataProvider->mPremVec[i])*(prem - dataProvider->mPremVec[i])*dataProvider->mWeightVec[i]*dataProvider->mWeightVec[i];
	}

	ret+= (dataProvider->mIsfitATM) ? 20.0*(prem - dataProvider->mPremVec[0])*(prem - dataProvider->mPremVec[0]) : 0.0;

	return AQLMath::sqrt(ret);
	
}

/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*					
AQLMathAntonovFXOption::setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const
{
	//temporary
	AQLDate asof = basedate;

	AQLDataHolder* dh;
	AQLMathAntonovFXOptionDataProvider* dataProvider = dynamic_cast<AQLMathAntonovFXOptionDataProvider*>(createNewDataProvider());
	att.setDataProvider(dataProvider);

	dh = &(object.getData(PRICING_DATA_MODELPARAM,ISNOTNULL));
	AQLObject& models = dynamic_cast<AQLDataReference &>(dh->get()).get().get();
	dh = &(models.getData(PRICING_DATA_FXTIMEGRIDS,ISNOTNULL));
	DoubleVector timegrids = dynamic_cast<AQLDataDoubles &>(dh->get()).get();

	//get maturity point 
	dh = &(object.getData(IR_CALIBRATION_DATA_OPTIONMATURITY,ISNOTNULL));
	AQLString optionmatu = dynamic_cast<AQLDataString &>(dh->get()).get();
	
	dh = &(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISNOTNULL));
	const AQLPriceDataSlidingRule& sr = dynamic_cast<const AQLPriceDataSlidingRule &>(dh->get());

	dh = &(object.getData(IR_CALIBRATION_DATA_SPOTLAG,ISNOTNULL));
	const int spotlag = dynamic_cast<const AQLDataInt &>(dh->get());

	dh = &(object.getData(IR_CALIBRATION_DATA_CURRENCY,ISNOTNULL));
	const AQLString keyFX = dynamic_cast<const AQLDataString &>(dh->get());

	dh = &(object.getData(PRICING_DATA_FIXINGCALENDAR,ISNOTNULL));
	const AQLString fixcalstr = dynamic_cast<const AQLPriceDataCalendar &>(dh->get()).convertToString();

	dh = &(object.getData(PRICING_DATA_TERMCALENDAR,ISNOTNULL));
	const AQLPriceDataCalendar& termcal = dynamic_cast<const AQLPriceDataCalendar &>(dh->get());

	AQLDate spotdate = AQLDateCalculations::getFXSpotDate(keyFX, asof, fixcalstr, spotlag, true);
	AQLDate settledate = AQLDateCalculations::getDate(spotdate, optionmatu, sr, &termcal, true);
	AQLDate expdate =  AQLDateCalculations::getFXSpotDate(keyFX, settledate, fixcalstr, -spotlag, true);
	AQLPriceDataDayCount dc(ACT_365_ISDA);
	double matT = dc.getTerm(asof,expdate,false);
	//mSpos;
	dh = &(object.getData(PRICING_DATA_STARTPOSITION,ISNOTNULL));
	dataProvider->mSpos= dynamic_cast<AQLDataInt &>(dh->get()).get();
	//mEndpos
	unsigned int epos =0;
	AQLAlgorithm::locate(timegrids,matT,timegrids.size(),epos);
	if(epos == timegrids.size())
	{
		--epos;
	}
	else
	{
		double diff = AQLMath::abs(timegrids[epos] - matT); 
		if (diff > AQLMath::abs(matT - timegrids[epos - 1]))
		{
			--epos;
		}
	}
	//check for simulation grid is arbitry
	if (epos == 1)
		throw AQLCoreInvalidData("FxTimeGrid must be partitioned between 0 and First Option Maturity",__FILE__,__LINE__);
	//check whether FxVolterm is longer than FxOption maturity;
	if(matT > timegrids.back())
		throw AQLCoreInvalidData("FxTimeGrids must be longer than FxOption maturity",__FILE__,__LINE__);
	dataProvider->mEpos= epos;
	//set MaturityTime
	dataProvider->mTm = timegrids[epos];
	//set MaturityDiscount
	dh = &(models.getData(PRICING_DATA_DOMESTICDFS,ISNOTNULL));
	const DoubleVector& dfs = dynamic_cast<AQLDataDoubles &>(dh->get()).get();
	dataProvider->mPTm = dfs[epos];

	dh = &(models.getData(PRICING_DATA_FXFORWARDGRIDS,ISNOTNULL));
	DoubleVector fxgrids = dynamic_cast<AQLDataDoubles &>(dh->get()).get();
	if(fxgrids.size()<epos+1)
		throw AQLCoreInvalidData("Size of FXForwardGrids is small",__FILE__,__LINE__); 
	dataProvider->mFxTm = fxgrids[epos];
	//set FxFowardGrids
	fxgrids.resize(epos+1);
	dataProvider->mFxGrids = fxgrids;
	
	//set BetaGrids
	dh = &(models.getData(PRICING_DATA_FXBETAGRIDS,ISNOTNULL));
	DoubleVector betagrids = dynamic_cast<AQLDataDoubles &>(dh->get()).get();
	if(betagrids.size()<epos+1)
		throw AQLCoreInvalidData("Size of FXBetaGrids is small",__FILE__,__LINE__);
	//betagrids.resize(epos+1);
	dataProvider->mBetaGrids = betagrids;
	//set VolGrids
	dh = &(models.getData(PRICING_DATA_FXVOLGRIDS,ISNOTNULL));
	DoubleVector volgrids = dynamic_cast<AQLDataDoubles &>(dh->get()).get();
	if(volgrids.size()<epos+1)
		throw AQLCoreInvalidData("Size of FXVolGrids is small",__FILE__,__LINE__);
	dataProvider->mVolGrids = volgrids;
	//set Pvolatility d, Pfvolatility bmat, amat
	dh = &(models.getData(PRICING_DATA_IRDOMESTICBONDVOLATILITY,ISNOTNULL));
	DoubleMatrix Pdvolmat = dynamic_cast<AQLDataDoubleMatrix &>(dh->get()).get();

	dh = &(models.getData(PRICING_DATA_IRFOREIGNBONDVOLATILITY,ISNOTNULL));
	DoubleMatrix Pfvolmat = dynamic_cast<AQLDataDoubleMatrix &>(dh->get()).get();

	dh = &(models.getData(PRICING_DATA_BMATRIX,ISNOTNULL));
	DoubleMatrix bmat = dynamic_cast<AQLDataDoubleMatrix &>(dh->get()).get();

	dh = &(models.getData(PRICING_DATA_AMATRIX,ISNOTNULL));
	DoubleMatrix amat = dynamic_cast<AQLDataDoubleMatrix &>(dh->get()).get();
	for(unsigned int i=0;i<Pdvolmat.size();i++)
	{
		Pdvolmat[i].resize(epos+1);
		Pfvolmat[i].resize(epos+1);
		bmat[i].resize(epos+1);
		amat[i].resize(epos+1);
	}
	Pdvolmat.resize(epos+1);
	Pfvolmat.resize(epos+1);
	bmat.resize(epos+1);
	amat.resize(epos+1);

	dataProvider->mPdvol = Pdvolmat;
	dataProvider->mPfvol = Pfvolmat;
	dataProvider->mamat = amat;
	dataProvider->mbmat = bmat;

	//set timegrids;
	timegrids.resize(epos+1);
	dataProvider->mTimeGrids = timegrids;

	//set correlations
	dh = &(models.getData(PRICING_DATA_SDECORRELATIONS,ISNOTNULL));
	DoubleVector cor = dynamic_cast<AQLDataDoubles &>(dh->get()).get();
	dataProvider->mCovdf = cor[0]; 
	dataProvider->mCovdfx = cor[1];
	dataProvider->mCovffx = cor[2];

	//set calibration target vectors
	dh = &(object.getData(PRICING_DATA_STRIKESTRINGS,ISNOTNULL));
	AQLStringVector strikestrs = dynamic_cast<AQLDataStrings &>(dh->get()).get();

	//IsCallVec;
	dh = &(object.getData(PRICING_DATA_ISCALLS,ISNOTNULL));
	BoolVector iscallvec = dynamic_cast<AQLDataBools &>(dh->get()).get();

	//is make calibdata from volatility
	unsigned int smilenum = strikestrs.size();
	dh = &(object.getData(PRICING_DATA_ISMAKECALIBDATAFROMVOLATILITY,ISNOTNULL));
	bool isdatafromvol = dynamic_cast<AQLDataBool &>(dh->get()).get();
	DoubleVector strikevec(smilenum),premvec(smilenum);
	if(isdatafromvol)
	{
		double spot = dataProvider->mFxGrids[0];
		double fwd = dataProvider->mFxTm;
		double pd = dataProvider->mPTm;
		double term = dataProvider->mTm;
		
		dh = &(models.getData(PRICING_DATA_FOREIGNDFS,ISNOTNULL));
		const DoubleVector& dff = dynamic_cast<AQLDataDoubles &>(dh->get()).get();
		double pf = dff[dataProvider->mEpos];

		dh = &(object.getData(PRICING_DATA_VOLATILITYS,ISNOTNULL));
		const DoubleVector& vols = dynamic_cast<AQLDataDoubles &>(dh->get()).get();

		dh = &(object.getData(PRICING_DATA_SPOTORFWD,ISNOTNULL));
		AQLString spotfwd = dynamic_cast<AQLDataString &>(dh->get()).get();
		spotfwd.toUpper();
		
		dh = &(object.getData(PRICING_DATA_ISDELTANEUTRAL,ISNOTNULL));
		bool isnueutral = dynamic_cast<AQLDataBool &>(dh->get()).get();
		for(unsigned int j=0;j<smilenum;j++)
		{
			double vol = vols[j];
			double strike=0.0;double prem=0.0;
			//j==0 is ATM_
			if(j==0)
			{
				if(isnueutral)
					strike = fwd * AQLMath::exp(-0.5*vol*vol*term);
				else
					strike = fwd;
			}
			else
			{
				AQLString key = strikestrs[j];
				key.remove(key.size()-1,1);
				AQLStringVector keys = key.toToken('D');
				double delta = keys[0].getDoubleValue()* 0.01;
				AQLString highlow = keys[1];
				highlow.toUpper();
				try
				{
					strike = AQLMathFXVanillaFuncUtility::calcstrikefromdelta(delta,spotfwd,highlow,spot,fwd,vol,0.0,0.0,pf,term);
				}
				catch(AQLCoreError &err)
				{
					AQLString msg = err.getMsg();
					if (msg.findString("Not Convergence from rtsafe") == -1)
						throw AQLCoreNumericalError("Fx Option convergence does not work", __FILE__, __LINE__);
					
					strike = AQLMathFXVanillaFuncUtility::calcmaxstrike(spotfwd, spot, fwd, vol, 0.0, 0.0, pf, term);
				}
				
			}
			AQLString callput = (iscallvec[j]) ? "CALL" :"PUT";
			callput.toUpper();
			AQLString premstr("PREM");
			AQLString buy("BUY");
			prem = AQLMathIRVanillaFuncUtility::bkOption(premstr,buy,callput,fwd,strike,vol,pd,asof,expdate);

			strikevec[j] = strike;
			premvec[j] = prem;
		}
		object.remove(PRICING_DATA_STRIKES);
		object.add(PRICING_DATA_STRIKES,new AQLDataDoubles(strikevec));
		object.remove(PRICING_DATA_OPTIONPREMIUMS);
		object.add(PRICING_DATA_OPTIONPREMIUMS,new AQLDataDoubles(premvec));
	}
	else
	{
		//StrikeVec;
		dh = &(object.getData(PRICING_DATA_STRIKES,ISNOTNULL));
		strikevec = dynamic_cast<AQLDataDoubles &>(dh->get()).get();

		//PremVec
		dh = &(object.getData(PRICING_DATA_OPTIONPREMIUMS,ISNOTNULL));
		premvec = dynamic_cast<AQLDataDoubles &>(dh->get()).get();
	}

	//WeightVec
	dh = &(object.getData(PRICING_DATA_WEIGHTS,ISNOTNULL));
	DoubleVector weightvec = dynamic_cast<AQLDataDoubles &>(dh->get()).get();

	//isfitATM
	dh = &(object.getData(PRICING_DATA_ISFITATM,ISNOTNULL));
	bool isfitatm = dynamic_cast<AQLDataBool &>(dh->get()).get();

	if(isfitatm && 0.0 == weightvec[0])
		throw AQLCoreInvalidData("ATM weight must be non-zero when isFitATM true",__FILE__,__LINE__);

	if(smilenum != strikevec.size() || smilenum != weightvec.size() || smilenum != iscallvec.size())
		throw AQLCoreInvalidData("FX calibration fitting size is wrong",__FILE__,__LINE__);

	DoubleVector::iterator itw = weightvec.begin();
	DoubleVector::iterator itk = strikevec.begin();
	DoubleVector::iterator itp = premvec.begin();
	BoolVector::iterator itc = iscallvec.begin();
	for(itw=weightvec.begin();itw!=weightvec.end();)
	{
		if(0.0==(*itw))
		{
			itw = weightvec.erase(itw);
			itk = strikevec.erase(itk);
			itp = premvec.erase(itp);
			itc = iscallvec.erase(itc);
			continue;
		}
		++itw;++itk;++itp;++itc;
	}
	smilenum = premvec.size();
	dataProvider->mStrikeVec = strikevec;
	dataProvider->mPremVec = premvec;
	dataProvider->mWeightVec = weightvec;
	dataProvider->mIsCallVec = iscallvec;
	dataProvider->mIsfitATM = isfitatm;
	dataProvider->mSmileNum = static_cast<int>(smilenum);

	//set initial val
	dh = &(object.getData(PRICING_DATA_VOLATILITYS,ISNOTNULL));
	double tmpvol = dynamic_cast<const AQLDataDoubles &>(dh->get()).get()[0];
	dh = &(object.getData(PRICING_DATA_TERMBETA,ISNOTNULL));
	double tmpbeta = dynamic_cast<AQLDataDouble &>(dh->get()).get();
	
	//virtual method set param mVGrids, mSGrids, mBetaGrids
	setCalibParamFirst(dataProvider,tmpvol,tmpbeta);

	//set AQL1DDataSet
	unsigned int size = timegrids.size();
	AQLStepInterpolation inter;
	DoubleArray SigmaSt(size),SigmaFX(size), SigmaSt2(size),SigmaStSigmaFX(size);
	for(unsigned int i=0;i<size;i++)
	{
		double t = timegrids[i];
		//modify antonov2
	/*	double a = dataProvider->getA(i);
		double b = dataProvider->getB(i);
		double c = dataProvider->getC(i);
		SigmaSt2[i] =  a*a+b+c; 
		SigmaSt[i] = (SigmaSt2[i]>=0.0) ? AQLMath::sqrt(SigmaSt2[i]) : 0.0;*/
		//modify antonov2
		double fxt = fxgrids[i];
		double fx0 = fxgrids[0];
		double beta = betagrids[i];
		//SigmaFX[i] = (fxt+(1.0-beta)/beta*fx0)/fxt* dataProvider->mVGrids[i];
		SigmaFX[i] = (1.+(1.-beta) / beta) * dataProvider->mVGrids[i];

		SigmaStSigmaFX[i] = SigmaFX[i]*(0.5*dataProvider->mbmat[i][dataProvider->mEpos] + SigmaFX[i]);
	}
	//modify antonov2
	//dataProvider->mSigmaGrids = SigmaSt;
	dataProvider->mSigmaFXGrids = SigmaFX;
	//dataProvider->mSigmaSt2Grids = SigmaSt2;
	dataProvider->mSigmaStSigmaFXGrids = SigmaStSigmaFX;
	//intergral set
	//dataProvider->mSigmaSt2Data.set(timegrids,SigmaSt2);
	//dataProvider->mSigmaSt2Data.setInterpolation(inter);
	dataProvider->mSigmaStSigmaFXData.set(timegrids,SigmaStSigmaFX);
	dataProvider->mSigmaStSigmaFXData.setInterpolation(inter);

	//mbSigmaFXGrids and mSmallFactorGrids
	DoubleVector tmpvec(size,0.0), tmpvec2(size,0.0);
	AQL1DDataSet tmpInt,tmpInt2;
	tmpInt.set(timegrids,tmpvec); tmpInt2.set(timegrids,tmpvec);
	tmpInt.setInterpolation(inter); tmpInt2.setInterpolation(inter);
	DoubleArray bSigmaFX(size,0.0),smallfactor(size,0.0);
	
	//modify antonov
	DoubleVector R(size,0.0), tmpvec3(size,0.0);
	AQL1DDataSet tmpInt3;
	tmpInt3.set(timegrids,tmpvec3);
	tmpInt3.setInterpolation(inter);
	DoubleVector rFF(size,0.0), tmpvec4(size,0.0);
	AQL1DDataSet tmpInt4;
	tmpInt4.set(timegrids,tmpvec4);
	tmpInt4.setInterpolation(inter);
	DoubleVector rZZ(size,0.0), tmpvec5(size,0.0);
	AQL1DDataSet tmpInt5;
	tmpInt5.set(timegrids,tmpvec5);
	tmpInt5.setInterpolation(inter);
	DoubleVector rr(size,0.0), tmpvec6(size,0.0);
	AQL1DDataSet tmpInt6;
	tmpInt6.set(timegrids,tmpvec6);
	tmpInt6.setInterpolation(inter);
	//modify antonov
	for(unsigned int i=1;i<size;i++)
	{
		for(unsigned int j=0;j<i;j++)
		{
			double s = timegrids[j];
			double s1 = timegrids[j+1];
			double vd = Pdvolmat[j][dataProvider->mEpos];
			double vf = Pfvolmat[j][dataProvider->mEpos];
			double vdt = Pdvolmat[j][i];
			double vft = Pfvolmat[j][i];
			//temporary modify for 3 factor
			tmpvec[j] = (dataProvider->mCovffx * vft - dataProvider->mCovdfx * vdt) * SigmaFX[j]; 
			//tmpvec[j] = 0.5 * bmat[j][i] * SigmaFX[j];
			tmpvec2[j] = vd*(vdt-vft*dataProvider->mCovdf) + vf*(vft-vdt*dataProvider->mCovdf);
			tmpInt.setParam(tmpvec);
			tmpInt2.setParam(tmpvec2);
			bSigmaFX[i] +=tmpInt.integral(s,s1);
			smallfactor[i] +=tmpInt2.integral(s,s1);

			//modify antonov
			//modify antonov4
			//tmpvec3[j] = - dataProvider->mCovffx * vft * SigmaFX[j] + 0.5 * dataProvider->mamat[j][i];
			tmpvec3[j] = //- dataProvider->mCovffx * (vf - vft) * SigmaFX[j] 
							dataProvider->mCovffx * (vf - vft) * SigmaFX[j]
						    + 0.5 * (vd - vdt) * (vd - vdt)
							- dataProvider->mCovdf * vd * (vf - vft)
							+ 0.5 * ( vf * vf - vft * vft);
			
			tmpInt3.setParam(tmpvec3);
			R[i] += tmpInt3.integral(s,s1);
	
			tmpvec4[j] = dataProvider->mamat[j][dataProvider->mEpos] + dataProvider->mbmat[j][dataProvider->mEpos] * SigmaFX[j] + SigmaFX[j] * SigmaFX[j];
			//tmpvec4[j] = dataProvider->mamat[j][i] + dataProvider->mbmat[j][i] * SigmaFX[j] + SigmaFX[j] * SigmaFX[j];
			tmpInt4.setParam(tmpvec4);
			rFF[i] += tmpInt4.integral(s,s1);

			tmpvec5[j] = (vd - vdt) * (vd - vdt) - 2 * dataProvider->mCovdf * (vd - vdt) * ( vf - vft)
					   + (vf - vft) * (vf - vft);
			tmpInt5.setParam(tmpvec5);
			rZZ[i] += tmpInt5.integral(s,s1);
			
			//modify antonov3
			// kikuchi
			double a1stdif = dataProvider->mamat[j][dataProvider->mEpos] - ( vd * vdt - dataProvider->mCovdf * vd * vft + vf * vft - dataProvider->mCovdf * vf * vdt);
			double b1stdif = dataProvider->mbmat[j][dataProvider->mEpos] - ( -2.0 * dataProvider->mCovdfx * vdt + 2.0 * dataProvider->mCovffx * vft);
			//double a1stdif = dataProvider->mamat[j][i] - ( vd * vdt - dataProvider->mCovdf * vd * vft + vf * vft - dataProvider->mCovdf * vf * vdt);
			//double b1stdif = dataProvider->mbmat[j][i] - ( -2.0 * dataProvider->mCovdfx * vdt + 2.0 * dataProvider->mCovffx * vft);
			tmpvec6[j] = - a1stdif - 0.5 * b1stdif * SigmaFX[j];
			//tmpvec6[j] = - dataProvider->mamat[j][i] - 0.5 * dataProvider->mbmat[j][i] * SigmaFX[j];
			tmpInt6.setParam(tmpvec6);
			rr[i] += tmpInt6.integral(s,s1);
			//modify antonov
		}
		//modify antonov
		if(rFF[i] != 0.0)
			rr[i] /= rFF[i];
		else
			rr[i] /= 1E-8;
		//modify antonov
	}
	dataProvider->mSmallFactorGrids = smallfactor;
	dataProvider->mbSigmaFXGrids = bSigmaFX;
	//modify antonov
	dataProvider->mR = R;
	dataProvider->mrFF = rFF;
	dataProvider->mrZZ = rZZ;
	dataProvider->mr = rr;
	//modify antonov


	//modify antonov2
	for(unsigned int i=0;i<size;i++)
	{
		double t = timegrids[i];
		double a = dataProvider->getA(i);
		double b = dataProvider->getB(i);
		double c = dataProvider->getC(i);
		SigmaSt2[i] =  a*a+b+c; 
		SigmaSt[i] = (SigmaSt2[i]>=0.0) ? AQLMath::sqrt(SigmaSt2[i]) : 0.0;
	}
	dataProvider->mSigmaGrids = SigmaSt;
	dataProvider->mSigmaSt2Grids = SigmaSt2;
	//intergral set
	dataProvider->mSigmaSt2Data.set(timegrids,SigmaSt2);
	dataProvider->mSigmaSt2Data.setInterpolation(inter);
	//modify antonov2




	

	//othersetup
	dataProvider->calcIntegral1();
	return dataProvider;
}

double						
AQLMathAntonovFXOption::getVTM(double T, const AQLDataValuation& att) const
{
	AQLMathAntonovFXOptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("AntonovFXOptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathAntonovFXOptionDataProvider&>(att.getDataProvider());

	if(dataProvider->mUpdateIntegral1flag)
		dataProvider->calcIntegral1();

	double x = getBetaTM(T,att);

	unsigned int pos =0;
	const DoubleVector& timegrids = dataProvider->mTimeGrids;
	AQLAlgorithm::locate(timegrids,T,timegrids.size(),pos);
	if(pos == timegrids.size())
	{
		--pos;
	}
	else
	{
		double diff = AQLMath::abs(timegrids[pos] - T); 
		if (diff > AQLMath::abs(T - timegrids[pos - 1]))
		{
			--pos;
		}
	}
	double y = dataProvider->mIntSigmaSt2Grids[pos];
	return x*x*y;
}

double
AQLMathAntonovFXOption::getBetaTM(double T, const AQLDataValuation& att) const
{
	AQLMathAntonovFXOptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("AntonovFXOptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathAntonovFXOptionDataProvider&>(att.getDataProvider());
	
	if(dataProvider->mUpdateIntegral1flag)
		dataProvider->calcIntegral1();

	unsigned int pos =0;
	const DoubleVector& timegrids = dataProvider->mTimeGrids;
	AQLAlgorithm::locate(timegrids,T,timegrids.size(),pos);
	if(pos == timegrids.size())
	{
		--pos;
	}
	else
	{
		double diff = AQLMath::abs(timegrids[pos] - T); 
		if (diff > AQLMath::abs(T - timegrids[pos - 1]))
		{
			--pos;
		}
	}
	double x = dataProvider->mIntSimgaSt2BetaStIntSigmaSt2[pos];
	double y = dataProvider->mIntSimgaSt2IntSigmaSt2[pos];
	if(0.0==y)
		y=1E-12;
	double ret = x/y;
	return ret;
	
}

double						
AQLMathAntonovFXOption::getTM(const AQLDataValuation& att) const
{
	AQLMathAntonovFXOptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("AntonovFXOptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathAntonovFXOptionDataProvider&>(att.getDataProvider());

	return dataProvider->mTm;
}

double						
AQLMathAntonovFXOption::getFxTM(const AQLDataValuation& att) const
{
	AQLMathAntonovFXOptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("AntonovFXOptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathAntonovFXOptionDataProvider&>(att.getDataProvider());

	return dataProvider->mFxTm;
}

double						
AQLMathAntonovFXOption::getPTM(const AQLDataValuation& att) const
{
	AQLMathAntonovFXOptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("AntonovFXOptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathAntonovFXOptionDataProvider&>(att.getDataProvider());

	return dataProvider->mPTm;
}

double
AQLMathAntonovFXOption::getVTM(const AQLDataValuation& att) const
{
	AQLMathAntonovFXOptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("AntonovFXOptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathAntonovFXOptionDataProvider&>(att.getDataProvider());

	return dataProvider->mVolTm;
}

double
AQLMathAntonovFXOption::getBetaTM(const AQLDataValuation& att) const
{
	AQLMathAntonovFXOptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("AntonovFXOptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathAntonovFXOptionDataProvider&>(att.getDataProvider());

	return dataProvider->mBetaTm;
}

DoubleVector	
AQLMathAntonovFXOption::getBetaResult(const AQLDataValuation& att) const 
{
	AQLMathAntonovFXOptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("AntonovFXOptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathAntonovFXOptionDataProvider&>(att.getDataProvider());

	return dataProvider->mBetaGrids;
}

DoubleVector	
AQLMathAntonovFXOption::getVolatilityResult(const AQLDataValuation& att) const 
{
	AQLMathAntonovFXOptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("AntonovFXOptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathAntonovFXOptionDataProvider&>(att.getDataProvider());

	return dataProvider->mVolGrids;
}

unsigned int				
AQLMathAntonovFXOption::getNextPos(const AQLDataValuation& att) const
{
	AQLMathAntonovFXOptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("AntonovFXOptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathAntonovFXOptionDataProvider&>(att.getDataProvider());

	return dataProvider->mEpos;
}

void			
AQLMathAntonovFXOption::setCalibParamFirst(AQLMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const
{
	unsigned int size = dataProvider->mTimeGrids.size();
	DoubleVector v(size),s(size);
	
	for(unsigned int i=dataProvider->mSpos; i<=dataProvider->mEpos;i++)
		dataProvider->mBetaGrids[i] = beta;
	
	for(unsigned int i=0;i<size;i++)
	{
		if(0.0==dataProvider->mBetaGrids[i])
			throw AQLCoreInvalidData("Beta 0.0 is not allowed",__FILE__,__LINE__);
		
		if(0.0==dataProvider->mFxGrids[i])
			throw AQLCoreInvalidData("Fx 0.0 is not allowed",__FILE__,__LINE__);
		
		v[i] = dataProvider->mBetaGrids[i] * dataProvider->mVolGrids[i];
		s[i] = (1.0 - dataProvider->mBetaGrids[i]) / dataProvider->mBetaGrids[i] * dataProvider->mFxGrids[i];
	}

	for(unsigned int i=dataProvider->mSpos;i<=dataProvider->mEpos;i++)
		v[i] = beta*vol;

	dataProvider->mVGrids = v;
	dataProvider->mSGrids = s;
	return;
}

void			
AQLMathAntonovFXOption::setCalibParam(AQLMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const
{
	for(unsigned int i=dataProvider->mSpos ;i<= dataProvider->mEpos;i++)
	{
		dataProvider->mVGrids[i] = vol*beta;
		dataProvider->mBetaGrids[i] = beta;
		dataProvider->mVolGrids[i] = vol;
	}
	return;
}

/*!
	@brief create new cache class
	@return cache class
*/
AQLDataProvider*
AQLMathAntonovFXOption::createNewDataProvider() const
{
	AQLMathAntonovFXOptionDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new AQLMathAntonovFXOptionDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}

AQLMathAntonovFXOption::AQLMathAntonovFXOptionDataProvider::AQLMathAntonovFXOptionDataProvider() 
:mSmileNum(0),mSpos(0),mEpos(0),mTm(0.0),mFxTm(0.0),mPTm(1.0), mBetaTm(0.0),mVolTm(0.0),
mFxGrids(DoubleVector()),mBetaGrids(DoubleVector()),mVolGrids(DoubleVector()),mTimeGrids(DoubleVector()),
mPdvol(DoubleMatrix()),mPfvol(DoubleMatrix()),mamat(DoubleMatrix()),mbmat(DoubleMatrix()),
mCovdf(0.0),mCovdfx(0.0),mCovffx(0.0),mIsCallVec(BoolVector()),mStrikeVec(DoubleVector()),mPremVec(DoubleVector()),
mWeightVec(DoubleVector()),mIsfitATM(true),mVGrids(DoubleVector()),mSGrids(DoubleVector()),mAGrids(DoubleVector()),
mBGrids(DoubleVector()),mCGrids(DoubleVector()),maGrids(DoubleVector()),mbGrids(DoubleVector()),
mSigmaGrids(DoubleVector()),mSigmaFXGrids(DoubleVector()),mSigmaSt2Grids(DoubleVector()),mSigmaSt2Data(),
mSigmaStSigmaFXGrids(DoubleVector()),mSigmaStSigmaFXData(),mIntSigmaSt2Grids(DoubleVector()),
mIntSigmaStSigmaFXGrids(DoubleVector()),mIntSimgaSt2BetaStIntSigmaSt2(DoubleVector()),mIntSimgaSt2IntSigmaSt2(DoubleVector()),
mSmallFactorGrids(DoubleVector()),mbSigmaFXGrids(DoubleVector()),mBetaSt(DoubleVector()),
mUpdateABCflag(true),mUpdateIntegral1flag(true)
//modify antonov
,mR(DoubleVector()),mrFF(DoubleVector()),mrZZ(DoubleVector()),mr(DoubleVector())
//modify antonov
{	
}

AQLMathAntonovFXOption::AQLMathAntonovFXOptionDataProvider::~AQLMathAntonovFXOptionDataProvider()
{
}

double 
AQLMathAntonovFXOption::AQLMathAntonovFXOptionDataProvider::getA(unsigned pos) const
{
	if(mUpdateABCflag)
		calcABC();
	return mAGrids[pos];
}

double 
AQLMathAntonovFXOption::AQLMathAntonovFXOptionDataProvider::getB(unsigned pos) const
{
	if(mUpdateABCflag)
		calcABC();
	return mBGrids[pos];
}

double 
AQLMathAntonovFXOption::AQLMathAntonovFXOptionDataProvider::getC(unsigned pos) const
{
	if(mUpdateABCflag)
		calcABC();
	return mCGrids[pos];
}

void 
AQLMathAntonovFXOption::AQLMathAntonovFXOptionDataProvider::calcABC(void) const
{
	unsigned int N = mTimeGrids.size();
	mAGrids.resize(N,0.0);
	mBGrids.resize(N,0.0);
	mCGrids.resize(N,0.0);

	for(unsigned int i=0;i<N;i++)
	{
		double v = mVGrids[i];
		double beta = mBetaGrids[i];
		double f = mFxGrids[i];
		double f0 = mFxGrids[0];
		double valA = (1.0-beta)/beta*v;
		double valb = mbmat[i][mEpos];
		double vala = mamat[i][mEpos];
	

		//modify antonov
		//mAGrids[i] = valA;
		//kikuchi
		mAGrids[i] = valA * AQLMath::exp(-mR[i]) * (1.0 + 0.5 * mrZZ[i] - 0.5 * mr[i] * mr[i] * mrFF[i]);
		//mAGrids[i] = valA * (1.0 + 0.5 * mrZZ[i] - 0.5 * mr[i] * mr[i] * mrFF[i]);
		//mAGrids[i] = valA * AQLMath::exp(-mR[i]);
		//mAGrids[i] = valA;
		//mAGrids[i] = valA * (1.0 + 0.5 * mrZZ[i] - 0.5 * mr[i] * mr[i] * mrFF[i]);
		//modify antonov
		
		//modify antonov4
		mBGrids[i] = mAGrids[i] * (2.0*v+valb);
		//mBGrids[i] = valA * (2.0*v+valb);
		
		mCGrids[i] = v*v + vala + v*valb;
	}
	mUpdateABCflag=false;
}

void 
AQLMathAntonovFXOption::AQLMathAntonovFXOptionDataProvider::calcIntegral1(void) const
{
	unsigned int N = mTimeGrids.size();
	mIntSigmaSt2Grids.resize(N);
	mIntSigmaStSigmaFXGrids.resize(N);
	mBetaSt.resize(N);

	AQL1DDataSet integral1,integral2;
	integral1.set(mTimeGrids,mSigmaSt2Grids);
	integral2.set(mTimeGrids,mSigmaStSigmaFXGrids);
	AQLStepInterpolation inter;
	integral1.setInterpolation(inter);
	integral2.setInterpolation(inter);
	
	DoubleVector SigmaSt2IntSigmaSt2(N,0.0),SigmaSt2BetaStIntSigmaSt2(N,0.0);
	for(unsigned int i=1;i<N;i++)
	{
		double val=0.0;
		mIntSigmaSt2Grids[i] =  mIntSigmaSt2Grids[i-1] + integral1.integral(mTimeGrids[i-1],mTimeGrids[i]);
		mIntSigmaStSigmaFXGrids[i] =  mIntSigmaStSigmaFXGrids[i-1] + integral2.integral(mTimeGrids[i-1],mTimeGrids[i]);
		
		//double add = mbSigmaFXGrids[i] + mSmallFactorGrids[i];
		//mBetaSt[i] = 1.0+ (-getA(i) * getA(i) - 0.5 * getB(i) )*( mIntSigmaStSigmaFXGrids[i] + add)/ mIntSigmaSt2Grids[i] / mSigmaSt2Grids[i];
		//modify antonov
		//kikuchi
		double tmpval = 1.0 + 0.5 * mrZZ[i] - 0.5 * mr[i] * mr[i] * mrFF[i];
		//double tmpval = 1.0;
		double secondexpansion = 1.0 + mr[i]/ tmpval;
		mBetaSt[i] = 1.0+ (-getA(i) * getA(i) - 0.5 * getB(i) )* secondexpansion / mSigmaSt2Grids[i];
		//modify antonov

		SigmaSt2IntSigmaSt2[i] =  mSigmaSt2Grids[i] * mIntSigmaSt2Grids[i];
		SigmaSt2BetaStIntSigmaSt2[i] = mSigmaSt2Grids[i] * mBetaSt[i] * mIntSigmaSt2Grids[i];
	}

	mIntSimgaSt2BetaStIntSigmaSt2.resize(N,0.0);
	mIntSimgaSt2IntSigmaSt2.resize(N,0.0);

	AQL1DDataSet integral3,integral4;
	integral3.set(mTimeGrids,SigmaSt2BetaStIntSigmaSt2);
	integral4.set(mTimeGrids,SigmaSt2IntSigmaSt2);
	integral3.setInterpolation(inter);
	integral4.setInterpolation(inter);

	for(unsigned int i=1;i<N;i++)
	{
		mIntSimgaSt2BetaStIntSigmaSt2[i] = mIntSimgaSt2BetaStIntSigmaSt2[i-1] + integral3.integral(mTimeGrids[i-1],mTimeGrids[i]);
		mIntSimgaSt2IntSigmaSt2[i] = mIntSimgaSt2IntSigmaSt2[i-1] + integral4.integral(mTimeGrids[i-1],mTimeGrids[i]);
	}
	mUpdateIntegral1flag = false;
}

void 
AQLMathAntonovFXOption::AQLMathAntonovFXOptionDataProvider::reset(double varV, double VarB,unsigned int spos,unsigned int epos) const
{
	unsigned int size = mTimeGrids.size();
	for(unsigned int i=spos;i<=epos;i++)
	{
		double t = mTimeGrids[i];
		//modify antonov2
		/*double a = getA(i);
		double b = getB(i);
		double c = getC(i);
		mSigmaSt2Grids[i] =  a*a+b+c; 
		mSigmaGrids[i] = (mSigmaSt2Grids[i]>=0.0) ? AQLMath::sqrt(mSigmaSt2Grids[i]) : 0.0;*/
		//modify antonov2
		double fxt = mFxGrids[i];
		double fx0 = mFxGrids[0];
		double beta = mBetaGrids[i];
		//mSigmaFXGrids[i] = (fxt+(1.0-beta)/beta*fx0)/fxt*mVGrids[i];
		mSigmaFXGrids[i] = (1.+(1.-beta)/beta) * mVGrids[i];
		mSigmaStSigmaFXGrids[i] = mSigmaFXGrids[i]*(0.5*mbmat[i][mEpos] + mSigmaFXGrids[i]);
	}
	//intergral set
	//modify antonov2
	//mSigmaSt2Data.set(mTimeGrids,mSigmaSt2Grids);
	mSigmaStSigmaFXData.set(mTimeGrids,mSigmaStSigmaFXGrids);
	//mbSigmaFXGrids and mSmallFactorGrids
	AQLStepInterpolation inter;
	DoubleVector tmpvec(size,0.0), tmpvec2(size,0.0);
	AQL1DDataSet tmpInt,tmpInt2;
	tmpInt.set(mTimeGrids,tmpvec); tmpInt2.set(mTimeGrids,tmpvec);
	tmpInt.setInterpolation(inter); tmpInt2.setInterpolation(inter);
	DoubleArray bSigmaFX(size,0.0),smallfactor(size,0.0);


	//modify antonov
	DoubleVector tmpvec3(size,0.0);
	AQL1DDataSet tmpInt3;
	tmpInt3.set(mTimeGrids,tmpvec3);
	tmpInt3.setInterpolation(inter);
	DoubleVector tmpvec4(size,0.0);
	AQL1DDataSet tmpInt4;
	tmpInt4.set(mTimeGrids,tmpvec4);
	tmpInt4.setInterpolation(inter);
	DoubleVector tmpvec5(size,0.0);
	AQL1DDataSet tmpInt5;
	tmpInt5.set(mTimeGrids,tmpvec5);
	tmpInt5.setInterpolation(inter);
	DoubleVector tmpvec6(size,0.0);
	AQL1DDataSet tmpInt6;
	tmpInt6.set(mTimeGrids,tmpvec6);
	tmpInt6.setInterpolation(inter);
	//modify antonov
	for(unsigned int i=spos;i<=epos;i++)
	{
		mbSigmaFXGrids[i] = 0.0;
		mSmallFactorGrids[i] = 0.0;
		//modify antonov
		mR[i] = 0.0;
		mrFF[i] = 0.0;
		mrZZ[i] = 0.0;
		mr[i] = 0.0;
		//modify antonov
		for(unsigned int j=0;j<i;j++)
		{
			double s = mTimeGrids[j];
			double s1 = mTimeGrids[j+1];
			double vd = mPdvol[j][mEpos];
			double vf = mPfvol[j][mEpos];
			double vdt = mPdvol[j][i];
			double vft = mPfvol[j][i];
			//temporary modify for 3 factor
			tmpvec[j] = (mCovffx * vft - mCovdfx * vdt) * mSigmaFXGrids[j]; 
			//tmpvec[j] = 0.5 * (mbmat[j][i] -vft')* mSigmaFXGrids[j];
			//tmpvec2[j] = vd*(vdt-vft*mCovdf) + vf*(vft-vft*mCovdf);
			tmpvec2[j] = vd*(vdt-vft*mCovdf) + vf*(vft-vdt*mCovdf);
			tmpInt.setParam(tmpvec);
			tmpInt2.setParam(tmpvec2);
			
			mbSigmaFXGrids[i] +=tmpInt.integral(s,s1);
			mSmallFactorGrids[i] +=tmpInt2.integral(s,s1);

			//modify antonov
			//modify antonov4
			//tmpvec3[j] = - mCovffx * vft * mSigmaFXGrids[j] + 0.5 * mamat[j][i];
			tmpvec3[j] = //- mCovffx * (vf - vft) * mSigmaFXGrids[j]
		 				 mCovffx * (vf - vft) * mSigmaFXGrids[j]
						 + 0.5 * (vd - vdt) * (vd - vdt)
						 - mCovdf * vd * (vf - vft)
						 + 0.5 * ( vf * vf - vft * vft);

			tmpInt3.setParam(tmpvec3);
			mR[i] += tmpInt3.integral(s,s1);

			//tmpvec4[j] = mamat[j][i] + mbmat[j][i] * mSigmaFXGrids[j] + mSigmaFXGrids[j] * mSigmaFXGrids[j];
			tmpvec4[j] = mamat[j][mEpos] + mbmat[j][mEpos] * mSigmaFXGrids[j] + mSigmaFXGrids[j] * mSigmaFXGrids[j];
			tmpInt4.setParam(tmpvec4);
			mrFF[i] += tmpInt4.integral(s,s1);

			tmpvec5[j] = (vd - vdt) * (vd - vdt)  - 2 * mCovdf * (vd - vdt) * ( vf - vft)
						 + (vf - vft) * (vf - vft);
			tmpInt5.setParam(tmpvec5);
			mrZZ[i] += tmpInt5.integral(s,s1);

			//modify antonov3
			double a1stdif = mamat[j][mEpos] - ( vd * vdt - mCovdf * vd * vft + vf * vft - mCovdf * vf * vdt);
			double b1stdif = mbmat[j][mEpos] - ( -2.0 * mCovdfx * vdt + 2.0 * mCovffx * vft);
			//double a1stdif = mamat[j][i] - ( vd * vdt - mCovdf * vd * vft + vf * vft - mCovdf * vf * vdt);
			//double b1stdif = mbmat[j][i] - ( -2.0 * mCovdfx * vdt + 2.0 * mCovffx * vft);
			tmpvec6[j] = - a1stdif - 0.5 * b1stdif * mSigmaFXGrids[j];
			//tmpvec6[j] = - mamat[j][i] - 0.5 * mbmat[j][i] * mSigmaFXGrids[j];
			tmpInt6.setParam(tmpvec6);
			mr[i] += tmpInt6.integral(s,s1);
			//modify antonov
		} 
			//modify antonov
		if(mrFF[i] != 0.0)
			mr[i] /= mrFF[i];
		else
			mr[i] /= 1E-8;
		//modify antonov
	}
	//other set up

	//modify antonov2
	for(unsigned int i=spos;i<=epos;i++)
	{
		//modify antonov2
		double a = getA(i);
		double b = getB(i);
		double c = getC(i);
		mSigmaSt2Grids[i] =  a*a+b+c; 
		mSigmaGrids[i] = (mSigmaSt2Grids[i]>=0.0) ? AQLMath::sqrt(mSigmaSt2Grids[i]) : 0.0;
	}
	//intergral set
	mSigmaSt2Data.set(mTimeGrids,mSigmaSt2Grids);
	//modify antonov2
	calcIntegral1();
}
