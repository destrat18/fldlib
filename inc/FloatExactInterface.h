/**************************************************************************/
/*                                                                        */
/*  Copyright (C) 2015-2025                                               */
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
// Unit      : FloatExact
// File      : FloatExactInterface.h
// Description :
//   Definition of a class of floating point comparison
//

#pragma once

// for log
#include <cmath>
#include "obj_interface/ExactTypesSize.h"

// #include <iosfwd>
#include <iostream>
#include <vector>
#include <functional>
#include <cassert>
#include <cstdint>

namespace NumericalDomains { namespace DDoubleExactInterface {

class end {};

class ExecutionPath {
  public:
   static void splitBranches(const char* file, int line);
   static std::pair<const char*, int> querySplitInfo();
   static void flushOut();
   static void setSourceLine(const char* file, int line);
   static int getCurrentUnstableBranch();
   static void writeCurrentPath(std::ostream& out);
   // float loop unstable
   static bool doesFollowFlow();
   static bool setFollowFlow(bool doesFollow=true);
   static void clearFollowFlow();
   static bool doesSupportUnstableInLoop();
   // end of float loop unstable

   static void setSupportAtomic();
   static void setSupportUnstableInLoop(bool value=true);
   static void setSupportBacktrace();
   static void setSupportVerbose();
   static void setSupportThreshold();
   static void setSupportFirstFollowFloat();
   static void initializeGlobals(const char* fileSuffix);
   static void finalizeGlobals();
   class Initialization {
     public:
      Initialization() {}
      void setSupportAtomic() { ExecutionPath::setSupportAtomic(); }
      void setSupportUnstableInLoop() { ExecutionPath::setSupportUnstableInLoop(); }
      void setSupportBacktrace() { ExecutionPath::setSupportBacktrace(); }
      void setSupportVerbose() { ExecutionPath::setSupportVerbose(); }
      void setSupportThreshold() { ExecutionPath::setSupportThreshold(); }
      void setSupportFirstFollowFloat() { ExecutionPath::setSupportFirstFollowFloat(); }
      void setResultFile(const char* fileSuffix) { initializeGlobals(fileSuffix); }
      ~Initialization() { finalizeGlobals(); }
   };

   class anticipated_termination { public: anticipated_termination() {} };
   class read_error {
     public:
      const char* message;
      read_error(const char* msg) : message(msg) {}
      const char* getMessage() const { return message; }
   };
   class precondition_error {
     public:
      const char* message;
      precondition_error(const char* msg) : message(msg) {}
      const char* getMessage() const { return message; }
      void print(std::ostream& out) { out << message << std::endl; }
   };
};

typedef ExecutionPath BaseFloatExact; // for splitBranches

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
class TFloatExact;

class MergeBranches {
  private:
   typedef void* /* size_t */ AlignType;
   static const int UMergeBranchesSizeInBytes = NumericalDomains::DDoubleExactInterface::MergeBranchesSize;
   static const int UMergeBranchesSize = (UMergeBranchesSizeInBytes + sizeof(AlignType)-1)/sizeof(AlignType);
   AlignType content[UMergeBranchesSize];

  public:
   MergeBranches(const char* file, int line);

   template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
   MergeBranches& operator<<(TFloatExact<USizeMantissa, USizeExponent, TypeImplementation>& value);
   bool operator<<(end);

   template <class TypeIterator>
   struct TPacker {
      TypeIterator iter, end;
      TPacker(TypeIterator aiter, TypeIterator aend) : iter(aiter), end(aend) {}
   };

   template <class TypeIterator>
   static TPacker<TypeIterator> packer(TypeIterator iter, TypeIterator end)
      {  return TPacker<TypeIterator>(iter, end); }

   template <class TypeIterator>
   MergeBranches& operator<<(TPacker<TypeIterator> packer)
      {  for (; packer.iter != packer.end; ++packer.iter)
            operator<<(*packer.iter);
         return *this;
      }
};

template <typename TypeIterator, class TypeSaveMemory>
class TPackedSaveMemory;

template <typename T1, class TypeSaveMemory>
class TSaveMemory {
  public:
   T1 save;
   TypeSaveMemory next;

   TSaveMemory(T1 saveArg, TypeSaveMemory nextArg) : save(saveArg), next(nextArg) {}
   TSaveMemory(const TSaveMemory<T1, TypeSaveMemory>&) = default;
   TSaveMemory(TSaveMemory<T1, TypeSaveMemory>&&) = default;

