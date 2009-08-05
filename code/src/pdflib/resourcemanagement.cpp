// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "resourcemanagement.h"
#include "resourcemanagementhelpers.h"
#include "resourcenames.h"
#include "colorspace.h"
#include "docwriterimpl.h"
#include "imagexobject.h"
#include "imagexobjectmask.h"
#include "graphicsstatedictonaryobject.h"
#include "fontdictionary.h"
#include "fontdescriptor.h"
#include "indirectobjectfwd.h"
#include "indirectobjectcallback.h"
#include "contentstream.h"
#include "objfmtbasic.h"
#include "defines.h"
#include "canvasimpl.h"
#include "visitornoop.h"
#include "patternimpl.h"
#include <core/jstd/transaffine.h>
#include <msg_pdflib.h>
#include <msg_jstd.h>
#include <core/jstd/optionsparser.h>
#include <resources/interfaces/imageman.h>
#include <resources/interfaces/typeman.h>
#include <resources/interfaces/imagedata.h>
#include <resources/interfaces/imagemaskdata.h>
#include <resources/interfaces/resourcectx.h>
#include <core/generic/checked_cast.h>
#include <core/generic/refcountedimpl.h>
#include <boost/mem_fn.hpp>
#include <boost/bind.hpp>
#include <boost/checked_delete.hpp>
#include <boost/scoped_ptr.hpp>


using namespace jag::jstd;
using namespace boost;

namespace jag {
namespace pdf {


// ---------------------------------------------------------------------------
//                           Function object
//

namespace
{
  enum FnSpecKeywords {
      FN_DOMAIN, FN_RANGE, FN_C0, FN_C1,
      FN_EXPONENT, FN_PS, FN_BOUNDS, FN_ENCODE };

