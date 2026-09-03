#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif


#include "AQLCoreAppError.h"
#include "AQLCoreSystemError.h"
#include "AQLCoreTemplateType.h"

#include <vector>
#include "AQLRatesBM.h"


class AQLRandBase;

using namespace std;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of brownian motion class

*/
class AQLRatesBM_BB : public AQLRatesBM
{
public:
//  LIFECYCLE
								//======================================
                            	// constructor
	AQLRatesBM_BB(const DoubleArray& timegrid,
              const AQLRandBase& rand,
              unsigned int factor_num = 1
              );
	
    AQLRatesBM_BB(const DoubleArray& timegrid,
              const AQLRandBase& rand,
              const std::vector<DoubleMatrix>& loading
              );

    AQLRatesBM_BB(std::vector<std::pair<AQLRatesBM*, unsigned int> >& bm,
              const std::vector<DoubleMatrix>& loading,
              bool deleteflag = true
              );
            					//======================================
                            	// copy constructor
    AQLRatesBM_BB(const AQLRatesBM_BB& rhs);
								//======================================
	                            // clone method
    virtual AQLRatesBM*     clone() const;

                                //======================================
	                            // Destructor
	virtual ~AQLRatesBM_BB();
                                //======================================

//  QUERY
 								//======================================
								// calcBM
    virtual void				calcBM(bool isIdling = false);

private:
								//======================================
								// init
	void                              init();
                               //======================================
								// buildPath
	void                        buildPath(DoubleArray &path, const DoubleArray &gaussians);


	unsigned long                     mNumberOfSteps; // number of steps
	unsigned int                      mNumFactor;     // factor num
	DoubleArray                       mTGrid;         // T_Grid
	DoubleArray                       mSqrtdt;        // Sqrtdt
	vector<unsigned long>             mLeftIndex;     // left index
	vector<unsigned long>             mRightIndex;    // right index 
	vector<unsigned long>             mBridgeIndex;   // bridge index

	DoubleArray                       mLeftWeight;    // left weight
	DoubleArray                       mRightWeight;   // right weight 
	DoubleArray                       mStdDiv;        // standard deviation 
};
