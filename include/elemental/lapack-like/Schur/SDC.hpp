/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_LAPACK_SCHUR_SDC_HPP
#define ELEM_LAPACK_SCHUR_SDC_HPP

#include "elemental/blas-like/level1/Axpy.hpp"
#include "elemental/blas-like/level1/Scale.hpp"
#include "elemental/blas-like/level1/UpdateDiagonal.hpp"
#include "elemental/lapack-like/Norm/Infinity.hpp"
#include "elemental/lapack-like/Norm/One.hpp"
#include "elemental/lapack-like/QR.hpp"
#include "elemental/lapack-like/Sign.hpp"
#include "elemental/lapack-like/Trace.hpp"
#include "elemental/matrices/Haar.hpp"
#include "elemental/matrices/Identity.hpp"

// See Z. Bai, J. Demmel, J. Dongarra, A. Petitet, H. Robinson, and K. Stanley's
// "The spectral decomposition of nonsymmetric matrices on distributed memory
// parallel computers". Currently available at:
// www.netlib.org/lapack/lawnspdf/lawn91.pdf
//
// as well as the improved version, which avoids pivoted QR, in J. Demmel, 
// I. Dumitriu, and O. Holtz, "Fast linear algebra is stable", 2007.
// www.netlib.org/lapack/lawnspdf/lawn186.pdf

namespace elem {
namespace schur {

template<typename F>
inline ValueInt<BASE(F)>
ComputePartition( Matrix<F>& A )
{
#ifndef RELEASE
    CallStackEntry cse("schur::ComputePartition");
#endif
    typedef BASE(F) Real;
    const Int n = A.Height();
    if( n == 0 ) 
    {
        ValueInt<Real> part;
        part.value = -1;
        part.index = -1;
        return;
    }

    // Compute the sets of row and column sums
    std::vector<Real> colSums(n-1,0), rowSums(n-1,0);
    for( Int j=0; j<n-1; ++j )
        for( Int i=j+1; i<n; ++i )
            colSums[j] += Abs( A.Get(i,j) ); 
    for( Int i=1; i<n-1; ++i )
        for( Int j=0; j<i; ++j )
            rowSums[i-1] += Abs( A.Get(i,j) );

    // Compute the list of norms and its minimum value/index
    ValueInt<Real> part;
    std::vector<Real> norms(n-1);
    norms[0] = colSums[0];
    part.value = norms[0];
    part.index = 1;
    for( Int j=1; j<n-1; ++j )
    {
        norms[j] = norms[j-1] + colSums[j] - rowSums[j-1];
        if( norms[j] < part.value )
        {
            part.value = norms[j];
            part.index = j+1;
        }
    }

    return part;
}

// The current implementation requires O(n^2/p + n lg p) work. Since the
// matrix-matrix multiplication alone requires O(n^3/p) work, and n <= p for
// most practical computations, it is at least O(n^2) work, which should dwarf
// the O(n lg p) unparallelized component of this algorithm.
template<typename F>
inline ValueInt<BASE(F)>
ComputePartition( DistMatrix<F>& A )
{
#ifndef RELEASE
    CallStackEntry cse("schur::ComputePartition");
#endif
    typedef BASE(F) Real;
    const Grid& g = A.Grid();
    const Int n = A.Height();
    if( n == 0 ) 
    {
        ValueInt<Real> part;
        part.value = -1;
        part.index = -1;
        return part;
    }

    // Compute the sets of row and column sums
    std::vector<Real> colSums(n-1,0), rowSums(n-1,0);
    const Int mLocal = A.LocalHeight();
    const Int nLocal = A.LocalWidth();
    const Int rowShift = A.RowShift();
    const Int colShift = A.ColShift();
    const Int rowStride = A.RowStride();
    const Int colStride = A.ColStride();
    for( Int jLoc=0; jLoc<nLocal; ++jLoc )
    {
        const Int j = rowShift + jLoc*rowStride;
        if( j < n-1 )
        {
            for( Int iLoc=0; iLoc<mLocal; ++iLoc )
            {
                const Int i = colShift + iLoc*colStride;
                if( i > j )
                {
                    colSums[j] += Abs( A.GetLocal(iLoc,jLoc) ); 
                    rowSums[i-1] += Abs( A.GetLocal(iLoc,jLoc) );
                }
            }
        }
    }
    mpi::AllReduce( &colSums[0], n-1, g.VCComm() );
    mpi::AllReduce( &rowSums[0], n-1, g.VCComm() );

    // Compute the list of norms and its minimum value/index
    // TODO: Think of the proper way to parallelize this if necessary
    ValueInt<Real> part;
    std::vector<Real> norms(n-1);
    norms[0] = colSums[0];
    part.value = norms[0];
    part.index = 1;
    for( Int j=1; j<n-1; ++j )
    {
        norms[j] = norms[j-1] + colSums[j] - rowSums[j-1];
        if( norms[j] < part.value )
        {
            part.value = norms[j];
            part.index = j+1;
        }
    }

    return part;
}

// G should be a rational function of A. If returnQ=true, G will be set to
// the computed unitary matrix upon exit.
template<typename F>
inline ValueInt<BASE(F)>
SignDivide( Matrix<F>& A, Matrix<F>& G, bool returnQ=false )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SignDivide");
#endif
    typedef BASE(F) Real;
    const Int n = A.Height();

