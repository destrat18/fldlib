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
// File      : FloatAffine.h
// Description :
//   Definition of a class of affine relations.
//

#pragma once

#include "fldlib_config.h"
#include "NumericalAnalysis/FloatAffineExecutionPath.h"

#ifndef FLOAT_ZONOTOPE_DOES_ABSORB_HIGH_LEVEL
#define FLOAT_ZONOTOPE_DOES_ABSORB_HIGH_LEVEL false
#endif

#ifndef FLOAT_ZONOTOPE_ALLOW_SIMPLEX
#define FLOAT_ZONOTOPE_ALLOW_SIMPLEX false
#endif

#ifndef FLOAT_ZONOTOPE_LIMIT_SYMBOL_ABSORPTION
#define FLOAT_ZONOTOPE_LIMIT_SYMBOL_ABSORPTION 48
#endif

#ifndef FLOAT_ZONOTOPE_DOES_EXCLUDE_CONSTANT_FROM_SYMBOL_ABSORPTION
#define FLOAT_ZONOTOPE_DOES_EXCLUDE_CONSTANT_FROM_SYMBOL_ABSORPTION false
#endif

#ifndef FLOAT_ZONOTOPE_START_SYMBOL_ABSORPTION
#define FLOAT_ZONOTOPE_START_SYMBOL_ABSORPTION 0
#endif

#include <cmath>
#include <fstream>
#include <sstream>
#include <tuple>
#include <type_traits>

#include "FloatInstrumentation/FloatAffine.inch"

namespace NumericalDomains { namespace DAffine {

typedef STG::IOObject::OSBase DiagnosisReadStream;

template <typename TypeIterator, class TypeSaveMemory>
class TPackedSaveMemory;

template<typename> struct thasEquationHolder { typedef int type; };

struct EveryType {};
struct AffineType : public EveryType {};

template <typename T>
void
tremoveHolder(T&, EveryType) {}

template <typename T>
void
tsetHolder(T&, T&, EveryType) {}

template <typename T, typename thasEquationHolder<typename T::InstrumentedAffineType>::type=0>
void
tremoveHolder(T& save, AffineType) {
   save.cloneShareParts();
   if (save.doesSupportUnstableInLoop()) {
      save.getSRealDomain().clearHolder();
      save.getSError().clearHolder();
   };
}

template <typename T, typename thasEquationHolder<typename T::InstrumentedAffineType>::type=0>
void
tsetHolder(T& source, T& save, AffineType) {
   save.cloneShareParts();
   if (source.doesSupportUnstableInLoop()) {
      source.getSRealDomain().setHolder(source.currentPathExplorer);
      source.getSError().setHolder(source.currentPathExplorer);
      save.getSRealDomain().clearHolder();
      save.getSError().clearHolder();
   };
}

template <typename T1, class TypeSaveMemory>
class TSaveMemory {
  public:
   T1 save;
   TypeSaveMemory next;

   TSaveMemory(T1& saveArg, TypeSaveMemory nextArg)
      :  save(saveArg), next(nextArg)
      {  tsetHolder(saveArg, save, AffineType()); }
   TSaveMemory(const TSaveMemory<T1, TypeSaveMemory>& source)
      :  save(source.save), next(source.next)
      {  tremoveHolder(save, AffineType()); }
   TSaveMemory(TSaveMemory<T1, TypeSaveMemory>&& source)
      :  save(std::move(source.save)), next(std::move(source.next))
      {  tremoveHolder(save, AffineType()); }

   template <typename T>
   TSaveMemory<T, TSaveMemory<T1, TypeSaveMemory> > operator<<(T& t)
      {  return TSaveMemory<T, TSaveMemory<T1, TypeSaveMemory> >(t, *this); }
   template <typename T>
   TSaveMemory<T, TSaveMemory<T1, TypeSaveMemory> > operator<<(const T& t)
      {  return TSaveMemory<T, TSaveMemory<T1, TypeSaveMemory> >(const_cast<T&>(t), *this); }
   template <typename TypeIterator>
   TPackedSaveMemory<TypeIterator, TSaveMemory<T1, TypeSaveMemory> > operator<<(MergeBranches::TPacker<TypeIterator> packer);

   TSaveMemory<T1, TypeSaveMemory>& operator<<(BaseExecutionPath::end) { return *this; }
   TSaveMemory<T1, TypeSaveMemory>& operator<<(BaseExecutionPath::nothing) { return *this; }
   TSaveMemory<T1, TypeSaveMemory>& setCurrentResult(bool result)
      {  next.setCurrentResult(result); return *this; }
   TypeSaveMemory& operator>>(T1& val)
      {  if (!next.getResult()) {
            val = save;
            tsetHolder(val, save, AffineType());
         }
         return next;
      }
   // to remove for the emission of compiler warnings
   TypeSaveMemory& operator>>(const T1& aval)
      {  T1& val = const_cast<T1&>(aval);
         if (!next.getResult()) {
            val = save;
            tsetHolder(val, save, AffineType());
         }
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
         for (; iter != end; ++iter) {
            save.insertAtEnd(*iter);
            tsetHolder(*iter, save.referenceAt(save.count()-1), AffineType());
         }
      }
   TPackedSaveMemory(const TPackedSaveMemory<TypeIterator, TypeSaveMemory>& source)
      :  save(source.save), next(source.next)
      {  int count = save.count();
         for (int index = 0; index < count; ++index)
            tremoveHolder(save.referenceAt(index), AffineType());
      }
   TPackedSaveMemory(TPackedSaveMemory<TypeIterator, TypeSaveMemory>&& source)
      :  save(std::move(source.save)), next(std::move(source.next))
      {  int count = save.count();
         for (int index = 0; index < count; ++index)
            tremoveHolder(save.referenceAt(index), AffineType());
      }

   template <typename T>
   TSaveMemory<T, TPackedSaveMemory<TypeIterator, TypeSaveMemory> > operator<<(T& t)
      {  return TSaveMemory<T, TPackedSaveMemory<TypeIterator, TypeSaveMemory> >(t, *this); }
   template <class TypeIteratorArgument>
   TPackedSaveMemory<TypeIteratorArgument, TPackedSaveMemory<TypeIterator, TypeSaveMemory> >
      operator<<(MergeBranches::TPacker<TypeIteratorArgument> packer)
      {  return TPackedSaveMemory<TypeIteratorArgument, TPackedSaveMemory<TypeIterator, TypeSaveMemory> >
            (packer.iter, packer.end, *this);
      }
   TPackedSaveMemory<TypeIterator, TypeSaveMemory>& operator<<(BaseExecutionPath::end) { return *this; }
   TPackedSaveMemory<TypeIterator, TypeSaveMemory>& operator<<(BaseExecutionPath::nothing) { return *this; }

   TPackedSaveMemory<TypeIterator, TypeSaveMemory>& setCurrentResult(bool result)
      {  next.setCurrentResult(result); return *this; }
   
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
   TSaveMemory<T, SaveMemory> operator<<(T& t)
      {  return TSaveMemory<T, SaveMemory>(t, *this); }
   template <typename T>
   TSaveMemory<T, SaveMemory> operator<<(const T& t)
      {  return TSaveMemory<T, SaveMemory>(const_cast<T&>(t), *this); }
   template <class TypeIterator>
   TPackedSaveMemory<TypeIterator, SaveMemory>
      operator<<(MergeBranches::TPacker<TypeIterator> packer)
      {  return TPackedSaveMemory<TypeIterator, SaveMemory>(packer.iter, packer.end, *this); }
   SaveMemory& operator<<(BaseExecutionPath::end) { return *this; }
   SaveMemory& operator<<(BaseExecutionPath::nothing) { return *this; }
   SaveMemory& setCurrentResult(bool result) { fResult = result; return *this; }
   bool getResult() const { return fResult; }
   bool operator>>(BaseExecutionPath::end)
      {  bool result = fResult;
         if (fResult)
            fResult = false;
         return result;
      }
   SaveMemory& operator>>(BaseExecutionPath::nothing) { return *this; }
};

template <typename TypeIterator, class TypeMergeMemory>
class TPackedMergeMemory;

template <typename T1, class TypeMergeMemory>
class TMergeMemory {
  public:
   T1 merge;
   TypeMergeMemory next;

