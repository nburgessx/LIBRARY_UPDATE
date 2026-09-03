// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/
// 2012, AlgoQuantHub.
///
#ifndef AQLMathLeastSquareHybridHWLVProcess_h
#define AQLMathLeastSquareHybridHWLVProcess_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLMathLeastSquareHybridHWLVProcess.h
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

#include "AQLQuantLibCostFunction.h"
#include "AQLMathLeastSquareHybridHWLVProcessQlib.h"


class CostFunction;
//===================== Class Declare AQLMathLeastSquareHybridHWLVProcess==================================
/*! 	

*/
class AQLMathLeastSquareHybridHWLVProcess : public AQLQuantLibCostFunction
{
public:

	enum mode { Diff, Ratio, Sqrt_Ratio };

    //
    AQLMathLeastSquareHybridHWLVProcess() {}

    AQLMathLeastSquareHybridHWLVProcess(const DoubleArray& fx0,
                                     std::vector<AQLMathHybridHWLVProcessHelper*>& processHelper,
                                     AQLMathTarget2fit2& t2f,
                                     mode mode_ = Diff
                                    );


	// Copy constructor
    AQLMathLeastSquareHybridHWLVProcess( const AQLMathLeastSquareHybridHWLVProcess& rhs );   

    // Destructor
    virtual ~AQLMathLeastSquareHybridHWLVProcess();

    //
    AQLMathLeastSquareHybridHWLVProcess& operator =(const AQLMathLeastSquareHybridHWLVProcess& rhs);
     
};

//===================== Class Declare AQLMathLeastSquareHybridHWDDProcess==================================
/*! 	

*/
//class CostFunction 
class AQLMathLeastSquareHybridHWDDProcess : public AQLMathLeastSquareHybridHWLVProcess
{
public:

	enum mode { Diff, Ratio, Sqrt_Ratio };

    AQLMathLeastSquareHybridHWDDProcess(const DoubleArray& fx0,
                                     std::vector<AQLMathHybridHWDDProcessHelper*>& processHelper,
                                     AQLMathTarget2fit2& t2f,
                                     mode mode_ = Diff
                                    );

    // Copy constructor
    AQLMathLeastSquareHybridHWDDProcess( const AQLMathLeastSquareHybridHWDDProcess& rhs );   

    // Destructor
    virtual ~AQLMathLeastSquareHybridHWDDProcess();
    //
    AQLMathLeastSquareHybridHWDDProcess& operator = (const AQLMathLeastSquareHybridHWDDProcess& rhs);
    
};

//===================== Class Declare AQLMathLeastSquareHybridHWDDProcess1==================================
/*! 	

*/
//class CostFunction 
class AQLMathLeastSquareHybridHWDDProcess1 : public AQLMathLeastSquareHybridHWDDProcess
{
public:
    AQLMathLeastSquareHybridHWDDProcess1(const DoubleArray& fx0,
                                      std::vector<AQLMathHybridHWDDProcessHelper*>& processHelper,
                                      AQLMathTarget2fit2& vol2fit,
                                      AQLMathTarget2fit2& skew2fit,
                                      DoubleArray weightVolSkew, //{vol_weight, skew_weight}
                                      mode mode_ = Diff
                                     );

    // Copy constructor
    AQLMathLeastSquareHybridHWDDProcess1( const AQLMathLeastSquareHybridHWDDProcess1& rhs );   

    // Destructor
    virtual ~AQLMathLeastSquareHybridHWDDProcess1();

    //
    AQLMathLeastSquareHybridHWDDProcess1& operator = (const AQLMathLeastSquareHybridHWDDProcess1& rhs);
    
};

//===================== Class Declare AQLMathLeastSquareHybridHWDDProcess2==================================
/*! 	

*/
//class CostFunction 
class AQLMathLeastSquareHybridHWDDProcess2 : public AQLMathLeastSquareHybridHWDDProcess
{
public:
    AQLMathLeastSquareHybridHWDDProcess2(const DoubleArray& fx0,
                                      std::vector<AQLMathHybridHWDDProcessHelper*>& processHelper,
                                      AQLMathTarget2fit2& vol2fit,
                                      double ts = 0.,
                                      mode mode_ = Diff
                                     );

    // Copy constructor
    AQLMathLeastSquareHybridHWDDProcess2( const AQLMathLeastSquareHybridHWDDProcess2& rhs );   

    // Destructor
    virtual ~AQLMathLeastSquareHybridHWDDProcess2();

    //
    AQLMathLeastSquareHybridHWDDProcess2& operator = (const AQLMathLeastSquareHybridHWDDProcess2& rhs);
    
};

//===================== Class Declare AQLMathLeastSquareHybridHWDDProcess3==================================
/*! 	

*/
//class CostFunction 
class AQLMathLeastSquareHybridHWDDProcess3 : public AQLMathLeastSquareHybridHWDDProcess
{
public:
    AQLMathLeastSquareHybridHWDDProcess3(const DoubleArray& fx0,
                                      std::vector<AQLMathHybridHWDDProcessHelper*>& processHelper,
                                      AQLMathTarget2fit2& skew2fit,
                                      double ts = 0.,
                                      mode mode_ = Diff
                                     );

    // Copy constructor
    AQLMathLeastSquareHybridHWDDProcess3( const AQLMathLeastSquareHybridHWDDProcess3& rhs );   

    // Destructor
    virtual ~AQLMathLeastSquareHybridHWDDProcess3();

    //
    AQLMathLeastSquareHybridHWDDProcess3& operator = (const AQLMathLeastSquareHybridHWDDProcess3& rhs);
    
 
};

//
//----------------------------------------------------------------------------------------
//


#endif
