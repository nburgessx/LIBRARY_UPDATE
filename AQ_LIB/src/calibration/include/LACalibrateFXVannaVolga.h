#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreTemplateType.h"
#include "LACalibrate.h"
#include "LAMathFXVolatilitySurfaceGenerate.h"
#include "LAPriceDataSlidingRule.h"
#include "LAFunctionBase.h"
#include "LABasic.h"

class LAObject;

class LACalibrateFXVannaVolga : public LACalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit LACalibrateFXVannaVolga(void);
	// destructor
	virtual ~LACalibrateFXVannaVolga(void);

	//=============================================
	//  setup
    virtual void	setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();
private :
	double mSpotRate;
	
	DoubleArray mDFRatios;
	LAString mFXCurrency;
	LAString mdYieldDataName;
	LAString mfYieldDataName;
	ATMInterpolationMethod mAtmMethod;
	mutable	std::vector<SmileData> mSmileData;
	mutable std::vector<FXOptionData > mFxParams;
	mutable DoubleVector mMatuTerms365;
	

	//hishida vannavolga
	class MMATM : public LAFunctionBase
	{
	public:
		// constructor
		MMATM(const FXOptionData& fxdata, const SmileData& smiledata)
		:mopdata(fxdata),msmiledata(smiledata){};
		// copy constructor
		MMATM(const MMATM &rhs)
		:mopdata(rhs.mopdata),msmiledata(rhs.msmiledata){} ;
		// Destructor
		virtual ~MMATM(){};
									//======================================
									// Return this class ID
		virtual function_t			getType() const{return 0;};
									//======================================
									// Make copy(clone) of this class
		virtual LACoreFunctionBase*		clone() const
		{
			return new MMATM(*this);
			/*try 
			{
				return new MMATM(*this);
			}
			catch (bad_alloc & e)
			{
				throw LACoreSystemError(e.what(), __FILE__, __LINE__);
			}*/
		
		};// %%% COVARIANT RETURN %%%

		virtual double				operator()(const DoubleArray& x) const 
		{
			if(x.size() == 1)
				return operator()(x[0]);
			throw LACoreInvalidData("parameter size must be one", __FILE__, __LINE__);
			
		};
		virtual double				operator()(const double& x) const 
		{
			double targetvol = msmiledata.vols[1];
			double k = msmiledata.strikes[1];

			double v1 = msmiledata.vols[0];
			double v2 = x;
			double v3 = msmiledata.vols[2];
			double k1 = msmiledata.strikes[0];
			double k2 = (mopdata.deltaType == FWD_PRE || mopdata.deltaType == SPOT_PRE)
						? mopdata.F * LAMath::exp(-0.5 * v2 * v2 * mopdata.T)
						: mopdata.F * LAMath::exp(0.5 * v2 * v2 * mopdata.T);
			double k3 = msmiledata.strikes[2];

			double targetimplyvol 
				= LAMath::log(k2 / k) * LAMath::log(k3 / k) / LAMath::log(k2 / k1) / LAMath::log(k3 / k1) * v1
				+ LAMath::log(k / k1) * LAMath::log(k3 / k) / LAMath::log(k2 / k1) / LAMath::log(k3 / k2) * v2
				+ LAMath::log(k / k1) * LAMath::log(k / k2) / LAMath::log(k3 / k1) / LAMath::log(k3 / k2) * v3;

			return /*(targetimplyvol-targetvol) * 10000*/targetimplyvol / targetvol - 1.0;
		};
		
		MMATM & operator=( const MMATM & ) { return *this; }
	private:
		FXOptionData mopdata;
		SmileData msmiledata;
		
	};
};

