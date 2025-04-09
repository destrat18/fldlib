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
// File      : BaseInteger.h
// Description :
//   Definition of a class of integers with unbound size.
//

#pragma once

#include "ForwardNumerics/BaseIntegerCommon.h"

namespace Numerics {

template <class IntegerTraits>
class TBigCellInt;
template <class IntegerTraits>
class TBigCell;
template <class IntegerTraits>
class TBigInt;

namespace DInteger {

class CellIntegerTraitsContract {
  public:
   CellIntegerTraitsContract() {}
   CellIntegerTraitsContract(const CellIntegerTraitsContract& /* source */) { AssumeUncalled }
   CellIntegerTraitsContract& operator=(const CellIntegerTraitsContract& /* source */) { AssumeUncalled return *this; }
   typedef uint32_t& ArrayProperty;
   ArrayProperty array(int /* index */) { AssumeUncalled uint32_t* result = nullptr; return *result; }
   uint32_t array(int /* index */) const { AssumeUncalled return 0; }
   uint32_t carray(int /* index */) const { AssumeUncalled return 0; }
   ArrayProperty operator[](int /* index */) { AssumeUncalled uint32_t* result = nullptr; return *result; }
   uint32_t operator[](int /* index */) const { AssumeUncalled return 0; }
   typedef CellIntegerTraitsContract MultResult;

   typedef CellIntegerTraitsContract QuotientResult;
   typedef CellIntegerTraitsContract RemainderResult;
   typedef CellIntegerTraitsContract NormalizedRemainderResult;
   void copyLow(const MultResult& /* result */) { AssumeUncalled }
   int getSize() const { AssumeUncalled return 0; }
   void adjustSize(int /* newSize */) { AssumeUncalled }
   void setSize(int /* exactSize */) { AssumeUncalled }
   void setBitSize(int /* exactSize */) { AssumeUncalled }
   void setCellSize(int /* exactSize */) { AssumeUncalled }
   void normalize() { AssumeUncalled }
   void assertSize(int /* newSize */) { AssumeUncalled }
   void clear() { AssumeUncalled }
   typedef CellIntegerTraitsContract ExtendedInteger;
};

template <int UCellSize>
class TBasicCellIntegerTraits : public CellIntegerTraitsContract {
  private:
   uint32_t auArray[UCellSize];
   typedef TBasicCellIntegerTraits<UCellSize> thisType;

  protected:
   uint32_t* _array() { return auArray; }
   const uint32_t* _array() const { return auArray; }

  public:
   TBasicCellIntegerTraits() { memset(auArray, 0, UCellSize*sizeof(uint32_t)); }
   TBasicCellIntegerTraits(const thisType& source) : CellIntegerTraitsContract()
      {  memcpy(auArray, source.auArray, UCellSize*sizeof(uint32_t)); }
   thisType& operator=(const thisType& source)
      {  memcpy(auArray, source.auArray, UCellSize*sizeof(uint32_t));
         return *this;
      }
   thisType& operator=(uint32_t source)
      {  if (UCellSize > 1)
            memset(auArray, 0, UCellSize*sizeof(uint32_t));
         auArray[0] = source;
         return *this;
      }
   typedef uint32_t& ArrayProperty;
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
   ArrayProperty array(int index)
      {  AssumeCondition(index >= 0 && (index < UCellSize))
         return auArray[index];
      }
   uint32_t array(int index) const
      {  AssumeCondition(index >= 0)
         return (index < UCellSize) ? auArray[index] : (uint32_t) 0U;
      }
   uint32_t carray(int index) const { return array(index); }
   ArrayProperty operator[](int index)
      {  AssumeCondition(index >= 0 && (index < UCellSize))
         return auArray[index];
      }
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
   uint32_t operator[](int index) const
      {  AssumeCondition(index >= 0)
         return (index < UCellSize) ? auArray[index] : (uint32_t) 0U;
      }

