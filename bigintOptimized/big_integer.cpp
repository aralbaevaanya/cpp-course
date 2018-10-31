#include "big_integer.h"
#include <cstring>
#include <algorithm>
#include <iostream>

const uint64_t BASE = 4294967296;
const size_t LENGTH = 32;

big_integer::big_integer() : sign(0) {}

big_integer::big_integer(big_integer const &other) : sign(other.sign), digits(other.digits) {}

big_integer::big_integer(int a) {
    sign = (a == 0 ? 0 : ((a > 0) ? 1 : -1));
    if (a != 0) {
        if (a == INT32_MIN) {
            digits.push_back(static_cast<uint32_t >(INT32_MAX) + 1);
        } else {
            digits.push_back(static_cast<uint32_t>(abs(a)));
        }
    }

}

big_integer::big_integer(uint64_t a) {
    if (a == 0) {
        sign = 0;
    } else {
        sign = 1;
        uint32_t first = static_cast<uint32_t>(a >> 32);
        uint32_t second = static_cast<uint32_t>(a - first);
        if (first > 0) {
            digits.push_back(first);
        }
        digits.push_back(second);
    }
}

big_integer::big_integer(std::string const &str) {
    sign = 0;
    for (size_t i = (str[0] == '-' ? 1 : 0); i < str.length(); ++i) {
        mul(*this, 10);
        *this += (int32_t) (str[i] - '0');
    }
    if (str[0] == '-') {
        sign = -1;
    }
    if (digits.size() == 0) {
        *this = big_integer();
    }
}


big_integer::~big_integer() = default;

void big_integer::clear() {
    while (digits.size() > 1 && !digits.back()) {
        digits.pop_back();
    }
    if ((digits.size() == 1 && digits[0] == 0 )|| digits.size() == 0) {
        *this = big_integer();
    }
}


big_integer &big_integer::operator=(big_integer const &other) {
    digits = other.digits;
    sign = other.sign;
    return *this;
}


big_integer &big_integer::operator+=(big_integer const &rhs) {
    if (rhs.sign == 0) {
        return *this;
    }
    if (sign == 0) {
        *this = big_integer(rhs);
        return *this;
    }
    if (sign != rhs.sign) {
        *this -= -rhs;
    } else {
        uint32_t carry = 0;
        for (size_t i = 0; (i < std::max(digits.size(), rhs.digits.size())) || carry; ++i) {
            if (i == digits.size()) {
                digits.push_back(0);
            }
            uint32_t save = digits[i];
            digits[i] += (i < rhs.digits.size() ? rhs.digits[i] : 0) + carry;
            if (digits[i] < save || (carry == 1 && digits[i] == save)) {
                carry = 1;
            } else {
                carry = 0;
            }
        }
    }
    return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    if (rhs.sign == 0) {
        return *this;
    }
    if (sign == 0) {
        *this = big_integer(-rhs);
        return *this;
    }
    if (sign != rhs.sign) {
        *this += -rhs;
    } else {
        if (*this >= rhs) {
            uint32_t carry = 0;
            for (size_t i = 0; i < digits.size(); ++i) {
                uint32_t save = digits[i];
                digits[i] -= (i < rhs.digits.size() ? rhs.digits[i] : 0) + carry;
                if (digits[i] > save || (carry == 1 && digits[i] == save)) {
                    carry = 1;
                } else {
                    carry = 0;
                }
            }
            clear();
        } else {
            big_integer buf(rhs);
            buf -= *this;
            *this = -buf;
        }
    }
    return *this;
}

