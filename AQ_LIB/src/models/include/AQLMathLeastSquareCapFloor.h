// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef AQLMathLeastSquareCapFloor_h
#define AQLMathLeastSquareCapFloor_h

//
#include <vector>

#include "AQLQuantLibCostFunction.h"
#include "AQLMathLeastSquareCapFloorQlib.h"

using namespace std;


class AQLMathCapFloor;
class AQLMathCapFloorVolLMMDiscModel;

class AQLMathLeastSquareCapFloor : public AQLQuantLibCostFunction
{
public:

    enum mode { Diff, Ratio, Sqrt_Ratio };
	enum target { Premium, Volatility };

    // Default constructor
	AQLMathLeastSquareCapFloor( const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
							 bool vega_weighted = false,
							 mode mode__ = Diff,
							 target target_ = Volatility
			);

	// constructor
	AQLMathLeastSquareCapFloor(bool isNew_ = true);

    // Copy constructor
    AQLMathLeastSquareCapFloor( const AQLMathLeastSquareCapFloor& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareCapFloor();

    virtual AQLMathLeastSquareCapFloor* clone() const = 0;
};

//
//--------------------------------------------------------------------------------
//

class AQLMathLeastSquareCapFloorVolDiscModel : public AQLMathLeastSquareCapFloor
{
public:

    // Default constructor
	AQLMathLeastSquareCapFloorVolDiscModel( const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
										 vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
										 bool vega_weighted = false,
										 mode mode__ = Diff,
										 target target_ = Volatility
			);

	// constructor
	AQLMathLeastSquareCapFloorVolDiscModel(bool isNew_ = true);

    // Copy constructor
    AQLMathLeastSquareCapFloorVolDiscModel( const AQLMathLeastSquareCapFloorVolDiscModel& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareCapFloorVolDiscModel();

    //
    virtual AQLMathLeastSquareCapFloorVolDiscModel* clone() const { return new AQLMathLeastSquareCapFloorVolDiscModel(*this); }

    //
    virtual AQLMathLeastSquareCapFloorVolDiscModel& operator =(const AQLMathLeastSquareCapFloorVolDiscModel& rhs);
};

//
//--------------------------------------------------------------------------------
//

class AQLMathLeastSquareCapFloorVolDiscModelV : public AQLMathLeastSquareCapFloorVolDiscModel
{
public:
    // Default constructor
    AQLMathLeastSquareCapFloorVolDiscModelV( const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
										  vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
										  bool vega_weighted = false,
										  mode mode__ = Diff,
										  target target_ = Volatility
			);

    // Copy constructor
    AQLMathLeastSquareCapFloorVolDiscModelV( const AQLMathLeastSquareCapFloorVolDiscModelV& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareCapFloorVolDiscModelV();

    //
    virtual AQLMathLeastSquareCapFloorVolDiscModelV* clone() const { return new AQLMathLeastSquareCapFloorVolDiscModelV(*this); }

    //
    AQLMathLeastSquareCapFloorVolDiscModelV& operator =(const AQLMathLeastSquareCapFloorVolDiscModelV& rhs);

};

//
//--------------------------------------------------------------------------------
//

class AQLMathLeastSquareCapFloorVolDiscModelF : public AQLMathLeastSquareCapFloorVolDiscModel
{
public:
    // Default constructor
    AQLMathLeastSquareCapFloorVolDiscModelF( const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
										  vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
										  double T_max_,
										  bool vega_weighted = false,
										  mode mode__ = Diff,
										  target target_ = Volatility
			);


    // Copy constructor
    AQLMathLeastSquareCapFloorVolDiscModelF( const AQLMathLeastSquareCapFloorVolDiscModelF& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareCapFloorVolDiscModelF();

    virtual AQLMathLeastSquareCapFloorVolDiscModelF* clone() const { return new AQLMathLeastSquareCapFloorVolDiscModelF(*this); }

    //
    AQLMathLeastSquareCapFloorVolDiscModelF& operator =(const AQLMathLeastSquareCapFloorVolDiscModelF& rhs);

};

//
//--------------------------------------------------------------------------------
//

class AQLMathLeastSquareCapFloorVolDiscModelG : public AQLMathLeastSquareCapFloorVolDiscModel
{
public:
    // Default constructor
    AQLMathLeastSquareCapFloorVolDiscModelG( const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
										  vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
										  bool vega_weighted = false,
										  mode mode__ = Diff,
										  target target_ = Volatility
			);

    // Copy constructor
    AQLMathLeastSquareCapFloorVolDiscModelG( const AQLMathLeastSquareCapFloorVolDiscModelG& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareCapFloorVolDiscModelG();

    //
    virtual AQLMathLeastSquareCapFloorVolDiscModelG* Clone() const { return new AQLMathLeastSquareCapFloorVolDiscModelG(*this); }

    //
    AQLMathLeastSquareCapFloorVolDiscModelG& operator =(const AQLMathLeastSquareCapFloorVolDiscModelG& rhs);

};

#endif
