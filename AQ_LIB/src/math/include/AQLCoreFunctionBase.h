/*! @file
    @brief Class declaration of virtual base class for valuation, intepolation, procedure etc.
*/


#ifndef AQLCoreFunctionBase_h
#define AQLCoreFunctionBase_h

#ifdef __GNUG__
#pragma interface
#endif

// define function_t which represents type of Function ID
#define function_t int

// AQLCoreFunctionBase Function ID
#define FN_BASE     0


/*! 
    @brief Virtual base class for valuation, intepolation, procedure etc.

	This class defineds a simple base class. This object is not allowded to 
	hold members except Function ID. Function ID must be issued of every sub class
*/
class AQLCoreFunctionBase 
{
public:
    // default constructor
    AQLCoreFunctionBase(void);
    // destructor
    virtual ~AQLCoreFunctionBase();

    // check whether this class derives from base class with type id
    virtual bool                isTypeOf(function_t id) const; 

    // deep copy of this object
    /*!
        @return the copied object
    */
    virtual AQLCoreFunctionBase*     clone() const =0;

    // return Function ID of this class
    virtual function_t          getType() const;

    // relational operator thorough magnitude relationship of FunctionID
    bool                        operator<(const AQLCoreFunctionBase& e) const
                                { return getType() < e.getType();}
    // relational operator thorough magnitude relationship of FunctionID
    bool                        operator==(const AQLCoreFunctionBase& e) const
                                { return getType() == e.getType();}
private:
    // assignment operator
    AQLCoreFunctionBase&             operator=(const AQLCoreFunctionBase& e);

protected:
    // copy constructor
    AQLCoreFunctionBase(const AQLCoreFunctionBase& e);
};

#endif
