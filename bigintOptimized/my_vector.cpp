#include "my_vector.h"

my_vector::my_vector() : cnt_elements(0), is_small(true) {
    for (unsigned int &i : digit) {
        i = 0;
    }
}

my_vector::my_vector(int a): cnt_elements(1), is_small(true) {
    digit[0] = static_cast<uint32_t>(a);
}

my_vector::my_vector(size_t size, uint32_t el):cnt_elements(size) {
    if (size <= SMALL_SIZE) {
        is_small = true;
        for (size_t i = 0; i < size; ++i) {
            digit[i] = el;
        }
    } else {
        is_small = false;
        vec = std::make_shared<std::vector<uint32_t >>(size, el);
    }
}

size_t my_vector::size() const {
    return cnt_elements;
}

my_vector &my_vector::operator=(my_vector const &other) {
    if(!is_small){
        vec.reset();
    }
    if (other.is_small) {
        for (size_t i = 0; i < SMALL_SIZE; i++) {
            digit[i] = other.digit[i];
        }
    } else {
        new (&vec) std::shared_ptr<std::vector<uint32_t>>(other.vec);
    }
    is_small = other.is_small;
    cnt_elements = other.cnt_elements;
    return *this;

}

uint32_t& my_vector::operator[](size_t i) {
    copy();
    if (is_small) {
        return digit[i];

    } else {
        return vec->at(i);
    }
}

const uint32_t my_vector::operator[](size_t i) const {
    if (is_small) {
        if (i < SMALL_SIZE) {
            return digit[i];
        }
    } else {
        return vec->at(i);
    }
    return 0;
}

uint32_t &my_vector::back() {
    copy();
    if (is_small) {
        return digit[cnt_elements - 1];
    } else {
        return vec->at(cnt_elements - 1);
    }
}

const uint32_t my_vector::back() const {
    if (is_small) {
        return digit[cnt_elements - 1];
    } else {
        return vec->at(cnt_elements - 1);
    }
}

void my_vector::resize(size_t new_size) {
    if (new_size <= SMALL_SIZE) {
        if (is_small) {
            return;
        } else {
            is_small = true;
            auto a = std::shared_ptr<std::vector<uint32_t>>(vec);
            vec.reset();
            for (size_t i = 0; i < SMALL_SIZE; ++i) {
                digit[i] = a->at(i);
            }
        }
    } else {
        if (is_small) {
            is_small = false;
            std::shared_ptr<std::vector<uint32_t>> a = std::make_shared<std::vector<uint32_t>>(new_size, 0);
            for (size_t i = 0; i < SMALL_SIZE; ++i) {
                a->at(i) = digit[i];
            }

            new (&vec) std::shared_ptr<std::vector<uint32_t>>(a);


        } else {
            if (new_size <= vec->size()) {
                return;
            }
            vec->resize(new_size, 0);
        }
    }

}

void my_vector::push_back(uint32_t x) {
    resize(++cnt_elements);
    back() = x;
}


void my_vector::pop_back() {
    back() = 0;
    if(cnt_elements>0){
        --cnt_elements;
    }
}

void my_vector::copy() {
    if (is_small) {
        return;
    }
    if (cnt_elements <= SMALL_SIZE) {
        is_small = true;
        auto a = std::shared_ptr<std::vector<uint32_t>>(vec);
        vec.reset();
        for (size_t i = 0; i < SMALL_SIZE; ++i) {
            digit[i] = a->at(i);
        }
      //  a.reset();
    }else {
        std::vector<uint32_t > p (*vec);
        vec.reset();
        vec = std::make_shared<std::vector<uint32_t>>(p);
    }
}

my_vector::my_vector(my_vector const &other) {
    is_small = other.is_small;
    cnt_elements = other.cnt_elements;
    if (is_small) {
        for (size_t i = 0; i < SMALL_SIZE; i++) {
            (*this).digit[i] = other.digit[i];
        }
    } else {

        new (&vec) std::shared_ptr<std::vector<uint32_t>>(other.vec);
    }
}

my_vector::~my_vector() {
    if (!is_small) {
        vec.reset();
    }

}

void my_vector::resize(size_t new_size, bool f) {
    cnt_elements = new_size;
    resize(new_size);
}