#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "LAObject.h"
#include "LACoreAutoPtr.h"
#include "LAFunctionBase.h"

//// DEFINES ////
// Entitiy id of LAMathCorrelation
#define ENTITY_IR_CORRELATION 15

#ifndef	IR_CALIBRATION_DATA_ISCORINPUT					
#define	IR_CALIBRATION_DATA_ISCORINPUT		"IsCorInput"		// Data Name of	IsCorInput
#endif						
#ifndef	IR_CALIBRATION_DATA_FACTORNUM_AFTER					
#define	IR_CALIBRATION_DATA_FACTORNUM_AFTER	"FactorNumAfterReduction"	// Data Name of	FactorNum_After
#endif						
#ifndef	IR_CALIBRATION_DATA_FACTORNUM_BEFORE					
#define	IR_CALIBRATION_DATA_FACTORNUM_BEFORE	"FactorNumBeforeReduction"	// Data Name of	FactorNum_Before
#endif						
#ifndef	IR_CALIBRATION_DATA_ISEXTRA_GRID_LARGE_T					
#define	IR_CALIBRATION_DATA_ISEXTRA_GRID_LARGE_T	"IsExtraGrid_T"			// Data Name of	IsExtraGrid_T
#endif						
#ifndef	IR_CALIBRATION_DATA_ISOPTIM					
#define	IR_CALIBRATION_DATA_ISOPTIM			"IsOptimization"			// Data Name of	IsOptim
#endif						
#ifndef	IR_CALIBRATION_DATA_ISMULTIVOL					
#define	IR_CALIBRATION_DATA_ISMULTIVOL		"IsMultiVolatility"		// Data Name of	IsMultiVol
#endif						
#ifndef	IR_CALIBRATION_DATA_GRID_LARGE_T					
#define	IR_CALIBRATION_DATA_GRID_LARGE_T		"Grid_T"			// Data Name of	Grid_T
#endif						
#ifndef	IR_CALIBRATION_DATA_GRID_SMALL_T					
#define	IR_CALIBRATION_DATA_GRID_SMALL_T		"Grid_t"			// Data Name of	Grid_t
#endif						
#ifndef	IR_CALIBRATION_DATA_ISDATAINPUT					
#define	IR_CALIBRATION_DATA_ISDATAINPUT		"IsDataInput"		// Data Name of	IsDataInput
#endif						
#ifndef	IR_CALIBRATION_DATA_CORRELATIONDATA					
#define	IR_CALIBRATION_DATA_CORRELATIONDATA	"CorrelationData"		// Data Name of	Correlation data
#endif						
#ifndef	IR_CALIBRATION_DATA_FACTORLOADINGDATA					
#define	IR_CALIBRATION_DATA_FACTORLOADINGDATA	"FactorLoadingData"		// Data Name of	FactorLoading
#endif						
#ifndef	IR_CALIBRATION_DATA_FUNCTIONS					
#define	IR_CALIBRATION_DATA_FUNCTIONS			"Functions"			// Data Name of	functions
#endif						
#ifndef	IR_CALIBRATION_DATA_OPTWEIGHT					
#define	IR_CALIBRATION_DATA_OPTWEIGHT			"OptimizeWeight"	// Data Name of	OptimizeWeight
#endif
#ifndef IR_CALIBRATION_DATA_CORFACTORDATA
#define IR_CALIBRATION_DATA_CORFACTORDATA         "CorFactorData" // Data Name of CorFactorData
#endif
#ifndef IR_CALIBRATION_DATA_EIGENVALUES
#define IR_CALIBRATION_DATA_EIGENVALUES         "EigenValues" // Data Name of EigenValues
#endif
#ifndef IR_CALIBRATION_DATA_EIGENVECTORS
#define IR_CALIBRATION_DATA_EIGENVECTORS         "EigenVectors" // Data Name of EigenValues
#endif


class LADataInstance;
class LADate;
class LADataBool;
class LADataBools;
class LADataDate;
class LAString;
class LADataDoubles;
class LADataDoubleMatrix;
class LADataString;
class LAPriceDataInterpolation;
class LAFunctionBase;

/*! 
    @brief Class to represent correlation.
*/
class LAMathCorrelation : public LAObject
{
public:
// LIFECYCLE
    // default constructor
	LAMathCorrelation(LADataInstance* dataInstance);
    // copy constructor
	LAMathCorrelation(const LAMathCorrelation& cor);
    // destructor
	virtual ~LAMathCorrelation();

//  QUERY
    // Return this class type
	virtual object_t	getType(void) const;
    // Check function for this class type
	virtual bool		isTypeOf(object_t id) const;
	// get this FX name
	const LADataString&	getName() const;
	// get this FX name. The setting of name is also possible. 
	LADataString&		getName();
	//	get interpolation method
	const LAPriceDataInterpolation&
						getInterpolation() const;
	//	get interpolation method. The setting of interpolation method is also possible. 
	LAPriceDataInterpolation&
						getInterpolation();	 
	//	get multi volatility flag
	const LADataBool&	getIsMultiVol() const;
	//	get multi volatility or not. The setting of multi volatility flag is also possible. 
	LADataBool&			getIsMultiVol();
	//	get T (rate reset time) grid
	const LADataDoubles&
						getTGrid() const;
	//	get T (rate reset time) grid. The setting of T grid is also possible. 
	LADataDoubles&		getTGrid();	    	
	// get Optimize Weight
	const LADataDoubleMatrix&
						getOptWeight() const;
	// get Optimize Weight. The setting of name is also possible. 
	LADataDoubleMatrix&	getOptWeight();

    const LADataDoubleMatrix&
                        getCorFactors() const;

	LADataDoubleMatrix& getCorFactors();

