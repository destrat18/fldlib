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
// File      : FloatExact.h
// Description :
//   Definition of a class of floating point comparison
//

#pragma once

#include "fldlib_config.h"
#include "NumericalAnalysis/FloatExactExecutionPath.h"
#include "FloatInstrumentation/FloatExact.inch"

namespace NumericalDomains { namespace DDoubleExact {

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

   TSaveMemory<T1, TypeSaveMemory>& operator<<(BaseExecutionPath::end) { return *this; }
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

template <typename, typename = void>
struct THasResizeContainer : std::false_type {};

template <typename T>
struct THasResizeContainer<T, std::void_t<decltype(&T::resizeContainer)>>
   : std::true_type {};

template<typename TypeIterator, typename TypeArg=int>
struct TContainerResizer {
   static int resizeContainer(TypeIterator& iter, int savedCount, TypeArg* val=nullptr)
   {  AssumeUncalled return 0; }
};

template <typename TypeIterator>
struct TContainerResizer<TypeIterator, typename std::enable_if<THasResizeContainer<TypeIterator>::value, int>::type> {
   static int resizeContainer(TypeIterator& iter, int savedCount, int* val=nullptr)
      {  return iter.resizeContainer(savedCount); }
};

template <typename TypeIterator, class TypeSaveMemory>
class TPackedSaveMemory {
  public:
   COL::TVector<typename TypeIterator::value_type> save;
   TypeSaveMemory next;

   TPackedSaveMemory(TypeIterator iter, TypeIterator end, TypeSaveMemory nextArg)
      :  next(nextArg)
      {  int count = end - iter;
         save.bookPlace(count);
         for (; iter != end; ++iter)
            save.insertAtEnd(*iter);
      }
   TPackedSaveMemory(const TPackedSaveMemory<TypeIterator, TypeSaveMemory>&) = default;
   TPackedSaveMemory(TPackedSaveMemory<TypeIterator, TypeSaveMemory>&&) = default;

   template <typename T>
   TSaveMemory<T, TSaveMemory<TypeIterator, TypeSaveMemory> > operator<<(T t)
      {  return TSaveMemory<T, TSaveMemory<TypeIterator, TypeSaveMemory> >(t, *this); }
   template <class TypeIteratorArgument>
   TPackedSaveMemory<TypeIteratorArgument, TPackedSaveMemory<TypeIterator, TypeSaveMemory>>
      operator<<(MergeBranches::TPacker<TypeIteratorArgument> packer)
      {  return TPackedSaveMemory<TypeIteratorArgument, TPackedSaveMemory<TypeIterator, TypeSaveMemory> >
            (packer.iter, packer.end, *this);
      }
   TPackedSaveMemory<TypeIterator, TypeSaveMemory>& operator<<(BaseExecutionPath::end) { return *this; }

   TPackedSaveMemory<TypeIterator, TypeSaveMemory>& setCurrent(bool result)
      {  next.setCurrent(result); return *this; }
   
