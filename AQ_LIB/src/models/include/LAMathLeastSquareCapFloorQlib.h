// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef LAMathLeastSquareCapFloorQlib_h
#define LAMathLeastSquareCapFloorQlib_h

//
#include <vector>

#include "LAQuantLibCostFunction.h"
#include "LACoreTemplateType.h"

using namespace std;


class LAMathCapFloor;
class LAMathCapFloorVolLMMDiscModel;

class LAMathLeastSquareCapFloorQlib : public QuantLib::CostFunction
{
public:

    enum mode { Diff, Ratio, Sqrt_Ratio };
	enum target { Premium, Volatility };

    // Default constructor
	LAMathLeastSquareCapFloorQlib( const vector< LAMathCapFloor* >& CapFloor_Mkt_,
								 bool vega_weighted = false,
								 mode mode__ = Diff,
								 target target_ = Volatility
			);

    // Copy constructor
    LAMathLeastSquareCapFloorQlib( const LAMathLeastSquareCapFloorQlib& rhs );

    // Destructor
    virtual ~LAMathLeastSquareCapFloorQlib();

    virtual LAMathLeastSquareCapFloorQlib* clone() const = 0;

    // compute value of the least square function
    virtual double value(const QuantLib::Array& x) const = 0;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const = 0;

    //
    void setWeight( const DoubleVector& weight_);

    //
    virtual LAMathLeastSquareCapFloorQlib& operator =(const LAMathLeastSquareCapFloorQlib& rhs);

    // Return target number to fit
    size_t getNumTarget2Fit() { return mn; }

protected:

    //
    size_t mn;

    // target to fit
    mutable DoubleVector mtarget2fit;

    // factors to fit
    mutable DoubleVector mfct2fit;

    // weight
    DoubleVector mWeight;

    //
    mode mMode_;

	//
    target mTarget_;
private:

    mutable vector< LAMathCapFloor* > mCapFloorMkt;

    // compute the target of the function to fit
    virtual void gettarget2fit() const;

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const = 0;

    bool mCloned;
};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareCapFloorVolDiscModelQlib : public LAMathLeastSquareCapFloorQlib
{
public:

    // Default constructor
	LAMathLeastSquareCapFloorVolDiscModelQlib( const vector< LAMathCapFloor* >& CapFloor_Mkt_,
											 vector< LAMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
											 bool vega_weighted = false,
											 mode mode__ = Diff,
											 target target_ = Volatility
			);

    // Copy constructor
    LAMathLeastSquareCapFloorVolDiscModelQlib( const LAMathLeastSquareCapFloorVolDiscModelQlib& rhs );

    // Destructor
    virtual ~LAMathLeastSquareCapFloorVolDiscModelQlib();

    //
    virtual LAMathLeastSquareCapFloorVolDiscModelQlib* clone() const { return new LAMathLeastSquareCapFloorVolDiscModelQlib(*this); }

    // compute value of the least square function
    virtual double value(const QuantLib::Array& x) const;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const;

    //
    virtual LAMathLeastSquareCapFloorVolDiscModelQlib& operator =(const LAMathLeastSquareCapFloorVolDiscModelQlib& rhs);

protected:

    // volatility parameters
    mutable DoubleVector mParamV;
    mutable DoubleVector mParamF;
    mutable DoubleVector mG;

    //
    mutable vector< LAMathCapFloorVolLMMDiscModel* > mCapFloorLMM;

private:

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const;

    bool mCloned;
};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareCapFloorVolDiscModelVQlib : public LAMathLeastSquareCapFloorVolDiscModelQlib
{
public:
    // Default constructor
    LAMathLeastSquareCapFloorVolDiscModelVQlib( const vector< LAMathCapFloor* >& CapFloor_Mkt_,
											  vector< LAMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
											  bool vega_weighted = false,
											  mode mode__ = Diff,
											  target target_ = Volatility
			);

    // Copy constructor
    LAMathLeastSquareCapFloorVolDiscModelVQlib( const LAMathLeastSquareCapFloorVolDiscModelVQlib& rhs );

    // Destructor
    virtual ~LAMathLeastSquareCapFloorVolDiscModelVQlib();

    //
    virtual LAMathLeastSquareCapFloorVolDiscModelVQlib* clone() const { return new LAMathLeastSquareCapFloorVolDiscModelVQlib(*this); }

    //
    LAMathLeastSquareCapFloorVolDiscModelVQlib& operator =(const LAMathLeastSquareCapFloorVolDiscModelVQlib& rhs);

protected:
private:

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const;

};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareCapFloorVolDiscModelFQlib : public LAMathLeastSquareCapFloorVolDiscModelQlib
{
public:
    // Default constructor
    LAMathLeastSquareCapFloorVolDiscModelFQlib( const vector< LAMathCapFloor* >& CapFloor_Mkt_,
											  vector< LAMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
											  double T_max_,
											  bool vega_weighted = false,
											  mode mode__ = Diff,
											  target target_ = Volatility
			);


    // Copy constructor
    LAMathLeastSquareCapFloorVolDiscModelFQlib( const LAMathLeastSquareCapFloorVolDiscModelFQlib& rhs );

    // Destructor
    virtual ~LAMathLeastSquareCapFloorVolDiscModelFQlib();

    virtual LAMathLeastSquareCapFloorVolDiscModelFQlib* clone() const { return new LAMathLeastSquareCapFloorVolDiscModelFQlib(*this); }

    //
    LAMathLeastSquareCapFloorVolDiscModelFQlib& operator =(const LAMathLeastSquareCapFloorVolDiscModelFQlib& rhs);

protected:
private:

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const;

    //
    double mT_max;

};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareCapFloorVolDiscModelGQlib : public LAMathLeastSquareCapFloorVolDiscModelQlib
{
public:
    // Default constructor
    LAMathLeastSquareCapFloorVolDiscModelGQlib( const vector< LAMathCapFloor* >& CapFloor_Mkt_,
											  vector< LAMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
											  bool vega_weighted = false,
											  mode mode__ = Diff,
											  target target_ = Volatility
			);

    // Copy constructor
    LAMathLeastSquareCapFloorVolDiscModelGQlib( const LAMathLeastSquareCapFloorVolDiscModelGQlib& rhs );

    // Destructor
    virtual ~LAMathLeastSquareCapFloorVolDiscModelGQlib();

    //
    virtual LAMathLeastSquareCapFloorVolDiscModelGQlib* clone() const { return new LAMathLeastSquareCapFloorVolDiscModelGQlib(*this); }

    //
    LAMathLeastSquareCapFloorVolDiscModelGQlib& operator =(const LAMathLeastSquareCapFloorVolDiscModelGQlib& rhs);

    // Return Optimize dimention
    double getOptDim() { return mn; }

protected:
private:

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const;

};

#endif