void mul(big_integer &left, uint32_t a) {
    uint32_t carry = 0;
    for (size_t i = 0; i < left.digits.size() || carry; ++i) {
        if (i == left.digits.size()) {
            left.digits.push_back(0);
        }
        uint64_t buf = left.digits[i] * static_cast<uint64_t> (a) + carry;
        carry = static_cast<uint32_t>(buf / BASE);
        left.digits[i] = static_cast<uint32_t>(buf % BASE);
    }
    left.clear();
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    big_integer ans;
    ans.digits.resize(digits.size() + rhs.digits.size(), true);
    ans.sign = sign * rhs.sign;
    uint64_t buf;
    for (size_t i = 0; i < digits.size(); ++i) {
        uint32_t carry = 0;
        for (size_t j = 0; j < rhs.digits.size() || carry; ++j) {
            buf = static_cast<uint64_t> (ans.digits[i + j]) + static_cast<uint64_t> (digits[i]) * (j < rhs.digits.size() ? rhs.digits[j] : 0) +
                  carry;
            ans.digits[i + j] = static_cast<uint32_t>(buf % BASE);
            carry = static_cast<uint32_t>(buf / BASE);
        }
    }

    ans.clear();
    *this = ans;
    return *this;

}

void div(big_integer &left, uint32_t v) {
    if (left.sign == 0) {
        return;
    }
    for (int32_t i = (int32_t) left.digits.size() - 1, rem = 0; i >= 0; --i) {
        uint64_t buf = left.digits[i] + rem * BASE;
        left.digits[i] = static_cast<uint32_t>((buf / v));
        rem = (int32_t) (buf % v);
    }
    left.clear();
    return;
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    if (sign == 0 || digits.size() < rhs.digits.size()) {
        return *this = big_integer();
    }
    if (rhs.digits.size() == 1) {
        div(*this, rhs.digits[0]);
        sign = sign * rhs.sign;
        return *this;
    }
    big_integer ans(0);
    int64_t n = rhs.digits.size();
    int64_t m = digits.size() - n;
    ans.digits.resize(m + n, true);
    ans.sign = sign * rhs.sign;
    big_integer saverhs = rhs;
    saverhs.sign = 1;
    sign = 1;
    int64_t uJ, vJ, i, buf3;
    uint64_t buf1, buf2, buf, scale, ansGuess, r, borrow, carry;
    scale = BASE / (rhs.digits[n - 1] + 1);
    if (scale > 1) {
        *this *= (big_integer) scale;
        saverhs *= (big_integer) scale;
    }
    digits.resize(digits.size() + 1,true);
    for (vJ = m, uJ = n + vJ; vJ >= 0; --vJ, --uJ) {
        ansGuess = (digits[uJ] * BASE + digits[uJ - 1]) / saverhs.digits[n - 1];
        r = (digits[uJ] * BASE + digits[uJ - 1]) % saverhs.digits[n - 1];
        while (r < BASE) {
            buf2 = (int64_t) saverhs.digits[n - 2] * ansGuess;
            buf1 = r * BASE + digits[uJ - 2];
            if ((buf2 > buf1) || (ansGuess == BASE)) {
                --ansGuess;
                r += saverhs.digits[n - 1];
            } else {
                break;
            }
        }
        carry = 0;
        borrow = 0;
        for (i = 0; i < n; i++) {
            buf1 = (int64_t) saverhs.digits[i] * ansGuess + carry;
            carry = buf1 / BASE;
            buf1 -= carry * BASE;
            buf3 = digits[vJ + i] - buf1 - borrow;
            if (buf3 < 0) {
                digits[vJ + i] = uint32_t(buf3 + BASE);
                borrow = 1;
            } else {
                digits[vJ + i] = uint32_t(buf3);
                borrow = 0;
            }
        }
        buf3 = digits[vJ + i] - carry - borrow;
        if (buf3 < 0) {
            digits[vJ + i] = static_cast<uint32_t>(buf3 + BASE);
            borrow = 1;
        } else {
            digits[vJ + i] = static_cast<uint32_t>(buf3);
            borrow = 0;
        }
        if (borrow == 0) {
            ans.digits[vJ] = static_cast<uint32_t>(ansGuess);
        } else {
            ans.digits[vJ] = static_cast<uint32_t>(ansGuess - 1);
            carry = 0;
            for (i = 0; i < n; i++) {
                buf = digits[vJ + i] + saverhs.digits[i] + carry;
                if (buf >= BASE) {
                    digits[vJ + i] = static_cast<uint32_t>(buf - BASE);
                    carry = 1;
                } else {
                    digits[vJ + i] = static_cast<uint32_t>(buf);
                    carry = 0;
                }
            }
            digits[vJ + i] = static_cast<uint32_t>(digits[vJ + i] + carry - BASE);
        }
    }
    ans.clear();
    return *this = ans;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    return *this -= (*this / rhs * rhs);
}

