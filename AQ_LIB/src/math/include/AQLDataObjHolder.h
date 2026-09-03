#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include <cstdlib>
#include "AQLDataBase.h"



/*! 
    @brief Class to store classes (such as MDBase or derived class).
		   This class is used for the registration of database class.
		   There is ownership of the pointer if mDeleteFlag is true.
		   The default mDeleteFlag is false. it is able to be selected
		   by setDeleteFlag(bool).
*/
class MDObjHolder 
{
public:
//  LIFECYCLE
    // constructor
    explicit MDObjHolder(MDBase* e = NULL, bool deleteFlag=false);
    // copy constructor
    MDObjHolder(const MDObjHolder& objectHolder);
    // destructor
    ~MDObjHolder();

//  QUERY
    // check whethe define or not
    bool                isDefined() const {return mpObj != NULL;}
    // get MDBase, behavior when it is not define is undefined
    const MDBase&   getObj(void) const {return *mpObj;}
    // get MDBase, behavior when it is not define is undefined
    MDBase&     getObj(void) {return *mpObj;}
    // get data, behavior when it is not define is undefined
    Records_var         get(const AQLObjectHolder& objHolder) const {return mpObj->get(objHolder);}

//  OPERATION
    // set the data get class
    void                set(MDBase* base, bool deleteFlag=false);
    // set delete flag
    void                setDeleteFlag(bool);

//  OPERATER
    // copy operator
    MDObjHolder&        operator=(const MDObjHolder& objectHolder) 
                                        {return copy(objectHolder);}
    // relational operator thorough magnitude relationship of Name
	//behavior when it is not define is undefined
    bool                operator==(const MDObjHolder& objectHolder) const;
    bool                operator < (const MDObjHolder& objectHolder) const;


protected:
    // copy the class to get data
    MDObjHolder&        copy(const MDObjHolder& objectHolder);

private:
    mutable                 MDBase* mpObj;// pointer to get data class
    bool                    mDeleteFlag;// delete flag
};

