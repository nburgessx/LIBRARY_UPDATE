#pragma once

#ifdef __GNUG__
#pragma interface
#endif




class LAString;
class LACoreDataService;
class LACalibrationParameters;
//===================== Class Declare LACalibrationParametersManager==================================
/*! 
    @brief LACalibrationParameters manager class

    This class is singleton
*/
class LACalibrationParametersManager
{
public:
	//==============================================
	// return singleton 
	static LACalibrationParametersManager     *getInstance();
	//==============================================
	// create LACalibrationParameters
	LACalibrationParameters *createCalibInfoCreator(const LAString &model) const;

private:

friend class LACoreDataService;
	// constructor
	LACalibrationParametersManager(void);
	// destructor
	~LACalibrationParametersManager(void);
	// copy constructor
	LACalibrationParametersManager(const LACalibrationParametersManager &rhs);
	LACalibrationParametersManager &operator=(const LACalibrationParametersManager &rhs);

	static LACalibrationParametersManager *mpInstance; // single instance

};