my_vector big_integer::to_twos_complement(big_integer const &a, size_t length) {
    my_vector vec = a.digits;
    vec.resize(length, true);
    if (a.sign == -1) {
        uint32_t carry = 1;
        for (size_t i = 0; i < vec.size(); ++i) {
            vec[i] = ~vec[i] + carry;
            if (vec[i] != 0) {
                carry = 0;
            }
        }
    }
    return vec;
}

big_integer big_integer::from_twos_complement(const my_vector &a) {
    big_integer num;
    num.digits = a;
    num.sign = 1;
    if (a.size() == 0) {
        num.sign = 0;
    }
    if ((a.back() >> (LENGTH - 1)) == 1) {
        uint32_t carry = 1;
        for (size_t i = 0; i < num.digits.size(); ++i) {
            num.digits[i] = ~num.digits[i] + carry;
            if (num.digits[i] != 0) {
                carry = 0;
            }
        }
        num.sign = -1;
    }
    num.clear();
    return num;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    size_t size = std::max(digits.size(), rhs.digits.size()) + 1;
    my_vector vec1 = to_twos_complement(*this, size);
    my_vector vec2 = to_twos_complement(rhs, size);
    for (size_t i = 0; i < size; ++i)
        vec1[i] &= vec2[i];
    return *this = from_twos_complement(vec1);
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    size_t size = std::max(digits.size(), rhs.digits.size()) + 1;
    my_vector vec1 = to_twos_complement(*this, size);
    my_vector vec2 = to_twos_complement(rhs, size);
    for (size_t i = 0; i < size; ++i)
        vec1[i] |= vec2[i];
    return *this = from_twos_complement(vec1);
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    size_t size = std::max(digits.size(), rhs.digits.size());
    my_vector vec1 = to_twos_complement(*this, size);
    my_vector vec2 = to_twos_complement(rhs, size);
    for (size_t i = 0; i < size; ++i)
        vec1[i] ^= vec2[i];
    return *this = from_twos_complement(vec1);
}

big_integer &big_integer::operator<<=(int rhs) {
    size_t shift = (size_t) (rhs / LENGTH);
    my_vector vector = to_twos_complement(*this, digits.size() + shift + 1);
    if (shift > 0) {
        for (size_t i = digits.size(); i > 0; --i)
            vector[i + shift - 1] = vector[i - 1];

        for (size_t i = 0; i < shift; ++i)
            vector[i] = 0;
    }

    shift = rhs - shift * LENGTH;
    if (shift != 0) {
        for (size_t i = vector.size(); i > 1; --i)
            vector[i - 1] = ((vector[i - 1] << shift) | (vector[i - 2] >> (LENGTH - shift)));
        vector[0] <<= shift;
    }

    return *this = from_twos_complement(vector);
}

