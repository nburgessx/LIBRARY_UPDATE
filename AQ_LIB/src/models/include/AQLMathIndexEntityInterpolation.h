#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLMathIndexEntity.h"


//// DEFINES ////
// Entitiy id of AQLMathIndexEntityInterpolation
#define ENTITY_INDEX 11


//#ifndef IR_MODEL_DATA_INDEXTYPE
//#define IR_MODEL_DATA_INDEXTYPE				"IndexType"				//  data name of index type



/*! 
    @brief Class to represent index.
*/
class AQLMathIndexEntityInterpolation : public AQLMathIndexEntity
{
public:

// LIFECYCLE
    // default constructor
	AQLMathIndexEntityInterpolation(AQLDataInstance* dataInstance);
    // copy constructor
	AQLMathIndexEntityInterpolation(const AQLMathIndexEntityInterpolation& irse);
    // destructor
	virtual ~AQLMathIndexEntityInterpolation();

	// get index 
	AQLObject*			clone() const;// %%% COVARIANT RETURN %%%

protected:
	// copy index object	 
	virtual AQLObject&	copy(const AQLObject& e);
	// calculate index
	virtual	void		calcIndex();	
	// set up this class for index calculation
	virtual	void		setUp(void);

private:
    //
    void convertTermtoDate(DateVector& dateVec, DoubleArray& termVec);
    virtual void calcInterCache();

	DoubleArray                 mInterCache;
    DoubleArray                 mT_Canonic;
    DateVector                  mDate_Canonic;

    DoubleMatrix                mIndex_TenorGrid;
    DoubleMatrix                mIndex_TermGrid;
};
