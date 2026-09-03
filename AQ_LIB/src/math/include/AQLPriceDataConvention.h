#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceDataType.h"
#include "AQLDate.h"
#include "AQLPriceDataDayCount.h"

// AQLPriceDataConvention Data ID
#define DATA_CONVENTION 42

// RateConvention
enum RateConvention
{
	CONT,  // Continuous Compound
    MNT,   // Monthly Compound
    QTR,   // Quarterly Compound
    SA,    // Semi-annual Compound
    ANN,   // Annual Compounded
    SIMPL  // Simple Interest Rate 
};


/*! 
    @brief Data class to represent the data (type rate convention).
*/
class AQLPriceDataConvention : public AQLPriceDataType
{
public:
//	LIFECYCLE
	// default constructor
	AQLPriceDataConvention(void);
	// copy constructor
    AQLPriceDataConvention(const AQLPriceDataConvention& attr);
	// constructor
	AQLPriceDataConvention(const DayCount dc, const RateConvention rc);
	// destructor
    virtual ~AQLPriceDataConvention(void);

//  QUERY
    // deep copy of the data object
    virtual AQLPriceDataType*    clone() const; 

    // return (interest rate calculation method) in the string representation
    virtual AQLString        convertToString(void) const;

    // get terms in years between fromDate and toDate
	double	                getTerm(const AQLDate& fromDate,	const AQLDate& toDate) const;

    // get terms in years (number of days per period) from the start date
	double	                getTerm(const AQLDate& fromDate,	const double& termD) const;

    // get terms in days (number of years per period) from the start date
	double	                getDayTerm(const AQLDate& fromDate,	const double& termY) const;
    
	// return discount factor based on the period and interest rate (annual interest)
	double					getDF(const double& rate, 
									const AQLDate& fromDate,
									const AQLDate& toDate) const;
    // return discount factor based on the period and interest rate (annual interest)
	double					getDF(const double& rate, const AQLDate& fromDate,
											const double& term) const;												

    // return yield to maturity based on the period and interest rate (annual interest)
	double					getCompound(const double& rate, 
									const AQLDate& fromDate,
									const AQLDate& toDate) const;
    // return yield to maturity based on the period and interest rate (annual interest)
	double					getCompound(const double& rate,
									const AQLDate& fromDate, 
									const double& term) const;

    // return interest rate (annual interest) based on the period and discount factor
	double					getRate(const double& df, const AQLDate& fromDate,
									const AQLDate& toDate) const;
    // return interest rate (annual interest) based on the period and discount factor
	double					getRate(const double& df, 
									const AQLDate& fromDate,
									const double& term) const;

    // convert interest rate
	double					convertTo(const double& rate, 
									const AQLDate& fromDate,
									const AQLDate& toDate,
									const AQLPriceDataConvention& cv) const;

    // get rate convention that the class holds
    /* 
        @return rate convention that the class holds
    */
	RateConvention          getConvention(void) const { return mRateConvention; }

    // get day count convention that the class holds
    /* 
        @return day count convention that the class holds
    */
    DayCount                getDayCount(void) const { return mDayCount.getDayCount(); }

    // calculate the rate of return from the period and interest rate (annual interest) based on the rate convention
	static double			rateToRet(const double& rate, 
								const double& term,
								const AQLPriceDataConvention& conv);

    // calculate interest rate (annual interest) from the period and the rate of return based on the rate convention
	static double			retToRate(const double& ret, 
								const double& term,
								const AQLPriceDataConvention& conv);


//  OPERATION
    // set the rate convention Data from the specified string
    virtual void            convertFromString(const AQLString& str);

    // set the interest calculation rules
    void                    setConvention(const RateConvention rc);
    // set the interest calculation rules
	void                    setConvention(const AQLString& rc);

    // set day count convention
	void                    setDayCount(const DayCount dc);
    // set day count convention
	void                    setDayCount(const AQLString& dc);

protected:
    // the contents of the object to be initialized with those of another object
	virtual AQLPriceDataType&	assignment(const AQLPriceDataType& a);

    // compare the contents against the other objecet
	virtual int				compare(const AQLPriceDataType& a) const;
private:
//	DayCount             mDayCount;       // day count convention
	RateConvention       mRateConvention; // rate convention
	AQLPriceDataDayCount		 mDayCount;		  // day count convention	
};

