#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include <cstdlib>
#include "AQLDataBase.h"



/*! 
    @brief Class to store classes (such as AQLBase or derived class).
		   This class is used for the registration of database class.
		   There is ownership of the pointer if mDeleteFlag is true.
		   The default mDeleteFlag is false. it is able to be selected
		   by setDeleteFlag(bool).
*/
class AQLObjHolder 
{
public:
    // constructor
    explicit AQLObjHolder(AQLBase* e = NULL, bool deleteFlag=false);
    // copy constructor
    AQLObjHolder(const AQLObjHolder& objectHolder);
    // destructor
    ~AQLObjHolder();

    // check whethe define or not
    bool                isDefined() const {return mpObj != NULL;}
    // get AQLBase, behavior when it is not define is undefined
    const AQLBase&   getObj(void) const {return *mpObj;}
    // get AQLBase, behavior when it is not define is undefined
    AQLBase&     getObj(void) {return *mpObj;}
    // get data, behavior when it is not define is undefined
    Records_var         get(const AQLObjectHolder& objHolder) const {return mpObj->get(objHolder);}

    // set the data get class
    void                set(AQLBase* base, bool deleteFlag=false);
    // set delete flag
    void                setDeleteFlag(bool);

//  OPERATER
    // copy operator
    AQLObjHolder&        operator=(const AQLObjHolder& objectHolder) 
                                        {return copy(objectHolder);}
    // relational operator thorough magnitude relationship of Name
	//behavior when it is not define is undefined
    bool                operator==(const AQLObjHolder& objectHolder) const;
    bool                operator < (const AQLObjHolder& objectHolder) const;


protected:
    // copy the class to get data
    AQLObjHolder&        copy(const AQLObjHolder& objectHolder);

private:
    mutable                 AQLBase* mpObj;// pointer to get data class
    bool                    mDeleteFlag;// delete flag
};

