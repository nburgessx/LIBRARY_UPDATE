// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef LAMathLeastSquareCapFloor_h
#define LAMathLeastSquareCapFloor_h

//
#include <vector>

#include "LAQuantLibCostFunction.h"
#include "LAMathLeastSquareCapFloorQlib.h"

using namespace std;


class LAMathCapFloor;
class LAMathCapFloorVolLMMDiscModel;

class LAMathLeastSquareCapFloor : public LAQuantLibCostFunction
{
public:

    enum mode { Diff, Ratio, Sqrt_Ratio };
	enum target { Premium, Volatility };

    // Default constructor
	LAMathLeastSquareCapFloor( const vector< LAMathCapFloor* >& CapFloor_Mkt_,
							 bool vega_weighted = false,
							 mode mode__ = Diff,
							 target target_ = Volatility
			);

	// constructor
	LAMathLeastSquareCapFloor(bool isNew_ = true);

    // Copy constructor
    LAMathLeastSquareCapFloor( const LAMathLeastSquareCapFloor& rhs );

    // Destructor
    virtual ~LAMathLeastSquareCapFloor();

    virtual LAMathLeastSquareCapFloor* clone() const = 0;
};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareCapFloorVolDiscModel : public LAMathLeastSquareCapFloor
{
public:

    // Default constructor
	LAMathLeastSquareCapFloorVolDiscModel( const vector< LAMathCapFloor* >& CapFloor_Mkt_,
										 vector< LAMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
										 bool vega_weighted = false,
										 mode mode__ = Diff,
										 target target_ = Volatility
			);

	// constructor
	LAMathLeastSquareCapFloorVolDiscModel(bool isNew_ = true);

    // Copy constructor
    LAMathLeastSquareCapFloorVolDiscModel( const LAMathLeastSquareCapFloorVolDiscModel& rhs );

    // Destructor
    virtual ~LAMathLeastSquareCapFloorVolDiscModel();

    //
    virtual LAMathLeastSquareCapFloorVolDiscModel* clone() const { return new LAMathLeastSquareCapFloorVolDiscModel(*this); }

    //
    virtual LAMathLeastSquareCapFloorVolDiscModel& operator =(const LAMathLeastSquareCapFloorVolDiscModel& rhs);
};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareCapFloorVolDiscModelV : public LAMathLeastSquareCapFloorVolDiscModel
{
public:
    // Default constructor
    LAMathLeastSquareCapFloorVolDiscModelV( const vector< LAMathCapFloor* >& CapFloor_Mkt_,
										  vector< LAMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
										  bool vega_weighted = false,
										  mode mode__ = Diff,
										  target target_ = Volatility
			);

    // Copy constructor
    LAMathLeastSquareCapFloorVolDiscModelV( const LAMathLeastSquareCapFloorVolDiscModelV& rhs );

    // Destructor
    virtual ~LAMathLeastSquareCapFloorVolDiscModelV();

    //
    virtual LAMathLeastSquareCapFloorVolDiscModelV* clone() const { return new LAMathLeastSquareCapFloorVolDiscModelV(*this); }

    //
    LAMathLeastSquareCapFloorVolDiscModelV& operator =(const LAMathLeastSquareCapFloorVolDiscModelV& rhs);

};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareCapFloorVolDiscModelF : public LAMathLeastSquareCapFloorVolDiscModel
{
public:
    // Default constructor
    LAMathLeastSquareCapFloorVolDiscModelF( const vector< LAMathCapFloor* >& CapFloor_Mkt_,
										  vector< LAMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
										  double T_max_,
										  bool vega_weighted = false,
										  mode mode__ = Diff,
										  target target_ = Volatility
			);


    // Copy constructor
    LAMathLeastSquareCapFloorVolDiscModelF( const LAMathLeastSquareCapFloorVolDiscModelF& rhs );

    // Destructor
    virtual ~LAMathLeastSquareCapFloorVolDiscModelF();

    virtual LAMathLeastSquareCapFloorVolDiscModelF* clone() const { return new LAMathLeastSquareCapFloorVolDiscModelF(*this); }

    //
    LAMathLeastSquareCapFloorVolDiscModelF& operator =(const LAMathLeastSquareCapFloorVolDiscModelF& rhs);

};

//
//--------------------------------------------------------------------------------
//

class LAMathLeastSquareCapFloorVolDiscModelG : public LAMathLeastSquareCapFloorVolDiscModel
{
public:
    // Default constructor
    LAMathLeastSquareCapFloorVolDiscModelG( const vector< LAMathCapFloor* >& CapFloor_Mkt_,
										  vector< LAMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
										  bool vega_weighted = false,
										  mode mode__ = Diff,
										  target target_ = Volatility
			);

    // Copy constructor
    LAMathLeastSquareCapFloorVolDiscModelG( const LAMathLeastSquareCapFloorVolDiscModelG& rhs );

    // Destructor
    virtual ~LAMathLeastSquareCapFloorVolDiscModelG();

    //
    virtual LAMathLeastSquareCapFloorVolDiscModelG* Clone() const { return new LAMathLeastSquareCapFloorVolDiscModelG(*this); }

    //
    LAMathLeastSquareCapFloorVolDiscModelG& operator =(const LAMathLeastSquareCapFloorVolDiscModelG& rhs);

};

#endif