	const LADataBools&	getIsExtraTGrid() const;

	LADataBools&		getIsExtraTGrid();

	//	make copy(clone) of this FX object object.
	LAObject*			clone() const;// %%% COVARIANT RETURN %%%
	//	get correlation as function matrix
	std::vector<std::vector<LAFunctionBase*> >
						getCorrelationFunc() const;

	// get FactorLoading(iÅ~jÅ~t)
	std::vector<DoubleMatrix>
						getFactorLoading(const DoubleArray& grid_t) const;

//  OPERATION 
	// set FactorLoading from grid_t and loading
	void				setFactorLoading(const DoubleArray& grid_t, const std::vector<DoubleMatrix>& loading);
	// set FactorLoading from loading
	void				setFactorLoading(const DoubleMatrix& loading);
	// set FactorLoading from loading as function matrix
	void				setFactorLoading(const std::vector<std::vector<LAFunctionBase*> >& loading);
	// set Correlation from grid_t and correlation of DoubleMatrix vector
	void				setCorrelation(const DoubleArray& grid_t, 
									 const std::vector<DoubleMatrix>& cor);
	// set Correlation as DoubleMatrix
	void				setCorrelation(const DoubleMatrix& cor);
	// set Correlation as function matrix
	void				setCorrelation(const std::vector<std::vector<LAFunctionBase*> >& cor);
	// set Correlation as function 
	void				setCorrelation(LAFunctionBase* cor);
	// set Correlation as zero 
	void				setCorrelation(unsigned int factornum);
	// calc FactorLoading
	void				calcFactorLoading() const;
	// calc FactorLoading
	DoubleMatrix		calcFactorLoading(const DoubleMatrix& corrMat, const DoubleMatrix& initialValue) const;

	// remove specified Data.If there is not Data to remove, do nothing.If member variable is specified to remove, do not remove it.
	virtual void        remove(const LAString& dataName);
	// Initialize this Object.
	virtual void		reset(void);
	
protected:
	// calc Correlation From FactorLoading
	std::vector<std::vector<LAFunctionBase*> >
						calcCorrelationFromFactorLoading() const;
	// calc Correlation From Correlation
	std::vector<std::vector<LAFunctionBase*> >
						calcCorrelationFromCorrelation() const;

	// culc Principal Component
	virtual void		calcPCA(const DoubleMatrix& cor, DoubleMatrix& vec, DoubleArray& val, unsigned int factornum) const;
	// calc FactorLoading
	virtual void		calcFactorLoading(const DoubleMatrix& cor, DoubleMatrix& loading, unsigned int factornum, bool isOptim) const;

	// copy FX object	 
	virtual LAObject&	copy(const LAObject& e);

private:
	// set Data specified by the name.
	LADataHolder&				add(const LAString& name);

	LADataHolder*				mpName;     // name (DATA_STRING)
	LADataHolder*				mpIsCorInput;// correlation input or not (DATA_BOOL)
	LADataHolder*				mpInter;// interpolation (DATA_INTERPOLATION)
	LADataHolder*				mpFactorNum_after;// factornum(after factor reduction) (DATA_INT)
	LADataHolder*				mpFactorNum_before;// factornum(before factor reduction) (DATA_INT)
	LADataHolder*				mpIsExtraGrid_T;// Extra T Grid flag(DATA_BOOLS)
	LADataHolder*				mpIsOptim;// optimization flag(DATA_BOOL)
	LADataHolder*				mpIsMultiVol;// multi volatility flag(DATA_BOOL)
	LADataHolder*				mpGrid_T;   // T grid (DATA_DOUBLES)
	LADataHolder*				mpGrid_t;   // t grid (DATA_DOUBLES)
	LADataHolder*				mpIsDataInput;// data input or function input(DATA_BOOL)
	LADataHolder*				mpCorrelation;// correlation data (DATA_DOUBLE_MATRIX)
	LADataHolder*				mpFactorLoading;// factor loading data (DATA_DOUBLE_MATRIX)
	LADataHolder*				mpFunctions;// data (DATA_FUNCTIONS)
	LADataHolder*				mpOptWeight;// Optimize Weight (DATA_DOUBLE_MATRIX)
    LADataHolder*				mpCorFactors;// correlation factors (DATA_DOUBLE_MATRIX)
    LADataHolder*				mpEigenValues;// eigen values (DATA_DOUBLES)
    LADataHolder*				mpEigenVectors;// eigen vectorss (DATA_DOUBLE_MATRIX)

	class LAMathFactorLoadingFunction : public LAFunctionBase
	{
	public:
		// Default constructor
		LAMathFactorLoadingFunction();
		// Destructor
		~LAMathFactorLoadingFunction();

									// Make copy(clone) of this class
		virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
									// purpose value for optimize
		virtual double				operator()(const DoubleArray& x) const;
		virtual double				operator()(const double& x) const;

		virtual	LAFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
									{
										return LAFunctionBase::operator()(pos, x);
									}
		
		LAMathFactorLoadingFunction & operator=( const LAMathFactorLoadingFunction & ) { return *this; }

		                            // set Correlation
		virtual void				setCorrelation(const DoubleMatrix& corr);
		                            // set Optimize Weight
		virtual void				setOptWeight(const DoubleMatrix& weight);
									// set Factor number
		virtual void				setFactornum(unsigned int& factornum);
									// culc b from paramater of thita
		static DoubleMatrix			fromthitaTob(const DoubleArray& thita, unsigned int CorrSize, unsigned int factornum);

	private:
		DoubleMatrix mWeight;		// Optimize Weight
		DoubleMatrix mCorr;			// Correlation
		unsigned int mFactornum;	// Factor Number
	};
};