   TMergeMemory(const T1&, TypeMergeMemory nextArg)
      :  merge(), next(nextArg) {}
   TMergeMemory(const TMergeMemory<T1, TypeMergeMemory>& source) = default;
   TMergeMemory(TMergeMemory<T1, TypeMergeMemory>&& source) = default;

   template <typename T>
   TMergeMemory<T, TMergeMemory<T1, TypeMergeMemory> > operator>>(T& t)
      {  return TMergeMemory<T, TMergeMemory<T1, TypeMergeMemory> >(t, *this); }
   template <typename T>
   TMergeMemory<T, TMergeMemory<T1, TypeMergeMemory> > operator>>(const T& t)
      {  return TMergeMemory<T, TMergeMemory<T1, TypeMergeMemory> >(const_cast<T&>(t), *this); }
   template <typename TypeIterator>
   TPackedMergeMemory<TypeIterator, TMergeMemory<T1, TypeMergeMemory> > operator>>(MergeBranches::TPacker<TypeIterator> packer);

   TMergeMemory<T1, TypeMergeMemory>& operator>>(BaseExecutionPath::end) { return *this; }
   TMergeMemory<T1, TypeMergeMemory>& operator>>(BaseExecutionPath::nothing) { return *this; }
   TMergeMemory<T1, TypeMergeMemory>& setCurrentComplete(bool isComplete)
      {  next.setCurrentComplete(isComplete); return *this; }
   TypeMergeMemory& operator<<(T1& val)
      {  if (next.isComplete()) {
            if (val.optimizeValue()) {
               if (next.isFirst()) {
                  merge = val;
                  if (merge.doesSupportUnstableInLoop()) {
                     merge.getSRealDomain().clearHolder();
                     merge.getSError().clearHolder();
                  };
               }
               else
                  merge.mergeWith(val);
            }
            else
               next.setCurrentComplete(false);
         };
         val = merge;
         return next;
      }
   // to remove for the emission of compiler warnings
   TypeMergeMemory& operator<<(const T1& aval)
      {  T1& val = const_cast<T1&>(aval);
         if (next.isComplete()) {
            if (val.optimizeValue()) {
               if (next.isFirst()) {
                  merge = val;
                  if (merge.doesSupportUnstableInLoop()) {
                     merge.getSRealDomain().clearHolder();
                     merge.getSError().clearHolder();
                  };
               }
               else
                  merge.mergeWith(val);
            }
            else
               next.setCurrentComplete(false);
         };
         val = merge;
         return next;
      }
   bool isFirst() const { return next.isFirst(); }
   bool isComplete() const { return next.isComplete(); }
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
   TPackedMergeMemory<TypeIterator, TypeMergeMemory>& operator>>(BaseExecutionPath::nothing) { return *this; }
   TPackedMergeMemory<TypeIterator, TypeMergeMemory>& setCurrentComplete(bool isComplete)
      {  next.setCurrentComplete(isComplete); return *this; }
   TypeMergeMemory& operator<<(MergeBranches::TPacker<TypeIterator>&& packer)
      {  int count = packer.end - packer.iter;
         if (next.isComplete()) {
            auto iter = packer.iter;
            if (next.isFirst()) {
               AssumeCondition(merge.count() == 0)
               merge.bookPlace(count);
               for (; iter != packer.end; ++iter) {
                  if (iter->optimizeValue()) {
                     merge.insertAtEnd(*iter);
                     if (merge.slast().doesSupportUnstableInLoop()) {
                        merge.slast().getSRealDomain().clearHolder();
                        merge.slast().getSError().clearHolder();
                     };
                  }
                  else
                     next.setCurrentComplete(false);
               };
            }
            else {
               for (int index = 0; index < count; ++index) {
                  if (iter->optimizeValue())
                     merge.referenceAt(index).mergeWith(*iter);
                  else
                     next.setCurrentComplete(false);
                  ++iter;
               }
            }
         }
         if (merge.count() > 0) {
            for (int index = 0; index < count; ++index) {
               *packer.iter = merge[index];
               ++packer.iter;
            }
         };
         return next;
      }
   bool isFirst() const { return next.isFirst(); }
   bool isComplete() const { return next.isComplete(); }
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
   bool fComplete;

  public:
   MergeMemory() : fFirst(true), fComplete(false) {}

   template <typename T>
   TMergeMemory<T, MergeMemory> operator>>(T& t)
      {  return TMergeMemory<T, MergeMemory>(t, *this); }
   template <typename T>
   TMergeMemory<T, MergeMemory> operator>>(const T& t)
      {  return TMergeMemory<T, MergeMemory>(const_cast<T&>(t), *this); }
   template <typename TypeIterator>
   TPackedMergeMemory<TypeIterator, MergeMemory> operator>>(const MergeBranches::TPacker<TypeIterator>& packer)
      {  return TPackedMergeMemory<TypeIterator, MergeMemory>
            (packer.iter, packer.end, *this);
      }
   MergeMemory& operator>>(BaseExecutionPath::end) { return *this; }
   MergeMemory& operator>>(BaseExecutionPath::nothing) { return *this; }
   MergeMemory& setCurrentComplete(bool isComplete)
      {  fComplete = isComplete; return *this; }
   bool isFirst() const { return fFirst; }
   bool isComplete() const { return fComplete; }
   bool operator<<(BaseExecutionPath::end)
      {  if (fComplete)
            fFirst = false;
         return true;
      }
   MergeMemory& operator<<(BaseExecutionPath::nothing) { return *this; }
};

template <typename T, class StackedParentMemory>
struct TStackedMemory {
   StackedParentMemory parent;
   T& arg;
   TStackedMemory(StackedParentMemory&& aparent, T& aarg) : parent(std::move(aparent)), arg(aarg) {}
   TStackedMemory(TStackedMemory<T, StackedParentMemory>&& source) = default;

   template <typename TypeArg>
   TStackedMemory<TypeArg, TStackedMemory<T, StackedParentMemory>> operator>>(TypeArg& newarg)
      {  return TStackedMemory<TypeArg, TStackedMemory<T, StackedParentMemory>>
               (std::move(*this), newarg);
      }
   typedef typename StackedParentMemory::Memory Memory;
   auto& getMemory() { return parent.getMemory(); }
   template <typename TypeMemory> struct TApplicationType {
      using type = typename StackedParentMemory::template TApplicationType<TypeMemory, T>::type;
   };
   struct ApplicationType {
      using type = typename StackedParentMemory::template TApplicationType
            <typename StackedParentMemory::Memory, T>::type;
   };

   template <typename TypeMemory>
   auto apply(TypeMemory&& mergeMemory) const { return parent.apply(std::move(mergeMemory >> arg)); }
   auto operator>>(BaseExecutionPath::end)
      {  return apply(std::move(getMemory())) >> BaseExecutionPath::end(); }
};

template <class BaseMemory>
struct StackedMemory {
   typedef BaseMemory Memory;
   BaseMemory& parent;

