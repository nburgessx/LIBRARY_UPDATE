#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "LAObject.h"
#include "LAMathAttrSDE.h"

//// DEFINES ////
// Entitiy id of LAMathPathEntity
#define ENTITY_PATH 12

#ifndef IR_MODEL_DATA_DAYCOUNT
#define IR_MODEL_DATA_DAYCOUNT	"DayCount"		//  data name of daycount 
#endif
#ifndef IR_MODEL_DATA_SDETIMEGRID
#define IR_MODEL_DATA_SDETIMEGRID	"SDETimeGrid"	//  data name of sde time grid
#endif
#ifndef IR_MODEL_DATA_SDEINTEGRALTIMEGRID
#define IR_MODEL_DATA_SDEINTEGRALTIMEGRID	"SDEIntegralTimeGrid"//  data name of sde integral time grid
#endif
#ifndef IR_MODEL_DATA_RANDGENERATOR
#define IR_MODEL_DATA_RANDGENERATOR	"RandGenerator"//  data name of rand generator
#endif
#ifndef IR_MODEL_DATA_STARTPATHNUM
#define IR_MODEL_DATA_STARTPATHNUM	"StartPathNum"//  data name of start path num
#endif
#ifndef IR_MODEL_DATA_SDEDATANAMES
#define IR_MODEL_DATA_SDEDATANAMES	"SDEAttrNames"//  data name of data names of sdes
#endif
#ifndef IR_MODEL_DATA_SIMULATIONSDEDATANAMES
#define IR_MODEL_DATA_SIMULATIONSDEDATANAMES	"SimulationSDEAttrNames"//  data name of data names of simulationsdes.
#endif
#ifndef IR_MODEL_DATA_INITIALVALUES
#define IR_MODEL_DATA_INITIALVALUES	"InitialValues"//  data name of initial values
#endif
#ifndef IR_MODEL_DATA_CORRELATIONMATRIX
#define IR_MODEL_DATA_CORRELATIONMATRIX "CorrelationMatrix"	//  data name of correlation matrix between SDEs
#endif
#ifndef IR_MODEL_DATA_CORRELATIONMATRIXREF
#define IR_MODEL_DATA_CORRELATIONMATRIXREF "CorrelationMatrixReference"	//  data name of reference of correlation matrix between SDEs
#endif
#ifndef IR_MODEL_DATA_SDEINTEGRALDIVNUM
#define IR_MODEL_DATA_SDEINTEGRALDIVNUM	"SDEIntegralDivNum"	//  data name of divided number of time gird of sde integral
#endif
#ifndef IR_MODEL_DATA_ISANTITHETIC
#define IR_MODEL_DATA_ISANTITHETIC		"IsAntithetic"	//  data name of antithetic or not
#endif
#ifndef IR_MODEL_DATA_CACHESIZE
#define IR_MODEL_DATA_CACHESIZE			"CacheSize"	//  data name of cachesize
#endif
#ifndef IR_MODEL_DATA_ISBROWNIANBRIDGE
#define IR_MODEL_DATA_ISBROWNIANBRIDGE			"IsBrownianBridge"	//  data name of brownian bridge
#endif
#ifndef IR_MODEL_DATA_IRCURVEPROS
#define IR_MODEL_DATA_IRCURVEPROS			"IRCurvePros"		//  data name of IRCurvePros 
#endif
#ifndef IR_MODEL_DATA_IRCURVEPRONAMES
#define IR_MODEL_DATA_IRCURVEPRONAMES			"IRCurveProNames"		//  data name of IRCurvePros 
#endif
#define TYPE_CACHESIZE_CHANGE							0x0004
#define TYPE_ANTITHETICFLAG_CHANGE						0x0008
#define TYPE_BROWNIANBRIDGE_CHANGE						0x0012

#define SIMUBASIS "SIMUBASIS"


class LADataInstance;
class LADataInt;
class LADataDoubles;
class LADataDoubleMatrix;
class LADataDate;
class LADataReference;
class LADataString;
class LADataStrings;
class LADataMultiReference;
class LAPriceDataDayCount;
class LAPriceDataRand;
class LARatesPathElementBase;
class LARatesSDEBase;
class LARatesBM;


