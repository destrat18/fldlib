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
// File      : BaseIntegerGeneric.h
// Description :
//   Definition of a class of integers with unbound size.
//

#pragma once

#include "ForwardNumerics/BaseIntegerCommon.h"

namespace Numerics {

class UnsignedLongBaseStoreTraits {
  public:
   static const int USizeBaseInBits = sizeof(uint64_t)*8;
   typedef uint64_t BaseType;
   typedef uint64_t* BaseTypePointer;
   typedef uint64_t& BaseTypeReference;
   typedef uint64_t BaseTypeConstReference;
   static int log_base_2(uint64_t value)
      {  int result = 1;
         while ((value >>= 1) != 0)
            ++result;
         return result;
      }
   static void clearArray(uint64_t* array, int count)
      {  memset(array, 0, count*sizeof(uint64_t)); }
   static void copyArray(uint64_t* target, const uint64_t* source, int count)
      {  memcpy(target, source, count*sizeof(uint64_t)); }
   static int sizeBaseInBits() { return 8*sizeof(uint64_t); }
   static int minCellsCountToStoreBits(int bitsNumber)
      {  return (int) ((bitsNumber + 8*sizeof(uint64_t) - 1) / (8*sizeof(uint64_t))); } 
   static void swapArray(uint64_t* target, uint64_t* source, int count)
      {  uint64_t temp;
         for (int i = 0; i < count; ++i) {
            temp = target[i];
            target[i] = source[i];
            source[i] = temp;
         };
      }
   static uint64_t detectCarryAfterAddition(uint64_t result, uint64_t operand)
      {  return (result < operand) ? (uint64_t) 1 : (uint64_t) 0; }
   static uint64_t detectCarryBeforeSubstraction(uint64_t first, uint64_t second)
      {  return (first < second) ? (uint64_t) 1 : (uint64_t) 0; }
   static uint64_t getStoreMidHighPart(uint64_t value, uint64_t store)
      {  return ((value << (4*sizeof(uint64_t))) | (store & ~(~(uint64_t) 0 << 4*sizeof(uint64_t)))); }
   static void storeIntoMidHighPart(uint64_t value, uint64_t& store)
      {  store &= ~(~(uint64_t) 0 << 4*sizeof(uint64_t));
         store |= (value << (4*sizeof(uint64_t)));
      }
   static uint64_t getStoreMidLowPart(uint64_t value, uint64_t store)
      {  return ((store & (~(uint64_t) 0 << 4*sizeof(uint64_t))) | value); }
   static void storeIntoMidLowPart(uint64_t value, uint64_t& store)
      {  store &= (~(uint64_t) 0 << 4*sizeof(uint64_t));
         store |= value;
      }
   static uint64_t getMidHighPart(uint64_t value)
      {  return value >> 4*sizeof(uint64_t); }
   static uint64_t getMidLowPart(uint64_t value)
      {  return value & ~(~(uint64_t) 0 << 4*sizeof(uint64_t)); }
   static uint64_t getLowPart(uint64_t value, int shift)
      {  return value & ~(~(uint64_t) 0 << shift); }
   static uint64_t getHighPart(uint64_t value, int shift)
      {  return (value >> shift); }
   static uint64_t getMiddlePart(uint64_t value, int lowBit, int sizeInBits)
      {  return (value >> lowBit) & ~(~(uint64_t) 0 << sizeInBits); }
   static uint64_t getStoreHighPart(uint64_t value, int shift)
      {  return (value << shift); }
   static uint64_t getStoreHighPart(uint64_t value, int shift, uint64_t store)
      {  return (value << shift) | (store & ~(~(uint64_t) 0 << shift)); }
   static void storeIntoHighPart(uint64_t value, int shift, uint64_t& store)
      {  store &= ~(~(uint64_t) 0 << shift);
         store |= (value << shift);
      }
   static uint64_t getStoreLowPart(uint64_t value, int shift)
      {  return value & ~(~(uint64_t) 0 << shift); }
   static uint64_t getStoreLowPart(uint64_t value, int shift, uint64_t store)
      {  return (store & (~(uint64_t) 0 << shift)) | (value & ~(~(uint64_t) 0 << shift)); }
   static void storeIntoLowPart(uint64_t value, int shift, uint64_t& store)
      {  store &= (~(uint64_t) 0 << shift);
         store |= (value & ~(~(uint64_t) 0 << shift));
      }
   static void setTrueBit(uint64_t& result, int index) { result |= ((uint64_t) 1 << index); }
   static void setFalseBit(uint64_t& result, int index) { result &= ~((uint64_t) 1 << index); }
   static void setBit(uint64_t& result, int index, bool value)
      {  if (value)
            result |= ((uint64_t) 1 << index);
         else
            result &= ~((uint64_t) 1 << index);
      }
   static bool getBit(uint64_t value, int index)
      {  return (value & ((uint64_t) 1 << index)) ? true : false; }
   static void leftShiftLocal(uint64_t& value, int index, int shift)
      {  uint64_t temp = value;
         temp &= (~(uint64_t) 0 << index);
         temp <<= shift;
         value = temp | (value & ~(~(uint64_t) 0 << index));
      }
   static void rightShiftLocal(uint64_t& value, int index, int shift)
      {  uint64_t temp = value;
         temp &= (~(uint64_t) 0 << index);
         temp >>= shift;
         value = temp | (value & ~(~(uint64_t) 0 << (index-shift)));
      }
   static void leftShiftAndClearLow(uint64_t& value, int shift, int low)
      {  value <<= shift;
         value &= ~(uint64_t) 0 << low;
      }
   static void rightShiftAndClearHigh(uint64_t& value, int shift, int high)
      {  value >>= shift;
         value &= ~(~(uint64_t) 0 << high);
      }
   static void rightShift(uint64_t& value, int shift) { value >>= shift; }
   static void leftShift(uint64_t& value, int shift) { value <<= shift; }
   static void negLowValuePart(uint64_t& value, int shift)
      {  if (shift > 0)
            value = (value & (~(uint64_t) 0 << shift)) | (~value & ~(~(uint64_t) 0 << shift));
      }
   static void clearLowValuePart(uint64_t& value, int shift)
      {  if (shift > 0)
            value &= (~(uint64_t) 0 << shift);
      }
   static void clearHighValuePart(uint64_t& value, int shift)
      {  if (shift < (int) (8*sizeof(uint64_t)))
            value &= ~(~(uint64_t) 0 << shift);
      }
   static uint64_t getSaturation() { return ~(uint64_t) 0; }
   static void saturateLowValuePart(uint64_t& value, int shift)
      {  if (shift > 0)
            value |= ~(~(uint64_t) 0 << shift);
      }
   static bool isZeroValue(uint64_t value) { return value == (uint64_t) 0; }
   static bool hasZeroValue(uint64_t value, int shift)
      {  AssumeCondition(shift <= (int) (8*sizeof(uint64_t)))
         return (shift <= 0) || ((value << (8*sizeof(uint64_t)-shift)) == (uint64_t) 0);
      }
   static bool isOneValue(uint64_t value) { return value == (uint64_t) 1; }
   static void writeValue(STG::IOObject::OSBase& out, uint64_t value, bool isRaw)
#ifdef DefineNoEnhancedObject
      {  if (!isRaw)
            out << value;
         else
            out.write(reinterpret_cast<char*>(&value), sizeof(value));
      }
#else
      {  out.write(value, isRaw); }
#endif
   static void readValue(STG::IOObject::ISBase& in, uint64_t& value, bool isRaw)
      {
#ifdef DefineNoEnhancedObject
         if (isRaw) {
            in.read(reinterpret_cast<char*>(&value), sizeof(value));
         }
         else
            in >> value;
#else
         in.read(value, isRaw);
#endif
      }
};

/* IntegerTraits should conform to DInteger::CellIntegerTraitsContract */
template <class BaseStoreTraits, class IntegerTraits>
class TGBigCellInt;
template <class BaseStoreTraits, class IntegerTraits>
class TGBigCell;
template <class BaseStoreTraits, class IntegerTraits>
class TGBigInt;

namespace DInteger {

/* BaseStoreTraits should conform to UnsignedBaseStoreTraits with optionally another base type */
template <class BaseStoreTraits>
class GCellIntegerTraitsContract : public BaseStoreTraits {
  public:
   GCellIntegerTraitsContract() {}
   GCellIntegerTraitsContract(const GCellIntegerTraitsContract& source) { AssumeUncalled }
   GCellIntegerTraitsContract& operator=(const GCellIntegerTraitsContract& source) { AssumeUncalled return *this; }

