#pragma once


#include "LARandBase.h"

// LAMersenneTwister Function ID
#define FN_RAND_MT     1102 
// LAMersenneTwister Function Name
#define FN_RAND_MT_STR	"fn_rand_mt"


/*! 
    @brief Class to generate MersenneTwister.
*/
class LAMersenneTwister : public LARandBase
{
public:
//  LIFECYCLE
    LAMersenneTwister(bool isHalleyMod = false);// Default Construcotr
    virtual ~LAMersenneTwister();// Destructor
    // copy constructor - Use default copy constructor
    // LAMersenneTwister(const LAMersenneTwister& v);

//  QUERY
                                //======================================
                                // check whether this class derives from base class with type id
    virtual bool                isTypeOf(function_t id) const;
                                //======================================
                                // deep copy of this object
    virtual LACoreFunctionBase*     clone() const;
                                //======================================
                                // get function type
    virtual function_t          getType() const;
                                //==========================================
                                // function to generate uniform random numbers
    virtual void                getUniforms(DoubleArray& variates); 
                                //==========================================
                                // function to set seed
	virtual void                setSeed(const UlongArray& seed);
								
	// assignment operator
	LAMersenneTwister & operator=( const LAMersenneTwister & ) { return *this; }

//  OPERATION

private:
    // N = 624 defined in .cpp file.
	unsigned long               mt[624];// the array for the state vector
	int	                        mti;	// mti==N+1 means mt[N] is not initialized
	unsigned long               mag01[2];
	

	// initializes mt[N] with a seed
	void                        init_genrand(const unsigned long& s);

	// initialize by an array with array-length
	void                        init_by_array(const UlongArray& init_key);

	// generates a random number on [0,0xffffffff]-interval
	inline unsigned long        genrand_int32(void);

	// generates a random number on [0,0x7fffffff]-interval
	inline long                 genrand_int31(void);

	// generates a random number on [0,1]-real-interval
	inline double               genrand_real1(void);

	// generates a random number on [0,1)-real-interval
	inline double               genrand_real2(void);

	// generates a random number on (0,1)-real-interval
	inline double               genrand_real3(void);

	// generates a random number on [0,1) with 53-bit resolution
	inline double               genrand_res53(void);
 

protected:

};