   TypeSaveMemory& operator>>(MergeBranches::TPacker<TypeIterator>&& packer)
      {  if (!next.getResult()) {
            int count = packer.end - packer.iter;
            if (count != save.count())
               count = TContainerResizer<TypeIterator>::resizeContainer(packer.iter, save.count());
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
   TPackedSaveMemory<TypeIterator, SaveMemory>
      operator<<(MergeBranches::TPacker<TypeIterator> packer)
      {  return TPackedSaveMemory<TypeIterator, SaveMemory>
            (packer.iter, packer.end, *this);
      }
   SaveMemory& operator<<(BaseExecutionPath::end) { return *this; }
   SaveMemory& setCurrent(bool result) { fResult = result; return *this; }
   bool getResult() const { return fResult; }
   bool operator>>(BaseExecutionPath::end)
      {  bool result = fResult;
         if (fResult)
            fResult = false;
         return result;
      }
};

template <typename TypeImplementation>
int tfinite(TypeImplementation val)
   {  AssumeUncalled return 0; }

template <> int tfinite(long double val);
template <> int tfinite(double val);
template <> int tfinite(float val);

template <typename TypeImplementation>
int tisfinite(TypeImplementation val)
   {  AssumeUncalled return 0; }

template <> int tisfinite(long double val);
template <> int tisfinite(double val);
template <> int tisfinite(float val);

template <typename TypeImplementation>
int tisnan(TypeImplementation val)
   {  AssumeUncalled return 0; }

template <> int tisnan(long double val);
template <> int tisnan(double val);
template <> int tisnan(float val);

template <typename TypeImplementation>
int tisinf(TypeImplementation val)
   {  AssumeUncalled return 0; }

template <> int tisinf(long double val);
template <> int tisinf(double val);
template <> int tisinf(float val);

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

template <class TypeBuiltDouble, typename TypeImplementation> requires std::floating_point<TypeImplementation> 
class TInstrumentedFloat
   : public TFloatExact<ExecutionPath, TypeBuiltDouble, TypeImplementation> {
  private:
   typedef TFloatExact<ExecutionPath, TypeBuiltDouble, TypeImplementation> inherited;
   typedef TInstrumentedFloat<TypeBuiltDouble, TypeImplementation> thisType;

  public:
   class ErrorParameter {};
   struct ValueFromString {}; 
   template <typename T> requires std::floating_point<T> || std::integral<T>
   struct OperatorType {
      typedef decltype(TypeImplementation(0) + T(0)) PlusType;
      typedef decltype(TypeImplementation(0) - T(0)) MinusType;
      typedef decltype(TypeImplementation(0) * T(0)) MultType;
      typedef decltype(TypeImplementation(0) / T(0)) DivType;

      typedef decltype(T(0) + TypeImplementation(0)) CoPlusType;
      typedef decltype(T(0) - TypeImplementation(0)) CoMinusType;
      typedef decltype(T(0) * TypeImplementation(0)) CoMultType;
      typedef decltype(T(0) / TypeImplementation(0)) CoDivType;
   };
   typedef DDoubleExact::MergeBranches MergeBranches;

   TInstrumentedFloat() = default;
   TInstrumentedFloat(const char* value, ValueFromString)
      {  STG::IOObject::ISBase* in = ExecutionPath::acquireConstantStream(value);
         inherited::initFrom(*in);
         ExecutionPath::releaseConstantStream(in);
      }
   TInstrumentedFloat(TypeImplementation value) { inherited::initFrom(value); }
   template <typename TypeSourceImplementation> TInstrumentedFloat(TypeSourceImplementation value)
         requires floating_point_promotion<TypeSourceImplementation, TypeImplementation>
      {  inherited::initFrom(value); }
   template <typename TypeSourceImplementation> TInstrumentedFloat(TypeSourceImplementation value)
         requires floating_point_nopromotion<TypeSourceImplementation, TypeImplementation>
      {
#ifndef FLOAT_ATOMIC
         TFloatExact<ExecutionPath, BuiltLongDouble, long double> receiver;
         receiver.initFrom(value);
         inherited::operator=(receiver);
#else
         inherited::initFrom((TypeImplementation) value);
#endif
      }
   template <typename TypeSourceImplementation> TInstrumentedFloat(TypeSourceImplementation value)
         requires std::integral<TypeSourceImplementation>
      :  inherited(value) {}
   TInstrumentedFloat(TypeImplementation value, TypeImplementation error, ErrorParameter)
      {  inherited::initFrom(value);
         inherited addition;
         addition.initFrom(error);
         addition.clearImplementation();
         inherited::operator+=(addition);
      }
   TInstrumentedFloat(const thisType& source) = default;
   TInstrumentedFloat& operator=(const thisType& source) = default;

   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   TInstrumentedFloat(const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& source)
      :  inherited(source) {}
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   thisType& operator=(const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& source)
      {  return (thisType&) inherited::operator=(source); }
   template<typename T> thisType& operator=(T source) requires std::floating_point<T> or std::integral<T>
      {  return (thisType&) inherited::operator=(thisType(source)); }

   thisType operator++() { return (thisType&) inherited::operator+=(thisType(1)); }
   thisType operator++(int) { thisType result = *this; inherited::operator+=(thisType(1)); return result; }
   friend thisType operator+(const thisType& first) { return first; }
   friend thisType operator-(const thisType& first)
      {  thisType result(first); result.oppositeAssign(); return result; }

   thisType& operator+=(const thisType& source) { return (thisType&) inherited::operator+=(source); }
   thisType& operator-=(const thisType& source) { return (thisType&) inherited::operator-=(source); }
   thisType& operator*=(const thisType& source) { return (thisType&) inherited::operator*=(source); }
   thisType& operator/=(const thisType& source) { return (thisType&) inherited::operator/=(source); }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   thisType& operator+=(const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& source)
      {  return (thisType&) inherited::operator+=(thisType(source)); }
   template<typename T> thisType& operator+=(T source) requires std::floating_point<T> || std::integral<T>
      {  return (thisType&) inherited::operator+=(thisType(source)); }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   thisType& operator-=(const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& source)
      {  return (thisType&) inherited::operator-=(thisType(source)); }
   template<typename T> thisType& operator-=(T source) requires std::floating_point<T> || std::integral<T>
      {  return (thisType&) inherited::operator-=(thisType(source)); }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   thisType& operator*=(const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& source)
      {  return (thisType&) inherited::operator*=(thisType(source)); }
   template<typename T> thisType& operator*=(T source) requires std::floating_point<T> || std::integral<T>
      {  return (thisType&) inherited::operator*=(thisType(source)); }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   thisType& operator/=(const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& source)
      {  return (thisType&) inherited::operator/=(thisType(source)); }
   template<typename T> thisType& operator/=(T source) requires std::floating_point<T> || std::integral<T>
      {  return (thisType&) inherited::operator/=(thisType(source)); }

   friend thisType operator+(const thisType& first, const thisType& second)
      {  thisType result(first); result += second; return result; }
   template<typename T> friend auto operator+(const thisType& first, T second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> result(second);
         result += first;
         return result;
      }
   template<typename T> friend auto operator+(T first, const thisType& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> result(first);
         result += second;
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto operator+(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> result(first);
         result += second;
         return result;
      }

   friend thisType operator-(const thisType& first, const thisType& second)
      {  thisType result(first); result -= second; return result; }
   template<typename T> friend auto operator-(const thisType& first, T second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::MinusType>::BuiltType,
            typename OperatorType<T>::MinusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::MinusType>::BuiltType,
            typename OperatorType<T>::MinusType> result(second);
         result.oppositeAssign();
         result += first;
         return result;
      }
   template<typename T> friend auto operator-(T first, const thisType& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoMinusType>::BuiltType,
            typename OperatorType<T>::CoMinusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoMinusType>::BuiltType,
            typename OperatorType<T>::CoMinusType> result(first);
         result -= second;
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto operator-(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))> result(first);
         result -= second;
         return result;
      }

   friend thisType operator*(const thisType& first, const thisType& second)
      {  thisType result(first); result *= second; return result; }
   template<typename T>
   friend auto operator*(const thisType& first, T second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::MultType>::BuiltType,
            typename OperatorType<T>::MultType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::MultType>::BuiltType,
            typename OperatorType<T>::MultType> result(second);
         result *= first;
         return result;
      }
   template<typename T> friend auto operator*(T first, const thisType& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoMultType>::BuiltType,
            typename OperatorType<T>::CoMultType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoMultType>::BuiltType,
            typename OperatorType<T>::CoMultType> result(first);
         result *= second;
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto operator*(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))> result(first);
         result *= second;
         return result;
      }

   friend thisType operator/(const thisType& first, const thisType& second)
      {  thisType result(first); result /= second; return result; }
   template<typename T> friend auto operator/(const thisType& first, T second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::DivType>::BuiltType,
            typename OperatorType<T>::DivType>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::DivType>::BuiltType,
            typename OperatorType<T>::DivType> TypeResult;
         TypeResult result(first);
         result /= TypeResult(second);
         return result;
      }
   template<typename T> friend auto operator/(T first, const thisType& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoDivType>::BuiltType,
            typename OperatorType<T>::CoDivType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoDivType>::BuiltType,
            typename OperatorType<T>::CoDivType> result(first);
         result /= second;
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto operator/(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))> result(first);
         result /= second;
         return result;
      }