    // G := sgn(G)
    // G := 1/2 ( G + I )
    Sign( G );
    UpdateDiagonal( G, F(1) );
    Scale( F(1)/F(2), G );

    // Compute the pivoted QR decomposition of the spectral projection 
    Matrix<F> t;
    Matrix<Int> p;
    elem::QR( G, t, p );

    // A := Q^H A Q
    const Real oneA = OneNorm( A );
    if( returnQ )
    {
        Matrix<F> B;
        ExpandPackedReflectors( LOWER, VERTICAL, UNCONJUGATED, 0, G, t );
        Gemm( ADJOINT, NORMAL, F(1), G, A, B );
        Gemm( NORMAL, NORMAL, F(1), B, G, A );
    }
    else
    {
        qr::ApplyQ( LEFT, ADJOINT, G, t, A );
        qr::ApplyQ( RIGHT, NORMAL, G, t, A );
    }

    // Return || E21 ||1 / || A ||1 and the chosen rank
    ValueInt<Real> part = ComputePartition( A );
    part.value /= oneA;
    return part;
}

template<typename F>
inline ValueInt<BASE(F)>
SignDivide( DistMatrix<F>& A, DistMatrix<F>& G, bool returnQ=false )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SignDivide");
#endif
    typedef BASE(F) Real;
    const Grid& g = A.Grid();
    const Int n = A.Height();

    // G := sgn(G)
    // G := 1/2 ( G + I )
    Sign( G );
    UpdateDiagonal( G, F(1) );
    Scale( F(1)/F(2), G );

    // Compute the pivoted QR decomposition of the spectral projection 
    DistMatrix<F,MD,STAR> t(g);
    DistMatrix<Int,VR,STAR> p(g);
    elem::QR( G, t, p );

    // A := Q^H A Q
    const Real oneA = OneNorm( A );
    if( returnQ )
    {
        DistMatrix<F> B(g);
        ExpandPackedReflectors( LOWER, VERTICAL, UNCONJUGATED, 0, G, t );
        Gemm( ADJOINT, NORMAL, F(1), G, A, B );
        Gemm( NORMAL, NORMAL, F(1), B, G, A );
    }
    else
    {
        qr::ApplyQ( LEFT, ADJOINT, G, t, A );
        qr::ApplyQ( RIGHT, NORMAL, G, t, A );
    }

    // Return || E21 ||1 / || A ||1 and the chosen rank
    ValueInt<Real> part = ComputePartition( A );
    part.value /= oneA;
    return part;
}

