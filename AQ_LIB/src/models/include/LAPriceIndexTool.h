#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLFunctionBase.h"
#include "LAMathIndexEntity.h"
#include "LAPricePayOff.h"
#include "LAPricePayOffTool.h"
#include "LAPriceCFGenUtility.h"
#include "LACompoundingFunc.h"
#include <climits>
#include <cmath>


#define index_t int
#define INDEX_BASE 3000
#define INDEX_TOOL 3001
#define INDEX_TOOLFIXED 3002
#define INDEX_TOOLCPN 3003
#define INDEX_TOOLCPNCF 3004
#define INDEX_TOOLCOMPOUND 3005
#define INDEX_TOOLLIBORCOMPOUND 3006


class AQLObject;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of abstract base class of index calculation.
*/
class LAPriceIndexToolBase
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceIndexToolBase();
    // destructor	
	virtual ~LAPriceIndexToolBase();
	/*!
		@brief make copy(clone) of this class
	    @return deep copy of this class
	*/
	virtual LAPriceIndexToolBase*		clone() const = 0;
	/*!
		@brief calculate index
		@return index
	*/
	virtual bool                	isTypeOf(index_t id) const {return id == INDEX_BASE;}; 

	virtual double					calcIndex(void) const = 0;
	// set up this class
	virtual	void					setUp(const AQLDate& basedate,
										const AQLDate& paydate,
										const AQLObject& trade,
										const AQLObject& indexinfo,
										const LAPricePayOff& payoff);
	// set up this class
	virtual	void					setUp(const AQLDate& basedate,	
										const AQLObject& trade,
										const AQLObject& indexinfo,
										const AQLDate& fixingdate,
										const LAPricePayOff& payoff);
	/*!
		@brief round(or round_up or round_down) index
		@param[in, out] coupon index value
	*/
	void							round(double& index) const
									{									
										if (!mIsRound) return;
										index = LAPriceCFGenUtility::round(index, mRoundFunction, mRoundDigit);	
									}
	// get DF ratio
	double                          getDFRatio() const
									{
										return mDiscountRatio;
									}
	// check DF ratio update
	bool                            isDFRatioUpdate() const         
									{ 
										return mIsDFRatioUpdate; 
									}

protected:
	/*!
		@brief cap or floor index
		@param[in, out] coupon coupon index
	*/
	void							capfloor(double& index) const
									{
										if (mIsCap) 
										{	
											double cap = capValue();
											index = index > cap ? cap : index;
										}
										if (mIsFloor)
										{
											double floor = floorValue();
											index = index < floor ? floor : index;
										}
									}
	bool isSavePastFixing(const AQLObject& trade);
	void							discountadjust(double& index) const
									{
										index *= mDiscountRatio;
										return;
									}


//	const AQLFunctionBase*	mpCap;// cap
//	const AQLFunctionBase*	mpFloor;// floor	
	//cap floor
	bool mIsCap;// cap flag
	bool mIsFloor;// < floor flag
	double mDiscountRatio;
	bool mIsDFRatioUpdate;// < discount ratio upate flag
private:
	/*!
		@brief get cap value
		@return cap value
	*/
	virtual double					capValue(void) const = 0;
	/*!
		@brief get floor value
		@return floor value
	*/
	virtual double					floorValue(void) const = 0;


	//round
	bool mIsRound;// round flag
	RoundFunction	mRoundFunction;// round function
	int	mRoundDigit;// round digit

};



/*! 
    @brief declaration of tool class of index calculation.
*/

