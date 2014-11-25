/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {
namespace lin_prog {

// Full system
// -----------
template<typename Real>
void FormSystem
( const Matrix<Real>& A, const Matrix<Real>& b, const Matrix<Real>& c,
  const Matrix<Real>& s, const Matrix<Real>& x, const Matrix<Real>& l,
  Real tau, Matrix<Real>& J, Matrix<Real>& y );
template<typename Real>
void FormSystem
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b, const AbstractDistMatrix<Real>& c,
  const AbstractDistMatrix<Real>& s, const AbstractDistMatrix<Real>& x,
  const AbstractDistMatrix<Real>& l,
  Real tau, AbstractDistMatrix<Real>& J, AbstractDistMatrix<Real>& y );

template<typename Real>
void SolveSystem
( Int m, Int n, Matrix<Real>& J, Matrix<Real>& y,
  Matrix<Real>& ds, Matrix<Real>& dx, Matrix<Real>& dl );
template<typename Real>
void SolveSystem
( Int m, Int n, AbstractDistMatrix<Real>& J, AbstractDistMatrix<Real>& y,
  AbstractDistMatrix<Real>& ds, AbstractDistMatrix<Real>& dx,
  AbstractDistMatrix<Real>& dl );

// Augmented system
// ----------------
template<typename Real>
void FormAugmentedSystem
( const Matrix<Real>& A, const Matrix<Real>& b, const Matrix<Real>& c,
  const Matrix<Real>& s, const Matrix<Real>& x, const Matrix<Real>& l,
  Real tau, Matrix<Real>& J, Matrix<Real>& y );
template<typename Real>
void FormAugmentedSystem
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b, const AbstractDistMatrix<Real>& c,
  const AbstractDistMatrix<Real>& s, const AbstractDistMatrix<Real>& x,
  const AbstractDistMatrix<Real>& l,
  Real tau, AbstractDistMatrix<Real>& J, AbstractDistMatrix<Real>& y );
template<typename Real>
void FormAugmentedSystem
( const SparseMatrix<Real>& A,
  const Matrix<Real>& b, const Matrix<Real>& c,
  const Matrix<Real>& s, const Matrix<Real>& x, const Matrix<Real>& l,
  Real tau, SparseMatrix<Real>& J, Matrix<Real>& y );
template<typename Real>
void FormAugmentedSystem
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b, const DistMultiVec<Real>& c,
  const DistMultiVec<Real>& s, const DistMultiVec<Real>& x,
  const DistMultiVec<Real>& l,
  Real tau, DistSparseMatrix<Real>& J, DistMultiVec<Real>& y );

template<typename Real>
void SolveAugmentedSystem
( const Matrix<Real>& s, const Matrix<Real>& x,
  Real tau, Matrix<Real>& J, Matrix<Real>& y,
  Matrix<Real>& ds, Matrix<Real>& dx, Matrix<Real>& dl );
template<typename Real>
void SolveAugmentedSystem
( const AbstractDistMatrix<Real>& s, const AbstractDistMatrix<Real>& x,
  Real tau, AbstractDistMatrix<Real>& J, AbstractDistMatrix<Real>& y,
  AbstractDistMatrix<Real>& ds, AbstractDistMatrix<Real>& dx,
  AbstractDistMatrix<Real>& dl );

// Normal system
// -------------
template<typename Real>
void NormalKKT
( const SparseMatrix<Real>& A,
  const Matrix<Real>& s, const Matrix<Real>& x,
  SparseMatrix<Real>& J );
template<typename Real>
void NormalKKT
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& s, const DistMultiVec<Real>& x,
  DistSparseMatrix<Real>& J );

template<typename Real>
void NormalKKTRHS
( const SparseMatrix<Real>& A,
  const Matrix<Real>& s, const Matrix<Real>& x,
  const Matrix<Real>& rmu, const Matrix<Real>& rc, const Matrix<Real>& rb,
        Matrix<Real>& y );
template<typename Real>
void NormalKKTRHS
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& s, const DistMultiVec<Real>& x,
  const DistMultiVec<Real>& rmu, const DistMultiVec<Real>& rc,
  const DistMultiVec<Real>& rb,
        DistMultiVec<Real>& y );

template<typename Real>
void ExpandNormalSolution
( const SparseMatrix<Real>& A, const Matrix<Real>& c,
  const Matrix<Real>& s,       const Matrix<Real>& x,
  const Matrix<Real>& rmu,     const Matrix<Real>& rc,
  const Matrix<Real>& dl, Matrix<Real>& ds, Matrix<Real>& dx );
template<typename Real>
void ExpandNormalSolution
( const DistSparseMatrix<Real>& A, const DistMultiVec<Real>& c,
  const DistMultiVec<Real>& s,     const DistMultiVec<Real>& x,
  const DistMultiVec<Real>& rmu,   const DistMultiVec<Real>& rc,
  const DistMultiVec<Real>& dl,
  DistMultiVec<Real>& ds, DistMultiVec<Real>& dx );

// Line search
// -----------
template<typename Real>
Real IPFLineSearch
( const Matrix<Real>& A,
  const Matrix<Real>& b,  const Matrix<Real>& c,
  const Matrix<Real>& s,  const Matrix<Real>& x,  const Matrix<Real>& l,
  const Matrix<Real>& ds, const Matrix<Real>& dx, const Matrix<Real>& dl,
  Real gamma, Real beta, Real psi=100, bool print=false );
template<typename Real>
Real IPFLineSearch
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b,  const AbstractDistMatrix<Real>& c,
  const AbstractDistMatrix<Real>& s,  const AbstractDistMatrix<Real>& x,
  const AbstractDistMatrix<Real>& l,
  const AbstractDistMatrix<Real>& ds, const AbstractDistMatrix<Real>& dx,
  const AbstractDistMatrix<Real>& dl,
  Real gamma, Real beta, Real psi=100, bool print=false );
template<typename Real>
Real IPFLineSearch
( const SparseMatrix<Real>& A,
  const Matrix<Real>& b,  const Matrix<Real>& c,
  const Matrix<Real>& s,  const Matrix<Real>& x,  const Matrix<Real>& l,
  const Matrix<Real>& ds, const Matrix<Real>& dx, const Matrix<Real>& dl,
  Real gamma, Real beta, Real psi=100, bool print=false );
template<typename Real>
Real IPFLineSearch
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b,
  const DistMultiVec<Real>& c,
  const DistMultiVec<Real>& s,
  const DistMultiVec<Real>& x,
  const DistMultiVec<Real>& l,
  const DistMultiVec<Real>& ds,
  const DistMultiVec<Real>& dx,
  const DistMultiVec<Real>& dl,
  Real gamma, Real beta, Real psi=100, bool print=false );

} // namespace lin_prog
} // namespace El