   static int getSize() { return UCellSize; }
   void normalize() {}
   void adjustSize(int /* newSize */) { AssumeUncalled }
   void assertSize(int newSize) { AssumeCondition(newSize <= UCellSize) }
   void setSize(int exactSize) { AssumeCondition(exactSize == UCellSize) }
   void setBitSize(int exactSize) { AssumeCondition((exactSize + 8*sizeof(uint32_t)-1)/(8*sizeof(uint32_t)) == UCellSize) }
   void setCellSize(int exactSize) { AssumeCondition(exactSize == UCellSize) }
   void clear() { memset(auArray, 0, UCellSize*sizeof(uint32_t)); }
   void swap(thisType& source)
      {  uint32_t temp[UCellSize];
         memcpy(temp, auArray, UCellSize*sizeof(uint32_t));
         memcpy(auArray, source.auArray, UCellSize*sizeof(uint32_t));
         memcpy(source.auArray, temp, UCellSize*sizeof(uint32_t));
      }
   uint32_t* arrayStart() { return auArray; }
   const uint32_t* arrayStart() const { return auArray; }
};

template <>
class TBasicCellIntegerTraits<0> : public CellIntegerTraitsContract {
  private:
   typedef TBasicCellIntegerTraits<0> thisType;

  protected:
   uint32_t* _array() { return nullptr; }
   const uint32_t* _array() const { return nullptr; }

  public:
   TBasicCellIntegerTraits() {}
   TBasicCellIntegerTraits(const thisType& /* source */) : CellIntegerTraitsContract() {}
   thisType& operator=(const thisType& /* source */) { return *this; }
   thisType& operator=(uint32_t /* source */) { return *this; }
   typedef uint32_t& ArrayProperty;
   ArrayProperty array(int /* index */) { AssumeUncalled uint32_t* result = nullptr; return *result; }
   uint32_t array(int /* index */) const { AssumeUncalled return 0; }
   uint32_t carray(int /* index */) const { AssumeUncalled return 0; }
   ArrayProperty operator[](int /* index */) { AssumeUncalled uint32_t* result = nullptr; return *result; }
   uint32_t operator[](int /* index */) const { AssumeUncalled return 0; }

   static int getSize() { return 0; }
   void normalize() {}
   void adjustSize(int /* newSize */) { AssumeUncalled }
   void assertSize(int newSize) { AssumeCondition(newSize == 0) }
   void setSize(int exactSize) { AssumeCondition(exactSize == 0) }
   void setBitSize(int exactSize) { AssumeCondition(exactSize == 0) }
   void setCellSize(int exactSize) { AssumeCondition(exactSize == 0) }
   void clear() {}
   void swap(thisType& /* source */) {}
   uint32_t* arrayStart() { return nullptr; }
   const uint32_t* arrayStart() const { return nullptr; }
};

}

#define DefineBigCellClass
#include "ForwardNumerics/BaseInteger.inch"
#undef DefineBigCellClass

typedef DInteger::TBigCellInt<DInteger::TCellIntegerTraits<1> > AloneBigCellInt;

template <>
class TBigCellInt<DInteger::TCellIntegerTraits<1> > : public AloneBigCellInt {
  private:
   typedef TBigCellInt<DInteger::TCellIntegerTraits<1> > thisType;
   typedef AloneBigCellInt inherited;

   uint32_t& value() { return inherited::array(0); }
   uint32_t& svalue() { return inherited::array(0); }
   uint32_t value() const { return inherited::array(0); }

   static uint32_t add(uint32_t& cell, uint32_t value)
      {  auto temp = cell;
         cell += value;
         return (cell < temp) ? 1U : 0U;
      }
   static uint32_t sub(uint32_t& cell, uint32_t value)
      {  auto temp = cell;
         cell -= value;
         return (cell > temp) ? 1U : 0U;
      }

  protected:
   static int log_base_2(uint32_t value) { return DInteger::Access::log_base_2(value); }
   uint32_t& array(int /* index */) { return value(); }
   uint32_t array(int /* index */) const { return value(); }
   uint32_t carray(int /* index */) const { return value(); }

  public:
   class MidArray {
     private:
      uint32_t& uArrayValue;
      int uIndex;