class LAPriceIndexTool : public LAPriceIndexToolBase 
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceIndexTool();
    // destructor	
	virtual ~LAPriceIndexTool();
	// make copy(clone) of this class
	virtual LAPriceIndexToolBase*		clone() const;// %%% COVARIANT RETURN %%%
	virtual bool                	isTypeOf(index_t id) const {return id == INDEX_TOOL ? true : LAPriceIndexToolBase::isTypeOf(id);};
	// calculate index
	virtual double					calcIndex(void) const
									{
										double index;
										if (mIndexPos.size() == 1 && mFixedRates.size() == 0) 
											index = mpIndex->getIndex()[mIndexPos[0]];
										else if (mIndexPos.size() == 0 && mFixedRates.size() == 1) 
											index = mFixedRates[0];
										else
										{
											for (unsigned int i = 0; i < mFixedRates.size(); i++)
												mX[i] = mFixedRates[i];
											for (unsigned int i = 0; i < mIndexPos.size(); i++)
												mX[i + mFixedRates.size()] = mpIndex->getIndex()[mIndexPos[i]];
											index = mpObservationOperator->operator ()(mX);
										}
										round(index);
										discountadjust(index);
										capfloor(index);
										return index;
									}
	// set up this class
	virtual	void					setUp(const AQLDate& basedate,
										const AQLDate& paydate,
										const AQLObject& trade,
										const AQLObject& indexinfo,
										const LAPricePayOff& payoff);

	// set up this class
	virtual	void					setUp(const AQLDate& basedate,	
										const AQLObject& trade,
										const AQLObject& indexinfo,
										const AQLDate& fixingdate,
										const LAPricePayOff& payoff);

	/*!
		@brief set index positions in intex array 
		@param[in, out] indexpos index positions
	*/	
	void							setIndexPos(const UintArray& indexpos) {mIndexPos = indexpos;}
	const DoubleArray &getIndexResult() const { return mX; }
	const LAMathIndexEntity* getIndexEntity(void) const {return mpIndex;}// index
	const UintArray& getIndexPos(void) const {return mIndexPos;}
	const DoubleArray& getFixedRates(void) const {return mFixedRates;}
protected:

	/*!
		@brief get cap value
		@return cap value
	*/
	virtual double					capValue(void) const {return mCap;}
	/*!
		@brief get floor value
		@return floor value
	*/
	virtual double					floorValue(void) const {return mFloor;} 

	double mCap;// cap value
	double mFloor;// floor value
	const LAMathIndexEntity* mpIndex;// index object
	UintArray mIndexPos;// < index positions
	const AQLFunctionBase* mpObservationOperator;// observation operator
	DoubleArray	mFixedRates;// fixed rates


	mutable DoubleArray	mX;// valiable for index calculation
private:

};


/*! 
    @brief declaration of tool class of index(fixed rate type).
*/
class LAPriceIndexToolFixed : public LAPriceIndexToolBase 
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceIndexToolFixed();
    // destructor	
	virtual ~LAPriceIndexToolFixed();
	// make copy(clone) of this class
	virtual LAPriceIndexToolBase*		clone() const;// %%% COVARIANT RETURN %%%
	virtual bool                	isTypeOf(index_t id) const {return id == INDEX_TOOLFIXED ? true : LAPriceIndexToolBase::isTypeOf(id);};
	/*!
		@brief calculate index
		@return index
	*/	
	virtual double					calcIndex(void) const 
									{
										double index = getFixedRate();
										discountadjust(index);
										return index;
									}

	/*!
		@brief get fixedrate
		@return fixedrate
	*/	
	virtual double					getFixedRate(void) const 
									{
										if (mRate)
											return mRate->get();
										else
											return mRates->get()[mPos];
									}

	/*!
		@brief get fixedrate
		@return fixedrate
	*/	
	virtual AQLDate*					getFixingDate(void) const 
									{
										return mpFixingDate;
									}


	// set up this class
	virtual	void					setUp(const AQLDate& basedate,
										const AQLDate& paydate,
										const AQLObject& trade,
										const AQLObject& indexinfo,
										const LAPricePayOff& payoff);
	// set up this class
	virtual	void					setUp(const AQLDate& basedate,	
										const AQLObject& trade,
										const AQLObject& indexinfo,
										const AQLDate& fixingdate,
										const LAPricePayOff& payoff);

protected:


private:
	/*!
		@brief get cap value
		@return cap value
	*/
	virtual double					capValue(void) const {return std::numeric_limits<double>::infinity();}
	/*!
		@brief get floor value
		@return floor value
	*/
	virtual double					floorValue(void) const {return -std::numeric_limits<double>::infinity();}
	//double mRate;// fixed rate
	const AQLDataDouble* mRate;
	const AQLDataDoubles* mRates;
	int mPos;
	AQLDate* mpFixingDate;// fixing date
};