   StackedMemory(BaseMemory& aparent) : parent(aparent) {}
   StackedMemory(StackedMemory&& source) = default;
   template <typename T>
   TStackedMemory<T, StackedMemory> operator>>(T& arg)
      {  return TStackedMemory<T, StackedMemory>(std::move(*this), arg); }
   BaseMemory& operator>>(BaseExecutionPath::end) { return parent; }
   Memory& getMemory() { return parent; }
   template <typename TypeMemory> struct TApplicationType {
      using type = TypeMemory;
   };
   template <typename TMemory>
   auto apply(TMemory&& baseMemory) const -> TMemory { return std::move(baseMemory); }
};

template <class BaseMemory>
StackedMemory<BaseMemory> createStackedMemory(const BaseMemory& parent)
   {  return StackedMemory<BaseMemory>(const_cast<BaseMemory&>(parent)); }

template<class ThisType, typename... TypeArgs>
inline void
continuousFlow(ThisType& thisVal,
      std::function<void (ThisType& thisArg, const TypeArgs&... args)> funAssign,
      const TypeArgs&... avals)
{  // see float_diagnosis.h FLOAT_SPLIT_ALL FLOAT_MERGE_ALL
   bool oldSupportUnstableInLoop = thisVal.doesSupportUnstableInLoop();
   thisVal.setSupportUnstableInLoop();
   auto* oldPathExplorer = ExecutionPath::getCurrentPathExplorer();
   bool oldDoesFollow = ExecutionPath::doesFollowFlow();
   ExecutionPath::clearFollowFlow();
   auto* oldOutputFile = ExecutionPath::getOutputFile();
   ExecutionPath::clearOutputFile();
   auto* oldInputTraceFile = ExecutionPath::inputTraceFile();
   const char* oldSynchronisationFile = ExecutionPath::synchronisationFile();
   int oldSynchronisationLine = ExecutionPath::synchronisationLine();
   bool isCompleteFlow = true;
   bool hasUnstableSynchronization = thisVal.getMode() == BaseExecutionPath::MRealAndImplementation;
   PathExplorer pathExplorer(ExecutionPath::queryMode(oldPathExplorer));
   ExecutionPath::setCurrentPathExplorer(&pathExplorer);

   auto mergeMemory = ((createStackedMemory(MergeMemory())
         >> thisVal) >> ... >> avals) >> BaseExecutionPath::end();
   auto saveMemory = ((SaveMemory() << thisVal) << ... << avals) << BaseExecutionPath::end();
   const char* sourceFile = __FILE__;
   int sourceLine = __LINE__;
   auto oldSourceInfo = BaseFloatAffine::querySplitInfo();
   bool doesIterate;
   do {
      try {
         BaseFloatAffine::splitBranches(sourceFile, sourceLine);
         funAssign(thisVal, avals...);
         isCompleteFlow = ((MergeBranches(sourceFile, sourceLine) << thisVal) << ... << avals)
               << BaseExecutionPath::end();
      } 
      catch (typename ThisType::anticipated_termination&) {
         isCompleteFlow = false;
         ExecutionPath::clearSynchronizationBranches();
      }
      catch (STG::EReadError& error) {
         if (const char* message = error.getMessage())
            ExecutionPath::getErrorStream() << "error: " << message << std::endl;
         else
            ExecutionPath::getErrorStream() << "error while reading input file!" << std::endl;
         isCompleteFlow = false;
         ExecutionPath::clearSynchronizationBranches();
      }
      ExecutionPath::setFollowFlow();
      doesIterate = ((mergeMemory.setCurrentComplete(isCompleteFlow) << thisVal) << ... << avals)
             << BaseExecutionPath::end();
      if (doesIterate)
         doesIterate = !(((createStackedMemory(saveMemory.setCurrentResult(pathExplorer
               .isFinished(ExecutionPath::queryMode(oldPathExplorer))))
            >> thisVal) >> ... >> avals) >> BaseExecutionPath::end());
   } while (doesIterate);

   ExecutionPath::setFollowFlow(oldDoesFollow, oldInputTraceFile,
         oldSynchronisationFile, oldSynchronisationLine);
   ExecutionPath::setOutputFile(oldOutputFile);
   ExecutionPath::setCurrentPathExplorer(oldPathExplorer);
   thisVal.setSupportUnstableInLoop(oldSupportUnstableInLoop);
   BaseFloatAffine::splitBranches(oldSourceInfo.first, oldSourceInfo.second);
   if (hasUnstableSynchronization && mergeMemory.isFirst())
      ExecutionPath::throwEmptyBranch(true);
}

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

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
class TInstrumentedFloatZonotope : public TFloatZonotope<ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> {
  private:
   typedef TFloatZonotope<ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> inherited;
   typedef TInstrumentedFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation> thisType;

  protected:
   friend class TMergeBranches<ExecutionPath>;

  public:
   typedef thisType InstrumentedAffineType;
   typedef DAffine::MergeBranches MergeBranches;
   struct ValueFromString {}; 

