/*!
    @file LAMathSwaptionVolLMM.h
    @brief Declares Swaption_LMM class.
    @author
    @date 2008/10
    @version 1.0

     2008, AlgoQuantHub. All rights reserved.
*/

#ifndef LAMathSwaptionVolLMM_h
#define LAMathSwaptionVolLMM_h

//#include "LAFunctionBase.h"
#include <LACoreTemplateType.h>

#include <map>

class LAMathPathEntity;
class LAMathCorrelationLMM;
class LAMathVolatilityLMMDiscModel;

using namespace std;

//
//--------------------------------------------------------------------------------
//

class LAMathSwaptionVolLMM
{
public:

	enum volType { Normal, Black };

    // constructor
    LAMathSwaptionVolLMM();

    // constructor
	LAMathSwaptionVolLMM( LARatesPathElementCurve& curve0_F_,
						LARatesPathElementCurve& curve0_D_,
						const DoubleVector& T_fix_L_,
						const DoubleVector& T_pay_L_,
						const DoubleVector& tau_L,
						const DoubleVector& T_pay_,
						const DoubleVector& tau_,
						double T_OptMat_,
						double T_Expire_,
						volType volType_
					  );

	//	copy constructor
	LAMathSwaptionVolLMM(const LAMathSwaptionVolLMM& rhs);

    // Destructor
	virtual ~LAMathSwaptionVolLMM();

    // Make copy(clone) of this class
    virtual LAMathSwaptionVolLMM* clone() const = 0;
//
//------------------
//
    // Equal operator
	virtual LAMathSwaptionVolLMM& operator=(const LAMathSwaptionVolLMM& rhs);

    // Get Variance of SR(Maturity, Expire)
    virtual double getVar() = 0;

    // Get volatility of SR(Maturity, Expire)
    virtual double getSwaptionVol();

	// Get premium of SR(Maturity, Expire)
    virtual double getSwaptionPrem() = 0;

	// Get Black Premium
    double getSwaptionBlackPrem( double vol, double forwardShift ) const;

	// Get Normal Premium
    double getSwaptionNormalPrem( double vol ) const;

	double getS0() {return mS0;};

	double getAnnuity() {return mAnnuity;};

	double getT_OptMat() {return mT_OptMat;};
//
//------------------
//

protected :

    // Get Forward LIBOR
    double getForwardLIBOR( const LARatesPathElementCurve* curve, double T_fix, double T_pay, double tau ) const;

    // Forward Rate Value
    double getForwardRateValue( const LARatesPathElementCurve* curve, double T_fix, double T_pay ) const;

    // Get FRA
    double getFRA( double T_fix_L, double T_pay_L, double tau_L, double fix_rate ) const;
	    
    // Get Annuity
    double getAnnuity( const DoubleVector& T_pay, const DoubleVector& tau ) const;

    // Get Swap Float
    double getSwapFloat( const DoubleVector& T_fix_L, const DoubleVector& T_pay_L ) const;	

    // Get Swap Value
    double getSwapValue( const DoubleVector& T_fix_L, const DoubleVector& T_pay_L, const DoubleVector& T_pay, const DoubleVector& tau, double swap_rate ) const;

    // Get Swap Rate
    double getSwapRate( const DoubleVector& T_fix_L, const DoubleVector& T_pay_L, const DoubleVector& T_pay, const DoubleVector& tau ) const;

//
//------------------
//

    //
    LARatesPathElementCurve* mCurve0_F; //initial forecast curve
    LARatesPathElementCurve* mCurve0_D; //initial discount curve

    //
    DoubleVector mT_fix_L; //fixing time for Libor
    DoubleVector mT_pay_L; //payment time for Libor
    DoubleVector mTau_L; //tau for Libor
    DoubleVector mT_pay; //payment time for Swap
    DoubleVector mTau; //tau for Swap
	size_t mL2SNum; //# of Libor CF / # of Swap CF

	//
    double mT_OptMat; //option maturity
    size_t mItr_OptMat_L; //iterator of maturity for Libor
    double mT_Expire; //underlying expiration
    size_t mItr_Expire_L; //iterator of expiration for Libor 

    //cache
    DoubleVector mL_F;   //Libor rate from forecast curve
    DoubleVector mL_D; //Libor rate from discount curve
    DoubleVector mWeight; //weight for converting Libor into Swap
    double mAnnuity; //Swap annuity
    double mS0; //Swap rate

	volType mVolType;

private:

//
//------------------
//
    // Get integrated value of vol_i * vol_j 
	virtual double getIntegrateCov(size_t i, size_t j, double t_s, double t_e) = 0;


	bool mCloned;
};

//
//--------------------------------------------------------------------------------
//

class LAMathSwaptionVolLMMDiscModel : public LAMathSwaptionVolLMM
{
public:

    // constructor
    LAMathSwaptionVolLMMDiscModel();

    // constructor
	LAMathSwaptionVolLMMDiscModel(LARatesPathElementCurve& curve0_F_,
								LARatesPathElementCurve& curve0_D_,
								LAMathCorrelationLMM& corr_LMM_,
								LAMathVolatilityLMMDiscModel& vol_LMM_,
								const DoubleVector& T_fix_L_,
								const DoubleVector& T_pay_L_,
								const DoubleVector& tau_L,
								const DoubleVector& T_pay_,
								const DoubleVector& tau_,
								double T_OptMat,
								double T_Expire,
								volType volType_
							  );

