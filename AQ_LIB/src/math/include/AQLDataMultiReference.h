/*! @file
    @brief Class declaration of data to refer multi entities.
	
*/


#ifndef AQLDataMultiReference_h
#define AQLDataMultiReference_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceDataType.h"
#include "AQLCoreAppError.h"

#define DATA_MULTIREFERENCE  33  // AQLDataMultiReference Data ID

class AQLObjectHolder;

/*! 
    @brief MultiReference Data class to represent the data (type multireference).
    
*/
class AQLDataMultiReference : public AQLPriceDataType
{
public:
//	LIFECYCLE
    // default constructor
	AQLDataMultiReference(void);
    // copy constructor
    AQLDataMultiReference(const AQLDataMultiReference& attr);
    // destructor
    virtual ~AQLDataMultiReference(void);

//  QUERY
    // deep copy of the data object
    virtual AQLPriceDataType*	clone() const; 

    // return string representation(separated by a delimiter string referenced by multiple Object name)
    virtual AQLString		convertToString(void) const;

     // return the number of referenced Entities
    /*!
        @return the number of referenced Entities
    */
	unsigned int			getSize() const {return mReferences.size();}

    // return the referenced Object selced by index(starting at 0)
	AQLObjectHolder&			get(int i) const;

//  OPERATOR
    // set the reference from string representation(separated by a delimiter string referenced by multiple Object name)
    virtual void            convertFromString(const AQLString& str);

    // add the referenced Object
	void                    add(AQLObjectHolder *e);

    // remove the Object specified from the referenced Object
	void                    remove(const AQLString& object);

protected:
   // the contents of the object to be initialized with those of another object
	virtual AQLPriceDataType&    assignment(const AQLPriceDataType& a);

    // compare the contents against the other objecet
	virtual int             compare(const AQLPriceDataType& a) const;

    //	set a pointer to the Holder of the Data
	virtual void			setHolder(AQLDataHolder* holder);

    // inform the Object Data that the Data has been updated
	virtual void			update(void);

private:
//not implemented
//#ifdef _MSC_VER
//	void					set(const std::vector<AQLString>& names);
//#else
//	void					set(const vector<AQLString>& names);
//#endif

    // set the referenced Object Data from the member variables and member variables of the specified Object
    inline void				setReferencee(AQLObjectHolder* objHolder, AQLObject* e);
    // delete the referenced Object Data from the member variables and member variables of the specified Object
    inline void				delReferencee(AQLObjectHolder* objHolder, AQLObject* e);
    // register the referenced Enity into ReferencePool, and get the Object(Holder)
    AQLObjectHolder*			getReferencee(const AQLString& name, AQLObject* e);


	// set member functions(mReferencees) and also set referenced Object to the Data member function based on mRefNames
	inline void				setReferencer(void);
//not implemented	inline void				delReferencer(AQLObjectHolder* e);
    // remove all entitites from referenced list
	inline void				delReferencer(void);

	std::vector<AQLObjectHolder*>		mReferences; // (multi)referenced Object Holder
	std::vector<AQLString>				mRefNames;   // (multi) referenced Object name
};
#endif