template<typename F>
inline ValueInt<BASE(F)>
RandomizedSignDivide
( Matrix<F>& A, Matrix<F>& G, 
  bool returnQ=false, Int maxIts=10, BASE(F) relTol=0 )
{
#ifndef RELEASE
    CallStackEntry cse("schur::RandomizedSignDivide");
#endif
    typedef BASE(F) Real;
    const Int n = A.Height();
    const Real oneA = OneNorm( A );
    if( relTol == Real(0) )
        relTol = 50*n*lapack::MachineEpsilon<Real>();

    // S := sgn(G)
    // S := 1/2 ( S + I )
    Matrix<F> S( G );
    Sign( S );
    UpdateDiagonal( S, F(1) );
    Scale( F(1)/F(2), S );

    ValueInt<Real> part;
    Matrix<F> V, B, t;
    for( Int it=0; it<maxIts; ++it )
    {
        G = S;

        // Compute the RURV of the spectral projector
        ImplicitHaar( V, t, n );
        qr::ApplyQ( RIGHT, NORMAL, V, t, G );
        elem::QR( G, t );

        // A := Q^H A Q [and reuse space for V for keeping original A]
        V = A;
        if( returnQ )
        {
            ExpandPackedReflectors( LOWER, VERTICAL, UNCONJUGATED, 0, G, t );
            Gemm( ADJOINT, NORMAL, F(1), G, A, B );
            Gemm( NORMAL, NORMAL, F(1), B, G, A );
        }
        else
        {
            qr::ApplyQ( LEFT, ADJOINT, G, t, A );
            qr::ApplyQ( RIGHT, NORMAL, G, t, A );
        }

        // || E21 ||1 / || A ||1 and the chosen rank
        part = ComputePartition( A );
        part.value /= oneA;

        if( part.value <= relTol || it == maxIts-1 )
            break;
        else
            A = V;
    }
    return part;
}

template<typename F>
inline ValueInt<BASE(F)>
RandomizedSignDivide
( DistMatrix<F>& A, DistMatrix<F>& G, 
  bool returnQ=false, Int maxIts=10, BASE(F) relTol=0 )
{
#ifndef RELEASE
    CallStackEntry cse("schur::RandomizedSignDivide");
#endif
    typedef BASE(F) Real;
    const Grid& g = A.Grid();
    const Int n = A.Height();
    const Real oneA = OneNorm( A );
    if( relTol == Real(0) )
        relTol = 50*n*lapack::MachineEpsilon<Real>();

    // S := sgn(G)
    // S := 1/2 ( S + I )
    DistMatrix<F> S( G );
    Sign( S );
    UpdateDiagonal( S, F(1) );
    Scale( F(1)/F(2), S );

    ValueInt<Real> part;
    DistMatrix<F> V(g), B(g);
    DistMatrix<F,MD,STAR> t(g);
    for( Int it=0; it<maxIts; ++it )
    {
        G = S;

        // Compute the RURV of the spectral projector
        ImplicitHaar( V, t, n );
        qr::ApplyQ( RIGHT, NORMAL, V, t, G );
        elem::QR( G, t );

        // A := Q^H A Q [and reuse space for V for keeping original A]
        V = A;
        if( returnQ )
        {
            ExpandPackedReflectors( LOWER, VERTICAL, UNCONJUGATED, 0, G, t );
            Gemm( ADJOINT, NORMAL, F(1), G, A, B );
            Gemm( NORMAL, NORMAL, F(1), B, G, A );
        }
        else
        {
            qr::ApplyQ( LEFT, ADJOINT, G, t, A );
            qr::ApplyQ( RIGHT, NORMAL, G, t, A );
        }

        // || E21 ||1 / || A ||1 and the chosen rank
        part = ComputePartition( A );
        part.value /= oneA;

        if( part.value <= relTol || it == maxIts-1 )
            break;
        else
            A = V;
    }
    return part;
}

template<typename Real>
inline ValueInt<Real>
SpectralDivide( Matrix<Real>& A )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SpectralDivide");
#endif
    const Int n = A.Height();
    const Real gershCenter = Trace(A) / n;

    Matrix<Real> d;
    A.GetDiagonal( d );
    SetDiagonal( A, Real(0) );
    const Real offDiagInf = InfinityNorm(A);
    A.SetDiagonal( d );

    const Real shift = SampleBall<Real>(-gershCenter,Real(0.001)*offDiagInf);

    Matrix<Real> G( A );
    UpdateDiagonal( G, shift );

    //ValueInt<Real> part = SignDivide( A, G );
    ValueInt<Real> part = RandomizedSignDivide( A, G );

    // TODO: Retry or throw exception if part.value not small enough

    return part;
}

