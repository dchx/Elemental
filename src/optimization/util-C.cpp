/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"
#include "El.h"
using namespace El;

extern "C" {

#define C_PROTO(SIG,SIGBASE,F) \
  /* Coherence
     --------- */ \
  ElError ElCoherence_ ## SIG \
  ( ElConstMatrix_ ## SIG A, Base<F>* coherence ) \
  { EL_TRY( *coherence = Coherence(*CReflect(A)) ) } \
  ElError ElCoherenceDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG A, Base<F>* coherence ) \
  { EL_TRY( *coherence = Coherence(*CReflect(A)) ) } \
  /* Covariance
     ---------- */ \
  ElError ElCovariance_ ## SIG \
  ( ElConstMatrix_ ## SIG D, ElMatrix_ ## SIG S ) \
  { EL_TRY( Covariance( *CReflect(D), *CReflect(S) ) ) } \
  ElError ElCovarianceDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG D, ElDistMatrix_ ## SIG S ) \
  { EL_TRY( Covariance( *CReflect(D), *CReflect(S) ) ) } \
  /* Log barrier
     ----------- */ \
  ElError ElLogBarrier_ ## SIG \
  ( ElUpperOrLower uplo, ElConstMatrix_ ## SIG A, Base<F>* barrier ) \
  { EL_TRY( *barrier = LogBarrier( CReflect(uplo), *CReflect(A) ) ) } \
  ElError ElLogBarrierDist_ ## SIG \
  ( ElUpperOrLower uplo, ElConstDistMatrix_ ## SIG A, Base<F>* barrier ) \
  { EL_TRY( *barrier = LogBarrier( CReflect(uplo), *CReflect(A) ) ) } \
  /* Log-det divergence
     ------------------ */ \
  ElError ElLogDetDiv_ ## SIG \
  ( ElUpperOrLower uplo, ElConstMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG B, Base<F>* div ) \
  { EL_TRY( *div = LogDetDiv( CReflect(uplo), *CReflect(A), *CReflect(B) ) ) } \
  ElError ElLogDetDivDist_ ## SIG \
  ( ElUpperOrLower uplo, ElConstDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG B, Base<F>* div ) \
  { EL_TRY( *div = LogDetDiv( CReflect(uplo), *CReflect(A), *CReflect(B) ) ) }

#define C_PROTO_REAL(SIG,REAL) \
  C_PROTO(SIG,SIG,REAL) \
  /* SOC dots
     -------- */ \
  ElError ElSOCDots_ ## SIG \
  ( ElConstMatrix_ ## SIG x, ElConstMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElConstMatrix_i orders, ElConstMatrix_i firstInds ) \
  { EL_TRY( \
      SOCDots \
      ( *CReflect(x), *CReflect(y), *CReflect(z), \
        *CReflect(orders), *CReflect(firstInds) ) ) } \
  ElError ElSOCDotsDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG x, ElConstDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z, \
    ElConstDistMatrix_i orders, ElConstDistMatrix_i firstInds, \
    ElInt cutoff ) \
  { EL_TRY( \
      SOCDots \
      ( *CReflect(x), *CReflect(y), *CReflect(z), \
        *CReflect(orders), *CReflect(firstInds), cutoff ) ) } \
  ElError ElSOCDotsDistMultiVec_ ## SIG \
  ( ElConstDistMultiVec_ ## SIG x, ElConstDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z, \
    ElConstDistMultiVec_i orders, ElConstDistMultiVec_i firstInds, \
    ElInt cutoff ) \
  { EL_TRY( \
      SOCDots \
      ( *CReflect(x), *CReflect(y), *CReflect(z), \
        *CReflect(orders), *CReflect(firstInds), cutoff ) ) } \
  /* SOC Broadcast
     ------------- */ \
  ElError ElSOCBroadcast_ ## SIG \
  ( ElMatrix_ ## SIG x, \
    ElConstMatrix_i orders, ElConstMatrix_i firstInds ) \
  { EL_TRY( \
      SOCBroadcast( *CReflect(x), *CReflect(orders), *CReflect(firstInds) ) \
    ) } \
  ElError ElSOCBroadcastDist_ ## SIG \
  ( ElDistMatrix_ ## SIG x, \
    ElConstDistMatrix_i orders, ElConstDistMatrix_i firstInds, \
    ElInt cutoff ) \
  { EL_TRY( \
      SOCBroadcast \
      ( *CReflect(x), *CReflect(orders), *CReflect(firstInds), cutoff ) ) } \
  ElError ElSOCBroadcastDistMultiVec_ ## SIG \
  ( ElDistMultiVec_ ## SIG x, \
    ElConstDistMultiVec_i orders, ElConstDistMultiVec_i firstInds, \
    ElInt cutoff ) \
  { EL_TRY( \
      SOCBroadcast \
      ( *CReflect(x), *CReflect(orders), *CReflect(firstInds), cutoff ) ) } \
  /* SOC Reflection
     -------------- */ \
  ElError ElSOCReflect_ ## SIG \
  ( ElMatrix_ ## SIG x, \
    ElConstMatrix_i orders, ElConstMatrix_i firstInds ) \
  { EL_TRY( SOCReflect( \
      *CReflect(x), *CReflect(orders), *CReflect(firstInds) ) ) } \
  ElError ElSOCReflectDist_ ## SIG \
  ( ElDistMatrix_ ## SIG x, \
    ElConstDistMatrix_i orders, ElConstDistMatrix_i firstInds ) \
  { EL_TRY( SOCReflect( \
      *CReflect(x), *CReflect(orders), *CReflect(firstInds) ) ) } \
  ElError ElSOCReflectDistMultiVec_ ## SIG \
  ( ElDistMultiVec_ ## SIG x, \
    ElConstDistMultiVec_i orders, ElConstDistMultiVec_i firstInds ) \
  { EL_TRY( SOCReflect( \
      *CReflect(x), *CReflect(orders), *CReflect(firstInds) ) ) } \
  /* SOC Determinants
     ---------------- */ \
  ElError ElSOCDets_ ## SIG \
  ( ElConstMatrix_ ## SIG x, ElMatrix_ ## SIG d, \
    ElConstMatrix_i orders, ElConstMatrix_i firstInds ) \
  { EL_TRY( SOCDets( *CReflect(x), *CReflect(d), *CReflect(orders), \
      *CReflect(firstInds) ) ) } \
  ElError ElSOCDetsDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG x, ElDistMatrix_ ## SIG d, \
    ElConstDistMatrix_i orders, ElConstDistMatrix_i firstInds, \
    ElInt cutoff ) \
  { EL_TRY( SOCDets( *CReflect(x), *CReflect(d), *CReflect(orders), \
      *CReflect(firstInds), cutoff ) ) } \
  ElError ElSOCDetsDistMultiVec_ ## SIG \
  ( ElConstDistMultiVec_ ## SIG x, ElDistMultiVec_ ## SIG d, \
    ElConstDistMultiVec_i orders, ElConstDistMultiVec_i firstInds, \
    ElInt cutoff ) \
  { EL_TRY( SOCDets( *CReflect(x), *CReflect(d), *CReflect(orders), \
      *CReflect(firstInds), cutoff ) ) } \
  /* Num non-SOC
     ----------- */ \
  ElError ElNumNonSOC_ ## SIG \
  ( ElConstMatrix_ ## SIG x, \
    ElConstMatrix_i orders, ElConstMatrix_i firstInds, \
    ElInt* numNonSOC ) \
  { EL_TRY( *numNonSOC = NumNonSOC( *CReflect(x), *CReflect(orders), \
      *CReflect(firstInds) ) ) } \
  ElError ElNumNonSOCDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG x, \
    ElConstDistMatrix_i orders, ElConstDistMatrix_i firstInds, \
    ElInt cutoff, ElInt* numNonSOC ) \
  { EL_TRY( *numNonSOC = NumNonSOC( *CReflect(x), *CReflect(orders), \
      *CReflect(firstInds), cutoff ) ) } \
  ElError ElNumNonSOCDistMultiVec_ ## SIG \
  ( ElConstDistMultiVec_ ## SIG x, \
    ElConstDistMultiVec_i orders, ElConstDistMultiVec_i firstInds, \
    ElInt cutoff, ElInt* numNonSOC ) \
  { EL_TRY( *numNonSOC = NumNonSOC( *CReflect(x), *CReflect(orders), \
      *CReflect(firstInds), cutoff ) ) } \
  /* SOC Apply quadratic
     ------------------- */ \
  ElError ElSOCApplyQuadratic_ ## SIG \
  ( ElConstMatrix_ ## SIG x, ElConstMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElConstMatrix_i orders, ElConstMatrix_i firstInds ) \
  { EL_TRY( SOCApplyQuadratic( *CReflect(x), *CReflect(y), *CReflect(z), \
      *CReflect(orders), *CReflect(firstInds) ) ) } \
  ElError ElSOCApplyQuadraticDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG x, ElConstDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z, \
    ElConstDistMatrix_i orders, ElConstDistMatrix_i firstInds, \
    ElInt cutoff ) \
  { EL_TRY( SOCApplyQuadratic( *CReflect(x), *CReflect(y), *CReflect(z), \
      *CReflect(orders), *CReflect(firstInds), cutoff ) ) } \
  ElError ElSOCApplyQuadraticDistMultiVec_ ## SIG \
  ( ElConstDistMultiVec_ ## SIG x, ElConstDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z, \
    ElConstDistMultiVec_i orders, ElConstDistMultiVec_i firstInds, \
    ElInt cutoff ) \
  { EL_TRY( SOCApplyQuadratic( *CReflect(x), *CReflect(y), *CReflect(z), \
      *CReflect(orders), *CReflect(firstInds), cutoff ) ) } \
  /* SOC Inverse
     ----------- */ \
  ElError ElSOCInverse_ ## SIG \
  ( ElConstMatrix_ ## SIG x, ElMatrix_ ## SIG xInv, \
    ElConstMatrix_i orders, ElConstMatrix_i firstInds ) \
  { EL_TRY( SOCInverse( *CReflect(x), *CReflect(xInv), *CReflect(orders), \
      *CReflect(firstInds) ) ) } \
  ElError ElSOCInverseDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG x, ElDistMatrix_ ## SIG xInv, \
    ElConstDistMatrix_i orders, ElConstDistMatrix_i firstInds, \
    ElInt cutoff ) \
  { EL_TRY( SOCInverse( *CReflect(x), *CReflect(xInv), *CReflect(orders), \
      *CReflect(firstInds), cutoff ) ) } \
  ElError ElSOCInverseDistMultiVec_ ## SIG \
  ( ElConstDistMultiVec_ ## SIG x, ElDistMultiVec_ ## SIG xInv, \
    ElConstDistMultiVec_i orders, ElConstDistMultiVec_i firstInds, \
    ElInt cutoff ) \
  { EL_TRY( SOCInverse( *CReflect(x), *CReflect(xInv), *CReflect(orders), \
      *CReflect(firstInds), cutoff ) ) } \
  /* SOC Square-root
     --------------- */ \
  ElError ElSOCSquareRoot_ ## SIG \
  ( ElConstMatrix_ ## SIG x, ElMatrix_ ## SIG xRoot, \
    ElConstMatrix_i orders, ElConstMatrix_i firstInds ) \
  { EL_TRY( SOCSquareRoot( *CReflect(x), *CReflect(xRoot), *CReflect(orders), \
      *CReflect(firstInds) ) ) } \
  ElError ElSOCSquareRootDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG x, ElDistMatrix_ ## SIG xRoot, \
    ElConstDistMatrix_i orders, ElConstDistMatrix_i firstInds, \
    ElInt cutoff ) \
  { EL_TRY( SOCSquareRoot( *CReflect(x), *CReflect(xRoot), *CReflect(orders), \
      *CReflect(firstInds), cutoff ) ) } \
  ElError ElSOCSquareRootDistMultiVec_ ## SIG \
  ( ElConstDistMultiVec_ ## SIG x, ElDistMultiVec_ ## SIG xRoot, \
    ElConstDistMultiVec_i orders, ElConstDistMultiVec_i firstInds, \
    ElInt cutoff ) \
  { EL_TRY( SOCSquareRoot( *CReflect(x), *CReflect(xRoot), *CReflect(orders), \
      *CReflect(firstInds), cutoff ) ) }

#define EL_NO_INT_PROTO
#include "El/macros/CInstantiate.h"

} // extern "C"
