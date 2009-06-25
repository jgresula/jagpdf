// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//



#ifndef __GRAPHICSSTATEDICTIONARY_H_JG_1528__
#define __GRAPHICSSTATEDICTIONARY_H_JG_1528__

#include <boost/static_assert.hpp>
#include <boost/shared_ptr.hpp>
#include <bitset>

namespace jag {
namespace pdf {
class ObjFmtBasic;

/// contains graphics state parameters which are set through a dictionary
class GraphicsStateDictionary
{
public:
    // when adding new members, do not forget to update operators
    bool   m_alpha_is_shape;
    Double m_stroking_alpha;
    Double m_nonstroking_alpha;
    FunctionHandle m_transfer_fn;
    GraphicsStateDictionary();

    enum {
        GS_ALPHA_IS_SHAPE
        , GS_STROKING_ALPHA
        , GS_NONSTROKING_ALPHA
        , GS_TRANSFER_FUNCTION

        , GS_NUM_DICT_PARAMS
    };

    // if this fail, then hashing must be adjusted
    BOOST_STATIC_ASSERT(GS_NUM_DICT_PARAMS <= 8*sizeof(unsigned long));
    std::bitset<GS_NUM_DICT_PARAMS>    m_param_changed;
};

bool operator==(GraphicsStateDictionary const& lhs, GraphicsStateDictionary const& rhs);
bool operator<(GraphicsStateDictionary const& lhs, GraphicsStateDictionary const& rhs);
size_t hash_value(GraphicsStateDictionary const& gs_dict);


}} //namespace jag::pdf

#endif //__GRAPHICSSTATEDICTIONARY_H_JG_1528__
