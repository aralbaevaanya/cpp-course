#ifndef BIG_INTEGER_MYVECTOR_H
#define BIG_INTEGER_MYVECTOR_H

#include <cstdint>
#include <vector>
#include <memory>


const size_t SMALL_SIZE = 4;
struct my_vector
{
    my_vector();
    explicit my_vector(int a);
    explicit my_vector(size_t size, uint32_t el );
    my_vector(my_vector const& other);
    ~my_vector();

    size_t size() const;
    my_vector& operator=(my_vector const& other);
    uint32_t& operator[](size_t i);
    const uint32_t operator[](size_t i) const;
    uint32_t& back();
    const uint32_t back() const;
    void resize(size_t new_size);
    void resize(size_t new_size,bool f);
    void push_back(uint32_t x);
    void pop_back();

   // friend big_integer&::operator*=(big_integer const& rhs);

private:
    size_t cnt_elements;
    bool is_small = true;

    union {
        uint32_t digit[SMALL_SIZE];
        std::shared_ptr<std::vector<uint32_t>> vec;
    };

    void copy();
};


#endif //BIG_INTEGER_MYVECTOR_H