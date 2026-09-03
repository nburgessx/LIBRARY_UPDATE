// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/
// 2012, AlgoQuantHub.
///
#ifndef LAMathLeastSquareHybridHWLVProcess_h
#define LAMathLeastSquareHybridHWLVProcess_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathLeastSquareHybridHWLVProcess.h
//
//  DESCRIPTION :      
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////

#include <AQLCoreUtil.h>
#include <vector>
#include <map>
#include "AQLFunction.h"
#include <limits>

#include "LAQuantLibCostFunction.h"
#include "LAMathLeastSquareHybridHWLVProcessQlib.h"


class CostFunction;
//===================== Class Declare LAMathLeastSquareHybridHWLVProcess==================================
/*! 	

*/
class LAMathLeastSquareHybridHWLVProcess : public LAQuantLibCostFunction
{
public:

	enum mode { Diff, Ratio, Sqrt_Ratio };

    //
    LAMathLeastSquareHybridHWLVProcess() {}

    LAMathLeastSquareHybridHWLVProcess(const DoubleArray& fx0,
                                     std::vector<LAMathHybridHWLVProcessHelper*>& processHelper,
                                     LAMathTarget2fit2& t2f,
                                     mode mode_ = Diff
                                    );


	// Copy constructor
    LAMathLeastSquareHybridHWLVProcess( const LAMathLeastSquareHybridHWLVProcess& rhs );   

    // Destructor
    virtual ~LAMathLeastSquareHybridHWLVProcess();

    //
    LAMathLeastSquareHybridHWLVProcess& operator =(const LAMathLeastSquareHybridHWLVProcess& rhs);
     
};

//===================== Class Declare LAMathLeastSquareHybridHWDDProcess==================================
/*! 	

*/
//class CostFunction 
class LAMathLeastSquareHybridHWDDProcess : public LAMathLeastSquareHybridHWLVProcess
{
public:

	enum mode { Diff, Ratio, Sqrt_Ratio };

    LAMathLeastSquareHybridHWDDProcess(const DoubleArray& fx0,
                                     std::vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                     LAMathTarget2fit2& t2f,
                                     mode mode_ = Diff
                                    );

    // Copy constructor
    LAMathLeastSquareHybridHWDDProcess( const LAMathLeastSquareHybridHWDDProcess& rhs );   

    // Destructor
    virtual ~LAMathLeastSquareHybridHWDDProcess();
    //
    LAMathLeastSquareHybridHWDDProcess& operator = (const LAMathLeastSquareHybridHWDDProcess& rhs);
    
};

//===================== Class Declare LAMathLeastSquareHybridHWDDProcess1==================================
/*! 	

*/
//class CostFunction 
class LAMathLeastSquareHybridHWDDProcess1 : public LAMathLeastSquareHybridHWDDProcess
{
public:
    LAMathLeastSquareHybridHWDDProcess1(const DoubleArray& fx0,
                                      std::vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                      LAMathTarget2fit2& vol2fit,
                                      LAMathTarget2fit2& skew2fit,
                                      DoubleArray weightVolSkew, //{vol_weight, skew_weight}
                                      mode mode_ = Diff
                                     );

    // Copy constructor
    LAMathLeastSquareHybridHWDDProcess1( const LAMathLeastSquareHybridHWDDProcess1& rhs );   

    // Destructor
    virtual ~LAMathLeastSquareHybridHWDDProcess1();

    //
    LAMathLeastSquareHybridHWDDProcess1& operator = (const LAMathLeastSquareHybridHWDDProcess1& rhs);
    
};

//===================== Class Declare LAMathLeastSquareHybridHWDDProcess2==================================
/*! 	

*/
//class CostFunction 
class LAMathLeastSquareHybridHWDDProcess2 : public LAMathLeastSquareHybridHWDDProcess
{
public:
    LAMathLeastSquareHybridHWDDProcess2(const DoubleArray& fx0,
                                      std::vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                      LAMathTarget2fit2& vol2fit,
                                      double ts = 0.,
                                      mode mode_ = Diff
                                     );

    // Copy constructor
    LAMathLeastSquareHybridHWDDProcess2( const LAMathLeastSquareHybridHWDDProcess2& rhs );   

    // Destructor
    virtual ~LAMathLeastSquareHybridHWDDProcess2();

    //
    LAMathLeastSquareHybridHWDDProcess2& operator = (const LAMathLeastSquareHybridHWDDProcess2& rhs);
    
};

//===================== Class Declare LAMathLeastSquareHybridHWDDProcess3==================================
/*! 	

*/
//class CostFunction 
class LAMathLeastSquareHybridHWDDProcess3 : public LAMathLeastSquareHybridHWDDProcess
{
public:
    LAMathLeastSquareHybridHWDDProcess3(const DoubleArray& fx0,
                                      std::vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                      LAMathTarget2fit2& skew2fit,
                                      double ts = 0.,
                                      mode mode_ = Diff
                                     );

    // Copy constructor
    LAMathLeastSquareHybridHWDDProcess3( const LAMathLeastSquareHybridHWDDProcess3& rhs );   

    // Destructor
    virtual ~LAMathLeastSquareHybridHWDDProcess3();

    //
    LAMathLeastSquareHybridHWDDProcess3& operator = (const LAMathLeastSquareHybridHWDDProcess3& rhs);
    
 
};

//
//----------------------------------------------------------------------------------------
//


#endif