  struct FnKeywords
      : public spirit::classic::symbols<unsigned>
  {
      FnKeywords()
      {
          add
              ("domain", FN_DOMAIN)
              ("range", FN_RANGE)
              ("c0", FN_C0)
              ("c1", FN_C1)
              ("exponent", FN_EXPONENT)
              ("func", FN_PS)
              ("bounds", FN_BOUNDS)
              ("encode", FN_ENCODE)
              ;
      }
  } g_fn_keywords;
} // anonymous namespace

//
// Represents a PDF function object
//
class FunctionObj
    : public IndirectObjectFwd
{
public:
    DEFINE_VISITABLE;
    enum FunctionType {TYPE_0 = 0,
                       TYPE_2 = 2,
                       TYPE_3 = 3,
                       TYPE_4 = 4};
    FunctionObj(DocWriterImpl &doc, Char const* spec, FunctionType fn_type,
                FunctionHandle const* funs=0, UInt nr_funs=0);

private:
    void on_before_output();
    void on_write(ObjFmt& fmt);

private:
    // all types
    FunctionType        m_fn_type;
    std::vector<Double> m_domain;
    std::vector<Double> m_range;
    DocWriterImpl&      m_doc;
    // type 2 fields
    std::vector<Double> m_c0;
    std::vector<Double> m_c1;
    int m_interpolation_exponent;
    // type 3 fields
    std::vector<FunctionHandle> m_functions;
    std::vector<Double>   m_bounds;
    std::vector<Double>   m_encode;
    std::vector<IndirectObjectRef> m_function_refs;
    // type 4 - stream
    scoped_ptr<ContentStream> m_stream;
    // all types except 4
    shared_ptr<IndirectObjectCallback> m_callback;
};

//
//
//
FunctionObj::FunctionObj(DocWriterImpl &doc, Char const* spec, FunctionType fn_type,
                         FunctionHandle const* funs, UInt nr_funs)
    : m_fn_type(fn_type)
    , m_doc(doc)
{
    JAG_PRECONDITION((fn_type != TYPE_3) || (funs && (nr_funs > 1)));
    try
    {
        ParsedResult const& p=
            parse_options(spec, ParseArgs(&g_fn_keywords));

        // parse options common for all function types
        bool domain_required = (m_fn_type == TYPE_4);

        parse_array(p, FN_DOMAIN, m_domain, !domain_required);
        parse_array(p, FN_RANGE, m_range);
        if (m_range.empty() && (m_fn_type == TYPE_0 || m_fn_type == TYPE_4))
            throw exception_invalid_value(msg_fn_range_not_set()) << JAGLOC;

        // parse options specific for the given function type
        switch(m_fn_type)
        {
        case TYPE_2: {
            parse_array(p, FN_C0, m_c0);
            parse_array(p, FN_C1, m_c1);
            std::size_t c0_len = m_c0.empty() ? 1 : m_c0.size();
            std::size_t c1_len = m_c1.empty() ? 1 : m_c1.size();
            if (c0_len != c1_len) {
                throw exception_invalid_value(msg_fn_inconsistent_c0_c1()) << JAGLOC;
            }
            m_interpolation_exponent = p.to_<int>(FN_EXPONENT, 1);
            break;
        }

        case TYPE_3:
            m_functions.resize(nr_funs);
            std::copy(funs, funs + nr_funs, m_functions.begin());
            parse_array(p, FN_BOUNDS, m_bounds, false, nr_funs - 1);
            parse_array(p, FN_ENCODE, m_encode, false, nr_funs * 2);
            break;

        case TYPE_4: {
            string_32_cow func_str(p.to_<string_32_cow>(FN_PS));
            if(func_str.empty())
            {
                throw exception_invalid_value(
                    msg_opt_not_specified()) << JAGLOC;
            }

            m_stream.reset(doc.create_content_stream().release());
            m_stream->stream().write(func_str.c_str(), func_str.size());
            break;
        }

        default:
            JAG_INTERNAL_ERROR;
        }

        if (!domain_required && m_domain.empty())
        {
            // provide common default values
            m_domain.resize(2);
            m_domain[0] = 0.0;
            m_domain[1] = 1.0;
        }

    }
    catch(exception const& exc)
    {
        throw exception_invalid_value(
            msg_invalid_function_spec(), &exc) << JAGLOC;
    }

    // decide which IIndirect implementation will be passed to
    // IndirectObjectFwd
    if (m_fn_type==TYPE_4)
    {
        m_stream->set_writer_callback(
            bind(&FunctionObj::on_write, this, _1));
        reset_indirect_object_worker(m_stream.get());
    }
    else
    {
        m_callback.reset(
            new IndirectObjectCallback(
                doc, bind(&FunctionObj::on_write, this, _1)));
        reset_indirect_object_worker(m_callback);
    }
}


//
//
//
void FunctionObj::on_before_output()
{
    // TYPE_3: get references to functions
    if (m_fn_type == TYPE_3)
    {
        m_function_refs.resize(m_functions.size());
        ResourceManagement* resmgm = &m_doc.res_mgm();
        std::transform(m_functions.begin(),
                       m_functions.end(),
                       m_function_refs.begin(),
                       bind(&ResourceManagement::function_ref, resmgm, _1));
    }
}

//
//
//
void FunctionObj::on_write(ObjFmt& fmt)
{
    // if type 4 then we are called as a part of a stream dictionary; in other
    // cases we need to start/end the dictionary here
    if (m_fn_type != TYPE_4)
        fmt.dict_start();

    fmt.dict_key("FunctionType").space().output(m_fn_type);
    output_array("Domain", fmt, m_domain.begin(), m_domain.end());
    output_array("Range", fmt, m_range.begin(), m_range.end());
    // TYPE_4 done
    if(m_fn_type == TYPE_2)
    {
        output_array("C0", fmt, m_c0.begin(), m_c0.end());
        output_array("C1", fmt, m_c1.begin(), m_c1.end());
        fmt.dict_key("N").space().output(m_interpolation_exponent);
    }
    else if (m_fn_type == TYPE_3)
    {
        fmt.dict_key("Functions");
        output_ref_array(fmt, m_function_refs.begin(), m_function_refs.end());
        output_array("Bounds", fmt, m_bounds.begin(), m_bounds.end());
        output_array("Encode", fmt, m_encode.begin(), m_encode.end());
    }

    if (m_fn_type != TYPE_4)
        fmt.dict_end();
}





// ---------------------------------------------------------------------------
//                           Resource management
//

namespace
{
  template<class Handle, class Map, class Table>
  IndirectObjectRef const& resource_ref(Handle handle, Map& map, Table& table)
  {
      typename Map::iterator it = map.find(handle);
      if (it == map.end())
      {
          IIndirectObject& resource(table.lookup(handle));
          resource.output_definition();
          it = map.insert(std::make_pair(handle,
                                         IndirectObjectRef(resource))).first;
      }
      return it->second;
  };
} // namespace




/// ctor
ResourceManagement::ResourceManagement(DocWriterImpl& doc)
    : m_doc(doc)
    , m_font_management(doc)
{
}

//
//
//
ResourceManagement::~ResourceManagement()
{
    // delete canvases
    CanvasMap::iterator it = m_canvas_map.begin();
    for(; it != m_canvas_map.end(); ++it)
        delete it->first;
}


//////////////////////////////////////////////////////////////////////////
IImageMan& ResourceManagement::image_man()
{
    return *m_doc.resource_ctx().image_man();
}


//////////////////////////////////////////////////////////////////////////
// COLOR SPACES
//////////////////////////////////////////////////////////////////////////

//
//
//
PatternHandle
ResourceManagement::tiling_pattern_load(Char const* pattern, ICanvas* canvas)
{
    CanvasRecord* canvas_rec = canvas_record(canvas);
    if (!canvas_rec || !canvas_rec->can_be_shared)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    std::auto_ptr<TilingPatternImpl> new_one(
        new TilingPatternImpl(m_doc, pattern, canvas));

    PatternHandle ph = m_pattern_table.add(new_one.release());

    // unset 'can_be_shared' flag so that the canvas cannot be used in other
    // objects
    canvas_rec->can_be_shared = false;

    return ph;
}


//
//
//
IIndirectObject& ResourceManagement::pattern(PatternHandle ph)
{
    return m_pattern_table.lookup(ph);
}


/// Adjusts a pattern matrix for the topdown mode.
///
/// Visits a pattern implementation, clones it and adds it to the
/// pattern table.
///  
class ResourceManagement::PatternVisitorTopdown
    : public VisitorNoOp
{
    ResourceManagement& m_res_mgm;
    double m_page_height;
    PatternHandle m_handle;
    
public:
    PatternVisitorTopdown(ResourceManagement& res_mgm, double page_height)
        : m_res_mgm(res_mgm)
        , m_page_height(page_height)
    {
    }
    
    PatternHandle handle() const {
        JAG_PRECONDITION(is_valid(m_handle));
        return m_handle;
    }
    
public: // visitor methods
    //
    bool visit(TilingPatternImpl& visited)
    {
        // the pattern canvas cannot be shared since the pattern matrix is part
        // of the canvas' content stream definition
        CanvasImpl* canvas =
            checked_static_cast<CanvasImpl*>(m_res_mgm.canvas_create());

        CanvasImpl* visited_canvas =
            checked_static_cast<CanvasImpl*>(visited.canvas());

        visited_canvas->copy_to(*canvas);
        std::auto_ptr<TilingPatternImpl> pattern(
            new TilingPatternImpl(m_res_mgm.m_doc,
                                  visited.definition_string(),
                                  canvas));

        adjust_matrix(pattern.get(), visited);
        m_handle = m_res_mgm.m_pattern_table.add(pattern.release());
        return true;
    }

    // 
    bool visit(ShadingPatternImpl& visited)
    {
        std::auto_ptr<ShadingPatternImpl> pattern(
            new ShadingPatternImpl(m_res_mgm.m_doc,
                                   visited.definition_string(),
                                   visited.shading_handle()));

        adjust_matrix(pattern.get(), visited);
        m_handle = m_res_mgm.m_pattern_table.add(pattern.release());
        return true;
    }

private:
    void adjust_matrix(PatternBase* pattern, PatternBase const& visited)
    {
        // adjust the pattern matrix
        if (visited.matrix().empty())
        {
            pattern->matrix(trans_affine_t(1, 0, 0, -1, 0, m_page_height));
        }
        else
        {
            JAG_ASSERT(visited.matrix().size() == 6);
            trans_affine_t mtx_orig(&visited.matrix()[0]);
            trans_affine_t mtx(1, 0, 0, -1, 0, m_page_height);
            mtx *= mtx_orig;
            pattern->matrix(mtx);
        }
    }
};


//
//
// 
bool operator<(ResourceManagement::PatternAndPageHeight const&lhs,
               ResourceManagement::PatternAndPageHeight const&rhs)
{
    if (lhs.first > rhs.first)
        return false;

    if (lhs.second < lhs.second)
        return true;

    return false;
}

//
//
// 
IndirectObjectRef const&
ResourceManagement::pattern_ref(PatternHandle ph, double page_height)
{
    if (page_height == 0.0)
        return resource_ref(ph, m_patterns, m_pattern_table);

    // In the topdown mode, the pattern's matrix must be
    // adjusted. m_patterns_by_page_height maintains {(orig_pattern,
    // page_height): adjusted_pattern} dictionary.
    typedef PatternsByPageHeight::iterator It;
    const PatternAndPageHeight key(ph, page_height);
    It found = m_patterns_by_page_height.find(key);
    if (found == m_patterns_by_page_height.end())
    {
        PatternVisitorTopdown visitor(*this, page_height);
        pattern(ph).accept(visitor);
        found = m_patterns_by_page_height.insert(
            PatternsByPageHeight::value_type(key,
                                             visitor.handle())).first;
    }

    return resource_ref(found->second, m_patterns, m_pattern_table);
}



//
//
//
IndirectObjectRef const& ResourceManagement::shading_ref(ShadingHandle sh)
{
    return resource_ref(sh, m_shadings, m_shading_table);
}


//
//
//
PatternHandle
ResourceManagement::shading_pattern_load(Char const* pattern,
                                         ColorSpaceHandle cs,
                                         FunctionHandle const* funcs,
                                         UInt num_functions)
{
    // the spec string is parsed twice; it would be possible to share
    // ParsedResult instance created in ShadingImpl but that would introduce
    // dependencies on spirit
    ShadingHandle shading_handle = m_shading_table.add(
        new ShadingImpl(m_doc, pattern, cs, funcs, num_functions));

    PatternHandle pattern_handle = m_pattern_table.add(
        new ShadingPatternImpl(m_doc, pattern, shading_handle));

    m_pattern_to_shading.insert(std::make_pair(pattern_handle, shading_handle));
    return pattern_handle;
}

//
//
//
ShadingHandle
ResourceManagement::shading_from_pattern(PatternHandle handle)
{
    PatternToShadingMap::iterator it = m_pattern_to_shading.find(handle);
    if (it == m_pattern_to_shading.end())
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    return it->second;
}


/**
 * @brief Retrives color space reference.
 *
 * @param handle color space
 * @return Reference to the color space.
 *
 * Note, that ColorSpaceHandle can represent either a color space
 * residing in the ColorSpace Manager or a trivial namespace (e.g.
 * DEVICE_RGB) that does not have any record in CS Manager. The latter
 * color space handles should not be passed to this method.
 **/
IndirectObjectRef const&
ResourceManagement::color_space_ref(ColorSpaceHandle handle)
{
    return reference_provider<ColorSpaceObj>(
        handle
        , m_color_spaces
        , identity<ColorSpaceHandle>()
        , m_doc);
}


//////////////////////////////////////////////////////////////////////////
// IMAGES
//////////////////////////////////////////////////////////////////////////


/**
 * @brief retrieves indirect object containing given image
 *
 * @param image_id image
 *
 * image is outputted during the first call requiring indirect object
 * of that image
 */
IndirectObjectRef const& ResourceManagement::image_reference(ImageHandle image_id)
{
    return reference_provider<ImageXObject>(
          image_id
        , m_images
        , bind(&IImageMan::image_data, &image_man(), _1)
        , m_doc
   );
}


//////////////////////////////////////////////////////////////////////////
// Functions
//////////////////////////////////////////////////////////////////////////

IndirectObjectRef const& ResourceManagement::function_ref(FunctionHandle handle)
{
    FunctionsMap::iterator it = m_functions.find(handle);
    if (it == m_functions.end())
    {
        IIndirectObject& fn(m_fun_table.lookup(handle));
        fn.output_definition();
        it = m_functions.insert(
            std::make_pair(handle, IndirectObjectRef(fn))).first;
    }
    return it->second;
}


//
//
//
FunctionHandle ResourceManagement::function_2_load(Char const* fun)
{
    return m_fun_table.add(new FunctionObj(m_doc, fun, FunctionObj::TYPE_2));
}

//
//
//
FunctionHandle ResourceManagement::function_4_load(Char const* fun)
{
    return m_fun_table.add(new FunctionObj(m_doc, fun, FunctionObj::TYPE_4));
}

FunctionHandle ResourceManagement::function_3_load(Char const* fun,
                               FunctionHandle const* funs, UInt nr_funs)
{
    return m_fun_table.add(new FunctionObj(m_doc, fun,
                                           FunctionObj::TYPE_3, funs, nr_funs));
}





//////////////////////////////////////////////////////////////////////////
// IMAGE MASKS
//////////////////////////////////////////////////////////////////////////

/**
 * @brief retrieves indirect object containing given image mask
 *
 * @param imagemask image mask
 *
 * image mask is outputted during the first call requiring indirect object
 * of that image mask
 */
IndirectObjectRef const&  ResourceManagement::image_mask_reference(ImageMaskHandle imagemask)
{
    return reference_provider<ImageXObjectMask>(
          imagemask
        , m_image_masks
        , bind(&IImageMan::image_mask_data, &image_man(), _1)
        , m_doc
   );
}





//////////////////////////////////////////////////////////////////////////
GraphicsStateHandle ResourceManagement::register_graphics_state(GraphicsStateDictionary const& gs_dict)
{
    GraphicsStateToHandle::iterator it = m_gs_to_handle.find(gs_dict);
    if (it == m_gs_to_handle.end())
    {
        GraphicsStateHandle result(m_handle_to_gs.register_resource(gs_dict));
        m_gs_to_handle.insert(GraphicsStateToHandle::value_type(gs_dict, result));
        return result;
    }

    return it->second;
}

/**
 * @brief retrieves indirect object containing given graphics state
 *
 * @param gs_handle graphics state handle
 *
 * graphics state is outputted during the first call requiring indirect object
 * of that graphics state
 */
IndirectObjectRef ResourceManagement::graphics_state_reference(GraphicsStateHandle gs_handle)
{
    typedef GSHandleToGSDict::record_t const& (GSHandleToGSDict::*overload_t)(GSHandleToGSDict::handle_t) const;

    return reference_provider<GraphicsStateDictionaryObject>(
        gs_handle
        , m_gs_to_obj
        , bind(static_cast<overload_t>(&GSHandleToGSDict::resource_record), &m_handle_to_gs, _1)
        , m_doc
   );
}


/// outputs queued resources
void ResourceManagement::finalize()
{
    m_font_management.output_fonts();
}


//
//
//
ICanvas* ResourceManagement::canvas_create() const
{
    std::auto_ptr<CanvasImpl> canvas(m_doc.create_canvas_impl());
    CanvasImpl* canvas_raw = canvas.get();
    m_canvas_map.insert(
        CanvasMap::value_type(
            static_cast<ICanvas*>(canvas.release()),
            CanvasRecord()));

    return canvas_raw;
}


//
// Gives reference to a shared canvas
//
IndirectObjectRef& ResourceManagement::canvas_ref(ICanvas* canvas)
{
    CanvasRecord* canvas_rec = canvas_record(canvas);
    JAG_PRECONDITION(
        // known canvas
        canvas_rec &&
        // cannot be private
        canvas_rec->can_be_shared);

    if (!is_valid(canvas_rec->ref))
    {
        CanvasImpl* canvas_impl = checked_static_cast<CanvasImpl*>(canvas);
        canvas_impl->output_definition();
        canvas_rec->ref.reset(*canvas_impl);
    }

    JAG_ASSERT(is_valid(canvas_rec->ref));
    return canvas_rec->ref;
}

//
//
//
ResourceManagement::CanvasRecord*
ResourceManagement::canvas_record(ICanvas* canvas)
{
    CanvasMap::iterator it = m_canvas_map.find(canvas);
    return it != m_canvas_map.end()
        ? &it->second
        : 0;
}


}} //namespace jag::pdf
