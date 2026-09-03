// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef AQLMathLeastSquareSwaptionVolQlib_h
#define AQLMathLeastSquareSwaptionVolQlib_h

#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/array.hpp>

#include "AQLMathVolatilityLMMDiscModel.h"

class AQLMathSwaptionVolLMMDiscModel;

class AQLMathLeastSquareSwaptionVolQlib : public QuantLib::CostFunction
{
public:

    enum mode { Diff, Ratio, Sqrt_Ratio };
	enum target { Premium, Volatility };

    // constructor
    explicit AQLMathLeastSquareSwaptionVolQlib(const DoubleMatrix& Swaption_target_value_Mkt_,
											mode mode_ = Diff,
											target target_ = Volatility
//                                    const vector< vector< pair<double, double> > >& OptMat_and_Exspire_
			);

    // constructor
    explicit AQLMathLeastSquareSwaptionVolQlib(const DoubleMatrix& Swaption_target_value_Mkt_,
											const DoubleMatrix& weight_,
											mode mode_ = Diff,
											target target_ = Volatility
//                                    const vector< vector< pair<double, double> > >& OptMat_and_Exspire_
			);

    // Copy constructor
    AQLMathLeastSquareSwaptionVolQlib( const AQLMathLeastSquareSwaptionVolQlib& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareSwaptionVolQlib();

    virtual AQLMathLeastSquareSwaptionVolQlib* clone() const = 0;

    // compute value of the least square function
    virtual double value(const QuantLib::Array& x) const = 0;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const = 0;

    //virtual void gradient(Array& grad, const Array& x) const { grad = x; }

    //
    virtual AQLMathLeastSquareSwaptionVolQlib& operator =(const AQLMathLeastSquareSwaptionVolQlib& rhs);

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

class AQLMathLeastSquareSwaptionVolDiscModelQlib : public AQLMathLeastSquareSwaptionVolQlib
{
public:
    // Default constructor
    AQLMathLeastSquareSwaptionVolDiscModelQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											 vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											 mode mode_ = Diff,
											 target target_ = Volatility
			);


    AQLMathLeastSquareSwaptionVolDiscModelQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											 vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											 const DoubleMatrix& weight_,
											 mode mode_ = Diff,
											 target target_ = Volatility
			);

    // Copy constructor
    AQLMathLeastSquareSwaptionVolDiscModelQlib( const AQLMathLeastSquareSwaptionVolDiscModelQlib& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareSwaptionVolDiscModelQlib();

    virtual AQLMathLeastSquareSwaptionVolDiscModelQlib* clone() const { return new AQLMathLeastSquareSwaptionVolDiscModelQlib(*this); }

    // compute value of the least square function
    virtual double value(const QuantLib::Array& x) const;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const;

    //virtual void gradient(Array& grad, const Array& x) const { grad = x; }

    //
    virtual AQLMathLeastSquareSwaptionVolDiscModelQlib& operator =(const AQLMathLeastSquareSwaptionVolDiscModelQlib& rhs);


protected:

    // volatility parameters
    mutable DoubleVector mParamV;
    mutable DoubleVector mParamF;
    mutable DoubleVector mG;

    //
    mutable vector< AQLMathSwaptionVolLMMDiscModel* > mSwaptionVol;


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

class AQLMathLeastSquareSwaptionVolDiscModelVQlib : public AQLMathLeastSquareSwaptionVolDiscModelQlib
{
public:
    // Default constructor
    AQLMathLeastSquareSwaptionVolDiscModelVQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											  mode mode_ = Diff,
											  target target_ = Volatility
			);

    AQLMathLeastSquareSwaptionVolDiscModelVQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											  const DoubleMatrix& weight_,
											  mode mode_ = Diff,
											  target target_ = Volatility
			);


    // Copy constructor
    AQLMathLeastSquareSwaptionVolDiscModelVQlib( const AQLMathLeastSquareSwaptionVolDiscModelVQlib& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareSwaptionVolDiscModelVQlib();

    virtual AQLMathLeastSquareSwaptionVolDiscModelVQlib* clone() const { return new AQLMathLeastSquareSwaptionVolDiscModelVQlib(*this); }

    //
    AQLMathLeastSquareSwaptionVolDiscModelVQlib& operator =(const AQLMathLeastSquareSwaptionVolDiscModelVQlib& rhs);

protected:
private:

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const;

};

//
//--------------------------------------------------------------------------------
//

class AQLMathLeastSquareSwaptionVolDiscModelFQlib : public AQLMathLeastSquareSwaptionVolDiscModelQlib
{
public:

    // Default constructor
    AQLMathLeastSquareSwaptionVolDiscModelFQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											  double T_max_,
											  mode mode_ = Diff,
											  target target_ = Volatility
			);

    // Default constructor
    AQLMathLeastSquareSwaptionVolDiscModelFQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											  double T_max_,
											  const DoubleMatrix& weight_,
											  mode mode_ = Diff,
											  target target_ = Volatility
			);


    // Copy constructor
    AQLMathLeastSquareSwaptionVolDiscModelFQlib( const AQLMathLeastSquareSwaptionVolDiscModelFQlib& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareSwaptionVolDiscModelFQlib();

    virtual AQLMathLeastSquareSwaptionVolDiscModelFQlib* clone() const { return new AQLMathLeastSquareSwaptionVolDiscModelFQlib(*this); }

    //
    AQLMathLeastSquareSwaptionVolDiscModelFQlib& operator =(const AQLMathLeastSquareSwaptionVolDiscModelFQlib& rhs);

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

class AQLMathLeastSquareSwaptionVolDiscModelGQlib : public AQLMathLeastSquareSwaptionVolDiscModelQlib
{
public:
    // Default constructor
    AQLMathLeastSquareSwaptionVolDiscModelGQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											  mode mode_ = Diff,
											  target target_ = Volatility
			);

    // Default constructor
    AQLMathLeastSquareSwaptionVolDiscModelGQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
											  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
											  const DoubleMatrix& weight_,
											  mode mode_ = Diff,
											  target target_ = Volatility
			);

    // Copy constructor
    AQLMathLeastSquareSwaptionVolDiscModelGQlib( const AQLMathLeastSquareSwaptionVolDiscModelGQlib& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareSwaptionVolDiscModelGQlib();

    virtual AQLMathLeastSquareSwaptionVolDiscModelGQlib* clone() const { return new AQLMathLeastSquareSwaptionVolDiscModelGQlib(*this); }

    //
    AQLMathLeastSquareSwaptionVolDiscModelGQlib& operator =(const AQLMathLeastSquareSwaptionVolDiscModelGQlib& rhs);

protected:
private:

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const;

};

#endif