   template <typename T>
   TSaveMemory<T, TSaveMemory<T1, TypeSaveMemory> > operator<<(T t)
      {  return TSaveMemory<T, TSaveMemory<T1, TypeSaveMemory> >(t, *this); }
   template <typename TypeIterator>
   TPackedSaveMemory<TypeIterator, TSaveMemory<T1, TypeSaveMemory> > operator<<(MergeBranches::TPacker<TypeIterator> packer);
   TSaveMemory<T1, TypeSaveMemory>& operator<<(end) { return *this; }
   TSaveMemory<T1, TypeSaveMemory>& setCurrent(bool result)
      {  next.setCurrent(result); return *this; }
   TypeSaveMemory& operator>>(T1& val)
      {  if (!next.getResult())
            val = save;
         return next;
      }
   TypeSaveMemory& operator>>(const T1& aval)
      {  T1& val = const_cast<T1&>(aval);
         if (!next.getResult())
            val = save;
         return next;
      }
   bool getResult() const { return next.getResult(); }
};

template <typename TypeIterator, class TypeSaveMemory>
class TPackedSaveMemory {
  public:
   std::vector<typename TypeIterator::value_type> save;
   TypeSaveMemory next;

   TPackedSaveMemory(TypeIterator iter, TypeIterator end, TypeSaveMemory nextArg)
      :  next(nextArg)
      {  int count = end - iter;
         save.reserve(count);
         for (; iter != end; ++iter)
            save.push_back(*iter);
      }
   TPackedSaveMemory(const TPackedSaveMemory<TypeIterator, TypeSaveMemory>&) = default;
   TPackedSaveMemory(TPackedSaveMemory<TypeIterator, TypeSaveMemory>&&) = default;

   template <typename T>
   TSaveMemory<T, TSaveMemory<TypeIterator, TypeSaveMemory> > operator<<(T t)
      {  return TSaveMemory<T, TSaveMemory<TypeIterator, TypeSaveMemory> >(t, *this); }
   template <class TypeIteratorArgument>
   TPackedSaveMemory<TypeIteratorArgument, TPackedSaveMemory<TypeIterator, TypeSaveMemory> >
      operator<<(MergeBranches::TPacker<TypeIteratorArgument> packer)
      {  return TPackedSaveMemory<TypeIteratorArgument, TPackedSaveMemory<TypeIterator, TypeSaveMemory> >
            (packer.iter, packer.end, *this);
      }
   TPackedSaveMemory<TypeIterator, TypeSaveMemory>& operator<<(end) { return *this; }

   TPackedSaveMemory<TypeIterator, TypeSaveMemory>& setCurrent(bool result)
      {  next.setCurrent(result); return *this; }
   
   TypeSaveMemory& operator>>(MergeBranches::TPacker<TypeIterator>&& packer)
      {  if (!next.getResult()) {
            int count = packer.end - packer.iter;
            assert(count == (int) save.size());
            for (int index = 0; index < count; ++index) {
               *packer.iter = save[index];
               ++packer.iter;
            }
         }
         return next;
      }
   bool getResult() const { return next.getResult(); }
};

template <typename T1, class TypeSaveMemory>
template <typename TypeIterator>
inline
TPackedSaveMemory<TypeIterator, TSaveMemory<T1, TypeSaveMemory> >
TSaveMemory<T1, TypeSaveMemory>::operator<<(MergeBranches::TPacker<TypeIterator> packer)
   {  return TPackedSaveMemory<TypeIterator, TSaveMemory<T1, TypeSaveMemory> >
         (packer.iter, packer.end, *this);
   }

class SaveMemory {
  private:
   bool fResult;

  public:
   SaveMemory() : fResult(false) {}

   template <typename T>
   TSaveMemory<T, SaveMemory> operator<<(T t)
      {  return TSaveMemory<T, SaveMemory>(t, *this); }
   template <class TypeIterator>
   TPackedSaveMemory<TypeIterator, SaveMemory> operator<<(MergeBranches::TPacker<TypeIterator> packer)
      {  return TPackedSaveMemory<TypeIterator, SaveMemory>(packer.iter, packer.end, *this); }
   SaveMemory& operator<<(end) { return *this; }
   SaveMemory& setCurrent(bool result) { fResult = result; return *this; }
   bool getResult() const { return fResult; }
   bool operator>>(end)
      {  bool result = fResult;
         if (fResult)
            fResult = false;
         return result;
      }
};

template<typename T1, typename T2>
concept floating_point_promotion = std::floating_point<T1> && std::floating_point<T2> && sizeof(T1) <= sizeof(T2);

template<typename T1, typename T2>
concept floating_point_nopromotion = std::floating_point<T1> && std::floating_point<T2>
      && sizeof(T1) > sizeof(T2) && sizeof(T1) <= sizeof(long double);

template<typename T1, typename T2>
concept integral_signed_promotion = std::integral<T1> && std::integral<T2> && std::is_signed<T1>::value && std::is_signed<T2>::value && sizeof(T1) <= sizeof(T2);

template<typename T1, typename T2>
concept integral_unsigned_promotion = std::integral<T1> && std::integral<T2> && std::is_unsigned<T1>::value && std::is_unsigned<T2>::value && sizeof(T1) <= sizeof(T2);

template<typename T1, typename T2>
concept integral_signed_nopromotion = std::integral<T1> && std::integral<T2> && std::is_signed<T1>::value && std::is_signed<T2>::value && sizeof(T1) > sizeof(T2) && sizeof(T1) <= sizeof(int64_t);

template<typename T1, typename T2>
concept integral_unsigned_nopromotion = std::integral<T1> && std::integral<T2> && std::is_unsigned<T1>::value && std::is_unsigned<T2>::value && sizeof(T1) > sizeof(T2) && sizeof(T1) <= sizeof(uint64_t);

namespace DFloatDigitsHelper {
   template <typename TypeImplementation>
   struct TFloatDigits {
   };

