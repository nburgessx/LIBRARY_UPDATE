// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef AQLMathLeastSquareCapFloorQlib_h
#define AQLMathLeastSquareCapFloorQlib_h

//
#include <vector>

#include "AQLQuantLibCostFunction.h"
#include "AQLCoreTemplateType.h"

using namespace std;


class AQLMathCapFloor;
class AQLMathCapFloorVolLMMDiscModel;

class AQLMathLeastSquareCapFloorQlib : public QuantLib::CostFunction
{
public:

    enum mode { Diff, Ratio, Sqrt_Ratio };
	enum target { Premium, Volatility };

    // Default constructor
	AQLMathLeastSquareCapFloorQlib( const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
								 bool vega_weighted = false,
								 mode mode__ = Diff,
								 target target_ = Volatility
			);

    // Copy constructor
    AQLMathLeastSquareCapFloorQlib( const AQLMathLeastSquareCapFloorQlib& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareCapFloorQlib();

    virtual AQLMathLeastSquareCapFloorQlib* clone() const = 0;

    // compute value of the least square function
    virtual double value(const QuantLib::Array& x) const = 0;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const = 0;

    //
    void setWeight( const DoubleVector& weight_);

    //
    virtual AQLMathLeastSquareCapFloorQlib& operator =(const AQLMathLeastSquareCapFloorQlib& rhs);

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

    mutable vector< AQLMathCapFloor* > mCapFloorMkt;

    // compute the target of the function to fit
    virtual void gettarget2fit() const;

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const = 0;

    bool mCloned;
};

//
//--------------------------------------------------------------------------------
//

class AQLMathLeastSquareCapFloorVolDiscModelQlib : public AQLMathLeastSquareCapFloorQlib
{
public:

    // Default constructor
	AQLMathLeastSquareCapFloorVolDiscModelQlib( const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
											 vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
											 bool vega_weighted = false,
											 mode mode__ = Diff,
											 target target_ = Volatility
			);

    // Copy constructor
    AQLMathLeastSquareCapFloorVolDiscModelQlib( const AQLMathLeastSquareCapFloorVolDiscModelQlib& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareCapFloorVolDiscModelQlib();

    //
    virtual AQLMathLeastSquareCapFloorVolDiscModelQlib* clone() const { return new AQLMathLeastSquareCapFloorVolDiscModelQlib(*this); }

    // compute value of the least square function
    virtual double value(const QuantLib::Array& x) const;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const;

    //
    virtual AQLMathLeastSquareCapFloorVolDiscModelQlib& operator =(const AQLMathLeastSquareCapFloorVolDiscModelQlib& rhs);

protected:

    // volatility parameters
    mutable DoubleVector mParamV;
    mutable DoubleVector mParamF;
    mutable DoubleVector mG;

    //
    mutable vector< AQLMathCapFloorVolLMMDiscModel* > mCapFloorLMM;

private:

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const;

    bool mCloned;
};

//
//--------------------------------------------------------------------------------
//

class AQLMathLeastSquareCapFloorVolDiscModelVQlib : public AQLMathLeastSquareCapFloorVolDiscModelQlib
{
public:
    // Default constructor
    AQLMathLeastSquareCapFloorVolDiscModelVQlib( const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
											  vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
											  bool vega_weighted = false,
											  mode mode__ = Diff,
											  target target_ = Volatility
			);

    // Copy constructor
    AQLMathLeastSquareCapFloorVolDiscModelVQlib( const AQLMathLeastSquareCapFloorVolDiscModelVQlib& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareCapFloorVolDiscModelVQlib();

    //
    virtual AQLMathLeastSquareCapFloorVolDiscModelVQlib* clone() const { return new AQLMathLeastSquareCapFloorVolDiscModelVQlib(*this); }

    //
    AQLMathLeastSquareCapFloorVolDiscModelVQlib& operator =(const AQLMathLeastSquareCapFloorVolDiscModelVQlib& rhs);

protected:
private:

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const;

};

//
//--------------------------------------------------------------------------------
//

class AQLMathLeastSquareCapFloorVolDiscModelFQlib : public AQLMathLeastSquareCapFloorVolDiscModelQlib
{
public:
    // Default constructor
    AQLMathLeastSquareCapFloorVolDiscModelFQlib( const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
											  vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
											  double T_max_,
											  bool vega_weighted = false,
											  mode mode__ = Diff,
											  target target_ = Volatility
			);


    // Copy constructor
    AQLMathLeastSquareCapFloorVolDiscModelFQlib( const AQLMathLeastSquareCapFloorVolDiscModelFQlib& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareCapFloorVolDiscModelFQlib();

    virtual AQLMathLeastSquareCapFloorVolDiscModelFQlib* clone() const { return new AQLMathLeastSquareCapFloorVolDiscModelFQlib(*this); }

    //
    AQLMathLeastSquareCapFloorVolDiscModelFQlib& operator =(const AQLMathLeastSquareCapFloorVolDiscModelFQlib& rhs);

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

class AQLMathLeastSquareCapFloorVolDiscModelGQlib : public AQLMathLeastSquareCapFloorVolDiscModelQlib
{
public:
    // Default constructor
    AQLMathLeastSquareCapFloorVolDiscModelGQlib( const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
											  vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
											  bool vega_weighted = false,
											  mode mode__ = Diff,
											  target target_ = Volatility
			);

    // Copy constructor
    AQLMathLeastSquareCapFloorVolDiscModelGQlib( const AQLMathLeastSquareCapFloorVolDiscModelGQlib& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareCapFloorVolDiscModelGQlib();

    //
    virtual AQLMathLeastSquareCapFloorVolDiscModelGQlib* clone() const { return new AQLMathLeastSquareCapFloorVolDiscModelGQlib(*this); }

    //
    AQLMathLeastSquareCapFloorVolDiscModelGQlib& operator =(const AQLMathLeastSquareCapFloorVolDiscModelGQlib& rhs);

    // Return Optimize dimention
    double getOptDim() { return mn; }

protected:
private:

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const;

};

#endif