template<typename Real>
inline ValueInt<Real>
SpectralDivide( Matrix<Complex<Real> >& A )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SpectralDivide");
#endif
    typedef Complex<Real> F;
    const Int n = A.Height();
    const F gershCenter = Trace(A) / n;

    Matrix<F> d;
    A.GetDiagonal( d );
    SetDiagonal( A, F(0) );
    const Real offDiagInf = InfinityNorm(A);
    A.SetDiagonal( d );

    const F shift = SampleBall<F>(-gershCenter,Real(0.001)*offDiagInf);

    const Real angle = Uniform<Real>(0,2*Pi);
    const F gamma = F(Cos(angle),Sin(angle));

    Matrix<F> G( A );
    UpdateDiagonal( G, shift );
    Scale( gamma, G );

    //ValueInt<Real> part = SignDivide( A, G );
    ValueInt<Real> part = RandomizedSignDivide( A, G );

    // TODO: Retry or throw exception if part.value not small enough

    return part;
}

template<typename Real>
inline ValueInt<Real>
SpectralDivide( Matrix<Real>& A, Matrix<Real>& Q )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SpectralDivide");
#endif
    const Int n = A.Height();
    const Real gershCenter = Trace(A) / n;

    Matrix<Real> d;
    A.GetDiagonal( d );
    SetDiagonal( A, Real(0) );
    const Real offDiagInf = InfinityNorm(A);
    A.SetDiagonal( d );

    const Real shift = SampleBall<Real>(-gershCenter,Real(0.001)*offDiagInf);

    Q = A;
    UpdateDiagonal( Q, shift );

    //ValueInt<Real> part = SignDivide( A, Q, true );
    ValueInt<Real> part = RandomizedSignDivide( A, Q, true );

    // TODO: Retry or throw exception if part.value not small enough
    
    return part;
}

template<typename Real>
inline ValueInt<Real>
SpectralDivide( Matrix<Complex<Real> >& A, Matrix<Complex<Real> >& Q )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SpectralDivide");
#endif
    typedef Complex<Real> F;
    const Int n = A.Height();
    const F gershCenter = Trace(A) / n;

    Matrix<F> d;
    A.GetDiagonal( d );
    SetDiagonal( A, F(0) );
    const Real offDiagInf = InfinityNorm(A);
    A.SetDiagonal( d );

    const F shift = SampleBall<F>(-gershCenter,Real(0.001)*offDiagInf);

    const Real angle = Uniform<Real>(0,2*Pi);
    const F gamma = F(Cos(angle),Sin(angle));

    Q = A;
    UpdateDiagonal( Q, shift );
    Scale( gamma, Q );

    //ValueInt<Real> part = SignDivide( A, Q, true );
    ValueInt<Real> part = RandomizedSignDivide( A, Q, true );

    // TODO: Retry or throw exception if part.value not small enough

    return part;
}

template<typename Real>
inline ValueInt<Real>
SpectralDivide( DistMatrix<Real>& A )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SpectralDivide");
#endif
    const Int n = A.Height();
    const Real gershCenter = Trace(A) / n;

    DistMatrix<Real,MD,STAR> d(A.Grid());
    A.GetDiagonal( d );
    SetDiagonal( A, Real(0) );
    const Real offDiagInf = InfinityNorm(A);
    A.SetDiagonal( d );

    Real shift = SampleBall<Real>(-gershCenter,Real(0.001)*offDiagInf);
    mpi::Broadcast( shift, 0, A.Grid().VCComm() );

    DistMatrix<Real> G( A );
    UpdateDiagonal( G, shift );

    //ValueInt<Real> part = SignDivide( A, G );
    ValueInt<Real> part = RandomizedSignDivide( A, G );

    // TODO: Retry or throw exception if part.value not small enough

    return part;
}