   template <>
   struct TFloatDigits<float> {
     public:
      static const int UBitSizeMantissa=FLT_MANT_DIG-1;
      static const int UBitSizeExponent=sizeof(float)*8-FLT_MANT_DIG;
      static const int UBitFullSizeExponent=UBitSizeExponent;
   };

   template <>
   struct TFloatDigits<double> {
     public:
      static const int UBitSizeMantissa=DBL_MANT_DIG-1;
      static const int UBitSizeExponent=sizeof(double)*8-DBL_MANT_DIG;
      static const int UBitFullSizeExponent=UBitSizeExponent;
   };

   template <>
   struct TFloatDigits<long double> {
     public:
      static const int UBitSizeMantissa=LDBL_MANT_DIG-1;
      static const int UBitSizeExponent
         = (LDBL_MAX_EXP == (1 << (16-2))) ? 15 /* leading 1 bit */
            : sizeof(long double)*8-LDBL_MANT_DIG;
      static const int UBitFullSizeExponent
         = (LDBL_MAX_EXP == (1 << (16-2))) ? 16 /* leading 1 bit */
            : sizeof(long double)*8-LDBL_MANT_DIG;
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
class TFloatExact {
  private:
   typedef TFloatExact<USizeMantissa, USizeExponent, TypeImplementation> thisType;

   typedef void* /* size_t */ AlignType;
   static const int UFloatExactSizeInBytes = NumericalDomains::DDoubleExactInterface
      ::TFloatExactSizeTraits<USizeMantissa, USizeExponent, TypeImplementation>::USize;
   static const int UFloatExactSize = (UFloatExactSizeInBytes + sizeof(AlignType)-1)/sizeof(AlignType);
   AlignType content[UFloatExactSize];

   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend class TFloatExact;
   friend class DDoubleExactInterface::MergeBranches;

   enum RoundMode { RMNearest, RMLowest, RMHighest, RMZero };
   int32_t asInt(RoundMode mode) const;
   uint32_t asUnsigned(RoundMode mode) const;
   int64_t asLongInt(RoundMode mode) const;
   uint64_t asUnsignedLong(RoundMode mode) const;

   int sfinite() const;
   int sisfinite() const;
   int sisnan() const;
   int sisinf() const;

  protected:
   void initFrom(int64_t value);
   void initFrom(uint64_t value);
   void initFrom(TypeImplementation value);
   void initFrom(TypeImplementation value, TypeImplementation error);

  public:
   typedef DDoubleExactInterface::MergeBranches MergeBranches;
   const char* queryDebugValue() const;
   typedef ExecutionPath::Initialization Initialization;
   typedef ExecutionPath::anticipated_termination anticipated_termination;
   typedef DDoubleExactInterface::end end;
   static void flushOut() { ExecutionPath::flushOut(); }

  public:
   class ErrorParameter {};
   struct ValueFromString {};
   TFloatExact();
   TFloatExact(const char* value, ValueFromString);
   template <typename T> TFloatExact(T value) requires floating_point_promotion<T, TypeImplementation>
      :  thisType()
      {  initFrom(value); }
   template <typename T> TFloatExact(T value) requires floating_point_nopromotion<T, TypeImplementation>
      :  thisType()
      {  
#ifndef FLOAT_ATOMIC
         DDoubleExactInterface::TFloatExact<LDBL_MANT_DIG-1, (LDBL_MAX_EXP == (1 << (16-2))) ? 15 /* leading 1 bit */ : sizeof(long double)*8-LDBL_MANT_DIG,
            long double> receiver;
         receiver.initFrom((long double) value);
         thisType::operator=(receiver);
#else
         initFrom((TypeImplementation) value);
#endif
      }
   template <typename T> TFloatExact(T value, T error, ErrorParameter)
         requires floating_point_promotion<T, TypeImplementation>
      :  thisType()
      {  initFrom(value, error); }
   template <typename T> TFloatExact(T value, T error, ErrorParameter)
         requires floating_point_nopromotion<T, TypeImplementation>
      :  thisType()
      {  DDoubleExactInterface::TFloatExact<LDBL_MANT_DIG-1, (LDBL_MAX_EXP == (1 << (16-2))) ? 15 /* leading 1 bit */ : sizeof(long double)*8-LDBL_MANT_DIG,
            long double> receiver;
         receiver.initFrom((long double) value, (long double) error);
         thisType::operator=(receiver);
      }
   template <typename T> TFloatExact(T value) requires integral_signed_promotion<T, int64_t>
      :  thisType()
      {  initFrom((int64_t) value); }
   template <typename T> TFloatExact(T value) requires integral_unsigned_promotion<T, uint64_t>
      :  thisType()
      {  initFrom((uint64_t) value); }
   TFloatExact(const thisType& source);
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   TFloatExact(const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source);
   ~TFloatExact();

