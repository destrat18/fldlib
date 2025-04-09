/**************************************************************************/
/*                                                                        */
/*  Copyright (C) 2014-2025                                               */
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
// Library   : ForwardNumerics
// Unit      : Integer
// File      : BaseIntegerCommon.h
// Description :
//   Definition of a common part between BaseInteger.h and BaseIntegerGeneric.h
//

#pragma once

#include <cstdint>

#ifndef DefineNoEnhancedObject
#include "StandardClasses/Persistence.h"

#else

#include <iostream>
#include <cstring>
#include <cassert>
#define AssumeUncalled assert(false);
#define AssumeCondition(cond) assert(cond);

namespace STG { namespace IOObject { typedef std::ostream OSBase; typedef std::istream ISBase; }}

enum ComparisonResult { CRLess=-1, CREqual=0, CRGreater=1, CRNonComparable=2 };

#endif

namespace Numerics {

namespace DInteger {

class Access {
  public:
   static int log_base_2(uint32_t value)
      {  int result = 1;
         while ((value >>= 1) != 0)
            ++result;
         return result;
      }
};

} // end of namespace DInteger

class UnsignedBaseStoreTraits {
  public:
   static const int USizeBaseInBits = sizeof(uint32_t)*8;
   typedef uint32_t BaseType;
   typedef uint32_t* BaseTypePointer;
   typedef uint32_t& BaseTypeReference;
   typedef uint32_t BaseTypeConstReference;
   static int log_base_2(uint32_t value) { return DInteger::Access::log_base_2(value); }
   static void clearArray(uint32_t* array, int count)
      {  memset(array, 0, count*sizeof(uint32_t)); }
   static void copyArray(uint32_t* target, const uint32_t* source, int count)
      {  memcpy(target, source, count*sizeof(uint32_t)); }
   template <typename TypeFst, typename TypeSnd>
   static void copyArray(TypeFst* target, const TypeSnd* source, int count)
      {  memcpy(target, source, count*sizeof(uint32_t)); }
   static int sizeBaseInBits() { return 8*sizeof(uint32_t); }
   static int minCellsCountToStoreBits(int bitsNumber)
      {  return (int) ((bitsNumber + 8*sizeof(uint32_t) - 1) / (8*sizeof(uint32_t))); } 

