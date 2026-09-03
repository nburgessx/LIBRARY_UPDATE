/*! @file
    @brief Class declaration of virtual base class for valuation, intepolation, procedure etc.

*/


#ifndef LACoreFunctionBase_h
#define LACoreFunctionBase_h

#ifdef __GNUG__
#pragma interface
#endif

// define function_t which represents type of Function ID
#define function_t int

// LACoreFunctionBase Function ID
#define FN_BASE     0


/*! 
    @brief Virtual base class for valuation, intepolation, procedure etc.

	This class defineds a simple base class. This object is not allowded to 
	hold members except Function ID. Function ID must be issued of every sub class
*/
class LACoreFunctionBase 
{
public:
//  LIFECYCLE
    // default constructor
    LACoreFunctionBase(void);
    // destructor
    virtual ~LACoreFunctionBase();

//  QUERY
    // check whether this class derives from base class with type id
    virtual bool                isTypeOf(function_t id) const; 

    // deep copy of this object
    /*!
        @return the copied object
    */
    virtual LACoreFunctionBase*     clone() const =0;

    // return Function ID of this class
    virtual function_t          getType() const;

// OPERATOR
    // relational operator thorough magnitude relationship of FunctionID
    bool                        operator<(const LACoreFunctionBase& e) const
                                { return getType() < e.getType();}
    // relational operator thorough magnitude relationship of FunctionID
    bool                        operator==(const LACoreFunctionBase& e) const
                                { return getType() == e.getType();}
private:
    // assignment operator
    LACoreFunctionBase&             operator=(const LACoreFunctionBase& e);

protected:
    // copy constructor
    LACoreFunctionBase(const LACoreFunctionBase& e);
};

#endif
