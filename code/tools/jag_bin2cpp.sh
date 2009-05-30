#!/bin/bash

# Copyright (c) 2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#
bin2cpp.py ..\src\resources\resfiles\iccprofiles\AdobeRGB.icc ..\src\resources\resfiles\icc_adobergb.cpp icc_adobe_rgb
bin2cpp.py ..\src\resources\resfiles\iccprofiles\sRGB.icm ..\src\resources\resfiles\icc_srgb.cpp icc_srgb