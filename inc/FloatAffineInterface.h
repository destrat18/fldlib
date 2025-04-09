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
// File      : FloatAffineInterface.h
// Description :
//   Definition of a class of affine relations.
//

#pragma once

/* for log */
#include <cmath>
#include "obj_interface/AffineTypesSize.h"

#include <iosfwd>
// #include <iostream>
#include <vector>
#include <functional>
#include <cassert>
#include <cstdint>

namespace NumericalDomains {

namespace DAffine {

class PathExplorer;
class DiagnosisReadStream;

}

namespace DAffineInterface {

class end {};
class nothing {};

class ExecutionPath;
class PathExplorer {
  private:
   typedef void* /* size_t */ AlignType;
   static const int UPathExplorerSizeInBytes = NumericalDomains::DAffineInterface::PathExplorerSize;
   static const int UPathExplorerSize = (UPathExplorerSizeInBytes + sizeof(AlignType)-1)/(sizeof(AlignType) - 1);
   AlignType content[UPathExplorerSize];
   friend class ExecutionPath;

  public:
   enum Mode { MRealAndImplementation, MOnlyReal, MOnlyImplementation };

   PathExplorer();
   PathExplorer(Mode mode);
   PathExplorer(const PathExplorer& source);
   PathExplorer(PathExplorer&& source);
   ~PathExplorer();
   PathExplorer& operator=(const PathExplorer& source);
   PathExplorer& operator=(PathExplorer&& source);

   Mode mode() const;
   bool isFinished(Mode mode);
   bool isFinished();
};

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
class TFloatZonotope;

class MergeBranches {
  private:
   typedef void* /* size_t */ AlignType;
   static const int UMergeBranchesSizeInBytes = NumericalDomains::DAffineInterface::MergeBranchesSize;
   static const int UMergeBranchesSize = (UMergeBranchesSizeInBytes + sizeof(AlignType)-1)/(sizeof(AlignType) - 1);
   AlignType content[UMergeBranchesSize];

  public:
   MergeBranches(const char* file, int line);

   template <class TypeIterator>
   struct TPacker {
      TypeIterator iter, end;
      TPacker(TypeIterator aiter, TypeIterator aend) : iter(aiter), end(aend) {}
   };

   template <class TypeIterator>
   static TPacker<TypeIterator> packer(TypeIterator iter, TypeIterator end)
      {  return TPacker<TypeIterator>(iter, end); }

   template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
   MergeBranches& operator<<(const TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>& value);
   template <class TypeIterator>
   MergeBranches& operator<<(TPacker<TypeIterator> packer)
      {  for (; packer.iter != packer.end; ++packer.iter)
            operator<<(*packer.iter);
         return *this;
      }
   bool operator<<(end);
   MergeBranches& operator<<(nothing) { return *this; }
};

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
tremoveHolder(T& save, AffineType) { save.removeHolder(); }

template <typename T, typename thasEquationHolder<typename T::InstrumentedAffineType>::type=0>
void
tsetHolder(T& source, T& save, AffineType) { source.setHolder(save); }

template <typename TypeIterator, class TypeSaveMemory>
class TPackedSaveMemory;

template <class T1, class TypeSaveMemory>
class TSaveMemory {
  public:
   T1 save;
   TypeSaveMemory next;

   TSaveMemory(const T1& saveArg, TypeSaveMemory nextArg)
      :  save(saveArg), next(nextArg) { tsetHolder(const_cast<T1&>(saveArg), save, AffineType()); }
   TSaveMemory(const TSaveMemory<T1, TypeSaveMemory>& source)
      :  save(std::move(const_cast<T1&>(source.save))), next(source.next)
      {  tremoveHolder(save, AffineType()); }
   TSaveMemory(TSaveMemory<T1, TypeSaveMemory>&& source)
      :  save(std::move(const_cast<T1&>(source.save))), next(source.next)
      {  tremoveHolder(save, AffineType()); }

   template <class T>
   TSaveMemory<T, TSaveMemory<T1, TypeSaveMemory> > operator<<(const T& t)
      {  return TSaveMemory<T, TSaveMemory<T1, TypeSaveMemory> >(t, *this); }
   template <typename TypeIterator>
   TPackedSaveMemory<TypeIterator, TSaveMemory<T1, TypeSaveMemory> > operator<<(MergeBranches::TPacker<TypeIterator> packer);
   TSaveMemory<T1, TypeSaveMemory>& operator<<(end) { return *this; }
   TSaveMemory<T1, TypeSaveMemory>& operator<<(nothing) { return *this; }
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
   std::vector<typename TypeIterator::value_type> save;
   TypeSaveMemory next;

   TPackedSaveMemory(TypeIterator iter, TypeIterator end, TypeSaveMemory nextArg)
      :  next(nextArg)
      {  int count = end - iter;
         save.reserve(count);
         for (; iter != end; ++iter) {
            save.push_back(*iter);
            tsetHolder(const_cast<typename TypeIterator::value_type&>(*iter), save.back(), AffineType());
         };
      }
   TPackedSaveMemory(const TPackedSaveMemory<TypeIterator, TypeSaveMemory>& source)
      :  save(source.save), next(source.next)
      {  for (auto& element : save)
            tremoveHolder(element, AffineType());
      }
   TPackedSaveMemory(TPackedSaveMemory<TypeIterator, TypeSaveMemory>&& source)
      :  save(std::move(source.save)), next(std::move(source.next))
      {  for (auto& element : save)
            tremoveHolder(element, AffineType());
      }

   template <class T>
   TSaveMemory<T, TSaveMemory<TypeIterator, TypeSaveMemory> > operator<<(T& t)
      {  return TSaveMemory<T, TSaveMemory<TypeIterator, TypeSaveMemory> >(t, *this); }
   template <class TypeIteratorArgument>
   TPackedSaveMemory<TypeIteratorArgument, TPackedSaveMemory<TypeIterator, TypeSaveMemory> >
      operator<<(MergeBranches::TPacker<TypeIteratorArgument> packer)
      {  return TPackedSaveMemory<TypeIteratorArgument, TPackedSaveMemory<TypeIterator, TypeSaveMemory> >
            (packer.iter, packer.end, *this);
      }
   TPackedSaveMemory<TypeIterator, TypeSaveMemory>& operator<<(end) { return *this; }
   TPackedSaveMemory<TypeIterator, TypeSaveMemory>& operator<<(nothing) { return *this; }