template<typename Real>
inline ValueInt<Real>
SpectralDivide( DistMatrix<Complex<Real> >& A )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SpectralDivide");
#endif
    typedef Complex<Real> F;
    const Int n = A.Height();
    const F gershCenter = Trace(A) / n;

    DistMatrix<F,MD,STAR> d(A.Grid());
    A.GetDiagonal( d );
    SetDiagonal( A, F(0) );
    const Real offDiagInf = InfinityNorm(A);
    A.SetDiagonal( d );

    F shift = SampleBall<F>(-gershCenter,Real(0.001)*offDiagInf);
    mpi::Broadcast( shift, 0, A.Grid().VCComm() );

    const Real angle = Uniform<Real>(0,2*Pi);
    F gamma = F(Cos(angle),Sin(angle));
    mpi::Broadcast( gamma, 0, A.Grid().VCComm() );

    DistMatrix<F> G( A );
    UpdateDiagonal( G, shift );
    Scale( gamma, G );

    //ValueInt<Real> part = SignDivide( A, G );
    ValueInt<Real> part = RandomizedSignDivide( A, G );

    // TODO: Retry or throw exception if part.value not small enough

    return part;
}

template<typename Real>
inline ValueInt<Real>
SpectralDivide( DistMatrix<Real>& A, DistMatrix<Real>& Q )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SpectralDivide");
#endif
    const Int n = A.Height();
    const Real gershCenter = Trace(A) / n;

    DistMatrix<Real,MD,STAR> d(A.Grid());
    A.GetDiagonal( d );
    SetDiagonal( A, Real(0) );
    const Real offDiagInf = InfinityNorm(A);
    A.SetDiagonal( d );

    Real shift = SampleBall<Real>(-gershCenter,Real(0.001)*offDiagInf);
    mpi::Broadcast( shift, 0, A.Grid().VCComm() );

    Q = A;
    UpdateDiagonal( Q, shift );

    //ValueInt<Real> part = SignDivide( A, Q, true );
    ValueInt<Real> part = RandomizedSignDivide( A, Q, true );

    // TODO: Retry or throw exception if part.value not small enough

    return part;
}

template<typename Real>
inline ValueInt<Real>
SpectralDivide( DistMatrix<Complex<Real> >& A, DistMatrix<Complex<Real> >& Q )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SpectralDivide");
#endif
    typedef Complex<Real> F;
    const Int n = A.Height();
    const F gershCenter = Trace(A) / n;

    DistMatrix<F,MD,STAR> d(A.Grid());
    A.GetDiagonal( d );
    SetDiagonal( A, F(0) );
    const Real offDiagInf = InfinityNorm(A);
    A.SetDiagonal( d );

    F shift = SampleBall<F>(-gershCenter,Real(0.001)*offDiagInf);
    mpi::Broadcast( shift, 0, A.Grid().VCComm() );

    const Real angle = Uniform<Real>(0,2*Pi);
    F gamma = F(Cos(angle),Sin(angle));
    mpi::Broadcast( gamma, 0, A.Grid().VCComm() );

    Q = A;
    UpdateDiagonal( Q, shift );
    Scale( gamma, Q );

    //ValueInt<Real> part = SignDivide( A, Q, true );
    ValueInt<Real> part = RandomizedSignDivide( A, Q, true );

    // TODO: Retry or throw exception if part.value not small enough

    return part;
}

template<typename F>
inline void
SDC( Matrix<F>& A, Int cutoff=256 )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SDC");
#endif
    typedef BASE(F) Real;
    const Int n = A.Height();
    if( n <= cutoff )
    {
        Matrix<Complex<Real> > w;
        schur::QR( A, w );
        return;
    }

    // Perform this level's split
    const ValueInt<Real> part = SpectralDivide( A );
    Matrix<F> ATL, ATR,
              ABL, ABR;
    PartitionDownDiagonal
    ( A, ATL, ATR,
         ABL, ABR, part.index );

    // Recurse on the two subproblems
    SDC( ATL, cutoff );
    SDC( ABR, cutoff );
}

