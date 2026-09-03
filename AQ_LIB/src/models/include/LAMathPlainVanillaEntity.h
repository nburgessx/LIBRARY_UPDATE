#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "LAObject.h"

//// DEFINES ////
// Entitiy id of LAMathPathEntity
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

class LADataInstance;
class LADataInt;
class LADataDoubles;
class LADataDoubleMatrix;
class LADataDate;
class LADataString;
class LADataStrings;
class LADataMultiReference;
class LAPriceDataDayCount;
class LADataReference;
class LAMathYieldCurvePro;
class LAMathYieldCurve;
class LAFunctionBase;
class LAMathVolatility;
class LAInterpolationBase;

/*! 
    @brief Class to represent Plain Vanilla Data.
*/
class LAMathPlainVanillaEntity : public LAObject
{
public:
// LIFECYCLE
    // default constructor
	LAMathPlainVanillaEntity(LADataInstance* dataInstance);
    // copy constructor
	LAMathPlainVanillaEntity(const LAMathPlainVanillaEntity& irse);
    // destructor
	virtual ~LAMathPlainVanillaEntity();

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
	// get this ir currencys
	const LADataStrings& getIRCurrencys() const;
	// get this ir currencys
	LADataStrings& getIRCurrencys();
	// get this ir simulation currencys
	const LADataStrings& getIRSimCurrencys() const;
	// get this ir simulation currencys
	LADataStrings& getIRSimCurrencys();
	// get this ir currencys
	const LADataStrings& getFXCurrencys() const;
	// get this ir currencys
	LADataStrings& getFXCurrencys();

	// get this ir curves
	const LADataMultiReference& getIRCurves() const;
	// get this ir curves
	LADataMultiReference& getIRCurves();

	// get this ir curvepro names
	const LADataStrings& getIRCurveProNames() const;
	// get this ir curvepro names
	LADataStrings& getIRCurveProNames();
	// get curvepro name
	const LAString& getIRCurveProName(const LAString &key) const;

	// get this ir volatilitys
	const LADataMultiReference& getIRVolatilitys() const;
	// get this ir volaitilitys
	LADataMultiReference& getIRVolatilitys();

	const LADataStrings& getIRCurveTypes() const;
	// get this ir curves
	LADataStrings& getIRCurveTypes();
	// set up ir curve type
	void setUpIRCurveTypes();

	// get this fx volatilitys
	const LADataMultiReference& getFXVolatilitys() const;
	// get this fx volaitilitys
	LADataMultiReference& getFXVolatilitys();

	// get this fx object
	const LADataReference& getFXEntity() const;
	// get this fx object
	LADataReference& getFXEntity();

	// get IR Curve
	const LAMathYieldCurve& getIRCurve(const LAString &key) const;
	// get IR Curve
	LAMathYieldCurve& getIRCurve(const LAString &key);
	// get IR CurvePro
	const LAMathYieldCurvePro& getIRCurvePro(const LAString &key) const;
	// get IR CurvePro
	LAMathYieldCurvePro& getIRCurvePro(const LAString &key);
	// get FX Vol
	LAMathVolatility& getFXVol(const LAString &key);
	// get FX Vol method
	const LAFunctionBase* getFXVolFunc(const LAString &key);
	// get IR Vol
	LAMathVolatility& getIRVol(const LAString &key);
	// get IR Vol method
	const LAFunctionBase* getIRVolFunc(const LAString &key);
	////////////////////////////////
	// get correlation matrix between SDEs
	const LADataDoubleMatrix&	
						getCorrelationMatrix() const;
	// get correlation matrix between SDEs. The setting of correlation matrix is also possible. 
	LADataDoubleMatrix&	getCorrelationMatrix();
	
	//	make copy(clone) of this path object object.
	LAObject*			clone() const;// %%% COVARIANT RETURN %%%
	
//  OPERATION 
	// remove specified Data.If there is not Data to remove, do nothing.If member variable is specified to remove, do not remove it.
	virtual void        remove(const LAString& dataName);
	// Initialize this Object.
	virtual void		reset(void);


	void  setUpLiborRateMap(void) const;
	void  clearLiborRateMap(void) const;
	std::map<LAString, LAInterpolationBase*>& getLiborRateMap(void) {return mLiborRateMap;};
	std::map<LAString, DoubleVector>& getLiborGridTermMap(void) {return mLiborGridTermMap;};
	

protected:
	// copy path object	 
	virtual LAObject&	copy(const LAObject& e);
private:
	// set Data specified by the name.
	LADataHolder&				add(const LAString& name);
	
	LADataHolder*				mpName;			// name (DATA_STRING)
	LADataHolder*               mpAsOfDate;		// base date (DATA_DATE)
	LADataHolder*				mpDC;			// daycount convention(DATA_DAYCOUNT)
	LADataHolder*				mpCor;// correlation matrix between SDEs(DATA_DOUBLEMATRIX)
	LADataHolder*				mpIRCurs; // ir currencys (DATA_STRINGS)
	LADataHolder*				mpIRSimCurs; // ir simulation currencys (DATA_STRINGS)
	LADataHolder*				mpFXCurs; // ir currencys (DATA_STRINGS)
	LADataHolder*				mpIRCurves; // ir curves (DATA_MULTIREFERENCE)
	LADataHolder*				mpIRCurveProNames; // ir curves (CALIBRATION_DATAAMES)
	LADataHolder*				mpIRVols; // ir vols (DATA_MULTIREFERENCE)
	LADataHolder*				mpFXVols; // fx vols (DATA_MULTIREFERENCE)
	LADataHolder*				mpFX; // fx vols (DATA_REFERENCE)
	LADataHolder*				mpIRCurveTypes; // ir curvetypes (DATA_STRINGS)
	mutable std::map<LAString, LAInterpolationBase*> mLiborRateMap;
	mutable std::map<LAString, DoubleVector> mLiborGridTermMap;
};