   typedef typename BaseStoreTraits::BaseTypeReference ArrayProperty;
   ArrayProperty array(int index) { AssumeUncalled typename BaseStoreTraits::BaseTypePointer result = nullptr; return *result; }
   typename BaseStoreTraits::BaseType array(int index) const { AssumeUncalled return 0; }
   typename BaseStoreTraits::BaseType carray(int index) const { AssumeUncalled return 0; }
   ArrayProperty operator[](int index) { AssumeUncalled typename BaseStoreTraits::BaseType* result = nullptr; return *result; }
   typename BaseStoreTraits::BaseTypeConstReference operator[](int index) const { AssumeUncalled return 0; }
   typedef GCellIntegerTraitsContract MultResult;

   typedef GCellIntegerTraitsContract QuotientResult;
   typedef GCellIntegerTraitsContract RemainderResult;
   typedef GCellIntegerTraitsContract NormalizedRemainderResult;
   void copyLow(const MultResult& result) { AssumeUncalled }
   int getSize() const { AssumeUncalled return 0; }
   void adjustSize(int newSize) { AssumeUncalled }
   void setSize(int exactSize) { AssumeUncalled }
   void setBitSize(int exactSize) { AssumeUncalled }
   void setCellSize(int exactSize) { AssumeUncalled }
   void normalize() { AssumeUncalled }
   void assertSize(int newSize) { AssumeUncalled }
   void clear() { AssumeUncalled }
   typedef GCellIntegerTraitsContract ExtendedInteger;
};

template <class BaseStoreTraits, int UCellSize>
class TGBasicCellIntegerTraits : public BaseStoreTraits {
  public:
   typedef typename BaseStoreTraits::BaseType BaseType;
   typedef typename BaseStoreTraits::BaseTypeConstReference BaseTypeConstReference;

