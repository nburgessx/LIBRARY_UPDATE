/*! @file
    @brief Class declaration for smart pointer.
*/


// ---------------------------------------------------------------------------
// 06MAR06 ysuzuki : The specification change of AQLAutoPt(modified to implement in the specification of "Version 3" from 
//					 what has been implemented in "Version 1".
//                   For more information, web page of "More Effective C + +",
//                   http://www.awprofessional.com/content/images/020163371X/autoptr/std::shared_ptr.html
//                   In addition, see also (memory file for C + + standard header files) 
//					 and implementation of std::shared_ptr class of gcc3.4.x VC + +7.

#ifndef AQLCoreAutoPtr_h
#define AQLCoreAutoPtr_h
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning( disable : 4512 )
#endif

#include <cstdio>


#ifdef _MSC_VER 
template<class T>
class AQLCoreAutoPtr;

template<class T>
struct AQLCoreAutoPtrRef {   
    AQLCoreAutoPtrRef(AQLCoreAutoPtr<T>& rhs) : mRef(rhs) {}
    AQLCoreAutoPtr<T>& mRef; 
};
#else 
// for gcc
template<class U>
struct AQLCoreAutoPtrRef {
    U* mpPtr;
    explicit AQLCoreAutoPtrRef(U *p): mpPtr(p) {}
};
#endif

/*! 
    @brief Class to realize smart pointer.
*/
template<class T>
class AQLCoreAutoPtr
{
public:
//  LIMBCYCLE
    // constructor
    explicit AQLCoreAutoPtr(T *p = NULL) : mpPtr(p) {}
#ifdef _MSC_VER 
    // copy constructor
    AQLCoreAutoPtr(AQLCoreAutoPtr<T>& rhs) : mpPtr(rhs.release()) {}

    // copy constructor
    AQLCoreAutoPtr(AQLCoreAutoPtrRef<T> rhs) : mpPtr(rhs.mRef.release()) {}
#else 
    // copy constructor
    AQLCoreAutoPtr(AQLCoreAutoPtr& rhs) : mpPtr(rhs.release()) {}

    // copy constructor
    template<class U>
    AQLCoreAutoPtr(AQLCoreAutoPtr<U>& rhs) : mpPtr(rhs.release()) {}
#endif
    // destructor    
    /*!
        release the memory reserved
    */
    ~AQLCoreAutoPtr(void) 
    { 
        delete mpPtr;
    }

    // get a pointer to keep
    /*!
        @return pointer to keep
    */
    T* get(void) const 
    {
        return mpPtr;
    }

//  OPERATION
    // set the NULL to the pointer to the data
    /*!
        @return pointer before setting NULL
    */
    T* release(void)
    {
        T* oldPtr = mpPtr;
        mpPtr = NULL;
        return oldPtr;
    }

    // change the pointer to the data
    /*!
        current memory will be freed
        @param[in] p pointer after the change
    */
    void reset(T * p=NULL)
    {
        if (mpPtr != p) 
        {
            delete mpPtr;
            mpPtr = p;
        }
    }
    
//  OPERATORS
#ifdef _MSC_VER 
    // assignment operator
    AQLCoreAutoPtr<T>& operator=(AQLCoreAutoPtr<T>& rhs) 
    {
        reset(rhs.release());
        return *this;
    }

    // assignment operator
    AQLCoreAutoPtr<T>& operator=(AQLCoreAutoPtrRef<T>& rhs)
    {
        reset(rhs.mRef.release());
        return *this;
    }
#else 
    // assignment operator
    AQLCoreAutoPtr& operator=(AQLCoreAutoPtr& rhs) 
    {
        reset(rhs.release());
        return *this;
    }

    // assignment operator
    template<class U>
    AQLCoreAutoPtr& operator=(AQLCoreAutoPtr<U>& rhs) 
    {
        reset(rhs.release());
        return *this;
    }
#endif

    // static cast
    T& operator*() const 
    {
        return *mpPtr;
    }

    // reference
    T* operator->() const 
    {
        return mpPtr;
    }

#ifdef _MSC_VER
    template<class U>
    operator AQLCoreAutoPtr<U>() { return (AQLCoreAutoPtr<U>(*this)); }

    template<class U>
    operator AQLCoreAutoPtrRef<U>() { return (AQLCoreAutoPtrRef<U>(*this)); }

    template<class U>
    AQLCoreAutoPtr<T>& operator=(AQLCoreAutoPtr<U>& rhs){
        reset(rhs.release());
        return (*this);
    }

    template<class U>
    AQLCoreAutoPtr(AQLCoreAutoPtr<U>& rhs) : mpPtr(rhs.release()) {}

#else
    // for gcc
    AQLCoreAutoPtr(AQLCoreAutoPtrRef<T> ref) : mpPtr(ref.mpPtr) {}
      
    AQLCoreAutoPtr& operator=(AQLCoreAutoPtrRef<T> ref)
    {
      if (ref.mpPtr != this->get()){
        delete mpPtr;
        mpPtr = ref.mpPtr;
      }
      return *this;
    }
      
    template<class U>
    operator AQLCoreAutoPtrRef<U>(){ return AQLCoreAutoPtrRef<U>(this->release()); }

    template<class U>
    operator AQLCoreAutoPtr<U>(){ return AQLCoreAutoPtr<U>(this->release()); }
      
#endif


private:
    T* mpPtr;   // pointer to data
};

#endif
