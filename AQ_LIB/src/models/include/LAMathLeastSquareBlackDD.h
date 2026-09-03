// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/
//  2012, AlgoQuantHub.

#ifndef LAMathLeastSquareBlackDD_h
#define LAMathLeastSquareBlackDD_h


#include "LAQuantLibCostFunction.h"
#include "LAQuantLibConstraint.h"
#include "LAMathLeastSquareBlackDDQlib.h"

//===================== Class Declare LAMathLeastSquareBlackDD==================================
/*! 

*/
class LAMathLeastSquareBlackDD : public LAQuantLibCostFunction
{
public:

    // Default constructor
    LAMathLeastSquareBlackDD(double s0,
                        LARatesPathElementCurve& curve0_d,
                        LARatesPathElementCurve& curve0_f,
                        const DoubleArray& t,
                        const DoubleMatrix& strike,
                        const DoubleMatrix& prem,
                        const DoubleMatrix& weight,
                        const vector<IntArray>& callPutFlg,
                        bool isCalibVol = true,
                        bool isCalibSkew = true
                        );

    // Copy constructor
    LAMathLeastSquareBlackDD( const LAMathLeastSquareBlackDD& rhs );

    // Destructor
    virtual ~LAMathLeastSquareBlackDD();

    virtual LAMathLeastSquareBlackDD* clone() const { return new LAMathLeastSquareBlackDD(*this); }

	LAMathLeastSquareBlackDD& operator =(const LAMathLeastSquareBlackDD& rhs);

	virtual DoubleArray getBlackVol(const DoubleArray& x) const;
};


//===================== Class Declare LAMathLeastSquareBlackDDVol==================================
/*! 

*/
class LAMathLeastSquareBlackDDVol : public LAQuantLibCostFunction
{
public:

    // Default constructor
    LAMathLeastSquareBlackDDVol(double s0,
                            LARatesPathElementCurve& curve0_d,
                            LARatesPathElementCurve& curve0_f,
                            const DoubleArray& t,
                            const DoubleMatrix& strike,
                            const DoubleMatrix& prem,
                            const DoubleMatrix& weight,
                            const vector<IntVector>& callPutFlg
                           );

	
    // Copy constructor
    LAMathLeastSquareBlackDDVol( const LAMathLeastSquareBlackDDVol& rhs );

    // Destructor
    virtual ~LAMathLeastSquareBlackDDVol();

    //
    virtual LAMathLeastSquareBlackDDVol* clone() const { return new LAMathLeastSquareBlackDDVol(*this); }

    //
    LAMathLeastSquareBlackDDVol& operator =(const LAMathLeastSquareBlackDDVol& rhs);
};

//
//--------------------------------------------------------------------------------
//

//===================== Class Declare LAMathLeastSquareBlackDDSkew==================================
/*! 

*/
class LAMathLeastSquareBlackDDSkew : public LAQuantLibCostFunction
{
public:

    // Default constructor
    LAMathLeastSquareBlackDDSkew(double s0,
                            LARatesPathElementCurve& curve0_d,
                            LARatesPathElementCurve& curve0_f,
                            const DoubleArray& t,
                            const DoubleMatrix& strike,
                            const DoubleMatrix& prem,
                            const DoubleMatrix& weight,
							const DoubleArray& impVol,
                            const vector<IntVector>& callPutFlg
                           );

	
    // Copy constructor
    LAMathLeastSquareBlackDDSkew( const LAMathLeastSquareBlackDDSkew& rhs );

    // Destructor
    virtual ~LAMathLeastSquareBlackDDSkew();

    //
    virtual LAMathLeastSquareBlackDDSkew* clone() const { return new LAMathLeastSquareBlackDDSkew(*this); }

    //
    LAMathLeastSquareBlackDDSkew& operator =(const LAMathLeastSquareBlackDDSkew& rhs);
};

//===================== Class Declare LAMathBoundaryConstraintMktSkewVol==================================
/*! 

*/
class LAMathBoundaryConstraintMktSkewVol : public LAQuantLibConstraint
{
public:

    // Default constructor
    LAMathBoundaryConstraintMktSkewVol(double skewMax, 
									 double skewMin, 
									 double volMax, 
									 double volMin, 
									 size_t n = 2
									 );

	
    // Copy constructor
    //LAMathBoundaryConstraintMktSkewVol( const LAMathBoundaryConstraintMktSkewVol& rhs );

    // Destructor
    virtual ~LAMathBoundaryConstraintMktSkewVol();

private:
    //
   // virtual LAMathBoundaryConstraintMktSkewVol* clone() const { return new LAMathBoundaryConstraintMktSkewVol(*this); }

    //
    LAMathBoundaryConstraintMktSkewVol& operator =(const LAMathBoundaryConstraintMktSkewVol& rhs);
};


#endif
