/*! @file
    @brief LACalibrateModel manager class 
*/
//  2007, Mizuho International London.
#ifndef LAModelConfiguration_h
#define LAModelConfiguration_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAModelConfiguration.h
//
//  DESCRIPTION :       LAModelConfiguration 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif



class LAString;
class LACoreDataService;
class LACalibrateModel;
//===================== Class Declare LAModelConfiguration==================================
/*! 
    @brief SDE manager class

    This class is singleton
*/
class LAModelConfiguration
{
public:
	//==============================================
	// return singleton 
	static LAModelConfiguration     *getInstance();
	//==============================================
	// create LACalibrateModel
	LACalibrateModel *createSDEGenerator(const LAString &model) const;

private:

friend class LACoreDataService;
	// constructor
	LAModelConfiguration(void);
	// destructor
	~LAModelConfiguration(void);
	// copy constructor
	LAModelConfiguration(const LAModelConfiguration &rhs);
	LAModelConfiguration &operator=(const LAModelConfiguration &rhs);

	static LAModelConfiguration *mpInstance; // single instance

};

#endif
