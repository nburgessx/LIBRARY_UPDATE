#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLCalibrate.h"
#include "AQLMathFXVolatilitySurfaceGenerate.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLFunctionBase.h"
#include "AQLBasic.h"

class AQLObject;

class AQLCalibrateFXVannaVolga : public AQLCalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLCalibrateFXVannaVolga(void);
	// destructor
	virtual ~AQLCalibrateFXVannaVolga(void);

	//=============================================
	//  setup
    virtual void	setUp(AQLObjectPool &objPool,  const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();
private :
	double mSpotRate;
	
	DoubleArray mDFRatios;
	AQLString mFXCurrency;
	AQLString mdYieldDataName;
	AQLString mfYieldDataName;
	ATMInterpolationMethod mAtmMethod;
	mutable	std::vector<SmileData> mSmileData;
	mutable std::vector<FXOptionData > mFxParams;
	mutable DoubleVector mMatuTerms365;
	

	//hishida vannavolga
	class AQLATM : public AQLFunctionBase
	{
	public:
		// constructor
		AQLATM(const FXOptionData& fxdata, const SmileData& smiledata)
		:mopdata(fxdata),msmiledata(smiledata){};
		// copy constructor
		AQLATM(const AQLATM &rhs)
		:mopdata(rhs.mopdata),msmiledata(rhs.msmiledata){} ;
		// Destructor
		virtual ~AQLATM(){};
									//======================================
									// Return this class ID
		virtual function_t			getType() const{return 0;};
									//======================================
									// Make copy(clone) of this class
		virtual AQLCoreFunctionBase*		clone() const
		{
			return new AQLATM(*this);
			/*try 
			{
				return new AQLATM(*this);
			}
			catch (bad_alloc & e)
			{
				throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
			}*/
		
		};// %%% COVARIANT RETURN %%%

		virtual double				operator()(const DoubleArray& x) const 
		{
			if(x.size() == 1)
				return operator()(x[0]);
			throw AQLCoreInvalidData("parameter size must be one", __FILE__, __LINE__);
			
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
						? mopdata.F * AQLMath::exp(-0.5 * v2 * v2 * mopdata.T)
						: mopdata.F * AQLMath::exp(0.5 * v2 * v2 * mopdata.T);
			double k3 = msmiledata.strikes[2];

			double targetimplyvol 
				= AQLMath::log(k2 / k) * AQLMath::log(k3 / k) / AQLMath::log(k2 / k1) / AQLMath::log(k3 / k1) * v1
				+ AQLMath::log(k / k1) * AQLMath::log(k3 / k) / AQLMath::log(k2 / k1) / AQLMath::log(k3 / k2) * v2
				+ AQLMath::log(k / k1) * AQLMath::log(k / k2) / AQLMath::log(k3 / k1) / AQLMath::log(k3 / k2) * v3;

			return /*(targetimplyvol-targetvol) * 10000*/targetimplyvol / targetvol - 1.0;
		};
		
		AQLATM & operator=( const AQLATM & ) { return *this; }
	private:
		FXOptionData mopdata;
		SmileData msmiledata;
		
	};
};