	//	copy constructor
	LAMathSwaptionVolLMMDiscModel(const LAMathSwaptionVolLMMDiscModel& rhs);

    // Destructor
	virtual ~LAMathSwaptionVolLMMDiscModel();

    // Make copy(clone) of this class
    virtual LAMathSwaptionVolLMMDiscModel* clone() const;
//
//------------------
//
    // Equal operator
	virtual LAMathSwaptionVolLMMDiscModel& operator=(const LAMathSwaptionVolLMMDiscModel& rhs);

    // Get Variance of SR(Maturity, Expire)
    virtual double getVar();

	// Get premium of SR(Maturity, Expire)
    virtual double getSwaptionPrem();
//
//------------------
//
	// Change parameters of volatility function.
    void setParam(const DoubleVector& paramV,
				  const DoubleVector& paramF,
				  const DoubleVector& G
			);

    // Change parameters of volatility function.
    void setParamV(const DoubleVector& paramV);

    // Change parameters of volatility function.
    void setParamF(const DoubleVector& paramF);

    // Change parameters of volatility function.
    void setParamG(const DoubleVector& G);


//
//------------------
//

protected :
//
//------------------
//
    // Get integrated value of vol_i * vol_j 
    virtual double getIntegrateCov(size_t i, size_t j, double t_s, double t_e);

//
//------------------
//

private:    
    
    //
    void getCorrCache();

    //
    void getVolCache();

    //
    size_t mItrS; // integrate range of vol;
    size_t mItrE; // integrate range of vol;

//
//------------------
// covariance

    //
    LAMathCorrelationLMM* mCorrLMM;
    mutable DoubleMatrix mCorrCache;
    mutable bool mCorrCached;

    //
    LAMathVolatilityLMMDiscModel* mVolLMM;
    DoubleMatrix mVolCache;
    bool mVolCached;

    DoubleVector mT_Vol;
    vector<size_t> mT_VolItr;
    DoubleVector mt_Vol;
    mutable bool mT_VolCached;

	bool mCloned;

};

//
//--------------------------------------------------------------------------------
//

class LAMathSwaptionVolLMMDiscModel_DD : public LAMathSwaptionVolLMMDiscModel
{
public:

    // constructor
    LAMathSwaptionVolLMMDiscModel_DD();

    // constructor
	LAMathSwaptionVolLMMDiscModel_DD( LARatesPathElementCurve& curve0_F_,
								    LARatesPathElementCurve& curve0_D_,
									LAMathCorrelationLMM& corr_LMM_,
								    LAMathVolatilityLMMDiscModel& vol_LMM_,
									const DoubleVector& T_fix_L_,
									const DoubleVector& T_pay_L_,
									const DoubleVector& tau_L,
									const DoubleVector& T_pay_,
									const DoubleVector& tau_,
								    double T_OptMat,
								    double T_Expire,
									volType volType_,
									double Q = 1.
					  			  );

    // Make copy(clone) of this class
	virtual LAMathSwaptionVolLMMDiscModel_DD* clone() const { return new LAMathSwaptionVolLMMDiscModel_DD(*this); }
//
//------------------
//

    // Get Variance of SR(Maturity, Expire)
    virtual double getVar();

	// Get premium of SR(Maturity, Expire)
    virtual double getSwaptionPrem();

protected :
private:
	
	//
	double mQ;
};

class LAMathSwaptionPremLMMDiscModel_ShiftedDD : public LAMathSwaptionVolLMMDiscModel
{
public:

    // constructor
    LAMathSwaptionPremLMMDiscModel_ShiftedDD();

    // constructor
	LAMathSwaptionPremLMMDiscModel_ShiftedDD( LARatesPathElementCurve& curve0_F_,
									   LARatesPathElementCurve& curve0_D_,
									   LAMathCorrelationLMM& corr_LMM_,
									   LAMathVolatilityLMMDiscModel& vol_LMM_,
									   const DoubleVector& T_fix_L_,
									   const DoubleVector& T_pay_L_,
									   const DoubleVector& tau_L,
									   const DoubleVector& T_pay_,
									   const DoubleVector& tau_,
									   double T_OptMat,
									   double T_Expire,
									   volType volType_,
									   double Q,
									   double constShift,
									   double forwardShift
					  				 );

    // Make copy(clone) of this class
	virtual LAMathSwaptionPremLMMDiscModel_ShiftedDD* clone() const { return new LAMathSwaptionPremLMMDiscModel_ShiftedDD(*this); }
//
	// Get ShiftedDD Weight
    virtual void getShiftedDDWeight();
//------------------
//

    // Get Variance of SR(Maturity, Expire)
    virtual double getVar();

	// Get Variance of Sihfted SR(Maturity, Expire)
    virtual double getVar_ShiftedDD();

	// Get premium of SR(Maturity, Expire)
    virtual double getSwaptionPrem();

protected :
private:
	
	//
	double mQ;
	double mConstShift;
	double mForwardShift;

	DoubleArray mWeightCache_ShiftedDD;
};

#endif
