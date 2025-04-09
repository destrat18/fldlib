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
// Unit      : FloatInterval
// File      : FloatIntervalBase.h
// Description :
//   Definition of a class of floating point intervals
//

#pragma once

#include "NumericalLattices/FloatIntervalBaseTypes.h"
#include <fstream>
#include <sstream>

namespace NumericalDomains { namespace DDoubleInterval {

/*********************************************************************/
/* Contract for the first template argument of TCompareFloatInterval */
/*********************************************************************/

template <class TDiagnosisTraits>
class TBaseFloatIntervalContract {
  public:
   static void initializeGlobals(const char* fileSuffix) {}
   static void finalizeGlobals() {}
   class Initialization {
     public:
      Initialization(const char* fileSuffix) { initializeGlobals(fileSuffix); }
      ~Initialization() { finalizeGlobals(); }
   };

   template <class TCompareFloatInterval>
   void notifyForCompare(const TCompareFloatInterval& source) const {}
   template <class TCompareFloatInterval>
   void notifyForBranchCompare(const TCompareFloatInterval& source) const {}
   template <class TCompareFloatInterval>
   void notifyForDivisionByZero(const TCompareFloatInterval& source) const {}
   template <class TCompareFloatInterval>
   void notifyForNegativeSqrt(const TCompareFloatInterval& source) const {}
   template <class TCompareFloatInterval>
   void notifyForNegativePow(const TCompareFloatInterval& source) const {}
   template <class TCompareFloatInterval>
   void notifyForNegativeOrNulLog(const TCompareFloatInterval& source) const {}

  protected:
   bool getThenBranch(bool executionResult) const { return executionResult; }
   uint32_t getConversionBranch(uint64_t diff, uint64_t executionResult) const { return 0; }
   bool doesFollow() const { return false; }

  public:
   Numerics::DDouble::Access::ReadParameters& minParams() const
      {  return *(Numerics::DDouble::Access::ReadParameters*) nullptr; }
   Numerics::DDouble::Access::ReadParameters& maxParams() const
      {  return *(Numerics::DDouble::Access::ReadParameters*) nullptr; }
   void persist() {}
};

template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
class TCompareFloatInterval : public TypeBaseFloatInterval {
  private:
   typedef TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation> thisType;
   typedef TypeBaseFloatInterval inherited;

#ifdef FLOAT_CONCRETE
   TypeImplementation dValue = 0;
#endif
   TypeBuiltDouble bfMin;
   TypeBuiltDouble bfMax;

   void adjustMinMax(TypeImplementation value);
   void adjustMin(TypeImplementation value);
   void adjustMax(TypeImplementation value);

   template <int MaxBitsNumberArgument, class TypeBaseFloatIntervalArgument, class TypeBuiltDoubleArgument, typename TypeImplementationArgument>
   friend class TCompareFloatInterval;

  public:
   void initFrom(STG::IOObject::ISBase& in);
   void initFrom(TypeImplementation value);
   void initFromAtomic(TypeImplementation value);

  protected:
   TypeBuiltDouble& smin() { return bfMin; }
   TypeBuiltDouble& smax() { return bfMax; }

  public:
   typedef TypeBuiltDouble BuiltDouble;
   void retrieveRelativeError(TypeBuiltDouble& result) const;

   const TypeBuiltDouble& min() const { return bfMin; }
   const TypeBuiltDouble& max() const { return bfMax; }

  public:
   TCompareFloatInterval() = default;
   TCompareFloatInterval(const thisType& source) = default;
   TCompareFloatInterval(TypeImplementation min, TypeImplementation max);
   template <typename TypeValue> TCompareFloatInterval(TypeValue value) requires std::integral<TypeValue>;
   TCompareFloatInterval(thisType&& source) = default; // [TODO] keep symbolic for constraints
   TCompareFloatInterval& operator=(const thisType& source) = default;
   TCompareFloatInterval& operator=(thisType&& source) = default; // [TODO] keep symbolic for constraints