     public:
      MidArray(thisType& source, int index)
         : uArrayValue(source.value()), uIndex(index) {}
      MidArray(MidArray&&) = default;
      MidArray(const MidArray&) = delete;
      MidArray& operator=(uint32_t value)
         {  uArrayValue = (uIndex > 0)
               ? ((value << (4*sizeof(uint32_t)))
                  | (uArrayValue & ~(~(uint32_t) 0 << 4*sizeof(uint32_t))))
               : ((uArrayValue & (~(uint32_t) 0 << 4*sizeof(uint32_t))) | value);
            return *this;
         }
      operator uint32_t() const
         {  return (uIndex > 0) ? (uArrayValue >> 4*sizeof(uint32_t))
               :  (uArrayValue & ~(~(uint32_t) 0 << 4*sizeof(uint32_t)));
         }
   };
   friend class MidArray;
   MidArray midArray(int index) { return MidArray(*this, index); }
   uint32_t midArray(int index) const
      {  return (index > 0) ? (value() >> 4*sizeof(uint32_t))
            :  (value() & ~(~(uint32_t) 0 << 4*sizeof(uint32_t)));
      }
   uint32_t cmidArray(int index) const { return midArray(index); }
   void setMidArray(int index, uint32_t value)
      {  svalue() = (index > 0)
            ? ((value << (4*sizeof(uint32_t)))
               | (svalue() & ~(~(uint32_t) 0 << 4*sizeof(uint32_t))))
            : ((svalue() & (~(uint32_t) 0 << 4*sizeof(uint32_t))) | value);
      }

   class BitArray {
     private:
      uint32_t* puArrayValue;
      int uIndex;

     public:
      BitArray(thisType& source, int index) : puArrayValue(&source.svalue()), uIndex(index) {}
      BitArray(BitArray&& source) = default;
      BitArray(const BitArray& source) = delete;
      // BitArray& operator=(BitArray&& source) = default;
      BitArray& operator=(const BitArray& source) = delete;
      BitArray& operator=(bool value)
         {  if (value)
               *puArrayValue |= ((uint32_t) 1 << uIndex);
            else
               *puArrayValue &= ~((uint32_t) 1 << uIndex);
            return *this;
         }
      operator bool() const
         {  return (*puArrayValue & ((uint32_t) 1 << uIndex)) ? true : false; }
   };
   friend class BitArray;
   BitArray bitArray(int index) { return BitArray(*this, index); }
   bool bitArray(int index) const { return (value() & ((uint32_t) 1 << index)) ? true : false; }
   bool cbitArray(int index) const { return bitArray(index); }
   void setBitArray(int index, bool value)
      {  if (value)
            svalue() |= ((uint32_t) 1 << index);
         else
            svalue() &= ~((uint32_t) 1 << index);
      }
   void setTrueBitArray(int index) { svalue() |= ((uint32_t) 1 << index); }
   void setFalseBitArray(int index) { svalue() &= ~((uint32_t) 1 << index); }

