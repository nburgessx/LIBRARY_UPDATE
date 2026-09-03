/*! @file
    @brief Class declaration to AQLMathAntonovFXOption model.

*/
//  2012, AlgoQuantHub.
#ifndef AQLContainer_h
#define AQLContainer_h

#include "AQLFunction.h"
#include <cmath>
#include <map>
#include "AQLBasic.h"


class AQLInterpolationBase;

//===================== Class Declare AQLMathObject==================================
/*! 
 
*/
class AQLMathObject
{

public:

    virtual ~AQLMathObject() {}

    virtual AQLMathObject* clone() const = 0;

    virtual double getT( double t ) = 0;

    virtual double integrate( double t_s, double t_e ) = 0;

	double expIntegrate( double t_s, double t_e ) { return AQLMath::exp( integrate( t_s, t_e ) ); }

    virtual AQLString getType() const = 0;
};

//===================== Class Declare AQLMathObjectConst==================================
/*! 
 
*/
class AQLMathObjectConst : public AQLMathObject
{

public:

    AQLMathObjectConst() : mC(0.0) {};

    AQLMathObjectConst( double c_ ) : mC( c_ ) {}

    AQLMathObjectConst* clone() const { return new AQLMathObjectConst(*this); }

    double getT( double t ) { t; return mC; }

    double integrate( double t_s, double t_e ) { return mC * (t_e - t_s); }

    AQLString getType() const { return "CONST"; }

private:

    double mC;
};

//===================== Class Declare AQLMathObjectArray==================================
/*! 
 
*/
class AQLMathObjectArray : public AQLMathObject
{

public:

    AQLMathObjectArray( const DoubleArray& x_,
                          const DoubleArray& y_,
                          AQLInterpolationBase& interpolation_
                        );

    AQLMathObjectArray( const AQLMathObjectArray& rhs );

    ~AQLMathObjectArray();

    AQLMathObjectArray& operator =( const AQLMathObjectArray& rhs );
  
    AQLMathObjectArray* clone() const { return new AQLMathObjectArray(*this); }

    double getT( double t );
    
    double integrate( double t_s, double t_e );

    AQLString getType() const { return "ARRAY"; }

    const DoubleArray &getX() const { return mX; }
    const DoubleArray &getY() const { return mY; }

	void setX(const DoubleArray& x_) const { return mX = x_; }
	void setX(const DoubleArray& y_) const { return mY = y_; }


private:

    DoubleArray mX;
    DoubleArray mY;
    AQLInterpolationBase *mpInterpolation;
    
    AQLMathFunction<AQLMathObjectArray> mGet;
    map<pair<double, double>, double>* mpIntegrateCache;
    bool mDeleteCache;

    bool mCloned;
};

#endif
