#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "LAMathIndexEntity.h"


//// DEFINES ////
// Entitiy id of LAMathIndexEntityInterpolation
#define ENTITY_INDEX 11


//#ifndef IR_MODEL_DATA_INDEXTYPE
//#define IR_MODEL_DATA_INDEXTYPE				"IndexType"				//  data name of index type



/*! 
    @brief Class to represent index.
*/
class LAMathIndexEntityInterpolation : public LAMathIndexEntity
{
public:

// LIFECYCLE
    // default constructor
	LAMathIndexEntityInterpolation(AQLDataInstance* dataInstance);
    // copy constructor
	LAMathIndexEntityInterpolation(const LAMathIndexEntityInterpolation& irse);
    // destructor
	virtual ~LAMathIndexEntityInterpolation();

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
