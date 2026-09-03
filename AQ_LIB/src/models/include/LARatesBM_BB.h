#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif


#include "LACoreAppError.h"
#include "LACoreSystemError.h"
#include "LACoreTemplateType.h"

#include <vector>
#include "LARatesBM.h"


class LARandBase;

using namespace std;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of brownian motion class

*/
class LARatesBM_BB : public LARatesBM
{
public:
//  LIFECYCLE
								//======================================
                            	// constructor
	LARatesBM_BB(const DoubleArray& timegrid,
              const LARandBase& rand,
              unsigned int factor_num = 1
              );
	
    LARatesBM_BB(const DoubleArray& timegrid,
              const LARandBase& rand,
              const std::vector<DoubleMatrix>& loading
              );

    LARatesBM_BB(std::vector<std::pair<LARatesBM*, unsigned int> >& bm,
              const std::vector<DoubleMatrix>& loading,
              bool deleteflag = true
              );
            					//======================================
                            	// copy constructor
    LARatesBM_BB(const LARatesBM_BB& rhs);
								//======================================
	                            // clone method
    virtual LARatesBM*     clone() const;

                                //======================================
	                            // Destructor
	virtual ~LARatesBM_BB();
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