   explicit operator TypeImplementation() const { return inherited::asImplementation(); }
   template <typename T> explicit operator T() const requires integral_signed_promotion<T, int32_t>
      {  return inherited::asInt(inherited::ReadParametersBase::RMZero); }
   template <typename T> explicit operator T() const requires integral_unsigned_promotion<T, uint32_t>
      {  return inherited::asUnsigned(inherited::ReadParametersBase::RMZero); }
   template <typename T> explicit operator T() const requires integral_signed_nopromotion<T, int32_t>
      {  return inherited::asLongInt(inherited::ReadParametersBase::RMZero); }
   template <typename T> explicit operator T() const requires integral_unsigned_nopromotion<T, uint32_t>
      {  return inherited::asUnsignedLong(inherited::ReadParametersBase::RMZero); }

   static bool compareLess(const thisType& first, const thisType& second)
      {  return first.inherited::operator<(second); }
   static bool compareLessOrEqual(const thisType& first, const thisType& second)
      {  return first.inherited::operator<=(second); }
   static bool compareEqual(const thisType& first, const thisType& second)
      {  return first.inherited::operator==(second); }
   static bool compareDifferent(const thisType& first, const thisType& second)
      {  return first.inherited::operator!=(second); }
   static bool compareGreaterOrEqual(const thisType& first, const thisType& second)
      {  return first.inherited::operator>=(second); }
   static bool compareGreater(const thisType& first, const thisType& second)
      {  return first.inherited::operator>(second); }
   friend bool operator<(const thisType& first, const thisType& second)
      {  return first.inherited::operator<(second); }
   friend bool operator<(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator<(thisType(second)); }
   template <typename T> friend bool operator<(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareLess(first, TypeComparison(second));
      }
   template <typename T> friend bool operator<(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareLess(TypeComparison(first), second);
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend bool operator<(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareLess(first, second);
      }

   friend bool operator<=(const thisType& first, const thisType& second)
      {  return first.inherited::operator<=(second); }
   friend bool operator<=(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator<=(thisType(second)); }
   template <typename T> friend bool operator<=(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareLessOrEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator<=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareLessOrEqual(TypeComparison(first), second);
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend bool operator<=(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareLessOrEqual(first, second);
      }

   friend bool operator==(const thisType& first, const thisType& second)
      {  return first.inherited::operator==(second); }
   friend bool operator==(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator==(thisType(second)); }
   template <typename T> friend bool operator==(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator==(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareEqual(TypeComparison(first), second);
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend bool operator==(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareEqual(first, second);
      }

   friend bool operator!=(const thisType& first, const thisType& second)
      {  return first.inherited::operator!=(second); }
   friend bool operator!=(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator!=(thisType(second)); }
   template <typename T> friend bool operator!=(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareDifferent(first, TypeComparison(second));
      }
   template <typename T> friend bool operator!=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareDifferent(TypeComparison(first), second);
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend bool operator!=(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareDifferent(first, second);
      }

   friend bool operator>=(const thisType& first, const thisType& second)
      {  return first.inherited::operator>=(second); }
   friend bool operator>=(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator>=(thisType(second)); }
   template <typename T> friend bool operator>=(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator>=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(TypeComparison(first), second);
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend bool operator>=(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(first, second);
      }

   friend bool operator>(const thisType& first, const thisType& second)
      {  return first.inherited::operator>(second); }
   friend bool operator>(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator>(thisType(second)); }
   template <typename T> friend bool operator>(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareGreater(first, TypeComparison(second));
      }
   template <typename T> friend bool operator>(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareGreater(TypeComparison(first), second);
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend bool operator>(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareGreater(first, second);
      }

   // math operators
   void continuousFlow(std::function<void (thisType& val)> funAssign)
      {  bool oldSupportUnstableInLoop = ExecutionPath::doesSupportUnstableInLoop();
         ExecutionPath::setSupportUnstableInLoop();
         const char* sourceFile;
         int sourceLine;
         bool isCompleteFlow = true;
         auto saveMemory = SaveMemory() << *this << BaseExecutionPath::end();
         bool oldDoesFollow = ExecutionPath::doesFollowFlow();
         ExecutionPath::clearFollowFlow();
         auto oldSourceInfo = BaseFloatExact::querySplitInfo();
         do {
            sourceFile = __FILE__; sourceLine = __LINE__;
            BaseFloatExact::splitBranches(sourceFile, sourceLine);

            funAssign(*this);

            isCompleteFlow = MergeBranches(sourceFile, sourceLine) << *this << BaseExecutionPath::end();
            ExecutionPath::setFollowFlow();
         } while(!(saveMemory.setCurrent(isCompleteFlow) >> *this >> BaseExecutionPath::end()));
         ExecutionPath::setFollowFlow(oldDoesFollow);
         ExecutionPath::setSupportUnstableInLoop(oldSupportUnstableInLoop);
         BaseFloatExact::splitBranches(oldSourceInfo.first, oldSourceInfo.second);
      }
   void continuousFlow(std::function<void (thisType& val, const thisType& arg)> funAssign, const thisType& anarg)
      {  bool oldSupportUnstableInLoop = ExecutionPath::doesSupportUnstableInLoop();
         ExecutionPath::setSupportUnstableInLoop();
         const char* sourceFile;
         int sourceLine;
         bool isCompleteFlow = true;
         thisType& arg = const_cast<thisType&>(anarg);
         auto saveMemory = SaveMemory()
            << *this << arg << BaseExecutionPath::end();
         bool oldDoesFollow = ExecutionPath::doesFollowFlow();
         ExecutionPath::clearFollowFlow();
         auto oldSourceInfo = BaseFloatExact::querySplitInfo();
         do {
            sourceFile = __FILE__; sourceLine = __LINE__;
            BaseFloatExact::splitBranches(sourceFile, sourceLine);

            funAssign(*this, anarg);

            isCompleteFlow = MergeBranches(sourceFile, sourceLine)
               << *this << arg << BaseExecutionPath::end();
            ExecutionPath::setFollowFlow();
         } while(!(saveMemory.setCurrent(isCompleteFlow) >> arg >> *this >> BaseExecutionPath::end()));
         ExecutionPath::setFollowFlow(oldDoesFollow);
         ExecutionPath::setSupportUnstableInLoop(oldSupportUnstableInLoop);
         BaseFloatExact::splitBranches(oldSourceInfo.first, oldSourceInfo.second);
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
            << *this << fstarg << sndarg << BaseExecutionPath::end();
         bool oldDoesFollow = ExecutionPath::doesFollowFlow();
         ExecutionPath::clearFollowFlow();
         auto oldSourceInfo = BaseFloatExact::querySplitInfo();
         do {
            sourceFile = __FILE__; sourceLine = __LINE__;
            BaseFloatExact::splitBranches(sourceFile, sourceLine);

            funAssign(*this, fstarg, sndarg);

            isCompleteFlow = MergeBranches(sourceFile, sourceLine)
               << *this << fstarg << sndarg << BaseExecutionPath::end();
            ExecutionPath::setFollowFlow();
         } while(!(saveMemory.setCurrent(isCompleteFlow) >> sndarg >> fstarg >> *this >> BaseExecutionPath::end()));
         ExecutionPath::setFollowFlow(oldDoesFollow);
         ExecutionPath::setSupportUnstableInLoop(oldSupportUnstableInLoop);
         BaseFloatExact::splitBranches(oldSourceInfo.first, oldSourceInfo.second);
      }

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
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> result(second);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ val.minAssign(source); },
               first);
         return result;
      }
   template<typename T> friend auto min(T first, const thisType& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ val.minAssign(source); },
               second);
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto min(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
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
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> result(second);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ val.maxAssign(source); },
               first);
         return result;
      }
   template<typename T> friend auto max(T first, const thisType& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ val.maxAssign(source); },
               second);
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto max(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
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
   void persist(const char* prefix) const { inherited::notifyForPersistence(*this, prefix); }

   friend std::ostream& operator<<(std::ostream& out, const thisType& source)
      {  return out << source.asImplementation(); }
   friend std::istream& operator>>(std::istream& in, thisType& source)
      {  TypeImplementation val;
         in >> val;
         source = thisType(val);
         return in;
      }

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
      {  auto result(source); result.logAssign(); result.operator/=(log(thisType(2.0))); return result; }
   friend thisType exp2(const thisType& source)
      {  thisType result(2.0); result.powAssign(source); return result; }
   friend thisType log10(const thisType& source)
      {  auto result(source); result.log10Assign(); return result; }

   friend thisType pow(const thisType& first, const thisType& second)
      {  auto result(first); result.powAssign(second); return result; }
   template<typename T> friend auto pow(const thisType& first, T second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(pow(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(pow(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(pow(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(pow(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto pow(T first, const thisType& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(pow(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(pow(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(pow(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(pow(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto pow(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }

   friend thisType powf(const thisType& first, const thisType& second)
      {  auto result(first); result.powAssign(second); return result; }
   template<typename T> friend auto powf(const thisType& first, T second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(powf(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(powf(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(powf(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(powf(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto powf(T first, const thisType& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(powf(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(powf(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(powf(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(powf(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto powf(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }

   friend thisType atan2(const thisType& first, const thisType& second)
      {  auto result(first); result.atan2Assign(second); return result; }
   template<typename T> friend auto atan2(const thisType& first, T second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(atan2(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(atan2(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(atan2(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(atan2(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto atan2(T first, const thisType& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(atan2(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(atan2(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(atan2(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(atan2(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(second));
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto atan2(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(second));
         return result;
      }

   friend thisType floor(const thisType& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMLowest)); },
               fst);
         return result;
      }
   friend thisType ceil(const thisType& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMHighest)); },
               fst);
         return result;
      }
   friend thisType trunc(const thisType& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMZero)); },
               fst);
         return result;
      }
   friend thisType round(const thisType& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMNearest)); },
               fst);
         return result;
      }
   friend thisType rint(const thisType& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMNearest)); },
               fst);
         return result;
      }
   friend thisType rintf(const thisType& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMNearest)); },
               fst);
         return result;
      }
   friend thisType dis_floor(const thisType& fst)
      {  return thisType(fst.asInt(thisType::ReadParametersBase::RMLowest)); }
   friend thisType dis_ceil(const thisType& fst)
      {  return thisType(fst.asInt(thisType::ReadParametersBase::RMHighest)); }
   friend thisType dis_trunc(const thisType& fst)
      {  return thisType(fst.asInt(thisType::ReadParametersBase::RMZero)); }
   friend thisType dis_round(const thisType& fst)
      {  return thisType(fst.asInt(thisType::ReadParametersBase::RMNearest)); }
   friend thisType dis_rint(const thisType& fst)
      {  return thisType(fst.asInt(thisType::ReadParametersBase::RMNearest /* fegetround */)); }
   friend thisType dis_rintf(const thisType& fst)
      {  return thisType(fst.asInt(thisType::ReadParametersBase::RMNearest /* fegetround */)); }

   friend thisType fmod(const thisType& first, const thisType& second)
      {  thisType multResult;
         multResult.continuousFlow(
               [](thisType& result, const thisType& source, const thisType& value)
               {  auto divResult(source); divResult /= value;
                  result = thisType(divResult.asInt(thisType::ReadParametersBase::RMZero));
                  result *= value;
                  result -= source;
                  result.oppositeAssign();
               },
               first, second);
         return multResult;
      }
   template<typename T> friend auto fmod(const thisType& first, T second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(fmod(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(fmod(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(fmod(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(fmod(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.continuousFlow(
               [](TypeResult& source, const TypeResult& value)
               {  auto divResult(source); divResult /= value;
                  TypeResult multResult(divResult.asInt(TypeResult::ReadParametersBase::RMZero));
                  multResult *= value; source -= multResult;
               },
               result, TypeResult(second));
         return result;
      }
   template<typename T> friend auto fmod(T first, const thisType& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(fmod(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(fmod(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(fmod(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(fmod(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.continuousFlow(
               [](TypeResult& source, const TypeResult& value)
               {  auto divResult(source); divResult /= value;
                  TypeResult multResult(divResult.asInt(TypeResult::ReadParametersBase::RMZero));
                  multResult *= value; source -= multResult;
               },
               result, TypeResult(second));
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto fmod(const thisType& first, const TInstrumentedFloat<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloat<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.continuousFlow(
               [](TypeResult& source, const TypeResult& value)
               {  auto divResult(source); divResult /= value;
                  TypeResult multResult(divResult.asInt(TypeResult::ReadParametersBase::RMZero));
                  multResult *= value; source -= multResult;
               },
               result, TypeResult(second));
         return result;
      }

   friend int fld_finite(const thisType& source) { return tfinite(source.asImplementation()); }
   friend int fld_isfinite(const thisType& source) { return tisfinite(source.asImplementation()); }
   friend int fld_isnan(const thisType& source) { return tisnan(source.asImplementation()); }
   friend int fld_isinf(const thisType& source) { return tisinf(source.asImplementation()); }
};

} // end of namespace DDoubleExact

typedef DDoubleExact::TInstrumentedFloat<DDoubleExact::BuiltFloat, float> FloatExact;
typedef DDoubleExact::TInstrumentedFloat<DDoubleExact::BuiltDouble, double> DoubleExact;
typedef DDoubleExact::TInstrumentedFloat<DDoubleExact::BuiltLongDouble, long double> LongDoubleExact;

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