/*! 
    @brief declaration of tool class of index(coupon type).
*/
class LAPriceIndexToolCpn : public LAPriceIndexToolBase 
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceIndexToolCpn();
    // destructor	
	virtual ~LAPriceIndexToolCpn();
	// copy constructor	
	LAPriceIndexToolCpn(const LAPriceIndexToolCpn& v);
	// make copy(clone) of this class
	virtual LAPriceIndexToolBase*		clone() const;// %%% COVARIANT RETURN %%%
	virtual bool                	isTypeOf(index_t id) const {return id == INDEX_TOOLCPN ? true : LAPriceIndexToolBase::isTypeOf(id);};
	/*!
		@brief calculate index
		@return index
	*/	
	virtual double					calcIndex(void) const
									{
										double index;
										const std::vector<PayOffToolHolderVector>&	
											vec = mPayOff->getPayOffMaster();
									/*	const std::vector<PayOffToolHolderVector>&	
											vec = mPayOff->getPayOff();*/
										if (mPayOffPos.size() == 1 && mFixedRates.size() == 0
											&& mpObservationOperator == NULL)
										{
											if (mIsActualCF)
											{
												index = vec[mLegNo][mPayOffPos[0]].getPayOff().getCoupon();
											}
											else
											{
												index = vec[mLegNo][mPayOffPos[0]].getPayOff().getCouponBeforeSelection(mCpnNo[mPayOffPos[0]]);
											}
										}
										else if  (mPayOffPos.size() == 0 && mFixedRates.size() == 1
												&& mpObservationOperator == NULL)
											index = mFixedRates[0];	
										else 
										{
											for (unsigned int i = 0; i < mFixedRates.size(); i++)
												mX[i] = mFixedRates[i];
											if (mIsActualCF)
											{
												for (unsigned int i = 0; i < mPayOffPos.size(); i++)
													mX[i + mFixedRates.size()] = vec[mLegNo][mPayOffPos[i]].getPayOff().getCoupon();
											}
											else
											{
												for (unsigned int i = 0; i < mPayOffPos.size(); i++)
												{
													mX[i + mFixedRates.size()] = vec[mLegNo][mPayOffPos[i]].getPayOff().getCouponBeforeSelection(mCpnNo[mPayOffPos[i]]);
												}
											}
											index = mpObservationOperator->operator ()(mX);
										}
										round(index);
										discountadjust(index);
										mCurrentIndex = index;
										capfloor(index);
										return index;									

									}

	// set up this class
	virtual	void					setUp(const AQLDate& basedate,
										const AQLDate& paydate,
										const AQLObject& trade,
										const AQLObject& indexinfo,
										const LAPricePayOff& payoff);

	// set up this class
	virtual	void					setUp(const AQLDate& basedate,	
										const AQLObject& trade,
										const AQLObject& indexinfo,
										const AQLDate& fixingdate,
										const LAPricePayOff& payoff);

protected:
	// set up past rate
	virtual void					setUpPastRate(const AQLObject& trade,
													unsigned int legNo,
													const AQLDate& fixingdate);
	// set up past rate
	virtual void					setUpPastRate(const AQLDate& basedate,	
													const AQLObject& trade,
													unsigned int legNo,
													const AQLDate& observationstartdate,
													const AQLDate& observationenddate);

	// set up cap and floor condition
	virtual	void					setUpCapandFloor(const AQLObject& trade,
													unsigned int legNo,
													const AQLDate& fixingdate,
													const LAPricePayOff& payoff);

	const LAPricePayOff* mPayOff;// payoff 
	unsigned int mLegNo;// leg number(firt leg number = 0)
	std::vector<unsigned int> mCpnNo;// leg number(firt coupon number = 0)
	bool mIsActualCF;// index calculated using mPayoff[mLegNo] or mPayOfff[mLegNo].mCoupons[mCpnInfo] 

	UintArray mPayOffPos;// past payoff positions that are used to calulate index
    const AQLFunctionBase* mpObservationOperator;// observation operator
	DoubleArray	mFixedRates;// fixed rates
	mutable	DoubleArray mX;// variable for index calculation

	AQLFunctionBase*	mpCap;// cap
	AQLFunctionBase*	mpFloor;// floor	
	int mLatestPos;// latest payoff position( most recently payoff position before fixingdate(include fixingdate)  
	DoubleArray mFixedRatesforCapFloor;// all fixed rates(used for calcuate cap and floor) 
	mutable double mCurrentIndex;// current index value before cap or floor
	mutable	DoubleArray mX2;// variable for cap or floor calculation

