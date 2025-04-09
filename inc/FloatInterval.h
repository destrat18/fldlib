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
// Unit      : Interval
// File      : FloatInterval.h
// Description :
//   Definition of a class of floating point intervals
//

#pragma once

#include "fldlib_config.h"
#include "NumericalAnalysis/FloatIntervalExecutionPath.h"
#include "FloatInstrumentation/FloatInterval.inch"

namespace NumericalDomains { namespace DDoubleInterval {

class MergeBranches {
  public:
   template <class TypeIterator>
   struct TPacker {
      TypeIterator iter, end;
      TPacker(TypeIterator aiter, TypeIterator aend) : iter(aiter), end(aend) {}
   };

   template <class TypeIterator>
   static TPacker<TypeIterator> packer(TypeIterator iter, TypeIterator end)
      {  return TPacker<TypeIterator>(iter, end); }
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

   TSaveMemory<T1, TypeSaveMemory>& operator<<(BaseExecutionPath::end) { return *this; }
   TSaveMemory<T1, TypeSaveMemory>& setCurrentResult(bool result)
      {  next.setCurrentResult(result); return *this; }
   TypeSaveMemory& operator>>(T1& val)
      {  if (!next.getResult())
            val = save;
         return next;
      }
   // to remove for the emission of compiler warnings
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
   TSaveMemory<T, TPackedSaveMemory<TypeIterator, TypeSaveMemory> > operator<<(T t)
      {  return TSaveMemory<T, TPackedSaveMemory<TypeIterator, TypeSaveMemory> >(t, *this); }
   template <class TypeIteratorArgument>
   TPackedSaveMemory<TypeIteratorArgument, TPackedSaveMemory<TypeIterator, TypeSaveMemory> >
      operator<<(MergeBranches::TPacker<TypeIteratorArgument> packer)
      {  return TPackedSaveMemory<TypeIteratorArgument, TPackedSaveMemory<TypeIterator, TypeSaveMemory> >
            (packer.iter, packer.end, *this);
      }
   TPackedSaveMemory<TypeIterator, TypeSaveMemory>& operator<<(BaseExecutionPath::end) { return *this; }

   TPackedSaveMemory<TypeIterator, TypeSaveMemory>& setCurrentResult(bool result)
      {  next.setCurrentResult(result); return *this; }
   
