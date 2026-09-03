/*! @file
    @brief Class declaration to LAMathAntonovFXOption model.

*/
//  2012, AlgoQuantHub.
#ifndef MMContainer_h
#define MMContainer_h

#include "LAFunction.h"
#include <cmath>
#include <map>
#include "LABasic.h"


class LAInterpolationBase;

//===================== Class Declare LAMathObject==================================
/*! 
 
*/
class LAMathObject
{

public:

    virtual ~LAMathObject() {}

    virtual LAMathObject* clone() const = 0;

    virtual double getT( double t ) = 0;

    virtual double integrate( double t_s, double t_e ) = 0;

	double expIntegrate( double t_s, double t_e ) { return LAMath::exp( integrate( t_s, t_e ) ); }

    virtual LAString getType() const = 0;
};

//===================== Class Declare LAMathObjectConst==================================
/*! 
 
*/
class LAMathObjectConst : public LAMathObject
{

public:

    LAMathObjectConst() : mC(0.0) {};

    LAMathObjectConst( double c_ ) : mC( c_ ) {}

    LAMathObjectConst* clone() const { return new LAMathObjectConst(*this); }

    double getT( double t ) { t; return mC; }

    double integrate( double t_s, double t_e ) { return mC * (t_e - t_s); }

    LAString getType() const { return "CONST"; }

private:

    double mC;
};

//===================== Class Declare LAMathObjectArray==================================
/*! 
 
*/
class LAMathObjectArray : public LAMathObject
{

public:

    LAMathObjectArray( const DoubleArray& x_,
                          const DoubleArray& y_,
                          LAInterpolationBase& interpolation_
                        );

    LAMathObjectArray( const LAMathObjectArray& rhs );

    ~LAMathObjectArray();

    LAMathObjectArray& operator =( const LAMathObjectArray& rhs );
  
    LAMathObjectArray* clone() const { return new LAMathObjectArray(*this); }

    double getT( double t );
    
    double integrate( double t_s, double t_e );

    LAString getType() const { return "ARRAY"; }

    const DoubleArray &getX() const { return mX; }
    const DoubleArray &getY() const { return mY; }

	void setX(const DoubleArray& x_) const { return mX = x_; }
	void setX(const DoubleArray& y_) const { return mY = y_; }


private:

    DoubleArray mX;
    DoubleArray mY;
    LAInterpolationBase *mpInterpolation;
    
    LAMathFunction<LAMathObjectArray> mGet;
    map<pair<double, double>, double>* mpIntegrateCache;
    bool mDeleteCache;

    bool mCloned;
};

#endif
