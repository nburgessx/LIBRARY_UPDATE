// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef LAMathLeastSquareSwaptionVol_h
#define LAMathLeastSquareSwaptionVol_h

#include "LAQuantLibCostFunction.h"
#include "LAMathLeastSquareSwaptionVolQlib.h"

#include "LAMathVolatilityLMMDiscModel.h"

class LAMathSwaptionVolLMMDiscModel;

class LAMathLeastSquareSwaptionVol : public LAQuantLibCostFunction
{
public:

    enum mode { Diff, Ratio, Sqrt_Ratio };
	enum target { Premium, Volatility };

    // constructor
    explicit LAMathLeastSquareSwaptionVol( const DoubleMatrix& Swaption_target_value_Mkt_,
										 mode mode_ = Diff,
										 target target_ = Volatility
//                                    const vector< vector< pair<double, double> > >& OptMat_and_Exspire_
			);

    // constructor
    explicit LAMathLeastSquareSwaptionVol( const DoubleMatrix& Swaption_target_value_Mkt_,
										 const DoubleMatrix& weight_,
										 mode mode_ = Diff,
										 target target_ = Volatility
//                                    const vector< vector< pair<double, double> > >& OptMat_and_Exspire_
			);

    // constructor
	explicit LAMathLeastSquareSwaptionVol(bool isNew_);

    // Copy constructor
    LAMathLeastSquareSwaptionVol( const LAMathLeastSquareSwaptionVol& rhs );

    // Destructor
    virtual ~LAMathLeastSquareSwaptionVol();

};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareSwaptionVolDiscModel : public LAMathLeastSquareSwaptionVol
{
public:
    // Default constructor
    LAMathLeastSquareSwaptionVolDiscModel( const DoubleMatrix& Swaption_target_value_Mkt_,
										 vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										 mode mode_ = Diff,
										 target target_ = Volatility
			);


    LAMathLeastSquareSwaptionVolDiscModel( const DoubleMatrix& Swaption_target_value_Mkt_,
										 vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										 const DoubleMatrix& weight_,
										 mode mode_ = Diff,
										 target target_ = Volatility
			);

    // constructor
	LAMathLeastSquareSwaptionVolDiscModel(bool isNew_);

    // Copy constructor
    LAMathLeastSquareSwaptionVolDiscModel( const LAMathLeastSquareSwaptionVolDiscModel& rhs );

    // Destructor
    virtual ~LAMathLeastSquareSwaptionVolDiscModel();

    virtual LAMathLeastSquareSwaptionVolDiscModel* clone() const { return new LAMathLeastSquareSwaptionVolDiscModel(*this); }

    //
    virtual LAMathLeastSquareSwaptionVolDiscModel& operator =(const LAMathLeastSquareSwaptionVolDiscModel& rhs);
};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareSwaptionVolDiscModelV : public LAMathLeastSquareSwaptionVolDiscModel
{
public:
    // Default constructor
    LAMathLeastSquareSwaptionVolDiscModelV( const DoubleMatrix& Swaption_target_value_Mkt_,
										  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										  mode mode_ = Diff,
										  target target_ = Volatility
			);

    LAMathLeastSquareSwaptionVolDiscModelV( const DoubleMatrix& Swaption_target_value_Mkt_,
										  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										  const DoubleMatrix& weight_,
										  mode mode_ = Diff,
										  target target_ = Volatility
			);


    // Copy constructor
    LAMathLeastSquareSwaptionVolDiscModelV( const LAMathLeastSquareSwaptionVolDiscModelV& rhs );

    // Destructor
    virtual ~LAMathLeastSquareSwaptionVolDiscModelV();

    virtual LAMathLeastSquareSwaptionVolDiscModelV* clone() const { return new LAMathLeastSquareSwaptionVolDiscModelV(*this); }

    //
    LAMathLeastSquareSwaptionVolDiscModelV& operator =(const LAMathLeastSquareSwaptionVolDiscModelV& rhs);

};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareSwaptionVolDiscModelF : public LAMathLeastSquareSwaptionVolDiscModel
{
public:

    // Default constructor
    LAMathLeastSquareSwaptionVolDiscModelF( const DoubleMatrix& Swaption_target_value_Mkt_,
										  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										  double T_max_,
										  mode mode_ = Diff,
										  target target_ = Volatility
			);

    // Default constructor
    LAMathLeastSquareSwaptionVolDiscModelF( const DoubleMatrix& Swaption_target_value_Mkt_,
										  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										  double T_max_,
										  const DoubleMatrix& weight_,
										  mode mode_ = Diff,
										  target target_ = Volatility
			);


    // Copy constructor
    LAMathLeastSquareSwaptionVolDiscModelF( const LAMathLeastSquareSwaptionVolDiscModelF& rhs );

    // Destructor
    virtual ~LAMathLeastSquareSwaptionVolDiscModelF();

    virtual LAMathLeastSquareSwaptionVolDiscModelF* clone() const { return new LAMathLeastSquareSwaptionVolDiscModelF(*this); }

    //
    LAMathLeastSquareSwaptionVolDiscModelF& operator =(const LAMathLeastSquareSwaptionVolDiscModelF& rhs);

};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareSwaptionVolDiscModelG : public LAMathLeastSquareSwaptionVolDiscModel
{
public:
    // Default constructor
    LAMathLeastSquareSwaptionVolDiscModelG( const DoubleMatrix& Swaption_vol_Mkt_,
										  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										  mode mode_ = Diff,
										  target target_ = Volatility
			);

    // Default constructor
    LAMathLeastSquareSwaptionVolDiscModelG( const DoubleMatrix& Swaption_target_value_Mkt_,
										  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										  const DoubleMatrix& weight_,
										  mode mode_ = Diff,
										  target target_ = Volatility
			);

    // Copy constructor
    LAMathLeastSquareSwaptionVolDiscModelG( const LAMathLeastSquareSwaptionVolDiscModelG& rhs );

    // Destructor
    virtual ~LAMathLeastSquareSwaptionVolDiscModelG();

    virtual LAMathLeastSquareSwaptionVolDiscModelG* clone() const { return new LAMathLeastSquareSwaptionVolDiscModelG(*this); }

    //
    LAMathLeastSquareSwaptionVolDiscModelG& operator =(const LAMathLeastSquareSwaptionVolDiscModelG& rhs);

};

#endif
