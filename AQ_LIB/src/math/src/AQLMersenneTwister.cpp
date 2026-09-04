/*! @file
    @brief Implementation to generate MersenneTwister.
*/




#include "AQLMersenneTwister.h"

using namespace std;


const int N = 624;// Period parameters

const int M = 397;// Period parameters

const unsigned long MATRIX_A = 0x9908b0dfUL;//! constant vector a

const unsigned long UPPER_MASK = 0x80000000UL;//! most significant w-r bits

const unsigned long LOWER_MASK = 0x7fffffffUL;//! least significant r bits 


/*!
	@brief default constructor
*/
AQLMersenneTwister::AQLMersenneTwister(bool isHalleyMod)
: AQLRandBase(isHalleyMod)
{
	// default value to mDim
	UintArray defaultDim(1);
	defaultDim[0] = 1;
	setDim(defaultDim);

	// static member in the original AQLMersenneTwister code
	mti = N+1;
	mag01[0] = 0x0UL;
	mag01[1] = MATRIX_A;
}

/*!
	@brief destructor
*/
AQLMersenneTwister::~AQLMersenneTwister()
{
}

/*!
	@brief deep copy of this object
	@return pointer to AQLCoreFunctionBase class
*/
AQLCoreFunctionBase*
AQLMersenneTwister::clone() const
{
    try 
	{
		return new AQLMersenneTwister(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
	@brief check whether this class derives from base class with type id
	@param[in] id function type to be checked
	@return True or false
*/
bool
AQLMersenneTwister::isTypeOf(function_t id) const
{
	return (id == FN_RAND_MT ? true : AQLRandBase::isTypeOf(id));
}

/*!
	@brief get type of the function
    @return function type
*/
function_t
AQLMersenneTwister::getType() const
{
	return FN_RAND_MT;
}

/*!
	@brief function to generate uniform random numbers
	@param[in] variates variable to store the calculation result
*/
void
AQLMersenneTwister::getUniforms(DoubleArray& variates)
{
	// there is no guarantee of results (This is of valarray specification of C + + standard)
	// if you do not have the same size of both valarray assignment
//	UintArray dim = getDim();
    UintArray dim( getDim().size() ); 
    dim = getDim();

    for (unsigned int i=0; i<dim[0]; i++)
	{
		variates[i] = genrand_real3();
	}
}

/*!
	@brief function to set seed
	@param[in] seed hold (multi) seed information
*/
void
AQLMersenneTwister::setSeed(const UlongArray& seed)
{
	if (seed.size() == 0)
	{
		throw AQLCoreInvalidData("Seed Array is empty.", __FILE__, __LINE__);
	}
	else if (seed.size() == 1)
	{
		init_genrand(seed[0]);
	}
	else
	{
		init_by_array(seed);
	}
	AQLRandBase::setSeed(seed);
}






// initializes mt[N] with a seed
void 
AQLMersenneTwister::init_genrand(const unsigned long& s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) 
	{
        mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void 
AQLMersenneTwister::init_by_array(const UlongArray& init_key)
{
	int key_length = init_key.size();
    int i, j, k;
    init_genrand(19650218UL);
    
	i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) 
	{
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL)) + init_key[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }

    for (k=N-1; k; k--) 
	{
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL)) - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}

// generates a random number on [0,0xffffffff]-interval 
unsigned long 
AQLMersenneTwister::genrand_int32(void)
{
    unsigned long y;
    //static unsigned long mag01[2]={0x0UL, MATRIX_A};

	// generate N words at one time
    if (mti >= N) 
	{ 
        int kk;

        if (mti == N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */

        for (kk=0; kk<N-M; kk++) 
		{
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }

        for (; kk<N-1; kk++) 
		{
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }
    y = mt[mti++];

    // Tempering
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

// generates a random number on [0,0x7fffffff]-interval
long 
AQLMersenneTwister::genrand_int31(void)
{
    return (long)(genrand_int32()>>1);
}

// generates a random number on [0,1]-real-interval
double 
AQLMersenneTwister::genrand_real1(void)
{
    return genrand_int32()*(1.0/4294967295.0); 
    // divided by 2^32-1
}

// generates a random number on [0,1)-real-interval
double 
AQLMersenneTwister::genrand_real2(void)
{
    return genrand_int32()*(1.0/4294967296.0); 
    // divided by 2^32
}

// generates a random number on (0,1)-real-interval
double 
AQLMersenneTwister::genrand_real3(void)
{
    return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0); 
    // divided by 2^32
}

// generates a random number on [0,1) with 53-bit resolution
double 
AQLMersenneTwister::genrand_res53(void) 
{ 
    unsigned long a=genrand_int32()>>5, b=genrand_int32()>>6; 
    return(a*67108864.0+b)*(1.0/9007199254740992.0); 
} 
// These real versions are due to Isaku Wada, 2002/01/09 added