typedef std::vector<LARatesPathElementBase*>	ONEPATH;   // vector of pointer to path element
typedef std::vector<ONEPATH>				MCPATH;   // matrix of pointer to path element

/*! 
    @brief Class to represent MC path.
*/
class LAMathPathEntity : public LAObject
{
public:
// LIFECYCLE
    // default constructor
	LAMathPathEntity(LADataInstance* dataInstance);
    // copy constructor
	LAMathPathEntity(const LAMathPathEntity& irse);
    // destructor
	virtual ~LAMathPathEntity();

//  QUERY
    // Return this class type
	virtual object_t	getType(void) const;
    // Check function for this class type
	virtual bool		isTypeOf(object_t id) const;
    // get basedate
	const LADataDate&	getAsOfDate(void) const;
    // get basedate. The setting of basedate is also possible. 
	LADataDate&			getAsOfDate(void);
	// get this path name
	const LADataString&	getName() const;
	// get this path name. The setting of name is also possible. 
	LADataString&		getName();
	// get daycount
	const LAPriceDataDayCount&	
						getDayCount() const;
	// get daycount. The setting of daycount is also possible. 
	LAPriceDataDayCount&	    getDayCount();
	// get sde time grid
	const LADataDoubles&	
						getSDETimeGrid() const;
	// get sde time grid. The setting of sde time grid is also possible. 
	LADataDoubles&		getSDETimeGrid();
	// get sde integral time grid
	const LADataDoubles&	
						getSDEIntegralTimeGrid() const;
	// get sde integral time grid. The setting of integral time grid is also possible. 
	LADataDoubles&	getSDEIntegralTimeGrid();
	// get divided number of integral time grid 
	const LADataInt&	
						getSDEIntegralDivNum() const;
	// get divided number of integral time grid . The setting of divided number is also possible. 
	LADataInt&			getSDEIntegralDivNum();
	// get rand generator
	const LAPriceDataRand&	
						getRand() const;
	// get rand generator. The setting of rand generator is also possible. 
	LAPriceDataRand&			getRand();
	// get data names of SDEs
	const LADataStrings&	
						getSDEAttrNames() const;
	// get data names of SDEs. The setting of data names of SDEs is also possible. 
	LADataStrings&		getSDEAttrNames();
	// get data names of SimulationSDEs
	const LADataStrings&	
						getSimulationSDEAttrNames() const;
	// get data names of SimulationSDEs. The setting of data names of SDEs is also possible. 
	LADataStrings&		getSimulationSDEAttrNames();

	// get initial values of SDEs
	const LADataMultiReference&	
						getInitialValues() const;
	// get initial values of SDEs. The setting of initial values of SDEs is also possible. 
	LADataMultiReference&
						getInitialValues();	
	// get start path number
	const LADataInt&	
						getStartPathNum() const;
	// get start path number. The setting of start path number is also possible. 
	LADataInt&			getStartPathNum();

	// get object name which has correlation matrix.
	LAString			getCorrelationMatrixEntityName() const;

	// get correlation matrix between SDEs
	const LADataDoubleMatrix&	
						getCorrelationMatrix() const;
	// get correlation matrix between SDEs. The setting of correlation matrix is also possible. 
	LADataDoubleMatrix&	getCorrelationMatrix();
	// get this ir curvepros names
	const LADataStrings& getIRCurveProNames() const;
	// get this ir curvepros names
	LADataStrings& getIRCurveProNames();

	//  set antithetic or not
	void				setAntithetic(bool flag = true); 
	/*!
		@brief check antithetic or not
		@return true:antithetic,false:not antithetic
	*/
//	bool				isAntithetic(void) {return mIsAntithetic;}
	//  set antithetic or not
	bool				isAntithetic(void);

	//  set brownian bridge or not 
	void				setBrownianBridge(bool flag = true); 
	//  set  brownian bridge or not
	bool				isBrownianBridge(void);

