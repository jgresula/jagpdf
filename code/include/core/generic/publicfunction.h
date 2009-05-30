// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __PUBLICFUNCTION_H_JG2025__
#define __PUBLICFUNCTION_H_JG2025__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <stdexcept>

// todo generate
// ERR_UNKNOWN_EXCEPTION
// ERR_OUT_OF_MEMORY
// put the following to the macros below



namespace jag
{
const jag::UInt INF_SUCCESS = 0;
const jag::UInt ERR_UNKNOWN_EXCEPTION = 1;
const jag::UInt ERR_OUT_OF_MEMORY        = 1;


void store_error(std::exception const& exc, JAGAPI_RESULT code);

} // namespace jag


#define EXPORTED_FUNC_PROLOGUE try {


#define EXPORTED_FUNC_EPILOGUE }                            \
    catch (std::bad_alloc& exc) {                            \
        store_error(exc.what(), ERR_OUT_OF_MEMORY);        \
        return ERR_OUT_OF_MEMORY;                            \
    }                                                        \
    catch (exception_base& exc) {                            \
        store_error(exc.what(), exc.err_code());            \
        return exc.err_code();                                \
    }                                                        \
    catch (std::exception& exc) {                            \
        store_error(exc.what(), ERR_UNKNOWN_EXCEPTION);    \
        return ERR_UNKNOWN_EXCEPTION;                        \
    }                                                        \
    return INF_SUCCESS;


#define EXPORTED_FUNC_EPILOGUE_HANDLE }                        \
    catch (std::bad_alloc& exc) {                            \
        store_error(exc.what(), ERR_OUT_OF_MEMORY);        \
        return 0;                                            \
    }                                                        \
    catch (exception_base& exc) {                            \
        store_error(exc.what(), exc.err_code());            \
        return 0;                                            \
    }                                                        \
    catch (std::exception& exc) {                            \
        store_error(exc.what(), ERR_UNKNOWN_EXCEPTION);    \
        return 0;                                            \
    }

#define EXPORTED_FUNC_EPILOGUE_(UNK_TYPE) }                    \
    catch (std::bad_alloc& exc) {                            \
        store_error(exc.what(), ERR_OUT_OF_MEMORY);        \
        return error_value_for_type<UNK_TYPE>();            \
    }                                                        \
    catch (exception_base& exc) {                            \
        store_error(exc.what(), exc.err_code());            \
        return error_value_for_type<UNK_TYPE>();            \
    }                                                        \
    catch (std::exception& exc) {                            \
        store_error(exc.what(), ERR_UNKNOWN_EXCEPTION);    \
        return error_value_for_type<UNK_TYPE>();            \
    }


#endif //__PUBLICFUNCTION_H_JG2025__

