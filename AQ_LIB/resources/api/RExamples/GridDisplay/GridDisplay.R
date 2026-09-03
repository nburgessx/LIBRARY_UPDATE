# MLIB test yield curnve 

#   E:/MLIB/MLIBQ_DEV1/target2017/32/ReleaseR/MLIB_CLIENT_API/out/DLL

mlib_test_yc<-function(){
  
  mlib_source_lib = "E:/MLIB/MLIBQ_DEV1/target2017/32/ReleaseR/MLIB_CLIENT_API/out/DLL/MLIB_CLIENT_API.dll"
  mlib_source = "E:/MLIB/MLIBQ_DEV1/target2017/32/ReleaseR/MLIB_CLIENT_API/out/DLL/MLIB_CLIENT_API.R"
  dyn.load(mlib_source_lib)
  source(mlib_source)
  
  setUpMLIB("","")
  
  meUtilityVersion()
  
  mlib_curve = meLWOLoad("E:/MLIB/MLIBQ_DEV1/target2017/32/ReleaseR/EURYC_EUR3ML.JSON")
  mlib_curve
  
  meLWOGridDisplay(mlib_curve)
  

}