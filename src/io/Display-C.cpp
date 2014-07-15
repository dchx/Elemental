/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"
#include "El-C.h"
using namespace El;

extern "C" {

#define C_PROTO(SIG,T) \
  /* Matrix */ \
  ElError ElDisplayMatrix_ ## SIG \
  ( ElConstMatrix_ ## SIG AHandle, const char* title ) \
  { EL_TRY( Display( *Reinterpret(AHandle), std::string(title) ) ) } \
  /* AbstractDistMatrix */ \
  ElError ElDisplayDistMatrix_ ## SIG \
  ( ElConstDistMatrix_ ## SIG AHandle, const char* title ) \
  { EL_TRY( Display( *Reinterpret(AHandle), std::string(title) ) ) }

#include "El/macros/CInstantiate.h"

} // extern "C"