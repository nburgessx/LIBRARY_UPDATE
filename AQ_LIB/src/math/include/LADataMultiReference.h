/*! @file
    @brief Class declaration of data to refer multi entities.
	
*/


#ifndef LADataMultiReference_h
#define LADataMultiReference_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceDataType.h"
#include "LACoreAppError.h"

#define DATA_MULTIREFERENCE  33  // LADataMultiReference Data ID

class LAObjectHolder;

/*! 
    @brief MultiReference Data class to represent the data (type multireference).
    
*/
class LADataMultiReference : public LAPriceDataType
{
public:
//	LIFECYCLE
    // default constructor
	LADataMultiReference(void);
    // copy constructor
    LADataMultiReference(const LADataMultiReference& attr);
    // destructor
    virtual ~LADataMultiReference(void);

//  QUERY
    // deep copy of the data object
    virtual LAPriceDataType*	clone() const; 

    // return string representation(separated by a delimiter string referenced by multiple Object name)
    virtual LAString		convertToString(void) const;

     // return the number of referenced Entities
    /*!
        @return the number of referenced Entities
    */
	unsigned int			getSize() const {return mReferences.size();}

    // return the referenced Object selced by index(starting at 0)
	LAObjectHolder&			get(int i) const;

//  OPERATOR
    // set the reference from string representation(separated by a delimiter string referenced by multiple Object name)
    virtual void            convertFromString(const LAString& str);

    // add the referenced Object
	void                    add(LAObjectHolder *e);

    // remove the Object specified from the referenced Object
	void                    remove(const LAString& object);

protected:
   // the contents of the object to be initialized with those of another object
	virtual LAPriceDataType&    assignment(const LAPriceDataType& a);

    // compare the contents against the other objecet
	virtual int             compare(const LAPriceDataType& a) const;

    //	set a pointer to the Holder of the Data
	virtual void			setHolder(LADataHolder* holder);

    // inform the Object Data that the Data has been updated
	virtual void			update(void);

private:
//not implemented
//#ifdef _MSC_VER
//	void					set(const std::vector<LAString>& names);
//#else
//	void					set(const vector<LAString>& names);
//#endif

    // set the referenced Object Data from the member variables and member variables of the specified Object
    inline void				setReferencee(LAObjectHolder* objHolder, LAObject* e);
    // delete the referenced Object Data from the member variables and member variables of the specified Object
    inline void				delReferencee(LAObjectHolder* objHolder, LAObject* e);
    // register the referenced Enity into ReferencePool, and get the Object(Holder)
    LAObjectHolder*			getReferencee(const LAString& name, LAObject* e);


	// set member functions(mReferencees) and also set referenced Object to the Data member function based on mRefNames
	inline void				setReferencer(void);
//not implemented	inline void				delReferencer(LAObjectHolder* e);
    // remove all entitites from referenced list
	inline void				delReferencer(void);

	std::vector<LAObjectHolder*>		mReferences; // (multi)referenced Object Holder
	std::vector<LAString>				mRefNames;   // (multi) referenced Object name
};
#endif
