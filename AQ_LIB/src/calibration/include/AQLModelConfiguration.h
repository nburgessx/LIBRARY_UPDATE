/*! @file
    @brief AQLCalibrateModel manager class 
*/
#ifndef AQLModelConfiguration_h
#define AQLModelConfiguration_h
#ifdef __GNUG__
#pragma interface
#endif



class AQLString;
class AQLCoreDataService;
class AQLCalibrateModel;
//===================== Class Declare AQLModelConfiguration==================================
/*! 
    @brief SDE manager class

    This class is singleton
*/
class AQLModelConfiguration
{
public:
	//==============================================
	// return singleton 
	static AQLModelConfiguration     *getInstance();
	//==============================================
	// create AQLCalibrateModel
	AQLCalibrateModel *createSDEGenerator(const AQLString &model) const;

private:

friend class AQLCoreDataService;
	// constructor
	AQLModelConfiguration(void);
	// destructor
	~AQLModelConfiguration(void);
	// copy constructor
	AQLModelConfiguration(const AQLModelConfiguration &rhs);
	AQLModelConfiguration &operator=(const AQLModelConfiguration &rhs);

	static AQLModelConfiguration *mpInstance; // single instance

};

#endif