   TFloatExact& operator=(const thisType& source);
   template <typename T>
   thisType& operator=(T source) requires floating_point_promotion<T, TypeImplementation>
      {  initFrom(source); return *this; }
   template <typename T> thisType& operator=(T source) requires floating_point_nopromotion<T, TypeImplementation>
      {
#ifndef FLOAT_ATOMIC
         DDoubleExactInterface::TFloatExact<LDBL_MANT_DIG-1, (LDBL_MAX_EXP == (1 << (16-2))) ? 15 /* leading 1 bit */ : sizeof(long double)*8-LDBL_MANT_DIG,
            long double> receiver;
         receiver.initFrom((long double) source);
         return (thisType&) thisType::operator=(receiver);
#else
         initFrom((TypeImplementation) source); return *this;
#endif
      }
   template <typename T> thisType& operator=(T value) requires integral_signed_promotion<T, int64_t>
      {  initFrom((int64_t) value); return *this; }
   template <typename T> thisType& operator=(T value) requires integral_unsigned_promotion<T, uint64_t>
      {  initFrom((uint64_t) value); return *this; }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator=(const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source);

   TypeImplementation asImplementation() const;
   void writeImplementation(std::ostream& out) const;
   void readImplementation(std::istream& in);
   friend std::ostream& operator<<(std::ostream& out, const thisType& source)
      {  return out << source.asImplementation(); }
   friend std::istream& operator>>(std::istream& in, thisType& source)
      {  TypeImplementation val;
         in >> val;
         source.operator=(thisType(val));
         return in;
      }

   thisType operator++() { return (thisType&) thisType::operator+=(thisType(1)); }
   thisType operator++(int) { thisType result = *this; thisType::operator+=(thisType(1)); return result; }
   friend thisType operator+(const thisType& first) { return first; }
   friend thisType operator-(const thisType& first)
      {  thisType result(first); result.oppositeAssign(); return result; }

   void oppositeAssign();
   thisType& operator+=(const thisType& source);
   thisType& operator-=(const thisType& source);
   thisType& operator*=(const thisType& source);
   thisType& operator/=(const thisType& source);
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator+=(const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      {  return operator+=(thisType(source)); }
   template<typename T> thisType& operator+=(T source) requires std::floating_point<T> || std::integral<T>
      {  return operator+=(thisType(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator-=(const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      {  return operator-=(thisType(source)); }
   template<typename T> thisType& operator-=(T source) requires std::floating_point<T> || std::integral<T>
      {  return operator-=(thisType(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator*=(const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      {  return operator*=(thisType(source)); }
   template<typename T> thisType& operator*=(T source) requires std::floating_point<T> || std::integral<T>
      {  return operator*=(thisType(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator/=(const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      {  return operator/=(thisType(source)); }
   template<typename T> thisType& operator/=(T source) requires std::floating_point<T> || std::integral<T>
      {  return operator/=(thisType(source)); }

   friend thisType operator+(const thisType& first, const thisType& second)
      {  thisType result(first); result += second; return result; }
   template<typename T> friend auto operator+(const thisType& first, T second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> result(second);
         result += first;
         return result;
      }
   template<typename T> friend auto operator+(T first, const thisType& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> result(first);
         result += second;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator+(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> result(first);
         result += second;
         return result;
      }

   friend thisType operator-(const thisType& first, const thisType& second)
      {  thisType result(first); result -= second; return result; }
   template<typename T> friend auto operator-(const thisType& first, T second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - T(0))> TypeResult;
         TypeResult result(first);
         result -= TypeResult(second);
         return result;
      }
   template<typename T> friend auto operator-(T first, const thisType& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) - TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) - TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) - TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(T(0) - TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) - TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) - TypeImplementation(0))> result(first);
         result -= second;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator-(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))> result(first);
         result -= second;
         return result;
      }

   friend thisType operator*(const thisType& first, const thisType& second)
      {  thisType result(first); result *= second; return result; }
   template<typename T> friend auto operator*(const thisType& first, T second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * T(0))> result(second);
         result *= first;
         return result;
      }
   template<typename T> friend auto operator*(T first, const thisType& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) * TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) * TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) * TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(T(0) * TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) * TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) * TypeImplementation(0))> result(first);
         result *= second;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator*(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))> result(first);
         result *= second;
         return result;
      }

