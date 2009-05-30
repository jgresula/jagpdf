// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __COLORSPACES_JG2215_H__
#define __COLORSPACES_JG2215_H__

#include <interfaces/refcounted.h>
#include <interfaces/streams.h>
#include <boost/intrusive_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>

namespace jag {


class IColorSpaceVisitor;

class IColorSpace
    : public IRefCounted
{
public:
    virtual void accept(IColorSpaceVisitor& v) = 0;
    virtual void check_validity() const = 0;
    virtual ColorSpaceType color_space_type() const = 0;
    virtual int num_components() const = 0;
    virtual boost::intrusive_ptr<IColorSpace> clone() const = 0;

protected:
    ~IColorSpace() {}
};



class IPalette
    : public IColorSpace
{
public:
    // pattern or indexed not allowed
    virtual void set(ColorSpace cs_type, Byte const* array_in, UInt length) = 0;

protected:
    ~IPalette() {}
};


// A special treatment for ICIELabBase is needed as it is a base class
// for classes that are part of the public API. The API classes are
// flattened so they contain methods from all it base classes.
//
// Doxygen does not know about that and generates a documentation for
// the base class method only - single id is generated. That causes a
// problem in the generated documention as we need a unique id for
// methods in all derived classes.

#define CIELAB_BASE_METHODS                                             \
    /** x & z positive */                                               \
    virtual void white_point(Double xw, Double zw) = 0;               \
    /** all non-negative */                                             \
    virtual void black_point (Double xb, Double yb, Double zb) = 0;

#if defined(__GCCXML__) || defined(__DOXYGEN__)
# define CIELAB_BASE_METHODS_IN_BASE_CLS
# define CIELAB_BASE_METHODS_IN_DERIVED_CLS CIELAB_BASE_METHODS
#else
# define CIELAB_BASE_METHODS_IN_BASE_CLS CIELAB_BASE_METHODS
# define CIELAB_BASE_METHODS_IN_DERIVED_CLS
#endif



/// Base interface for CIE based color spaces
class ICIELabBase
    : public IColorSpace
{
public:
    CIELAB_BASE_METHODS_IN_BASE_CLS

protected:
    ~ICIELabBase() {}
};



/// CIE L*a*b
class ICIELab
    : public ICIELabBase
{
public:
    CIELAB_BASE_METHODS_IN_DERIVED_CLS
    virtual void range(Double amin, Double amax, Double bmin, Double bmax) = 0;

protected:
    ~ICIELab() {}
};



/// CIE-based A
class ICIECalGray
    : public ICIELabBase
{
public:
    CIELAB_BASE_METHODS_IN_DERIVED_CLS
    virtual void gamma(Double val) = 0;

protected:
    ~ICIECalGray() {}
};



/// CIE-based ABC
class ICIECalRGB
    : public ICIELabBase
{
public:
    CIELAB_BASE_METHODS_IN_DERIVED_CLS

    virtual void gamma(Double gr, Double gg, Double gb) = 0;
    virtual void matrix(
        Double xa, Double ya, Double za,
        Double xb, Double yb, Double zb,
        Double xc, Double yc, Double zc) = 0;

protected:
    ~ICIECalRGB() {}
};


class IICCBased
    : public IColorSpace
{
public:
    /// 1, 3, or 4 - could be automatic if we understood the icc format
    virtual void num_components(Int val) = 0;
    virtual void icc_profile(Char const* file_path) = 0;
    /**
     * @brief Uploads an ICC profile from a stream.
     *
     * @param stream    stream with a profile
     *
     */
    virtual void icc_profile_stream(std::auto_ptr<jag::ISeqStreamInput> stream) = 0;
    /// any colorspace except pattern, num components must match
    virtual void alternate(ColorSpace csh) = 0;

protected:
    ~IICCBased() {}
};

} // namespace jag

#endif // __COLORSPACES_JG2215_H__
/** EOF @file */
