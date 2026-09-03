// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef AQLMathLeastSquareSwaptionVol_h
#define AQLMathLeastSquareSwaptionVol_h

#include "AQLQuantLibCostFunction.h"
#include "AQLMathLeastSquareSwaptionVolQlib.h"

#include "AQLMathVolatilityLMMDiscModel.h"

class AQLMathSwaptionVolLMMDiscModel;

class AQLMathLeastSquareSwaptionVol : public AQLQuantLibCostFunction
{
public:

    enum mode { Diff, Ratio, Sqrt_Ratio };
	enum target { Premium, Volatility };

    // constructor
    explicit AQLMathLeastSquareSwaptionVol( const DoubleMatrix& Swaption_target_value_Mkt_,
										 mode mode_ = Diff,
										 target target_ = Volatility
//                                    const vector< vector< pair<double, double> > >& OptMat_and_Exspire_
			);

    // constructor
    explicit AQLMathLeastSquareSwaptionVol( const DoubleMatrix& Swaption_target_value_Mkt_,
										 const DoubleMatrix& weight_,
										 mode mode_ = Diff,
										 target target_ = Volatility
//                                    const vector< vector< pair<double, double> > >& OptMat_and_Exspire_
			);

    // constructor
	explicit AQLMathLeastSquareSwaptionVol(bool isNew_);

    // Copy constructor
    AQLMathLeastSquareSwaptionVol( const AQLMathLeastSquareSwaptionVol& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareSwaptionVol();

};

//
//--------------------------------------------------------------------------------
//

class AQLMathLeastSquareSwaptionVolDiscModel : public AQLMathLeastSquareSwaptionVol
{
public:
    // Default constructor
    AQLMathLeastSquareSwaptionVolDiscModel( const DoubleMatrix& Swaption_target_value_Mkt_,
										 vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										 mode mode_ = Diff,
										 target target_ = Volatility
			);


    AQLMathLeastSquareSwaptionVolDiscModel( const DoubleMatrix& Swaption_target_value_Mkt_,
										 vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										 const DoubleMatrix& weight_,
										 mode mode_ = Diff,
										 target target_ = Volatility
			);

    // constructor
	AQLMathLeastSquareSwaptionVolDiscModel(bool isNew_);

    // Copy constructor
    AQLMathLeastSquareSwaptionVolDiscModel( const AQLMathLeastSquareSwaptionVolDiscModel& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareSwaptionVolDiscModel();

    virtual AQLMathLeastSquareSwaptionVolDiscModel* clone() const { return new AQLMathLeastSquareSwaptionVolDiscModel(*this); }

    //
    virtual AQLMathLeastSquareSwaptionVolDiscModel& operator =(const AQLMathLeastSquareSwaptionVolDiscModel& rhs);
};

//
//--------------------------------------------------------------------------------
//

class AQLMathLeastSquareSwaptionVolDiscModelV : public AQLMathLeastSquareSwaptionVolDiscModel
{
public:
    // Default constructor
    AQLMathLeastSquareSwaptionVolDiscModelV( const DoubleMatrix& Swaption_target_value_Mkt_,
										  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										  mode mode_ = Diff,
										  target target_ = Volatility
			);

    AQLMathLeastSquareSwaptionVolDiscModelV( const DoubleMatrix& Swaption_target_value_Mkt_,
										  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										  const DoubleMatrix& weight_,
										  mode mode_ = Diff,
										  target target_ = Volatility
			);


    // Copy constructor
    AQLMathLeastSquareSwaptionVolDiscModelV( const AQLMathLeastSquareSwaptionVolDiscModelV& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareSwaptionVolDiscModelV();

    virtual AQLMathLeastSquareSwaptionVolDiscModelV* clone() const { return new AQLMathLeastSquareSwaptionVolDiscModelV(*this); }

    //
    AQLMathLeastSquareSwaptionVolDiscModelV& operator =(const AQLMathLeastSquareSwaptionVolDiscModelV& rhs);

};

//
//--------------------------------------------------------------------------------
//

class AQLMathLeastSquareSwaptionVolDiscModelF : public AQLMathLeastSquareSwaptionVolDiscModel
{
public:

    // Default constructor
    AQLMathLeastSquareSwaptionVolDiscModelF( const DoubleMatrix& Swaption_target_value_Mkt_,
										  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										  double T_max_,
										  mode mode_ = Diff,
										  target target_ = Volatility
			);

    // Default constructor
    AQLMathLeastSquareSwaptionVolDiscModelF( const DoubleMatrix& Swaption_target_value_Mkt_,
										  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										  double T_max_,
										  const DoubleMatrix& weight_,
										  mode mode_ = Diff,
										  target target_ = Volatility
			);


    // Copy constructor
    AQLMathLeastSquareSwaptionVolDiscModelF( const AQLMathLeastSquareSwaptionVolDiscModelF& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareSwaptionVolDiscModelF();

    virtual AQLMathLeastSquareSwaptionVolDiscModelF* clone() const { return new AQLMathLeastSquareSwaptionVolDiscModelF(*this); }

    //
    AQLMathLeastSquareSwaptionVolDiscModelF& operator =(const AQLMathLeastSquareSwaptionVolDiscModelF& rhs);

};

//
//--------------------------------------------------------------------------------
//

class AQLMathLeastSquareSwaptionVolDiscModelG : public AQLMathLeastSquareSwaptionVolDiscModel
{
public:
    // Default constructor
    AQLMathLeastSquareSwaptionVolDiscModelG( const DoubleMatrix& Swaption_vol_Mkt_,
										  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										  mode mode_ = Diff,
										  target target_ = Volatility
			);

    // Default constructor
    AQLMathLeastSquareSwaptionVolDiscModelG( const DoubleMatrix& Swaption_target_value_Mkt_,
										  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
										  const DoubleMatrix& weight_,
										  mode mode_ = Diff,
										  target target_ = Volatility
			);

    // Copy constructor
    AQLMathLeastSquareSwaptionVolDiscModelG( const AQLMathLeastSquareSwaptionVolDiscModelG& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareSwaptionVolDiscModelG();

    virtual AQLMathLeastSquareSwaptionVolDiscModelG* clone() const { return new AQLMathLeastSquareSwaptionVolDiscModelG(*this); }

    //
    AQLMathLeastSquareSwaptionVolDiscModelG& operator =(const AQLMathLeastSquareSwaptionVolDiscModelG& rhs);

};

#endif
