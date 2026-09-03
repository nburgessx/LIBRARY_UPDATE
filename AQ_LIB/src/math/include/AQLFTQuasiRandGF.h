#pragma once

#include "AQLRandBase.h"
#include "AQLMersenneTwister.h"

// AQLFTQuasiRandGF Function ID
#define FN_RAND_QUASIGF     1106
// AQLFTQuasiRandGF Function Name
#define FN_RAND_QUASIGF_STR	"fn_rand_quasigf"

///////////////////////////////////////////////////////////////////////
/*!
    @brief Class declaration of Quasi-random number generator. 
*/

class AQLFTQuasiRandGF : public AQLRandBase
{
public:

//  LIFECYCLE
	//constructor
    AQLFTQuasiRandGF();

	//copy constructor
//	AQLFTQuasiRandGF(const AQLFTQuasiRandGF&);
	
	//destructor
    virtual ~AQLFTQuasiRandGF();

//  QUERY
                                //======================================
                                // check whether this class derives from base class with type id
    virtual bool                isTypeOf(function_t id) const;
                                //======================================
                                // deep copy of this object
    virtual AQLCoreFunctionBase*     clone() const;
                                //======================================
                                // get function type
    virtual function_t          getType() const;
								
                                //==========================================
                                // function to generate uniform random numbers
    virtual void                getUniforms(DoubleArray& variates);

                                //==========================================
								// function to set dimension information(override the base class to match the setting of the dimension)
								//dimValue[0] total dimension
								//dimValue[1] dimension for quasi-random number
	virtual void				setDim(const UintArray& dimValue);


	
                                //==========================================
                                // function to set seed(override the base class to match the setting of the dimension)
								//Seed[0]:start number
								//Seed[1]:necessary Sequence number
								//Seed[2]:Seed For Generator Matrix
								//Seed[3]:Seed For Mersennne Twister
	virtual void                setSeed(const UlongArray& seed);

	AQLFTQuasiRandGF & operator=( const AQLFTQuasiRandGF & ) { return *this; }



//  OPERATION
private:
	/*! 
		@brief Subclasses for performing matrix computations necessary in the development of quasi-random number.
	*/
	class AQLMatrixData {
	public:
		//LIFECYCLE
		// constructor
		AQLMatrixData(void);
		// constructor (n row,m column)
		AQLMatrixData(unsigned int n, unsigned int m);
		// destructor
		~AQLMatrixData();
		// copy constructor
		AQLMatrixData(const AQLMatrixData& m);

		//OPERATION
								// resize data
		void					resize(unsigned int row, unsigned int col);

								// function to get row numbers
								/*!
									@return row numbers
								*/
		unsigned int			row(void) const {return mRow;}

								// function to get column numbers
		unsigned int			col(void) const {return mCol;}

								// function to get the value at element(i,j)
		int						getValue(unsigned int i, unsigned int j) const;
								
								// function to set the value(double) to element(i,j)
		void					setValue(unsigned int i, unsigned int j, int value);

								// function to return pointer with arrays
								/*!
									@return pointer
								*/
		int*					operator[](unsigned int n) {return mpData[n];}

								// function to return pointer with arrays
								/*!
									@return pointer
								*/
		const int*				operator[](unsigned int n) const {return mpData[n];}

								
								// assignment
		void					operator =(const AQLMatrixData& other);

								// product of the matrix
		AQLMatrixData			matMult(const AQLMatrixData& other,int base) const;
		void					print(void);

	private:
		// pointer for matrix data
		int** mpData;

		// row number of matrix
		unsigned int mRow;

		// column number of matrix
		unsigned int mCol;

	};

	// matrix for Genelalization
	AQLMatrixData mRandMat;
	
	// area used in Generator Matrix
	AQLMatrixData mGeneMat;
	
	// vector of radix expansion
	AQLMatrixData mDigitVector;

	// matrix multipled by GeneratorMatrix after radix expansion
	AQLMatrixData mDigitVectortmp;
	
	// (1,1) element
	UintArray mqvec;
		

	// flag to judge the setting of mSeedmDim
	bool			mSeedSet,mDimSet;
	
	// radix for quasi-random
	unsigned int	mBase;

	// seed of Generalization Matrix
	unsigned int	mMSeed;

	// state of Sequence Number
	unsigned long	mState; 

	// maximum of Sequence
	unsigned long	mMax; 

	// parameter calc1
	void			paraCalc(void);

	// parameter calc2
	void			getmqvec(void);

	// Mersennne Twister Object 
	AQLMersenneTwister mInner;

	//A seed of the inner rand() function only used by this class
	long			ftholdrand; 
	
	//A seed Setting function of the inner rand() function only used by this class
	void			ftsrand (unsigned long seed); 

	//the inner rand() function only used by this class
	int				ftrand(void);

protected:

};