private:
	/*!
		@brief get cap value
		@return cap value
	*/
	virtual double					capValue(void) const
									{
										for (unsigned int i = 0; i < mFixedRatesforCapFloor.size(); i++)
											mX2[i] = mFixedRatesforCapFloor[i];
										if (mLatestPos >= 0)
										{
											const std::vector<PayOffToolHolderVector>&	
												vec = mPayOff->getPayOffMaster();
											/*const std::vector<PayOffToolHolderVector>&	
												vec = mPayOff->getPayOff();*/
											for (int i = 0; i <= mLatestPos; i++)
												mX2[i + mFixedRatesforCapFloor.size()] = vec[mLegNo][mPayOffPos[i]].getPayOff().getCoupon();
										}
										mX2.back() = mCurrentIndex;
										return mpCap->operator ()(mX2);
										
									}
	/*!
		@brief get floor value
		@return floor value
	*/
	virtual double					floorValue(void) const
									{
										if (mIsCap) return mpFloor->operator ()(mX2);
										for (unsigned int i = 0; i < mFixedRatesforCapFloor.size(); i++)
											mX2[i] = mFixedRatesforCapFloor[i];
										if (mLatestPos >= 0)
										{
											const std::vector<PayOffToolHolderVector>&	
												vec = mPayOff->getPayOffMaster();
											/*const std::vector<PayOffToolHolderVector>&	
												vec = mPayOff->getPayOff();*/
											for (int i = 0; i <= mLatestPos; i++)
												mX2[i + mFixedRatesforCapFloor.size()] = vec[mLegNo][mPayOffPos[i]].getPayOff().getCoupon();
										}
										mX2.back() = mCurrentIndex;
										return mpFloor->operator ()(mX2);
										
									}

};

/*! 
    @brief declaration of tool class of index(coupon payoff type).
*/
class LAPriceIndexToolCpnCF : public LAPriceIndexToolCpn 
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceIndexToolCpnCF();
    // destructor	
	virtual ~LAPriceIndexToolCpnCF();
	// Make copy(clone) of this class
	virtual LAPriceIndexToolBase*		clone() const;// %%% COVARIANT RETURN %%%
	virtual bool                	isTypeOf(index_t id) const {return id == INDEX_TOOLCPNCF ? true : LAPriceIndexToolCpn::isTypeOf(id);};
	// calculate index
	virtual double					calcIndex(void) const
									{
										double index;
										const std::vector<PayOffToolHolderVector>&	
											vec = mPayOff->getPayOffMaster();
										/*const std::vector<PayOffToolHolderVector>&	
											vec = mPayOff->getPayOff();*/
										if (mPayOffPos.size() == 1 && mFixedRates.size() == 0
												&& mpObservationOperator == NULL)
											index =  vec[mLegNo][mPayOffPos[0]].getPayOff().getCpnPayOffbyPayOffCur();
										else if  (mPayOffPos.size() == 0 && mFixedRates.size() == 1
												&& mpObservationOperator == NULL)
											index = mFixedRates[0];	
										else 
										{
											for (unsigned int i = 0; i < mFixedRates.size(); i++)
												mX[i] = mFixedRates[i];
											for (unsigned int i = 0; i < mPayOffPos.size(); i++)
												mX[i + mFixedRates.size()] = vec[mLegNo][mPayOffPos[i]].getPayOff().getCpnPayOffbyPayOffCur();
											index = mpObservationOperator->operator ()(mX);
										}
										round(index);
										discountadjust(index);
										mCurrentIndex = index;
										capfloor(index);
										return index;
									}

	// set up this class
	virtual	void					setUp(const AQLDate& basedate,
										const AQLDate& paydate,
										const AQLObject& trade,
										const AQLObject& indexinfo,
										const LAPricePayOff& payoff);

	// set up this class
	virtual	void					setUp(const AQLDate& basedate,	
										const AQLObject& trade,
										const AQLObject& indexinfo,
										const AQLDate& fixingdate,
										const LAPricePayOff& payoff);