big_integer &big_integer::operator>>=(int rhs) {
    size_t shift = (size_t) (rhs / LENGTH);
    my_vector vector = to_twos_complement(*this, digits.size() + 1);
    uint32_t pad = vector.back();
    if (shift > 0) {
        for (size_t i = 0; i < digits.size() - shift; ++i) {
            vector[i] = vector[i + shift];
        }
        for (size_t i = digits.size() - shift; i < vector.size(); ++i) {
            vector[i] = pad;
        }
    }
    shift = rhs - shift * LENGTH;
    if (shift != 0) {
        for (size_t i = 0; i < vector.size() - 1; ++i) {
            vector[i] = ((vector[i] >> shift) | (vector[i + 1] << (LENGTH - shift)));
        }
        vector[vector.size() - 1] = (vector[vector.size() - 1] >> shift);
        if (sign < 0) {
            vector[vector.size() - 1] |= (UINT32_MAX << (LENGTH - shift));
        }
        // vector.back() = pad;
    }
    return (*this = from_twos_complement(vector));
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer ans = *this;
    ans.sign = -sign;
    if (digits.size() == 0) {
        ans.sign = 0;
    }
    return ans;
}

big_integer big_integer::operator~() const {
    big_integer ans(*this);
    ans = -ans - 1;
    return ans;
}

big_integer &big_integer::operator++() {
    return *this += 1;
}

big_integer big_integer::operator++(int) {
    big_integer ans = *this;
    ++*this;
    return ans;
}

big_integer &big_integer::operator--() {
    return *this -= 1;
}

big_integer big_integer::operator--(int) {
    big_integer ans = *this;
    --*this;
    return ans;
}

big_integer operator+(big_integer a, big_integer const &b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const &b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const &b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const &b) {
    return a /= b;
}

int64_t big_integer::operator%(uint32_t v) {
    int64_t m = 0;
    for (int32_t i = int32_t(digits.size()) - 1; i >= 0; --i)
        m = int64_t((digits[i] + m * BASE) % v);
    return m * sign;
}

big_integer operator%(big_integer a, big_integer const &b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const &b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const &b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const &b) {
    return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}

bool operator==(big_integer const &a, big_integer const &b) {
    if (a.sign != b.sign || a.digits.size() != b.digits.size()) {
        return false;
    }
    for (size_t i = 0; i < a.digits.size(); ++i) {
        if (a.digits[i] != b.digits[i]) {
            return false;
        }
    }
    return true;
}

bool operator!=(big_integer const &a, big_integer const &b) {
    return !(a == b);
}

bool operator<(big_integer const &a, big_integer const &b) {
    if (a.sign == 0 && b.sign == 0) {
        return false;
    }
    if (a.sign != b.sign) {
        return a.sign < b.sign;
    }
    if (a.digits.size() != b.digits.size()) {
        return a.digits.size() < b.digits.size();
    }
    for (size_t i = a.digits.size(); i > 0; --i) {
        if (a.digits[i - 1] != b.digits[i - 1]) {
            return a.digits[i - 1] < b.digits[i - 1];
        }
    }
    return false;
}

bool operator>(big_integer const &a, big_integer const &b) {
    return !(a <= b);
}

bool operator<=(big_integer const &a, big_integer const &b) {
    if (a.sign == 0 && b.sign == 0) {
        return true;
    }
    if (a.sign != b.sign) {
        return a.sign <= b.sign;
    }
    if (a.digits.size() != b.digits.size()) {
        return a.digits.size() <= b.digits.size();
    }
    for (size_t i = a.digits.size(); i > 0; --i) {
        if (a.digits[i - 1] != b.digits[i - 1]) {
            return a.digits[i - 1] <= b.digits[i - 1];
        }
    }
    return true;
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return !(a < b);
}

std::string to_string(big_integer const &a) {
    big_integer b = a;
    if (a.sign == 0) {
        return "0";
    }
    if (a.sign == -1) {
        b.sign = -b.sign;
    }
    std::string str;
    while (b != 0) {
        int64_t mod = b % 10;
        if (b != 0) {
            str.append(1, char(mod + '0'));
        } else {
            str.append(1, '0');
        }
        div(b, 10);
    }
    if (a.sign == -1) {
        str.append("-");
    }
    std::reverse(str.begin(), str.end());
    return str;
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    return s << to_string(a);
}