template<typename F>
inline void
SDC( Matrix<F>& A, Matrix<F>& Q, bool formATR=true, Int cutoff=256 )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SDC");
#endif
    typedef BASE(F) Real;
    const Int n = A.Height();
    if( n <= cutoff )
    {
        Matrix<Complex<Real> > w;
        schur::QR( A, Q, w, formATR );
        return;
    }

    // Perform this level's split
    const ValueInt<Real> part = SpectralDivide( A, Q );
    Matrix<F> ATL, ATR,
              ABL, ABR;
    PartitionDownDiagonal
    ( A, ATL, ATR,
         ABL, ABR, part.index );
    Matrix<F> QL, QR;
    PartitionRight( Q, QL, QR, part.index );

    // Recurse on the top-left quadrant and update Schur vectors and ATR
    Matrix<F> Z, G;
    SDC( ATL, Z, formATR, cutoff );
    G = QL;
    Gemm( NORMAL,  NORMAL, F(1), G, Z, QL );
    if( formATR )
        Gemm( ADJOINT, NORMAL, F(1), Z, ATR, G );

    // Recurse on the bottom-right quadrant and update Schur vectors and ATR
    SDC( ABR, Z, formATR, cutoff );
    if( formATR )
        Gemm( NORMAL, NORMAL, F(1), G, Z, ATR ); 
    G = QR;
    Gemm( NORMAL, NORMAL, F(1), G, Z, QR );
}

template<typename F>
inline void
SDC( DistMatrix<F>& A, Int cutoff=256 )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SDC");
#endif
    typedef BASE(F) Real;
    const Grid& g = A.Grid();
    const Int n = A.Height();
    if( n <= cutoff )
    {
        DistMatrix<F,CIRC,CIRC> A_CIRC_CIRC( A );
        Matrix<Complex<Real> > w;
        if( g.VCRank() == A_CIRC_CIRC.Root() )
            schur::QR( A_CIRC_CIRC.Matrix(), w );
        A = A_CIRC_CIRC;
        return;
    }

    // Perform this level's split
    const ValueInt<Real> part = SpectralDivide( A );
    DistMatrix<F> ATL(g), ATR(g),
                  ABL(g), ABR(g);
    PartitionDownDiagonal
    ( A, ATL, ATR,
         ABL, ABR, part.index );

    // Recurse on the two subproblems
    SDC( ATL, cutoff );
    SDC( ABR, cutoff );
}

template<typename F>
inline void
SDC( DistMatrix<F>& A, DistMatrix<F>& Q, bool formATR=true, Int cutoff=256 )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SDC");
#endif
    typedef BASE(F) Real;
    const Grid& g = A.Grid();
    const Int n = A.Height();
    if( n <= cutoff )
    {
        DistMatrix<F,CIRC,CIRC> A_CIRC_CIRC( A ), Q_CIRC_CIRC( n, n, g );
        Matrix<Complex<Real> > w;
        if( g.VCRank() == A_CIRC_CIRC.Root() )
            schur::QR( A_CIRC_CIRC.Matrix(), Q_CIRC_CIRC.Matrix(), w, formATR );
        A = A_CIRC_CIRC;
        Q = Q_CIRC_CIRC;
        return;
    }

    // Perform this level's split
    const ValueInt<Real> part = SpectralDivide( A, Q );
    DistMatrix<F> ATL(g), ATR(g),
                  ABL(g), ABR(g);
    PartitionDownDiagonal
    ( A, ATL, ATR,
         ABL, ABR, part.index );
    DistMatrix<F> QL(g), QR(g);
    PartitionRight( Q, QL, QR, part.index );

    // Recurse on the two subproblems
    DistMatrix<F> ZT(g), ZB(g);
    SDC( ATL, ZT, formATR, cutoff );
    SDC( ABR, ZB, formATR, cutoff );

    // Update the Schur vectors
    DistMatrix<F> G(g);
    G = QL;
    Gemm( NORMAL, NORMAL, F(1), G, ZT, QL );
    G = QR;
    Gemm( NORMAL, NORMAL, F(1), G, ZB, QR );

    if( formATR )
    {
        // Update the top-right quadrant
        Gemm( ADJOINT, NORMAL, F(1), ZT, ATR, G );
        Gemm( NORMAL, NORMAL, F(1), G, ZB, ATR ); 
    }
}

} // namespace schur
} // namespace elem

#endif // ifndef ELEM_LAPACK_SCHUR_SDC_HPP