  private:
   typename BaseStoreTraits::BaseType auArray[UCellSize];
   typedef TGBasicCellIntegerTraits<BaseStoreTraits, UCellSize> thisType;

  protected:
   BaseType* _array() { return auArray; }
   const BaseType* _array() const { return auArray; }

  public:
   TGBasicCellIntegerTraits() { BaseStoreTraits::clearArray(auArray, UCellSize); }
   TGBasicCellIntegerTraits(const thisType& source)
      {  BaseStoreTraits::copyArray(auArray, source.auArray, UCellSize); }
   thisType& operator=(const thisType& source)
      {  BaseStoreTraits::copyArray(auArray, source.auArray, UCellSize);
         return *this;
      }
   thisType& operator=(BaseTypeConstReference source)
      {  if (UCellSize > 1)
            BaseStoreTraits::clearArray(auArray, UCellSize);
         auArray[0] = source;
         return *this;
      }
   typedef BaseType& ArrayProperty;
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
   ArrayProperty array(int index)
      {  AssumeCondition(index >= 0 && (index < UCellSize))
         return auArray[index];
      }
   BaseType array(int index) const
      {  AssumeCondition(index >= 0)
         BaseType result;
         if (index < UCellSize)
            result = auArray[index];
         else
            result = 0x0;
         return result;
      }
   BaseTypeConstReference carray(int index) const { return array(index); }
   ArrayProperty operator[](int index)
      {  AssumeCondition(index >= 0 && (index < UCellSize))
         return auArray[index];
      }
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
   BaseType operator[](int index) const
      {  AssumeCondition(index >= 0)
         BaseType result;
         if (index < UCellSize)
            result = auArray[index];
         else
            result = 0x0;
         return result;
      }

