// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __INDIRECTOBJECTFWD_JG1236_H__
#define __INDIRECTOBJECTFWD_JG1236_H__

#include "interfaces/indirect_object.h"
#include <core/generic/assert.h>
#include <core/generic/null_deleter.h>
#include <boost/shared_ptr.hpp>

namespace jag {
namespace pdf {


/**
 * @brief A forwarding IIndirectObject implementation.
 *
 * Base class for classes that aggregate some IIndirectObject
 * and want to propagate it to their interface.
 */
class IndirectObjectFwd
    : public IIndirectObject
{
public:
    IndirectObjectFwd() {}
    IndirectObjectFwd(boost::shared_ptr<IIndirectObject> fwd)
        : m_fwd(fwd)
    {}
    void reset_indirect_object_worker(boost::shared_ptr<IIndirectObject> fwd) {
        m_fwd = fwd;
    }

    void reset_indirect_object_worker(IIndirectObject* fwd) {
        m_fwd = boost::shared_ptr<IIndirectObject>(fwd, null_deleter);
    }


public: //IIndirectObject
    void output_definition() {
        JAG_PRECONDITION(m_fwd);
        on_before_output();
        m_fwd->output_definition();
    }


    Int object_number() const {
        JAG_PRECONDITION(m_fwd);
        return m_fwd->object_number();
    }


    Int generation_number() const {
        JAG_PRECONDITION(m_fwd);
        return m_fwd->generation_number();
    }

protected:
    /**
     * @brief Before output hook.
     *
     * Derived class might override this method. It gets invoked
     * just before outputting the worker object.
     */
    virtual void on_before_output() {};

private:
    // ? is it necessary to have shared_ptr here? Would not be enough to have
    // just a reference?
    boost::shared_ptr<IIndirectObject> m_fwd;
};

}} // namespace jag::pdf

#endif // __INDIRECTOBJECTFWD_JG1236_H__
/** EOF @file */
