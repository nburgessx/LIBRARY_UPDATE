// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef LAMathLeastSquareSwaptionVolQlib_h
#define LAMathLeastSquareSwaptionVolQlib_h

#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/array.hpp>

#include "LAMathVolatilityLMMDiscModel.h"

class LAMathSwaptionVolLMMDiscModel;

class LAMathLeastSquareSwaptionVolQlib : public QuantLib::CostFunction
{
public:

    enum mode { Diff, Ratio, Sqrt_Ratio };
	enum target { Premium, Volatility };

    // constructor
    explicit LAMathLeastSquareSwaptionVolQlib(const DoubleMatrix& Swaption_target_value_Mkt_,
											mode mode_ = Diff,
											target target_ = Volatility
//                                    const vector< vector< pair<double, double> > >& OptMat_and_Exspire_
			);

    // constructor
    explicit LAMathLeastSquareSwaptionVolQlib(const DoubleMatrix& Swaption_target_value_Mkt_,
											const DoubleMatrix& weight_,
											mode mode_ = Diff,
											target target_ = Volatility
//                                    const vector< vector< pair<double, double> > >& OptMat_and_Exspire_
			);

    // Copy constructor
    LAMathLeastSquareSwaptionVolQlib( const LAMathLeastSquareSwaptionVolQlib& rhs );

    // Destructor
    virtual ~LAMathLeastSquareSwaptionVolQlib();

    virtual LAMathLeastSquareSwaptionVolQlib* clone() const = 0;

    // compute value of the least square function
    virtual double value(const QuantLib::Array& x) const = 0;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const = 0;

    //virtual void gradient(Array& grad, const Array& x) const { grad = x; }

    //
    virtual LAMathLeastSquareSwaptionVolQlib& operator =(const LAMathLeastSquareSwaptionVolQlib& rhs);

    // Return target number to fit
    size_t getNumTarget2Fit() { return mNumTarget; }

    //
    void setMode(string mode__);


protected:

    // target swaption vol to fit
    DoubleMatrix mSwaptionTargetValue;

    size_t mn;
    size_t mm;

    // number of target to fit
    vector< vector<bool> > mUseThisTarget;
    size_t mNumTarget;

    // factors to fit
    mutable DoubleMatrix mfct2fit;

    // weight
    DoubleMatrix mWeight;

    //
    mode mMode_;
	
    //
    target mTarget_;


private:

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const = 0;
};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareSwaptionVolDiscModelQlib : public LAMathLeastSquareSwaptionVolQlib
{
public:
    // Default constructor
    LAMathLeastSquareSwaptionVolDiscModelQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											 vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											 mode mode_ = Diff,
											 target target_ = Volatility
			);


    LAMathLeastSquareSwaptionVolDiscModelQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											 vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											 const DoubleMatrix& weight_,
											 mode mode_ = Diff,
											 target target_ = Volatility
			);

    // Copy constructor
    LAMathLeastSquareSwaptionVolDiscModelQlib( const LAMathLeastSquareSwaptionVolDiscModelQlib& rhs );

    // Destructor
    virtual ~LAMathLeastSquareSwaptionVolDiscModelQlib();

    virtual LAMathLeastSquareSwaptionVolDiscModelQlib* clone() const { return new LAMathLeastSquareSwaptionVolDiscModelQlib(*this); }

    // compute value of the least square function
    virtual double value(const QuantLib::Array& x) const;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const;

    //virtual void gradient(Array& grad, const Array& x) const { grad = x; }

    //
    virtual LAMathLeastSquareSwaptionVolDiscModelQlib& operator =(const LAMathLeastSquareSwaptionVolDiscModelQlib& rhs);


protected:

    // volatility parameters
    mutable DoubleVector mParamV;
    mutable DoubleVector mParamF;
    mutable DoubleVector mG;

    //
    mutable vector< LAMathSwaptionVolLMMDiscModel* > mSwaptionVol;


private:

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const;

    bool mCloned;

    //
    mutable bool mGetGSize;
};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareSwaptionVolDiscModelVQlib : public LAMathLeastSquareSwaptionVolDiscModelQlib
{
public:
    // Default constructor
    LAMathLeastSquareSwaptionVolDiscModelVQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											  mode mode_ = Diff,
											  target target_ = Volatility
			);

    LAMathLeastSquareSwaptionVolDiscModelVQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											  const DoubleMatrix& weight_,
											  mode mode_ = Diff,
											  target target_ = Volatility
			);


    // Copy constructor
    LAMathLeastSquareSwaptionVolDiscModelVQlib( const LAMathLeastSquareSwaptionVolDiscModelVQlib& rhs );

    // Destructor
    virtual ~LAMathLeastSquareSwaptionVolDiscModelVQlib();

    virtual LAMathLeastSquareSwaptionVolDiscModelVQlib* clone() const { return new LAMathLeastSquareSwaptionVolDiscModelVQlib(*this); }

    //
    LAMathLeastSquareSwaptionVolDiscModelVQlib& operator =(const LAMathLeastSquareSwaptionVolDiscModelVQlib& rhs);

protected:
private:

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const;

};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareSwaptionVolDiscModelFQlib : public LAMathLeastSquareSwaptionVolDiscModelQlib
{
public:

    // Default constructor
    LAMathLeastSquareSwaptionVolDiscModelFQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											  double T_max_,
											  mode mode_ = Diff,
											  target target_ = Volatility
			);

    // Default constructor
    LAMathLeastSquareSwaptionVolDiscModelFQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											  double T_max_,
											  const DoubleMatrix& weight_,
											  mode mode_ = Diff,
											  target target_ = Volatility
			);


    // Copy constructor
    LAMathLeastSquareSwaptionVolDiscModelFQlib( const LAMathLeastSquareSwaptionVolDiscModelFQlib& rhs );

    // Destructor
    virtual ~LAMathLeastSquareSwaptionVolDiscModelFQlib();

    virtual LAMathLeastSquareSwaptionVolDiscModelFQlib* clone() const { return new LAMathLeastSquareSwaptionVolDiscModelFQlib(*this); }

    //
    LAMathLeastSquareSwaptionVolDiscModelFQlib& operator =(const LAMathLeastSquareSwaptionVolDiscModelFQlib& rhs);

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

class LAMathLeastSquareSwaptionVolDiscModelGQlib : public LAMathLeastSquareSwaptionVolDiscModelQlib
{
public:
    // Default constructor
    LAMathLeastSquareSwaptionVolDiscModelGQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											  mode mode_ = Diff,
											  target target_ = Volatility
			);

    // Default constructor
    LAMathLeastSquareSwaptionVolDiscModelGQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											  const DoubleMatrix& weight_,
											  mode mode_ = Diff,
											  target target_ = Volatility
			);

    // Copy constructor
    LAMathLeastSquareSwaptionVolDiscModelGQlib( const LAMathLeastSquareSwaptionVolDiscModelGQlib& rhs );

    // Destructor
    virtual ~LAMathLeastSquareSwaptionVolDiscModelGQlib();

    virtual LAMathLeastSquareSwaptionVolDiscModelGQlib* clone() const { return new LAMathLeastSquareSwaptionVolDiscModelGQlib(*this); }

    //
    LAMathLeastSquareSwaptionVolDiscModelGQlib& operator =(const LAMathLeastSquareSwaptionVolDiscModelGQlib& rhs);

protected:
private:

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const;

};

#endif
