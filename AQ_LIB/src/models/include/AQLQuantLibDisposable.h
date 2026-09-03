#pragma once

template <class T>
class AQLQuantLibDisposable : public T {
  public:
	AQLQuantLibDisposable(T& t);
	AQLQuantLibDisposable(const AQLQuantLibDisposable<T>& t);
	AQLQuantLibDisposable<T>& operator=(const AQLQuantLibDisposable<T>& t);
};


// inline definitions

template <class T>
inline AQLQuantLibDisposable<T>::AQLQuantLibDisposable(T& t) {
	this->swap(t);
}

template <class T>
inline AQLQuantLibDisposable<T>::AQLQuantLibDisposable(const AQLQuantLibDisposable<T>& t) : T() {
	this->swap(const_cast<AQLQuantLibDisposable<T>&>(t));
}

template <class T>
inline AQLQuantLibDisposable<T>& AQLQuantLibDisposable<T>::operator=(const AQLQuantLibDisposable<T>& t) {
	this->swap(const_cast<AQLQuantLibDisposable<T>&>(t));
	return *this;
}
