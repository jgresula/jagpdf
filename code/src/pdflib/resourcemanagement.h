// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __RESOURCEMANAGEMENT_H_JG2223__
#define __RESOURCEMANAGEMENT_H_JG2223__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include "indirectobjectref.h"
#include "fontmanagement.h"
#include "patternimpl.h"
#include <resources/utils/resourcetable.h>

#include <resources/interfaces/resourcehandle.h>
#include "resourcehandletable.h"
#include "graphicsstatedictionary.h"

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/functional/hash.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <map>
#include <vector>

namespace jag {
class IImageData;
class IImageMan;
class ITypeMan;
class ICanvas;

namespace pdf {

class DocWriterImpl;
class TilingPatternImpl;
class FunctionObj;


/**
 * @brief repository for resources handled by single document
 *
 * Main purpose is to manage resources referenced throughout a document,
 * in particular to map resource handles to indirect objects.
 * For resources that are specific to PDF (e.g. patterns) provides also
 * factory functions for definition objects (specs).
 *
 * There is a set of methods with signature
 * @code
 * IndirectObjectRef const& RES_reference(RESHandle)
 * @endcode
 *
 * Majority of these methods output the resource object on the first call of this
 * method which imposes restrictions on clients concerning when this method can be
 * called.
 *
 * On the other hand, there are also resources that are outputted during the document
 * finalization (e.g. fonts in order to know used character codes). These methods
 * can be called anytime as they do not output anything within the call they just
 * put the particular resource to an output queue that is processed later.
 *
 * Check documentation of individual functions.
 *
 */
class ResourceManagement
{
public:
    explicit ResourceManagement(DocWriterImpl& doc);
    ~ResourceManagement();

public:
    // color spaces
    PatternHandle tiling_pattern_load(Char const* pattern, ICanvas* canvas);
    PatternHandle shading_pattern_load(Char const* pattern,
                                       ColorSpaceHandle cs,
                                       FunctionHandle const* funcs,
                                       UInt num_functions);
    IIndirectObject& pattern(PatternHandle ph);
    IndirectObjectRef const& pattern_ref(PatternHandle ph, double page_height);

    
    IndirectObjectRef const& shading_ref(ShadingHandle sh);
    ShadingHandle shading_from_pattern(PatternHandle handle);
    IndirectObjectRef const& color_space_ref(ColorSpaceHandle color_space_id);

    // canvas
    ICanvas* canvas_create() const;
    IndirectObjectRef& canvas_ref(ICanvas* canvas);
    struct CanvasRecord;
    CanvasRecord* canvas_record(ICanvas*);


    // images & masks
    IndirectObjectRef const& image_reference(ImageHandle image_id);
    IndirectObjectRef const& image_mask_reference(ImageMaskHandle imagemask);

    // functions
    IndirectObjectRef const& function_ref(FunctionHandle handle);
    FunctionHandle function_2_load(Char const* fun);
    FunctionHandle function_4_load(Char const* fun);
    FunctionHandle function_3_load(Char const* fun,
                                   FunctionHandle const* funs, UInt nr_funs);

    // graphics states
    GraphicsStateHandle register_graphics_state(jag::pdf::GraphicsStateDictionary const& gs_dict);
    IndirectObjectRef graphics_state_reference(GraphicsStateHandle gs_handle);

    // fonts
    FontManagement& fonts() { return m_font_management; }
    FontManagement const& fonts() const { return m_font_management; }


public:
    void finalize();

public:
    // canvas
    struct CanvasRecord
    {
        IndirectObjectRef  ref;
        bool               can_be_shared;
        CanvasRecord() : can_be_shared(true) {}
    };


private:
    IImageMan& image_man();
    class PatternVisitorTopdown;

private:
    DocWriterImpl&                    m_doc;
    FontManagement                      m_font_management;

    // image stuff
    typedef std::map<ImageHandle,IndirectObjectRef>    ImageMap;
    ImageMap    m_images;

    typedef std::map<ImageMaskHandle,IndirectObjectRef>    ImageMaskMap;
    ImageMaskMap    m_image_masks;

    typedef std::map<ImageSoftMaskHandle,IndirectObjectRef>    ImageSoftMaskMap;
    ImageSoftMaskMap    m_image_soft_masks;


    // color spaces
    typedef std::map<ColorSpaceHandle,IndirectObjectRef> ColorSpacesMap;
    ColorSpacesMap m_color_spaces;

    // functions
    typedef std::map<FunctionHandle,IndirectObjectRef> FunctionsMap;
    FunctionsMap m_functions;
    resources::ResourceTable<FunctionHandle,
                             IIndirectObject,
                             boost::ptr_vector<IIndirectObject> > m_fun_table;

    // patterns
    typedef std::map<PatternHandle,IndirectObjectRef> PatternsMap;
    PatternsMap m_patterns;
    resources::ResourceTable<PatternHandle,
                             IIndirectObject,
                             boost::ptr_vector<IIndirectObject> > m_pattern_table;

    typedef std::pair<PatternHandle, double> PatternAndPageHeight;
    friend bool operator<(PatternAndPageHeight const&lhs,
                          PatternAndPageHeight const&rhs);

    typedef std::map<PatternAndPageHeight, PatternHandle> PatternsByPageHeight;
    PatternsByPageHeight m_patterns_by_page_height;

    // shadings
    typedef std::map<ShadingHandle,IndirectObjectRef> ShadingsMap;
    ShadingsMap m_shadings;
    resources::ResourceTable<ShadingHandle,
                             IIndirectObject,
                             boost::ptr_vector<IIndirectObject> > m_shading_table;
    typedef std::map<PatternHandle,ShadingHandle> PatternToShadingMap;
    PatternToShadingMap m_pattern_to_shading;


    // graphics states - two copies are stored per handle
    // one in GraphicsStateToHandle, the second in ResourceHandleTable<>
    typedef std::map<GraphicsStateDictionary, GraphicsStateHandle> GraphicsStateToHandle;
    GraphicsStateToHandle m_gs_to_handle;

    typedef ResourceHandleTable<GraphicsStateHandle,GraphicsStateDictionary> GSHandleToGSDict;
    GSHandleToGSDict m_handle_to_gs;

    typedef std::map<GraphicsStateHandle,IndirectObjectRef>    GSStateToObject;
    GSStateToObject m_gs_to_obj;

    typedef std::map<ICanvas*, CanvasRecord> CanvasMap;
    mutable CanvasMap m_canvas_map; // keys deleted manually in destructor
};


}} //namespace jag::pdf


#endif //__RESOURCEMANAGEMENT_H_JG2223__






