   static int getSize() { return UCellSize; }
   void normalize() {}
   void adjustSize(int /* newSize */) { AssumeUncalled }
   void assertSize(int newSize) { AssumeCondition(newSize <= UCellSize) }
   void setSize(int exactSize) { AssumeCondition(exactSize == UCellSize) }
   void setBitSize(int exactSize)
      {  AssumeCondition(BaseStoreTraits::minCellsCountToStoreBits(exactSize) == UCellSize) }
   void setCellSize(int exactSize) { AssumeCondition(exactSize == UCellSize) }
   void clear() { BaseStoreTraits::clearArray(auArray, UCellSize); }
   void swap(thisType& source) { BaseStoreTraits::swapArray(auArray, source.auArray, UCellSize); }
   BaseType* arrayStart() { return auArray; }
   const BaseType* arrayStart() const { return auArray; }
};

template <class BaseStoreTraits>
class TGBasicCellIntegerTraits<BaseStoreTraits, 0> : public GCellIntegerTraitsContract<BaseStoreTraits> {
  private:
   typedef TGBasicCellIntegerTraits<BaseStoreTraits, 0> thisType;

  protected:
   typename BaseStoreTraits::BaseType* _array() { return nullptr; }
   const typename BaseStoreTraits::BaseType* _array() const { return nullptr; }

  public:
   TGBasicCellIntegerTraits() {}
   TGBasicCellIntegerTraits(const thisType& source) {}
   thisType& operator=(const thisType& source) { return *this; }
   thisType& operator=(typename BaseStoreTraits::BaseType source) { return *this; }
   typedef typename BaseStoreTraits::BaseTypeReference ArrayProperty;
   ArrayProperty array(int index) { AssumeUncalled typename BaseStoreTraits::BaseType* result = nullptr; return *result; }
   typename BaseStoreTraits::BaseTypeConstReference array(int index) const { AssumeUncalled return 0; }
   typename BaseStoreTraits::BaseTypeConstReference carray(int index) const { AssumeUncalled return 0; }
   ArrayProperty operator[](int index) { AssumeUncalled typename BaseStoreTraits::BaseTypePointer result = nullptr; return *result; }
   typename BaseStoreTraits::BaseTypeConstReference operator[](int index) const { AssumeUncalled return 0; }