   static void swapArray(uint32_t* target, uint32_t* source, int count)
      {  uint32_t temp;
         for (int i = 0; i < count; ++i) {
            temp = target[i];
            target[i] = source[i];
            source[i] = temp;
         };
      }
   static uint32_t detectCarryAfterAddition(uint32_t result, uint32_t operand)
      {  return (result < operand) ? 1U : 0U; }
   static uint32_t detectCarryBeforeSubstraction(uint32_t first, uint32_t second)
      {  return (first < second) ? 1U : 0U; }
   static uint32_t getStoreMidHighPart(uint32_t value, uint32_t store)
      {  return ((value << (4*sizeof(uint32_t))) | (store & ~(~(uint32_t) 0 << 4*sizeof(uint32_t)))); }
   static void storeIntoMidHighPart(uint32_t value, uint32_t& store)
      {  store &= ~(~(uint32_t) 0 << 4*sizeof(uint32_t));
         store |= (value << (4*sizeof(uint32_t)));
      }
   template <typename TypeProperty>
   static void storeIntoMidHighPart(uint32_t value, TypeProperty store)
      {  store &= ~(~(uint32_t) 0 << 4*sizeof(uint32_t));
         store |= (value << (4*sizeof(uint32_t)));
      }
   static uint32_t getStoreMidLowPart(uint32_t value, uint32_t store)
      {  return ((store & (~(uint32_t) 0 << 4*sizeof(uint32_t))) | value); }
   static void storeIntoMidLowPart(uint32_t value, uint32_t& store)
      {  store &= (~(uint32_t) 0 << 4*sizeof(uint32_t));
         store |= value;
      }
   template <typename TypeProperty>
   static void storeIntoMidLowPart(uint32_t value, TypeProperty store)
      {  store &= (~(uint32_t) 0 << 4*sizeof(uint32_t));
         store |= value;
      }
   static uint32_t getMidHighPart(uint32_t value)
      {  return value >> 4*sizeof(uint32_t); }
   static uint32_t getMidLowPart(uint32_t value)
      {  return value & ~(~(uint32_t) 0 << 4*sizeof(uint32_t)); }
   static uint32_t getLowPart(uint32_t value, int shift)
      {  return value & ~(~(uint32_t) 0 << shift); }
   static uint32_t getHighPart(uint32_t value, int shift)
      {  return (value >> shift); }
   static uint32_t getMiddlePart(uint32_t value, int lowBit, int sizeInBits)
      {  return (value >> lowBit) & ~(~(uint32_t) 0 << sizeInBits); }
   static uint32_t getStoreHighPart(uint32_t value, int shift)
      {  return (value << shift); }
   static uint32_t getStoreHighPart(uint32_t value, int shift, uint32_t store)
      {  return (value << shift) | (store & ~(~(uint32_t) 0 << shift)); }
   static void storeIntoHighPart(uint32_t value, int shift, uint32_t& store)
      {  store &= ~(~(uint32_t) 0 << shift);
         store |= (value << shift);
      }
   template <typename TypeProperty>
   static void storeIntoHighPart(uint32_t value, int shift, TypeProperty store)
      {  store &= ~(~(uint32_t) 0 << shift);
         store |= (value << shift);
      }
   static uint32_t getStoreLowPart(uint32_t value, int shift)
      {  return value & ~(~(uint32_t) 0 << shift); }
   static uint32_t getStoreLowPart(uint32_t value, int shift, uint32_t store)
      {  return (store & (~(uint32_t) 0 << shift)) | (value & ~(~(uint32_t) 0 << shift)); }
   static void storeIntoLowPart(uint32_t value, int shift, uint32_t& store)
      {  store &= (~(uint32_t) 0 << shift);
         store |= (value & ~(~(uint32_t) 0 << shift));
      }
   template <typename TypeProperty>
   static void storeIntoLowPart(uint32_t value, int shift, TypeProperty store)
      {  store &= (~(uint32_t) 0 << shift);
         store |= (value & ~(~(uint32_t) 0 << shift));
      }
   static void setTrueBit(uint32_t& result, int index) { result |= ((uint32_t) 1 << index); }
   template <typename TypeProperty>
   static void setTrueBit(TypeProperty result, int index) { result |= ((uint32_t) 1 << index); }
   static void setFalseBit(uint32_t& result, int index) { result &= ~((uint32_t) 1 << index); }
   template <typename TypeProperty>
   static void setFalseBit(TypeProperty result, int index) { result &= ~((uint32_t) 1 << index); }
   static void setBit(uint32_t& result, int index, bool value)
      {  if (value)
            result |= ((uint32_t) 1 << index);
         else
            result &= ~((uint32_t) 1 << index);
      }
   template <typename TypeProperty>
   static void setBit(TypeProperty result, int index, bool value)
      {  if (value)
            result |= ((uint32_t) 1 << index);
         else
            result &= ~((uint32_t) 1 << index);
      }
   static bool getBit(uint32_t value, int index)
      {  return (value & ((uint32_t) 1 << index)) ? true : false; }
   static void leftShiftLocal(uint32_t& value, int index, int shift)
      {  uint32_t temp = value;
         temp &= (~(uint32_t) 0 << index);
         temp <<= shift;
         value = temp | (value & ~(~(uint32_t) 0 << index));
      }
   template <typename TypeProperty>
   static void leftShiftLocal(TypeProperty value, int index, int shift)
      {  uint32_t temp = value;
         temp &= (~(uint32_t) 0 << index);
         temp <<= shift;
         value = temp | (value & ~(~(uint32_t) 0 << index));
      }
   static void rightShiftLocal(uint32_t& value, int index, int shift)
      {  uint32_t temp = value;
         temp &= (~(uint32_t) 0 << index);
         temp >>= shift;
         value = temp | (value & ~(~(uint32_t) 0 << (index-shift)));
      }
   template <typename TypeProperty>
   static void rightShiftLocal(TypeProperty value, int index, int shift)
      {  uint32_t temp = value;
         temp &= (~(uint32_t) 0 << index);
         temp >>= shift;
         value = temp | (value & ~(~(uint32_t) 0 << (index-shift)));
      }
   static void leftShiftAndClearLow(uint32_t& value, int shift, int low)
      {  value <<= shift;
         value &= ~(uint32_t) 0 << low;
      }
   template <typename TypeProperty>
   static void leftShiftAndClearLow(TypeProperty value, int shift, int low)
      {  value <<= shift;
         value &= ~(uint32_t) 0 << low;
      }
   static void rightShiftAndClearHigh(uint32_t& value, int shift, int high)
      {  value >>= shift;
         value &= ~(~(uint32_t) 0 << high);
      }
   template <typename TypeProperty>
   static void rightShiftAndClearHigh(TypeProperty value, int shift, int high)
      {  value >>= shift;
         value &= ~(~(uint32_t) 0 << high);
      }
   static void rightShift(uint32_t& value, int shift) { value >>= shift; }
   template <typename TypeProperty>
   static void rightShift(TypeProperty value, int shift) { value >>= shift; }
   static void leftShift(uint32_t& value, int shift) { value <<= shift; }
   template <typename TypeProperty>
   static void leftShift(TypeProperty value, int shift) { value <<= shift; }
   static void negLowValuePart(uint32_t& value, int shift)
      {  if (shift > 0)
            value = (value & (~(uint32_t) 0 << shift)) | (~value & ~(~(uint32_t) 0 << shift));
      }
   template <typename TypeProperty>
   static void negLowValuePart(TypeProperty value, int shift)
      {  if (shift > 0)
            value = (value & (~(uint32_t) 0 << shift)) | (~value & ~(~(uint32_t) 0 << shift));
      }
   static void clearLowValuePart(uint32_t& value, int shift)
      {  if (shift > 0)
            value &= (~(uint32_t) 0 << shift);
      }
   template <typename TypeProperty>
   static void clearLowValuePart(TypeProperty value, int shift)
      {  if (shift > 0)
            value &= (~(uint32_t) 0 << shift);
      }
   static void clearHighValuePart(uint32_t& value, int shift)
      {  if (shift < (int) (8*sizeof(uint32_t)))
            value &= ~(~(uint32_t) 0 << shift);
      }
   template <typename TypeProperty>
   static void clearHighValuePart(TypeProperty value, int shift)
      {  if (shift < (int) (8*sizeof(uint32_t)))
            value &= ~(~(uint32_t) 0 << shift);
      }
   static uint32_t getSaturation() { return ~(uint32_t) 0; }
   static void saturateLowValuePart(uint32_t& value, int shift)
      {  if (shift > 0)
            value |= ~(~(uint32_t) 0 << shift);
      }
   template <typename TypeProperty>
   static void saturateLowValuePart(TypeProperty value, int shift)
      {  if (shift > 0)
            value |= ~(~(uint32_t) 0 << shift);
      }
   static bool isZeroValue(uint32_t value) { return value == (uint32_t) 0U; }
   static bool hasZeroValue(uint32_t value, int shift)
      {  AssumeCondition(shift <= (int) (8*sizeof(uint32_t)))
         return (shift <= 0) || ((value << (8*sizeof(uint32_t)-shift)) == (uint32_t) 0U);
      }
   static bool isOneValue(uint32_t value) { return value == (uint32_t) 1U; }
   static void writeValue(STG::IOObject::OSBase& out, uint32_t value, bool isRaw)
#ifndef DefineNoEnhancedObject
      {  out.write(value, isRaw); }
#else
      {  if (!isRaw)
            out << value;
         else
            out.write(reinterpret_cast<char*>(&value), sizeof(value));
      }
#endif
   static void readValue(STG::IOObject::ISBase& in, uint32_t& value, bool isRaw)
      {
#ifndef DefineNoEnhancedObject
         in.read(value, isRaw);
#else
         if (isRaw) {
            in.read(reinterpret_cast<char*>(&value), sizeof(value));
         }
         else
            in >> value;
#endif
      }
};

} // end of namespace Numerics