   friend thisType operator/(const thisType& first, const thisType& second)
      {  thisType result(first); result /= second; return result; }
   template<typename T> friend auto operator/(const thisType& first, T second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / T(0))> TypeResult;
         TypeResult result(first);
         result /= TypeResult(second);
         return result;
      }
   template<typename T> friend auto operator/(T first, const thisType& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) / TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) / TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) / TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(T(0) / TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) / TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) / TypeImplementation(0))> result(first);
         result /= second;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator/(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))> result(first);
         result /= second;
         return result;
      }

   explicit operator TypeImplementation() const;
   template <typename T> explicit operator T() const requires integral_signed_promotion<T, int32_t>
      {  return asInt(RMZero); }
   template <typename T> explicit operator T() const requires integral_unsigned_promotion<T, uint32_t>
      {  return asUnsigned(RMZero); }
   template <typename T> explicit operator T() const requires integral_signed_nopromotion<T, int32_t>
      {  return asLongInt(RMZero); }
   template <typename T> explicit operator T() const requires integral_unsigned_nopromotion<T, uint32_t>
      {  return asUnsignedLong(RMZero); }

   static bool compareLess(const thisType& first, const thisType& second);
   static bool compareLessOrEqual(const thisType& first, const thisType& second);
   static bool compareEqual(const thisType& first, const thisType& second);
   static bool compareDifferent(const thisType& first, const thisType& second);
   static bool compareGreaterOrEqual(const thisType& first, const thisType& second);
   static bool compareGreater(const thisType& first, const thisType& second);
   friend bool operator<(const thisType& first, const thisType& second)
      {  return compareLess(first, second); }
   friend bool operator<(const thisType& first, TypeImplementation second)
      {  return compareLess(first, thisType(second)); }
   template <typename T> friend bool operator<(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeComparison;
         return TypeComparison::compareLess(first, TypeComparison(second));
      }
   template <typename T> friend bool operator<(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeComparison;
         return TypeComparison::compareLess(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator<(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareLess(first, second);
      }

   friend bool operator<=(const thisType& first, const thisType& second)
      {  return compareLessOrEqual(first, second); }
   friend bool operator<=(const thisType& first, TypeImplementation second)
      {  return compareLessOrEqual(first, thisType(second)); }
   template <typename T> friend bool operator<=(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeComparison;
         return TypeComparison::compareLessOrEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator<=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeComparison;
         return TypeComparison::compareLessOrEqual(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator<=(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareLessOrEqual(first, second);
      }

   friend bool operator==(const thisType& first, const thisType& second)
      {  return compareEqual(first, second); }
   friend bool operator==(const thisType& first, TypeImplementation second)
      {  return compareEqual(first, thisType(second)); }
   template <typename T> friend bool operator==(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeComparison;
         return TypeComparison::compareEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator==(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeComparison;
         return TypeComparison::compareEqual(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator==(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareEqual(first, second);
      }

   friend bool operator!=(const thisType& first, const thisType& second)
      {  return compareDifferent(first, second); }
   friend bool operator!=(const thisType& first, TypeImplementation second)
      {  return compareDifferent(first, thisType(second)); }
   template <typename T> friend bool operator!=(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeComparison;
         return TypeComparison::compareDifferent(first, TypeComparison(second));
      }
   template <typename T> friend bool operator!=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeComparison;
         return TypeComparison::compareDifferent(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator!=(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareDifferent(first, second);
      }

   friend bool operator>=(const thisType& first, const thisType& second)
      {  return compareGreaterOrEqual(first, second); }
   friend bool operator>=(const thisType& first, TypeImplementation second)
      {  return compareGreaterOrEqual(first, thisType(second)); }
   template <typename T> friend bool operator>=(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator>=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator>=(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(first, second);
      }

   friend bool operator>(const thisType& first, const thisType& second)
      {  return compareGreater(first, second); }
   friend bool operator>(const thisType& first, TypeImplementation second)
      {  return compareGreater(first, thisType(second)); }
   template <typename T> friend bool operator>(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeComparison;
         return TypeComparison::compareGreater(first, TypeComparison(second));
      }
   template <typename T> friend bool operator>(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeComparison;
         return TypeComparison::compareGreater(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator>(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareGreater(first, second);
      }

   // math operations
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

   void continuousFlow(std::function<void (thisType& val)> funAssign)
      {  bool oldSupportUnstableInLoop = ExecutionPath::doesSupportUnstableInLoop();
         ExecutionPath::setSupportUnstableInLoop();
         const char* sourceFile;
         int sourceLine;
         bool isCompleteFlow = true;
         auto saveMemory = SaveMemory() << *this << end();
         bool oldDoesFollow = ExecutionPath::doesFollowFlow();
         ExecutionPath::clearFollowFlow();
         auto oldSourceInfo = ExecutionPath::querySplitInfo();
         do {
            sourceFile = __FILE__; sourceLine = __LINE__;
            BaseFloatExact::splitBranches(sourceFile, sourceLine);

            funAssign(*this);

            isCompleteFlow = MergeBranches(sourceFile, sourceLine) << *this << end();
            ExecutionPath::setFollowFlow();
         } while(!(saveMemory.setCurrent(isCompleteFlow) >> *this >> end()));
         ExecutionPath::setFollowFlow(oldDoesFollow);
         ExecutionPath::setSupportUnstableInLoop(oldSupportUnstableInLoop);
         ExecutionPath::splitBranches(oldSourceInfo.first, oldSourceInfo.second);
      }
   void continuousFlow(std::function<void (thisType& val, const thisType& arg)> funAssign, const thisType& anarg)
      {  bool oldSupportUnstableInLoop = ExecutionPath::doesSupportUnstableInLoop();
         ExecutionPath::setSupportUnstableInLoop();
         const char* sourceFile;
         int sourceLine;
         bool isCompleteFlow = true;
         thisType& arg = const_cast<thisType&>(anarg);
         auto saveMemory = SaveMemory()
            << *this << arg << end();
         bool oldDoesFollow = ExecutionPath::doesFollowFlow();
         ExecutionPath::clearFollowFlow();
         auto oldSourceInfo = ExecutionPath::querySplitInfo();
         do {
            sourceFile = __FILE__; sourceLine = __LINE__;
            BaseFloatExact::splitBranches(sourceFile, sourceLine);

            funAssign(*this, anarg);

            isCompleteFlow = MergeBranches(sourceFile, sourceLine)
               << *this << arg << end();
            ExecutionPath::setFollowFlow();
         } while(!(saveMemory.setCurrent(isCompleteFlow) >> arg >> *this >> end()));
         ExecutionPath::setFollowFlow(oldDoesFollow);
         ExecutionPath::setSupportUnstableInLoop(oldSupportUnstableInLoop);
         ExecutionPath::splitBranches(oldSourceInfo.first, oldSourceInfo.second);
      }
   void continuousFlow(std::function<void (thisType& val, const thisType& fstarg, const thisType& sndarg)> funAssign,
         const thisType& afstarg, const thisType& asndarg)
      {  bool oldSupportUnstableInLoop = ExecutionPath::doesSupportUnstableInLoop();
         ExecutionPath::setSupportUnstableInLoop();
         const char* sourceFile;
         int sourceLine;
         bool isCompleteFlow = true;
         thisType& fstarg = const_cast<thisType&>(afstarg);
         thisType& sndarg = const_cast<thisType&>(asndarg);
         auto saveMemory = SaveMemory()
            << *this << fstarg << sndarg << end();
         bool oldDoesFollow = ExecutionPath::doesFollowFlow();
         ExecutionPath::clearFollowFlow();
         auto oldSourceInfo = ExecutionPath::querySplitInfo();
         do {
            sourceFile = __FILE__; sourceLine = __LINE__;
            BaseFloatExact::splitBranches(sourceFile, sourceLine);

            funAssign(*this, fstarg, sndarg);

            isCompleteFlow = MergeBranches(sourceFile, sourceLine)
               << *this << fstarg << sndarg << end();
            ExecutionPath::setFollowFlow();
         } while(!(saveMemory.setCurrent(isCompleteFlow) >> sndarg >> fstarg >> *this >> end()));
         ExecutionPath::setFollowFlow(oldDoesFollow);
         ExecutionPath::setSupportUnstableInLoop(oldSupportUnstableInLoop);
         ExecutionPath::splitBranches(oldSourceInfo.first, oldSourceInfo.second);
      }

   void absAssign();
   void minAssign(const thisType& source);
   void maxAssign(const thisType& source);
   void medianAssign(const thisType& fst, const thisType& snd);

   friend thisType abs(const thisType& source)
      {  auto result(source);
         result.continuousFlow([](thisType& val){ val.absAssign(); });
         return result;
      }
   friend thisType abs(thisType&& source)
      {  source.continuousFlow([](thisType& val){ val.absAssign(); });
         return source;
      }
   friend thisType fabs(const thisType& source)
      {  auto result(source);
         result.continuousFlow([](thisType& val){ val.absAssign(); });
         return result;
      }
   friend thisType fabs(thisType&& source)
      {  source.continuousFlow([](thisType& val){ val.absAssign(); });
         return source;
      }

   friend thisType min(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ val.minAssign(source); },
               second);
         return result;
      }
   friend thisType min(thisType&& first, const thisType& second)
      {  first.continuousFlow(
               [](thisType& val, const thisType& source){ val.minAssign(source); },
               second);
         return first;
      }
   friend thisType min(const thisType& first, thisType&& second)
      {  second.continuousFlow(
               [](thisType& val, const thisType& source){ val.minAssign(source); },
               first);
         return second;
      }
   friend thisType min(thisType&& first, thisType&& second)
      {  first.continuousFlow(
               [](thisType& val, const thisType& source){ val.minAssign(source); },
               second);
         return first;
      }
   template<typename T> friend auto min(const thisType& first, T second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> result(second);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ val.minAssign(source); },
               first);
         return result;
      }
   template<typename T> friend auto min(T first, const thisType& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ val.minAssign(source); },
               second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto min(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ val.minAssign(source); },
               second);
         return result;
      }

   friend thisType max(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ val.maxAssign(source); },
               second);
         return result;
      }
   friend thisType max(thisType&& first, const thisType& second)
      {  first.continuousFlow(
               [](thisType& val, const thisType& source){ val.maxAssign(source); },
               second);
         return first;
      }
   friend thisType max(const thisType& first, thisType&& second)
      {  second.continuousFlow(
               [](thisType& val, const thisType& source){ val.maxAssign(source); },
               first);
         return second;
      }
   friend thisType max(thisType&& first, thisType&& second)
      {  first.continuousFlow(
               [](thisType& val, const thisType& source){ val.maxAssign(source); },
               second);
         return first;
      }
   template<typename T> friend auto max(const thisType& first, T second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> result(second);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ val.maxAssign(source); },
               first);
         return result;
      }
   template<typename T> friend auto max(T first, const thisType& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ val.maxAssign(source); },
               second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto max(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ val.maxAssign(source); },
               second);
         return result;
      }

   thisType median(const thisType& fst, const thisType& snd) const
      {  auto result(*this);
         result.continuousFlow(
               [](thisType& val, const thisType& fst, const thisType& snd)
                  { val.medianAssign(fst, snd); },
               fst, snd);
         return result;
      }
   thisType median(TypeImplementation afst, const thisType& snd) const
      {  auto result(*this);
         thisType fst(afst);
         result.continuousFlow(
               [](thisType& val, const thisType& fst, const thisType& snd)
                  { val.medianAssign(fst, snd); },
               fst, snd);
         return result;
      }
   thisType median(const thisType& fst, TypeImplementation asnd) const
      {  auto result(*this);
         thisType snd(asnd);
         result.continuousFlow(
               [](thisType& val, const thisType& fst, const thisType& snd)
                  { val.medianAssign(fst, snd); },
               fst, snd);
         return result;
      }
   thisType median(TypeImplementation afst, TypeImplementation asnd) const
      {  auto result(*this);
         thisType fst(afst), snd(asnd);
         result.continuousFlow(
               [](thisType& val, const thisType& fst, const thisType& snd)
                  { val.medianAssign(fst, snd); },
               fst, snd);
         return result;
      }

   void persist(const char* prefix) const;

   friend thisType sqrt(const thisType& source)
      {  auto result(source); result.sqrtAssign(); return result; }
   friend thisType sin(const thisType& source)
      {  auto result(source); result.sinAssign(); return result; }
   friend thisType cos(const thisType& source)
      {  auto result(source); result.cosAssign(); return result; }
   friend thisType asin(const thisType& source)
      {  auto result(source); result.asinAssign(); return result; }
   friend thisType acos(const thisType& source)
      {  auto result(source); result.acosAssign(); return result; }
   friend thisType tan(const thisType& source)
      {  auto result(source); result.tanAssign(); return result; }
   friend thisType atan(const thisType& source)
      {  auto result(source); result.atanAssign(); return result; }
   friend thisType exp(const thisType& source)
      {  auto result(source); result.expAssign(); return result; }
   friend thisType log(const thisType& source)
      {  auto result(source); result.logAssign(); return result; }
   friend thisType log2(const thisType& source)
      {  thisType result(source);
         result.logAssign();
         result.divAssign(log(thisType(2.0)));
         return result;
      }
   friend thisType exp2(const thisType& source)
      {  thisType result = 2.0;
         result.powAssign(source);
         return result;
      }
   friend thisType log10(const thisType& source)
      {  auto result(source); result.log10Assign(); return result; }

   friend thisType pow(const thisType& first, const thisType& second)
      {  auto result(first); result.powAssign(second); return result; }
   template<typename T> friend auto pow(const thisType& first, T second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto pow(T first, const thisType& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(pow(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(pow(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto pow(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }

   friend thisType powf(const thisType& first, const thisType& second)
      {  auto result(first); result.powAssign(second); return result; }
   template<typename T> friend auto powf(const thisType& first, T second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto powf(T first, const thisType& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(powf(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(powf(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto powf(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }

   friend thisType atan2(const thisType& first, const thisType& second)
      {  auto result(first); result.atan2Assign(second); return result; }
   template<typename T> friend auto atan2(const thisType& first, T second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto atan2(T first, const thisType& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(atan2(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(atan2(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto atan2(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(second));
         return result;
      }

   friend thisType floor(const thisType& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMLowest)); },
               fst);
         return result;
      }
   friend thisType ceil(const thisType& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMHighest)); },
               fst);
         return result;
      }
   friend thisType trunc(const thisType& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMZero)); },
               fst);
         return result;
      }
   friend thisType round(const thisType& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMNearest)); },
               fst);
         return result;
      }
   friend thisType rint(const thisType& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMNearest)); },
               fst);
         return result;
      }
   friend thisType rintf(const thisType& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMNearest)); },
               fst);
         return result;
      }
   friend thisType dis_floor(const thisType& fst) { return thisType(fst.asInt(RMLowest)); }
   friend thisType dis_ceil(const thisType& fst) { return thisType(fst.asInt(RMHighest)); }
   friend thisType dis_trunc(const thisType& fst) { return thisType(fst.asInt(RMZero)); }
   friend thisType dis_round(const thisType& fst) { return thisType(fst.asInt(RMNearest)); } 
   friend thisType dis_rintf(const thisType& fst) { return thisType(fst.asInt(RMNearest /* fegetround */)); } 
   friend thisType dis_rint(const thisType& fst) { return thisType(fst.asInt(RMNearest /* fegetround */)); } 

   friend thisType fmod(const thisType& first, const thisType& second)
      {  thisType multResult;
         multResult.continuousFlow(
               [](thisType& result, const thisType& source, const thisType& value)
               {  auto divResult(source); divResult /= value;
                  result = thisType(divResult.asInt(RMZero));
                  result *= value;
                  result -= source;
                  result.oppositeAssign();
               },
               first, second);
         return multResult;
      }
   template<typename T> friend auto fmod(const thisType& first, T second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.continuousFlow(
               [](TypeResult& source, const TypeResult& value)
               {  auto divResult(source); divResult /= value;
                  TypeResult multResult(divResult.asInt(RMZero));
                  multResult *= value; source -= multResult;
               },
               result, TypeResult(second));
         return result;
      }
   template<typename T> friend auto fmod(T first, const thisType& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(fmod(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(fmod(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.continuousFlow(
               [](TypeResult& source, const TypeResult& value)
               {  auto divResult(source); divResult /= value;
                  TypeResult multResult(divResult.asInt(RMZero));
                  multResult *= value; source -= multResult;
               },
               result, TypeResult(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto fmod(const thisType& first, const TFloatExact<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatExact<
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatExact<DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.continuousFlow(
               [](TypeResult& source, const TypeResult& value)
               {  auto divResult(source); divResult /= value;
                  TypeResult multResult(divResult.asInt(RMZero));
                  multResult *= value; source -= multResult;
               },
               result, TypeResult(second));
         return result;
      }

   friend int fld_finite(const thisType& source) { return source.sfinite(); }
   friend int fld_isfinite(const thisType& source) { return source.sisfinite(); }
   friend int fld_isnan(const thisType& source) { return source.sisnan(); }
   friend int fld_isinf(const thisType& source) { return source.sisinf(); }
};

} // end of namespace DDoubleExactInterface

typedef DDoubleExactInterface::TFloatExact<23, 8, float> FloatExact;
typedef DDoubleExactInterface::TFloatExact<52, 11, double> DoubleExact;
typedef DDoubleExactInterface::TFloatExact<LDBL_MANT_DIG-1,
      (LDBL_MAX_EXP == (1 << (16-2))) ? 15 /* leading 1 bit */
                                      : sizeof(long double)*8-LDBL_MANT_DIG,
      long double> LongDoubleExact;

class ParseFloatExact : public FloatExact {
  public:
   ParseFloatExact(const char* value) : FloatExact(value, ValueFromString()) {}
};

class ParseDoubleExact : public DoubleExact {
  public:
   ParseDoubleExact(const char* value) : DoubleExact(value, ValueFromString()) {}
};

class ParseLongDoubleExact : public LongDoubleExact {
  public:
   ParseLongDoubleExact(const char* value) : LongDoubleExact(value, ValueFromString()) {}
};

} // end of namespace NumericalDomains

