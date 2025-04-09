/**************************************************************************/
/*                                                                        */
/*  Copyright (C) 2011-2025                                               */
/*    CEA (Commissariat a l'Energie Atomique et aux Energies              */
/*         Alternatives)                                                  */
/*                                                                        */
/*  you can redistribute it and/or modify it under the terms of the GNU   */
/*  Lesser General Public License as published by the Free Software       */
/*  Foundation, version 2.1.                                              */
/*                                                                        */
/*  It is distributed in the hope that it will be useful,                 */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/*  GNU Lesser General Public License for more details.                   */
/*                                                                        */
/*  See the GNU Lesser General Public License version 2.1                 */
/*  for more details (enclosed in the file LICENSE).                      */
/*                                                                        */
/**************************************************************************/

/////////////////////////////////
//
// Library   : NumericalDomains
// Unit      : Affine relationships
// File      : FloatAffineOption.cpp
// Description :
//   Implementation of an optional class of affine relations with conditional
//     relationships between discrete int and continuous float domains.
//

#include "FloatAffineOption.template"
#include "Pointer/Vector.template"

namespace NumericalDomains {

bool FldlibBase::active = false;
int FldlibBase::numberOfBranches = 0;
int FldlibBase::splitBranchIdentifier = 0;

template class TFldlibZonotopeOption<float>;
template class TFldlibZonotopeOption<double>;
template class TFldlibZonotopeOption<long double>;

template void DAffine::TMergeBranches<DAffine::ExecutionPath>::pushForMerge<TFldlibFloatingBranchOption<TFldlibZonotopeOption<double> >>(TFldlibFloatingBranchOption<TFldlibZonotopeOption<double> >&);
template void DAffine::TMergeBranches<DAffine::ExecutionPath>::pushForMerge<TFldlibFloatingBranchOption<TFldlibZonotopeOption<float> >>(TFldlibFloatingBranchOption<TFldlibZonotopeOption<float> >&);
template void DAffine::TMergeBranches<DAffine::ExecutionPath>::pushForMerge<TFldlibFloatingBranchOption<TFldlibZonotopeOption<long double> >>(TFldlibFloatingBranchOption<TFldlibZonotopeOption<long double> >&);

template void DAffine::TMergeBranches<DAffine::ExecutionPath>::pushForMerge<TFldlibZonotopeOption<double> >(NumericalDomains::TFldlibZonotopeOption<double>&);
template void DAffine::TMergeBranches<DAffine::ExecutionPath>::pushForMerge<TFldlibZonotopeOption<float> >(NumericalDomains::TFldlibZonotopeOption<float>&);
template void DAffine::TMergeBranches<DAffine::ExecutionPath>::pushForMerge<TFldlibZonotopeOption<long double> >(NumericalDomains::TFldlibZonotopeOption<long double>&);
};

template class COL::TVector<NumericalDomains::TFldlibZonotopeOption<float>, COL::DVector::TElementTraits<NumericalDomains::TFldlibZonotopeOption<float> >, COL::DVector::ReallocTraits>;
template class COL::TVector<NumericalDomains::TFldlibZonotopeOption<double>, COL::DVector::TElementTraits<NumericalDomains::TFldlibZonotopeOption<double> >, COL::DVector::ReallocTraits>;
template class COL::TVector<NumericalDomains::TFldlibZonotopeOption<long double>, COL::DVector::TElementTraits<NumericalDomains::TFldlibZonotopeOption<long double> >, COL::DVector::ReallocTraits>;