   TypeSaveMemory& operator>>(MergeBranches::TPacker<TypeIterator>&& packer)
      {  if (!next.getResult()) {
            int count = packer.end - packer.iter;
            AssumeCondition(count == save.count())
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
   SaveMemory& setCurrentResult(bool result) { fResult = result; return *this; }
   bool getResult() const { return fResult; }
   bool operator>>(BaseExecutionPath::end)
      {  bool result = fResult;
         if (fResult)
            fResult = false;
         return result;
      }
};

template <typename TypeIterator, class TypeMergeMemory>
class TPackedMergeMemory;

template <typename T1, class TypeMergeMemory>
class TMergeMemory {
  public:
   T1 merge;
   TypeMergeMemory next;

   TMergeMemory(const T1&, TypeMergeMemory nextArg) : merge(), next(nextArg) {}
   TMergeMemory(const TMergeMemory<T1, TypeMergeMemory>&) = default;
   TMergeMemory(TMergeMemory<T1, TypeMergeMemory>&&) = default;

   template <typename T>
   TMergeMemory<T, TMergeMemory<T1, TypeMergeMemory> > operator>>(T& t)
      {  return TMergeMemory<T, TMergeMemory<T1, TypeMergeMemory> >(t, *this); }
   template <typename TypeIterator>
   TPackedMergeMemory<TypeIterator, TMergeMemory<T1, TypeMergeMemory> > operator>>(MergeBranches::TPacker<TypeIterator> packer);

   TMergeMemory<T1, TypeMergeMemory>& operator>>(BaseExecutionPath::end) { return *this; }
   TypeMergeMemory& operator<<(T1& val)
      {  if (next.isFirst())
            merge = val;
         else
            merge.mergeWith(val);
         val = merge;
         return next;
      }
   // to remove for the emission of compiler warnings
   TypeMergeMemory& operator<<(const T1& aval)
      {  T1& val = const_cast<T1&>(aval);
         if (next.isFirst())
            merge = val;
         else
            merge.mergeWith(val);
         val = merge;
         return next;
      }
   bool isFirst() const { return next.isFirst(); }
};

template <typename TypeIterator, class TypeMergeMemory>
class TPackedMergeMemory {
  public:
   COL::TVector<typename TypeIterator::value_type> merge;
   TypeMergeMemory next;

   TPackedMergeMemory(TypeIterator iter, TypeIterator end, TypeMergeMemory nextArg)
      :  next(nextArg) {}
   TPackedMergeMemory(const TPackedMergeMemory<TypeIterator, TypeMergeMemory>&) = default;
   TPackedMergeMemory(TPackedMergeMemory<TypeIterator, TypeMergeMemory>&&) = default;

   template <typename T>
   TMergeMemory<T, TPackedMergeMemory<TypeIterator, TypeMergeMemory> > operator>>(T& t)
      {  return TMergeMemory<T, TPackedMergeMemory<TypeIterator, TypeMergeMemory> >(t, *this); }
   template <class TypeIteratorArgument>
   TPackedMergeMemory<TypeIteratorArgument, TPackedMergeMemory<TypeIterator, TypeMergeMemory> >
      operator>>(MergeBranches::TPacker<TypeIteratorArgument> packer)
      {  return TPackedMergeMemory<TypeIteratorArgument, TPackedMergeMemory<TypeIterator, TypeMergeMemory> >
            (packer.iter, packer.end, *this);
      }
   TPackedMergeMemory<TypeIterator, TypeMergeMemory>& operator>>(BaseExecutionPath::end) { return *this; }

   TypeMergeMemory& operator<<(MergeBranches::TPacker<TypeIterator>&& packer)
      {  int count = packer.end - packer.iter;
         auto iter = packer.iter;
         if (next.isFirst()) {
            AssumeCondition(merge.count() == 0)
            merge.bookPlace(count);
            for (; iter != packer.end; ++iter)
               merge.insertAtEnd(*iter);
         }
         else {
            for (int index = 0; index < count; ++index) {
               merge.referenceAt(index).mergeWith(*iter);
               ++iter;
            }
         }
         for (int index = 0; index < count; ++index) {
            *packer.iter = merge[index];
            ++packer.iter;
         }
         return next;
      }
   bool isFirst() const { return next.isFirst(); }
};

template <typename T1, class TypeMergeMemory>
template <typename TypeIterator>
inline
TPackedMergeMemory<TypeIterator, TMergeMemory<T1, TypeMergeMemory> >
TMergeMemory<T1, TypeMergeMemory>::operator>>(MergeBranches::TPacker<TypeIterator> packer)
   {  return TPackedMergeMemory<TypeIterator, TMergeMemory<T1, TypeMergeMemory> >
         (packer.iter, packer.end, *this);
   }

class MergeMemory {
  private:
   bool fFirst;

  public:
   MergeMemory() : fFirst(true) {}

   template <typename T>
   TMergeMemory<T, MergeMemory> operator>>(T& t)
      {  return TMergeMemory<T, MergeMemory>(t, *this); }
   template <typename TypeIterator>
   TPackedMergeMemory<TypeIterator, MergeMemory> operator>>(MergeBranches::TPacker<TypeIterator>&& packer)
      {  return TPackedMergeMemory<TypeIterator, MergeMemory>(packer.iter, packer.end, *this); }
   MergeMemory& operator>>(BaseExecutionPath::end) { return *this; }
   bool isFirst() const { return fFirst; }
   bool operator<<(BaseExecutionPath::end)
      {  fFirst = false;
         return true;
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

template <class TypeBuiltDouble, typename TypeImplementation>
class TInstrumentedFloatInterval : public TFloatInterval<BaseFloatInterval, TypeBuiltDouble, TypeImplementation> {
  private:
   typedef TInstrumentedFloatInterval<TypeBuiltDouble, TypeImplementation> thisType;
   typedef TFloatInterval<BaseFloatInterval, TypeBuiltDouble, TypeImplementation> inherited;

  public:
   typedef DDoubleInterval::MergeBranches MergeBranches;
   struct ValueFromString {};

  public:
   TInstrumentedFloatInterval() = default;
   TInstrumentedFloatInterval(const char* value, ValueFromString)
      {  STG::IOObject::ISBase* in = BaseFloatInterval::acquireConstantStream(value);
         inherited::initFrom(*in);
         BaseFloatInterval::releaseConstantStream(in);
      }
   template <typename TypeValue> TInstrumentedFloatInterval(TypeValue value)
         requires floating_point_promotion<TypeValue, TypeImplementation>
      {  if (!inherited::fSupportAtomic && inherited::oTraceFile)
            inherited::initFrom((TypeImplementation) value);
         else
            inherited::initFromAtomic((TypeImplementation) value);
      }
   template <typename TypeValue> TInstrumentedFloatInterval(TypeValue value)
         requires floating_point_nopromotion<TypeValue, TypeImplementation>
      {  TFloatInterval<BaseFloatInterval, DDoubleInterval::BuiltLongDouble, long double> receiver;
         if (!inherited::fSupportAtomic && inherited::oTraceFile) {
            receiver.initFrom(value);
            inherited::operator=(receiver);
         }
         else {
            inherited::initFromAtomic((TypeImplementation) value);
            // receiver.initFromAtomic(value);
            // inherited::operator=(receiver);
         }
      }
   template <typename TypeValue> TInstrumentedFloatInterval(TypeValue value) requires std::integral<TypeValue>
      :  inherited(value) {}
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   TInstrumentedFloatInterval(const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& source)
      :  inherited(source) {}

   TInstrumentedFloatInterval(TypeImplementation min, TypeImplementation max) : inherited(min, max) {}
   TInstrumentedFloatInterval(TypeImplementation min, TypeImplementation max,
         TypeImplementation errmin, TypeImplementation errmax)
      :  inherited(errmin < 0 ? min+errmin : min,
                   errmax > 0 ? max+errmax : max) {}
   TInstrumentedFloatInterval(const thisType& source) = default;
   TInstrumentedFloatInterval(thisType&& source) = default; // [TODO] keep symbolic for constraints
   TInstrumentedFloatInterval& operator=(const thisType& source) = default;
   TInstrumentedFloatInterval& operator=(thisType&& source) = default; // [TODO] keep symbolic for constraints
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   thisType& operator=(const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& source)
      {  return (thisType&) inherited::operator=(source); }
   template<typename T> thisType& operator=(T source) requires std::floating_point<T> or std::integral<T>
      {  return (thisType&) inherited::operator=(thisType(source)); }

   const TypeBuiltDouble& min() const { return inherited::min(); }
   const TypeBuiltDouble& max() const { return inherited::max(); }

   thisType operator++() { return (thisType&) inherited::operator+=(thisType(1)); }
   thisType operator++(int) { thisType result = *this; inherited::operator+=(thisType(1)); return result; }
   friend thisType operator+(const thisType& first) { return first; }
   friend thisType operator-(const thisType& first)
      {  thisType result(first); result.oppositeAssign(); return result; }

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
   thisType& operator+=(const thisType& source) { return (thisType&) inherited::operator+=(source); }
   thisType& operator-=(const thisType& source) { return (thisType&) inherited::operator-=(source); }
   thisType& operator*=(const thisType& source) { return (thisType&) inherited::operator*=(source); }
   thisType& operator/=(const thisType& source) { return (thisType&) inherited::operator/=(source); }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   thisType& operator+=(const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& source)
      {  return (thisType&) inherited::operator+=(thisType(source)); }
   template<typename T> thisType& operator+=(T source) requires std::floating_point<T> || std::integral<T>
      {  return (thisType&) inherited::operator+=(thisType(source)); }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   thisType& operator-=(const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& source)
      {  return (thisType&) inherited::operator-=(thisType(source)); }
   template<typename T> thisType& operator-=(T source) requires std::floating_point<T> || std::integral<T>
      {  return (thisType&) inherited::operator-=(thisType(source)); }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   thisType& operator*=(const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& source)
      {  return (thisType&) inherited::operator*=(thisType(source)); }
   template<typename T> thisType& operator*=(T source) requires std::floating_point<T> || std::integral<T>
      {  return (thisType&) inherited::operator*=(thisType(source)); }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   thisType& operator/=(const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& source)
      {  return (thisType&) inherited::operator/=(thisType(source)); }
   template<typename T> thisType& operator/=(T source) requires std::floating_point<T> || std::integral<T>
      {  return (thisType&) inherited::operator/=(thisType(source)); }

   friend thisType operator+(const thisType& first, const thisType& second)
      {  thisType result(first); result += second; return result; }
   template<typename T> friend auto operator+(const thisType& first, T second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> result(second);
         result += first;
         return result;
      }
   template<typename T> friend auto operator+(T first, const thisType& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> result(first);
         result += second;
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto operator+(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> result(first);
         result += second;
         return result;
      }

   friend thisType operator-(const thisType& first, const thisType& second)
      {  thisType result(first); result -= second; return result; }
   template<typename T> friend auto operator-(const thisType& first, T second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::MinusType>::BuiltType,
            typename OperatorType<T>::MinusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::MinusType>::BuiltType,
            typename OperatorType<T>::MinusType> result(second);
         result.oppositeAssign();
         result += first;
         return result;
      }
   template<typename T> friend auto operator-(T first, const thisType& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoMinusType>::BuiltType,
            typename OperatorType<T>::CoMinusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoMinusType>::BuiltType,
            typename OperatorType<T>::CoMinusType> result(first);
         result -= second;
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto operator-(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))> result(first);
         result -= second;
         return result;
      }

   friend thisType operator*(const thisType& first, const thisType& second)
      {  thisType result(first); result *= second; return result; }
   template<typename T> friend auto operator*(const thisType& first, T second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::MultType>::BuiltType,
            typename OperatorType<T>::MultType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::MultType>::BuiltType,
            typename OperatorType<T>::MultType> result(second);
         result *= first;
         return result;
      }
   template<typename T> friend auto operator*(T first, const thisType& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoMultType>::BuiltType,
            typename OperatorType<T>::CoMultType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoMultType>::BuiltType,
            typename OperatorType<T>::CoMultType> result(first);
         result *= second;
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto operator*(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))> result(first);
         result *= second;
         return result;
      }

   friend thisType operator/(const thisType& first, const thisType& second)
      {  thisType result(first); result /= second; return result; }
   template<typename T> friend auto operator/(const thisType& first, T second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::DivType>::BuiltType,
            typename OperatorType<T>::DivType>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::DivType>::BuiltType,
            typename OperatorType<T>::DivType> TypeResult;
         TypeResult result(first);
         result /= TypeResult(second);
         return result;
      }
   template<typename T> friend auto operator/(T first, const thisType& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoDivType>::BuiltType,
            typename OperatorType<T>::CoDivType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoDivType>::BuiltType,
            typename OperatorType<T>::CoDivType> result(first);
         result /= second;
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto operator/(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))> result(first);
         result /= second;
         return result;
      }

#ifdef FLOAT_CONCRETE
   explicit operator TypeImplementation() const { return inherited::asImplementation(); }
#endif
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
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareLess(first, TypeComparison(second));
      }
   template <typename T> friend bool operator<(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareLess(TypeComparison(first), second);
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend bool operator<(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareLess(first, second);
      }

   friend bool operator<=(const thisType& first, const thisType& second)
      {  return first.inherited::operator<=(second); }
   friend bool operator<=(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator<=(thisType(second)); }
   template <typename T> friend bool operator<=(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareLessOrEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator<=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareLessOrEqual(TypeComparison(first), second);
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend bool operator<=(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareLessOrEqual(first, second);
      }

   friend bool operator==(const thisType& first, const thisType& second)
      {  return first.inherited::operator==(second); }
   friend bool operator==(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator==(thisType(second)); }
   template <typename T> friend bool operator==(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator==(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareEqual(TypeComparison(first), second);
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend bool operator==(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareEqual(first, second);
      }

   friend bool operator!=(const thisType& first, const thisType& second)
      {  return first.inherited::operator!=(second); }
   friend bool operator!=(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator!=(thisType(second)); }
   template <typename T> friend bool operator!=(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareDifferent(first, TypeComparison(second));
      }
   template <typename T> friend bool operator!=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareDifferent(TypeComparison(first), second);
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend bool operator!=(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareDifferent(first, second);
      }

   friend bool operator>=(const thisType& first, const thisType& second)
      {  return first.inherited::operator>=(second); }
   friend bool operator>=(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator>=(thisType(second)); }
   template <typename T> friend bool operator>=(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator>=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(TypeComparison(first), second);
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend bool operator>=(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(first, second);
      }

   friend bool operator>(const thisType& first, const thisType& second)
      {  return first.inherited::operator>(second); }
   friend bool operator>(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator>(thisType(second)); }
   template <typename T> friend bool operator>(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareGreater(first, TypeComparison(second));
      }
   template <typename T> friend bool operator>(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareGreater(TypeComparison(first), second);
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend bool operator>(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareGreater(first, second);
      }

   // math operators
   void continuousFlow(std::function<void (thisType& val)> funAssign)
      {  bool oldSupportUnstableInLoop = ExecutionPath::doesSupportUnstableInLoop();
         ExecutionPath::setSupportUnstableInLoop();
         auto* oldPathExplorer = ExecutionPath::getCurrentPathExplorer();
         bool oldDoesFollow = ExecutionPath::doesFollowFlow();
         ExecutionPath::clearFollowFlow();
         auto* oldInputTraceFile = ExecutionPath::inputTraceFile();
         PathExplorer pathExplorer;
         ExecutionPath::setCurrentPathExplorer(&pathExplorer);
         auto mergeMemory = MergeMemory() >> *this >> BaseExecutionPath::end();
         auto saveMemory = SaveMemory() << *this << BaseExecutionPath::end();
         bool doesIterate;
         do {
            funAssign(*this);
            ExecutionPath::setFollowFlow();
            doesIterate = mergeMemory << *this << BaseExecutionPath::end();
            if (doesIterate)
               doesIterate = !(saveMemory.setCurrentResult(pathExplorer.isFinished()) >> *this >> BaseExecutionPath::end());
         } while (doesIterate);
         ExecutionPath::setFollowFlow(oldDoesFollow, oldInputTraceFile);
         ExecutionPath::setCurrentPathExplorer(oldPathExplorer);
         ExecutionPath::setSupportUnstableInLoop(oldSupportUnstableInLoop);
      }
   void continuousFlow(std::function<void (thisType& val, const thisType& arg)> funAssign, const thisType& anarg)
      {  bool oldSupportUnstableInLoop = ExecutionPath::doesSupportUnstableInLoop();
         ExecutionPath::setSupportUnstableInLoop();
         auto* oldPathExplorer = ExecutionPath::getCurrentPathExplorer();
         bool oldDoesFollow = ExecutionPath::doesFollowFlow();
         ExecutionPath::clearFollowFlow();
         auto* oldInputTraceFile = ExecutionPath::inputTraceFile();
         PathExplorer pathExplorer;
         ExecutionPath::setCurrentPathExplorer(&pathExplorer);
         thisType& arg = const_cast<thisType&>(anarg);
         auto mergeMemory = MergeMemory() >> arg >> *this >> BaseExecutionPath::end();
         auto saveMemory = SaveMemory() << *this << arg << BaseExecutionPath::end();
         bool doesIterate;
         do {
            funAssign(*this, anarg);
            ExecutionPath::setFollowFlow();
            doesIterate = mergeMemory << *this << arg << BaseExecutionPath::end();
            if (doesIterate)
               doesIterate = !(saveMemory.setCurrentResult(pathExplorer.isFinished()) >> arg >> *this >> BaseExecutionPath::end());
         } while (doesIterate);
         ExecutionPath::setFollowFlow(oldDoesFollow, oldInputTraceFile);
         ExecutionPath::setCurrentPathExplorer(oldPathExplorer);
         ExecutionPath::setSupportUnstableInLoop(oldSupportUnstableInLoop);
      }
   void continuousFlow(std::function<void (thisType& val, const thisType& fstarg, const thisType& sndarg)> funAssign,
         const thisType& afstarg, const thisType& asndarg)
      {  bool oldSupportUnstableInLoop = ExecutionPath::doesSupportUnstableInLoop();
         ExecutionPath::setSupportUnstableInLoop();
         auto* oldPathExplorer = ExecutionPath::getCurrentPathExplorer();
         bool oldDoesFollow = ExecutionPath::doesFollowFlow();
         ExecutionPath::clearFollowFlow();
         auto* oldInputTraceFile = ExecutionPath::inputTraceFile();
         PathExplorer pathExplorer;
         ExecutionPath::setCurrentPathExplorer(&pathExplorer);
         thisType& fstarg = const_cast<thisType&>(afstarg);
         thisType& sndarg = const_cast<thisType&>(asndarg);
         auto mergeMemory = MergeMemory() >> sndarg >> fstarg >> *this >> BaseExecutionPath::end();
         auto saveMemory = SaveMemory() << *this << fstarg << sndarg << BaseExecutionPath::end();
         bool doesIterate;
         do {
            funAssign(*this, afstarg, asndarg);
            ExecutionPath::setFollowFlow();
            doesIterate = mergeMemory << *this << fstarg << sndarg << BaseExecutionPath::end();
            if (doesIterate)
               doesIterate = !(saveMemory.setCurrentResult(pathExplorer.isFinished()) >> sndarg >> fstarg >> *this >> BaseExecutionPath::end());
         } while (doesIterate);
         ExecutionPath::setFollowFlow(oldDoesFollow, oldInputTraceFile);
         ExecutionPath::setCurrentPathExplorer(oldPathExplorer);
         ExecutionPath::setSupportUnstableInLoop(oldSupportUnstableInLoop);
      }
   friend thisType abs(const thisType& source)
      {  auto result(source);
         result.continuousFlow([](thisType& val){ if (val < 0) val.oppositeAssign(); });
         return result;
      }
   friend thisType abs(thisType&& source)
      {  source.continuousFlow([](thisType& val){ if (val < 0) val.oppositeAssign(); });
         return source;
      }
   friend thisType fabs(const thisType& source)
      {  auto result(source);
         result.continuousFlow([](thisType& val){ if (val < 0) val.oppositeAssign(); });
         return result;
      }
   friend thisType fabs(thisType&& source)
      {  source.continuousFlow([](thisType& val){ if (val < 0) val.oppositeAssign(); });
         return source;
      }

   friend thisType min(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ if (val > source) val = source; },
               second);
         return result;
      }
   friend thisType min(thisType&& first, const thisType& second)
      {  first.continuousFlow(
               [](thisType& val, const thisType& source){ if (val > source) val = source; },
               second);
         return first;
      }
   friend thisType min(const thisType& first, thisType&& second)
      {  second.continuousFlow(
               [](thisType& val, const thisType& source){ if (val > source) val = source; },
               first);
         return second;
      }
   friend thisType min(thisType&& first, thisType&& second)
      {  first.continuousFlow(
               [](thisType& val, const thisType& source){ if (val > source) val = source; },
               second);
         return first;
      }
   template<typename T> friend auto min(const thisType& first, T second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> result(second);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ if (val > source) val = source; },
               first);
         return result;
      }
   template<typename T> friend auto min(T first, const thisType& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ if (val > source) val = source; },
               second);
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto min(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ if (val > source) val = source; },
               second);
         return result;
      }