	// get path 
	/*!
		@return path
		@param[in] sde position
	*/
	const ONEPATH&		getPath(unsigned int pos) const {return *mpPath[pos];}
	// get path 
	const LARatesPathElementBase&
						getPath(unsigned int pos, double t) const;
	// get cache path
	const ONEPATH&		getCache(unsigned int mnum, unsigned int pos) const;

	// get current path number
	/*!
		@return get current path number
	*/
//	int					getCurrentPathNum()const {return mPos;}
	// set next path
	void				setNextPath();	
	// set up for MC calculation
	void				setUpforMC();
	// get cash size
	int					getCacheSize() const;	
	// set cash size
	void				setCacheSize(unsigned int size);	
	// get odd or even
	/*!
		@return odd or even
	*/	
	bool				isOdd() const {return mIsOdd;}	
	
	//	make copy(clone) of this path object object.
	LAObject*			clone() const;// %%% COVARIANT RETURN %%%
	
//  OPERATION 
	// remove specified Data.If there is not Data to remove, do nothing.If member variable is specified to remove, do not remove it.
	virtual void        remove(const LAString& dataName);
	// Initialize this Object.
	virtual void		reset(void);

protected:
	// copy path object	 
	virtual LAObject&	copy(const LAObject& e);
	// set intial value to sde except
	void				setInitialValue();

private:
	// clear cash	 
	void						clearCache(void);
	// set up sde
	void						setUpSDE(void);
	// calculate factor loading from correlation
	DoubleMatrix				calcFactorLoading(const DoubleMatrix& cor);
	// set Data specified by the name.
	LADataHolder&				add(const LAString& name);
	// get correlation holder. Initialize it if not yet.
	LADataHolder*				getCorrelationHolder() const;

	LADataHolder*				mpName;			// name (DATA_STRING)
	LADataHolder*               mpAsOfDate;		// base date (DATA_DATE)
	LADataHolder*				mpDC;			// daycount convention(DATA_DAYCOUNT)
	LADataHolder*				mpSDETimeGrid;// time gird of output(DATA_DOUBLES)
	LADataHolder*				mpSDEIntegralTimeGrid;// time gird of sde integral(DATA_DOUBLES)
	LADataHolder*				mpRand;			// rand generator(DATA_RAND)
	LADataHolder*				mpStartPathNum;	// start path number(DATA_INT)
	LADataHolder*				mpSDEAttrNames;	// data names of SDEs(DATA_STRINGS)
	LADataHolder*				mpSimSDEAttrNames;	// data names of SimSDEs(DATA_STRINGS)
	LADataHolder*				mpInitialValues;// initial values of SDEs(DATA_MULTIREFERENCE)
	LADataHolder*				mpCor;// correlation matrix between SDEs(DATA_DOUBLEMATRIX)
	LADataHolder*				mpSDEIntegralDivNum;// divided number of time gird of sde integral(DATA_INT)
	LADataHolder*				mpIsAntithetic;// antithetic flag
	LADataHolder*				mpCacheSize;// cache size
	LADataHolder*				mpIsBrownianBridge;// brownian bridge flag
	LADataHolder*				mpIRCurveProNames; // ir curvespro names (DATA_STRINGS)
	

	std::vector<LARatesSDEBase*>	mSDEs;			// vector of pointer to sde
//	bool						mIsAntithetic;	// antithetic flag
//	int							mCacheSize;		// cash size 
	int							mCachePos;		// cash position
	int							mAntiCachePos;	// anti cash position
	int							mPos;			// path position number
	std::vector<const ONEPATH*>	mpPath;			// current path
	std::vector<MCPATH>			mCache;			// cash	
	std::vector<MCPATH>			mAntiCache;		// cash for Antithetic
	bool						mIsOdd;			// odd or even flag for Antithetic 
	std::set<LARatesBM*>			mpBMs;			// Brownian Motion
	LARatesBM*						mpBM;			// Brownian Motion
	int							mPathVersion;	// version
	DoubleArray					mSDEIntegralTimeGrid;// integral time grid 

};