   void mergeWith(const thisType& source)
      {  
#ifdef FLOAT_CONCRETE
         dValue = source.dValue;
#endif
         if (bfMin > source.bfMin)
            bfMin = source.bfMin;
         if (bfMax < source.bfMax)
            bfMax = source.bfMax;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   TCompareFloatInterval(const TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltArgument, TypeImplementationArgument>& source);
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   TCompareFloatInterval& operator=(const TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltArgument, TypeImplementationArgument>& source)
      {  return operator=(thisType(source)); }

   void setToMin() { bfMax = bfMin; }
   void setToMax() { bfMin = bfMax; }
   bool operator<(const thisType& source) const;
   bool operator<=(const thisType& source) const;
   bool operator==(const thisType& source) const;
   bool operator!=(const thisType& source) const;
   bool operator>=(const thisType& source) const;
   bool operator>(const thisType& source) const;

   thisType& operator+=(const thisType& source);
   thisType& operator-=(const thisType& source);
   thisType& operator*=(const thisType& source);
   thisType& operator/=(const thisType& source);

   void oppositeAssign()
      {  
#ifdef FLOAT_CONCRETE
         dValue = -dValue;
#endif
         bfMin.swap(bfMax);
         bfMin.opposite();
         bfMax.opposite();
         inherited::notifyForCompare(*this);
      }

   typedef TypeImplementation ImplementationType;
#ifdef FLOAT_CONCRETE
   TypeImplementation asImplementation() const { return dValue; }
#endif
   void retrieveImplementationBounds(TypeImplementation& min, TypeImplementation& max) const
      {  DDoubleInterval::setContent(min, bfMin, false /* isUpper */, typename TypeBaseFloatInterval::FloatDigitsHelper());
         DDoubleInterval::setContent(max, bfMax, true /* isUpper */, typename TypeBaseFloatInterval::FloatDigitsHelper());
      }
   typedef Numerics::DDouble::Access::ReadParameters ReadParametersBase;
   // roundMode=RMZero for C conversions, roundMode=RMNearest for Ada conversions, roundMode=RMLowest for floor
   int64_t asLongInt(ReadParametersBase::RoundMode roundMode=ReadParametersBase::RMZero) const;
   uint64_t asUnsignedLong(ReadParametersBase::RoundMode roundMode=ReadParametersBase::RMLowest) const;
   int asInt(ReadParametersBase::RoundMode roundMode=ReadParametersBase::RMZero) const
      {  return asLongInt(roundMode); }
   unsigned asUnsigned(ReadParametersBase::RoundMode roundMode=ReadParametersBase::RMLowest) const
      {  return asUnsignedLong(roundMode); }

   void sqrtAssign();
   void sinAssign();
   void cosAssign();
   void asinAssign();
   void acosAssign();
   void tanAssign();
   void atanAssign();
   void expAssign();
   void logAssign();
   void log10Assign();
   void powAssign(const thisType& value);
   void absAssign();
};

template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
inline void
TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>::adjustMinMax(TypeImplementation value) {
   if (value < 0)
      value = -value;
   if (value > 0 && !bfMin.isInfty()) {
      bool isMinApproximate, isMaxApproximate;
      uint32_t shiftCount = bfMin.BitSizeMantissa/2;
      if (shiftCount < 8*sizeof(uint32_t)) {
         isMinApproximate = (bfMin.getMantissa()[0] & ~(~0U << shiftCount)) != 0;
         isMaxApproximate = (bfMax.getMantissa()[0] & ~(~0U << shiftCount)) != 0;
      }
      else {
         isMinApproximate = bfMin.getMantissa()[0] != 0;
         isMaxApproximate = bfMax.getMantissa()[0] != 0;
      }
      if (isMinApproximate) {
         if (bfMin.getSMantissa().dec().hasCarry()) {
            if (bfMin.getSBasicExponent().dec().hasCarry())
               bfMin.setZero();
         };
      };
      if (isMaxApproximate) {
         if (bfMax.getSMantissa().inc().hasCarry()) {
            bfMax.getSBasicExponent().inc();
            if (bfMax.isInfty() && inherited::maxParams().isInftyAvoided()) {
               bfMax.getSMantissa().dec();
               bfMax.getSBasicExponent().dec();
            };
         };
      };
   };
}

template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
inline void
TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>::initFrom(STG::IOObject::ISBase& in) {
   auto params = inherited::minParams();
   params.setNearestRound();
   params.setRoundToEven();
   bfMin.readDecimal(in, params);
   params.clear();
   bfMax = bfMin;
#ifndef FLOAT_CONCRETE
   TypeImplementation dValue = 0;
#endif
   DDoubleInterval::setContent(dValue, bfMin, false /* isUpper */, typename TypeBaseFloatInterval::FloatDigitsHelper());
   adjustMinMax(dValue);
   inherited::notifyForCompare(*this);
}

template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
inline void
TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>::initFrom(TypeImplementation value) {
#ifdef FLOAT_CONCRETE
   dValue = value;
#endif
   DDoubleInterval::fillContent(bfMin, value, typename TypeBaseFloatInterval::FloatDigitsHelper());
   DDoubleInterval::fillContent(bfMax, value, typename TypeBaseFloatInterval::FloatDigitsHelper());
   adjustMinMax(value);
   inherited::notifyForCompare(*this);
}

template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
inline void
TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>::initFromAtomic(TypeImplementation value) {
#ifdef FLOAT_CONCRETE
   dValue = value;
#endif
   DDoubleInterval::fillContent(bfMin, value, typename TypeBaseFloatInterval::FloatDigitsHelper());
   DDoubleInterval::fillContent(bfMax, value, typename TypeBaseFloatInterval::FloatDigitsHelper());
   inherited::notifyForCompare(*this);
}

template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
template <class TypeBuiltArgument, typename TypeImplementationArgument>
inline
TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>::TCompareFloatInterval(
      const TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltArgument, TypeImplementationArgument>& source)
#ifdef FLOAT_CONCRETE
   : dValue((TypeImplementation) source.dValue)
#endif
{
   auto& minParams = inherited::minParams();
   auto& maxParams = inherited::maxParams();
   typename inherited::FloatConversion conversion;
   conversion.setSizeMantissa(source.bfMin.BitSizeMantissa).setSizeExponent(source.bfMin.BitSizeExponent);
   int sizeMantissa = (source.bfMin.BitSizeMantissa + 8*sizeof(uint32_t) - 1)/(8*sizeof(uint32_t));
   for (int index = 0; index < sizeMantissa; ++index)
      conversion.mantissa()[index] = source.bfMin.getMantissa()[index];
   int sizeExponent = (source.bfMin.BitSizeExponent + 8*sizeof(uint32_t) - 1)/(8*sizeof(uint32_t));
   for (int index = 0; index < sizeExponent; ++index)
      conversion.exponent()[index] = source.bfMin.getBasicExponent()[index];
   conversion.setNegative(source.bfMin.isNegative());
   bfMin.setFloat(conversion, minParams);
   minParams.clear();
   for (int index = 0; index < sizeMantissa; ++index)
      conversion.mantissa()[index] = source.bfMax.getMantissa()[index];
   for (int index = 0; index < sizeExponent; ++index)
      conversion.exponent()[index] = source.bfMax.getBasicExponent()[index];
   conversion.setNegative(source.bfMax.isNegative());
   bfMax.setFloat(conversion, maxParams);
   maxParams.clear();
   inherited::notifyForCompare(*this);
}

template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
inline void
TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>::adjustMin(TypeImplementation min) {
   if (min < 0)
      min = -min;
   if (min > 0 && !bfMin.isInfty()) {
      bool isApproximate;
      uint32_t shiftCount = bfMin.BitSizeMantissa/2;
      if (shiftCount < 8*sizeof(uint32_t)) {
         isApproximate = (bfMin.getMantissa()[0] & ~(~0U << shiftCount)) != 0;
      }
      else
         isApproximate = bfMin.getMantissa()[0] != 0;
      if (isApproximate) {
         if (bfMin.getSMantissa().dec().hasCarry()) {
            if (bfMin.getSBasicExponent().dec().hasCarry())
               bfMin.setZero();
         };
      };
   };
}

template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
inline void
TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>::adjustMax(TypeImplementation max) {
   if (max < 0)
      max = -max;
   if (max > 0 && !bfMax.isInfty()) {
      bool isApproximate;
      uint32_t shiftCount = bfMin.BitSizeMantissa/2;
      if (shiftCount < 8*sizeof(uint32_t)) {
         isApproximate = (bfMax.getMantissa()[0] & ~(~0U << shiftCount)) != 0;
      }
      else
         isApproximate = bfMax.getMantissa()[0] != 0;
      if (isApproximate) {
         if (bfMax.getSMantissa().inc().hasCarry()) {
            bfMax.getSBasicExponent().inc();
            if (bfMax.isInfty() && inherited::maxParams().isInftyAvoided()) {
               bfMax.getSMantissa().dec();
               bfMax.getSBasicExponent().dec();
            };
         };
      };
   };
}

template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
inline
TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>::TCompareFloatInterval(
      TypeImplementation min, TypeImplementation max)
#ifdef FLOAT_CONCRETE
   :  dValue((min + max)/2)
#endif
{
   DDoubleInterval::fillContent(bfMin, min, typename TypeBaseFloatInterval::FloatDigitsHelper());
   DDoubleInterval::fillContent(bfMax, max, typename TypeBaseFloatInterval::FloatDigitsHelper());
   adjustMin(min);
   adjustMax(max);
   inherited::notifyForCompare(*this);
}

template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
template <typename TypeValue>
inline
TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>::TCompareFloatInterval(TypeValue value)
   requires std::integral<TypeValue>
#ifdef FLOAT_CONCRETE
   :  dValue((TypeImplementation) value)
#endif
{ // [TODO] to improve
   auto& minParams = inherited::minParams();
   auto& maxParams = inherited::maxParams();
   typename TypeBuiltDouble::IntConversion conversion;
   if (std::is_unsigned<TypeValue>::value) {
      conversion.setUnsigned();
      conversion.assign((uint64_t) value);
   }
   else {
      conversion.setSigned();
      conversion.assign((int64_t) value);
   }
   bfMin.setInteger(conversion, minParams);
   minParams.clear();
   bfMax.setInteger(conversion, maxParams);
   maxParams.clear();
   inherited::notifyForCompare(*this);
}

template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
inline TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>&
TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>::operator+=(const thisType& source) {
#ifdef FLOAT_CONCRETE
   dValue += source.dValue;
#endif
   auto& minParams = inherited::minParams();
   auto& maxParams = inherited::maxParams();
   bfMin.plusAssign(source.bfMin, minParams);
   minParams.clear();
   bfMax.plusAssign(source.bfMax, maxParams);
   maxParams.clear();
   inherited::notifyForCompare(*this);
   return *this;
}
   
template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
inline TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>&
TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>::operator-=(const thisType& source) {
#ifdef FLOAT_CONCRETE
   dValue -= source.dValue;
#endif
   auto& minParams = inherited::minParams();
   auto& maxParams = inherited::maxParams();
   bfMin.minusAssign(source.bfMax, minParams);
   minParams.clear();
   bfMax.minusAssign(source.bfMin, maxParams);
   maxParams.clear();
   inherited::notifyForCompare(*this);
   return *this;
}
         
template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
inline void
TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>::sqrtAssign() {
   bool hasNegativeSqrt = bfMin.isNegative() && !bfMin.isZero();
   if (hasNegativeSqrt) {
      bfMin = 0;
      if (bfMax.isNegative() && !bfMax.isZero())
         bfMax = 0;
   };

   TypeImplementation min, max;
   DDoubleInterval::setContent(min, bfMin, false /* isUpper */, typename TypeBaseFloatInterval::FloatDigitsHelper());
   DDoubleInterval::setContent(max, bfMax, true /* isUpper */, typename TypeBaseFloatInterval::FloatDigitsHelper());
   min = (TypeImplementation) ::sqrt((double) min);
   max = (TypeImplementation) ::sqrt((double) max);

   DDoubleInterval::fillContent(bfMin, min, typename TypeBaseFloatInterval::FloatDigitsHelper());
   DDoubleInterval::fillContent(bfMax, max, typename TypeBaseFloatInterval::FloatDigitsHelper());
   adjustMin(min);
   adjustMax(max);
   if (hasNegativeSqrt)
      inherited::notifyForNegativeSqrt(*this);

#ifdef FLOAT_CONCRETE
   dValue = (TypeImplementation) ::sqrt((double) dValue);
#endif
   inherited::notifyForCompare(*this);
}

template <int UMaxBitsNumber, class TypeBaseFloatInterval, class TypeBuiltDouble, typename TypeImplementation>
inline void
TCompareFloatInterval<UMaxBitsNumber, TypeBaseFloatInterval, TypeBuiltDouble, TypeImplementation>::absAssign() {
   if (bfMin.isNegative()) {
      if (bfMax.isPositive()) {
         bfMin.opposite();
         if (bfMin > bfMax)
            bfMin.swap(bfMax);
         bfMin.setZero();
      }
      else {
         bfMin.opposite();
         bfMax.opposite();
         bfMin.swap(bfMax);
      };
   };

#ifdef FLOAT_CONCRETE
   if (dValue < 0)
      dValue = -dValue;
   // dValue = ::fabs((double) dValue);
#endif
   inherited::notifyForCompare(*this);
}

} // end of namespace DDoubleInterval

} // end of namespace NumericalDomains