protected:
	// set up past rate
	virtual void					setUpPastRate(const AQLObject& trade,
													unsigned int legNo,
													const AQLDate& fixingdate);
	// set up past rate
	virtual void					setUpPastRate(const AQLDate& basedate,	
													const AQLObject& trade,
													unsigned int legNo,
													const AQLDate& observationstartdate,
													const AQLDate& observationenddate);

	// set up cap and floor condition
	virtual	void					setUpCapandFloor(const AQLObject& trade,
													unsigned int legNo,
													const AQLDate& fixingdate,
													const LAPricePayOff& payoff);

	/*!
		@brief get cap value
		@return cap value
	*/
	virtual double					capValue(void) const
									{
										for (unsigned int i = 0; i < mFixedRatesforCapFloor.size(); i++)
											mX2[i] = mFixedRatesforCapFloor[i];
										if (mLatestPos >= 0)
										{
											const std::vector<PayOffToolHolderVector>&	
												vec = mPayOff->getPayOffMaster();
											/*const std::vector<PayOffToolHolderVector>&	
												vec = mPayOff->getPayOff();*/
											for (int i = 0; i <= mLatestPos; i++)
												mX2[i + mFixedRatesforCapFloor.size()] = vec[mLegNo][mPayOffPos[i]].getPayOff().getCpnPayOffbyPayOffCur();
										}
										mX2.back() = mCurrentIndex;
										return mpCap->operator ()(mX2);
										
									}
	/*!
		@brief get floor value
		@return floor value
	*/
	virtual double					floorValue(void) const
									{
										if (mIsCap) return mpFloor->operator ()(mX2);
										for (unsigned int i = 0; i < mFixedRatesforCapFloor.size(); i++)
											mX2[i] = mFixedRatesforCapFloor[i];
										if (mLatestPos >= 0)
										{
											const std::vector<PayOffToolHolderVector>&	
												vec = mPayOff->getPayOffMaster();
											/*const std::vector<PayOffToolHolderVector>&	
												vec = mPayOff->getPayOff();*/
											for (int i = 0; i <= mLatestPos; i++)
												mX2[i + mFixedRatesforCapFloor.size()] = vec[mLegNo][mPayOffPos[i]].getPayOff().getCpnPayOffbyPayOffCur();
										}
										mX2.back() = mCurrentIndex;
										return mpFloor->operator ()(mX2);
										
									}
private:


};





class LAPriceIndexToolCompound : public LAPriceIndexTool
{
public:
	LAPriceIndexToolCompound();
	virtual ~LAPriceIndexToolCompound();
	virtual LAPriceIndexToolBase* clone() const;// %%% COVARIANT RETURN %%%
	virtual bool isTypeOf(index_t id) const {return id == INDEX_TOOLLIBORCOMPOUND ? true : LAPriceIndexTool::isTypeOf(id);};



	virtual	void setUp(const AQLDate& basedate,
                       const AQLDate& paydate,
                       const AQLObject& trade,
                       const AQLObject& indexinfo,
                       const LAPricePayOff& payoff);
	virtual	void setUp(const AQLDate& basedate,	
                       const AQLObject& trade,
                       const AQLObject& indexinfo,
                       const AQLDate& fixingdate,
                       const LAPricePayOff& payoff);





	virtual void calcIndices(DoubleVector& indices, const size_t start_pos, const size_t end_pos) const;
    virtual void setFixingInfo(AQLDate& fixing_date, AQLString& fixing_flag) const;
    virtual void setFixingInfo(DateVector& fixing_date, AQLStringVector& fixing_flag) const;

protected:
	virtual void setUpIndexPosAndFixedRate(const AQLDate& basedate,
                                           const AQLObject& indexInfo,
                                           const AQLObject& trade);
    virtual void setUpFixingDates(const AQLObject& indexInfo);
    virtual void setUpStartAndEndDates(const AQLObject& indexInfo);


	bool flagForCompoundAllDays;
    mutable DoubleArray	mCompoundTerms;
    mutable DoubleArray	mMargins;
    AQLPriceDataDayCount mCompoundDC;
    DateVector cfCalcStartDates, cfCalcEndDates, fixingDates;
};
