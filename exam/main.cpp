#include <iostream>
#include "set_restore_guard.h"

int main() {
    int i =0;
    set_restore_guard<int> k;
    k.set(i,1);
    std:: cout << i <<' ';
    k.restore();
    std:: cout << i <<' ';
    return 0;
}