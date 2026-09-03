#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// Funciton ID of LACompoundMethod
#define FN_COMPOUNDING	1300
// Function Name of LACompoundMethod
#define FN_COMPOUNDING_STR	"fn_compounding"

#define FN_COMPOUNDING1	1301
#define FN_COMPOUNDING1_STR	"fn_compounding1"
#define FN_COMPOUNDING2	1302
#define FN_COMPOUNDING2_STR	"fn_compounding2"
#define FN_COMPOUNDING3	1303
#define FN_COMPOUNDING3_STR	"fn_compounding3"
#define FN_COMPOUNDING4	1304
#define FN_COMPOUNDING4_STR	"fn_compounding4"
#define FN_COMPOUNDING5	1305
#define FN_COMPOUNDING5_STR	"fn_compounding5"
#define FN_COMPOUNDING6	1306
#define FN_COMPOUNDING6_STR	"fn_compounding6"
//for libor compounding
#define FN_COMPOUNDING7	1307
#define FN_COMPOUNDING7_STR	"fn_compounding7"
#define FN_COMPOUNDING8	1308
#define FN_COMPOUNDING8_STR	"fn_compounding8"
#define FN_COMPOUNDING9	1309
#define FN_COMPOUNDING9_STR	"fn_compounding9"
//for fed fund basis swap
#define FN_COMPOUNDING10 1310
#define FN_COMPOUNDING10_STR "fn_compounding10"

//for AlgoQuantHub
#define FN_COMPOUNDING11 1311
#define FN_COMPOUNDING11_STR "fn_compounding11"






///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent linear function.

	This class derives from AQLFunctionBase

*/
class LACompoundMethod : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LACompoundMethod();
	// destructor
	virtual ~LACompoundMethod();

	// assignment operator
	LACompoundMethod & operator=( const LACompoundMethod & ) { return *this; }


//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
								
	                            //==========================================
								// return function value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
								// return function value
	virtual double				operator()(double x) const
								{
									return AQLFunctionBase::operator()(x);								
								}


	// 20060929 override virtual function of base class
	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								};
private:

protected:
	virtual double getCompoundingCoeff(double coupon, double margin) const;
	virtual double getCompoundedVal(double coupon, double margin) const;
	virtual double getAdditionalVal(double coupon, double margin) const;
};

class LACompoundMethod1 : public LACompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	LACompoundMethod1();
	// destructor
	virtual ~LACompoundMethod1();
	LACompoundMethod1 & operator=( const LACompoundMethod1 & ) { return *this; }
//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
protected:
	virtual double getCompoundingCoeff(double coupon, double margin) const;
	virtual double getCompoundedVal(double coupon, double margin) const;
	virtual double getAdditionalVal(double coupon, double margin) const;
};

class LACompoundMethod2 : public LACompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	LACompoundMethod2();
	// destructor
	virtual ~LACompoundMethod2();
	LACompoundMethod2 & operator=( const LACompoundMethod2 & ) { return *this; }
//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
protected:
	virtual double getCompoundingCoeff(double coupon, double margin) const;
	virtual double getCompoundedVal(double coupon, double margin) const;
	virtual double getAdditionalVal(double coupon, double margin) const;
};

class LACompoundMethod3 : public LACompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	LACompoundMethod3();
	// destructor
	virtual ~LACompoundMethod3();
	LACompoundMethod3 & operator=( const LACompoundMethod3 & ) { return *this; }
//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
protected:
	virtual double getCompoundingCoeff(double coupon, double margin) const;
	virtual double getCompoundedVal(double coupon, double margin) const;
	virtual double getAdditionalVal(double coupon, double margin) const;
};

class LACompoundMethod4 : public LACompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	LACompoundMethod4();
	// destructor
	virtual ~LACompoundMethod4();
	LACompoundMethod4 & operator=( const LACompoundMethod4 & ) { return *this; }
