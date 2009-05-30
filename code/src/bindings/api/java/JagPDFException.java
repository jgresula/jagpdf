// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


// implementation of JagPDFException which is thrown by JagPDF.

package com.jagpdf;

public final class JagPDFException extends RuntimeException
{
    public JagPDFException() {}
    public JagPDFException(String msg) { super(msg); }
}
