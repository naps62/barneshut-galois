#ifndef ___NINJA_H___
#define ___NINJA_H___

#include <functional>

/**	Converts an object's reference to a pointer.
 */
template<typename T>
struct Deref : public std::unary_function<T, T*> {
  T* operator()(T& item) const { return &item; }
};

#endif//___NINJA_H___