//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
protected:
	virtual double getCompoundingCoeff(double coupon, double margin) const;
	virtual double getCompoundedVal(double coupon, double margin) const;
	virtual double getAdditionalVal(double coupon, double margin) const;
};

class LACompoundMethod5 : public LACompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	LACompoundMethod5();
	// destructor
	virtual ~LACompoundMethod5();
	LACompoundMethod5 & operator=( const LACompoundMethod5 & ) { return *this; }
//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
protected:
	virtual double getCompoundingCoeff(double coupon, double margin) const;
	virtual double getCompoundedVal(double coupon, double margin) const;
	virtual double getAdditionalVal(double coupon, double margin) const;
};

class LACompoundMethod6 : public LACompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	LACompoundMethod6();
	// destructor
	virtual ~LACompoundMethod6();
	LACompoundMethod6 & operator=( const LACompoundMethod6 & ) { return *this; }
//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
protected:
	virtual double getCompoundingCoeff(double coupon, double margin) const;
	virtual double getCompoundedVal(double coupon, double margin) const;
	virtual double getAdditionalVal(double coupon, double margin) const;
};

//Compounding (ISDA)
class LACompoundMethod7 : public LACompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	LACompoundMethod7();
	// destructor
	virtual ~LACompoundMethod7();
	LACompoundMethod7 & operator=( const LACompoundMethod7 & ) { return *this; }
//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;

		                            //==========================================
								// return function value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
								// return function value
	virtual double				operator()(double x) const
								{
									return AQLFunctionBase::operator()(x);								
								}

	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								};
protected:
};


//Flat Compounding (ISDA)
class LACompoundMethod8 : public LACompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	LACompoundMethod8();
	// destructor
	virtual ~LACompoundMethod8();
	LACompoundMethod8 & operator=( const LACompoundMethod8 & ) { return *this; }
//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;

		                            //==========================================
								// return function value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
								// return function value
	virtual double				operator()(double x) const
								{
									return AQLFunctionBase::operator()(x);								
								}

	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								};
protected:
};

//Compounding with Simple Spread (non-ISDA))
class LACompoundMethod9 : public LACompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	LACompoundMethod9();
	// destructor
	virtual ~LACompoundMethod9();
	LACompoundMethod9 & operator=( const LACompoundMethod9 & ) { return *this; }
//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;

		                            //==========================================
								// return function value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
								// return function value
	virtual double				operator()(double x) const
								{
									return AQLFunctionBase::operator()(x);								
								}

	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								};
protected:
};

class LACompoundMethod10 : public LACompoundMethod
{
public:
	LACompoundMethod10() : LACompoundMethod() {}
	virtual ~LACompoundMethod10() {}
	LACompoundMethod10& operator=(const LACompoundMethod10&) { return *this; }

	virtual bool isTypeOf(function_t id) const;
	virtual AQLCoreFunctionBase* clone() const;
	virtual function_t getType() const;
	virtual double operator()(const DoubleArray& x) const;
	virtual double operator()(double x) const { return AQLFunctionBase::operator ()(x); }
	virtual AQLFunctionBase& operator()(unsigned int pos, const DoubleArray& x) { return AQLFunctionBase::operator ()(pos, x); }
};

class LACompoundMethod11 : public LACompoundMethod
{
public:
	LACompoundMethod11() : LACompoundMethod() {}
	virtual ~LACompoundMethod11() {}
	LACompoundMethod11& operator=(const LACompoundMethod11&) { return *this; }

	virtual bool isTypeOf(function_t id) const;
	virtual AQLCoreFunctionBase* clone() const;
	virtual function_t getType() const;
	virtual double operator()(const DoubleArray& x) const;
	virtual double operator()(double x) const { return AQLFunctionBase::operator ()(x); }
	virtual AQLFunctionBase& operator()(unsigned int pos, const DoubleArray& x) { return AQLFunctionBase::operator ()(pos, x); }
};