   friend thisType max(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ if (val < source) val = source; },
               second);
         return result;
      }
   friend thisType max(thisType&& first, const thisType& second)
      {  first.continuousFlow(
               [](thisType& val, const thisType& source){ if (val < source) val = source; },
               second);
         return first;
      }
   friend thisType max(const thisType& first, thisType&& second)
      {  second.continuousFlow(
               [](thisType& val, const thisType& source){ if (val < source) val = source; },
               first);
         return second;
      }
   friend thisType max(thisType&& first, thisType&& second)
      {  first.continuousFlow(
               [](thisType& val, const thisType& source){ if (val < source) val = source; },
               second);
         return first;
      }
   template<typename T> friend auto max(const thisType& first, T second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::PlusType>::BuiltType,
            typename OperatorType<T>::PlusType> result(second);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ if (val < source) val = source; },
               first);
         return result;
      }
   template<typename T> friend auto max(T first, const thisType& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            typename OperatorType<T>::CoPlusType>::BuiltType,
            typename OperatorType<T>::CoPlusType> result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ if (val < source) val = source; },
               second);
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto max(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::BuiltType,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ if (val < source) val = source; },
               second);
         return result;
      }

   thisType median(const thisType& fst, const thisType& snd) const
      {  auto result(*this);
         result.continuousFlow(
               [](thisType& val, const thisType& fst, const thisType& snd)
                  {  if (val <= fst) {
                        if (fst <= snd)
                           val = fst;
                        // snd < fst 
                        else if (val <= snd)
                           val = snd;
                        // snd < val <= fst
                     }
                     // fst < val
                     else if (snd <= fst)
                        val = fst;
                     // fst < snd
                     else if (val > snd)
                        val = snd;
                  }, fst, snd);
         return result;
      }
   thisType median(TypeImplementation afst, const thisType& snd) const
      {  auto result(*this);
         thisType fst(afst);
         result.continuousFlow(
               [](thisType& val, const thisType& fst, const thisType& snd)
                  {  if (val <= fst) {
                        if (fst <= snd)
                           val = fst;
                        // snd < fst 
                        else if (val <= snd)
                           val = snd;
                        // snd < val <= fst
                     }
                     // fst < val
                     else if (snd <= fst)
                        val = fst;
                     // fst < snd
                     else if (val > snd)
                        val = snd;
                  }, fst, snd);
         return result;
      }
   thisType median(const thisType& fst, TypeImplementation asnd) const
      {  auto result(*this);
         thisType snd(asnd);
         result.continuousFlow(
               [](thisType& val, const thisType& fst, const thisType& snd)
                  {  if (val <= fst) {
                        if (fst <= snd)
                           val = fst;
                        // snd < fst 
                        else if (val <= snd)
                           val = snd;
                        // snd < val <= fst
                     }
                     // fst < val
                     else if (snd <= fst)
                        val = fst;
                     // fst < snd
                     else if (val > snd)
                        val = snd;
                  }, fst, snd);
         return result;
      }
   thisType median(TypeImplementation afst, TypeImplementation asnd) const
      {  auto result(*this);
         thisType fst(afst), snd(asnd);
         result.continuousFlow(
               [](thisType& val, const thisType& fst, const thisType& snd)
                  {  if (val <= fst) {
                        if (fst <= snd)
                           val = fst;
                        // snd < fst 
                        else if (val <= snd)
                           val = snd;
                        // snd < val <= fst
                     }
                     // fst < val
                     else if (snd <= fst)
                        val = fst;
                     // fst < snd
                     else if (val > snd)
                        val = snd;
                  }, fst, snd);
         return result;
      }

   void persist(const char* prefix) { inherited::notifyForPersistence(*this, prefix); }

   friend std::ostream& operator<<(std::ostream& out, const thisType& source)
      {  return out << source.asImplementation(); }
   friend std::istream& operator>>(std::istream& in, thisType& source)
      {  TypeImplementation val;
         in >> val;
         source = thisType(val);
         return in;
      }

   friend thisType sqrt(const thisType& source)
      {  auto result(std::move(source)); result.sqrtAssign(); return result; }
   friend thisType sin(const thisType& source)
      {  auto result(std::move(source)); result.sinAssign(); return result; }
   friend thisType cos(const thisType& source)
      {  auto result(std::move(source)); result.cosAssign(); return result; }
   friend thisType asin(const thisType& source)
      {  auto result(std::move(source)); result.asinAssign(); return result; }
   friend thisType acos(const thisType& source)
      {  auto result(std::move(source)); result.acosAssign(); return result; }
   friend thisType tan(const thisType& source)
      {  auto result(std::move(source)); result.tanAssign(); return result; }
   friend thisType atan(const thisType& source)
      {  auto result(std::move(source)); result.atanAssign(); return result; }
   friend thisType exp(const thisType& source)
      {  auto result(std::move(source)); result.expAssign(); return result; }
   friend thisType log(const thisType& source)
      {  auto result(std::move(source)); result.logAssign(); return result; }
   friend thisType log2(const thisType& source)
      {  auto result(std::move(source)); result.logAssign();
         result.operator/=(log(thisType(2.0)));
         return result;
      }
   friend thisType exp2(const thisType& source)
      {  thisType result(2.0);
         result.powAssign(source);
         return result;
      }
   friend thisType log10(const thisType& source)
      {  auto result(std::move(source)); result.log10Assign(); return result; }

   friend thisType pow(const thisType& first, const thisType& second)
      {  auto result(first); result.powAssign(second); return result; }
   template<typename T> friend auto pow(const thisType& first, T second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(pow(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(pow(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(pow(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(pow(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto pow(T first, const thisType& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(pow(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(pow(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(pow(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(pow(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto pow(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }

   friend thisType powf(const thisType& first, const thisType& second)
      {  auto result(first); result.powAssign(second); return result; }
   template<typename T> friend auto powf(const thisType& first, T second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(powf(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(powf(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(powf(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(powf(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto powf(T first, const thisType& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(powf(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(powf(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(powf(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(powf(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto powf(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }

   friend thisType atan2(const thisType& first, const thisType& second)
      {  auto result(first); result.atan2Assign(second); return result; }
   template<typename T> friend auto atan2(const thisType& first, T second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(atan2(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(atan2(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(atan2(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(atan2(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto atan2(T first, const thisType& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(atan2(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(atan2(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(atan2(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(atan2(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(second));
         return result;
      }
   template <class TypeBuiltArgument, typename TypeImplementationArgument>
   friend auto atan2(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
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
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(fmod(TypeImplementation(0), T(0)))>::BuiltType,
            decltype(fmod(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
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
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
            decltype(fmod(T(0), TypeImplementation(0)))>::BuiltType,
            decltype(fmod(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble,
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
   friend auto fmod(const thisType& first, const TInstrumentedFloatInterval<TypeBuiltArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatInterval<typename TFloatDigitsConnectionHelper<TypeBuiltDouble, decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::BuiltType,
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

} // end of namespace DDoubleInterval

typedef DDoubleInterval::TInstrumentedFloatInterval<DDoubleInterval::BuiltFloat, float> FloatInterval;
typedef DDoubleInterval::TInstrumentedFloatInterval<DDoubleInterval::BuiltDouble, double> DoubleInterval;
typedef DDoubleInterval::TInstrumentedFloatInterval<DDoubleInterval::BuiltLongDouble, long double> LongDoubleInterval;

class ParseFloatInterval : public FloatInterval {
  public:
   ParseFloatInterval(const char* value) : FloatInterval(value, ValueFromString()) {}
};

class ParseDoubleInterval : public DoubleInterval {
  public:
   ParseDoubleInterval(const char* value) : DoubleInterval(value, ValueFromString()) {}
};

class ParseLongDoubleInterval : public LongDoubleInterval {
  public:
   ParseLongDoubleInterval(const char* value) : LongDoubleInterval(value, ValueFromString()) {}
};

} // end of namespace NumericalDomains