   TPackedSaveMemory<TypeIterator, TypeSaveMemory>& setCurrentResult(bool result)
      {  next.setCurrentResult(result); return *this; }
   
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

template <class T1, class TypeSaveMemory>
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

   template <class T>
   TSaveMemory<T, SaveMemory> operator<<(const T& t)
      {  return TSaveMemory<T, SaveMemory>(t, *this); }
   template <class TypeIterator>
   TPackedSaveMemory<TypeIterator, SaveMemory> operator<<(MergeBranches::TPacker<TypeIterator> packer)
      {  return TPackedSaveMemory<TypeIterator, SaveMemory>(packer.iter, packer.end, *this); }
   SaveMemory& operator<<(end) { return *this; }
   SaveMemory& operator<<(nothing) { return *this; }
   SaveMemory& setCurrentResult(bool result) { fResult = result; return *this; }
   bool getResult() const { return fResult; }
   bool operator>>(end)
      {  bool result = fResult;
         if (fResult)
            fResult = false;
         return result;
      }
   SaveMemory& operator>>(nothing) { return *this; }
};

template <typename TypeIterator, class TypeMergeMemory>
class TPackedMergeMemory;

template <class T1, class TypeMergeMemory>
class TMergeMemory {
  public:
   T1 merge;
   TypeMergeMemory next;

   TMergeMemory(const T1&, TypeMergeMemory nextArg) : merge(), next(nextArg) {}
   TMergeMemory(const TMergeMemory<T1, TypeMergeMemory>&) = default;
   TMergeMemory(TMergeMemory<T1, TypeMergeMemory>&&) = default;

