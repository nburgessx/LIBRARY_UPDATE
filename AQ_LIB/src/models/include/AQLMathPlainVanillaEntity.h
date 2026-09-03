#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLObject.h"

//// DEFINES ////
// Entitiy id of AQLMathPathEntity
#define ENTITY_PLAINVANILLA 20

#ifndef IR_MODEL_DATA_IRCURRENCYS
#define IR_MODEL_DATA_IRCURRENCYS			"IRCurrencys"		//  data name of IRCurrencys 
#endif
#ifndef IR_MODEL_DATA_IRSIMCURRENCYS
#define IR_MODEL_DATA_IRSIMCURRENCYS			"IRSimCurrencys"	//  data name of IRSimCurrencys 
#endif
#ifndef IR_MODEL_DATA_IRCURVES
#define IR_MODEL_DATA_IRCURVES				"IRCurves"		//  data name of IRCurves 
#endif
#ifndef IR_MODEL_DATA_IRCURVEPROS
#define IR_MODEL_DATA_IRCURVEPROS			"IRCurvePros"		//  data name of IRCurvePros 
#endif
#ifndef IR_MODEL_DATA_IRCURVEPRONAMES
#define IR_MODEL_DATA_IRCURVEPRONAMES			"IRCurveProNames"		//  data name of IRCurvePros 
#endif
#ifndef IR_MODEL_DATA_IRVOLATILITYS
#define IR_MODEL_DATA_IRVOLATILITYS			"IRVolatilitys"		//  data name of IRVolatilitys 
#endif
#ifndef IR_MODEL_DATA_FXCURRENCYS
#define IR_MODEL_DATA_FXCURRENCYS			"FXCurrencys"		//  data name of FXCurrencys 
#endif
#ifndef IR_MODEL_DATA_FXVOLATILITYS
#define IR_MODEL_DATA_FXVOLATILITYS			"FXVolatilitys"		//  data name of IRVolatilitys 
#endif
#ifndef IR_MODEL_DATA_FXRATE
#define IR_MODEL_DATA_FXRATE					"FXRate"		//  data name of FXRate 
#endif
#ifndef IR_MODEL_DATA_IRCURVETYPES
#define IR_MODEL_DATA_IRCURVETYPES			"IRCurveTypes"		//  data name of IRCurves 
#endif

class AQLDataInstance;
class AQLDataInt;
class AQLDataDoubles;
class AQLDataDoubleMatrix;
class AQLDataDate;
class AQLDataString;
class AQLDataStrings;
class AQLDataMultiReference;
class AQLPriceDataDayCount;
class AQLDataReference;
class AQLMathYieldCurvePro;
class AQLMathYieldCurve;
class AQLFunctionBase;
class AQLMathVolatility;
class AQLInterpolationBase;

/*! 
    @brief Class to represent Plain Vanilla Data.
*/
class AQLMathPlainVanillaEntity : public AQLObject
{
public:
// LIFECYCLE
    // default constructor
	AQLMathPlainVanillaEntity(AQLDataInstance* dataInstance);
    // copy constructor
	AQLMathPlainVanillaEntity(const AQLMathPlainVanillaEntity& irse);
    // destructor
	virtual ~AQLMathPlainVanillaEntity();

//  QUERY
    // Return this class type
	virtual object_t	getType(void) const;
    // Check function for this class type
	virtual bool		isTypeOf(object_t id) const;
    // get basedate
	const AQLDataDate&	getAsOfDate(void) const;
    // get basedate. The setting of basedate is also possible. 
	AQLDataDate&			getAsOfDate(void);
	// get this path name
	const AQLDataString&	getName() const;
	// get this path name. The setting of name is also possible. 
	AQLDataString&		getName();
	// get daycount
	const AQLPriceDataDayCount&	
						getDayCount() const;
	// get daycount. The setting of daycount is also possible. 
	AQLPriceDataDayCount&	    getDayCount();
	// get this ir currencys
	const AQLDataStrings& getIRCurrencys() const;
	// get this ir currencys
	AQLDataStrings& getIRCurrencys();
	// get this ir simulation currencys
	const AQLDataStrings& getIRSimCurrencys() const;
	// get this ir simulation currencys
	AQLDataStrings& getIRSimCurrencys();
	// get this ir currencys
	const AQLDataStrings& getFXCurrencys() const;
	// get this ir currencys
	AQLDataStrings& getFXCurrencys();

	// get this ir curves
	const AQLDataMultiReference& getIRCurves() const;
	// get this ir curves
	AQLDataMultiReference& getIRCurves();