   TInstrumentedFloatZonotope() = default;
   TInstrumentedFloatZonotope(const char* value, ValueFromString)
      {  STG::IOObject::ISBase* in = ExecutionPath::acquireConstantStream(value);
         inherited::initFrom(*in);
         ExecutionPath::releaseConstantStream(in);
      }
   template <typename TypeValue> TInstrumentedFloatZonotope(TypeValue value)
         requires floating_point_promotion<TypeValue, TypeImplementation>
      {  if (!inherited::fSupportAtomic && inherited::oTraceFile)
            inherited::initFrom((TypeImplementation) value);
         else
            inherited::initFromAtomic((TypeImplementation) value);
      }
   template <typename TypeValue> TInstrumentedFloatZonotope(TypeValue value)
         requires floating_point_nopromotion<TypeValue, TypeImplementation>
      {  typedef DAffine::FloatDigitsHelper::TFloatDigits<long double> FloatDigits;
         TFloatZonotope<ExecutionPath, FloatDigits::UBitSizeMantissa,
               FloatDigits::UBitSizeExponent, long double> receiver;
         if (!inherited::fSupportAtomic && inherited::oTraceFile) {
            receiver.initFrom(value);
            inherited::operator=(std::move(receiver));
         }
         else {
            inherited::initFromAtomic((TypeImplementation) value);
            // receiver.initFromAtomic(value);
            // inherited::operator=(std::move(receiver));
         }
      }
   TInstrumentedFloatZonotope(TypeImplementation min, TypeImplementation max)
      :  inherited(min, max)
      {  if (inherited::doesSupportUnstableInLoop())
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
      }
   TInstrumentedFloatZonotope(TypeImplementation min, TypeImplementation max,
         TypeImplementation errmin, TypeImplementation errmax)
      :  inherited(min, max)
      {  inherited::setError(errmin, errmax);
         if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
      }
   template <typename TypeValue> TInstrumentedFloatZonotope(TypeValue value) requires std::integral<TypeValue>
      :  inherited(value) {}
   TInstrumentedFloatZonotope(const thisType& source) : inherited(source)
      {  if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
      }
   TInstrumentedFloatZonotope(thisType&& source) : inherited(std::move(source)) // [TODO] keep symbolic for constraints
      {  if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   TInstrumentedFloatZonotope(const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      :  inherited(source)
      {  if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   TInstrumentedFloatZonotope(TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& source)
      :  inherited(std::move(source))
      {  if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
      }
   TInstrumentedFloatZonotope& operator=(const thisType& source)
      {  inherited::operator=(source);
         if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
         return *this;
      }
   TInstrumentedFloatZonotope& operator=(thisType&& source) // [TODO] keep symbolic for constraints
      {  inherited::operator=(std::move(source));
         if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
         return *this;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   TInstrumentedFloatZonotope& operator=(const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      {  inherited::operator=(source);
         if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
         return *this;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   TInstrumentedFloatZonotope& operator=(TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& source)
      {  inherited::operator=(std::forward<TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument> >(std::move(source)));
         if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
         return *this;
      }
   template<typename T> thisType& operator=(T source) requires std::floating_point<T> or std::integral<T>
      {  return (thisType&) inherited::operator=(thisType(source)); }

   void cloneShareParts() {}

   thisType operator++() { return (thisType&) inherited::operator+=(thisType(1)); }
   thisType operator++(int) { thisType result = *this; inherited::operator+=(thisType(1)); return result; }
   friend thisType operator+(const thisType& first) { return first; }
   friend thisType operator+(thisType&& first) { return first; }
   friend thisType operator-(const thisType& first)
      {  thisType result(first); result.oppositeAssign(); return result; }
   friend thisType operator-(thisType&& first)
      {  thisType result(std::move(first)); result.oppositeAssign(); return result; }

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
   thisType& operator+=(const thisType& source)
      {  this->plusAssign(source, Equation::PCSourceRValue);
         if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
         return *this;
      }
   thisType& operator+=(thisType&& source)
      {  this->plusAssign(source, Equation::PCSourceXValue);
         if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
         return *this;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator+=(const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      {  return (thisType&) thisType::operator+=(std::forward<thisType>(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator+=(TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& source)
      {  return (thisType&) thisType::operator+=(std::forward<thisType>(source)); }
   template<typename T> thisType& operator+=(T source) requires std::floating_point<T> || std::integral<T>
      {  return (thisType&) thisType::operator+=(thisType(source)); }

   thisType& operator-=(const thisType& source)
      {  this->minusAssign(source, Equation::PCSourceRValue);
         if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
         return *this;
      }
   thisType& operator-=(thisType&& source)
      {  this->minusAssign(source, Equation::PCSourceXValue);
         if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
         return *this;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator-=(const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      {  return (thisType&) thisType::operator-=(thisType(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator-=(TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& source)
      {  return (thisType&) thisType::operator-=(std::forward<thisType>(source)); }
   template<typename T> thisType& operator-=(T source) requires std::floating_point<T> || std::integral<T>
      {  return (thisType&) thisType::operator-=(thisType(source)); }

   thisType& operator*=(const thisType& source)
      {  this->multAssign(source, Equation::PCSourceRValue);
         if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
         return *this;
      }
   thisType& operator*=(thisType&& source)
      {  this->multAssign(source, Equation::PCSourceXValue);
         if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
         return *this;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator*=(const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      {  return (thisType&) thisType::operator*=(thisType(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator*=(TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& source)
      {  return (thisType&) thisType::operator*=(std::forward<thisType>(source)); }
   template<typename T> thisType& operator*=(T source) requires std::floating_point<T> || std::integral<T>
      {  return (thisType&) thisType::operator*=(thisType(source)); }

   thisType& operator/=(const thisType& source)
      {  this->divAssign(source, Equation::PCSourceRValue);
         if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
         return *this;
      }
   thisType& operator/=(thisType&& source)
      {  this->divAssign(source, Equation::PCSourceXValue);
         if (inherited::doesSupportUnstableInLoop()) {
            inherited::getSRealDomain().setHolder(inherited::currentPathExplorer);
            inherited::getSError().setHolder(inherited::currentPathExplorer);
         };
         return *this;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator/=(const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      {  return (thisType&) thisType::operator/=(thisType(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator/=(TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& source)
      {  return (thisType&) thisType::operator/=(std::forward<thisType>(source)); }
   template<typename T> thisType& operator/=(T source) requires std::floating_point<T> || std::integral<T>
      {  return (thisType&) thisType::operator/=(thisType(source)); }

   friend thisType operator+(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.plusAssign(second, Equation::PCSourceRValue);
         return result;
      }
   friend thisType operator+(thisType&& first, const thisType& second)
      {  thisType result(std::move(first));
         result.plusAssign(second, Equation::PCSourceRValue);
         return result;
      }
   friend thisType operator+(const thisType& first, thisType&& second)
      {  thisType result(first);
         result.plusAssign(second, Equation::PCSourceXValue);
         return result;
      }
   friend thisType operator+(thisType&& first, thisType&& second)
      {  thisType result(std::move(first));
         result.plusAssign(second, Equation::PCSourceXValue);
         return result;
      }
   template<typename T> friend auto operator+(const thisType& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType> result(second);
         result += first;
         return result;
      }
   template<typename T> friend auto operator+(thisType&& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType> result(std::move(first));
         result += thisType(second);
         return result;
      }
   template<typename T> friend auto operator+(T first, const thisType& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType> result(first);
         result += second;
         return result;
      }
   template<typename T> friend auto operator+(T first, thisType&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType> result(std::move(second));
         result += thisType(first);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator+(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> result(first);
         result += second;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator+(thisType&& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> result(std::move(first));
         result += second;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator+(const thisType& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> result(std::move(second));
         result += first;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator+(thisType&& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> result(std::move(first));
         result += std::move(second);
         return result;
      }

   friend thisType operator-(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.minusAssign(second, Equation::PCSourceRValue);
         return result;
      }
   friend thisType operator-(thisType&& first, const thisType& second)
      {  thisType result(std::move(first));
         result.minusAssign(second, Equation::PCSourceRValue);
         return result;
      }
   friend thisType operator-(const thisType& first, thisType&& second)
      {  thisType result(first);
         result.minusAssign(second, Equation::PCSourceXValue);
         return result;
      }
   friend thisType operator-(thisType&& first, thisType&& second)
      {  thisType result(std::move(first));
         result.minusAssign(second, Equation::PCSourceXValue);
         return result;
      }
   template<typename T> friend auto operator-(const thisType& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MinusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MinusType>::UBitSizeExponent,
            typename OperatorType<T>::MinusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MinusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MinusType>::UBitSizeExponent,
            typename OperatorType<T>::MinusType> result(second);
         result.oppositeAssign();
         result += first;
         return result;
      }
   template<typename T> friend auto operator-(thisType&& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MinusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MinusType>::UBitSizeExponent,
            typename OperatorType<T>::MinusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MinusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MinusType>::UBitSizeExponent,
            typename OperatorType<T>::MinusType> result(std::move(first));
         result -= thisType(second);
         return result;
      }
   template<typename T> friend auto operator-(T first, const thisType& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMinusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMinusType>::UBitSizeExponent,
            typename OperatorType<T>::CoMinusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMinusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMinusType>::UBitSizeExponent,
            typename OperatorType<T>::CoMinusType> result(first);
         result -= second;
         return result;
      }
   template<typename T> friend auto operator-(T first, thisType&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMinusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMinusType>::UBitSizeExponent,
            typename OperatorType<T>::CoMinusType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMinusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMinusType>::UBitSizeExponent,
            typename OperatorType<T>::CoMinusType> result(std::move(second));
         result.oppositeAssign();
         result += thisType(first);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator-(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))> result(first);
         result -= second;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator-(thisType&& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))> result(std::move(first));
         result -= second;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator-(const thisType& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))> result(std::move(second));
         result.oppositeAssign();
         result += first;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator-(thisType&& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))> result(std::move(first));
         result -= std::move(second);
         return result;
      }

   friend thisType operator*(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.multAssign(second, Equation::PCSourceRValue);
         return result;
      }
   friend thisType operator*(thisType&& first, const thisType& second)
      {  thisType result(std::move(first));
         result.multAssign(second, Equation::PCSourceRValue);
         return result;
      }
   friend thisType operator*(const thisType& first, thisType&& second)
      {  thisType result(first);
         result.multAssign(second, Equation::PCSourceXValue);
         return result;
      }
   friend thisType operator*(thisType&& first, thisType&& second)
      {  thisType result(std::move(first));
         result.multAssign(second, Equation::PCSourceXValue);
         return result;
      }
   template<typename T> friend auto operator*(const thisType& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MultType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MultType>::UBitSizeExponent,
            typename OperatorType<T>::MultType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MultType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MultType>::UBitSizeExponent,
            typename OperatorType<T>::MultType> result(second);
         result *= first;
         return result;
      }
   template<typename T> friend auto operator*(thisType&& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MultType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MultType>::UBitSizeExponent,
            typename OperatorType<T>::MultType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MultType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::MultType>::UBitSizeExponent,
            typename OperatorType<T>::MultType> result(std::move(first));
         result *= thisType(second);
         return result;
      }
   template<typename T> friend auto operator*(T first, const thisType& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMultType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMultType>::UBitSizeExponent,
            typename OperatorType<T>::CoMultType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMultType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMultType>::UBitSizeExponent,
            typename OperatorType<T>::CoMultType> result(first);
         result *= second;
         return result;
      }
   template<typename T> friend auto operator*(T first, thisType&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMultType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMultType>::UBitSizeExponent,
            typename OperatorType<T>::CoMultType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMultType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoMultType>::UBitSizeExponent,
            typename OperatorType<T>::CoMultType> result(std::move(second));
         result *= thisType(first);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator*(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))> result(first);
         result *= second;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator*(thisType&& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))> result(std::move(first));
         result *= second;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator*(const thisType& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))> result(std::move(second));
         result *= first;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator*(thisType&& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))> result(std::move(first));
         result *= std::move(second);
         return result;
      }

   friend thisType operator/(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.divAssign(second, Equation::PCSourceRValue);
         return result;
      }
   friend thisType operator/(thisType&& first, const thisType& second)
      {  thisType result(std::move(first));
         result.divAssign(second, Equation::PCSourceRValue);
         return result;
      }
   friend thisType operator/(const thisType& first, thisType&& second)
      {  thisType result(first);
         result.divAssign(second, Equation::PCSourceXValue);
         return result;
      }
   friend thisType operator/(thisType&& first, thisType&& second)
      {  thisType result(std::move(first));
         result.divAssign(second, Equation::PCSourceXValue);
         return result;
      }
   template<typename T> friend auto operator/(const thisType& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::DivType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::DivType>::UBitSizeExponent,
            typename OperatorType<T>::DivType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::DivType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::DivType>::UBitSizeExponent,
            typename OperatorType<T>::DivType> result(first);
         result /= thisType(second);
         return result;
      }
   template<typename T> friend auto operator/(thisType&& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::DivType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::DivType>::UBitSizeExponent,
            typename OperatorType<T>::DivType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::DivType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::DivType>::UBitSizeExponent,
            typename OperatorType<T>::DivType> result(std::move(first));
         result /= thisType(second);
         return result;
      }
   template<typename T> friend auto operator/(T first, const thisType& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoDivType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoDivType>::UBitSizeExponent,
            typename OperatorType<T>::CoDivType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoDivType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoDivType>::UBitSizeExponent,
            typename OperatorType<T>::CoDivType> result(first);
         result /= second;
         return result;
      }
   template<typename T> friend auto operator/(T first, thisType&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoDivType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoDivType>::UBitSizeExponent,
            typename OperatorType<T>::CoDivType>
         requires std::floating_point<T> || std::integral<T>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoDivType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoDivType>::UBitSizeExponent,
            typename OperatorType<T>::CoDivType> result(first);
         result /= std::move(second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator/(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))> result(first);
         result /= second;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator/(thisType&& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))> result(std::move(first));
         result /= second;
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator/(const thisType& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))> result(first);
         result /= std::move(second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator/(thisType&& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))>
      {  TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))> result(std::move(first));
         result /= std::move(second);
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
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareLess(first, TypeComparison(second));
      }
   template <typename T> friend bool operator<(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareLess(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator<(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareLess(first, second);
      }

   friend bool operator<=(const thisType& first, const thisType& second)
      {  return first.inherited::operator<=(second); }
   friend bool operator<=(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator<=(thisType(second)); }
   template <typename T> friend bool operator<=(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareLessOrEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator<=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareLessOrEqual(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator<=(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareLessOrEqual(first, second);
      }

   friend bool operator==(const thisType& first, const thisType& second)
      {  return first.inherited::operator==(second); }
   friend bool operator==(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator==(thisType(second)); }
   template <typename T> friend bool operator==(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator==(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareEqual(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator==(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareEqual(first, second);
      }

   friend bool operator!=(const thisType& first, const thisType& second)
      {  return first.inherited::operator!=(second); }
   friend bool operator!=(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator!=(thisType(second)); }
   template <typename T> friend bool operator!=(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareDifferent(first, TypeComparison(second));
      }
   template <typename T> friend bool operator!=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareDifferent(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator!=(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareDifferent(first, second);
      }

   friend bool operator>=(const thisType& first, const thisType& second)
      {  return first.inherited::operator>=(second); }
   friend bool operator>=(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator>=(thisType(second)); }
   template <typename T> friend bool operator>=(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator>=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator>=(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(first, second);
      }

   friend bool operator>(const thisType& first, const thisType& second)
      {  return first.inherited::operator>(second); }
   friend bool operator>(const thisType& first, TypeImplementation second)
      {  return first.inherited::operator>(thisType(second)); }
   template <typename T> friend bool operator>(const thisType& first, T second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType> TypeComparison;
         return TypeComparison::compareGreater(first, TypeComparison(second));
      }
   template <typename T> friend bool operator>(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType> TypeComparison;
         return TypeComparison::compareGreater(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator>(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeComparison;
         return TypeComparison::compareGreater(first, second);
      }

   // math operators
   friend thisType abs(const thisType& source)
      {  auto result(source);
#ifdef FLOAT_CONCRETE
         auto cstValue = std::abs(inherited::asImplementation());
#endif
         std::function<void(thisType&)> fun = [](thisType& val){ if (val < 0) val.oppositeAssign(); };
         continuousFlow(result, fun);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   friend thisType abs(thisType&& source)
      {  auto result(std::move(source));
#ifdef FLOAT_CONCRETE
         auto cstValue = std::abs(inherited::asImplementation());
#endif
         std::function<void(thisType&)> fun = [](thisType& val){ if (val < 0) val.oppositeAssign(); };
         continuousFlow(result, fun);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   friend thisType fabs(const thisType& source)
      {  auto result(source);
#ifdef FLOAT_CONCRETE
         auto cstValue = std::abs(inherited::asImplementation());
#endif
         std::function<void(thisType&)> fun = [](thisType& val){ if (val < 0) val.oppositeAssign(); };
         continuousFlow(result, fun);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   friend thisType fabs(thisType&& source)
      {  auto result(std::move(source));
#ifdef FLOAT_CONCRETE
         auto cstValue = std::abs(inherited::asImplementation());
#endif
         std::function<void(thisType&)> fun = [](thisType& val){ if (val < 0) val.oppositeAssign(); };
         continuousFlow(result, fun);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   friend thisType min(const thisType& first, const thisType& second)
      {  auto result(first), source(second); // source should gain back its original value
#ifdef FLOAT_CONCRETE
         auto cstValue = std::min(result.asImplementation(), source.asImplementation());
#endif
         std::function<void(thisType&, const thisType&)> fun = [](thisType& val, const thisType& source)
               { if (val > source) val = source; };
         continuousFlow(result, fun, source);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   friend thisType min(thisType&& first, const thisType& second)
      {  auto source(second);
#ifdef FLOAT_CONCRETE
         auto cstValue = std::min(first.asImplementation(), source.asImplementation());
#endif
         std::function<void(thisType&, const thisType&)> fun = [](thisType& val, const thisType& source)
               { if (val > source) val = source; };
         continuousFlow(first, fun, source);
#ifdef FLOAT_CONCRETE
         first.getSImplementation() = cstValue;
#endif
         return first;
      }
   friend thisType min(const thisType& first, thisType&& second)
      {  auto result(first);
#ifdef FLOAT_CONCRETE
         auto cstValue = std::min(result.asImplementation(), second.asImplementation());
#endif
         std::function<void(thisType&, const thisType&)> fun = [](thisType& val, const thisType& source)
               { if (val > source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   friend thisType min(thisType&& first, thisType&& second)
      {  
#ifdef FLOAT_CONCRETE
         auto cstValue = std::min(first.asImplementation(), second.asImplementation());
#endif
         std::function<void(thisType&, const thisType&)> fun = [](thisType& val, const thisType& source)
               { if (val > source) val = source; };
         continuousFlow(first, fun, second);
#ifdef FLOAT_CONCRETE
         first.getSImplementation() = cstValue;
#endif
         return first;
      }
   template<typename T> friend auto min(const thisType& first, T asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::min(first.asImplementation(), asecond);
#endif
         TypeResult result(first), second(asecond);
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val > source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template<typename T> friend auto min(thisType&& first, T asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::min(first.asImplementation(), asecond);
#endif
         TypeResult result(std::move(first)), second(asecond);
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val > source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template<typename T> friend auto min(T first, const thisType& asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::min(first, asecond.asImplementation());
#endif
         TypeResult result(first), second(asecond);
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val > source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template<typename T> friend auto min(T first, thisType&& asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::min(first, asecond.asImplementation());
#endif
         TypeResult result(first), second(std::move(asecond));
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val > source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto min(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::min(first.asImplementation(), asecond.asImplementation());
#endif
         TypeResult result(first), second(asecond);
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val > source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto min(thisType&& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::min(first.asImplementation(), asecond.asImplementation());
#endif
         TypeResult result(std::move(first)), second(asecond);
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val > source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto min(const thisType& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::min(first.asImplementation(), asecond.asImplementation());
#endif
         TypeResult result(first), second(std::move(asecond));
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val > source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto min(thisType&& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::min(first.asImplementation(), asecond.asImplementation());
#endif
         TypeResult result(std::move(first)), second(std::move(asecond));
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val > source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }

   friend thisType max(const thisType& first, const thisType& second)
      {  auto result(first), source(second); // source should gain back its original value
#ifdef FLOAT_CONCRETE
         auto cstValue = std::max(result.asImplementation(), source.asImplementation());
#endif
         std::function<void(thisType&, const thisType&)> fun = [](thisType& val, const thisType& source)
               { if (val < source) val = source; };
         continuousFlow(result, fun, source);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   friend thisType max(thisType&& first, const thisType& second)
      {  auto source(second);
#ifdef FLOAT_CONCRETE
         auto cstValue = std::max(first.asImplementation(), source.asImplementation());
#endif
         std::function<void(thisType&, const thisType&)> fun = [](thisType& val, const thisType& source)
               { if (val < source) val = source; };
         continuousFlow(first, fun, source);
#ifdef FLOAT_CONCRETE
         first.getSImplementation() = cstValue;
#endif
         return first;
      }
   friend thisType max(const thisType& first, thisType&& second)
      {  auto result(first);
#ifdef FLOAT_CONCRETE
         auto cstValue = std::max(result.asImplementation(), second.asImplementation());
#endif
         std::function<void(thisType&, const thisType&)> fun = [](thisType& val, const thisType& source)
               { if (val < source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   friend thisType max(thisType&& first, thisType&& second)
      {  
#ifdef FLOAT_CONCRETE
         auto cstValue = std::max(first.asImplementation(), second.asImplementation());
#endif
         std::function<void(thisType&, const thisType&)> fun = [](thisType& val, const thisType& source)
               { if (val < source) val = source; };
         continuousFlow(first, fun, second);
#ifdef FLOAT_CONCRETE
         first.getSImplementation() = cstValue;
#endif
         return first;
      }
   template<typename T> friend auto max(const thisType& first, T asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::max(first.asImplementation(), asecond);
#endif
         TypeResult result(first), second(asecond);
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val < source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template<typename T> friend auto max(thisType&& first, T asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::PlusType>::UBitSizeExponent,
            typename OperatorType<T>::PlusType> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::max(first.asImplementation(), asecond);
#endif
         TypeResult result(std::move(first)), second(asecond);
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val < source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template<typename T> friend auto max(T first, const thisType& asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::max(first, asecond.asImplementation());
#endif
         TypeResult result(first), second(asecond);
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val < source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template<typename T> friend auto max(T first, thisType&& asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<typename OperatorType<T>::CoPlusType>::UBitSizeExponent,
            typename OperatorType<T>::CoPlusType> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::max(first, asecond.asImplementation());
#endif
         TypeResult result(first), second(std::move(asecond));
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val < source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto max(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::max(first.asImplementation(), asecond.asImplementation());
#endif
         TypeResult result(first), second(asecond);
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val < source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto max(thisType&& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::max(first.asImplementation(), asecond.asImplementation());
#endif
         TypeResult result(std::move(first)), second(asecond);
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val < source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto max(const thisType& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::max(first.asImplementation(), asecond.asImplementation());
#endif
         TypeResult result(first), second(std::move(asecond));
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val < source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto max(thisType&& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeResult;
#ifdef FLOAT_CONCRETE
         auto cstValue = std::max(first.asImplementation(), asecond.asImplementation());
#endif
         TypeResult result(std::move(first)), second(std::move(asecond));
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& val, const TypeResult& source)
               { if (val < source) val = source; };
         continuousFlow(result, fun, second);
#ifdef FLOAT_CONCRETE
         result.getSImplementation() = cstValue;
#endif
         return result;
      }

   thisType median(const thisType& afst, const thisType& asnd) const
      {  auto result(*this), fst(afst), snd(asnd);
         std::function<void(thisType&, const thisType&, const thisType&)> fun
            = [](thisType& val, const thisType& fst, const thisType& snd)
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
                  };
         continuousFlow(result, fun, fst, snd);
         return result;
      }
   thisType median(TypeImplementation afst, const thisType& asnd) const
      {  auto result(*this), snd(asnd);
         thisType fst(afst);
         std::function<void(thisType&, const thisType&, const thisType&)> fun
            = [](thisType& val, const thisType& fst, const thisType& snd)
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
                  };
         continuousFlow(result, fun, fst, snd);
         return result;
      }
   thisType median(const thisType& afst, TypeImplementation asnd) const
      {  auto result(*this), fst(afst);
         thisType snd(asnd);
         std::function<void(thisType&, const thisType&, const thisType&)> fun
            = [](thisType& val, const thisType& fst, const thisType& snd)
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
                  };
         continuousFlow(result, fun, fst, snd);
         return result;
      }
   thisType median(TypeImplementation afst, TypeImplementation asnd) const
      {  auto result(*this);
         thisType fst(afst), snd(asnd);
         std::function<void(thisType&, const thisType&, const thisType&)> fun
            = [](thisType& val, const thisType& fst, const thisType& snd)
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
                  };
         continuousFlow(result, fun, fst, snd);
         return result;
      }

   void lightPersist(const char* prefix) const { inherited::lightPersist(*this, prefix); }
   void persist(const char* prefix) const { inherited::persist(*this, prefix); }

   friend std::ostream& operator<<(std::ostream& out, const thisType& source)
      {  return out << source.asImplementation(); }
   friend std::istream& operator>>(std::istream& in, thisType& source)
      {  TypeImplementation val;
         in >> val;
         source = thisType(val);
         return in;
      }

   friend thisType sqrt(const thisType& source)
      {  thisType result(source); result.sqrtAssign(); return result; }
   friend thisType sqrt(thisType&& source)
      {  thisType result(std::forward<thisType>(source)); result.sqrtAssign(); return result; }
   friend thisType sin(const thisType& source)
      {  thisType result(source); result.sinAssign(); return result; }
   friend thisType sin(thisType&& source)
      {  thisType result(std::forward<thisType>(source)); result.sinAssign(); return result; }
   friend thisType cos(const thisType& source)
      {  thisType result(source); result.cosAssign(); return result; }
   friend thisType cos(thisType&& source)
      {  thisType result(std::forward<thisType>(source)); result.cosAssign(); return result; }
   friend thisType asin(const thisType& source)
      {  thisType result(source); result.asinAssign(); return result; }
   friend thisType asin(thisType&& source)
      {  thisType result(std::forward<thisType>(source)); result.asinAssign(); return result; }
   friend thisType acos(const thisType& source)
      {  thisType result(source); result.acosAssign(); return result; }
   friend thisType acos(thisType&& source)
      {  thisType result(std::forward<thisType>(source)); result.acosAssign(); return result; }
   friend thisType tan(const thisType& source)
      {  thisType result(source); result.tanAssign(); return result; }
   friend thisType tan(thisType&& source)
      {  thisType result(std::forward<thisType>(source)); result.tanAssign(); return result; }
   friend thisType atan(const thisType& source)
      {  thisType result(source); result.atanAssign(); return result; }
   friend thisType atan(thisType&& source)
      {  thisType result(std::forward<thisType>(source)); result.atanAssign(); return result; }
   friend thisType exp(const thisType& source)
      {  thisType result(source); result.expAssign(); return result; }
   friend thisType exp(thisType&& source)
      {  thisType result(std::forward<thisType>(source)); result.expAssign(); return result; }
   friend thisType log(const thisType& source)
      {  thisType result(source); result.logAssign(); return result; }
   friend thisType log(thisType&& source)
      {  thisType result(std::forward<thisType>(source)); result.logAssign(); return result; }
   friend thisType log2(const thisType& source)
      {  thisType result(source); result.logAssign(); result.divAssign(log(thisType(2.0)), Equation::PCSourceXValue); return result; }
   friend thisType log2(thisType&& source)
      {  thisType result(std::forward<thisType>(source)); result.logAssign(); result.divAssign(log(thisType(2.0)), Equation::PCSourceXValue); return result; }
   friend thisType exp2(const thisType& fst)
      {  thisType result = 2.0; result.powAssign(fst, Equation::PCSourceRValue); return result; }
   friend thisType exp2(thisType&& fst)
      {  thisType result = 2.0; result.powAssign(fst, Equation::PCSourceXValue); return result; }
   friend thisType log10(const thisType& source)
      {  thisType result(source); result.log10Assign(); return result; }
   friend thisType log10(thisType&& source)
      {  thisType result(std::forward<thisType>(source)); result.log10Assign(); return result; }

   friend thisType pow(const thisType& source, const thisType& value)
      {  thisType result(source); result.powAssign(value, Equation::PCSourceRValue); return result; }
   friend thisType pow(const thisType& source, thisType&& value)
      {  thisType result(source); result.powAssign(value, Equation::PCSourceXValue); return result; }
   friend thisType pow(thisType&& source, const thisType& value)
      {  thisType result(std::forward<thisType>(source)); result.powAssign(value, Equation::PCSourceRValue); return result; }
   friend thisType pow(thisType&& source, thisType&& value)
      {  thisType result(std::forward<thisType>(source)); result.powAssign(value, Equation::PCSourceXValue); return result; }
   template<typename T> friend auto pow(const thisType& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto pow(thisType&& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(std::move(first));
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto pow(T first, const thisType& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(pow(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(pow(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto pow(T first, thisType&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(pow(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(pow(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(std::move(second)));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto pow(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto pow(thisType&& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::move(first));
         result.powAssign(TypeResult(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto pow(const thisType& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(std::move(second)));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto pow(thisType&& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::move(first));
         result.powAssign(TypeResult(std::move(second)));
         return result;
      }

   friend thisType powf(const thisType& source, const thisType& value)
      {  thisType result(source); result.powAssign(value, Equation::PCSourceRValue); return result; }
   friend thisType powf(const thisType& source, thisType&& value)
      {  thisType result(source); result.powAssign(value, Equation::PCSourceXValue); return result; }
   friend thisType powf(thisType&& source, const thisType& value)
      {  thisType result(std::forward<thisType>(source)); result.powAssign(value, Equation::PCSourceRValue); return result; }
   friend thisType powf(thisType&& source, thisType&& value)
      {  thisType result(std::forward<thisType>(source)); result.powAssign(value, Equation::PCSourceXValue); return result; }
   template<typename T> friend auto powf(const thisType& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto powf(thisType&& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(std::move(first));
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto powf(T first, const thisType& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(powf(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(powf(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto powf(T first, thisType&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(powf(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(powf(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(std::move(second)));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto powf(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto powf(thisType&& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::move(first));
         result.powAssign(TypeResult(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto powf(const thisType& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(std::move(second)));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto powf(thisType&& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::move(first));
         result.powAssign(TypeResult(std::move(second)));
         return result;
      }

   friend thisType atan2(const thisType& source, const thisType& value)
      {  thisType result(source); result.atan2Assign(value, Equation::PCSourceRValue); return result; }
   friend thisType atan2(const thisType& source, thisType&& value)
      {  thisType result(source); result.atan2Assign(value, Equation::PCSourceXValue); return result; }
   friend thisType atan2(thisType&& source, const thisType& value)
      {  thisType result(std::forward<thisType>(source)); result.atan2Assign(value, Equation::PCSourceRValue); return result; }
   friend thisType atan2(thisType&& source, thisType&& value)
      {  thisType result(std::forward<thisType>(source)); result.atan2Assign(value, Equation::PCSourceXValue); return result; }
   template<typename T> friend auto atan2(const thisType& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto atan2(thisType&& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(std::move(first));
         result.atan2Assign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto atan2(T first, const thisType& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(atan2(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(atan2(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto atan2(T first, thisType&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(atan2(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(atan2(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(std::move(second)));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto atan2(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto atan2(thisType&& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::move(first));
         result.atan2Assign(TypeResult(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto atan2(const thisType& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(std::move(second)));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto atan2(thisType&& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::move(first));
         result.atan2Assign(TypeResult(std::move(second)));
         return result;
      }

   friend thisType floor(const thisType& afst)
      {  thisType result, fst(afst); // afst should gain back its original value
         std::function<void(thisType&, const thisType&)> fun = [](thisType& result, const thisType& fst)
               {  result = thisType(fst.asInt(inherited::ReadParametersBase::RMLowest)); };
         continuousFlow(result, fun, fst);
         return result;
      }
   friend thisType floor(thisType&& fst)
      {  thisType result;
         std::function<void(thisType&, const thisType&)> fun = [](thisType& result, const thisType& fst)
               {  result = thisType(fst.asInt(inherited::ReadParametersBase::RMLowest)); };
         continuousFlow(result, fun, fst);
         return result;
      }
   friend thisType ceil(const thisType& afst)
      {  thisType result, fst(afst);
         std::function<void(thisType&, const thisType&)> fun = [](thisType& result, const thisType& fst)
               {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMHighest)); };
         continuousFlow(result, fun, fst);
         return result;
      }
   friend thisType ceil(thisType&& fst)
      {  thisType result;
         std::function<void(thisType&, const thisType&)> fun = [](thisType& result, const thisType& fst)
               {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMHighest)); };
         continuousFlow(result, fun, fst);
         return result;
      }
   friend thisType trunc(const thisType& afst)
      {  thisType result, fst(afst);
         std::function<void(thisType&, const thisType&)> fun = [](thisType& result, const thisType& fst)
               {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMZero)); };
         continuousFlow(result, fun, fst);
         return result;
      }
   friend thisType trunc(thisType&& fst)
      {  thisType result;
         std::function<void(thisType&, const thisType&)> fun = [](thisType& result, const thisType& fst)
               {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMZero)); };
         continuousFlow(result, fun, fst);
         return result;
      }
   friend thisType round(const thisType& afst)
      {  thisType result, fst(afst);
         std::function<void(thisType&, const thisType&)> fun = [](thisType& result, const thisType& fst)
               {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMNearest)); };
         continuousFlow(result, fun, fst);
         return result;
      }
   friend thisType round(thisType&& fst)
      {  thisType result;
         std::function<void(thisType&, const thisType&)> fun = [](thisType& result, const thisType& fst)
               {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMNearest)); };
         continuousFlow(result, fun, fst);
         return result;
      }
   friend thisType rint(const thisType& afst)
      {  thisType result, fst(afst);
         std::function<void(thisType&, const thisType&)> fun = [](thisType& result, const thisType& fst)
               {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMNearest)); };
         continuousFlow(result, fun, fst);
         return result;
      }
   friend thisType rint(thisType&& fst)
      {  thisType result;
         std::function<void(thisType&, const thisType&)> fun = [](thisType& result, const thisType& fst)
               {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMNearest)); };
         continuousFlow(result, fun, fst);
         return result;
      }
   friend thisType rintf(const thisType& afst)
      {  thisType result, fst(afst);
         std::function<void(thisType&, const thisType&)> fun = [](thisType& result, const thisType& fst)
               {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMNearest)); };
         continuousFlow(result, fun, fst);
         return result;
      }
   friend thisType rintf(thisType&& fst)
      {  thisType result;
         std::function<void(thisType&, const thisType&)> fun = [](thisType& result, const thisType& fst)
               {  result = thisType(fst.asInt(thisType::ReadParametersBase::RMNearest)); };
         continuousFlow(result, fun, fst);
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
         std::function<void(thisType&, const thisType&, const thisType&)> fun = [](thisType& result, const thisType& source, const thisType& value)
            {  auto divResult(source); divResult /= value;
               result = thisType(divResult.asInt(thisType::ReadParametersBase::RMZero));
               result *= value;
               result -= source;
               result.oppositeAssign();
            };
         continuousFlow(multResult, fun, first, second);
         return multResult;
      }
   friend thisType fmod(thisType&& first, const thisType& second)
      {  std::function<void(thisType&, const thisType&)> fun = [](thisType& source, const thisType& value)
            {  auto divResult(source); divResult /= value;
               thisType multResult(divResult.asInt(thisType::ReadParametersBase::RMZero));
               multResult *= value; source -= multResult;
            };
         continuousFlow(first, fun, first, second);
         return first;
      }
   friend thisType fmod(const thisType& first, thisType&& second)
      {  thisType multResult;
         std::function<void(thisType&, const thisType&, const thisType&)> fun = [](thisType& result, const thisType& source, const thisType& value)
            {  auto divResult(source); divResult /= value;
               result = thisType(divResult.asInt(thisType::ReadParametersBase::RMZero));
               result *= value;
               result -= source;
               result.oppositeAssign();
            };
         continuousFlow(multResult, fun, first, second);
         return multResult;
      }
   friend thisType fmod(thisType&& first, thisType&& second)
      {  std::function<void(thisType&, const thisType&)> fun = [](thisType& source, const thisType& value)
            {  auto divResult(source); divResult /= value;
               thisType multResult(divResult.asInt(thisType::ReadParametersBase::RMZero));
               multResult *= value; source -= multResult;
            };
         continuousFlow(first, fun, first, second);
         return first;
      }
   template<typename T> friend auto fmod(const thisType& first, T asecond)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult second(asecond);
         TypeResult result;
         std::function<void(TypeResult&, const TypeResult&, const TypeResult&)> fun = [](TypeResult& multResult, const TypeResult& source, const TypeResult& value)
            {  auto divResult(source); divResult /= value;
               multResult = TypeResult(divResult.asInt(TypeResult::ReadParametersBase::RMZero));
               multResult *= value;
               multResult -= source;
               multResult.oppositeAssign();
            };
         continuousFlow(result, fun, (const TypeResult&) first, second);
         return result;
      }
   template<typename T> friend auto fmod(thisType&& first, T second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(std::move(first));
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& source, const TypeResult& value)
            {  auto divResult(source); divResult /= value;
               TypeResult multResult(divResult.asInt(TypeResult::ReadParametersBase::RMZero));
               multResult *= value; source -= multResult;
            };
         continuousFlow(result, fun, result, TypeResult(second));
         return result;
      }
   template<typename T> friend auto fmod(T first, const thisType& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(fmod(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(fmod(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& source, const TypeResult& value)
            {  auto divResult(source); divResult /= value;
               TypeResult multResult(divResult.asInt(TypeResult::ReadParametersBase::RMZero));
               multResult *= value; source -= multResult;
            };
         continuousFlow(result, fun, result, (const TypeResult&) second);
         return result;
      }
   template<typename T> friend auto fmod(T first, thisType&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(fmod(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(fmod(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& source, const TypeResult& value)
            {  auto divResult(source); divResult /= value;
               TypeResult multResult(divResult.asInt(TypeResult::ReadParametersBase::RMZero));
               multResult *= value; source -= multResult;
            };
         continuousFlow(result, fun, result, (const TypeResult&) std::forward<TypeResult>(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto fmod(const thisType& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult multResult;
         std::function<void(TypeResult&, const TypeResult&, const TypeResult&)> fun = [](TypeResult& result, const TypeResult& source, const TypeResult& value)
            {  auto divResult(source); divResult /= value;
               result = TypeResult(divResult.asInt(TypeResult::ReadParametersBase::RMZero));
               result *= value;
               result -= source;
               result.oppositeAssign();
            };
         continuousFlow(multResult, fun, (const TypeResult&) first, (const TypeResult&) second);
         return multResult;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto fmod(thisType&& first, const TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::move(first));
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& source, const TypeResult& value)
            {  auto divResult(source); divResult /= value;
               TypeResult multResult(divResult.asInt(TypeResult::ReadParametersBase::RMZero));
               multResult *= value; source -= multResult;
            };
         continuousFlow(result, fun, result, (const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto fmod(const thisType& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult multResult;
         std::function<void(TypeResult&, const TypeResult&, const TypeResult&)> fun = [](TypeResult& result, const TypeResult& source, const TypeResult& value)
            {  auto divResult(source); divResult /= value;
               result = TypeResult(divResult.asInt(TypeResult::ReadParametersBase::RMZero));
               result *= value;
               result -= source;
               result.oppositeAssign();
            };
         continuousFlow(multResult, fun, (const TypeResult&) first, (const TypeResult&) std::forward<TypeResult>(second));
         return multResult;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto fmod(thisType&& first, TInstrumentedFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TInstrumentedFloatZonotope<
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            FloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::move(first));
         std::function<void(TypeResult&, const TypeResult&)> fun = [](TypeResult& source, const TypeResult& value)
            {  auto divResult(source); divResult /= value;
               TypeResult multResult(divResult.asInt(TypeResult::ReadParametersBase::RMZero));
               multResult *= value; source -= multResult;
            };
         continuousFlow(result, fun, result, (const TypeResult&) std::forward<TypeResult>(second));
         return result;
      }

   friend int fld_finite(const thisType& source) { return tfinite(source.asImplementation()); }
   friend int fld_isfinite(const thisType& source) { return tisfinite(source.asImplementation()); }
   friend int fld_isnan(const thisType& source) { return tisnan(source.asImplementation()); }
   friend int fld_isinf(const thisType& source) { return tisinf(source.asImplementation()); }
};

} // end of namespace DAffine

typedef DAffine::FloatDigitsHelper::TFloatDigits<long double> LongDoubleFloatDigits;

typedef DAffine::TInstrumentedFloatZonotope<23, 8, float> FloatZonotope;
typedef DAffine::TInstrumentedFloatZonotope<52, 11, double> DoubleZonotope;
typedef DAffine::TInstrumentedFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa,
        LongDoubleFloatDigits::UBitSizeExponent, long double> LongDoubleZonotope;

class ParseFloatZonotope : public FloatZonotope {
  public:
   ParseFloatZonotope(const char* value) : FloatZonotope(value, ValueFromString()) {}
};

class ParseDoubleZonotope : public DoubleZonotope {
  public:
   ParseDoubleZonotope(const char* value) : DoubleZonotope(value, ValueFromString()) {}
};

class ParseLongDoubleZonotope : public LongDoubleZonotope {
  public:
   ParseLongDoubleZonotope(const char* value) : LongDoubleZonotope(value, ValueFromString()) {}
};

} // end of namespace NumericalDomains

namespace COL { namespace DVector {

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
class TElementTraits<NumericalDomains::DAffine::TInstrumentedFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation> > {
  private:
   typedef NumericalDomains::DAffine::TInstrumentedFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation> TypeElement;

  public:
   static const bool FInitialCleared = true;
   static void clear(TypeElement& element)
      {  element = TypeElement(); }
   static void clearAll(TypeElement* array, int count)
      {  while (--count >= 0)
            clear(array[count]);
      }
   static void copy(TypeElement& element, const TypeElement& source)
      {  element = source; }
   static void move(TypeElement& element, TypeElement&& source)
      {  element = std::move(source); }
   static void copyAll(TypeElement* array, const TypeElement* source, int count)
      {  for (int index = 0; index < count; ++index)
            copy(array[index], source[index]);
      }
   static void moveAll(TypeElement* array, TypeElement* source, int count)
      {  for (int index = 0; index < count; ++index)
            move(array[index], std::move(source[index]));
      }
};

}} // end of namespace COL::DVector