   template <class T>
   TMergeMemory<T, TMergeMemory<T1, TypeMergeMemory> > operator>>(const T& t)
      {  return TMergeMemory<T, TMergeMemory<T1, TypeMergeMemory> >(t, *this); }
   template <typename TypeIterator>
   TPackedMergeMemory<TypeIterator, TMergeMemory<T1, TypeMergeMemory> > operator>>(MergeBranches::TPacker<TypeIterator> packer);
   TMergeMemory<T1, TypeMergeMemory>& operator>>(end) { return *this; }
   TMergeMemory<T1, TypeMergeMemory>& operator>>(nothing) { return *this; }
   TMergeMemory<T1, TypeMergeMemory>& setCurrentComplete(bool isComplete)
      {  next.setCurrentComplete(isComplete); return *this; }
   TypeMergeMemory& operator<<(T1& val)
      {  if (isComplete()) {
            if (val.optimizeValue()) {
               if (next.isFirst())
                  merge.recordFrom(std::move(val));
               else
                  merge.mergeWith(std::move(val));
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
         if (isComplete()) {
            if (val.optimizeValue()) {
               if (next.isFirst())
                  merge.recordFrom(std::move(val));
               else
                  merge.mergeWith(std::move(val));
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
   std::vector<typename TypeIterator::value_type> merge;
   TypeMergeMemory next;

   TPackedMergeMemory(TypeIterator iter, TypeIterator end, TypeMergeMemory nextArg)
      :  next(nextArg) {}
   TPackedMergeMemory(const TPackedMergeMemory<TypeIterator, TypeMergeMemory>&) = default;
   TPackedMergeMemory(TPackedMergeMemory<TypeIterator, TypeMergeMemory>&&) = default;

   template <class T>
   TMergeMemory<T, TPackedMergeMemory<TypeIterator, TypeMergeMemory> > operator>>(T& t)
      {  return TMergeMemory<T, TPackedMergeMemory<TypeIterator, TypeMergeMemory> >(t, *this); }
   template <class TypeIteratorArgument>
   TPackedMergeMemory<TypeIteratorArgument, TPackedMergeMemory<TypeIterator, TypeMergeMemory> >
      operator>>(MergeBranches::TPacker<TypeIteratorArgument> packer)
      {  return TPackedMergeMemory<TypeIteratorArgument, TPackedMergeMemory<TypeIterator, TypeMergeMemory> >
            (packer.iter, packer.end, *this);
      }
   TPackedMergeMemory<TypeIterator, TypeMergeMemory>& operator>>(end) { return *this; }
   TPackedMergeMemory<TypeIterator, TypeMergeMemory>& operator>>(nothing) { return *this; }
   TPackedMergeMemory<TypeIterator, TypeMergeMemory>& setCurrentComplete(bool isComplete)
      {  next.setCurrentComplete(isComplete); return *this; }
   TypeMergeMemory& operator<<(MergeBranches::TPacker<TypeIterator>&& packer)
      {  int count = packer.end - packer.iter;
         if (isComplete()) {
            auto iter = packer.iter;
            if (next.isFirst()) {
               assert(merge.size() == 0);
               merge.reserve(count);
               for (; iter != packer.end; ++iter) {
                  if (iter->optimizeValue()) {
                     typename TypeIterator::value_type val;
                     merge.push_back(val);
                     merge.back().recordFrom(std::move(*iter));
                  }
                  else
                     next.setCurrentComplete(false);
               };
            }
            else {
               for (int index = 0; index < count; ++index) {
                  if (iter->optimizeValue())
                     merge[index].mergeWith(std::move(*iter));
                  else
                     next.setCurrentComplete(false);
                  ++iter;
               }
            }
         };
         if (merge.size() > 0) {
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

template <class T1, class TypeMergeMemory>
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

   template <class T>
   TMergeMemory<T, MergeMemory> operator>>(const T& t)
      {  return TMergeMemory<T, MergeMemory>(t, *this); }
   template <typename TypeIterator>
   TPackedMergeMemory<TypeIterator, MergeMemory> operator>>(MergeBranches::TPacker<TypeIterator>&& packer)
      {  return TPackedMergeMemory<TypeIterator, MergeMemory>(packer.iter, packer.end, *this); }
   MergeMemory& operator>>(end) { return *this; }
   MergeMemory& operator>>(nothing) { return *this; }
   MergeMemory& setCurrentComplete(bool isComplete) { fComplete = isComplete; return *this; }
   bool isFirst() const { return fFirst; }
   bool isComplete() const { return fComplete; }
   bool operator<<(end)
      {  if (fComplete)
            fFirst = false;
         return true;
      }
   MergeMemory operator<<(nothing) { return *this; }
};

class ExecutionPath {
  public:
   static void splitBranches(const char* file, int line);
   static std::pair<const char*, int> querySplitInfo();

   bool hasMultipleBranches() const;
   static void setSupportAtomic();
   static void setSupportUnstableInLoop(bool value=true);
   static void setSupportBacktrace();
   static void setSupportVerbose();
   static void setSupportThreshold();
   static void setSupportFirstFollowFloat();
   static void setSupportPureZonotope();
   static void setTrackErrorOrigin();
   static void setSupportMapSymbols();
   static void setLimitNoiseSymbolsNumber(int limit);
   static void setSimplificationTriggerPercent(double percent);

   static void initializeGlobals(const char* fileSuffix);
   static void finalizeGlobals();
   static bool doesSupportUnstableInLoop();
   class Initialization {
     public:
      Initialization() {}
      void setSupportAtomic() { ExecutionPath::setSupportAtomic(); }
      void setSupportUnstableInLoop() { ExecutionPath::setSupportUnstableInLoop(); }
      void setSupportBacktrace() { ExecutionPath::setSupportBacktrace(); }
      void setSupportVerbose() { ExecutionPath::setSupportVerbose(); }
      void setSupportThreshold() { ExecutionPath::setSupportThreshold(); }
      void setSupportFirstFollowFloat() { ExecutionPath::setSupportFirstFollowFloat(); }
      void setSupportPureZonotope() { ExecutionPath::setSupportPureZonotope(); }
      void setTrackErrorOrigin() { ExecutionPath::setTrackErrorOrigin(); }
      void setSupportMapSymbols() { ExecutionPath::setSupportMapSymbols(); }
      void setLimitNoiseSymbolsNumber(int limit) { ExecutionPath::setLimitNoiseSymbolsNumber(limit); }
      void setResultFile(const char* fileSuffix) { initializeGlobals(fileSuffix); }
      void setSimplificationTriggerPercent(double percent) { ExecutionPath::setSimplificationTriggerPercent(percent); }
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
   static void flushOut();
   static void setSourceLine(const char* file, int line);
   static void writeCurrentPath(std::ostream& out);
   static DAffine::PathExplorer* getCurrentPathExplorer();
   static void setCurrentPathExplorer(PathExplorer* pathExplorer);
   static void setCurrentPathExplorer(DAffine::PathExplorer* pathExplorer);

   // float loop unstable
   static DAffine::DiagnosisReadStream* inputTraceFile();
   static const char* synchronisationFile();
   static int synchronisationLine();
   static bool doesFollowFlow();
   static void clearFollowFlow();
   static void setFollowFlow(bool doesFollowFlow, DAffine::DiagnosisReadStream* inputTraceFile,
         const char* synchronizationFile, int synchronizationLine);
   static void setFollowFlow();
   static PathExplorer::Mode queryMode(DAffine::PathExplorer* pathExplorer);
   // end of float loop unstable

   static void throwEmptyBranch(bool isUnstable);

  public:
   static void clearSynchronizationBranches();
};

typedef ExecutionPath BaseFloatAffine; // for splitBranches

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
class TFloatZonotope {
  private:
   typedef TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation> thisType;

   typedef void* /* size_t */ AlignType;
   static const int UFloatZonotopeSizeInBytes = NumericalDomains::DAffineInterface
      ::TFloatZonotopeSizeTraits<USizeMantissa, USizeExponent, TypeImplementation>::USize;
   static const int UFloatZonotopeSize = (UFloatZonotopeSizeInBytes + sizeof(AlignType)-1)/(sizeof(AlignType) - 1);
   AlignType content[UFloatZonotopeSize];

   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend class TFloatZonotope;
   friend class DAffineInterface::MergeBranches;

   int sfinite() const;
   int sisfinite() const;
   int sisnan() const;
   int sisinf() const;

  protected:
   enum RoundMode { RMNearest, RMLowest, RMHighest, RMZero };
   int32_t asInt(RoundMode mode) const;
   uint32_t asUnsigned(RoundMode mode) const;
   int64_t asLongInt(RoundMode mode) const;
   uint64_t asUnsignedLong(RoundMode mode) const;

   void initFrom(int64_t value);
   void initFrom(uint64_t value);
   void initFrom(TypeImplementation value);
   void initFrom(TypeImplementation min, TypeImplementation max);
   void initFrom(TypeImplementation min, TypeImplementation max,
         TypeImplementation errmin, TypeImplementation errmax);

  public:
   typedef DAffineInterface::MergeBranches MergeBranches;

   const char* queryDebugValue() const;
   const char* queryLightDebugValue() const;
   typedef ExecutionPath::Initialization Initialization;
   typedef ExecutionPath::anticipated_termination anticipated_termination;
   typedef DAffineInterface::end end;
   typedef DAffineInterface::nothing nothing;
   static void flushOut() { ExecutionPath::flushOut(); }

  public:
   typedef thisType InstrumentedAffineType;
   void setHolder(thisType& save);
   void removeHolder();

   struct ValueFromString {}; 

   TFloatZonotope();
   TFloatZonotope(const char* value, ValueFromString);
   template <typename T> TFloatZonotope(T value) requires floating_point_promotion<T, TypeImplementation>
      :  thisType()
      {  initFrom(value); }
   template <typename T> TFloatZonotope(T value) requires floating_point_nopromotion<T, TypeImplementation>
      :  thisType()
      {  
#ifndef FLOAT_ATOMIC
         DAffineInterface::TFloatZonotope<LDBL_MANT_DIG-1, (LDBL_MAX_EXP == (1 << (16-2))) ? 15 /* leading 1 bit */ : sizeof(long double)*8-LDBL_MANT_DIG,
            long double> receiver;
         receiver.initFrom((long double) value);
         thisType::operator=(receiver);
#else
         initFrom((TypeImplementation) value);
#endif
      }
   template <typename T> TFloatZonotope(T min, T max)
         requires floating_point_promotion<T, TypeImplementation>
      :  thisType()
      {  initFrom(min, max); }
   template <typename T> TFloatZonotope(T min, T max, T errmin, T errmax)
         requires floating_point_promotion<T, TypeImplementation>
      :  thisType()
      {  initFrom(min, max, errmin, errmax); }
   template <typename T> TFloatZonotope(T min, T max)
         requires floating_point_nopromotion<T, TypeImplementation>
      :  thisType()
      {  DAffineInterface::TFloatZonotope<LDBL_MANT_DIG-1, (LDBL_MAX_EXP == (1 << (16-2))) ? 15 /* leading 1 bit */ : sizeof(long double)*8-LDBL_MANT_DIG,
            long double> receiver;
         receiver.initFrom((long double) min, (long double) max);
         thisType::operator=(receiver);
      }
   template <typename T> TFloatZonotope(T min, T max, T errmin, T errmax)
         requires floating_point_nopromotion<T, TypeImplementation>
      :  thisType()
      {  DAffineInterface::TFloatZonotope<LDBL_MANT_DIG-1, (LDBL_MAX_EXP == (1 << (16-2))) ? 15 /* leading 1 bit */ : sizeof(long double)*8-LDBL_MANT_DIG,
            long double> receiver;
         receiver.initFrom((long double) min, (long double) max, (long double) errmin, (long double) errmax);
         thisType::operator=(receiver);
      }
   template <typename T> TFloatZonotope(T value) requires integral_signed_promotion<T, int64_t>
      :  thisType()
      {  initFrom((int64_t) value); }
   template <typename T> TFloatZonotope(T value) requires integral_unsigned_promotion<T, uint64_t>
      :  thisType()
      {  initFrom((uint64_t) value); }
   TFloatZonotope(const thisType& source);
   TFloatZonotope(thisType&& source);
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   TFloatZonotope(const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source);
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   TFloatZonotope(TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& source);
   ~TFloatZonotope();

   thisType& operator=(const thisType& source);
   thisType& operator=(thisType&& source);
   template<typename T> thisType& operator=(T source) requires std::floating_point<T> or std::integral<T>
      {  return operator=(thisType(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator=(const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source);
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator=(TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& source);

   void cloneShareParts() {}
   void recordFrom(const thisType& source);
   void mergeWith(const thisType& source);
   void recordFrom(thisType&& source);
   void mergeWith(thisType&& source);

   TypeImplementation asImplementation() const;
   void readImplementation(std::istream& in);
   void writeImplementation(std::ostream& out) const;
   friend std::ostream& operator<<(std::ostream& out, const thisType& value)
      {  value.writeImplementation(out); return out; }
   friend std::istream& operator>>(std::istream& in, thisType& value)
      {  value.readImplementation(in); return in; }

   void oppositeAssign();
   void inverseAssign();
   void plusAssign(const thisType& source);
   void plusAssign(thisType&& source);
   void minusAssign(const thisType& source);
   void minusAssign(thisType&& source);
   void multAssign(const thisType& source);
   void multAssign(thisType&& source);
   void divAssign(const thisType& source);
   void divAssign(thisType&& source);

   thisType operator++() { return (thisType&) thisType::operator+=(thisType(1)); }
   thisType operator++(int) { thisType result = *this; thisType::operator+=(thisType(1)); return result; }
   friend thisType operator+(const thisType& first) { return first; }
   friend thisType operator+(thisType&& first) { return first; }
   friend thisType operator-(const thisType& first)
      {  thisType result(first); result.oppositeAssign(); return result; }
   friend thisType operator-(thisType&& first)
      {  thisType result(std::forward<thisType>(first)); result.oppositeAssign(); return result; }

   thisType& operator+=(const thisType& source);
   thisType& operator+=(thisType&& source);
   thisType& operator-=(const thisType& source);
   thisType& operator-=(thisType&& source);
   thisType& operator*=(const thisType& source);
   thisType& operator*=(thisType&& source);
   thisType& operator/=(const thisType& source);
   thisType& operator/=(thisType&& source);
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator+=(const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      {  return operator+=(thisType(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator+=(TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& source)
      {  return operator+=(std::forward<thisType>(source)); }
   template<typename T> thisType& operator+=(T source) requires std::floating_point<T> || std::integral<T>
      {  return operator+=(thisType(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator-=(const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      {  return operator-=(thisType(source)); }
   template<typename T> thisType& operator-=(T source) requires std::floating_point<T> || std::integral<T>
      {  return operator-=(thisType(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator*=(const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      {  return operator*=(thisType(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator*=(TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& source)
      {  return operator*=(std::forward<thisType>(source)); }
   template<typename T> thisType& operator*=(T source) requires std::floating_point<T> || std::integral<T>
      {  return operator*=(thisType(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator/=(const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& source)
      {  return operator/=(thisType(source)); }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   thisType& operator/=(TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& source)
      {  return operator/=(std::forward<thisType>(source)); }
   template<typename T> thisType& operator/=(T source) requires std::floating_point<T> || std::integral<T>
      {  return operator/=(thisType(source)); }

   friend thisType operator+(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.plusAssign(second);
         return result;
      }
   friend thisType operator+(thisType&& first, const thisType& second)
      {  thisType result(std::forward<thisType>(first));
         result.plusAssign(second);
         return result;
      }
   friend thisType operator+(const thisType& first, thisType&& second)
      {  thisType result(first);
         result.plusAssign(std::forward<thisType>(second));
         return result;
      }
   friend thisType operator+(thisType&& first, thisType&& second)
      {  thisType result(std::forward<thisType>(first));
         result.plusAssign(std::forward<thisType>(second));
         return result;
      }
   template<typename T> friend auto operator+(const thisType& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeResult;
         TypeResult result(second);
         result.plusAssign((const TypeResult&) first);
         return result;
      }
   template<typename T> friend auto operator+(thisType&& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.plusAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto operator+(T first, const thisType& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeResult;
         TypeResult result(first);
         result.plusAssign((const TypeResult&) second);
         return result;
      }
   template<typename T> friend auto operator+(T first, thisType&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeResult;
         TypeResult result(first);
         result.plusAssign(std::forward<TypeResult>(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator+(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeResult;
         TypeResult result(first);
         result.plusAssign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator+(thisType&& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.plusAssign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator+(const thisType& first, TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeResult;
         TypeResult result(std::forward<TypeResult>(second));
         result.plusAssign((const TypeResult&) first);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator+(thisType&& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.plusAssign(std::forward<TypeResult>(second));
         return result;
      }

   friend thisType operator-(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.minusAssign(second);
         return result;
      }
   friend thisType operator-(thisType&& first, const thisType& second)
      {  thisType result(std::forward<thisType>(first));
         result.minusAssign(second);
         return result;
      }
   friend thisType operator-(const thisType& first, thisType&& second)
      {  thisType result(first);
         result.minusAssign(std::forward<thisType>(second));
         return result;
      }
   friend thisType operator-(thisType&& first, thisType&& second)
      {  thisType result(std::forward<thisType>(first));
         result.minusAssign(std::forward<thisType>(second));
         return result;
      }
   template<typename T> friend auto operator-(const thisType& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - T(0))> TypeResult;
         TypeResult result(first);
         result.minusAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto operator-(thisType&& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - T(0))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.minusAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto operator-(T first, const thisType& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) - TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) - TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) - TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) - TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) - TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) - TypeImplementation(0))> TypeResult;
         TypeResult result(first);
         result.minusAssign((const TypeResult&) second);
         return result;
      }
   template<typename T> friend auto operator-(T first, thisType&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) - TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) - TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) - TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) - TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) - TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) - TypeImplementation(0))> TypeResult;
         TypeResult result(first);
         result.minusAssign(std::forward<TypeResult>(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator-(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))> TypeResult;
         TypeResult result(first);
         result.minusAssign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator-(thisType&& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))> TypeResult;
         TypeResult result(std::move(first));
         result.minusAssign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator-(const thisType& first, TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))> TypeResult;
         TypeResult result(first);
         result.minusAssign(std::forward<TypeResult>(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator-(thisType&& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) - TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) - TypeImplementationArgument(0))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.minusAssign(std::forward<TypeResult>(second));
         return result;
      }

   friend thisType operator*(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.multAssign(second);
         return result;
      }
   friend thisType operator*(thisType&& first, const thisType& second)
      {  thisType result(std::forward<thisType>(first));
         result.multAssign(second);
         return result;
      }
   friend thisType operator*(const thisType& first, thisType&& second)
      {  thisType result(first);
         result.multAssign(std::forward<thisType>(second));
         return result;
      }
   friend thisType operator*(thisType&& first, thisType&& second)
      {  thisType result(std::forward<thisType>(first));
         result.multAssign(std::forward<thisType>(second));
         return result;
      }
   template<typename T> friend auto operator*(const thisType& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * T(0))> TypeResult;
         TypeResult result(second);
         result.multAssign((const TypeResult&) first);
         return result;
      }
   template<typename T> friend auto operator*(thisType&& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * T(0))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.multAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto operator*(T first, const thisType& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) * TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) * TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) * TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) * TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) * TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) * TypeImplementation(0))> TypeResult;
         TypeResult result(first);
         result.multAssign((const TypeResult&) second);
         return result;
      }
   template<typename T> friend auto operator*(T first, thisType&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) * TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) * TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) * TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) * TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) * TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) * TypeImplementation(0))> TypeResult;
         TypeResult result(first);
         result.multAssign(std::forward<TypeResult>(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator*(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))> TypeResult;
         TypeResult result(first);
         result.multAssign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator*(thisType&& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.multAssign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator*(const thisType& first, TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))> TypeResult;
         TypeResult result(std::forward<TypeResult>(second));
         result.multAssign((const TypeResult&) first);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator*(thisType&& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) * TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) * TypeImplementationArgument(0))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.multAssign(std::forward<TypeResult>(second));
         return result;
      }

   friend thisType operator/(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.divAssign(second);
         return result;
      }
   friend thisType operator/(thisType&& first, const thisType& second)
      {  thisType result(std::forward<thisType>(first));
         result.divAssign(second);
         return result;
      }
   friend thisType operator/(const thisType& first, thisType&& second)
      {  thisType result(first);
         result.divAssign(std::forward<thisType>(second));
         return result;
      }
   friend thisType operator/(thisType&& first, thisType&& second)
      {  thisType result(std::forward<thisType>(first));
         result.divAssign(std::forward<thisType>(second));
         return result;
      }
   template<typename T> friend auto operator/(const thisType& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / T(0))> TypeResult;
         TypeResult result(first);
         result.divAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto operator/(thisType&& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / T(0))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.divAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto operator/(T first, const thisType& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) / TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) / TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) / TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) / TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) / TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) / TypeImplementation(0))> TypeResult;
         TypeResult result(first);
         result.divAssign((const TypeResult&) second);
         return result;
      }
   template<typename T> friend auto operator/(T first, thisType&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) / TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) / TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) / TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) / TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) / TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) / TypeImplementation(0))> TypeResult;
         TypeResult result(first);
         result.divAssign(std::forward<TypeResult>(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator/(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))> TypeResult;
         TypeResult result(first);
         result.divAssign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator/(thisType&& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))> TypeResult;
         TypeResult result(std::move(first));
         result.divAssign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator/(const thisType& first, TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))> TypeResult;
         TypeResult result(first);
         result.divAssign(std::forward<TypeResult>(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto operator/(thisType&& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) / TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) / TypeImplementationArgument(0))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.divAssign(std::forward<TypeResult>(second));
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

   bool optimizeValue();
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
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeComparison;
         return TypeComparison::compareLess(first, TypeComparison(second));
      }
   template <typename T> friend bool operator<(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeComparison;
         return TypeComparison::compareLess(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator<(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
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
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeComparison;
         return TypeComparison::compareLessOrEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator<=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeComparison;
         return TypeComparison::compareLessOrEqual(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator<=(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
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
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeComparison;
         return TypeComparison::compareEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator==(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeComparison;
         return TypeComparison::compareEqual(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator==(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
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
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeComparison;
         return TypeComparison::compareDifferent(first, TypeComparison(second));
      }
   template <typename T> friend bool operator!=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeComparison;
         return TypeComparison::compareDifferent(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator!=(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
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
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(first, TypeComparison(second));
      }
   template <typename T> friend bool operator>=(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeComparison;
         return TypeComparison::compareGreaterOrEqual(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator>=(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
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
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> TypeComparison;
         return TypeComparison::compareGreater(first, TypeComparison(second));
      }
   template <typename T> friend bool operator>(T first, const thisType& second)
         requires std::floating_point<T> or std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> TypeComparison;
         return TypeComparison::compareGreater(TypeComparison(first), second);
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend bool operator>(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
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
   void powAssign(thisType&& value);
   void atan2Assign(const thisType& value);
   void atan2Assign(thisType&& value);

   void continuousFlow(std::function<void (thisType& val)> funAssign)
      {  // see float_diagnosis.h FLOAT_SPLIT_ALL FLOAT_MERGE_ALL
         bool oldSupportUnstableInLoop = ExecutionPath::doesSupportUnstableInLoop();
         ExecutionPath::setSupportUnstableInLoop();
         auto* oldPathExplorer = ExecutionPath::getCurrentPathExplorer();
         bool oldDoesFollow = ExecutionPath::doesFollowFlow();
         ExecutionPath::clearFollowFlow();
         auto* oldInputTraceFile = ExecutionPath::inputTraceFile();
         const char* oldSynchronisationFile = ExecutionPath::synchronisationFile();
         int oldSynchronisationLine = ExecutionPath::synchronisationLine();
         bool isCompleteFlow = true;
         PathExplorer pathExplorer(ExecutionPath::queryMode(oldPathExplorer));
         ExecutionPath::setCurrentPathExplorer(&pathExplorer);
         auto mergeMemory = MergeMemory() >> *this >> end();
         auto saveMemory = SaveMemory() << *this << end();
         const char* sourceFile = __FILE__;
         int sourceLine = __LINE__;
         auto oldSourceInfo = BaseFloatAffine::querySplitInfo();
         bool doesIterate;
         do {
            try {
               BaseFloatAffine::splitBranches(sourceFile, sourceLine);
               funAssign(*this);
               isCompleteFlow = MergeBranches(sourceFile, sourceLine) << *this << end();
            } 
            catch (typename thisType::anticipated_termination&) {
               isCompleteFlow = false;
               ExecutionPath::clearSynchronizationBranches();
            }
            catch (ExecutionPath::read_error& error) {
               if (const char* message = error.getMessage())
                  std::cerr << "error: " << message << std::endl;
               else
                  std::cerr << "error while reading input file!" << std::endl;
               isCompleteFlow = false;
               ExecutionPath::clearSynchronizationBranches();
            }
            ExecutionPath::setFollowFlow();
            doesIterate = mergeMemory.setCurrentComplete(isCompleteFlow) << *this << end();
            if (doesIterate)
               doesIterate = !(saveMemory.setCurrentResult(pathExplorer.isFinished(ExecutionPath::queryMode(oldPathExplorer))) >> *this >> end());
         } while (doesIterate);
         ExecutionPath::setFollowFlow(oldDoesFollow, oldInputTraceFile,
               oldSynchronisationFile, oldSynchronisationLine);
         ExecutionPath::setCurrentPathExplorer(oldPathExplorer);
         ExecutionPath::setSupportUnstableInLoop(oldSupportUnstableInLoop);
         BaseFloatAffine::splitBranches(oldSourceInfo.first, oldSourceInfo.second);
         if (mergeMemory.isFirst())
            ExecutionPath::throwEmptyBranch(true);
      }
   void continuousFlow(std::function<void (thisType& val, const thisType& arg)> funAssign, const thisType& anarg)
      {  // see float_diagnosis.h FLOAT_SPLIT_ALL FLOAT_MERGE_ALL
         bool oldSupportUnstableInLoop = ExecutionPath::doesSupportUnstableInLoop();
         ExecutionPath::setSupportUnstableInLoop();
         auto* oldPathExplorer = ExecutionPath::getCurrentPathExplorer();
         bool oldDoesFollow = ExecutionPath::doesFollowFlow();
         ExecutionPath::clearFollowFlow();
         auto* oldInputTraceFile = ExecutionPath::inputTraceFile();
         const char* oldSynchronisationFile = ExecutionPath::synchronisationFile();
         int oldSynchronisationLine = ExecutionPath::synchronisationLine();
         bool isCompleteFlow = true;
         PathExplorer pathExplorer(ExecutionPath::queryMode(oldPathExplorer));
         ExecutionPath::setCurrentPathExplorer(&pathExplorer);
         thisType& arg = const_cast<thisType&>(anarg);
         auto mergeMemory = MergeMemory() >> arg >> *this >> end();
         auto saveMemory = SaveMemory() << *this << arg << end();
         const char* sourceFile = __FILE__;
         int sourceLine = __LINE__;
         auto oldSourceInfo = BaseFloatAffine::querySplitInfo();
         bool doesIterate;
         do {
            try {
               BaseFloatAffine::splitBranches(sourceFile, sourceLine);
               funAssign(*this, anarg);
               isCompleteFlow = MergeBranches(sourceFile, sourceLine) << *this << arg << end();
            } 
            catch (typename thisType::anticipated_termination&) {
               isCompleteFlow = false;
               ExecutionPath::clearSynchronizationBranches();
            }
            catch (ExecutionPath::read_error& error) {
               if (const char* message = error.getMessage())
                  std::cerr << "error: " << message << std::endl;
               else
                  std::cerr << "error while reading input file!" << std::endl;
               isCompleteFlow = false;
               ExecutionPath::clearSynchronizationBranches();
            }
            ExecutionPath::setFollowFlow();
            doesIterate = mergeMemory.setCurrentComplete(isCompleteFlow) << *this << arg << end();
            if (doesIterate)
               doesIterate = !(saveMemory.setCurrentResult(pathExplorer.isFinished(ExecutionPath::queryMode(oldPathExplorer))) >> arg >> *this >> end());
         } while (doesIterate);
         ExecutionPath::setFollowFlow(oldDoesFollow, oldInputTraceFile,
               oldSynchronisationFile, oldSynchronisationLine);
         ExecutionPath::setCurrentPathExplorer(oldPathExplorer);
         ExecutionPath::setSupportUnstableInLoop(oldSupportUnstableInLoop);
         BaseFloatAffine::splitBranches(oldSourceInfo.first, oldSourceInfo.second);
         if (mergeMemory.isFirst())
            ExecutionPath::throwEmptyBranch(true);
      }
   void continuousFlow(std::function<void (thisType& val, const thisType& fstarg, const thisType& sndarg)> funAssign,
         const thisType& afstarg, const thisType& asndarg)
      {  // see float_diagnosis.h FLOAT_SPLIT_ALL FLOAT_MERGE_ALL
         bool oldSupportUnstableInLoop = ExecutionPath::doesSupportUnstableInLoop();
         ExecutionPath::setSupportUnstableInLoop();
         auto* oldPathExplorer = ExecutionPath::getCurrentPathExplorer();
         bool oldDoesFollow = ExecutionPath::doesFollowFlow();
         ExecutionPath::clearFollowFlow();
         auto* oldInputTraceFile = ExecutionPath::inputTraceFile();
         const char* oldSynchronisationFile = ExecutionPath::synchronisationFile();
         int oldSynchronisationLine = ExecutionPath::synchronisationLine();
         bool isCompleteFlow = true;
         PathExplorer pathExplorer(ExecutionPath::queryMode(oldPathExplorer));
         ExecutionPath::setCurrentPathExplorer(&pathExplorer);
         thisType& fstarg = const_cast<thisType&>(afstarg);
         thisType& sndarg = const_cast<thisType&>(asndarg);
         auto mergeMemory = MergeMemory() >> sndarg >> fstarg >> *this >> end();
         auto saveMemory = SaveMemory() << *this << fstarg << sndarg << end();
         const char* sourceFile = __FILE__;
         int sourceLine = __LINE__;
         auto oldSourceInfo = BaseFloatAffine::querySplitInfo();
         bool doesIterate;
         do {
            try {
               BaseFloatAffine::splitBranches(sourceFile, sourceLine);
               funAssign(*this, afstarg, asndarg);
               isCompleteFlow = MergeBranches(sourceFile, sourceLine) << *this << fstarg << sndarg << end();
            } 
            catch (typename thisType::anticipated_termination&) {
               isCompleteFlow = false;
               ExecutionPath::clearSynchronizationBranches();
            }
            catch (ExecutionPath::read_error& error) {
               if (const char* message = error.getMessage())
                  std::cerr << "error: " << message << std::endl;
               else
                  std::cerr << "error while reading input file!" << std::endl;
               isCompleteFlow = false;
               ExecutionPath::clearSynchronizationBranches();
            }
            ExecutionPath::setFollowFlow();
            doesIterate = mergeMemory.setCurrentComplete(isCompleteFlow) << *this << fstarg << sndarg << end();
            if (doesIterate)
               doesIterate = !(saveMemory.setCurrentResult(pathExplorer.isFinished(ExecutionPath::queryMode(oldPathExplorer))) >> sndarg >> fstarg >> *this >> end());
         } while (doesIterate);
         ExecutionPath::setFollowFlow(oldDoesFollow, oldInputTraceFile,
               oldSynchronisationFile, oldSynchronisationLine);
         ExecutionPath::setCurrentPathExplorer(oldPathExplorer);
         ExecutionPath::setSupportUnstableInLoop(oldSupportUnstableInLoop);
         BaseFloatAffine::splitBranches(oldSourceInfo.first, oldSourceInfo.second);
         if (mergeMemory.isFirst())
            ExecutionPath::throwEmptyBranch(true);
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
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> result(second);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ if (val > source) val = source; },
               first);
         return result;
      }
   template<typename T> friend auto min(T first, const thisType& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ if (val > source) val = source; },
               second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto min(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
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
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + T(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + T(0))> result(second);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ if (val < source) val = source; },
               first);
         return result;
      }
   template<typename T> friend auto max(T first, const thisType& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(T(0) + TypeImplementation(0))>::UBitSizeExponent,
            decltype(T(0) + TypeImplementation(0))> result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ if (val < source) val = source; },
               second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto max(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))>
      {  TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(TypeImplementation(0) + TypeImplementationArgument(0))>::UBitSizeExponent,
            decltype(TypeImplementation(0) + TypeImplementationArgument(0))> result(first);
         result.continuousFlow(
               [](thisType& val, const thisType& source){ if (val < source) val = source; },
               second);
         return result;
      }

   thisType median(const thisType& afst, const thisType& asnd) const
      {  auto result(*this), fst(afst), snd(asnd);
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
   thisType median(TypeImplementation afst, const thisType& asnd) const
      {  auto result(*this), snd(asnd);
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
   thisType median(const thisType& afst, TypeImplementation asnd) const
      {  auto result(*this), fst(afst);
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

   void lightPersist(const char* prefix) const;
   void persist(const char* prefix) const;

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
      {   thisType result(std::forward<thisType>(source)); result.cosAssign(); return result; }
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
      {  thisType result(source); result.logAssign(); result.divAssign(log(thisType(2.0))); return result; }
   friend thisType log2(thisType&& source)
      {  thisType result(std::forward<thisType>(source)); result.logAssign(); result.divAssign(log(thisType(2.0))); return result; }
   friend thisType exp2(const thisType& source)
      {  thisType result = 2.0; result.powAssign(source); return result; }
   friend thisType exp2(thisType&& source)
      {  thisType result = 2.0; result.powAssign(std::forward<thisType>(source)); return result; }
   friend thisType log10(const thisType& source)
      {  thisType result(source); result.log10Assign(); return result; }
   friend thisType log10(thisType&& source)
      {  thisType result(std::forward<thisType>(source)); result.log10Assign(); return result; }

   friend thisType pow(const thisType& source, const thisType& value)
      {  thisType result(source); result.powAssign(value); return result; }
   friend thisType pow(const thisType& source, thisType&& value)
      {  thisType result(source); result.powAssign(std::forward<thisType>(value)); return result; }
   friend thisType pow(thisType&& source, const thisType& value)
      {  thisType result(std::forward<thisType>(source)); result.powAssign(value); return result; }
   friend thisType pow(thisType&& source, thisType&& value)
      {  thisType result(std::forward<thisType>(source)); result.powAssign(std::forward<thisType>(value)); return result; }
   template<typename T> friend auto pow(const thisType& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto pow(thisType&& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto pow(T first, const thisType& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(pow(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(pow(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign((const TypeResult&) second);
         return result;
      }
   template<typename T> friend auto pow(T first, thisType&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(pow(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(pow(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(std::forward<TypeResult>(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto pow(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto pow(thisType&& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.powAssign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto pow(const thisType& first, TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(std::forward<TypeResult>(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto pow(thisType&& first, TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(pow(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.powAssign(std::forward<TypeResult>(second));
         return result;
      }

   friend thisType powf(const thisType& source, const thisType& value)
      {  thisType result(source); result.powAssign(value); return result; }
   friend thisType powf(const thisType& source, thisType&& value)
      {  thisType result(source); result.powAssign(std::forward<thisType>(value)); return result; }
   friend thisType powf(thisType&& source, const thisType& value)
      {  thisType result(std::forward<thisType>(source)); result.powAssign(value); return result; }
   friend thisType powf(thisType&& source, thisType&& value)
      {  thisType result(std::forward<thisType>(source)); result.powAssign(std::forward<thisType>(value)); return result; }
   template<typename T> friend auto powf(const thisType& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto powf(thisType&& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.powAssign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto powf(T first, const thisType& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(powf(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(powf(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign((const TypeResult&) second);
         return result;
      }
   template<typename T> friend auto powf(T first, thisType&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(powf(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(powf(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(std::forward<TypeResult>(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto powf(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto powf(thisType&& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.powAssign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto powf(const thisType& first, TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.powAssign(std::forward<TypeResult>(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto powf(thisType&& first, TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(powf(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.powAssign(std::forward<TypeResult>(second));
         return result;
      }

   friend thisType atan2(const thisType& source, const thisType& value)
      {  thisType result(source); result.atan2Assign(value); return result; }
   friend thisType atan2(const thisType& source, thisType&& value)
      {  thisType result(source); result.atan2Assign(std::forward<thisType>(value)); return result; }
   friend thisType atan2(thisType&& source, const thisType& value)
      {  thisType result(std::forward<thisType>(source)); result.atan2Assign(value); return result; }
   friend thisType atan2(thisType&& source, thisType&& value)
      {  thisType result(std::forward<thisType>(source)); result.atan2Assign(std::forward<thisType>(value)); return result; }
   template<typename T> friend auto atan2(const thisType& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto atan2(thisType&& first, T second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), T(0)))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.atan2Assign(TypeResult(second));
         return result;
      }
   template<typename T> friend auto atan2(T first, const thisType& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(atan2(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(atan2(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign((const TypeResult&) second);
         return result;
      }
   template<typename T> friend auto atan2(T first, thisType&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(atan2(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(atan2(T(0), TypeImplementation(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(std::forward<TypeResult>(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto atan2(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto atan2(thisType&& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.atan2Assign((const TypeResult&) second);
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto atan2(const thisType& first, TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(first);
         result.atan2Assign(std::forward<TypeResult>(second));
         return result;
      }
   template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
   friend auto atan2(thisType&& first, TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(atan2(TypeImplementation(0), TypeImplementationArgument(0)))> TypeResult;
         TypeResult result(std::forward<TypeResult>(first));
         result.atan2Assign(std::forward<TypeResult>(second));
         return result;
      }

   friend thisType floor(const thisType& afst)
      {  thisType result, fst(afst); // afst should gain back its original value
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMLowest)); },
               fst);
         return result;
      }
   friend thisType floor(thisType&& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMLowest)); },
               fst);
         return result;
      }
   friend thisType ceil(const thisType& afst)
      {  thisType result, fst(afst);
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMHighest)); },
               fst);
         return result;
      }
   friend thisType ceil(thisType&& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMHighest)); },
               fst);
         return result;
      }
   friend thisType trunc(const thisType& afst)
      {  thisType result, fst(afst);
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMZero)); },
               fst);
         return result;
      }
   friend thisType trunc(thisType&& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMZero)); },
               fst);
         return result;
      }
   friend thisType round(const thisType& afst)
      {  thisType result, fst(afst);
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMNearest)); },
               fst);
         return result;
      }
   friend thisType round(thisType&& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMNearest)); },
               fst);
         return result;
      }
   friend thisType rint(const thisType& afst)
      {  thisType result, fst(afst);
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMNearest)); },
               fst);
         return result;
      }
   friend thisType rint(thisType&& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMNearest)); },
               fst);
         return result;
      }
   friend thisType rintf(const thisType& afst)
      {  thisType result, fst(afst);
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMNearest)); },
               fst);
         return result;
      }
   friend thisType rintf(thisType&& fst)
      {  thisType result;
         result.continuousFlow(
               [](thisType& result, const thisType& fst)
                  {  result = thisType(fst.asInt(RMNearest)); },
               fst);
         return result;
      }
   friend thisType dis_floor(const thisType& fst)
      {  return thisType(fst.asInt(RMLowest)); }
   friend thisType dis_ceil(const thisType& fst)
      {  return thisType(fst.asInt(RMHighest)); }
   friend thisType dis_trunc(const thisType& fst)
      {  return thisType(fst.asInt(RMZero)); }
   friend thisType dis_round(const thisType& fst)
      {  return thisType(fst.asInt(RMNearest)); }
   friend thisType dis_rint(const thisType& fst)
      {  return thisType(fst.asInt(RMNearest /* fegetround */)); }
   friend thisType dis_rintf(const thisType& fst)
      {  return thisType(fst.asInt(RMNearest /* fegetround */)); }

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
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), T(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), T(0)))>::UBitSizeMantissa,
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
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeExponent,
            decltype(fmod(T(0), TypeImplementation(0)))>
         requires std::floating_point<T> || std::integral<T>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(fmod(T(0), TypeImplementation(0)))>::UBitSizeMantissa,
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
   friend auto fmod(const thisType& first, const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& second)
      -> TFloatZonotope<
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
            DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeExponent,
            decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>
      {  typedef TFloatZonotope<DFloatDigitsHelper::TFloatDigits<decltype(fmod(TypeImplementation(0), TypeImplementationArgument(0)))>::UBitSizeMantissa,
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

} // end of namespace DAffineInterface

typedef DAffineInterface::TFloatZonotope<23, 8, float> FloatZonotope;
typedef DAffineInterface::TFloatZonotope<52, 11, double> DoubleZonotope;
typedef DAffineInterface::TFloatZonotope<LDBL_MANT_DIG-1,
      (LDBL_MAX_EXP == (1 << (16-2))) ? 15 /* leading 1 bit */
                                      : sizeof(long double)*8-LDBL_MANT_DIG,
      long double> LongDoubleZonotope;

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

