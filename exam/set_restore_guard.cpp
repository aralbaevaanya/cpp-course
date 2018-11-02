//
// Created by aralb on 02.11.2018.
//

#include "set_restore_guard.h"
template <typename T>
set_restore_guard<T>::set_restore_guard() : is_stores(false){}

template<typename T>
set_restore_guard<T>::set_restore_guard(T &var, const T new_val) :
        is_stores(true),
        var_ptr(&var)
{
    old_val = *var;
    var = new_val;
}

template <typename T>
set_restore_guard<T>::~set_restore_guard() = default;


template<typename T>
void set_restore_guard<T>::set(T &var, const T &new_val)
{
    is_stores = true;
    old_val = var;
    var = new_val;
    var_ptr = &var;
}
