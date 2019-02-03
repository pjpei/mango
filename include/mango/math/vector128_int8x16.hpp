/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2018 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#pragma once

#include "vector.hpp"

namespace mango
{

    template <>
    struct Vector<s8, 16>
    {
        using VectorType = simd::int8x16;
        using ScalarType = s8;
        enum { VectorSize = 16 };

        union
        {
            VectorType m;
            DeAggregate<ScalarType> component[VectorSize];
        };

        ScalarType& operator [] (size_t index)
        {
            assert(index < VectorSize);
            return component[index].data;
        }

        ScalarType operator [] (size_t index) const
        {
            assert(index < VectorSize);
            return component[index].data;
        }

        const ScalarType* data() const
        {
            return reinterpret_cast<const ScalarType *>(component);
        }

        explicit Vector() {}
        ~Vector() {}

        Vector(s8 s)
            : m(simd::int8x16_set1(s))
        {
        }

        Vector(s8 v0, s8 v1, s8 v2, s8 v3, s8 v4, s8 v5, s8 v6, s8 v7, s8 v8, s8 v9, s8 v10, s8 v11, s8 v12, s8 v13, s8 v14, s8 v15)
            : m(simd::int8x16_set16(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15))
        {
        }

        Vector(simd::int8x16 v)
            : m(v)
        {
        }

        Vector& operator = (simd::int8x16 v)
        {
            m = v;
            return *this;
        }

        Vector& operator = (s8 s)
        {
            m = simd::int8x16_set1(s);
            return *this;
        }

        operator simd::int8x16 () const
        {
            return m;
        }

#ifdef int128_is_hardware_vector
        operator simd::int8x16::vector () const
        {
            return m.data;
        }
#endif
    };

    template <>
    inline Vector<s8, 16> load_low<s8, 16>(const s8 *source)
    {
        return simd::int8x16_load_low(source);
    }

    static inline void store_low(s8 *dest, Vector<s8, 16> v)
    {
        simd::int8x16_store_low(dest, v);
    }

    static inline const Vector<s8, 16> operator + (Vector<s8, 16> v)
    {
        return v;
    }

    static inline Vector<s8, 16> operator - (Vector<s8, 16> v)
    {
        return simd::sub(simd::int8x16_zero(), v);
    }

    static inline Vector<s8, 16>& operator += (Vector<s8, 16>& a, Vector<s8, 16> b)
    {
        a = simd::add(a, b);
        return a;
    }

    static inline Vector<s8, 16>& operator -= (Vector<s8, 16>& a, Vector<s8, 16> b)
    {
        a = simd::sub(a, b);
        return a;
    }

    static inline Vector<s8, 16> operator + (Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::add(a, b);
    }

    static inline Vector<s8, 16> operator - (Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::sub(a, b);
    }

    static inline Vector<s8, 16> nand(Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::bitwise_nand(a, b);
    }

    static inline Vector<s8, 16> operator & (Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::bitwise_and(a, b);
    }

    static inline Vector<s8, 16> operator | (Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::bitwise_or(a, b);
    }

    static inline Vector<s8, 16> operator ^ (Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::bitwise_xor(a, b);
    }

    static inline Vector<s8, 16> operator ~ (Vector<s8, 16> a)
    {
        return simd::bitwise_not(a);
    }

    static inline Vector<s8, 16> abs(Vector<s8, 16> a)
    {
        return simd::abs(a);
    }

    static inline Vector<s8, 16> adds(Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::adds(a, b);
    }

    static inline Vector<s8, 16> subs(Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::subs(a, b);
    }

    static inline Vector<s8, 16> min(Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::min(a, b);
    }

    static inline Vector<s8, 16> max(Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::max(a, b);
    }

    static inline Vector<s8, 16> clamp(Vector<s8, 16> a, Vector<s8, 16> amin, Vector<s8, 16> amax)
    {
        return simd::clamp(a, amin, amax);
    }

    static inline mask8x16 operator > (Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::compare_gt(a, b);
    }

    static inline mask8x16 operator >= (Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::compare_ge(a, b);
    }

    static inline mask8x16 operator < (Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::compare_lt(a, b);
    }

    static inline mask8x16 operator <= (Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::compare_le(a, b);
    }

    static inline mask8x16 operator == (Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::compare_eq(a, b);
    }

    static inline mask8x16 operator != (Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::compare_neq(a, b);
    }

    static inline Vector<s8, 16> select(mask8x16 mask, Vector<s8, 16> a, Vector<s8, 16> b)
    {
        return simd::select(mask, a, b);
    }

} // namespace mango
