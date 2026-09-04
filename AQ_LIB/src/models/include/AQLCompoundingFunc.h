#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// Funciton ID of AQLCompoundMethod
#define FN_COMPOUNDING	1300
// Function Name of AQLCompoundMethod
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

#define FN_COMPOUNDING11 1311
#define FN_COMPOUNDING11_STR "fn_compounding11"






///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent linear function.

	This class derives from AQLFunctionBase

*/
class AQLCompoundMethod : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	AQLCompoundMethod();
	// destructor
	virtual ~AQLCompoundMethod();

	// assignment operator
	AQLCompoundMethod & operator=( const AQLCompoundMethod & ) { return *this; }


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

class AQLCompoundMethod1 : public AQLCompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	AQLCompoundMethod1();
	// destructor
	virtual ~AQLCompoundMethod1();
	AQLCompoundMethod1 & operator=( const AQLCompoundMethod1 & ) { return *this; }
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

class AQLCompoundMethod2 : public AQLCompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	AQLCompoundMethod2();
	// destructor
	virtual ~AQLCompoundMethod2();
	AQLCompoundMethod2 & operator=( const AQLCompoundMethod2 & ) { return *this; }
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

class AQLCompoundMethod3 : public AQLCompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	AQLCompoundMethod3();
	// destructor
	virtual ~AQLCompoundMethod3();
	AQLCompoundMethod3 & operator=( const AQLCompoundMethod3 & ) { return *this; }
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

class AQLCompoundMethod4 : public AQLCompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	AQLCompoundMethod4();
	// destructor
	virtual ~AQLCompoundMethod4();
	AQLCompoundMethod4 & operator=( const AQLCompoundMethod4 & ) { return *this; }
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

class AQLCompoundMethod5 : public AQLCompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	AQLCompoundMethod5();
	// destructor
	virtual ~AQLCompoundMethod5();
	AQLCompoundMethod5 & operator=( const AQLCompoundMethod5 & ) { return *this; }
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

class AQLCompoundMethod6 : public AQLCompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	AQLCompoundMethod6();
	// destructor
	virtual ~AQLCompoundMethod6();
	AQLCompoundMethod6 & operator=( const AQLCompoundMethod6 & ) { return *this; }
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
class AQLCompoundMethod7 : public AQLCompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	AQLCompoundMethod7();
	// destructor
	virtual ~AQLCompoundMethod7();
	AQLCompoundMethod7 & operator=( const AQLCompoundMethod7 & ) { return *this; }
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
class AQLCompoundMethod8 : public AQLCompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	AQLCompoundMethod8();
	// destructor
	virtual ~AQLCompoundMethod8();
	AQLCompoundMethod8 & operator=( const AQLCompoundMethod8 & ) { return *this; }
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
class AQLCompoundMethod9 : public AQLCompoundMethod
{
public:
//  LIFECYCLE
	// constructor
	AQLCompoundMethod9();
	// destructor
	virtual ~AQLCompoundMethod9();
	AQLCompoundMethod9 & operator=( const AQLCompoundMethod9 & ) { return *this; }
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

class AQLCompoundMethod10 : public AQLCompoundMethod
{
public:
	AQLCompoundMethod10() : AQLCompoundMethod() {}
	virtual ~AQLCompoundMethod10() {}
	AQLCompoundMethod10& operator=(const AQLCompoundMethod10&) { return *this; }

	virtual bool isTypeOf(function_t id) const;
	virtual AQLCoreFunctionBase* clone() const;
	virtual function_t getType() const;
	virtual double operator()(const DoubleArray& x) const;
	virtual double operator()(double x) const { return AQLFunctionBase::operator ()(x); }
	virtual AQLFunctionBase& operator()(unsigned int pos, const DoubleArray& x) { return AQLFunctionBase::operator ()(pos, x); }
};

class AQLCompoundMethod11 : public AQLCompoundMethod
{
public:
	AQLCompoundMethod11() : AQLCompoundMethod() {}
	virtual ~AQLCompoundMethod11() {}
	AQLCompoundMethod11& operator=(const AQLCompoundMethod11&) { return *this; }

	virtual bool isTypeOf(function_t id) const;
	virtual AQLCoreFunctionBase* clone() const;
	virtual function_t getType() const;
	virtual double operator()(const DoubleArray& x) const;
	virtual double operator()(double x) const { return AQLFunctionBase::operator ()(x); }
	virtual AQLFunctionBase& operator()(unsigned int pos, const DoubleArray& x) { return AQLFunctionBase::operator ()(pos, x); }
};