   static int getSize() { return 0; }
   void normalize() {}
   void adjustSize(int newSize) { AssumeUncalled }
   void assertSize(int newSize) { AssumeCondition(newSize == 0) }
   void setSize(int exactSize) { AssumeCondition(exactSize == 0) }
   void setBitSize(int exactSize) { AssumeCondition(exactSize == 0) }
   void setCellSize(int exactSize) { AssumeCondition(exactSize == 0) }
   void clear() {}
   void swap(thisType& source) {}
   typename BaseStoreTraits::BaseType* arrayStart() { return nullptr; }
   const typename BaseStoreTraits::BaseType* arrayStart() const { return nullptr; }
};

} // end of namespace DInteger

#define DefineGeneric
#define DefineBigCellClass
#include "ForwardNumerics/BaseInteger.inch"
#undef DefineGeneric
#undef DefineBigCellClass

template <class BaseStoreTraits>
class TGBigCellInt<BaseStoreTraits, DInteger::TGCellIntegerTraits<BaseStoreTraits, 1> >
   :  public DInteger::TGBigCellInt<BaseStoreTraits, DInteger::TGCellIntegerTraits<BaseStoreTraits, 1> > {
  public:
   typedef typename BaseStoreTraits::BaseType BaseType;
   typedef typename BaseStoreTraits::BaseTypeReference BaseTypeReference;
   typedef typename BaseStoreTraits::BaseTypeConstReference BaseTypeConstReference;

  private:
   typedef TGBigCellInt<BaseStoreTraits, DInteger::TGCellIntegerTraits<BaseStoreTraits, 1> > thisType;
   typedef DInteger::TGBigCellInt<BaseStoreTraits, DInteger::TGCellIntegerTraits<BaseStoreTraits, 1> > inherited;

   BaseTypeReference value() { return inherited::operator[](0); }
   BaseTypeReference svalue() { return inherited::operator[](0); }
   BaseTypeConstReference value() const { return inherited::operator[](0); }

// static BaseType add(BaseTypeReference cell, BaseType value)
//    {  BaseType temp = cell;
//       cell += value;
//       return (cell < temp) ? 1 : 0;
//    }
// static BaseType sub(BaseTypeReference cell, BaseType value)
//    {  BaseType temp = cell;
//       cell -= value;
//       return (cell > temp) ? 1 : 0;
//    }

  protected:
   static int log_base_2(BaseTypeConstReference value) { return BaseStoreTraits::log_base_2(value); }
   BaseTypeReference array(int index) { return inherited::value(); }
   BaseTypeConstReference array(int index) const { return inherited::value(); }
   BaseTypeConstReference carray(int index) const { return inherited::value(); }

  public:
   class MidArray {
     private:
      BaseTypeReference uArrayValue;
      int uIndex;

     public:
      MidArray(thisType& source, int index)
         : uArrayValue(source.value()), uIndex(index) {}
      MidArray(MidArray&&) = default;
      MidArray(const MidArray&) = delete;
      MidArray& operator=(BaseTypeConstReference value)
         {  if (uIndex > 0)
               BaseStoreTraits::storeIntoMidHighPart(value, uArrayValue);
            else
               BaseStoreTraits::storeIntoMidLowPart(value, uArrayValue);
            return *this;
         }
      operator BaseType() const
         {  return (uIndex > 0)
               ? BaseStoreTraits::getMidHighPart(uArrayValue)
               : BaseStoreTraits::getMidLowPart(uArrayValue);
         }
   };
   friend class MidArray;
   MidArray midArray(int index) { return MidArray(*this, index); }
   BaseType midArray(int index) const
      {  return (index > 0)
            ? BaseStoreTraits::getMidHighPart(value())
            : BaseStoreTraits::getMidLowPart(value());
      }
   BaseType cmidArray(int index) const
      {  return (index > 0)
            ? BaseStoreTraits::getMidHighPart(value())
            : BaseStoreTraits::getMidLowPart(value());
      }
   void setMidArray(int index, BaseTypeConstReference midValue)
      {  if (index > 0)
            BaseStoreTraits::storeIntoMidHighPart(midValue, svalue());
         else
            BaseStoreTraits::storeIntoMidLowPart(midValue, svalue());
      }

   class BitArray {
     private:
      BaseType* puArrayValue;
      int uIndex;

     public:
      BitArray(thisType& source, int index) : puArrayValue(&source.svalue()), uIndex(index) {}
      BitArray(BitArray&& source) = default;
      BitArray(const BitArray& source) = delete;
      BitArray& operator=(const BitArray& source) = delete;
      BitArray& operator=(bool value)
         {  if (value)
               BaseStoreTraits::setTrueBit(*puArrayValue, uIndex);
            else
               BaseStoreTraits::setFalseBit(*puArrayValue, uIndex);
            return *this;
         }
      operator bool() const
         {  return BaseStoreTraits::getBit(*puArrayValue, uIndex); }
   };
   friend class BitArray;
   BitArray bitArray(int index) { return BitArray(*this, index); }
   bool bitArray(int index) const
      {  return BaseStoreTraits::getBit(value(), index); }
   bool cbitArray(int index) const
      {  return BaseStoreTraits::getBit(value(), index); }
   void setBitArray(int index, bool value)
      {  if (value)
            BaseStoreTraits::setTrueBit(svalue(), index);
         else
            BaseStoreTraits::setFalseBit(svalue(), index);
      }
   void setTrueBitArray(int index)
      {  BaseStoreTraits::setTrueBit(svalue(), index); }
   void setFalseBitArray(int index)
      {  BaseStoreTraits::setFalseBit(svalue(), index); }

   TGBigCellInt() = default;
   TGBigCellInt(BaseTypeConstReference value) : inherited(value) {}
   TGBigCellInt(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   thisType& operator=(BaseTypeConstReference value) { svalue() = value; return *this; }

   BaseTypeConstReference operator[](int index) const { return value(); }
   BaseTypeReference& operator[](int /* index */) { return value(); }
   ComparisonResult compare(const thisType& source) const
      {  return (value() < source.value()) ? CRLess
            : ((value() > source.value()) ? CRGreater : CREqual);
      }
   std::strong_ordering operator<=>(const thisType& source) const { return value() <=> source.value(); }
   bool operator==(const thisType& source) const { return value() == source.value(); }
   thisType& operator<<=(int shift) { value() <<= shift; return *this; }
   thisType& operator>>=(int shift) { value() >>= shift; return *this; }
   void leftShiftLocal(int index, int shift) { BaseStoreTraits::leftShiftLocal(value(), index, shift); }
   void rightShiftLocal(int index, int shift) { BaseStoreTraits::rightShiftLocal(value(), index, shift); }

   thisType& operator|=(const thisType& source) { value() |= source.value(); return *this; }
   thisType& operator^=(const thisType& source) { value() ^= source.value(); return *this; }
   thisType& operator&=(const thisType& source) { value() &= source.value(); return *this; }
   thisType& neg() { svalue() = ~svalue(); return *this; }
   thisType& neg(int shift)
      {  BaseStoreTraits::negLowValuePart(svalue(), shift); return *this; }
   thisType& clear(int shift)
      {  BaseStoreTraits::clearLowValuePart(svalue(), shift); return *this; }
   thisType& clearHigh(int shift)
      {  BaseStoreTraits::clearHighValuePart(svalue(), shift); return *this; }
   thisType& saturate(int shift)
      {  BaseStoreTraits::saturateLowValuePart(svalue(), shift); return *this; }
   bool isZero() const { return BaseStoreTraits::isZeroValue(value()); }
   bool hasZero(int shift) const { return BaseStoreTraits::hasZeroValue(value(), shift); }
 
   typedef typename inherited::Carry Carry;
   Carry add(const thisType& source)
      {  value() += source.value();
         return Carry(BaseStoreTraits::detectCarryAfterAddition(value(), source.value()));
      }
   Carry sub(const thisType& source)
      {  Carry carry(BaseStoreTraits::detectCarryBeforeSubstraction(value(), source.value()));
         value() -= source.value();
         return carry;
      }
   Carry plusAssign(const thisType& source) { return add(source); }
   Carry minusAssign(const thisType& source) { return sub(source); }
   Carry inc() { ++svalue(); return Carry(BaseStoreTraits::isZeroValue(value()) ? 0x1 : 0x0); }
   Carry dec()
      {  Carry carry(BaseStoreTraits::isZeroValue(value()) ? 0x1 : 0x0);
         --svalue();
         return carry;
      }

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
      {  AssumeCondition(BaseStoreTraits::isZero(source.value()))
         value() /= source.value();
         return *this;
      }
   thisType& operator/=(BaseTypeConstReference source)
      {  AssumeCondition(BaseStoreTraits::isZero(source))
         value() /= source;
         return *this;
      }
   BaseType operator%(BaseTypeConstReference source) const
      {  return value() % source; }
   thisType& operator%=(const thisType& source)
      {  svalue() %= source.value(); return *this; }

   BaseType log_base_2() const { return BaseStoreTraits::log_base_2(value()); }
   BaseTypeConstReference getValue() const { return value(); }
   bool isAtomic() const { return true; }
   void swap(thisType& source) { inherited::swap(source); }
   void clear() { value() = 0x0; }
};

namespace DInteger {

template<int i>
struct genable_if_two {};
 
template<>
struct genable_if_two<2> { static const int value = 2; };

template <>
class TGBasicCellIntegerTraits<UnsignedBaseStoreTraits,
         genable_if_two<sizeof(uint64_t)/(sizeof(uint32_t))>::value>
   :  public GCellIntegerTraitsContract<UnsignedBaseStoreTraits> {
  private:
   uint64_t ulValue;
   typedef TGBasicCellIntegerTraits<UnsignedBaseStoreTraits, 2> thisType;

  protected:
   uint64_t& svalue() { return ulValue; }
   uint64_t& value() { return ulValue; }
   const uint64_t& value() const { return ulValue; }

  public:
   TGBasicCellIntegerTraits() : ulValue(0) {}
   TGBasicCellIntegerTraits(uint32_t value) : ulValue(value) {}
   TGBasicCellIntegerTraits(const thisType& source)
      :  GCellIntegerTraitsContract<UnsignedBaseStoreTraits>(), ulValue(source.ulValue) {}
   thisType& operator=(const thisType& source) { ulValue = source.ulValue; return *this; }
   thisType& operator=(uint32_t source) { ulValue = source; return *this; }
   class ArrayProperty {
     private:
      uint64_t* pulValue;
      int uIndex;

     public:
      ArrayProperty(thisType& source, int index) : pulValue(&source.svalue()), uIndex(index) {}
      ArrayProperty(ArrayProperty&& source) = default;
      // ArrayProperty(const ArrayProperty& source) = delete;
      ArrayProperty(const ArrayProperty& source) = default;
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

typedef DInteger::TGBigCellInt<UnsignedBaseStoreTraits,
      DInteger::TGCellIntegerTraits<UnsignedBaseStoreTraits, 2> > GDoubleBigCellInt;

template <>
class TGBigCellInt<UnsignedBaseStoreTraits,
   DInteger::TGCellIntegerTraits<UnsignedBaseStoreTraits,
         DInteger::genable_if_two<sizeof(uint64_t)/(sizeof(uint32_t))>::value> >
   :  public GDoubleBigCellInt {
  private:
   typedef TGBigCellInt<UnsignedBaseStoreTraits,
         DInteger::TGCellIntegerTraits<UnsignedBaseStoreTraits, 2> > thisType;
   typedef GDoubleBigCellInt inherited;

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
      {  return UnsignedBaseStoreTraits::log_base_2(value); }

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

   TGBigCellInt() : inherited() {}
   TGBigCellInt(uint32_t value) : inherited(value) {}
   TGBigCellInt(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   thisType& operator=(uint32_t value) { svalue() = value; return *this; }

   ComparisonResult compare(const thisType& source) const
      {  return (value() < source.value()) ? CRLess
            : ((value() > source.value()) ? CRGreater : CREqual);
      }
   std::strong_ordering operator<=>(const thisType& source) const { return value() <=> source.value(); }
   bool operator==(const thisType& source) const { return value() == source.value(); }
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
   bool isZero() const { return value() == (uint64_t) 0; }
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
   Carry inc() { return Carry(++value() == (uint32_t) 0 ? 1U : 0U); }
   Carry dec() { return Carry(value()-- == (uint32_t) 0 ? 1U : 0U); }

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
      {  AssumeCondition(source.value() != (uint32_t) 0)
         value() /= source.value();
         return *this;
      }
   thisType& operator/=(uint64_t source)
      {  AssumeCondition(source != (uint64_t) 0)
         value() /= source;
         return *this;
      }
   uint64_t operator%(uint64_t source) const
      {  return value() % source; }
   thisType& operator%=(const thisType& source)
      {  value() %= source.value(); return *this; }

   int log_base_2() const { return log_base_2(value()); }
   uint64_t getValue() const { return value(); }
   bool isAtomic() const { return !carray(1); }
   void swap(thisType& source) { inherited::swap(source); }
   void clear() { value() = (uint64_t) 0; }
};

#define DefineGeneric
#define DefineBigIntClass
#include "ForwardNumerics/BaseInteger.inch"
#undef DefineGeneric
#undef DefineBigIntClass

} // end of namespace Numerics

