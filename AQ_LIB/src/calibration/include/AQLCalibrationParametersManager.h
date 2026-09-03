#pragma once

#ifdef __GNUG__
#pragma interface
#endif




class AQLString;
class AQLCoreDataService;
class AQLCalibrationParameters;
//===================== Class Declare AQLCalibrationParametersManager==================================
/*! 
    @brief AQLCalibrationParameters manager class

    This class is singleton
*/
class AQLCalibrationParametersManager
{
public:
	//==============================================
	// return singleton 
	static AQLCalibrationParametersManager     *getInstance();
	//==============================================
	// create AQLCalibrationParameters
	AQLCalibrationParameters *createCalibInfoCreator(const AQLString &model) const;

private:

friend class AQLCoreDataService;
	// constructor
	AQLCalibrationParametersManager(void);
	// destructor
	~AQLCalibrationParametersManager(void);
	// copy constructor
	AQLCalibrationParametersManager(const AQLCalibrationParametersManager &rhs);
	AQLCalibrationParametersManager &operator=(const AQLCalibrationParametersManager &rhs);

	static AQLCalibrationParametersManager *mpInstance; // single instance

};
