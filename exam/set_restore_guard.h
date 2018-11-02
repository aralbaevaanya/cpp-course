//
// Created by aralb on 02.11.2018.
//

#ifndef EXAM_SET_RESTORE_GUARD_H
#define EXAM_SET_RESTORE_GUARD_H


#include <assert.h>

template<typename T>
struct set_restore_guard
{
    set_restore_guard();

    set_restore_guard(T& var, T new_val);

    ~set_restore_guard();

    void set(T& var, T const& new_val);
    void restore()
    {
        assert(is_stores);
        *var_ptr = old_val;
        is_stores = false;
    }

    explicit operator bool() const
    {
        return is_stores;
    }

private:
    T old_val;
    T * var_ptr = nullptr;
    bool is_stores;
};




#endif //EXAM_SET_RESTORE_GUARD_H
