/*  This file is part of the Vc library.

    Copyright (C) 2009-2012 Matthias Kretz <kretz@kde.org>

    Vc is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Vc is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Vc.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef VC_COMMON_TRIGONOMETRIC_H
#define VC_COMMON_TRIGONOMETRIC_H

//#define VC_DEBUG_TRIGONOMETRIC
#ifdef VC_DEBUG_TRIGONOMETRIC
#include <iostream>
#include <iomanip>
#endif
#include "macros.h"

namespace Vc
{
namespace Common
{
#ifdef VC__USE_NAMESPACE
    using Vc::VC__USE_NAMESPACE::Const;
    using Vc::VC__USE_NAMESPACE::Vector;
#endif
    typedef Vector<float> float_v;
    typedef Vector<double> double_v;
    typedef Vector<sfloat> sfloat_v;

    /*
     * algorithm for sine and cosine:
     *
     * The result can be calculated with sine or cosine depending on the π/4 section the input is
     * in.
     * sine   ≈ x + x³
     * cosine ≈ 1 - x²
     *
     * sine:
     * Map -x to x and invert the output
     * Extend precision of x - n * π/4 by calculating
     * ((x - n * p1) - n * p2) - n * p3 (p1 + p2 + p3 = π/4)
     *
     * Calculate Taylor series with tuned coefficients.
     * Fix sign.
     */
    template<typename _T> static inline Vector<_T> sin(const Vector<_T> &_x)
    {
        typedef Vector<_T> V;
        typedef Const<_T> C;
        typedef typename V::EntryType T;
        typedef typename V::Mask M;
        typedef typename V::IndexType IV;

        V x = abs(_x);
        M sign = _x < V::Zero();
        IV j = static_cast<IV>(x * C::_4_pi());
        typename IV::Mask mask = (j & IV::One()) != IV::Zero();
        ++j(mask);
        V y = static_cast<V>(j);
        j &= 7;
        sign ^= j > 3;
        j(j > 3) -= 4;

        M lossMask = x > C::lossThreshold();
        x(lossMask) = x - y * C::_pi_4();
        x(!lossMask) = ((x - y * C::_pi_4_hi()) - y * C::_pi_4_rem1()) - y * C::_pi_4_rem2();

        V z = x * x;
        V cos_s = ((C::cosCoeff(2)  * z +
                    C::cosCoeff(1)) * z +
                    C::cosCoeff(0)) * (z * z)
            - C::_1_2() * z + V::One();
        V sin_s = ((C::sinCoeff(2)  * z +
                    C::sinCoeff(1)) * z +
                    C::sinCoeff(0)) * (z * x)
            + x;
        y = sin_s;
        y(j == IV::One() || j == 2) = cos_s;
        y(sign) = -y;
        return y;
    }

    template<> inline double_v sin(const double_v &_x)
    {
        typedef double_v V;
        typedef Const<double> C;
        typedef V::EntryType T;
        typedef V::Mask M;
        typedef V::IndexType IV;

        V x = abs(_x);
        M sign = _x < V::Zero();
        V y = floor(x / V(C::_pi_4()));
        V z = y - floor(y * C::_1_16()) * C::_16();
        IV j = static_cast<IV>(z);
        IV::Mask mask = (j & IV::One()) != IV::Zero();
        ++j(mask);
        y(static_cast<M>(mask)) += V::One();
        j &= 7;
        sign ^= static_cast<M>(j > 3);
        j(j > 3) -= 4;

        // since y is an integer we don't need to split y into low and high parts until the integer
        // requires more bits than there are zero bits at the end of _pi_4_hi (30 bits -> 1e9)
        z = ((x - y * C::_pi_4_hi()) - y * C::_pi_4_rem1()) - y * C::_pi_4_rem2();

        V zz = z * z;
        V cos_s = (((((C::cosCoeff(5)  * zz +
                       C::cosCoeff(4)) * zz +
                       C::cosCoeff(3)) * zz +
                       C::cosCoeff(2)) * zz +
                       C::cosCoeff(1)) * zz +
                       C::cosCoeff(0)) * (zz * zz)
                  - C::_1_2() * zz + V::One();
        V sin_s = (((((C::sinCoeff(5)  * zz +
                       C::sinCoeff(4)) * zz +
                       C::sinCoeff(3)) * zz +
                       C::sinCoeff(2)) * zz +
                       C::sinCoeff(1)) * zz +
                       C::sinCoeff(0)) * (zz * z)
                  + z;
        y = sin_s;
        y(static_cast<M>(j == IV::One() || j == 2)) = cos_s;
        y(sign) = -y;
        return y;
    }
    template<typename _T> static inline Vector<_T> cos(const Vector<_T> &_x) {
        typedef Vector<_T> V;
        typedef Const<_T> C;
        typedef typename V::EntryType T;
        typedef typename V::Mask M;
        typedef typename V::IndexType IV;

        V x = abs(_x);
        IV j = static_cast<IV>(x * C::_4_pi());
        typename IV::Mask mask = (j & IV::One()) != IV::Zero();
        ++j(mask);
        V y = static_cast<V>(j);
        j &= 7;
        M sign = j > 3;
        j(j > 3) -= 4;
        sign ^= j > IV::One();

        M lossMask = x > C::lossThreshold();
        x(lossMask) = x - y * C::_pi_4();
        x(!lossMask) = ((x - y * C::_pi_4_hi()) - y * C::_pi_4_rem1()) - y * C::_pi_4_rem2();

        V z = x * x;
        V cos_s = ((C::cosCoeff(2)  * z +
                    C::cosCoeff(1)) * z +
                    C::cosCoeff(0)) * (z * z)
            - C::_1_2() * z + V::One();
        V sin_s = ((C::sinCoeff(2)  * z +
                    C::sinCoeff(1)) * z +
                    C::sinCoeff(0)) * (z * x)
            + x;
        y = cos_s;
        y(j == IV::One() || j == 2) = sin_s;
        y(sign) = -y;
        return y;
    }
    template<> inline double_v cos(const double_v &_x)
    {
        typedef double_v V;
        typedef Const<double> C;
        typedef V::EntryType T;
        typedef V::Mask M;
        typedef V::IndexType IV;

        V x = abs(_x);
        V y = floor(x / C::_pi_4());
        V z = y - floor(y * C::_1_16()) * C::_16();
        IV j = static_cast<IV>(z);
        IV::Mask mask = (j & IV::One()) != IV::Zero();
        ++j(mask);
        y(static_cast<M>(mask)) += V::One();
        j &= 7;
        M sign = static_cast<M>(j > 3);
        j(j > 3) -= 4;
        sign ^= static_cast<M>(j > IV::One());

        // since y is an integer we don't need to split y into low and high parts until the integer
        // requires more bits than there are zero bits at the end of _pi_4_hi (30 bits -> 1e9)
        z = ((x - y * C::_pi_4_hi()) - y * C::_pi_4_rem1()) - y * C::_pi_4_rem2();

        V zz = z * z;
        V cos_s = (((((C::cosCoeff(5)  * zz +
                       C::cosCoeff(4)) * zz +
                       C::cosCoeff(3)) * zz +
                       C::cosCoeff(2)) * zz +
                       C::cosCoeff(1)) * zz +
                       C::cosCoeff(0)) * (zz * zz)
                  - C::_1_2() * zz + V::One();
        V sin_s = (((((C::sinCoeff(5)  * zz +
                       C::sinCoeff(4)) * zz +
                       C::sinCoeff(3)) * zz +
                       C::sinCoeff(2)) * zz +
                       C::sinCoeff(1)) * zz +
                       C::sinCoeff(0)) * (zz * z)
                  + z;
        y = cos_s;
        y(static_cast<M>(j == IV::One() || j == 2)) = sin_s;
        y(sign) = -y;
        return y;
    }
    template<typename _T> static inline void sincos(const Vector<_T> &_x, Vector<_T> *_sin, Vector<_T> *_cos) {
        typedef Vector<_T> V;
        typedef Const<_T> C;
        typedef typename V::EntryType T;
        typedef typename V::Mask M;
        typedef typename V::IndexType IV;

        V x = abs(_x);
        IV j = static_cast<IV>(x * C::_4_pi());
        typename IV::Mask mask = (j & IV::One()) != IV::Zero();
        ++j(mask);
        V y = static_cast<V>(j);
        j &= 7;
        M sign = static_cast<M>(j > 3);
        j(j > 3) -= 4;

        M lossMask = x > C::lossThreshold();
        x(lossMask) = x - y * C::_pi_4();
        x(!lossMask) = ((x - y * C::_pi_4_hi()) - y * C::_pi_4_rem1()) - y * C::_pi_4_rem2();

        V z = x * x;
        V cos_s = ((C::cosCoeff(2)  * z +
                    C::cosCoeff(1)) * z +
                    C::cosCoeff(0)) * (z * z)
            - C::_1_2() * z + V::One();
        V sin_s = ((C::sinCoeff(2)  * z +
                    C::sinCoeff(1)) * z +
                    C::sinCoeff(0)) * (z * x)
            + x;

        V c = cos_s;
        c(static_cast<M>(j == IV::One() || j == 2)) = sin_s;
        c(sign ^ static_cast<M>(j > IV::One())) = -c;
        *_cos = c;

        V s = sin_s;
        s(static_cast<M>(j == IV::One() || j == 2)) = cos_s;
        s(sign ^ static_cast<M>(_x < V::Zero())) = -s;
        *_sin = s;
    }
    template<> inline void sincos(const double_v &_x, double_v *_sin, double_v *_cos) {
        typedef double_v V;
        typedef Const<double> C;
        typedef V::EntryType T;
        typedef V::Mask M;
        typedef V::IndexType IV;

        V x = abs(_x);
        V y = floor(x / C::_pi_4());
        V z = y - floor(y * C::_1_16()) * C::_16();
        IV j = static_cast<IV>(z);
        IV::Mask mask = (j & IV::One()) != IV::Zero();
        ++j(mask);
        y(static_cast<M>(mask)) += V::One();
        j &= 7;
        M sign = static_cast<M>(j > 3);
        j(j > 3) -= 4;

        // since y is an integer we don't need to split y into low and high parts until the integer
        // requires more bits than there are zero bits at the end of _pi_4_hi (30 bits -> 1e9)
        z = ((x - y * C::_pi_4_hi()) - y * C::_pi_4_rem1()) - y * C::_pi_4_rem2();

        V zz = z * z;
        V cos_s = (((((C::cosCoeff(5)  * zz +
                       C::cosCoeff(4)) * zz +
                       C::cosCoeff(3)) * zz +
                       C::cosCoeff(2)) * zz +
                       C::cosCoeff(1)) * zz +
                       C::cosCoeff(0)) * (zz * zz)
                  - C::_1_2() * zz + V::One();
        V sin_s = (((((C::sinCoeff(5)  * zz +
                       C::sinCoeff(4)) * zz +
                       C::sinCoeff(3)) * zz +
                       C::sinCoeff(2)) * zz +
                       C::sinCoeff(1)) * zz +
                       C::sinCoeff(0)) * (zz * z)
                  + z;

        V c = cos_s;
        c(static_cast<M>(j == IV::One() || j == 2)) = sin_s;
        c(sign ^ static_cast<M>(j > IV::One())) = -c;
        *_cos = c;

        V s = sin_s;
        s(static_cast<M>(j == IV::One() || j == 2)) = cos_s;
        s(sign ^ static_cast<M>(_x < V::Zero())) = -s;
        *_sin = s;
    }
    template<typename _T> static inline Vector<_T> asin (const Vector<_T> &_x) {
        typedef Const<_T> C;
        typedef Vector<_T> V;
        typedef typename V::EntryType T;
        typedef typename V::Mask M;

        const M &negative = _x < V::Zero();

        const V &a = abs(_x);
        const M outOfRange = a > V::One();
        const M &small = a < C::smallAsinInput();
        const M &gt_0_5 = a > C::_1_2();
        V x = a;
        V z = a * a;
        z(gt_0_5) = (V::One() - a) * C::_1_2();
        x(gt_0_5) = sqrt(z);
        z = ((((T(4.2163199048e-2)  * z
              + T(2.4181311049e-2)) * z
              + T(4.5470025998e-2)) * z
              + T(7.4953002686e-2)) * z
              + T(1.6666752422e-1)) * z * x
              + x;
        z(gt_0_5) = C::_pi_2() - (z + z);
        z(small) = a;
        z(negative) = -z;
        z.setQnan(outOfRange);

        return z;
    }
    template<> inline double_v asin (const double_v &_x) {
        typedef Const<double> C;
        typedef double_v V;
        typedef V::EntryType T;
        typedef V::Mask M;

        const M negative = _x < V::Zero();

        const V a = abs(_x);
        const M outOfRange = a > V::One();
        const M small = a < C::smallAsinInput();
        const M large = a > C::largeAsinInput();

        V zz = V::One() - a;
        const V r = (((C::asinCoeff0(0) * zz + C::asinCoeff0(1)) * zz + C::asinCoeff0(2)) * zz +
                C::asinCoeff0(3)) * zz + C::asinCoeff0(4);
        const V s = (((zz + C::asinCoeff1(0)) * zz + C::asinCoeff1(1)) * zz +
                C::asinCoeff1(2)) * zz + C::asinCoeff1(3);
        V sqrtzz = sqrt(zz + zz);
        V z = C::_pi_4() - sqrtzz;
        z -= sqrtzz * (zz * r / s) - C::_pi_2_rem();
        z += C::_pi_4();

        V a2 = a * a;
        const V p = ((((C::asinCoeff2(0) * a2 + C::asinCoeff2(1)) * a2 + C::asinCoeff2(2)) * a2 +
                    C::asinCoeff2(3)) * a2 + C::asinCoeff2(4)) * a2 + C::asinCoeff2(5);
        const V q = ((((a2 + C::asinCoeff3(0)) * a2 + C::asinCoeff3(1)) * a2 +
                    C::asinCoeff3(2)) * a2 + C::asinCoeff3(3)) * a2 + C::asinCoeff3(4);
        z(!large) = a * (a2 * p / q) + a;

        z(negative) = -z;
        z(small) = _x;
        z.setQnan(outOfRange);

        return z;
    }
    template<typename _T> static inline Vector<_T> atan (const Vector<_T> &_x) {
        typedef Const<_T> C;
        typedef Vector<_T> V;
        typedef typename V::EntryType T;
        typedef typename V::Mask M;
        V x = abs(_x);
        const M &gt_tan_3pi_8 = x > C::atanThrsHi();
        const M &gt_tan_pi_8  = x > C::atanThrsLo() && !gt_tan_3pi_8;
        V y = V::Zero();
        y(gt_tan_3pi_8) = C::_pi_2();
        y(gt_tan_pi_8)  = C::_pi_4();
        x(gt_tan_3pi_8) = -V::One() / x;
        x(gt_tan_pi_8)  = (x - V::One()) / (x + V::One());
        const V &x2 = x * x;
        y += (((C::atanP(0)  * x2
              - C::atanP(1)) * x2
              + C::atanP(2)) * x2
              - C::atanP(3)) * x2 * x
              + x;
        y(_x < V::Zero()) = -y;
        y.setQnan(isnan(_x));
        return y;
    }
    template<> inline double_v atan (const double_v &_x) {
        typedef Const<double> C;
        typedef double_v V;
        typedef V::EntryType T;
        typedef V::Mask M;

        M sign = _x < V::Zero();
        V x = abs(_x);
        M finite = isfinite(_x);
        V ret = C::_pi_2();
        V y = V::Zero();
        const M large = x > C::atanThrsHi();
        const M gt_06 = x > C::atanThrsLo();
        V tmp = (x - V::One()) / (x + V::One());
        tmp(large) = -V::One() / x;
        x(gt_06) = tmp;
        y(gt_06) = C::_pi_4();
        y(large) = C::_pi_2();
        V z = x * x;
        const V p = (((C::atanP(0) * z + C::atanP(1)) * z + C::atanP(2)) * z + C::atanP(3)) * z + C::atanP(4);
        const V q = ((((z + C::atanQ(0)) * z + C::atanQ(1)) * z + C::atanQ(2)) * z + C::atanQ(3)) * z + C::atanQ(4);
        z = z * p / q;
        z = x * z + x;
        V morebits = C::_pi_2_rem();
        morebits(!large) *= C::_1_2();
        z(gt_06) += morebits;
        ret(finite) = y + z;
        ret(sign) = -ret;
        ret.setQnan(isnan(_x));
        return ret;
    }
    template<typename _T> static inline Vector<_T> atan2(const Vector<_T> &y, const Vector<_T> &x) {
        typedef Const<_T> C;
        typedef Vector<_T> V;
        typedef typename V::EntryType T;
        typedef typename V::Mask M;

        const M xZero = x == V::Zero();
        const M yZero = y == V::Zero();
        const M xMinusZero = xZero && x.isNegative();
        const M yNeg = y < V::Zero();
        const M xInf = !isfinite(x);
        const M yInf = !isfinite(y);

        V a = C::_pi().copySign(y);
        a.setZero(x >= V::Zero());

        // setting x to any finite value will have atan(y/x) return sign(y/x)*pi/2, just in case x is inf
        V _x = x;
        _x(yInf) = V::One().copySign(x);

        a += Vc::Common::atan(y / _x);

        // if x is +0 and y is +/-0 the result is +0
        a.setZero(xZero && yZero);

        // for x = -0 we add/subtract pi to get the correct result
        a(xMinusZero) += C::_pi().copySign(y);

        // atan2(-Y, +/-0) = -pi/2
        a(xZero && yNeg) = -C::_pi_2();

        // if both inputs are inf the output is +/- (3)pi/4
        a(xInf && yInf) += C::_pi_4().copySign(x ^ ~y);

        // correct the sign of y if the result is 0
        a(a == V::Zero()) = a.copySign(y);

        // any NaN input will lead to NaN output
        a.setQnan(isnan(y) || isnan(x));

        return a;
    }
    template<> inline double_v atan2 (const double_v &y, const double_v &x) {
        typedef Const<double> C;
        typedef double_v V;
        typedef V::EntryType T;
        typedef V::Mask M;

        const M xZero = x == V::Zero();
        const M yZero = y == V::Zero();
        const M xMinusZero = xZero && x.isNegative();
        const M yNeg = y < V::Zero();
        const M xInf = !isfinite(x);
        const M yInf = !isfinite(y);

        V a = V(C::_pi()).copySign(y);
        a.setZero(x >= V::Zero());

        // setting x to any finite value will have atan(y/x) return sign(y/x)*pi/2, just in case x is inf
        V _x = x;
        _x(yInf) = V::One().copySign(x);

        a += Vc::Common::atan(y / _x);

        // if x is +0 and y is +/-0 the result is +0
        a.setZero(xZero && yZero);

        // for x = -0 we add/subtract pi to get the correct result
        a(xMinusZero) += C::_pi().copySign(y);

        // atan2(-Y, +/-0) = -pi/2
        a(xZero && yNeg) = -C::_pi_2();

        // if both inputs are inf the output is +/- (3)pi/4
        a(xInf && yInf) += C::_pi_4().copySign(x ^ ~y);

        // correct the sign of y if the result is 0
        a(a == V::Zero()) = a.copySign(y);

        // any NaN input will lead to NaN output
        a.setQnan(isnan(y) || isnan(x));

        return a;
    }
} // namespace Common
#ifdef VC__USE_NAMESPACE
namespace VC__USE_NAMESPACE
{
    using Vc::Common::sin;
    using Vc::Common::cos;
    using Vc::Common::sincos;
    using Vc::Common::asin;
    using Vc::Common::atan;
    using Vc::Common::atan2;
} // namespace VC__USE_NAMESPACE
#undef VC__USE_NAMESPACE
#endif
} // namespace Vc

#include "undomacros.h"

#endif // VC_COMMON_TRIGONOMETRIC_H