	// get this ir curvepro names
	const AQLDataStrings& getIRCurveProNames() const;
	// get this ir curvepro names
	AQLDataStrings& getIRCurveProNames();
	// get curvepro name
	const AQLString& getIRCurveProName(const AQLString &key) const;

	// get this ir volatilitys
	const AQLDataMultiReference& getIRVolatilitys() const;
	// get this ir volaitilitys
	AQLDataMultiReference& getIRVolatilitys();

	const AQLDataStrings& getIRCurveTypes() const;
	// get this ir curves
	AQLDataStrings& getIRCurveTypes();
	// set up ir curve type
	void setUpIRCurveTypes();

	// get this fx volatilitys
	const AQLDataMultiReference& getFXVolatilitys() const;
	// get this fx volaitilitys
	AQLDataMultiReference& getFXVolatilitys();

	// get this fx object
	const AQLDataReference& getFXEntity() const;
	// get this fx object
	AQLDataReference& getFXEntity();

	// get IR Curve
	const AQLMathYieldCurve& getIRCurve(const AQLString &key) const;
	// get IR Curve
	AQLMathYieldCurve& getIRCurve(const AQLString &key);
	// get IR CurvePro
	const AQLMathYieldCurvePro& getIRCurvePro(const AQLString &key) const;
	// get IR CurvePro
	AQLMathYieldCurvePro& getIRCurvePro(const AQLString &key);
	// get FX Vol
	AQLMathVolatility& getFXVol(const AQLString &key);
	// get FX Vol method
	const AQLFunctionBase* getFXVolFunc(const AQLString &key);
	// get IR Vol
	AQLMathVolatility& getIRVol(const AQLString &key);
	// get IR Vol method
	const AQLFunctionBase* getIRVolFunc(const AQLString &key);
	////////////////////////////////
	// get correlation matrix between SDEs
	const AQLDataDoubleMatrix&	
						getCorrelationMatrix() const;
	// get correlation matrix between SDEs. The setting of correlation matrix is also possible. 
	AQLDataDoubleMatrix&	getCorrelationMatrix();
	
	//	make copy(clone) of this path object object.
	AQLObject*			clone() const;// %%% COVARIANT RETURN %%%
	
//  OPERATION 
	// remove specified Data.If there is not Data to remove, do nothing.If member variable is specified to remove, do not remove it.
	virtual void        remove(const AQLString& dataName);
	// Initialize this Object.
	virtual void		reset(void);


	void  setUpLiborRateMap(void) const;
	void  clearLiborRateMap(void) const;
	std::map<AQLString, AQLInterpolationBase*>& getLiborRateMap(void) {return mLiborRateMap;};
	std::map<AQLString, DoubleVector>& getLiborGridTermMap(void) {return mLiborGridTermMap;};
	

protected:
	// copy path object	 
	virtual AQLObject&	copy(const AQLObject& e);
private:
	// set Data specified by the name.
	AQLDataHolder&				add(const AQLString& name);
	
	AQLDataHolder*				mpName;			// name (DATA_STRING)
	AQLDataHolder*               mpAsOfDate;		// base date (DATA_DATE)
	AQLDataHolder*				mpDC;			// daycount convention(DATA_DAYCOUNT)
	AQLDataHolder*				mpCor;// correlation matrix between SDEs(DATA_DOUBLEMATRIX)
	AQLDataHolder*				mpIRCurs; // ir currencys (DATA_STRINGS)
	AQLDataHolder*				mpIRSimCurs; // ir simulation currencys (DATA_STRINGS)
	AQLDataHolder*				mpFXCurs; // ir currencys (DATA_STRINGS)
	AQLDataHolder*				mpIRCurves; // ir curves (DATA_MULTIREFERENCE)
	AQLDataHolder*				mpIRCurveProNames; // ir curves (CALIBRATION_DATAAMES)
	AQLDataHolder*				mpIRVols; // ir vols (DATA_MULTIREFERENCE)
	AQLDataHolder*				mpFXVols; // fx vols (DATA_MULTIREFERENCE)
	AQLDataHolder*				mpFX; // fx vols (DATA_REFERENCE)
	AQLDataHolder*				mpIRCurveTypes; // ir curvetypes (DATA_STRINGS)
	mutable std::map<AQLString, AQLInterpolationBase*> mLiborRateMap;
	mutable std::map<AQLString, DoubleVector> mLiborGridTermMap;
};
