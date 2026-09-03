// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/
//  2012, AlgoQuantHub.

#ifndef AQLMathLeastSquareBlackDD_h
#define AQLMathLeastSquareBlackDD_h


#include "AQLQuantLibCostFunction.h"
#include "AQLQuantLibConstraint.h"
#include "AQLMathLeastSquareBlackDDQlib.h"

//===================== Class Declare AQLMathLeastSquareBlackDD==================================
/*! 

*/
class AQLMathLeastSquareBlackDD : public AQLQuantLibCostFunction
{
public:

    // Default constructor
    AQLMathLeastSquareBlackDD(double s0,
                        AQLRatesPathElementCurve& curve0_d,
                        AQLRatesPathElementCurve& curve0_f,
                        const DoubleArray& t,
                        const DoubleMatrix& strike,
                        const DoubleMatrix& prem,
                        const DoubleMatrix& weight,
                        const vector<IntArray>& callPutFlg,
                        bool isCalibVol = true,
                        bool isCalibSkew = true
                        );

    // Copy constructor
    AQLMathLeastSquareBlackDD( const AQLMathLeastSquareBlackDD& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareBlackDD();

    virtual AQLMathLeastSquareBlackDD* clone() const { return new AQLMathLeastSquareBlackDD(*this); }

	AQLMathLeastSquareBlackDD& operator =(const AQLMathLeastSquareBlackDD& rhs);

	virtual DoubleArray getBlackVol(const DoubleArray& x) const;
};


//===================== Class Declare AQLMathLeastSquareBlackDDVol==================================
/*! 

*/
class AQLMathLeastSquareBlackDDVol : public AQLQuantLibCostFunction
{
public:

    // Default constructor
    AQLMathLeastSquareBlackDDVol(double s0,
                            AQLRatesPathElementCurve& curve0_d,
                            AQLRatesPathElementCurve& curve0_f,
                            const DoubleArray& t,
                            const DoubleMatrix& strike,
                            const DoubleMatrix& prem,
                            const DoubleMatrix& weight,
                            const vector<IntVector>& callPutFlg
                           );

	
    // Copy constructor
    AQLMathLeastSquareBlackDDVol( const AQLMathLeastSquareBlackDDVol& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareBlackDDVol();

    //
    virtual AQLMathLeastSquareBlackDDVol* clone() const { return new AQLMathLeastSquareBlackDDVol(*this); }

    //
    AQLMathLeastSquareBlackDDVol& operator =(const AQLMathLeastSquareBlackDDVol& rhs);
};

//
//--------------------------------------------------------------------------------
//

//===================== Class Declare AQLMathLeastSquareBlackDDSkew==================================
/*! 

*/
class AQLMathLeastSquareBlackDDSkew : public AQLQuantLibCostFunction
{
public:

    // Default constructor
    AQLMathLeastSquareBlackDDSkew(double s0,
                            AQLRatesPathElementCurve& curve0_d,
                            AQLRatesPathElementCurve& curve0_f,
                            const DoubleArray& t,
                            const DoubleMatrix& strike,
                            const DoubleMatrix& prem,
                            const DoubleMatrix& weight,
							const DoubleArray& impVol,
                            const vector<IntVector>& callPutFlg
                           );

	
    // Copy constructor
    AQLMathLeastSquareBlackDDSkew( const AQLMathLeastSquareBlackDDSkew& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareBlackDDSkew();

    //
    virtual AQLMathLeastSquareBlackDDSkew* clone() const { return new AQLMathLeastSquareBlackDDSkew(*this); }

    //
    AQLMathLeastSquareBlackDDSkew& operator =(const AQLMathLeastSquareBlackDDSkew& rhs);
};

//===================== Class Declare AQLMathBoundaryConstraintMktSkewVol==================================
/*! 

*/
class AQLMathBoundaryConstraintMktSkewVol : public AQLQuantLibConstraint
{
public:

    // Default constructor
    AQLMathBoundaryConstraintMktSkewVol(double skewMax, 
									 double skewMin, 
									 double volMax, 
									 double volMin, 
									 size_t n = 2
									 );

	
    // Copy constructor
    //AQLMathBoundaryConstraintMktSkewVol( const AQLMathBoundaryConstraintMktSkewVol& rhs );

    // Destructor
    virtual ~AQLMathBoundaryConstraintMktSkewVol();

private:
    //
   // virtual AQLMathBoundaryConstraintMktSkewVol* clone() const { return new AQLMathBoundaryConstraintMktSkewVol(*this); }

    //
    AQLMathBoundaryConstraintMktSkewVol& operator =(const AQLMathBoundaryConstraintMktSkewVol& rhs);
};


#endif