   TBigCellInt() : inherited() {}
   TBigCellInt(uint32_t value) : inherited(value) {}
   TBigCellInt(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   thisType& operator=(uint32_t value) { svalue() = value; return *this; }

   uint32_t operator[](int /* index */) const { return value(); }
   uint32_t& operator[](int /* index */) { return value(); }
   ComparisonResult compare(const thisType& source) const
      {  return (value() < source.value()) ? CRLess
            : ((value() > source.value()) ? CRGreater : CREqual);
      }
   std::strong_ordering operator<=>(const thisType& source) const
      {  return value() <=> source.value(); }
   bool operator==(const thisType& source) const
      {  return value() == source.value(); }
   thisType& operator<<=(int shift) { value() <<= shift; return *this; }
   thisType& operator>>=(int shift) { value() >>= shift; return *this; }
   void leftShiftLocal(int index, int shift)
      {  uint32_t temp = value();
         temp &= (~(uint32_t) 0 << index);
         temp <<= shift;
         value() = temp | (value() & ~(~(uint32_t) 0 << index));
      }
   void rightShiftLocal(int index, int shift)
      {  uint32_t temp = value();
         temp &= (~(uint32_t) 0 << index);
         temp >>= shift;
         value() = temp | (value() & ~(~(uint32_t) 0 << (index-shift)));
      }

   thisType& operator|=(const thisType& source) { value() |= source.value(); return *this; }
   thisType& operator^=(const thisType& source) { value() ^= source.value(); return *this; }
   thisType& operator&=(const thisType& source) { value() &= source.value(); return *this; }
   thisType& neg() { svalue() = ~svalue(); return *this; }
   thisType& neg(int shift)
      {  if (shift > 0)
            svalue() = (value() & (~(uint32_t) 0 << shift)) | (~value() & ~(~(uint32_t) 0 << shift));
         return *this;
      }
   thisType& clear(int shift)
      {  if (shift > 0)
            svalue() = (value() & (~(uint32_t) 0 << shift));
         return *this;
      }
   thisType& clearHigh(int shift)
      {  if (shift < (int) (8*sizeof(uint32_t)))
            svalue() = (value() & ~(~(uint32_t) 0 << shift));
         return *this;
      }
   thisType& saturate(int shift)
      {  if (shift > 0)
            svalue() = (value() | ~(~(uint32_t) 0 << shift));
         return *this;
      }
   bool isZero() const { return value() == (uint32_t) 0U; }
   bool hasZero(int shift) const
      {  AssumeCondition(shift <= (int) (8*sizeof(uint32_t)))
         return (shift <= 0)
            || ((value() << (8*sizeof(uint32_t)-shift)) == (uint32_t) 0U);
      }
 
   typedef typename inherited::Carry Carry;
   Carry add(const thisType& source)
      {  value() += source.value();
         return Carry((value() < source.value()) ? 1U : 0U);
      }
   Carry sub(const thisType& source)
      {  Carry carry((value() < source.value()) ? 1U : 0U);
         value() -= source.value();
         return carry;
      }
   Carry plusAssign(const thisType& source) { return add(source); }
   Carry minusAssign(const thisType& source) { return sub(source); }
   Carry inc() { return Carry(++svalue() == (uint32_t) 0U ? 1U : 0U); }
   Carry dec() { return Carry(svalue()-- == (uint32_t) 0U ? 1U : 0U); }

   thisType& operator+=(const thisType& source) { add(source); return *this; }
   thisType operator+(const thisType& source) const
      {  thisType result = *this; result += source; return result; }
   thisType& operator-=(const thisType& source) { sub(source); return *this; }
   thisType operator-(const thisType& source) const
      {  thisType result = *this; result -= source; return result; }
   thisType& operator--() { dec(); return *this; }
   thisType& operator++() { inc(); return *this; }

   int getSize() const { return 1; }
   void assertSize(int newSize) { AssumeCondition(newSize <= 1) }
   
   thisType& operator/=(const thisType& source)
      {  AssumeCondition(source.value() != (uint32_t) 0U)
         value() /= source.value();
         return *this;
      }
   thisType& operator/=(uint32_t source)
      {  AssumeCondition(source != (uint32_t) 0U)
         value() /= source;
         return *this;
      }
   uint32_t operator%(uint32_t source) const
      {  return value() % source; }
   thisType& operator%=(const thisType& source)
      {  value() %= source.value(); return *this; }

   int log_base_2() const { return log_base_2(value()); }
   uint32_t getValue() const { return value(); }
   bool isAtomic() const { return true; }
   void swap(thisType& source) { inherited::swap(source); }
   void clear() { value() = (uint32_t) 0U; }
};

namespace DInteger {

template<int i>
struct enable_if_two {};
 
template<>
struct enable_if_two<2> { static const int value = 2; };

template <>
class TBasicCellIntegerTraits<
         enable_if_two<sizeof(uint64_t)/(sizeof(uint32_t))>::value>
   :  public CellIntegerTraitsContract {
  private:
   uint64_t ulValue;
   typedef TBasicCellIntegerTraits<2> thisType;

  protected:
   uint64_t& svalue() { return ulValue; }
   uint64_t& value() { return ulValue; }
   const uint64_t& value() const { return ulValue; }

  public:
   TBasicCellIntegerTraits() : ulValue(0) {}
   TBasicCellIntegerTraits(uint32_t value) : ulValue(value) {}
   TBasicCellIntegerTraits(const thisType& source)
      :  CellIntegerTraitsContract(), ulValue(source.ulValue) {}
   thisType& operator=(const thisType& source) { ulValue = source.ulValue; return *this; }
   thisType& operator=(uint32_t source) { ulValue = source; return *this; }
   class ArrayProperty {
     private:
      uint64_t* pulValue;
      int uIndex;

     public:
      ArrayProperty(thisType& source, int index) : pulValue(&source.svalue()), uIndex(index) {}
      ArrayProperty(ArrayProperty&& source) = default;
      ArrayProperty(const ArrayProperty& source) = delete;
      // ArrayProperty& operator=(ArrayProperty&& source) = default;
      ArrayProperty& operator=(const ArrayProperty& source) = delete;
      ArrayProperty& operator=(uint32_t value)
         {  uint64_t val = value;
            if (uIndex != 0) {
               val <<= (sizeof(uint32_t)*8);
               *pulValue &= ~(~(uint64_t) 0 << (sizeof(uint32_t)*8));
            }
            else
               *pulValue &= (~(uint64_t) 0 << (sizeof(uint32_t)*8));
            *pulValue |= val;
            return *this;
         }
      ArrayProperty& operator|=(uint32_t value)
         {  uint64_t val = value;
            if (uIndex != 0)
               val <<= (sizeof(uint32_t)*8);
            *pulValue |= val;
            return *this;
         }
      ArrayProperty& operator^=(uint32_t value)
         {  uint64_t val = value;
            if (uIndex != 0)
               val <<= (sizeof(uint32_t)*8);
            *pulValue ^= val;
            return *this;
         }
      ArrayProperty& operator&=(uint32_t value)
         {  uint64_t val = value;
            if (uIndex != 0) {
               val <<= (sizeof(uint32_t)*8);
               val |= ~(~(uint64_t) 0 << (sizeof(uint32_t)*8));
            }
            else
               val |= (~(uint64_t) 0 << (sizeof(uint32_t)*8));
            *pulValue &= val;
            return *this;
         }
      ArrayProperty& operator++()
         {  uint64_t val = *pulValue;
            if (uIndex == 0) {
               ++val;
               val &= ~(~(uint64_t) 0 << (sizeof(uint32_t)*8));
               *pulValue &= (~(uint64_t) 0 << (sizeof(uint32_t)*8));
            }
            else {
               val >>= (sizeof(uint32_t)*8);
               ++val;
               val <<= (sizeof(uint32_t)*8);
               *pulValue &= ~(~(uint64_t) 0 << (sizeof(uint32_t)*8));
            };
            *pulValue |= val;
            return *this;
         }
      ArrayProperty& operator+=(uint32_t inc)
         {  uint64_t val = *pulValue;
            if (uIndex == 0) {
               val += inc;
               val &= ~(~(uint64_t) 0 << (sizeof(uint32_t)*8));
               *pulValue &= (~(uint64_t) 0 << (sizeof(uint32_t)*8));
            }
            else {
               val >>= (sizeof(uint32_t)*8);
               val += inc;
               val <<= (sizeof(uint32_t)*8);
               *pulValue &= ~(~(uint64_t) 0 << (sizeof(uint32_t)*8));
            };
            *pulValue |= val;
            return *this;
         }
      ArrayProperty& operator--()
         {  uint64_t val = *pulValue;
            if (uIndex == 0) {
               --val;
               val &= ~(~(uint64_t) 0 << (sizeof(uint32_t)*8));
               *pulValue &= (~(uint64_t) 0 << (sizeof(uint32_t)*8));
            }
            else {
               val >>= (sizeof(uint32_t)*8);
               --val;
               val <<= (sizeof(uint32_t)*8);
               *pulValue &= ~(~(uint64_t) 0 << (sizeof(uint32_t)*8));
            };
            *pulValue |= val;
            return *this;
         }
      ArrayProperty& operator-=(uint32_t dec)
         {  uint64_t val = *pulValue;
            if (uIndex == 0) {
               val -= dec;
               val &= ~(~(uint64_t) 0 << (sizeof(uint32_t)*8));
               *pulValue &= (~(uint64_t) 0 << (sizeof(uint32_t)*8));
            }
            else {
               val >>= (sizeof(uint32_t)*8);
               val -= dec;
               val <<= (sizeof(uint32_t)*8);
               *pulValue &= ~(~(uint64_t) 0 << (sizeof(uint32_t)*8));
            };
            *pulValue |= val;
            return *this;
         }
      operator uint32_t() const
         {  uint32_t result;
            if (uIndex == 0)
               result = (uint32_t) *pulValue;
            else {
               uint64_t val = *pulValue;
               val >>= (sizeof(uint32_t)*8);
               result = (uint32_t) val;
            }
            return result;
         }
   };
   ArrayProperty array(int index)
      {  AssumeCondition(!(index & ~(uint32_t) 1))
         return ArrayProperty(*this, index);
      }
   uint32_t array(int index) const
      {  AssumeCondition(index >= 0)
         uint32_t result;
         if (index == 0)
            result = (uint32_t) ulValue;
         else if (index == 1) {
            uint64_t val = ulValue;
            val >>= (sizeof(uint32_t)*8);
            result = (uint32_t) val;
         }
         else
            result = 0;
         return result;
      }
   uint32_t carray(int index) const { return array(index); }
   ArrayProperty operator[](int index)
      {  AssumeCondition(!(index & ~(uint32_t) 1))
         return ArrayProperty(*this, index);
      }
   uint32_t operator[](int index) const
      {  AssumeCondition(index >= 0)
         return array(index);
      }

   static int getSize() { return 2; }
   void normalize() {}
   void adjustSize(int /* newSize */) { AssumeUncalled }
   void assertSize(int newSize) { AssumeCondition(newSize <= 2) }
   void setSize(int exactSize) { AssumeCondition(exactSize == 2) }
   void setBitSize(int exactSize) { AssumeCondition((exactSize + 8*sizeof(uint32_t)-1)/(8*sizeof(uint32_t)) == 2) }
   void setCellSize(int exactSize) { AssumeCondition(exactSize == 2) }
   void clear() { ulValue = 0; }
   void swap(thisType& source)
      {  uint64_t temp = ulValue;
         ulValue = source.ulValue;
         source.ulValue = temp;
      }
   uint64_t* arrayStart() { return &ulValue; }
   const uint64_t* arrayStart() const { return &ulValue; }
};

} // end of namespace DInteger

typedef DInteger::TBigCellInt<DInteger::TCellIntegerTraits<2> > DoubleBigCellInt;

template <>
class TBigCellInt<DInteger::TCellIntegerTraits<
         DInteger::enable_if_two<sizeof(uint64_t)/(sizeof(uint32_t))>::value> >
   :  public DoubleBigCellInt {
  private:
   typedef TBigCellInt<DInteger::TCellIntegerTraits<2> > thisType;
   typedef DoubleBigCellInt inherited;

   uint64_t& value() { return inherited::value(); } // { return *((uint64_t*) _array()); }
   uint64_t& svalue() { return inherited::value(); } // { return *((uint64_t*) _array()); }
   const uint64_t& value() const { return inherited::value(); } // { return *((uint64_t*) _array()); }

   static uint32_t add(uint32_t& cell, uint32_t value)
      {  uint32_t temp = cell;
         cell += value;
         return (cell < temp) ? 1U : 0U;
      }
   static uint32_t sub(uint32_t& cell, uint32_t value)
      {  uint32_t temp = cell;
         cell -= value;
         return (cell > temp) ? 1U : 0U;
      }

  protected:
   static int log_base_2(uint64_t value)
      {  int result = 1;
         while ((value >>= 1) != 0)
            ++result;
         return result;
      }
   static int log_base_2(uint32_t value)
      {  return DInteger::Access::log_base_2(value); }

  public:
   class MidArray {
     private:
      uint64_t* plluValue;
      int uIndex;

     public:
      MidArray(thisType& source, int index)
         : plluValue(&source.value()), uIndex(index) { AssumeCondition(uIndex < 4) }
      MidArray(MidArray&&) = default;
      MidArray(const MidArray&) = delete;
      MidArray& operator=(uint32_t value)
         {  *plluValue &= (~(~(uint64_t) 0 << 4*sizeof(uint32_t)) << (uIndex*4*sizeof(uint32_t)));
            *plluValue |= (((uint64_t) value) << (uIndex*4*sizeof(uint32_t)));
            return *this;
         }
      operator uint32_t() const
         {  return (uint32_t) ((*plluValue >> (uIndex*4*sizeof(uint32_t)))
               & ~(~(uint64_t) 0 << 4*sizeof(uint32_t)));
         }
   };
   friend class MidArray;
   MidArray midArray(int index) { return MidArray(*this, index); }
   uint32_t midArray(int index) const
      {  AssumeCondition(index < 4)
         return (uint32_t) ((value() >> (index*4*sizeof(uint32_t)))
               & ~(~(uint64_t) 0 << 4*sizeof(uint32_t)));
      }
   uint32_t cmidArray(int index) const { return midArray(index); }
   void setMidArray(int index, uint32_t value)
      {  AssumeCondition(index < 4)
         svalue() &= (~(~(uint64_t) 0 << 4*sizeof(uint32_t)) << (index*4*sizeof(uint32_t)));
         svalue() |= (((uint64_t) value) << (index*4*sizeof(uint32_t)));
      }

   class BitArray {
     private:
      uint64_t* plluValue;
      int uIndex;

     public:
      BitArray(thisType& source, int index)
         :  plluValue(&source.value()), uIndex(index)
         {  AssumeCondition((uint32_t) index < 2*8*sizeof(uint32_t)) }
      BitArray(BitArray&&) = default;
      BitArray(const BitArray&) = delete;
      // BitArray& operator=(BitArray&& source) = default;
      BitArray& operator=(const BitArray& source) = delete;
      BitArray& operator=(bool value)
         {  if (value)
               *plluValue |= ((uint64_t) 1 << uIndex);
            else
               *plluValue &= ~((uint64_t) 1 << uIndex);
            return *this;
         }
      operator bool() const
         {  return (*plluValue & ((uint64_t) 1 << uIndex)) != (uint64_t) 0; }
   };
   friend class BitArray;
   BitArray bitArray(int index)
      {  return BitArray(*this, index); }
   bool bitArray(int index) const
      {  AssumeCondition((uint32_t) index < 2*8*sizeof(uint32_t))
         return (value() & ((uint64_t) 1 << index)) != (uint64_t) 0;
      }
   bool cbitArray(int index) const { return bitArray(index); }
   void setBitArray(int index, bool value)
      {  AssumeCondition((uint32_t) index < 2*8*sizeof(uint32_t))
         if (value)
            svalue() |= ((uint64_t) 1 << index);
         else
            svalue() &= ~((uint64_t) 1 << index);
      }
   void setTrueBitArray(int index)
      {  AssumeCondition((uint32_t) index < 2*8*sizeof(uint32_t))
         value() |= ((uint64_t) 1 << index);
      }
   void setFalseBitArray(int index)
      {  AssumeCondition((uint32_t) index < 2*8*sizeof(uint32_t))
         value() &= ~((uint64_t) 1 << index);
      }

   TBigCellInt() = default;
   TBigCellInt(uint32_t value) : inherited(value) {}
   TBigCellInt(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   thisType& operator=(uint32_t value) { svalue() = value; return *this; }

   ComparisonResult compare(const thisType& source) const
      {  return (value() < source.value()) ? CRLess
            : ((value() > source.value()) ? CRGreater : CREqual);
      }
   std::strong_ordering operator<=>(const thisType& source) const
      {  return value() <=> source.value(); }
   bool operator==(const thisType& source) const
      {  return value() == source.value(); }
   thisType& operator<<=(int shift) { value() <<= shift; return *this; }
   thisType& operator>>=(int shift) { value() >>= shift; return *this; }
   void leftShiftLocal(int index, int shift)
      {  uint64_t temp = value();
         temp &= (~(uint64_t) 0 << index);
         temp <<= shift;
         value() = temp | (value() & ~(~(uint64_t) 0 << index));
      }
   void rightShiftLocal(int index, int shift)
      {  uint64_t temp = value();
         temp &= (~(uint64_t) 0 << index);
         temp >>= shift;
         value() = temp | (value() & ~(~(uint64_t) 0 << (index-shift)));
      }
   thisType& operator|=(const thisType& source) { value() |= source.value(); return *this; }
   thisType& operator^=(const thisType& source) { value() ^= source.value(); return *this; }
   thisType& operator&=(const thisType& source) { value() &= source.value(); return *this; }
   thisType& neg()
      {  value() = ~value();
         return *this;
      }
   thisType& neg(int shift)
      {  if (shift > 0)
            value() = (value() & (~(uint64_t) 0 << shift)) | (~value() & ~(~(uint64_t) 0 << shift));
         return *this;
      }
   thisType& clear(int shift)
      {  if (shift > 0)
            value() = (value() & (~(uint64_t) 0 << shift));
         return *this;
      }
   thisType& clearHigh(int shift)
      {  if (shift < (int) (8*sizeof(uint64_t)))
            value() = (value() & ~(~(uint64_t) 0 << shift));
         return *this;
      }
   thisType& saturate(int shift)
      {  if (shift > 0)
            value() = (value() | ~(~(uint64_t) 0 << shift));
         return *this;
      }
   bool isZero() const { return value() == (uint64_t) 0U; }
   bool hasZero(int shift) const
      {  AssumeCondition((uint32_t) shift <= 8*sizeof(uint64_t))
         return (shift <= 0)
            || ((value() << (8*sizeof(uint64_t)-shift)) == (uint64_t) 0);
      }
 
   Carry add(const thisType& source)
      {  value() += source.value();
         return Carry((value() < source.value()) ? 1U : 0U);
      }
   Carry sub(const thisType& source)
      {  Carry carry((value() < source.value()) ? 1U : 0U);
         value() -= source.value();
         return carry;
      }
   Carry plusAssign(const thisType& source) { return add(source); }
   Carry minusAssign(const thisType& source) { return sub(source); }
   Carry inc() { return Carry(++value() == (uint32_t) 0U ? 1U : 0U); }
   Carry dec() { return Carry(value()-- == (uint32_t) 0U ? 1U : 0U); }

   thisType& operator+=(const thisType& source) { add(source); return *this; }
   thisType operator+(const thisType& source) const
      {  thisType result = *this; result += source; return result; }
   thisType& operator-=(const thisType& source) { sub(source); return *this; }
   thisType operator-(const thisType& source) const
      {  thisType result = *this; result -= source; return result; }
   thisType& operator--() { dec(); return *this; }
   thisType& operator++() { inc(); return *this; }

   int getSize() const { return 2; }
   void assertSize(int newSize) { AssumeCondition(newSize <= 2) }
   
   thisType& operator/=(const thisType& source)
      {  AssumeCondition(source.value() != (uint32_t) 0U)
         value() /= source.value();
         return *this;
      }
   thisType& operator/=(uint32_t source)
      {  AssumeCondition(source != (uint32_t) 0U)
         value() /= source;
         return *this;
      }
   uint64_t operator%(uint32_t source) const
      {  return value() % source; }
   thisType& operator%=(const thisType& source)
      {  value() %= source.value(); return *this; }

   int log_base_2() const { return log_base_2(value()); }
   uint32_t getValue() const { return (uint32_t) value(); }
   bool isAtomic() const { return !carray(1); }
   void swap(thisType& source) { inherited::swap(source); }
   void clear() { value() = (uint32_t) 0U; }
};

#define DefineBigIntClass
#include "ForwardNumerics/BaseInteger.inch"
#undef DefineBigIntClass

} // end of namespace Numerics

