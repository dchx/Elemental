/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_LATTICE_HPP
#define EL_LATTICE_HPP

namespace El {

template<typename Real>
void LLL( Matrix<Real>& B, Real delta, Real eta, Real theta, Real innerTol );

} // namespace El

#endif // ifndef EL_LATTICE_HPP