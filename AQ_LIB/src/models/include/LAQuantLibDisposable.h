#pragma once

template <class T>
class LAQuantLibDisposable : public T {
  public:
	LAQuantLibDisposable(T& t);
	LAQuantLibDisposable(const LAQuantLibDisposable<T>& t);
	LAQuantLibDisposable<T>& operator=(const LAQuantLibDisposable<T>& t);
};


// inline definitions

template <class T>
inline LAQuantLibDisposable<T>::LAQuantLibDisposable(T& t) {
	this->swap(t);
}

template <class T>
inline LAQuantLibDisposable<T>::LAQuantLibDisposable(const LAQuantLibDisposable<T>& t) : T() {
	this->swap(const_cast<LAQuantLibDisposable<T>&>(t));
}

template <class T>
inline LAQuantLibDisposable<T>& LAQuantLibDisposable<T>::operator=(const LAQuantLibDisposable<T>& t) {
	this->swap(const_cast<LAQuantLibDisposable<T>&>(t));
	return *this;
}
