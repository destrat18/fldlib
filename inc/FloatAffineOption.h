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
// File      : FloatAffineOption.h
// Description :
//   Definition of an optional class of affine relations with conditional
//     relationships between discrete int and continuous float domains.
//

#pragma once

#include "FloatAffine.h"
#include <memory>

struct NotYetImplemented {};

namespace NumericalDomains {

template <typename T>
struct IntegralOrFloatingPointType {};

template <typename T> requires std::integral<T> or std::floating_point<T>
struct IntegralOrFloatingPointType<T> {
  public:
   typedef T base_type;
};

template <typename T>
struct IntegralType {};

template <typename T> requires std::integral<T>
struct IntegralType<T> {
  public:
   typedef T base_type;
};

template <typename T>
struct FloatingPointType {};

template <typename T> requires std::floating_point<T>
struct FloatingPointType<T> {
  public:
   typedef T base_type;
};

template <typename T1, typename T2>
concept same_as_without_references = std::same_as<std::remove_cvref_t<T1>, std::remove_cvref_t<T2>>;

template<typename T>               
concept has_field_isBranch = requires { 
    T::isBranch;                       
};

template <typename T>
concept enhanced_floating_point = not has_field_isBranch<T> and requires(T a) {
   { sqrt(a) } -> same_as_without_references<T>;
};

template <typename T>
class EFloatingPointType {
};

template <typename T> requires enhanced_floating_point<T>
struct EFloatingPointType<T> {
  public:
   typedef T base_type;
};

template <typename T>
concept is_not_void_type = !std::is_void_v<T>;

template <typename T>
concept indexed_container = requires(const T& a, std::size_t sz) {
   { a[sz] } -> is_not_void_type;
};

template <typename T>
concept integral_indexed_container = requires(const T& a, std::size_t sz) {
   { a[sz] } -> std::integral;
};

template <typename T>
concept enhanced_floating_point_container = requires(const T& a, std::size_t sz) {
   { a[sz] } -> enhanced_floating_point;
};

template <typename T>
concept integral_indexed_double_container = requires(const T& a, std::size_t iz, std::size_t sz) {
   { a[iz][sz] } -> std::integral;
};

template <typename T>
concept enhanced_floating_point_double_container = requires(const T& a, std::size_t iz, std::size_t sz) {
   { a[iz][sz] } -> enhanced_floating_point;
};

class MergedExecutionPath {
  public:
   struct BranchInformation {
      DAffine::Equation constraint;
      enum ConstraintComparisonResult
         {  CCRStrictNegative, CCRNegativeOrZero, CCREqualsZero, CCRDifferentZero,
            CCRPositiveOrZero, CCRStrictPositive
         } constraintCompareToZero;
      std::string origin;
   };
  private:
   int uNumberOfMergedPatch = 0;
   std::vector<BranchInformation> branchDictionary;

  public:
   MergedExecutionPath() = default;
};

template <typename DoubleType> requires std::floating_point<DoubleType>
class TFldlibZonotopeOption;
template <typename IntegerType> requires std::integral<IntegerType>
class TFldlibIntegerBranchOption;
template <typename FloatingType> requires enhanced_floating_point<FloatingType>
class TFldlibBaseFloatingBranchOption;
template <typename FloatingType> requires enhanced_floating_point<FloatingType>
class TFldlibFloatingBranchOption;

class FldlibBase : public DAffine::ExecutionPath {
  public:
   static int numberOfBranches;
   static int splitBranchIdentifier;
   static bool active;

  public:
   template<typename DoubleType>
   class TFloatZonotope {
   };

   static bool mergePath(std::vector<std::pair<int, bool>>& path, const std::vector<std::pair<int, bool>>& source)
      {  size_t thisIndex = 0, sourceIndex = 0;
         while (thisIndex < path.size() && sourceIndex < source.size()) {
            if (path[thisIndex].first < source[sourceIndex].first)
               ++thisIndex;
            else if (path[thisIndex].first > source[sourceIndex].first) {
               path.insert(path.begin() + thisIndex, source[sourceIndex]);
               ++sourceIndex;
            }
            else { // path[thisIndex].first == source[sourceIndex].first
               if (path[thisIndex].second == source[sourceIndex].second) {
                  ++thisIndex;
                  ++sourceIndex;
               }
               else
                  return false;
            }
         }
         return true;
      }
   class BranchCursor {
     protected:
      std::vector<std::pair<int, bool>> path;

     public:
      BranchCursor() = default;
      BranchCursor(const BranchCursor&) = default;
      BranchCursor(BranchCursor&&) = default;
      BranchCursor& operator=(const BranchCursor&) = default;
      BranchCursor& operator=(BranchCursor&&) = default;
      const std::vector<std::pair<int, bool>>& getPathAt() const { return path; }
   };

   template<typename ValueType>
   class TValueWithBranches { // : public PNT::SharedPointer<DiscreteSymbol>
     private:
      typedef TValueWithBranches<ValueType> thisType;
      int uMergeBranchIndex;
      ValueType uThenValue;
      std::shared_ptr<thisType> spibThenBranch;
      ValueType uElseValue;
      std::shared_ptr<thisType> spibElseBranch;
      // invariant uMergeBranchIndex < all the uMergeBranchIndex in spibThenBranch and spibElseBranch

      template <typename T> friend class TValueWithBranches;

     public:
      TValueWithBranches(int mergeBranchIndex, const ValueType& thenValue,
            const std::shared_ptr<thisType>& thenBranch, const ValueType& elseValue,
            const std::shared_ptr<thisType>& elseBranch)
         :  uMergeBranchIndex(mergeBranchIndex), uThenValue(thenValue), spibThenBranch(thenBranch),
            uElseValue(elseValue), spibElseBranch(elseBranch) {}
      TValueWithBranches(const thisType& source) = default;
      TValueWithBranches(thisType&& source) = default;
      TValueWithBranches<ValueType>& operator=(const thisType& source) = default;
      TValueWithBranches<ValueType>& operator=(thisType&& source) = default;
      template <typename T> TValueWithBranches(const TValueWithBranches<T>& source)
         :  uMergeBranchIndex(source.uMergeBranchIndex), uThenValue(source.uThenValue),
            spibThenBranch(source.spibThenBranch.get() ? new thisType(*source.spibThenBranch) : nullptr),
            uElseValue(source.uElseValue),
            spibElseBranch(source.spibElseBranch.get() ? new thisType(*source.spibElseBranch) : nullptr) {}
      template <typename T> TValueWithBranches(TValueWithBranches<T>&& source)
         :  uMergeBranchIndex(source.uMergeBranchIndex), uThenValue(source.uThenValue),
            spibThenBranch(source.spibThenBranch.get() ? new thisType(*source.spibThenBranch) : nullptr),
            uElseValue(source.uElseValue),
            spibElseBranch(source.spibElseBranch.get() ? new thisType(*source.spibElseBranch) : nullptr) {}
      void conditionalAssigns(std::vector<std::pair<int, bool>>::const_iterator pathBegin,
            std::vector<std::pair<int, bool>>::const_iterator pathEnd, const ValueType& sourceValue);
      void conditionalAssigns(std::vector<std::pair<int, bool>>::const_iterator pathBegin,
            std::vector<std::pair<int, bool>>::const_iterator pathEnd, const thisType& source);
      void applyAssign(std::function<void (ValueType&)> function);
      template <typename T>
      void applyAssign(const TValueWithBranches<T>& source,
            std::function<void (ValueType&, const T&)> function); /* requires std::integral<T> */
      void apply(const std::function<void (const ValueType&)>& function) const;
      template <typename T>
      void apply(const TValueWithBranches<T>& source,
            const std::function<void (const ValueType&, const T&)>& function) const; /* requires std::integral<T> */
      static void applyBranchAssign(int mergeBranchIndex, ValueType& thisValue,
            std::shared_ptr<TValueWithBranches<ValueType>>& thisBranch,
            const std::function<void (ValueType&,
               std::shared_ptr<TValueWithBranches<ValueType>>&)>& function);
      static void applyBranchAssign(int mergeBranchIndex, ValueType& thisValue,
            std::shared_ptr<TValueWithBranches<ValueType>>& thisBranch, ValueType& sourceValue,
            std::shared_ptr<TValueWithBranches<ValueType>>& sourceBranch,
            const std::function<void (ValueType&,
               std::shared_ptr<TValueWithBranches<ValueType>>&, ValueType&,
               std::shared_ptr<TValueWithBranches<ValueType>>&)>& function);
      struct BranchConstraint {
         enum ConstraintResult { CRStable, CRSimplifyWithConstant, CRSimplifyWithBranch, CRRemove }
            constraintResult = CRStable;
         std::shared_ptr<TValueWithBranches<ValueType>> simplifiedResult;
         ValueType constantResult = 0; 

         BranchConstraint() = default;
         explicit BranchConstraint(ConstraintResult aconstraintResult) : constraintResult(aconstraintResult) {}
         BranchConstraint(const BranchConstraint&) = default;
         BranchConstraint(BranchConstraint&&) = default;
         BranchConstraint(const std::shared_ptr<TValueWithBranches<ValueType>>& asimplifiedResult)
            :  constraintResult(CRSimplifyWithBranch), simplifiedResult(asimplifiedResult) {}
         explicit BranchConstraint(ValueType aconstantResult)
            :  constraintResult(CRSimplifyWithConstant), constantResult(aconstantResult) {}
         BranchConstraint& operator=(const BranchConstraint&) = default;
         BranchConstraint& operator=(BranchConstraint&&) = default;

         void mergeWith(const BranchConstraint& source, int mergeBranchIndex)
            {  if (source.constraintResult == CRRemove || constraintResult == CRStable)
                  return;
               if (constraintResult == CRRemove || source.constraintResult == CRStable) {
                  operator=(source);
                  return;
               }
               if (constraintResult == CRSimplifyWithConstant && source.constraintResult == CRSimplifyWithConstant) {
                  if (constantResult == source.constantResult)
                     return;
               }
               constraintResult = CRSimplifyWithBranch;
               simplifiedResult.reset(new thisType(mergeBranchIndex,
                     constantResult, simplifiedResult,
                     source.constantResult, source.simplifiedResult));
            }
      };
      BranchConstraint mergeBranchConstraint(BranchConstraint& branchConstraintThen,
            BranchConstraint& branchConstraintElse) const;
      BranchConstraint constraint(const std::function<bool (const ValueType&)>& function) const;
      template <typename T>
      std::pair<BranchConstraint, typename TValueWithBranches<T>::BranchConstraint>
         constraint(const TValueWithBranches<T>& source,
            const std::function<bool (const ValueType&, const T&)>& function); /* requires std::integral<T> */
      int getMergeBranchIndex() const { return uMergeBranchIndex; }
      ValueType& getSThenValue() { return uThenValue; }
      ValueType& getSElseValue() { return uThenValue; }
      std::shared_ptr<thisType>& getSThenBranch() { return spibThenBranch; }
      std::shared_ptr<thisType>& getSElseBranch() { return spibElseBranch; }
   };

   template<typename IntegerType> requires std::integral<IntegerType>
   class TIntegerWithBranches : public TValueWithBranches<IntegerType> {
     private:
      typedef TValueWithBranches<IntegerType> inherited;
      typedef TIntegerWithBranches<IntegerType> thisType;

     public:
      TIntegerWithBranches(int mergeBranchIndex, const IntegerType& thenValue,
            const std::shared_ptr<thisType>& thenBranch, const IntegerType& elseValue,
            const std::shared_ptr<thisType>& elseBranch)
         :  inherited(mergeBranchIndex, thenValue, thenBranch, elseValue, elseBranch) {}
      TIntegerWithBranches(const thisType& source) = default;
      TIntegerWithBranches(thisType&& source) = default;
      template <typename T> TIntegerWithBranches(const TIntegerWithBranches<T>& source) requires std::integral<T>
         :  inherited(source) {}
      template <typename T> TIntegerWithBranches(TIntegerWithBranches<T>&& source) requires std::integral<T>
         :  inherited(std::move(source)) {}
      thisType& operator=(const thisType& source) = default;
      thisType& operator=(thisType&& source) = default;

      void applyAssign(const std::function<void (IntegerType&)>& function)
         {  inherited::applyAssign(function); }
      void apply(const std::function<void (const IntegerType&)>& function) const
         {  inherited::apply(function); }
      template <typename T>
      void applyAssign(const TValueWithBranches<T>& source,
            const std::function<void (IntegerType&, const T&)>& function) requires std::integral<T>
         {  inherited::applyAssign(source, function); }
      template <typename T>
      void apply(const TValueWithBranches<T>& source,
            const std::function<void (const IntegerType&, const T&)>& function) const requires std::integral<T>
         {  inherited::apply(source, function); }
      typename inherited::BranchConstraint
         constraint(const std::function<bool (const IntegerType&)>& function) const
         {  return inherited::constraint(function); }
      template <typename T>
      std::pair<typename inherited::BranchConstraint, typename TValueWithBranches<T>::BranchConstraint>
         constraint(const TValueWithBranches<T>& source,
            const std::function<bool (const IntegerType&, const T&)>& function) requires std::integral<T>
         {  return inherited::constraint(source, function); }
   };

   template<typename FloatingType> requires enhanced_floating_point<FloatingType>
   class TFloatingWithBranches
      : public TValueWithBranches<FloatingType> {
     private:
      typedef FloatingType ValueType;
      typedef TValueWithBranches<FloatingType> inherited;
      typedef TFloatingWithBranches<FloatingType> thisType;

     public:
      TFloatingWithBranches(int mergeBranchIndex, const ValueType& thenValue,
            const std::shared_ptr<thisType>& thenBranch, const ValueType& elseValue,
            const std::shared_ptr<thisType>& elseBranch)
         :  inherited(mergeBranchIndex, thenValue, thenBranch, elseValue, elseBranch) {}
      TFloatingWithBranches(const thisType& source) = default;
      TFloatingWithBranches(thisType&& source) = default;
      template <typename T> TFloatingWithBranches(const TFloatingWithBranches<T>& source) requires enhanced_floating_point<T>
         :  inherited(source) {}
      template <typename T> TFloatingWithBranches(TFloatingWithBranches<T>&& source) requires enhanced_floating_point<T>
         :  inherited(std::move(source)) {}
      thisType& operator=(const thisType& source) = default;
      thisType& operator=(thisType&& source) = default;

      void applyAssign(const std::function<void (FloatingType&)>& function)
         {  inherited::applyAssign(function); }
      void apply(const std::function<void (const FloatingType&)>& function) const
         {  inherited::apply(function); }
      void applyAssign(const TValueWithBranches<FloatingType>& source,
            const std::function<void (FloatingType&, const FloatingType&)>& function)
         {  inherited::applyAssign(source, function); }
      template <typename T>
      void applyAssign(const TValueWithBranches<T>& source,
            const std::function<void (FloatingType&, const T&)>& function) requires enhanced_floating_point<T>
         {  inherited::applyAssign(source, function); }
      template <typename T>
      void apply(const TValueWithBranches<T>& source,
            const std::function<void (const FloatingType&, const T&)>& function) const requires enhanced_floating_point<T>
         {  inherited::apply(source, function); }
      typename inherited::BranchConstraint
         constraint(const std::function<bool (const FloatingType&)>& function) const
         {  return inherited::constraint(function); }
      template <typename T>
      std::pair<typename inherited::BranchConstraint, typename TValueWithBranches<T>::BranchConstraint>
         constraint(const TValueWithBranches<T>& source,
            const std::function<bool (const FloatingType&, const T&)>& function) requires enhanced_floating_point<T>
         {  return inherited::constraint(source, function); }
   };

   enum BasicComparisonResult { BCRLess, BCREqual, BCRGreater, BCREnd };
   enum FullComparisonResult
      {  FCRUndefined = 0x0,
         FCRLess = 1 << BCRLess, FCREqual = 1 << BCREqual, FCRGreater = 1 << BCRGreater,
         FCRLessOrEqual = FCRLess | FCREqual, FCRDifferent = FCRLess | FCRGreater,
         FCRGreaterOrEqual = FCRGreater | FCREqual, FCRAll = ~(~0 << BCREnd)
      };

   static MergedExecutionPath mergedExecutionPath;

   class MergeBranches : public DoubleZonotope::MergeBranches {
     private:
      typedef typename DoubleZonotope::MergeBranches inherited;

     public:
      MergeBranches(const char* file, int line) : inherited(file, line) {}

      template <typename DoubleType>
      MergeBranches& operator<<(TFldlibZonotopeOption<DoubleType>& value)
         {  inherited::pushForMerge(value);
            return *this;
         }
      template <typename IntegerType>
      MergeBranches& operator<<(TFldlibIntegerBranchOption<IntegerType>& value)
         {  inherited::pushForMerge(value);
            return *this;
         }
      template <typename FloatingType>
      MergeBranches& operator<<(TFldlibFloatingBranchOption<FloatingType>& value)
         {  inherited::pushForMerge(value);
            return *this;
         }
      template <class TypeIterator>
      MergeBranches& operator<<(TPacker<TypeIterator> packer)
         {  for (; packer.iter != packer.end; ++packer.iter)
               operator<<(*packer.iter);
            return *this;
         }
      bool operator<<(end) { return inherited::operator<<(end()); }
      MergeBranches& operator<<(nothing) { return *this; }
   };
   typedef NumericalDomains::DAffine::MergeMemory MergeMemory;
   typedef NumericalDomains::DAffine::SaveMemory SaveMemory;
};

template<>
class FldlibBase::TFloatZonotope<float> : public FloatZonotope {
  public:
   TFloatZonotope() = default;
   TFloatZonotope(const TFloatZonotope<float>&) = default;
   TFloatZonotope(TFloatZonotope<float>&&) = default;
   template <typename T> TFloatZonotope(T source)
      requires std::floating_point<T> || std::integral<T>
      :  FloatZonotope(source) {}
   TFloatZonotope(const char* valueString, FloatZonotope::ValueFromString traits) : FloatZonotope(valueString, traits) {}
   TFloatZonotope(float min, float max)
      :  FloatZonotope(min, max) {}
   TFloatZonotope(float min, float max, float minErr, float maxErr)
      :  FloatZonotope(min, max, minErr, maxErr) {}
   template <typename T> TFloatZonotope(const TFloatZonotope<T>& source)
      requires std::floating_point<T>
      :  FloatZonotope(source) {}
   template <typename T> TFloatZonotope(TFloatZonotope<T>&& source)
      requires std::floating_point<T>
      :  FloatZonotope(std::move(source)) {}
   TFloatZonotope(const FloatZonotope& source) : FloatZonotope(source) {}
   TFloatZonotope(FloatZonotope&& source) : FloatZonotope(std::move(source)) {}
   TFloatZonotope<float>& operator=(const TFloatZonotope<float>&) = default;
   TFloatZonotope<float>& operator=(TFloatZonotope<float>&&) = default;
   TFloatZonotope<float>& operator=(const FloatZonotope& source)
      {  return (TFloatZonotope<float>&) FloatZonotope::operator=(source); }
   TFloatZonotope<float>& operator=(FloatZonotope&& source)
      {  return (TFloatZonotope<float>&) FloatZonotope::operator=(std::move(source)); }

   template <typename T> TFloatZonotope<float>& operator=(T source)
      requires std::floating_point<T> || std::integral<T>
      {  return (TFloatZonotope<float>&) FloatZonotope::operator=(source); }
   template <typename T> TFloatZonotope<float>& operator=(const TFloatZonotope<T>& source)
      requires std::floating_point<T>
      {  return (TFloatZonotope<float>&) FloatZonotope::operator=(source); }
   template <typename T> TFloatZonotope<float>& operator=(TFloatZonotope<T>&& source)
      requires std::floating_point<T>
      {  return (TFloatZonotope<float>&) FloatZonotope::operator=(std::move(source)); }
};

template<>
class FldlibBase::TFloatZonotope<double> : public DoubleZonotope {
  public:
   TFloatZonotope() = default;
   TFloatZonotope(const TFloatZonotope<double>&) = default;
   TFloatZonotope(TFloatZonotope<double>&&) = default;
   template <typename T> TFloatZonotope(T source)
      requires std::floating_point<T> || std::integral<T>
      :  DoubleZonotope(source) {}
   TFloatZonotope(const char* valueString, DoubleZonotope::ValueFromString traits) : DoubleZonotope(valueString, traits) {}
   TFloatZonotope(double min, double max)
      :  DoubleZonotope(min, max) {}
   TFloatZonotope(double min, double max, double minErr, double maxErr)
      :  DoubleZonotope(min, max, minErr, maxErr) {}
   template <typename T> TFloatZonotope(const TFloatZonotope<T>& source)
      requires std::floating_point<T>
      :  DoubleZonotope(source) {}
   template <typename T> TFloatZonotope(TFloatZonotope<T>&& source)
      requires std::floating_point<T>
      :  DoubleZonotope(std::move(source)) {}
   TFloatZonotope(const DoubleZonotope& source) : DoubleZonotope(source) {}
   TFloatZonotope(DoubleZonotope&& source) : DoubleZonotope(std::move(source)) {}
   TFloatZonotope<double>& operator=(const TFloatZonotope<double>&) = default;
   TFloatZonotope<double>& operator=(TFloatZonotope<double>&&) = default;
   TFloatZonotope<double>& operator=(const DoubleZonotope& source)
      {  return (TFloatZonotope<double>&) DoubleZonotope::operator=(source); }
   TFloatZonotope<double>& operator=(DoubleZonotope&& source)
      {  return (TFloatZonotope<double>&) DoubleZonotope::operator=(std::move(source)); }

   template <typename T> TFloatZonotope<double>& operator=(T source)
      requires std::floating_point<T> || std::integral<T>
      {  return (TFloatZonotope<double>&) DoubleZonotope::operator=(source); }
   template <typename T> TFloatZonotope<double>& operator=(const TFloatZonotope<T>& source)
      requires std::floating_point<T>
      {  return (TFloatZonotope<double>&) DoubleZonotope::operator=(source); }
   template <typename T> TFloatZonotope<double>& operator=(TFloatZonotope<T>&& source)
      requires std::floating_point<T>
      {  return (TFloatZonotope<double>&) DoubleZonotope::operator=(std::move(source)); }
};

template<>
class FldlibBase::TFloatZonotope<long double> : public LongDoubleZonotope {
  public:
   TFloatZonotope() = default;
   TFloatZonotope(const TFloatZonotope<long double>&) = default;
   TFloatZonotope(TFloatZonotope<long double>&&) = default;
   template <typename T> TFloatZonotope(T source)
      requires std::floating_point<T> || std::integral<T>
      :  LongDoubleZonotope(source) {}
   TFloatZonotope(const char* valueString, LongDoubleZonotope::ValueFromString traits) : LongDoubleZonotope(valueString, traits) {}
   TFloatZonotope(long double min, long double max)
      :  LongDoubleZonotope(min, max) {}
   TFloatZonotope(long double min, long double max, long double minErr, long double maxErr)
      :  LongDoubleZonotope(min, max, minErr, maxErr) {}
   template <typename T> TFloatZonotope(const TFloatZonotope<T>& source)
      requires std::floating_point<T>
      :  LongDoubleZonotope(source) {}
   template <typename T> TFloatZonotope(TFloatZonotope<T>&& source)
      requires std::floating_point<T>
      :  LongDoubleZonotope(std::move(source)) {}
   TFloatZonotope(const LongDoubleZonotope& source) : LongDoubleZonotope(source) {}
   TFloatZonotope(LongDoubleZonotope&& source) : LongDoubleZonotope(std::move(source)) {}
   TFloatZonotope<long double>& operator=(const TFloatZonotope<long double>&) = default;
   TFloatZonotope<long double>& operator=(TFloatZonotope<long double>&&) = default;
   TFloatZonotope<long double>& operator=(const LongDoubleZonotope& source)
      {  return (TFloatZonotope<long double>&) LongDoubleZonotope::operator=(source); }
   TFloatZonotope<long double>& operator=(LongDoubleZonotope&& source)
      {  return (TFloatZonotope<long double>&) LongDoubleZonotope::operator=(std::move(source)); }

   template <typename T> TFloatZonotope<long double>& operator=(T source)
      requires std::floating_point<T> || std::integral<T>
      {  return (TFloatZonotope<long double>&) LongDoubleZonotope::operator=(source); }
   template <typename T> TFloatZonotope<long double>& operator=(const TFloatZonotope<T>& source)
      requires std::floating_point<T>
      {  return (TFloatZonotope<long double>&) LongDoubleZonotope::operator=(source); }
   template <typename T> TFloatZonotope<long double>& operator=(TFloatZonotope<T>&& source)
      requires std::floating_point<T>
      {  return (TFloatZonotope<long double>&) LongDoubleZonotope::operator=(std::move(source)); }
};

template <typename DoubleType> requires std::floating_point<DoubleType>
class TFldlibZonotopeOption : public FldlibBase {
  public:
   DoubleType value;
   std::shared_ptr<TFloatZonotope<DoubleType>> zonotope;
   typedef typename TFloatZonotope<DoubleType>::BuiltDouble BuiltDouble;
   typedef typename TFloatZonotope<DoubleType>::BuiltReal BuiltReal;
   typedef typename TFloatZonotope<DoubleType>::Equation Equation;
   typedef DAffine::SymbolsManager SymbolsManager;
   typedef FldlibBase::MergeBranches MergeBranches;

  private:
   typedef TFldlibZonotopeOption<DoubleType> thisType;
   void normalize()
      {  if (active) {
            if (!zonotope.get())
               zonotope = std::make_shared<TFloatZonotope<DoubleType>>(value);
         }
         else
            zonotope.reset();
      }
   void normalizeForChange()
      {  if (active) {
            if (!zonotope.get())
               zonotope = std::make_shared<TFloatZonotope<DoubleType>>(value);
            else if (zonotope.use_count() != 1)
               zonotope = std::make_shared<TFloatZonotope<DoubleType>>(*zonotope);
         }
         else
            zonotope.reset();
      }
   template <typename T> requires std::floating_point<T> friend class TFldlibZonotopeOption;

  public:
   TFldlibZonotopeOption() = default;
   template<typename T> TFldlibZonotopeOption(T source) requires std::floating_point<T>
      :  value(source) { normalize(); }
   template<typename T> TFldlibZonotopeOption(T source) requires std::integral<T>
      :  value(source) { normalize(); }
   TFldlibZonotopeOption(DoubleType min, DoubleType max)
      :  value((min+max)/2.0), zonotope(new TFloatZonotope<DoubleType>(min, max))
      {  normalize(); }
   typedef TFloatZonotope<DoubleType>::ValueFromString ValueFromString;
   TFldlibZonotopeOption(const char* valueString, ValueFromString traits)
      {  std::istringstream out(valueString);
         out >> value;
         if (active)
            zonotope = std::make_shared<TFloatZonotope<DoubleType>>(valueString, traits);
      }
   TFldlibZonotopeOption(DoubleType min, DoubleType max, DoubleType minErr, DoubleType maxErr)
      :  value((min+max)/2.0), zonotope(new TFloatZonotope<DoubleType>(min, max, minErr, maxErr))
      {  normalize(); }
   TFldlibZonotopeOption(const thisType& source) = default;
   TFldlibZonotopeOption(thisType&& source) = default;
   template<typename T> TFldlibZonotopeOption(const TFldlibZonotopeOption<T>& source) requires std::floating_point<T>
      :  value(source.value)
      {  if (active) {
            if (!source.zonotope.get())
               zonotope = std::make_shared<TFloatZonotope<DoubleType>>(value);
            else
               zonotope = std::make_shared<TFloatZonotope<DoubleType>>(*source.zonotope);
         }
      }
   template<typename T> TFldlibZonotopeOption(TFldlibZonotopeOption<T>&& source) requires std::floating_point<T>
      :  value(source.value)
      {  if (active) {
            if (!source.zonotope.get())
               zonotope = std::make_shared<TFloatZonotope<DoubleType>>(value);
            else
               zonotope = std::make_shared<TFloatZonotope<DoubleType>>(std::move(*source.zonotope));
         }
      }

   template<typename T> TFldlibZonotopeOption<DoubleType>& operator=(T source) requires std::floating_point<T>
      {  value = source;
         zonotope.reset();
         normalize();
         return *this;
      }
   template<typename T> TFldlibZonotopeOption<DoubleType>& operator=(T source) requires std::integral<T>
      {  value = source;
         zonotope.reset();
         normalize();
         return *this;
      }
   TFldlibZonotopeOption& operator=(const thisType& source) = default;
   TFldlibZonotopeOption& operator=(thisType&& source) = default;
   template<typename T> TFldlibZonotopeOption<DoubleType>& operator=(const TFldlibZonotopeOption<T>& source)
         requires std::floating_point<T>
      {  value = source.value;
         zonotope.reset();
         if (active) {
            if (source.zonotope.get())
               zonotope = std::make_shared<TFloatZonotope<DoubleType>>(*source.zonotope);
            else
               normalize();
         }
         return *this;
      }
   template<typename T> TFldlibZonotopeOption<DoubleType>& operator=(TFldlibZonotopeOption<T>&& source)
         requires std::floating_point<T>
      {  value = source.value;
         zonotope.reset();
         if (active) {
            if (source.zonotope.get())
               zonotope = std::make_shared<TFloatZonotope<DoubleType>>(*source.zonotope);
            else
               normalize();
         }
         return *this;
      }

   thisType operator++() {  ++value; zonotope.reset(); /* TODO += 1 */ return *this; }
   thisType operator++(int) { thisType result = *this; value++; zonotope.reset(); return result; }
   // thisType operator+() const { return *this; }

   friend thisType operator+(const thisType& first) { return first; }
   friend thisType operator+(thisType&& first) { return std::move(first); }

   template<typename T> TFldlibZonotopeOption<DoubleType>& operator+=(T source)
         requires std::floating_point<T> || std::integral<T>
      {  normalizeForChange();
         if (active) *zonotope += source;
         value += source;
         return *this;
      }
   thisType& operator+=(const thisType& source)
      {  normalizeForChange();
         if (active) {
            if (source.zonotope.get())
               *zonotope += *source.zonotope;
            else
               *zonotope += source.value;
         }
         value += source.value;
         return *this;
      }
   template<typename T> TFldlibZonotopeOption<DoubleType>& operator+=(const TFldlibZonotopeOption<T>& source)
         requires std::floating_point<T>
      {  normalizeForChange();
         if (active) {
            if (source.zonotope.get())
               *zonotope += *source.zonotope;
            else
               *zonotope += source.value;
         }
         value += source.value;
         return *this;
      }
   template<typename T> TFldlibZonotopeOption<DoubleType>& operator+=(TFldlibZonotopeOption<T>&& source)
         requires std::floating_point<T>
      {  normalizeForChange();
         if (active) {
            if (source.zonotope.get())
               *zonotope += *source.zonotope;
            else
               *zonotope += source.value;
         }
         value += source.value;
         return *this;
      }
   template<typename T> friend auto operator+(const TFldlibZonotopeOption<DoubleType>& first,
         T second) -> TFldlibZonotopeOption<decltype(DoubleType(0) + typename IntegralOrFloatingPointType<T>::base_type(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value + second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope += second;
         result.value += second;
         return result;
      }
   template<typename T> friend auto operator+(TFldlibZonotopeOption<DoubleType>&& first,
         T second) -> TFldlibZonotopeOption<decltype(DoubleType(0) + typename IntegralOrFloatingPointType<T>::base_type(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value + second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope += second;
         result.value += second;
         return result;
      }
   template<typename T> friend auto operator+(T first, const TFldlibZonotopeOption<DoubleType>& second)
         -> TFldlibZonotopeOption<decltype(typename IntegralOrFloatingPointType<T>::base_type(0) + DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first + second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope += *second.zonotope;
            else
               *result.zonotope += second.value;
         }
         result.value += second.value;
         return result;
      }
   template<typename T> friend auto operator+(T first, TFldlibZonotopeOption<DoubleType>&& second)
         -> TFldlibZonotopeOption<decltype(typename IntegralOrFloatingPointType<T>::base_type(0) + DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first + second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope += *second.zonotope;
            else
               *result.zonotope += second.value;
         }
         result.value += second.value;
         return result;
      }
   friend thisType operator+(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope += *second.zonotope;
            else
               *result.zonotope += second.value;
         }
         result.value += second.value;
         return result;
      }
   friend thisType operator+(thisType&& first, const thisType& second)
      {  thisType result(std::move(first));
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope += *second.zonotope;
            else
               *result.zonotope += second.value;
         }
         result.value += second.value;
         return result;
      }
   friend thisType operator+(const thisType& first, thisType&& second)
      {  thisType result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope += *second.zonotope;
            else
               *result.zonotope += second.value;
         }
         result.value += second.value;
         return result;
      }
   friend thisType operator+(thisType&& first, thisType&& second)
      {  thisType result(std::move(first));
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope += *second.zonotope;
            else
               *result.zonotope += second.value;
         }
         result.value += second.value;
         return result;
      }
   template<typename T> friend auto operator+(const thisType& first,
         const TFldlibZonotopeOption<T>& second)
      {  thisType result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope += *second.zonotope;
            else
               *result.zonotope += second.value;
         }
         result.value += second;
         return result;
      }
   template<typename T> friend auto operator+(thisType&& first,
         const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) + T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value + second.value)> result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope += *second.zonotope;
            else
               *result.zonotope += second.value;
         }
         result.value += second.value;
         return result;
      }
   template<typename T> friend auto operator+(const thisType& first,
         TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) + T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value + second.value)> result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope += *second.zonotope;
            else
               *result.zonotope += second.value;
         }
         result.value += second.value;
         return result;
      }
   template<typename T> friend auto operator+(thisType&& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) + T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value + second.value)> result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope += *second.zonotope;
            else
               *result.zonotope += second.value;
         }
         result.value += second.value;
         return result;
      }

   friend thisType operator-(const thisType& first)
      {  thisType result(first);
         result.normalizeForChange();
         if (active)
            result.zonotope->oppositeAssign();
         result.value = -result.value;
         return result;
      }
   friend thisType operator-(thisType&& first)
      {  thisType result(std::move(first));
         result.normalizeForChange();
         if (active)
            result.zonotope->oppositeAssign();
         result.value = -result.value;
         return result;
      }

   template<typename T> TFldlibZonotopeOption<DoubleType>& operator-=(T source)
         requires std::floating_point<T> || std::integral<T>
      {  normalizeForChange();
         if (active) *zonotope -= source;
         value -= source;
         return *this;
      }
   thisType& operator-=(const thisType& source)
      {  normalizeForChange();
         if (active) {
            if (source.zonotope.get())
               *zonotope -= *source.zonotope;
            else
               *zonotope -= source.value;
         }
         value -= source.value;
         return *this;
      }
   thisType& operator-=(thisType&& source)
      {  normalizeForChange();
         if (active) {
            if (source.zonotope.get())
               *zonotope -= *source.zonotope;
            else
               *zonotope -= source.value;
         }
         value -= source.value;
         return *this;
      }
   template<typename T> TFldlibZonotopeOption<DoubleType>& operator-=(const TFldlibZonotopeOption<T>& source)
         requires std::floating_point<T>
      {  normalizeForChange();
         if (active) {
            if (source.zonotope.get())
               *zonotope -= *source.zonotope;
            else
               *zonotope -= source.value;
         }
         value -= source.value;
         return *this;
      }
   template<typename T> TFldlibZonotopeOption<DoubleType>& operator-=(TFldlibZonotopeOption<T>&& source)
         requires std::floating_point<T>
      {  normalizeForChange();
         if (active) {
            if (source.zonotope.get())
               *zonotope -= *source.zonotope;
            else
               *zonotope -= source.value;
         }
         value -= source.value;
         return *this;
      }
   template<typename T>
   friend auto operator-(const TFldlibZonotopeOption<DoubleType>& first,
         T second) -> TFldlibZonotopeOption<decltype(DoubleType(0) - typename IntegralOrFloatingPointType<T>::base_type(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value - second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope -= second;
         result.value -= second;
         return result;
      }
   template<typename T> friend auto operator-(TFldlibZonotopeOption<DoubleType>&& first,
         T second) -> TFldlibZonotopeOption<decltype(DoubleType(0) - typename IntegralOrFloatingPointType<T>::base_type(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value - second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope -= second;
         result.value -= second;
         return result;
      }
   template<typename T> friend auto operator-(T first, const TFldlibZonotopeOption<DoubleType>& second)
         -> TFldlibZonotopeOption<decltype(typename IntegralOrFloatingPointType<T>::base_type(0) - DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first - second.value)> result;
         result = first;
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope -= *second.zonotope;
            else
               *result.zonotope -= second.value;
         }
         result.value -= second.value;
         return result;
      }
   template<typename T> friend auto operator-(T first, TFldlibZonotopeOption<DoubleType>&& second)
         -> TFldlibZonotopeOption<decltype(typename IntegralOrFloatingPointType<T>::base_type(0) - DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first - second.value)> result;
         result = first;
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope -= *second.zonotope;
            else
               *result.zonotope -= second.value;
         }
         result.value -= second.value;
         return result;
      }
   friend thisType operator-(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope -= *second.zonotope;
            else
               *result.zonotope -= second.value;
         }
         result.value -= second.value;
         return result;
      }
   friend thisType operator-(thisType&& first, const thisType& second)
      {  thisType result(std::move(first));
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope -= *second.zonotope;
            else
               *result.zonotope -= second.value;
         }
         result.value -= second.value;
         return result;
      }
   friend thisType operator-(const thisType& first, thisType&& second)
      {  thisType result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope -= *second.zonotope;
            else
               *result.zonotope -= second.value;
         }
         result.value -= second.value;
         return result;
      }
   friend thisType operator-(thisType&& first, thisType&& second)
      {  thisType result(std::move(first));
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope -= *second.zonotope;
            else
               *result.zonotope -= second.value;
         }
         result.value -= second.value;
         return result;
      }
   template<typename T> friend auto operator-(const thisType& first,
         const TFldlibZonotopeOption<T>& second)
      {  thisType result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope -= *second.zonotope;
            else
               *result.zonotope -= second.value;
         }
         result.value -= second;
         return result;
      }
   template<typename T> friend auto operator-(thisType&& first,
         const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) - T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value - second.value)> result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope -= *second.zonotope;
            else
               *result.zonotope -= second.value;
         }
         result.value -= second.value;
         return result;
      }
   template<typename T> friend auto operator-(const thisType& first,
         TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) - T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value - second.value)> result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope -= *second.zonotope;
            else
               *result.zonotope -= second.value;
         }
         result.value -= second.value;
         return result;
      }
   template<typename T> friend auto operator-(thisType&& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) - T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value - second.value)> result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope -= *second.zonotope;
            else
               *result.zonotope -= second.value;
         }
         result.value -= second.value;
         return result;
      }

   template<typename T> TFldlibZonotopeOption<DoubleType>& operator*=(T source)
         requires std::floating_point<T> || std::integral<T>
      {  normalizeForChange();
         if (active) *zonotope *= source;
         value *= source;
         return *this;
      }
   thisType& operator*=(const thisType& source)
      {  normalizeForChange();
         if (active) {
            if (source.zonotope.get())
               *zonotope *= *source.zonotope;
            else
               *zonotope *= source.value;
         }
         value *= source.value;
         return *this;
      }
   template<typename T> TFldlibZonotopeOption<DoubleType>& operator*=(const TFldlibZonotopeOption<T>& source)
         requires std::floating_point<T>
      {  normalizeForChange();
         if (active) {
            if (source.zonotope.get())
               *zonotope *= *source.zonotope;
            else
               *zonotope *= source.value;
         }
         value *= source.value;
         return *this;
      }
   template<typename T> TFldlibZonotopeOption<DoubleType>& operator*=(TFldlibZonotopeOption<T>&& source)
         requires std::floating_point<T>
      {  normalizeForChange();
         if (active) {
            if (source.zonotope.get())
               *zonotope *= *source.zonotope;
            else
               *zonotope *= source.value;
         }
         value *= source.value;
         return *this;
      }
   template<typename T> friend auto operator*(const TFldlibZonotopeOption<DoubleType>& first,
         T second) -> TFldlibZonotopeOption<decltype(DoubleType(0) * typename IntegralOrFloatingPointType<T>::base_type(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value * second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope *= second;
         result.value *= second;
         return result;
      }
   template<typename T> friend auto operator*(TFldlibZonotopeOption<DoubleType>&& first,
         T second) -> TFldlibZonotopeOption<decltype(DoubleType(0) * typename IntegralOrFloatingPointType<T>::base_type(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value * second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope *= second;
         result.value *= second;
         return result;
      }
   template<typename T> friend auto operator*(T first, const TFldlibZonotopeOption<DoubleType>& second)
         -> TFldlibZonotopeOption<decltype(typename IntegralOrFloatingPointType<T>::base_type(0) * DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first * second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope *= *second.zonotope;
            else
               *result.zonotope *= second.value;
         }
         result.value *= second.value;
         return result;
      }
   template<typename T> friend auto operator*(T first, TFldlibZonotopeOption<DoubleType>&& second)
         -> TFldlibZonotopeOption<decltype(typename IntegralOrFloatingPointType<T>::base_type(0) * DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first * second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope *= *second.zonotope;
            else
               *result.zonotope *= second.value;
         }
         result.value *= second.value;
         return result;
      }
   friend thisType operator*(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope *= *second.zonotope;
            else
               *result.zonotope *= second.value;
         }
         result.value *= second.value;
         return result;
      }
   friend thisType operator*(thisType&& first, const thisType& second)
      {  thisType result(std::move(first));
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope *= *second.zonotope;
            else
               *result.zonotope *= second.value;
         }
         result.value *= second.value;
         return result;
      }
   friend thisType operator*(const thisType& first, thisType&& second)
      {  thisType result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope *= *second.zonotope;
            else
               *result.zonotope *= second.value;
         }
         result.value *= second.value;
         return result;
      }
   friend thisType operator*(thisType&& first, thisType&& second)
      {  thisType result(std::move(first));
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope *= *second.zonotope;
            else
               *result.zonotope *= second.value;
         }
         result.value *= second.value;
         return result;
      }
   template<typename T> friend auto operator*(const thisType& first,
         const TFldlibZonotopeOption<T>& second)
      {  thisType result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope *= *second.zonotope;
            else
               *result.zonotope *= second.value;
         }
         result.value *= second;
         return result;
      }
   template<typename T> friend auto operator*(thisType&& first,
         const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope *= *second.zonotope;
            else
               *result.zonotope *= second.value;
         }
         result.value *= second.value;
         return result;
      }
   template<typename T> friend auto operator*(const thisType& first,
         TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope *= *second.zonotope;
            else
               *result.zonotope *= second.value;
         }
         result.value *= second.value;
         return result;
      }
   template<typename T> friend auto operator*(thisType&& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope *= *second.zonotope;
            else
               *result.zonotope *= second.value;
         }
         result.value *= second.value;
         return result;
      }

   template<typename T> TFldlibZonotopeOption<DoubleType>& operator/=(T source)
         requires std::floating_point<T> || std::integral<T>
      {  normalizeForChange();
         if (active) *zonotope /= source;
         value /= source;
         return *this;
      }
   thisType& operator/=(const thisType& source)
      {  normalizeForChange();
         if (active) {
            if (source.zonotope.get())
               *zonotope /= *source.zonotope;
            else
               *zonotope /= source.value;
         }
         value /= source.value;
         return *this;
      }
   template<typename T> TFldlibZonotopeOption<DoubleType>& operator/=(const TFldlibZonotopeOption<T>& source)
         requires std::floating_point<T>
      {  normalizeForChange();
         if (active) {
            if (source.zonotope.get())
               *zonotope /= *source.zonotope;
            else
               *zonotope /= source.value;
         }
         value /= source.value;
         return *this;
      }
   template<typename T> TFldlibZonotopeOption<DoubleType>& operator/=(TFldlibZonotopeOption<T>&& source)
         requires std::floating_point<T>
      {  normalizeForChange();
         if (active) {
            if (source.zonotope.get())
               *zonotope /= *source.zonotope;
            else
               *zonotope /= source.value;
         }
         value /= source.value;
         return *this;
      }
   template<typename T> friend auto operator/(const TFldlibZonotopeOption<DoubleType>& first,
         T second) -> TFldlibZonotopeOption<decltype(DoubleType(0) / typename IntegralOrFloatingPointType<T>::base_type(1))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value / second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope /= second;
         result.value /= second;
         return result;
      }
   template<typename T> friend auto operator/(TFldlibZonotopeOption<DoubleType>&& first,
         T second) -> TFldlibZonotopeOption<decltype(DoubleType(0) / typename IntegralOrFloatingPointType<T>::base_type(1))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value / second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope /= second;
         result.value /= second;
         return result;
      }
   template<typename T> friend auto operator/(T first, const TFldlibZonotopeOption<DoubleType>& second)
         -> TFldlibZonotopeOption<decltype(typename IntegralOrFloatingPointType<T>::base_type(0) / DoubleType(1))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first / second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope /= *second.zonotope;
            else
               *result.zonotope /= second.value;
         }
         result.value /= second.value;
         return result;
      }
   template<typename T> friend auto operator/(T first, TFldlibZonotopeOption<DoubleType>&& second)
         -> TFldlibZonotopeOption<decltype(typename IntegralOrFloatingPointType<T>::base_type(0) / DoubleType(1))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first / second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope /= *second.zonotope;
            else
               *result.zonotope /= second.value;
         }
         result.value /= second.value;
         return result;
      }
   friend thisType operator/(const thisType& first, const thisType& second)
      {  thisType result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope /= *second.zonotope;
            else
               *result.zonotope /= second.value;
         }
         result.value /= second.value;
         return result;
      }
   friend thisType operator/(thisType&& first, const thisType& second)
      {  thisType result(std::move(first));
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope /= *second.zonotope;
            else
               *result.zonotope /= second.value;
         }
         result.value /= second.value;
         return result;
      }
   friend thisType operator/(const thisType& first, thisType&& second)
      {  thisType result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope /= *second.zonotope;
            else
               *result.zonotope /= second.value;
         }
         result.value /= second.value;
         return result;
      }
   friend thisType operator/(thisType&& first, thisType&& second)
      {  thisType result(std::move(first));
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope /= *second.zonotope;
            else
               *result.zonotope /= second.value;
         }
         result.value /= second.value;
         return result;
      }
   template<typename T> friend auto operator/(const thisType& first,
         const TFldlibZonotopeOption<T>& second)
      {  thisType result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope /= *second.zonotope;
            else
               *result.zonotope /= second.value;
         }
         result.value /= second;
         return result;
      }
   template<typename T> friend auto operator/(thisType&& first,
         const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) / T(1))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value / second.value)> result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope /= *second.zonotope;
            else
               *result.zonotope /= second.value;
         }
         result.value /= second.value;
         return result;
      }
   template<typename T> friend auto operator/(const thisType& first,
         TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) / T(1))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value / second.value)> result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope /= *second.zonotope;
            else
               *result.zonotope /= second.value;
         }
         result.value /= second.value;
         return result;
      }
   template<typename T> friend auto operator/(thisType&& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) / T(1))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value / second.value)> result(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope /= *second.zonotope;
            else
               *result.zonotope /= second.value;
         }
         result.value /= second.value;
         return result;
      }

   operator DoubleType() const { return value; }
   template <typename T> operator T() const requires std::floating_point<T>
      {  return (T) value; }
   template <typename T> operator T() const requires std::integral<T>
      {  if (active) {
            if (zonotope.get())
               return (T) *zonotope;
         }
         return (T) value;
      }

   // operator space_shift to add
   friend bool operator<(const thisType& first, const thisType& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope < *second.zonotope;
               return *first.zonotope < second.value;
            }
            if (second.zonotope.get())
               return first.value < *second.zonotope;
         }
         return first.value < second.value;
      }
   friend bool operator<(thisType&& first, const thisType& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope < *second.zonotope;
               return *first.zonotope < second.value;
            }
            if (second.zonotope.get())
               return first.value < *second.zonotope;
         }
         return first.value < second.value;
      }
   friend bool operator<(const thisType& first, thisType&& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope < *second.zonotope;
               return *first.zonotope < second.value;
            }
            if (second.zonotope.get())
               return first.value < *second.zonotope;
         }
         return first.value < second.value;
      }
   friend bool operator<(thisType&& first, thisType&& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope < *second.zonotope;
               return *first.zonotope < second.value;
            }
            if (second.zonotope.get())
               return first.value < *second.zonotope;
         }
         return first.value < second.value;
      }
   template <typename T> friend bool operator<(const thisType& first,
         const TFldlibZonotopeOption<T>& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope < *second.zonotope;
               return *first.zonotope < second.value;
            }
            if (second.zonotope.get())
               return first.value < *second.zonotope;
         }
         return first.value < second.value;
      }
   template <typename T> friend bool operator<(const thisType& first,
         TFldlibZonotopeOption<T>&& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope < *second.zonotope;
               return *first.zonotope < second.value;
            }
            if (second.zonotope.get())
               return first.value < *second.zonotope;
         }
         return first.value < second.value;
      }
   template <typename T> friend bool operator<(thisType&& first,
         const TFldlibZonotopeOption<T>& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope < *second.zonotope;
               return *first.zonotope < second.value;
            }
            if (second.zonotope.get())
               return first.value < *second.zonotope;
         }
         return first.value < second.value;
      }
   template <typename T> friend bool operator<(thisType&& first,
         TFldlibZonotopeOption<T>&& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope < *second.zonotope;
               return *first.zonotope < second.value;
            }
            if (second.zonotope.get())
               return first.value < *second.zonotope;
         }
         return first.value < second.value;
      }
   template <typename T> friend bool operator<(const thisType& first, T second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (first.zonotope.get())
               return *first.zonotope < second;
         }
         return first.value < second;
      }
   template <typename T> friend bool operator<(thisType&& first, T second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (first.zonotope.get())
               return *first.zonotope < second;
         }
         return first.value < second;
      }
   template <typename T> friend bool operator<(T first, const thisType& second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (second.zonotope.get())
               return first < *second.zonotope;
         }
         return first < second.value;
      }
   template <typename T> friend bool operator<(T first, thisType&& second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (second.zonotope.get())
               return first < *second.zonotope;
         }
         return first < second.value;
      }

   friend bool operator<=(const thisType& first, const thisType& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope <= *second.zonotope;
               return *first.zonotope <= second.value;
            }
            if (second.zonotope.get())
               return first.value <= *second.zonotope;
         }
         return first.value <= second.value;
      }
   friend bool operator<=(const thisType& first, thisType&& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope <= *second.zonotope;
               return *first.zonotope <= second.value;
            }
            if (second.zonotope.get())
               return first.value <= *second.zonotope;
         }
         return first.value <= second.value;
      }
   friend bool operator<=(thisType&& first, const thisType& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope <= *second.zonotope;
               return *first.zonotope <= second.value;
            }
            if (second.zonotope.get())
               return first.value <= *second.zonotope;
         }
         return first.value <= second.value;
      }
   friend bool operator<=(thisType&& first, thisType&& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope <= *second.zonotope;
               return *first.zonotope <= second.value;
            }
            if (second.zonotope.get())
               return first.value <= *second.zonotope;
         }
         return first.value <= second.value;
      }
   template <typename T> friend bool operator<=(const thisType& first,
         const TFldlibZonotopeOption<T>& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope <= *second.zonotope;
               return *first.zonotope <= second.value;
            }
            if (second.zonotope.get())
               return first.value <= *second.zonotope;
         }
         return first.value <= second.value;
      }
   template <typename T> friend bool operator<=(const thisType& first,
         TFldlibZonotopeOption<T>&& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope <= *second.zonotope;
               return *first.zonotope <= second.value;
            }
            if (second.zonotope.get())
               return first.value <= *second.zonotope;
         }
         return first.value <= second.value;
      }
   template <typename T> friend bool operator<=(thisType&& first,
         const TFldlibZonotopeOption<T>& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope <= *second.zonotope;
               return *first.zonotope <= second.value;
            }
            if (second.zonotope.get())
               return first.value <= *second.zonotope;
         }
         return first.value <= second.value;
      }
   template <typename T> friend bool operator<=(thisType&& first,
         TFldlibZonotopeOption<T>&& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope <= *second.zonotope;
               return *first.zonotope <= second.value;
            }
            if (second.zonotope.get())
               return first.value <= *second.zonotope;
         }
         return first.value <= second.value;
      }
   template <typename T> friend bool operator<=(const thisType& first, T second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (first.zonotope.get())
               return *first.zonotope <= second;
         }
         return first.value <= second;
      }
   template <typename T> friend bool operator<=(thisType&& first, T second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (first.zonotope.get())
               return *first.zonotope <= second;
         }
         return first.value <= second;
      }
   template <typename T> friend bool operator<=(T first, const thisType& second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (second.zonotope.get())
               return first <= *second.zonotope;
         }
         return first <= second.value;
      }
   template <typename T> friend bool operator<=(T first, thisType&& second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (second.zonotope.get())
               return first <= *second.zonotope;
         }
         return first <= second.value;
      }

   friend bool operator==(const thisType& first, const thisType& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope == *second.zonotope;
               return *first.zonotope == second.value;
            }
            if (second.zonotope.get())
               return first.value == *second.zonotope;
         }
         return first.value == second.value;
      }
   friend bool operator==(const thisType& first, thisType&& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope == *second.zonotope;
               return *first.zonotope == second.value;
            }
            if (second.zonotope.get())
               return first.value == *second.zonotope;
         }
         return first.value == second.value;
      }
   friend bool operator==(thisType&& first, const thisType& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope == *second.zonotope;
               return *first.zonotope == second.value;
            }
            if (second.zonotope.get())
               return first.value == *second.zonotope;
         }
         return first.value == second.value;
      }
   friend bool operator==(thisType&& first, thisType&& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope == *second.zonotope;
               return *first.zonotope == second.value;
            }
            if (second.zonotope.get())
               return first.value == *second.zonotope;
         }
         return first.value == second.value;
      }
   template <typename T> friend bool operator==(const thisType& first,
         const TFldlibZonotopeOption<T>& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope == *second.zonotope;
               return *first.zonotope == second.value;
            }
            if (second.zonotope.get())
               return first.value == *second.zonotope;
         }
         return first.value == second.value;
      }
   template <typename T> friend bool operator==(const thisType& first,
         TFldlibZonotopeOption<T>&& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope == *second.zonotope;
               return *first.zonotope == second.value;
            }
            if (second.zonotope.get())
               return first.value == *second.zonotope;
         }
         return first.value == second.value;
      }
   template <typename T> friend bool operator==(thisType&& first,
         const TFldlibZonotopeOption<T>& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope == *second.zonotope;
               return *first.zonotope == second.value;
            }
            if (second.zonotope.get())
               return first.value == *second.zonotope;
         }
         return first.value == second.value;
      }
   template <typename T> friend bool operator==(thisType&& first,
         TFldlibZonotopeOption<T>&& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope == *second.zonotope;
               return *first.zonotope == second.value;
            }
            if (second.zonotope.get())
               return first.value == *second.zonotope;
         }
         return first.value == second.value;
      }
   template <typename T> friend bool operator==(const thisType& first, T second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (first.zonotope.get())
               return *first.zonotope == second;
         }
         return first.value == second;
      }
   template <typename T> friend bool operator==(thisType&& first, T second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (first.zonotope.get())
               return *first.zonotope == second;
         }
         return first.value == second;
      }
   template <typename T> friend bool operator==(T first, const thisType& second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (second.zonotope.get())
               return first == *second.zonotope;
         }
         return first == second.value;
      }
   template <typename T> friend bool operator==(T first, thisType&& second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (second.zonotope.get())
               return first == *second.zonotope;
         }
         return first == second.value;
      }

   friend bool operator!=(const thisType& first, const thisType& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope != *second.zonotope;
               return *first.zonotope != second.value;
            }
            if (second.zonotope.get())
               return first.value != *second.zonotope;
         }
         return first.value != second.value;
      }
   friend bool operator!=(const thisType& first, thisType&& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope != *second.zonotope;
               return *first.zonotope != second.value;
            }
            if (second.zonotope.get())
               return first.value != *second.zonotope;
         }
         return first.value != second.value;
      }
   friend bool operator!=(thisType&& first, const thisType& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope != *second.zonotope;
               return *first.zonotope != second.value;
            }
            if (second.zonotope.get())
               return first.value != *second.zonotope;
         }
         return first.value != second.value;
      }
   friend bool operator!=(thisType&& first, thisType&& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope != *second.zonotope;
               return *first.zonotope != second.value;
            }
            if (second.zonotope.get())
               return first.value != *second.zonotope;
         }
         return first.value != second.value;
      }
   template <typename T> friend bool operator!=(const thisType& first,
         const TFldlibZonotopeOption<T>& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope != *second.zonotope;
               return *first.zonotope != second.value;
            }
            if (second.zonotope.get())
               return first.value != *second.zonotope;
         }
         return first.value != second.value;
      }
   template <typename T> friend bool operator!=(const thisType& first,
         TFldlibZonotopeOption<T>&& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope != *second.zonotope;
               return *first.zonotope != second.value;
            }
            if (second.zonotope.get())
               return first.value != *second.zonotope;
         }
         return first.value != second.value;
      }
   template <typename T> friend bool operator!=(thisType&& first,
         const TFldlibZonotopeOption<T>& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope != *second.zonotope;
               return *first.zonotope != second.value;
            }
            if (second.zonotope.get())
               return first.value != *second.zonotope;
         }
         return first.value != second.value;
      }
   template <typename T> friend bool operator!=(thisType&& first,
         TFldlibZonotopeOption<T>&& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope != *second.zonotope;
               return *first.zonotope != second.value;
            }
            if (second.zonotope.get())
               return first.value != *second.zonotope;
         }
         return first.value != second.value;
      }
   template <typename T> friend bool operator!=(const thisType& first, T second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (first.zonotope.get())
               return *first.zonotope != second;
         }
         return first.value != second;
      }
   template <typename T> friend bool operator!=(thisType&& first, T second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (first.zonotope.get())
               return *first.zonotope != second;
         }
         return first.value != second;
      }
   template <typename T> friend bool operator!=(T first, const thisType& second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (second.zonotope.get())
               return first != *second.zonotope;
         }
         return first != second.value;
      }
   template <typename T> friend bool operator!=(T first, thisType&& second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (second.zonotope.get())
               return first != *second.zonotope;
         }
         return first != second.value;
      }

   friend bool operator>=(const thisType& first, const thisType& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope >= *second.zonotope;
               return *first.zonotope >= second.value;
            }
            if (second.zonotope.get())
               return first.value >= *second.zonotope;
         }
         return first.value >= second.value;
      }
   friend bool operator>=(const thisType& first, thisType&& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope >= *second.zonotope;
               return *first.zonotope >= second.value;
            }
            if (second.zonotope.get())
               return first.value >= *second.zonotope;
         }
         return first.value >= second.value;
      }
   friend bool operator>=(thisType&& first, const thisType& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope >= *second.zonotope;
               return *first.zonotope >= second.value;
            }
            if (second.zonotope.get())
               return first.value >= *second.zonotope;
         }
         return first.value >= second.value;
      }
   friend bool operator>=(thisType&& first, thisType&& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope >= *second.zonotope;
               return *first.zonotope >= second.value;
            }
            if (second.zonotope.get())
               return first.value >= *second.zonotope;
         }
         return first.value >= second.value;
      }
   template <typename T> friend bool operator>=(const thisType& first,
         const TFldlibZonotopeOption<T>& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope >= *second.zonotope;
               return *first.zonotope >= second.value;
            }
            if (second.zonotope.get())
               return first.value >= *second.zonotope;
         }
         return first.value >= second.value;
      }
   template <typename T> friend bool operator>=(const thisType& first,
         TFldlibZonotopeOption<T>&& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope >= *second.zonotope;
               return *first.zonotope >= second.value;
            }
            if (second.zonotope.get())
               return first.value >= *second.zonotope;
         }
         return first.value >= second.value;
      }
   template <typename T> friend bool operator>=(thisType&& first,
         const TFldlibZonotopeOption<T>& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope >= *second.zonotope;
               return *first.zonotope >= second.value;
            }
            if (second.zonotope.get())
               return first.value >= *second.zonotope;
         }
         return first.value >= second.value;
      }
   template <typename T> friend bool operator>=(thisType&& first,
         TFldlibZonotopeOption<T>&& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope >= *second.zonotope;
               return *first.zonotope >= second.value;
            }
            if (second.zonotope.get())
               return first.value >= *second.zonotope;
         }
         return first.value >= second.value;
      }
   template <typename T> friend bool operator>=(const thisType& first, T second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (first.zonotope.get())
               return *first.zonotope >= second;
         }
         return first.value >= second;
      }
   template <typename T> friend bool operator>=(thisType&& first, T second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (first.zonotope.get())
               return *first.zonotope >= second;
         }
         return first.value >= second;
      }
   template <typename T> friend bool operator>=(T first, const thisType& second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (second.zonotope.get())
               return first >= *second.zonotope;
         }
         return first >= second.value;
      }
   template <typename T> friend bool operator>=(T first, thisType&& second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (second.zonotope.get())
               return first >= *second.zonotope;
         }
         return first >= second.value;
      }

   friend bool operator>(const thisType& first, const thisType& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope > *second.zonotope;
               return *first.zonotope > second.value;
            }
            if (second.zonotope.get())
               return first.value > *second.zonotope;
         }
         return first.value > second.value;
      }
   friend bool operator>(const thisType& first, thisType&& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope > *second.zonotope;
               return *first.zonotope > second.value;
            }
            if (second.zonotope.get())
               return first.value > *second.zonotope;
         }
         return first.value > second.value;
      }
   friend bool operator>(thisType&& first, const thisType& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope > *second.zonotope;
               return *first.zonotope > second.value;
            }
            if (second.zonotope.get())
               return first.value > *second.zonotope;
         }
         return first.value > second.value;
      }
   friend bool operator>(thisType&& first, thisType&& second)
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope > *second.zonotope;
               return *first.zonotope > second.value;
            }
            if (second.zonotope.get())
               return first.value > *second.zonotope;
         }
         return first.value > second.value;
      }
   template <typename T> friend bool operator>(const thisType& first,
         const TFldlibZonotopeOption<T>& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope > *second.zonotope;
               return *first.zonotope > second.value;
            }
            if (second.zonotope.get())
               return first.value > *second.zonotope;
         }
         return first.value > second.value;
      }
   template <typename T> friend bool operator>(const thisType& first,
         TFldlibZonotopeOption<T>&& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope > *second.zonotope;
               return *first.zonotope > second.value;
            }
            if (second.zonotope.get())
               return first.value > *second.zonotope;
         }
         return first.value > second.value;
      }
   template <typename T> friend bool operator>(thisType&& first,
         const TFldlibZonotopeOption<T>& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope > *second.zonotope;
               return *first.zonotope > second.value;
            }
            if (second.zonotope.get())
               return first.value > *second.zonotope;
         }
         return first.value > second.value;
      }
   template <typename T> friend bool operator>(thisType&& first,
         TFldlibZonotopeOption<T>&& second)
         requires std::floating_point<T>
      {  if (active) {
            if (first.zonotope.get()) {
               if (second.zonotope.get())
                  return *first.zonotope > *second.zonotope;
               return *first.zonotope > second.value;
            }
            if (second.zonotope.get())
               return first.value > *second.zonotope;
         }
         return first.value > second.value;
      }
   template <typename T> friend bool operator>(const thisType& first, T second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (first.zonotope.get())
               return *first.zonotope > second;
         }
         return first.value > second;
      }
   template <typename T> friend bool operator>(thisType&& first, T second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (first.zonotope.get())
               return *first.zonotope > second;
         }
         return first.value > second;
      }
   template <typename T> friend bool operator>(T first, const thisType& second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (second.zonotope.get())
               return first > *second.zonotope;
         }
         return first > second.value;
      }
   template <typename T> friend bool operator>(T first, thisType&& second)
         requires std::floating_point<T> || std::integral<T>
      {  if (active) {
            if (second.zonotope.get())
               return first > *second.zonotope;
         }
         return first > second.value;
      }

   // math operators
   friend thisType sqrt(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         bool hasRealValue = false;
         if (active) {
            *result.zonotope = sqrt(*result.zonotope);
            if (result.value <= 0 && result.zonotope->getCurrentPathExplorer()->mode()
                  == result.zonotope->MOnlyReal) {
               hasRealValue = true;
               result.value = result.zonotope->asImplementation();
            }
         }
         if (!hasRealValue)
            result.value = sqrt(result.value);
         return result;
      }
   friend thisType sqrt(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         bool hasRealValue = false;
         if (active) {
            *result.zonotope = sqrt(*result.zonotope);
            if (result.value <= 0 && result.zonotope->getCurrentPathExplorer()->mode()
                  == result.zonotope->MOnlyReal) {
               hasRealValue = true;
               result.value = result.zonotope->asImplementation();
            }
         }
         if (!hasRealValue)
            result.value = sqrt(result.value);
         return result;
      }
   friend thisType sin(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = sin(*result.zonotope);
         result.value = sin(result.value);
         return result;
      }
   friend thisType sin(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = sin(*result.zonotope);
         result.value = sin(result.value);
         return result;
      }
   friend thisType cos(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = cos(*result.zonotope);
         result.value = cos(result.value);
         return result;
      }
   friend thisType cos(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = cos(*result.zonotope);
         result.value = cos(result.value);
         return result;
      }
   friend thisType asin(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = asin(*result.zonotope);
         result.value = asin(result.value);
         return result;
      }
   friend thisType asin(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = asin(*result.zonotope);
         result.value = asin(result.value);
         return result;
      }

   friend thisType acos(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = acos(*result.zonotope);
         result.value = acos(result.value);
         return result;
      }
   friend thisType acos(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = acos(*result.zonotope);
         result.value = acos(result.value);
         return result;
      }
   friend thisType tan(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = tan(*result.zonotope);
         result.value = tan(result.value);
         return result;
      }
   friend thisType tan(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = tan(*result.zonotope);
         result.value = tan(result.value);
         return result;
      }
   friend thisType atan(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = atan(*result.zonotope);
         result.value = atan(result.value);
         return result;
      }
   friend thisType atan(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = atan(*result.zonotope);
         result.value = atan(result.value);
         return result;
      }
   friend thisType exp(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = exp(*result.zonotope);
         result.value = exp(result.value);
         return result;
      }
   friend thisType exp(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = exp(*result.zonotope);
         result.value = exp(result.value);
         return result;
      }
   friend thisType log(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = log(*result.zonotope);
         result.value = log(result.value);
         return result;
      }
   friend thisType log(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = log(*result.zonotope);
         result.value = log(result.value);
         return result;
      }
   friend thisType log10(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = log10(*result.zonotope);
         result.value = log10(result.value);
         return result;
      }
   friend thisType log10(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = log10(*result.zonotope);
         result.value = log10(result.value);
         return result;
      }
   friend thisType floor(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = floor(*result.zonotope);
         result.value = floor(result.value);
         return result;
      }
   friend thisType floor(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = floor(*result.zonotope);
         result.value = floor(result.value);
         return result;
      }
   friend thisType ceil(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = ceil(*result.zonotope);
         result.value = ceil(result.value);
         return result;
      }
   friend thisType ceil(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = ceil(*result.zonotope);
         result.value = ceil(result.value);
         return result;
      }
   friend thisType trunc(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = trunc(*result.zonotope);
         result.value = trunc(result.value);
         return result;
      }
   friend thisType trunc(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = trunc(*result.zonotope);
         result.value = trunc(result.value);
         return result;
      }
   friend thisType round(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = round(*result.zonotope);
         result.value = round(result.value);
         return result;
      }
   friend thisType round(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = round(*result.zonotope);
         result.value = round(result.value);
         return result;
      }

   template<typename T> friend auto pow(const TFldlibZonotopeOption<DoubleType>& first,
         T second) -> TFldlibZonotopeOption<decltype(DoubleType(0) * second)>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value * second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope = pow(*result.zonotope, second);
         result.value = std::pow(result.value, second);
         return result;
      }
   template<typename T> friend auto pow(TFldlibZonotopeOption<DoubleType>&& first, T second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * second)>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value * second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope = pow(*result.zonotope, second);
         result.value = std::pow(result.value, second);
         return result;
      }
   template<typename T> friend auto pow(T first, const TFldlibZonotopeOption<DoubleType>& second)
         -> TFldlibZonotopeOption<decltype(first * DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first * second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = pow(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = pow(*result.zonotope, second.value);
         }
         result.value = std::pow(result.value, second.value);
         return result;
      }
   template<typename T> friend auto pow(T first, TFldlibZonotopeOption<DoubleType>&& second)
         -> TFldlibZonotopeOption<decltype(first * DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first * second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = pow(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = pow(*result.zonotope, second.value);
         }
         result.value = std::pow(result.value, second.value);
         return result;
      }
   friend thisType pow(const thisType& first, const thisType& second)
      {  thisType result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = pow(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = pow(*result.zonotope, second.value);
         }
         result.value = std::pow(result.value, second.value);
         return result;
      }
   friend thisType pow(thisType&& first, const thisType& second)
      {  thisType result = std::move(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = pow(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = pow(*result.zonotope, second.value);
         }
         result.value = std::pow(result.value, second.value);
         return result;
      }
   friend thisType pow(const thisType& first, thisType&& second)
      {  thisType result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = pow(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = pow(*result.zonotope, second.value);
         }
         result.value = std::pow(result.value, second.value);
         return result;
      }
   friend thisType pow(thisType&& first, thisType&& second)
      {  thisType result = std::move(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = pow(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = pow(*result.zonotope, second.value);
         }
         result.value = std::pow(result.value, second.value);
         return result;
      }
   template<typename T> friend auto pow(const thisType& first, const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = pow(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = pow(*result.zonotope, second.value);
         }
         result.value = std::pow(result.value, second.value);
         return result;
      }
   template<typename T> friend auto pow(thisType&& first, const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = pow(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = pow(*result.zonotope, second.value);
         }
         result.value = std::pow(result.value, second.value);
         return result;
      }
   template<typename T> friend auto pow(const thisType& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = pow(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = pow(*result.zonotope, second.value);
         }
         result.value = std::pow(result.value, second.value);
         return result;
      }
   template<typename T> friend auto pow(thisType&& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = pow(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = pow(*result.zonotope, second.value);
         }
         result.value = std::pow(result.value, second.value);
         return result;
      }

   template<typename T> friend auto powf(const TFldlibZonotopeOption<DoubleType>& first,
         T second) -> TFldlibZonotopeOption<decltype(DoubleType(0) * second)>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value * second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope = powf(*result.zonotope, second);
         result.value = powf(result.value, second);
         return result;
      }
   template<typename T> friend auto powf(TFldlibZonotopeOption<DoubleType>&& first, T second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * second)>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value * second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope = powf(*result.zonotope, second);
         result.value = powf(result.value, second);
         return result;
      }
   template<typename T> friend auto powf(T first, const TFldlibZonotopeOption<DoubleType>& second)
         -> TFldlibZonotopeOption<decltype(first * DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first * second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = powf(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = powf(*result.zonotope, second.value);
         }
         result.value = powf(result.value, second.value);
         return result;
      }
   template<typename T> friend auto powf(T first, TFldlibZonotopeOption<DoubleType>&& second)
         -> TFldlibZonotopeOption<decltype(first * DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first * second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = powf(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = powf(*result.zonotope, second.value);
         }
         result.value = powf(result.value, second.value);
         return result;
      }
   friend thisType powf(const thisType& first, const thisType& second)
      {  thisType result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = powf(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = powf(*result.zonotope, second.value);
         }
         result.value = powf(result.value, second.value);
         return result;
      }
   friend thisType powf(thisType&& first, const thisType& second)
      {  thisType result = std::move(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = powf(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = powf(*result.zonotope, second.value);
         }
         result.value = powf(result.value, second.value);
         return result;
      }
   friend thisType powf(const thisType& first, thisType&& second)
      {  thisType result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = powf(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = powf(*result.zonotope, second.value);
         }
         result.value = powf(result.value, second.value);
         return result;
      }
   friend thisType powf(thisType&& first, thisType&& second)
      {  thisType result = std::move(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = powf(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = powf(*result.zonotope, second.value);
         }
         result.value = powf(result.value, second.value);
         return result;
      }
   template<typename T> friend auto powf(const thisType& first, const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = powf(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = powf(*result.zonotope, second.value);
         }
         result.value = powf(result.value, second.value);
         return result;
      }
   template<typename T> friend auto powf(thisType&& first, const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = powf(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = powf(*result.zonotope, second.value);
         }
         result.value = powf(result.value, second.value);
         return result;
      }
   template<typename T> friend auto powf(const thisType& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = powf(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = powf(*result.zonotope, second.value);
         }
         result.value = powf(result.value, second.value);
         return result;
      }
   template<typename T> friend auto powf(thisType&& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = powf(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = powf(*result.zonotope, second.value);
         }
         result.value = powf(result.value, second.value);
         return result;
      }

   template<typename T> friend auto atan2(const TFldlibZonotopeOption<DoubleType>& first,
         T second) -> TFldlibZonotopeOption<decltype(DoubleType(0) * second)>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value * second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope = atan2(*result.zonotope, second);
         result.value = std::atan2(result.value, second);
         return result;
      }
   template<typename T> friend auto atan2(TFldlibZonotopeOption<DoubleType>&& first, T second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * second)>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value * second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope = atan2(*result.zonotope, second);
         result.value = std::atan2(result.value, second);
         return result;
      }
   template<typename T> friend auto atan2(T first, const TFldlibZonotopeOption<DoubleType>& second)
         -> TFldlibZonotopeOption<decltype(first * DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first * second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = atan2(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = atan2(*result.zonotope, second.value);
         }
         result.value = std::atan2(result.value, second.value);
         return result;
      }
   template<typename T> friend auto atan2(T first, TFldlibZonotopeOption<DoubleType>&& second)
         -> TFldlibZonotopeOption<decltype(first * DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first * second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = atan2(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = atan2(*result.zonotope, second.value);
         }
         result.value = std::atan2(result.value, second.value);
         return result;
      }
   friend thisType atan2(const thisType& first, const thisType& second)
      {  thisType result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = atan2(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = atan2(*result.zonotope, second.value);
         }
         result.value = std::atan2(result.value, second.value);
         return result;
      }
   friend thisType atan2(thisType&& first, const thisType& second)
      {  thisType result = std::move(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = atan2(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = atan2(*result.zonotope, second.value);
         }
         result.value = std::atan2(result.value, second.value);
         return result;
      }
   friend thisType atan2(const thisType& first, thisType&& second)
      {  thisType result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = atan2(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = atan2(*result.zonotope, second.value);
         }
         result.value = std::atan2(result.value, second.value);
         return result;
      }
   friend thisType atan2(thisType&& first, thisType&& second)
      {  thisType result = std::move(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = atan2(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = atan2(*result.zonotope, second.value);
         }
         result.value = std::atan2(result.value, second.value);
         return result;
      }
   template<typename T> friend auto atan2(const thisType& first, const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = atan2(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = atan2(*result.zonotope, second.value);
         }
         result.value = std::atan2(result.value, second.value);
         return result;
      }
   template<typename T> friend auto atan2(thisType&& first, const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = atan2(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = atan2(*result.zonotope, second.value);
         }
         result.value = std::atan2(result.value, second.value);
         return result;
      }
   template<typename T> friend auto atan2(const thisType& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = atan2(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = atan2(*result.zonotope, second.value);
         }
         result.value = std::atan2(result.value, second.value);
         return result;
      }
   template<typename T> friend auto atan2(thisType&& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = atan2(*result.zonotope, *second.zonotope);
            else
               *result.zonotope = atan2(*result.zonotope, second.value);
         }
         result.value = std::atan2(result.value, second.value);
         return result;
      }

   friend thisType abs(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = abs(*result.zonotope);
         result.value = abs(result.value);
         return result;
      }
   friend thisType abs(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = abs(*result.zonotope);
         result.value = abs(result.value);
         return result;
      }
   friend thisType fabs(const thisType& source)
      {  thisType result = source;
         result.normalizeForChange();
         if (active)
            *result.zonotope = fabs(*result.zonotope);
         result.value = fabs(result.value);
         return result;
      }
   friend thisType fabs(thisType&& source)
      {  thisType result = std::move(source);
         result.normalizeForChange();
         if (active)
            *result.zonotope = fabs(*result.zonotope);
         result.value = fabs(result.value);
         return result;
      }

   template<typename T> friend auto fmin(const TFldlibZonotopeOption<DoubleType>& first,
         T second) -> TFldlibZonotopeOption<decltype(DoubleType(0) * second)>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value * second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope = result.zonotope->min(second);
         result.value = std::fmin(result.value, second);
         return result;
      }
   template<typename T> friend auto fmin(TFldlibZonotopeOption<DoubleType>&& first, T second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * second)>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value * second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope = result.zonotope->min(second);
         result.value = std::fmin(result.value, second);
         return result;
      }
   template<typename T> friend auto fmin(T first, const TFldlibZonotopeOption<DoubleType>& second)
         -> TFldlibZonotopeOption<decltype(first * DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first * second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->min(*second.zonotope);
            else
               *result.zonotope = result.zonotope->min(second.value);
         }
         result.value = std::fmin(result.value, second.value);
         return result;
      }
   template<typename T> friend auto fmin(T first, TFldlibZonotopeOption<DoubleType>&& second)
         -> TFldlibZonotopeOption<decltype(first * DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first * second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->min(*second.zonotope);
            else
               *result.zonotope = result.zonotope->min(second.value);
         }
         result.value = std::fmin(result.value, second.value);
         return result;
      }
   friend thisType fmin(const thisType& first, const thisType& second)
      {  thisType result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->min(*second.zonotope);
            else
               *result.zonotope = result.zonotope->min(second.value);
         }
         result.value = std::fmin(result.value, second.value);
         return result;
      }
   friend thisType fmin(thisType&& first, const thisType& second)
      {  thisType result = std::move(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->min(*second.zonotope);
            else
               *result.zonotope = result.zonotope->min(second.value);
         }
         result.value = std::fmin(result.value, second.value);
         return result;
      }
   friend thisType fmin(const thisType& first, thisType&& second)
      {  thisType result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->min(*second.zonotope);
            else
               *result.zonotope = result.zonotope->min(second.value);
         }
         result.value = std::fmin(result.value, second.value);
         return result;
      }
   friend thisType fmin(thisType&& first, thisType&& second)
      {  thisType result = std::move(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->min(*second.zonotope);
            else
               *result.zonotope = result.zonotope->min(second.value);
         }
         result.value = std::fmin(result.value, second.value);
         return result;
      }
   template<typename T> friend auto fmin(const thisType& first, const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->min(*second.zonotope);
            else
               *result.zonotope = result.zonotope->min(second.value);
         }
         result.value = std::fmin(result.value, second.value);
         return result;
      }
   template<typename T> friend auto fmin(thisType&& first, const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->min(*second.zonotope);
            else
               *result.zonotope = result.zonotope->min(second.value);
         }
         result.value = std::fmin(result.value, second.value);
         return result;
      }
   template<typename T> friend auto fmin(const thisType& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->min(*second.zonotope);
            else
               *result.zonotope = result.zonotope->min(second.value);
         }
         result.value = std::fmin(result.value, second.value);
         return result;
      }
   template<typename T> friend auto fmin(thisType&& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->min(*second.zonotope);
            else
               *result.zonotope = result.zonotope->min(second.value);
         }
         result.value = std::fmin(result.value, second.value);
         return result;
      }

   template<typename T> friend auto fmax(const TFldlibZonotopeOption<DoubleType>& first,
         T second) -> TFldlibZonotopeOption<decltype(DoubleType(0) * second)>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value * second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope = result.zonotope->max(second.value);
         result.value = std::fmax(result.value, second);
         return result;
      }
   template<typename T> friend auto fmax(TFldlibZonotopeOption<DoubleType>&& first, T second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * second)>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first.value * second)> result(first);
         result.normalizeForChange();
         if (active)
            *result.zonotope = result.zonotope->max(second.value);
         result.value = std::fmax(result.value, second);
         return result;
      }
   template<typename T> friend auto fmax(T first, const TFldlibZonotopeOption<DoubleType>& second)
         -> TFldlibZonotopeOption<decltype(first * DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first * second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->max(*second.zonotope);
            else
               *result.zonotope = result.zonotope->max(second.value);
         }
         result.value = std::fmax(result.value, second.value);
         return result;
      }
   template<typename T> friend auto fmax(T first, TFldlibZonotopeOption<DoubleType>&& second)
         -> TFldlibZonotopeOption<decltype(first * DoubleType(0))>
         requires std::floating_point<T> || std::integral<T>
      {  TFldlibZonotopeOption<decltype(first * second.value)> result(first);
         result.normalize();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->max(*second.zonotope);
            else
               *result.zonotope = result.zonotope->max(second.value);
         }
         result.value = std::fmax(result.value, second.value);
         return result;
      }
   friend thisType fmax(const thisType& first, const thisType& second)
      {  thisType result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->max(*second.zonotope);
            else
               *result.zonotope = result.zonotope->max(second.value);
         }
         result.value = std::fmax(result.value, second.value);
         return result;
      }
   friend thisType fmax(thisType&& first, const thisType& second)
      {  thisType result = std::move(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->max(*second.zonotope);
            else
               *result.zonotope = result.zonotope->max(second.value);
         }
         result.value = std::fmax(result.value, second.value);
         return result;
      }
   friend thisType fmax(const thisType& first, thisType&& second)
      {  thisType result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->max(*second.zonotope);
            else
               *result.zonotope = result.zonotope->max(second.value);
         }
         result.value = std::fmax(result.value, second.value);
         return result;
      }
   friend thisType fmax(thisType&& first, thisType&& second)
      {  thisType result = std::move(first);
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->max(*second.zonotope);
            else
               *result.zonotope = result.zonotope->max(second.value);
         }
         result.value = std::fmax(result.value, second.value);
         return result;
      }
   template<typename T> friend auto fmax(const thisType& first, const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->max(*second.zonotope);
            else
               *result.zonotope = result.zonotope->max(second.value);
         }
         result.value = std::fmax(result.value, second.value);
         return result;
      }
   template<typename T> friend auto fmax(thisType&& first, const TFldlibZonotopeOption<T>& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->max(*second.zonotope);
            else
               *result.zonotope = result.zonotope->max(second.value);
         }
         result.value = std::fmax(result.value, second.value);
         return result;
      }
   template<typename T> friend auto fmax(const thisType& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->max(*second.zonotope);
            else
               *result.zonotope = result.zonotope->max(second.value);
         }
         result.value = std::fmax(result.value, second.value);
         return result;
      }
   template<typename T> friend auto fmax(thisType&& first, TFldlibZonotopeOption<T>&& second)
         -> TFldlibZonotopeOption<decltype(DoubleType(0) * T(0))>
         requires std::floating_point<T>
      {  TFldlibZonotopeOption<decltype(first.value * second.value)> result = first;
         result.normalizeForChange();
         if (active) {
            if (second.zonotope.get())
               *result.zonotope = result.zonotope->max(*second.zonotope);
            else
               *result.zonotope = result.zonotope->max(second.value);
         }
         result.value = std::fmax(result.value, second.value);
         return result;
      }

   friend int fld_isfinite(const thisType& source) { return std::isfinite(source.value); }
   friend int fld_isnan(const thisType& source) { return std::isnan(source.value); }
   friend int fld_isinf(const thisType& source) { return std::isinf(source.value); }

   friend int isfinite(const thisType& source) { return std::isfinite(source.value); }
   friend int isnan(const thisType& source) { return std::isnan(source.value); }
   friend int isinf(const thisType& source) { return std::isinf(source.value); }

   void write(std::ostream& out) const
      {  out << "value: " << value;
         if (active && zonotope.get()) {
            out << ", domain: ";
            STG::IOObject::OSStream outStream(out);
            zonotope->writeBasicInterval(outStream);
         }
      }
   void lightPersist(const char* prefix) const
      {  if (zonotope.get())
            zonotope->lightPersist(prefix);
      }
   void persist(const char* prefix) const
      {  if (zonotope.get())
            zonotope->persist(prefix);
      }
   friend std::ostream& operator<<(std::ostream& out, const TFldlibZonotopeOption<DoubleType>& domain)
      {  domain.write(out); return out; }
   friend std::istream& operator>>(std::istream& in, TFldlibZonotopeOption<DoubleType>& domain)
      {  throw NotYetImplemented(); return in; }
   typedef DAffine::BaseExecutionPath::end end;
   typedef anticipated_termination anticipated_termination;

   void mergeWith(const thisType& source)
      {  if (!zonotope)
            zonotope = std::make_shared<TFloatZonotope<DoubleType>>(value);
         if (source.zonotope)
            zonotope->mergeWith(*source.zonotope);
         else
            zonotope->mergeWith(TFloatZonotope<DoubleType>(value));
      }
   bool optimizeValue() { return zonotope ? zonotope->optimizeValue() : true; }
   typedef typename TFloatZonotope<DoubleType>::EquationHolder EquationHolder;
   struct EquationOption {
      EquationHolder* equation;
      void clearHolder() { if (equation) equation->clearHolder(); }
      void setHolder(PathExplorer* holder) { if (equation) equation->setHolder(holder); }
   };
   EquationOption getSRealDomain()
      {  return EquationOption { zonotope ? &zonotope->getSRealDomain() : nullptr }; }
   EquationOption getSError()
      {  return EquationOption { zonotope ? &zonotope->getSError() : nullptr }; }
   void addHighLevelUpdateError(DAffine::THighLevelUpdateVector<Equation>& highLevelUpdates,
         const BuiltReal& highLevelError)
      {  if (zonotope) zonotope->addHighLevelUpdateError(highLevelUpdates, highLevelError); }
   void updateLocalState() { if (zonotope) zonotope->updateLocalState(); }
   static void writeImplementationHeader(STG::IOObject::OSBase& outFile)
      {  TFloatZonotope<DoubleType>::writeImplementationHeader(outFile); }
   static void writeRealHeader(STG::IOObject::OSBase& outFile)
      {  TFloatZonotope<DoubleType>::writeRealHeader(outFile); }

   void readImplementationSynchronizationFromFile(STG::IOObject::ISBase& inFile,
         typename BuiltDouble::ReadParameters& readParameters,
         typename Equation::ReadParameters& equationReadParameters,
         STG::IOObject::OSBase& outFile,
         typename BuiltDouble::WriteParameters& writeParameters,
         typename Equation::WriteParameters& equationWriteParameters,
         SymbolsManager& symbolsManager, BuiltReal& highLevelError)
      {  if (zonotope)
            zonotope->readImplementationSynchronizationFromFile(inFile, readParameters,
                  equationReadParameters, outFile, writeParameters, equationWriteParameters,
                  symbolsManager, highLevelError);
      }
   void writeImplementationSynchronizationToFile(STG::IOObject::OSBase& outFile,
         const typename BuiltDouble::WriteParameters& writeParameters,
         const typename Equation::WriteParameters& equationWriteParameters) const
      {  if (zonotope)
            zonotope->writeImplementationSynchronizationToFile(outFile, writeParameters,
                  equationWriteParameters);
      }
   void readRealSynchronizationFromFile(STG::IOObject::ISBase& inFile,
         typename Equation::ReadParameters& equationReadParameters,
         STG::IOObject::OSBase& outFile,
         typename Equation::WriteParameters& equationWriteParameters,
         SymbolsManager& symbolsManager, BuiltReal& highLevelError)
      {  if (zonotope)
            zonotope->readRealSynchronizationFromFile(inFile, equationReadParameters, outFile,
                  equationWriteParameters, symbolsManager, highLevelError);
      }
   void writeRealSynchronizationToFile(STG::IOObject::OSBase& outFile,
         const typename Equation::WriteParameters& equationWriteParameters) const
      {  if (zonotope)
            zonotope->writeRealSynchronizationToFile(outFile, equationWriteParameters);
      }
   void readImplementationSynchronizationFromMemory(
         DAffine::BasePathExplorer::AbstractImplementationArray& implementationArray,
         DAffine::BasePathExplorer::AbstractEquationArray& equationArray,
         COL::TVector<int>& codeArray, SymbolsManager& symbolsManager, BuiltReal& highLevelError)
      {  int hasZonotope = codeArray.first();
         codeArray.removeAt(0);
         if (hasZonotope) {
            AssumeCondition(hasZonotope == 2)
            if (!zonotope)
               zonotope.reset(new TFloatZonotope<DoubleType>());
            else {
               int numberOfBounds = codeArray.first();
               codeArray.removeAt(0);
               int numberOfEquations = codeArray.first();
               codeArray.removeAt(0);
               AssumeCondition(numberOfBounds == 2 && numberOfEquations == 1)
            }
            codeArray.insertAtEnd(2); // 2 codes
            codeArray.insertAtEnd(2); // 2 bounds for the implementation
            codeArray.insertAtEnd(1); // 1 equation for the real
            zonotope->readImplementationSynchronizationFromMemory(implementationArray,
                  equationArray, codeArray, symbolsManager, highLevelError);
         }
         else {
            zonotope.reset();
            codeArray.insertAtEnd(0); // 0 code
         }
      }
   void writeImplementationSynchronizationToMemory(
         DAffine::BasePathExplorer::AbstractImplementationArray& implementationArray,
         DAffine::BasePathExplorer::AbstractEquationArray& equationArray, COL::TVector<int>& codeArray) const
      {  if (zonotope) {
            zonotope->writeImplementationSynchronizationToMemory(implementationArray,
                  equationArray, codeArray);
            codeArray.insertAtEnd(2); // 2 codes
            codeArray.insertAtEnd(2); // 2 bounds for the implementation
            codeArray.insertAtEnd(1); // 1 equation for the real
         }
         else
            codeArray.insertAtEnd(0); // 0 code
      }
   void readRealSynchronizationFromMemory(
         DAffine::BasePathExplorer::AbstractEquationArray& equationArray,
         COL::TVector<int>& codeArray, SymbolsManager& symbolsManager, BuiltReal& highLevelError)
      {  int hasZonotope = codeArray.first();
         codeArray.removeAt(0);
         if (hasZonotope) {
            AssumeCondition(hasZonotope == 1)
            if (!zonotope)
               zonotope.reset(new TFloatZonotope<DoubleType>());
            else {
               int numberOfEquations = codeArray.first();
               codeArray.removeAt(0);
               AssumeCondition(numberOfEquations == 1)
            }
            codeArray.insertAtEnd(1); // 1 code
            codeArray.insertAtEnd(1); // 1 equation for the real
            zonotope->readRealSynchronizationFromMemory(equationArray, codeArray,
                  symbolsManager, highLevelError);
         }
         else {
            zonotope.reset();
            codeArray.insertAtEnd(0); // 0 code
         }
      }
   void writeRealSynchronizationToMemory(DAffine::BasePathExplorer::AbstractEquationArray& equationArray,
         COL::TVector<int>& codeArray) const
      {  if (zonotope) {
            zonotope->writeRealSynchronizationToMemory(equationArray, codeArray);
            codeArray.insertAtEnd(1); // 1 code
            codeArray.insertAtEnd(1); // 1 equation for the real
         }
         else
            codeArray.insertAtEnd(0); // 0 code
      }
   void moveImplementationInMemory(
         DAffine::BasePathExplorer::AbstractImplementationArray& implementationArray,
         DAffine::BasePathExplorer::AbstractEquationArray& equationArray, COL::TVector<int>& codeArray) const
      {  int hasZonotope = codeArray.first();
         codeArray.removeAt(0);
         if (hasZonotope) {
            AssumeCondition(hasZonotope == 2 && zonotope)
            int numberOfBounds = codeArray.first();
            codeArray.removeAt(0);
            int numberOfEquations = codeArray.first();
            codeArray.removeAt(0);
            AssumeCondition(numberOfBounds == 2 && numberOfEquations == 1)
            codeArray.insertAtEnd(2); // 2 codes
            codeArray.insertAtEnd(2); // 2 bounds for the implementation
            codeArray.insertAtEnd(1); // 1 equation for the real
            zonotope->moveImplementationInMemory(implementationArray, equationArray, codeArray);
         }
         else {
            AssumeCondition(!zonotope)
            codeArray.insertAtEnd(0); // 0 code
         }
      }
   void moveRealInMemory(DAffine::BasePathExplorer::AbstractEquationArray& equationArray, COL::TVector<int>& codeArray) const
      {  int hasZonotope = codeArray.first();
         codeArray.removeAt(0);
         if (hasZonotope) {
            AssumeCondition(hasZonotope == 1 && zonotope)
            int numberOfEquations = codeArray.first();
            codeArray.removeAt(0);
            AssumeCondition(numberOfEquations == 1)
            codeArray.insertAtEnd(1); // 1 code
            codeArray.insertAtEnd(1); // 1 equation for the real
            zonotope->moveRealInMemory(equationArray, codeArray);
         }
         else {
            AssumeCondition(!zonotope)
            codeArray.insertAtEnd(0); // 0 code
         }
      }
   typedef thisType InstrumentedAffineType;
   void cloneShareParts() {  normalizeForChange(); }
};

template <typename IntegerType> requires std::integral<IntegerType>
class TFldlibIntegerBranchOption : public FldlibBase {
  public:
   IntegerType value;
   std::shared_ptr<TValueWithBranches<IntegerType>> conditionalValue;
   static const bool isBranch = true;

   typedef TFloatZonotope<double>::BuiltDouble BuiltDouble;
   void updateLocalState() {}
   std::shared_ptr<TIntegerWithBranches<IntegerType>> sconditionalValue() const
      {  return std::static_pointer_cast<TIntegerWithBranches<IntegerType>>(conditionalValue); }
   TIntegerWithBranches<IntegerType>& cconditionalValue() const
      {  return static_cast<TIntegerWithBranches<IntegerType>&>(*conditionalValue); }
   class BranchCursor : public FldlibBase::BranchCursor {
     private:
      std::vector<std::pair<IntegerType*, std::shared_ptr<TValueWithBranches<IntegerType>>*>> vibPathParent;
      TFldlibIntegerBranchOption* pibSupport = nullptr;

     public:
      BranchCursor() = default;
      BranchCursor(const TFldlibIntegerBranchOption<IntegerType>& support)
         :  pibSupport(&const_cast<TFldlibIntegerBranchOption<IntegerType>&>(support)) {}
      BranchCursor(const BranchCursor&) = default;
      BranchCursor(BranchCursor&&) = default;
      BranchCursor& operator=(const BranchCursor&) = default;
      BranchCursor& operator=(BranchCursor&&) = default;

      IntegerType& elementSAt()
         {  AssumeCondition(!vibPathParent.empty() && !vibPathParent.back().second->get())
            return *vibPathParent.back().first;
         }
      const IntegerType& elementAt() const
         {  AssumeCondition(!vibPathParent.empty() && !vibPathParent.back().second->get())
            return *vibPathParent.back().first;
         }
      bool setToFirst();
      bool setToNext();
   };

  private:
   typedef TFldlibIntegerBranchOption<IntegerType> thisType;

  public:
   TFldlibIntegerBranchOption() = default;
   TFldlibIntegerBranchOption(IntegerType thisValue) : value(thisValue) {}
   TFldlibIntegerBranchOption(IntegerType thisValue, const std::shared_ptr<TValueWithBranches<IntegerType>>& thisConditionalValue)
      :  value(thisValue), conditionalValue(thisConditionalValue) {}
// template<typename T> TFldlibIntegerBranchOption(T source) requires std::integral<T>
//    :  value(source) {}
   TFldlibIntegerBranchOption(const thisType& source) = default;
   TFldlibIntegerBranchOption(thisType&& source) = default;
   template <typename T> TFldlibIntegerBranchOption(const TFldlibIntegerBranchOption<T>& source)
      :  value(source.value), conditionalValue(source.conditionalValue.get()
            ?  new TIntegerWithBranches<IntegerType>(source.cconditionalValue()) : nullptr) {}
   template <typename T> TFldlibIntegerBranchOption(TFldlibIntegerBranchOption<T>&& source)
      :  value(source.value), conditionalValue(source.conditionalValue.get()
            ?  new TIntegerWithBranches<IntegerType>(source.cconditionalValue()) : nullptr) {}

// template<typename T> thisType& operator=(T source) requires std::integral<T>
//    {  value = source;
//       conditionalValue.reset();
//       return *this;
//    }
   thisType& operator=(const thisType& source) = default;
   thisType& operator=(thisType&& source) = default;
   template<typename T> thisType& operator=(const TFldlibIntegerBranchOption<T>& source)
         requires std::integral<T>
      {  value = source.value;
         conditionalValue.reset(source.conditionalValue.get()
            ? new TIntegerWithBranches<IntegerType>(source.cconditionalValue()) : nullptr);
         return *this;
      }
   template<typename T> thisType& operator=(TFldlibIntegerBranchOption<T>&& source)
         requires std::integral<T>
      {  value = source.value;
         conditionalValue.reset(source.conditionalValue.get()
            ? new TIntegerWithBranches<IntegerType>(source.cconditionalValue()) : nullptr);
         return *this;
      }
   void conditionalAssigns(const std::vector<std::pair<int, bool>>& path, const thisType& source);

   IntegerType getMaxValue() const
      {  IntegerType result = std::numeric_limits<IntegerType>::min();
         if (conditionalValue)
            conditionalValue->apply([&result](const IntegerType& val)
               {  if (val > result)
                     result = val;
               });
         else
            result = value;
         return result;
      }
   IntegerType getMinValue() const
      {  IntegerType result = std::numeric_limits<IntegerType>::max();
         if (conditionalValue)
            conditionalValue->apply([&result](const IntegerType& val)
               {  if (val < result)
                     result = val;
               });
         else
            result = value;
         return result;
      }
   thisType& operator++()
      {  ++value;
         if (conditionalValue.get()) {
            if (conditionalValue.use_count() > 1)
               conditionalValue.reset(new TIntegerWithBranches<IntegerType>(cconditionalValue()));
            cconditionalValue().applyAssign([](IntegerType& arg) { ++arg; });
         }
         return *this;
      }
   thisType& operator++(int)
      {  ++value;
         if (conditionalValue.get()) {
            if (conditionalValue.use_count() > 1)
               conditionalValue.reset(new TIntegerWithBranches<IntegerType>(cconditionalValue()));
            cconditionalValue().applyAssign([](IntegerType& arg) { ++arg; });
         }
         return *this;
      }
   thisType& operator--()
      {  --value;
         if (conditionalValue.get()) {
            if (conditionalValue.use_count() > 1)
               conditionalValue.reset(new TIntegerWithBranches<IntegerType>(cconditionalValue()));
            cconditionalValue().applyAssign([](IntegerType& arg) { --arg; });
         }
         return *this;
      }
   thisType& operator--(int)
      {  --value;
         if (conditionalValue.get()) {
            if (conditionalValue.use_count() > 1)
               conditionalValue.reset(new TIntegerWithBranches<IntegerType>(cconditionalValue()));
            cconditionalValue().applyAssign([](IntegerType& arg) { --arg; });
         }
         return *this;
      }

   friend thisType operator+(const thisType& first) { return first; }
   friend thisType operator+(thisType&& first) { return std::move(first); }

   template<typename T> thisType& operator+=(T source)
         requires std::floating_point<T> || std::integral<T>;
   thisType& operator+=(const thisType& source);
   thisType& operator+=(thisType&& source);
   template<typename T> thisType& operator+=(const TFldlibIntegerBranchOption<T>& source)
         requires std::integral<T>;
   template<typename T> thisType& operator+=(TFldlibIntegerBranchOption<T>&& source)
         requires std::integral<T>;
   template<typename T> auto operator+(T second) const -> TFldlibIntegerBranchOption<decltype(IntegerType(0) + typename IntegralType<T>::base_type(0))>
         requires std::integral<T>;
   friend thisType operator+(const thisType& first, const thisType& second)
      {  thisType result(first); result += second; return result; }
   friend thisType operator+(thisType&& first, const thisType& second)
      {  thisType result(std::move(first)); result += second; return result; }
   friend thisType operator+(const thisType& first, thisType&& second)
      {  thisType result(first); result += second; return result; }
   friend thisType operator+(thisType&& first, thisType&& second)
      {  thisType result(std::move(first)); result += second; return result; }
   template<typename T> friend auto operator+(const thisType& first,
         const TFldlibIntegerBranchOption<T>& second)
      {  thisType result(first);
         result.value += second;
         return result;
      }

// friend thisType operator-(const thisType& first);
// friend thisType operator-(thisType&& first);

   template<typename T> thisType& operator-=(T source)
         requires std::floating_point<T> || std::integral<T>;
   thisType& operator-=(const thisType& source);
   thisType& operator-=(thisType&& source);
   template<typename T> thisType& operator-=(const TFldlibIntegerBranchOption<T>& source)
         requires std::integral<T>;
   template<typename T> thisType& operator-=(TFldlibIntegerBranchOption<T>&& source)
         requires std::integral<T>;
   friend thisType operator-(const thisType& first, const thisType& second)
      {  thisType result(first); result -= second; return result; }
   friend thisType operator-(thisType&& first, const thisType& second)
      {  thisType result(std::move(first)); result -= second; return result; }
   friend thisType operator-(const thisType& first, thisType&& second)
      {  thisType result(first); result -= second; return result; }
   friend thisType operator-(thisType&& first, thisType&& second)
      {  thisType result(std::move(first)); result -= second; return result; }

   template<typename T> TFldlibIntegerBranchOption<IntegerType>& operator*=(T source)
         requires std::floating_point<T> || std::integral<T>;
   thisType& operator*=(const thisType& source);
   thisType& operator*=(thisType&& source);
   template<typename T> thisType& operator*=(const TFldlibIntegerBranchOption<T>& source)
         requires std::integral<T>;
   template<typename T> thisType& operator*=(TFldlibIntegerBranchOption<T>&& source)
         requires std::integral<T>;
   friend thisType operator*(const thisType& first, const thisType& second)
      {  thisType result(first); result *= second; return result; }
   friend thisType operator*(thisType&& first, const thisType& second)
      {  thisType result(std::move(first)); result *= second; return result; }
   friend thisType operator*(const thisType& first, thisType&& second)
      {  thisType result(first); result *= second; return result; }
   friend thisType operator*(thisType&& first, thisType&& second)
      {  thisType result(std::move(first)); result *= second; return result; }

   template<typename T> thisType& operator/=(T source)
         requires std::floating_point<T> || std::integral<T>;
   thisType& operator/=(const thisType& source);
   thisType& operator/=(thisType&& source);
   template<typename T> thisType& operator/=(const TFldlibIntegerBranchOption<T>& source)
         requires std::integral<T>;
   template<typename T> thisType& operator/=(TFldlibIntegerBranchOption<T>&& source)
         requires std::integral<T>;
   friend thisType operator/(const thisType& first, const thisType& second)
      {  thisType result(first); result /= second; return result; }
   friend thisType operator/(thisType&& first, const thisType& second)
      {  thisType result(std::move(first)); result /= second; return result; }
   friend thisType operator/(const thisType& first, thisType&& second)
      {  thisType result(first); result /= second; return result; }
   friend thisType operator/(thisType&& first, thisType&& second)
      {  thisType result(std::move(first)); result /= second; return result; }

   template<typename T> thisType& operator%=(T source)
         requires std::integral<T>;
   thisType& operator%=(const thisType& source);
   thisType& operator%=(thisType&& source);
   template<typename T> thisType& operator%=(const TFldlibIntegerBranchOption<T>& source)
         requires std::integral<T>;
   template<typename T> thisType& operator%=(TFldlibIntegerBranchOption<T>&& source)
         requires std::integral<T>;
   friend thisType operator%(const thisType& first, const thisType& second)
      {  thisType result(first); result %= second; return result; }
   friend thisType operator%(thisType&& first, const thisType& second)
      {  thisType result(std::move(first)); result %= second; return result; }
   friend thisType operator%(const thisType& first, thisType&& second)
      {  thisType result(first); result %= second; return result; }
   friend thisType operator%(thisType&& first, thisType&& second)
      {  thisType result(std::move(first)); result %= second; return result; }

   // operator space_shift to add
   // FullComparisonResult fullCompare(const thisType& source) const;
   template <typename T> requires std::integral<T>
   FullComparisonResult fullCompare(const TFldlibIntegerBranchOption<T>& source) const;
   template <typename T>
   bool constraintCompare(TFldlibIntegerBranchOption<T>& source, FullComparisonResult expectedResult
      /* , MergedExecutionPath& mergedExecutionPath could remove some BranchInformation in branchDictionary*/);
   static bool followNewBranch(FullComparisonResult comparisonResult, FullComparisonResult trueExpectedResult);

   TFldlibIntegerBranchOption<int> compareAsDomain(
         const TFldlibIntegerBranchOption<IntegerType>& source, FullComparisonResult result) const;
   template <typename T> requires std::integral<T> TFldlibIntegerBranchOption<int> compareAsDomain(
         const TFldlibIntegerBranchOption<T>& source, FullComparisonResult result) const;
   bool compareAsBool(
         TFldlibIntegerBranchOption<IntegerType>& source, FullComparisonResult result);
   template <typename T> requires std::integral<T> bool compareAsBool(
         TFldlibIntegerBranchOption<T>& source, FullComparisonResult result);
   template <typename T> friend TFldlibIntegerBranchOption<int> operator<(const thisType& first, T second)
         requires std::integral<T>
      // {  return first < TFldlibIntegerBranchOption<T>(second); }
      {  return first.compareAsDomain(TFldlibIntegerBranchOption<T>(second), FldlibBase::FCRLess); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator<(thisType&& first, T second)
         requires std::integral<T>
      {  return first.compareAsDomain(TFldlibIntegerBranchOption<T>(second), FldlibBase::FCRLess); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator<(T first, const thisType& second)
         requires std::integral<T>
      // {  return TFldlibIntegerBranchOption<T>(first) < second; }
      {  return TFldlibIntegerBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRLess); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator<(T first, thisType&& second)
         requires std::integral<T>
      {  return TFldlibIntegerBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRLess); }

   template <typename T> friend TFldlibIntegerBranchOption<int> operator<=(const thisType& first, T second)
         requires std::integral<T>
      // {  return first <= TFldlibIntegerBranchOption<T>(second); }
      {  return first.compareAsDomain(TFldlibIntegerBranchOption<T>(second), FldlibBase::FCRLessOrEqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator<=(thisType&& first, T second)
         requires std::integral<T>
      {  return first.compareAsDomain(TFldlibIntegerBranchOption<T>(second), FldlibBase::FCRLessOrEqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator<=(T first, const thisType& second)
         requires std::integral<T>
      // {  return TFldlibIntegerBranchOption<T>(first) <= second; }
      {  return TFldlibIntegerBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRLessOrEqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator<=(T first, thisType&& second)
         requires std::integral<T>
      {  return TFldlibIntegerBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRLessOrEqual); }

   template <typename T> friend TFldlibIntegerBranchOption<int> operator==(const thisType& first, T second)
         requires std::integral<T>
      // {  return first == TFldlibIntegerBranchOption<T>(second); }
      {  return first.compareAsDomain(TFldlibIntegerBranchOption<T>(second), FldlibBase::FCREqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator==(thisType&& first, T second)
         requires std::integral<T>
      {  return first.compareAsDomain(TFldlibIntegerBranchOption<T>(second), FldlibBase::FCREqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator==(T first, const thisType& second)
         requires std::integral<T>
      // {  return TFldlibIntegerBranchOption<T>(first) == second; }
      {  return TFldlibIntegerBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCREqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator==(T first, thisType&& second)
         requires std::integral<T>
      {  return TFldlibIntegerBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCREqual); }

   template <typename T> friend TFldlibIntegerBranchOption<int> operator!=(const thisType& first, T second)
         requires std::integral<T>
      // {  return first != TFldlibIntegerBranchOption<T>(second); }
      {  return first.compareAsDomain(TFldlibIntegerBranchOption<T>(second), FldlibBase::FCRDifferent); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator!=(thisType&& first, T second)
         requires std::integral<T>
      {  return first.compareAsDomain(TFldlibIntegerBranchOption<T>(second), FldlibBase::FCRDifferent); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator!=(T first, const thisType& second)
         requires std::integral<T>
      // {  return TFldlibIntegerBranchOption<T>(first) != second; }
      {  return TFldlibIntegerBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRDifferent); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator!=(T first, thisType&& second)
         requires std::integral<T>
      {  return TFldlibIntegerBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRDifferent); }

   template <typename T> friend TFldlibIntegerBranchOption<int> operator>=(const thisType& first, T second)
         requires std::integral<T>
      // {  return first >= TFldlibIntegerBranchOption<T>(second); }
      {  return first.compareAsDomain(TFldlibIntegerBranchOption<T>(second), FldlibBase::FCRGreaterOrEqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator>=(thisType&& first, T second)
         requires std::integral<T>
      {  return first.compareAsDomain(TFldlibIntegerBranchOption<T>(second), FldlibBase::FCRGreaterOrEqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator>=(T first, const thisType& second)
         requires std::integral<T>
      // {  return TFldlibIntegerBranchOption<T>(first) >= second; }
      {  return TFldlibIntegerBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRGreaterOrEqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator>=(T first, thisType&& second)
         requires std::integral<T>
      {  return TFldlibIntegerBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRGreaterOrEqual); }

   template <typename T> friend TFldlibIntegerBranchOption<int> operator>(const thisType& first, T second)
         requires std::integral<T>
      // {  return first > TFldlibIntegerBranchOption<T>(second); }
      {  return first.compareAsDomain(TFldlibIntegerBranchOption<T>(second), FldlibBase::FCRGreater); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator>(thisType&& first, T second)
         requires std::integral<T>
      {  return first.compareAsDomain(TFldlibIntegerBranchOption<T>(second), FldlibBase::FCRGreater); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator>(T first, const thisType& second)
         requires std::integral<T>
      // {  return TFldlibIntegerBranchOption<T>(first) > second; }
      {  return TFldlibIntegerBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRGreater); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator>(T first, thisType&& second)
         requires std::integral<T>
      {  return TFldlibIntegerBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRGreater); }
   thisType min(const thisType& source) const;
   thisType min(thisType&& source) const;
   thisType& minAssign(const thisType& source);
   thisType max(const thisType& source) const;
   thisType max(thisType&& source) const;
   thisType& maxAssign(const thisType& source);

   void mergeWith(int mergeBranchIndex, thisType& source);
   void mergeWith(thisType& source) { mergeWith(FldlibBase::splitBranchIdentifier, source); }
   bool optimizeValue() { return true; }
   thisType& getSRealDomain() { return *this; }
   thisType& getSError() { return *this; }
   void clearHolder() {}
   typedef DAffine::SymbolsManager SymbolsManager;
   typedef typename TFloatZonotope<double>::BuiltReal BuiltReal;
   void addHighLevelUpdateError(DAffine::THighLevelUpdateVector<Equation>& highLevelUpdates,
         const BuiltReal& highLevelError) {}
   static void writeImplementationHeader(STG::IOObject::OSBase& outFile)
      {  TFldlibZonotopeOption<double>::writeImplementationHeader(outFile); }
   static void writeRealHeader(STG::IOObject::OSBase& outFile)
      {  TFldlibZonotopeOption<double>::writeRealHeader(outFile); }
   void readImplementationSynchronizationFromFile(STG::IOObject::ISBase& inFile,
         typename BuiltDouble::ReadParameters& readParameters,
         typename Equation::ReadParameters& equationReadParameters,
         STG::IOObject::OSBase& outFile,
         typename BuiltDouble::WriteParameters& writeParameters,
         typename Equation::WriteParameters& equationWriteParameters,
         SymbolsManager& symbolsManager, BuiltReal& highLevelError);
   void writeImplementationSynchronizationToFile(STG::IOObject::OSBase& outFile,
         const typename BuiltDouble::WriteParameters& writeParameters,
         const typename Equation::WriteParameters& equationWriteParameters) const;
   void readRealSynchronizationFromFile(STG::IOObject::ISBase& inFile,
         typename Equation::ReadParameters& equationReadParameters,
         STG::IOObject::OSBase& outFile,
         typename Equation::WriteParameters& equationWriteParameters,
         SymbolsManager& symbolsManager, BuiltReal& highLevelError);
   void writeRealSynchronizationToFile(STG::IOObject::OSBase& outFile,
         const typename Equation::WriteParameters& equationWriteParameters) const;
   void readImplementationSynchronizationFromMemory(
         DAffine::BasePathExplorer::AbstractImplementationArray& implementationArray,
         DAffine::BasePathExplorer::AbstractEquationArray& equationArray,
         COL::TVector<int>& codeArray, SymbolsManager& symbolsManager, BuiltReal& highLevelError);
   void writeImplementationSynchronizationToMemory(
         DAffine::BasePathExplorer::AbstractImplementationArray& implementationArray,
         DAffine::BasePathExplorer::AbstractEquationArray& equationArray, COL::TVector<int>& codeArray) const;
   void readRealSynchronizationFromMemory(
         DAffine::BasePathExplorer::AbstractEquationArray& equationArray,
         COL::TVector<int>& codeArray, SymbolsManager& symbolsManager, BuiltReal& highLevelError);
   void writeRealSynchronizationToMemory(DAffine::BasePathExplorer::AbstractEquationArray& equationArray,
         COL::TVector<int>& codeArray) const;
   void moveImplementationInMemory(
         DAffine::BasePathExplorer::AbstractImplementationArray& implementationArray,
         DAffine::BasePathExplorer::AbstractEquationArray& equationArray, COL::TVector<int>& codeArray) const;
   void moveRealInMemory(DAffine::BasePathExplorer::AbstractEquationArray& equationArray, COL::TVector<int>& codeArray) const;

// void write(std::ostream& out) const
//    {  out << "value: " << value; }
// void lightPersist(const char* prefix) const
//    {  if (zonotope.get())
//          zonotope->lightPersist(prefix);
//    }
// void persist(const char* prefix) const
//    {  if (zonotope.get())
//          zonotope->persist(prefix);
//    }
// friend std::ostream& operator<<(std::ostream& out, const TFldlibIntegerBranchOption<IntegerType>& source)
//    {  source.write(out); return out; }
// friend std::istream& operator>>(std::istream& in, TFldlibIntegerBranchOption<IntegerType>& source)
//    {  throw NotYetImplemented(); return in; }
   // typedef thisType InstrumentedAffineType;
   // void cloneShareParts() {  normalizeForChange(); }
};

template <typename FloatingType> requires enhanced_floating_point<FloatingType>
class TFldlibBaseFloatingBranchOption : public FldlibBase {
  public:
   FloatingType value;
   std::shared_ptr<TValueWithBranches<FloatingType>> conditionalValue;
   std::shared_ptr<TFloatingWithBranches<FloatingType>> sconditionalValue() const
      {  return std::static_pointer_cast<TFloatingWithBranches<FloatingType>>(conditionalValue); }
   TFloatingWithBranches<FloatingType>& cconditionalValue() const
      {  return static_cast<TFloatingWithBranches<FloatingType>&>(*conditionalValue); }

   static const bool isBranch = true;
   class BranchCursor : public FldlibBase::BranchCursor {
     private:
      std::vector<std::pair<FloatingType*, std::shared_ptr<TValueWithBranches<FloatingType>>*>> vibPathParent;
      TFldlibBaseFloatingBranchOption* pibSupport = nullptr;

     public:
      BranchCursor() = default;
      BranchCursor(const TFldlibBaseFloatingBranchOption<FloatingType>& support)
         :  pibSupport(&const_cast<TFldlibBaseFloatingBranchOption<FloatingType>&>(support)) {}
      BranchCursor(const BranchCursor&) = default;
      BranchCursor(BranchCursor&&) = default;
      BranchCursor& operator=(const BranchCursor&) = default;
      BranchCursor& operator=(BranchCursor&&) = default;

      FloatingType& elementSAt()
         {  AssumeCondition(!vibPathParent.empty() && !vibPathParent.back().second->get())
            return *vibPathParent.back().first;
         }
      const FloatingType& elementAt() const
         {  AssumeCondition(!vibPathParent.empty() && !vibPathParent.back().second->get())
            return *vibPathParent.back().first;
         }
      bool setToFirst();
      bool setToNext();
   };

  private:
   typedef TFldlibBaseFloatingBranchOption<FloatingType> thisType;

  public:
   TFldlibBaseFloatingBranchOption() = default;
   TFldlibBaseFloatingBranchOption(const FloatingType& thisValue) : value(thisValue) {}
   TFldlibBaseFloatingBranchOption(const FloatingType& thisValue,
         std::shared_ptr<TValueWithBranches<FloatingType>>& thisConditionalValue)
      :  value(thisValue), conditionalValue(thisConditionalValue) {}
   TFldlibBaseFloatingBranchOption(FloatingType&& thisValue) : value(std::move(thisValue)) {}
// template<typename T> TFldlibBaseFloatingBranchOption(T source) requires enhanced_floating_point<T>
//    :  value(source) {}
   template<typename IntegerType> requires std::integral<IntegerType>
   TFldlibBaseFloatingBranchOption(const TFldlibIntegerBranchOption<IntegerType>& source)
      :  value(source.value)
      {  if (source.conditionalValue)
            conditionalValue.reset(new TValueWithBranches<FloatingType>(*source.conditionalValue));
      }
   TFldlibBaseFloatingBranchOption(const thisType& source) = default;
   TFldlibBaseFloatingBranchOption(thisType&& source) = default;
   template <typename T> TFldlibBaseFloatingBranchOption(const TFldlibFloatingBranchOption<T>& source)
      :  value(source.value), conditionalValue(source.conditionalValue.get()
            ?  new TFloatingWithBranches<FloatingType>(source.cconditionalValue()) : nullptr) {}
   template <typename T> TFldlibBaseFloatingBranchOption(TFldlibFloatingBranchOption<T>&& source)
      :  value(source.value), conditionalValue(source.conditionalValue.get()
            ?  new TFloatingWithBranches<FloatingType>(source.cconditionalValue()) : nullptr) {}
   template <typename T> TFldlibBaseFloatingBranchOption(const TFldlibIntegerBranchOption<T>& source)
      :  value(source.value), conditionalValue(source.conditionalValue.get()
            ?  new TFloatingWithBranches<FloatingType>(source.cconditionalValue()) : nullptr) {}
   template <typename T> TFldlibBaseFloatingBranchOption(TFldlibIntegerBranchOption<T>&& source)
      :  value(source.value), conditionalValue(source.conditionalValue.get()
            ?  new TFloatingWithBranches<FloatingType>(source.cconditionalValue()) : nullptr) {}
   template <typename T> TFldlibBaseFloatingBranchOption(T source) requires std::floating_point<T>
      :  value(source) {}

// template<typename T> thisType& operator=(const T& source) requires enhanced_floating_point<T>
//    {  value = source;
//       conditionalValue.reset();
//       return *this;
//    }
// template<typename T> thisType& operator=(T&& source) requires enhanced_floating_point<T>
//    {  value = std::move(source);
//       conditionalValue.reset();
//       return *this;
//    }
   thisType& operator=(const thisType& source) = default;
   thisType& operator=(thisType&& source) = default;
   template<typename T> thisType& operator=(const TFldlibBaseFloatingBranchOption<T>& source)
         requires enhanced_floating_point<T>
      {  value = source.value;
         conditionalValue.reset(source.conditionalValue.get()
            ? new TFloatingWithBranches<FloatingType>(source.cconditionalValue()) : nullptr);
         return *this;
      }
   template<typename T> thisType& operator=(TFldlibBaseFloatingBranchOption<T>&& source)
         requires enhanced_floating_point<T>
      {  value = source.value;
         conditionalValue.reset(source.conditionalValue.get()
            ? new TFloatingWithBranches<FloatingType>(source.cconditionalValue()) : nullptr);
         return *this;
      }
   void conditionalAssigns(const std::vector<std::pair<int, bool>>& path, const thisType& source);

   thisType& operator++()
      {  ++value;
         if (conditionalValue.get()) {
            if (conditionalValue.use_count() > 1)
               conditionalValue.reset(new TFloatingWithBranches<FloatingType>(cconditionalValue()));
            cconditionalValue().applyAssign([](FloatingType& arg) { ++arg; });
         }
         return *this;
      }
   thisType& operator++(int)
      {  ++value;
         if (conditionalValue.get()) {
            if (conditionalValue.use_count() > 1)
               conditionalValue.reset(new TFloatingWithBranches<FloatingType>(cconditionalValue()));
            cconditionalValue().applyAssign([](FloatingType& arg) { ++arg; });
         }
         return *this;
      }

   friend thisType operator+(const thisType& first) { return first; }
   friend thisType operator+(thisType&& first) { return std::move(first); }

   template<typename T> thisType& operator+=(T source)
         requires enhanced_floating_point<T> || std::integral<T>;
   thisType& operator+=(const thisType& source);
   thisType& operator+=(thisType&& source);
   template<typename T> thisType& operator+=(const TFldlibBaseFloatingBranchOption<T>& source)
         requires enhanced_floating_point<T>;
   template<typename T> thisType& operator+=(TFldlibBaseFloatingBranchOption<T>&& source)
         requires enhanced_floating_point<T>;
   template<typename T> auto operator+(T second) const -> TFldlibBaseFloatingBranchOption<decltype(FloatingType(0) + typename EFloatingPointType<T>::base_type(0))>
         requires enhanced_floating_point<T>;
   friend thisType operator+(const thisType& first, const thisType& second)
      {  thisType result(first); result += second; return result; }
   friend thisType operator+(thisType&& first, const thisType& second)
      {  thisType result(std::move(first)); result += second; return result; }
   friend thisType operator+(const thisType& first, thisType&& second)
      {  thisType result(first); result += second; return result; }
   friend thisType operator+(thisType&& first, thisType&& second)
      {  thisType result(std::move(first)); result += second; return result; }
   template<typename T> friend auto operator+(const thisType& first,
         const TFldlibBaseFloatingBranchOption<T>& second)
      {  thisType result(first);
         result.value += second;
         return result;
      }

// friend thisType operator-(const thisType& first);
// friend thisType operator-(thisType&& first);
   template<typename T> thisType& operator-=(T source)
         requires enhanced_floating_point<T> || std::integral<T>;
   thisType& operator-=(const thisType& source);
   thisType& operator-=(thisType&& source);
   template<typename T> thisType& operator-=(const TFldlibBaseFloatingBranchOption<T>& source)
         requires enhanced_floating_point<T>;
   template<typename T> thisType& operator-=(TFldlibBaseFloatingBranchOption<T>&& source)
         requires enhanced_floating_point<T>;
   friend thisType operator-(const thisType& first, const thisType& second)
      {  thisType result(first); result -= second; return result; }
   friend thisType operator-(thisType&& first, const thisType& second)
      {  thisType result(std::move(first)); result -= second; return result; }
   friend thisType operator-(const thisType& first, thisType&& second)
      {  thisType result(first); result -= second; return result; }
   friend thisType operator-(thisType&& first, thisType&& second)
      {  thisType result(std::move(first)); result -= second; return result; }

   template<typename T> TFldlibBaseFloatingBranchOption<FloatingType>& operator*=(T source)
         requires enhanced_floating_point<T> || std::integral<T>;
   thisType& operator*=(const thisType& source);
   thisType& operator*=(thisType&& source);
   template<typename T> thisType& operator*=(const TFldlibBaseFloatingBranchOption<T>& source)
         requires enhanced_floating_point<T>;
   template<typename T> thisType& operator*=(TFldlibBaseFloatingBranchOption<T>&& source)
         requires enhanced_floating_point<T>;
   friend thisType operator*(const thisType& first, const thisType& second)
      {  thisType result(first); result *= second; return result; }
   friend thisType operator*(thisType&& first, const thisType& second)
      {  thisType result(std::move(first)); result *= second; return result; }
   friend thisType operator*(const thisType& first, thisType&& second)
      {  thisType result(first); result *= second; return result; }
   friend thisType operator*(thisType&& first, thisType&& second)
      {  thisType result(std::move(first)); result *= second; return result; }

   template<typename T> thisType& operator/=(T source)
         requires enhanced_floating_point<T> || std::integral<T>;
   thisType& operator/=(const thisType& source);
   thisType& operator/=(thisType&& source);
   template<typename T> thisType& operator/=(const TFldlibBaseFloatingBranchOption<T>& source)
         requires enhanced_floating_point<T>;
   template<typename T> thisType& operator/=(TFldlibBaseFloatingBranchOption<T>&& source)
         requires enhanced_floating_point<T>;
   friend thisType operator/(const thisType& first, const thisType& second)
      {  thisType result(first); result /= second; return result; }
   friend thisType operator/(thisType&& first, const thisType& second)
      {  thisType result(std::move(first)); result /= second; return result; }
   friend thisType operator/(const thisType& first, thisType&& second)
      {  thisType result(first); result /= second; return result; }
   friend thisType operator/(thisType&& first, thisType&& second)
      {  thisType result(std::move(first)); result /= second; return result; }

   // operator space_shift to add
   // FullComparisonResult fullCompare(const thisType& source) const;
   template <typename T> requires enhanced_floating_point<T>
   FullComparisonResult fullCompare(const TFldlibBaseFloatingBranchOption<T>& source) const;
   template <typename T>
   bool constraintCompare(TFldlibBaseFloatingBranchOption<T>& source, FullComparisonResult expectedResult
      /* , MergedExecutionPath& mergedExecutionPath could remove some BranchInformation in branchDictionary*/);
   static bool followNewBranch(FullComparisonResult comparisonResult, FullComparisonResult trueExpectedResult);
   std::vector<int> fullDiscreteConvert() const;
   TFldlibIntegerBranchOption<int> compareAsDomain(
         const TFldlibBaseFloatingBranchOption<FloatingType>& source, FullComparisonResult result) const;
   template <typename T> requires enhanced_floating_point<T> TFldlibIntegerBranchOption<int> compareAsDomain(
         const TFldlibBaseFloatingBranchOption<T>& source, FullComparisonResult result) const;
   bool compareAsBool(
         TFldlibBaseFloatingBranchOption<FloatingType>& source, FullComparisonResult result);
   template <typename T> requires enhanced_floating_point<T> bool compareAsBool(
         TFldlibBaseFloatingBranchOption<T>& source, FullComparisonResult result);

   template <typename T> friend TFldlibIntegerBranchOption<int> operator<(const thisType& first, T second)
         requires enhanced_floating_point<T>
      {  return first.compareAsDomain(TFldlibBaseFloatingBranchOption<T>(second), FldlibBase::FCRLess); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator<(thisType&& first, T second)
         requires enhanced_floating_point<T>
      {  return first.compareAsDomain(TFldlibBaseFloatingBranchOption<T>(second), FldlibBase::FCRLess); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator<(T first, const thisType& second)
         requires enhanced_floating_point<T>
      {  return TFldlibBaseFloatingBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRLess); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator<(T first, thisType&& second)
         requires enhanced_floating_point<T>
      {  return TFldlibBaseFloatingBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRLess); }

   template <typename T> friend TFldlibIntegerBranchOption<int> operator<=(const thisType& first, T second)
         requires enhanced_floating_point<T>
      {  return first.compareAsDomain(TFldlibBaseFloatingBranchOption<T>(second), FldlibBase::FCRLessOrEqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator<=(thisType&& first, T second)
         requires enhanced_floating_point<T>
      {  return first.compareAsDomain(TFldlibBaseFloatingBranchOption<T>(second), FldlibBase::FCRLessOrEqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator<=(T first, const thisType& second)
         requires enhanced_floating_point<T>
      {  return TFldlibBaseFloatingBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRLessOrEqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator<=(T first, thisType&& second)
         requires enhanced_floating_point<T>
      {  return TFldlibBaseFloatingBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRLessOrEqual); }

   template <typename T> friend TFldlibIntegerBranchOption<int> operator==(const thisType& first, T second)
         requires enhanced_floating_point<T>
      {  return first.compareAsDomain(TFldlibBaseFloatingBranchOption<T>(second), FldlibBase::FCREqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator==(thisType&& first, T second)
         requires enhanced_floating_point<T>
      {  return first.compareAsDomain(TFldlibBaseFloatingBranchOption<T>(second), FldlibBase::FCREqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator==(T first, const thisType& second)
         requires enhanced_floating_point<T>
      {  return TFldlibBaseFloatingBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCREqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator==(T first, thisType&& second)
         requires enhanced_floating_point<T>
      {  return TFldlibBaseFloatingBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCREqual); }

   template <typename T> friend TFldlibIntegerBranchOption<int> operator!=(const thisType& first, T second)
         requires enhanced_floating_point<T>
      {  return first.compareAsDomain(TFldlibBaseFloatingBranchOption<T>(second), FldlibBase::FCRDifferent); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator!=(thisType&& first, T second)
         requires enhanced_floating_point<T>
      {  return first.compareAsDomain(TFldlibBaseFloatingBranchOption<T>(second), FldlibBase::FCRDifferent); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator!=(T first, const thisType& second)
         requires enhanced_floating_point<T>
      {  return TFldlibBaseFloatingBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRDifferent); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator!=(T first, thisType&& second)
         requires enhanced_floating_point<T>
      {  return TFldlibBaseFloatingBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRDifferent); }

   template <typename T> friend TFldlibIntegerBranchOption<int> operator>=(const thisType& first, T second)
         requires enhanced_floating_point<T>
      {  return first.compareAsDomain(TFldlibBaseFloatingBranchOption<T>(second), FldlibBase::FCRGreaterOrEqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator>=(thisType&& first, T second)
         requires enhanced_floating_point<T>
      {  return first.compareAsDomain(TFldlibBaseFloatingBranchOption<T>(second), FldlibBase::FCRGreaterOrEqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator>=(T first, const thisType& second)
         requires enhanced_floating_point<T>
      {  return TFldlibBaseFloatingBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRGreaterOrEqual); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator>=(T first, thisType&& second)
         requires enhanced_floating_point<T>
      {  return TFldlibBaseFloatingBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRGreaterOrEqual); }

   template <typename T> friend TFldlibIntegerBranchOption<int> operator>(const thisType& first, T second)
         requires enhanced_floating_point<T>
      {  return first.compareAsDomain(TFldlibBaseFloatingBranchOption<T>(second), FldlibBase::FCRGreater); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator>(thisType&& first, T second)
         requires enhanced_floating_point<T>
      {  return first.compareAsDomain(TFldlibBaseFloatingBranchOption<T>(second), FldlibBase::FCRGreater); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator>(T first, const thisType& second)
         requires enhanced_floating_point<T>
      {  return TFldlibBaseFloatingBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRGreater); }
   template <typename T> friend TFldlibIntegerBranchOption<int> operator>(T first, thisType&& second)
         requires enhanced_floating_point<T>
      {  return TFldlibBaseFloatingBranchOption<T>(first).compareAsDomain(second, FldlibBase::FCRGreater); }

   void mergeWith(int mergeBranchIndex, thisType& source);
   void lightPersist(const char* prefix)
      {  if (!conditionalValue)
            value.lightPersist(prefix);
         else {
            BranchCursor indexCursor(*this);
            while (indexCursor.setToNext()) {
               // [TODO] indexCursor.getPathAt().lightPersist(prefix);
               indexCursor.elementAt().lightPersist(prefix);
            }
         }
      }
   void persist(const char* prefix)
      {  if (!conditionalValue)
            value.persist(prefix);
         else {
            BranchCursor indexCursor(*this);
            while (indexCursor.setToNext()) {
               // [TODO] indexCursor.getPathAt().lightPersist(prefix);
               indexCursor.elementAt().persist(prefix);
            }
         }
      }
   void write(std::ostream& out) const
      {  out << "value: " << value;
         if (conditionalValue) {
            out << " (";
            BranchCursor indexCursor(*this);
            bool isFirst = true;
            while (indexCursor.setToNext()) {
               // [TODO] indexCursor.getPathAt().lightPersist(prefix);
               if (isFirst)
                  isFirst = false;
               else
                  out << ", ";
               out << indexCursor.elementAt();
            }
            out << ")";
         }
      }
   friend std::ostream& operator<<(std::ostream& out, const TFldlibBaseFloatingBranchOption<FloatingType>& source)
      {  source.write(out); return out; }
   friend std::istream& operator>>(std::istream& in, TFldlibBaseFloatingBranchOption<FloatingType>& source)
      {  throw NotYetImplemented(); return in; }

   void updateLocalState()
      {  value.updateLocalState();
         if (conditionalValue.get()) {
            if (conditionalValue.use_count() > 1)
               conditionalValue.reset(new TFloatingWithBranches<FloatingType>(cconditionalValue()));
            cconditionalValue().applyAssign([](FloatingType& arg) { arg.updateLocalState(); });
         }
      }
};

template <typename FloatingType> requires enhanced_floating_point<FloatingType>
class TFldlibFloatingBranchOption : public TFldlibBaseFloatingBranchOption<FloatingType> {
  private:
   typedef TFldlibBaseFloatingBranchOption<FloatingType> inherited;
   typedef TFldlibFloatingBranchOption<FloatingType> thisType;
  public:
   TFldlibFloatingBranchOption() = default;
   TFldlibFloatingBranchOption(const FloatingType& thisValue) : inherited(thisValue) {}
   TFldlibFloatingBranchOption(FloatingType&& thisValue) : inherited(std::move(thisValue)) {}
// template<typename T> TFldlibFloatingBranchOption(T source) requires enhanced_floating_point<T>
//    :  inherited(source) {}
   template<typename IntegerType> requires std::integral<IntegerType>
   TFldlibFloatingBranchOption(const TFldlibIntegerBranchOption<IntegerType>& source)
      :  inherited(source) {}
   TFldlibFloatingBranchOption(const thisType& source) = default;
   TFldlibFloatingBranchOption(const inherited& source) : inherited(source) {}
   TFldlibFloatingBranchOption(thisType&& source) = default;
   TFldlibFloatingBranchOption(inherited&& source) : inherited(std::move(source)) {}
   template <typename T> TFldlibFloatingBranchOption(const TFldlibBaseFloatingBranchOption<T>& source)
      :  inherited(source) {}
   template <typename T> TFldlibFloatingBranchOption(TFldlibBaseFloatingBranchOption<T>&& source)
      :  inherited(std::move(source)) {}
   template <typename T> TFldlibFloatingBranchOption(const TFldlibIntegerBranchOption<T>& source)
      :  inherited(source) {}
   template <typename T> TFldlibFloatingBranchOption(TFldlibIntegerBranchOption<T>&& source)
      :  inherited(std::move(source)) {}
   template <typename T> TFldlibFloatingBranchOption(T source) requires std::floating_point<T>
      :  inherited(source) {}

// template<typename T> thisType& operator=(const T& source) requires enhanced_floating_point<T>
//    {  value = source;
//       conditionalValue.reset();
//       return *this;
//    }
// template<typename T> thisType& operator=(T&& source) requires enhanced_floating_point<T>
//    {  value = std::move(source);
//       conditionalValue.reset();
//       return *this;
//    }
   thisType& operator=(const FloatingType& source)
      {  return (thisType&) inherited::operator=(source); }
   thisType& operator=(FloatingType&& source)
      {  return (thisType&) inherited::operator=(std::move(source)); }
   thisType& operator=(const thisType& source) = default;
   thisType& operator=(const inherited& source) { return (thisType&) inherited::operator=(source); }
   thisType& operator=(thisType&& source) = default;
   thisType& operator=(inherited&& source) { return (thisType&) inherited::operator=(std::move(source)); }
   template<typename T> thisType& operator=(const TFldlibBaseFloatingBranchOption<T>& source)
         requires enhanced_floating_point<T>
      {  return (thisType&) inherited::operator=(source); }
   template<typename T> thisType& operator=(TFldlibBaseFloatingBranchOption<T>&& source)
         requires enhanced_floating_point<T>
      {  return (thisType&) inherited::operator=(std::move(source)); }
   // typedef thisType InstrumentedAffineType;
   // void cloneShareParts() {  normalizeForChange(); }
};

template <typename DoubleType> requires std::floating_point<DoubleType>
class TFldlibFloatingBranchOption<TFldlibZonotopeOption<DoubleType>>
      : public TFldlibBaseFloatingBranchOption<TFldlibZonotopeOption<DoubleType>> {
  private:
   typedef TFldlibBaseFloatingBranchOption<TFldlibZonotopeOption<DoubleType>> inherited;
   typedef TFldlibFloatingBranchOption<TFldlibZonotopeOption<DoubleType>> thisType;
  public:
   TFldlibFloatingBranchOption() = default;
   TFldlibFloatingBranchOption(const TFldlibZonotopeOption<DoubleType>& thisValue) : inherited(thisValue) {}
   TFldlibFloatingBranchOption(const TFldlibZonotopeOption<DoubleType>& thisValue,
         std::shared_ptr<typename inherited::TValueWithBranches<TFldlibZonotopeOption<DoubleType>>>& thisConditionalValue)
      :  inherited(thisValue, thisConditionalValue) {}
   TFldlibFloatingBranchOption(TFldlibZonotopeOption<DoubleType>&& thisValue) : inherited(std::move(thisValue)) {}
// template<typename T> TFldlibFloatingBranchOption(T source) requires enhanced_floating_point<T>
//    :  inherited(source) {}
   TFldlibFloatingBranchOption(const thisType& source) = default;
   TFldlibFloatingBranchOption(const inherited& source) : inherited(source) {}
   TFldlibFloatingBranchOption(thisType&& source) = default;
   TFldlibFloatingBranchOption(inherited&& source) : inherited(std::move(source)) {}
   template <typename T> TFldlibFloatingBranchOption(const TFldlibBaseFloatingBranchOption<T>& source)
      :  inherited(source) {}
   template <typename T> TFldlibFloatingBranchOption(TFldlibBaseFloatingBranchOption<T>&& source)
      :  inherited(std::move(source)) {}
   template <typename T> TFldlibFloatingBranchOption(const TFldlibIntegerBranchOption<T>& source)
      :  inherited(source) {}
   template <typename T> TFldlibFloatingBranchOption(TFldlibIntegerBranchOption<T>&& source)
      :  inherited(std::move(source)) {}
   template <typename T> TFldlibFloatingBranchOption(T source) requires std::floating_point<T>
      :  inherited(source) {}

   thisType& operator=(const TFldlibZonotopeOption<DoubleType>& source)
      {  return (thisType&) inherited::operator=(source); }
   thisType& operator=(TFldlibZonotopeOption<DoubleType>&& source)
      {  return (thisType&) inherited::operator=(std::move(source)); }
   thisType& operator=(const thisType& source) = default;
   thisType& operator=(const inherited& source) { return (thisType&) inherited::operator=(source); }
   thisType& operator=(thisType&& source) = default;
   thisType& operator=(inherited&& source) { return (thisType&) inherited::operator=(std::move(source)); }
   template<typename T> thisType& operator=(const TFldlibBaseFloatingBranchOption<T>& source)
         requires enhanced_floating_point<T>
      {  return (thisType&) inherited::operator=(source); }
   template<typename T> thisType& operator=(TFldlibBaseFloatingBranchOption<T>&& source)
         requires enhanced_floating_point<T>
      {  return (thisType&) inherited::operator=(std::move(source)); }

   static std::vector<int> fullDiscreteConvert(const TFldlibZonotopeOption<DoubleType>& source);
   std::vector<int> fullDiscreteConvert() const;
   friend TFldlibIntegerBranchOption<int> trunc(const TFldlibFloatingBranchOption<TFldlibZonotopeOption<DoubleType>>& thisArg)
      {  std::vector<int> localResult = thisArg.fullDiscreteConvert();
         if (localResult.size() == 1)
            return TFldlibIntegerBranchOption<int>(localResult.back());
         AssumeCondition(localResult.size() >= 2)
         typedef FldlibBase::TIntegerWithBranches<int> IntegerWithBranches;
         typedef FldlibBase::TValueWithBranches<int> ValueWithBranches;
         TFldlibIntegerBranchOption<int> result((int) thisArg.value);
         result.conditionalValue.reset(new IntegerWithBranches(++FldlibBase::numberOfBranches,
               localResult.front(), std::shared_ptr<IntegerWithBranches>(), localResult.back(),
               std::shared_ptr<IntegerWithBranches>()));
         if (localResult.size() > 2) {
            if (thisArg.conditionalValue) {
               std::vector<int> localThenResult = thisType(thisArg.conditionalValue->getSThenValue(),
                     thisArg.conditionalValue->getSThenBranch()).fullDiscreteConvert();
               std::vector<int> localElseResult = thisType(thisArg.conditionalValue->getSElseValue(),
                     thisArg.conditionalValue->getSElseBranch()).fullDiscreteConvert();
               if (localThenResult.size() == 1 && localElseResult.size() == 1) {
                  result.conditionalValue.reset(new IntegerWithBranches(thisArg.conditionalValue->getMergeBranchIndex(),
                     localThenResult.back(), std::shared_ptr<IntegerWithBranches>(),
                     localElseResult.back(), std::shared_ptr<IntegerWithBranches>()));
                  return result;
               }
            }
            std::shared_ptr<ValueWithBranches>* endResult = &result.conditionalValue->getSElseBranch();
            int* endValue = &result.conditionalValue->getSElseValue();
            auto iter = localResult.begin(), iterNext = ++iter, iterEnd = localResult.end();
            while (iterNext != iterEnd) {
               endResult->reset(new IntegerWithBranches(++FldlibBase::numberOfBranches,
                  *iter, std::shared_ptr<IntegerWithBranches>(), *iterNext,
                  std::shared_ptr<IntegerWithBranches>()));
               *endValue = *iter;
               ++iter; 
               ++iterNext; 
               endResult = &(*endResult)->getSElseBranch();
               endValue = &(*endResult)->getSElseValue();
            }
            *endValue = *iter;
         }
         return result;
      }

   bool optimizeValue()
      {  bool result = true;
         if (inherited::conditionalValue) {
            typename inherited::BranchCursor cursor(*this);
            while (cursor.setToNext()) {
               bool localResult = cursor.elementSAt().optimizeValue();
               result = result || localResult;
            }
         }
         return result;
      }
   typedef typename inherited::EquationHolder EquationHolder;
   struct EquationVector {
      std::vector<EquationHolder*> equations;
      void clearHolder()
         {  for(auto* equation : equations) { if (equation) equation->clearHolder(); }; }
   };
   EquationVector getSRealDomain()
      {  EquationVector result;
         if (inherited::conditionalValue) {
            typename inherited::BranchCursor cursor(*this);
            while (cursor.setToNext())
               result.equations.push_back(cursor.elementSAt().getSRealDomain().equation);
         }
         else
            result.equations.push_back(inherited::value.getSRealDomain().equation);
         return result;
      }
   EquationVector getSError()
      {  EquationVector result;
         if (inherited::conditionalValue) {
            typename inherited::BranchCursor cursor(*this);
            while (cursor.setToNext())
               result.equations.push_back(cursor.elementSAt().getSError().equation);
         }
         else
            result.equations.push_back(inherited::value.getSError().equation);
         return result;
      }
   void mergeWith(const thisType& source);
   typedef typename TFldlibZonotopeOption<DoubleType>::BuiltDouble BuiltDouble;
   typedef typename TFldlibZonotopeOption<DoubleType>::Equation Equation;
   typedef typename TFldlibZonotopeOption<DoubleType>::BuiltReal BuiltReal;
   typedef typename TFldlibZonotopeOption<DoubleType>::SymbolsManager SymbolsManager;
   void addHighLevelUpdateError(DAffine::THighLevelUpdateVector<Equation>& highLevelUpdates,
         const BuiltReal& highLevelError)
      {  if (inherited::conditionalValue) {
            typename inherited::BranchCursor cursor(*this);
            while (cursor.setToNext())
               cursor.elementSAt().addHighLevelUpdateError(highLevelUpdates, highLevelError);
         }
         else
            inherited::value.addHighLevelUpdateError(highLevelUpdates, highLevelError);
      }
   static void writeImplementationHeader(STG::IOObject::OSBase& outFile)
      {  TFldlibZonotopeOption<DoubleType>::writeImplementationHeader(outFile); }
   static void writeRealHeader(STG::IOObject::OSBase& outFile)
      {  TFldlibZonotopeOption<DoubleType>::writeRealHeader(outFile); }
   void readImplementationSynchronizationFromFile(STG::IOObject::ISBase& inFile,
         typename BuiltDouble::ReadParameters& readParameters,
         typename Equation::ReadParameters& equationReadParameters,
         STG::IOObject::OSBase& outFile,
         typename BuiltDouble::WriteParameters& writeParameters,
         typename Equation::WriteParameters& equationWriteParameters,
         SymbolsManager& symbolsManager, BuiltReal& highLevelError);
   void writeImplementationSynchronizationToFile(STG::IOObject::OSBase& outFile,
         const typename BuiltDouble::WriteParameters& writeParameters,
         const typename Equation::WriteParameters& equationWriteParameters) const;
   void readRealSynchronizationFromFile(STG::IOObject::ISBase& inFile,
         typename Equation::ReadParameters& equationReadParameters,
         STG::IOObject::OSBase& outFile,
         typename Equation::WriteParameters& equationWriteParameters,
         SymbolsManager& symbolsManager, BuiltReal& highLevelError);
   void writeRealSynchronizationToFile(STG::IOObject::OSBase& outFile,
         const typename Equation::WriteParameters& equationWriteParameters) const;
   void readImplementationSynchronizationFromMemory(
         DAffine::BasePathExplorer::AbstractImplementationArray& implementationArray,
         DAffine::BasePathExplorer::AbstractEquationArray& equationArray,
         COL::TVector<int>& codeArray, SymbolsManager& symbolsManager, BuiltReal& highLevelError);
   void writeImplementationSynchronizationToMemory(
         DAffine::BasePathExplorer::AbstractImplementationArray& implementationArray,
         DAffine::BasePathExplorer::AbstractEquationArray& equationArray, COL::TVector<int>& codeArray) const;
   void readRealSynchronizationFromMemory(
         DAffine::BasePathExplorer::AbstractEquationArray& equationArray,
         COL::TVector<int>& codeArray, SymbolsManager& symbolsManager, BuiltReal& highLevelError);
   void writeRealSynchronizationToMemory(DAffine::BasePathExplorer::AbstractEquationArray& equationArray,
         COL::TVector<int>& codeArray) const;
   void moveImplementationInMemory(
         DAffine::BasePathExplorer::AbstractImplementationArray& implementationArray,
         DAffine::BasePathExplorer::AbstractEquationArray& equationArray, COL::TVector<int>& codeArray) const;
   void moveRealInMemory(DAffine::BasePathExplorer::AbstractEquationArray& equationArray, COL::TVector<int>& codeArray) const;
   typedef thisType InstrumentedAffineType;
   void cloneShareParts() { inherited::value.cloneShareParts(); }
};

template<typename T>
class TFLDLibVectorBranchOption {
  private:
   typedef TFLDLibVectorBranchOption<T> thisType;
   TFldlibIntegerBranchOption<int> ibSizes;
   std::vector<T> vibContent;

  public:
   TFLDLibVectorBranchOption() = default;
   void reserve(size_t expected_size) { vibContent.reserve(expected_size); }
   void clear() { vibContent.clear(); ibSizes = 0; }

   const TFldlibIntegerBranchOption<int>& size() const { return ibSizes; }
   TFldlibIntegerBranchOption<int>& ssize() { return ibSizes; }
   std::vector<T>& scontent() { return vibContent; }
   const std::vector<T>& ccontent() const { return vibContent; }
   T& operator[](int index) { return vibContent[index]; }
   const T& operator[](int index) const { return vibContent(index); }
   T operator[](const TFldlibIntegerBranchOption<int>& index) const
      {  if (!index.conditionalValue)
            return vibContent[index.value];

         T result;
         TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(index);
         while (indexCursor.setToNext())
            result.conditionalAssigns(indexCursor.getPathAt(), vibContent[indexCursor.elementAt()]);
         return result;
      }
   class ElementProperty {
      std::vector<T>& vibContent;
      TFldlibIntegerBranchOption<int> uIndex;

     public:
      ElementProperty(TFLDLibVectorBranchOption<T>& container,
            const TFldlibIntegerBranchOption<int>& index)
         :  vibContent(container.vibContent), uIndex(index) {}
      ElementProperty(const ElementProperty& source) = default;
      ElementProperty(ElementProperty&& source) = default;

      operator T() const
         {  if (!uIndex.conditionalValue)
               return vibContent[uIndex.value];

            T result;
            TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
            while (indexCursor.setToNext())
               result.conditionalAssigns(indexCursor.getPathAt(), vibContent[indexCursor.elementAt()]);
            return result;
         }
      ElementProperty& operator=(const T& value)
         {  if (!uIndex.conditionalValue)
               vibContent[uIndex.value] = value;
            else {
               TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
               while (indexCursor.setToNext())
                  vibContent[indexCursor.elementAt()].conditionalAssigns(
                     indexCursor.getPathAt(), value);
            }
            return *this;
         }
      ElementProperty& operator=(T&& value)
         {  if (!uIndex.conditionalValue)
               vibContent[uIndex.value] = std::move(value);
            else {
               TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
               while (indexCursor.setToNext())
                  vibContent[indexCursor.elementAt()].conditionalAssigns(
                     indexCursor.getPathAt(), std::move(value));
            }
            return *this;
         }
   };
   friend class ElementProperty;
   ElementProperty operator[](const TFldlibIntegerBranchOption<int>& index)
      {  return ElementProperty(*this, index); }
   void push_back(const T& value)
      {  vibContent.push_back(value);
         if (ibSizes.conditionalValue.get()) {
            TFldlibIntegerBranchOption<int>::BranchCursor sizeCursor(ibSizes);
            while (sizeCursor.setToNext())
               vibContent[sizeCursor.elementAt()].conditionalAssigns(sizeCursor.getPathAt(), value);
         }
         ++ibSizes;
      }
   void pop_back()
      {  --ibSizes; }
   void conditional_push_back(const std::vector<std::pair<int, bool>>& path,
         const T& value)
      {  vibContent.push_back(value);
         if (ibSizes.conditionalValue.get()) {
            TFldlibIntegerBranchOption<int>::BranchCursor sizeCursor(ibSizes);
            while (sizeCursor.setToNext()) {
               std::vector<std::pair<int, bool>> newPath(path);
               if (FldlibBase::mergePath(newPath, sizeCursor.getPathAt()))
                  vibContent[sizeCursor.elementAt()].conditionalAssigns(newPath, value);
            }
         }
         ibSizes.conditionalAssigns(path, ibSizes+1);
      }
   
   void mergeWith(int mergeBranchIndex, TFLDLibVectorBranchOption<T>& source)
      {  AssumeUnimplemented
      }
   class forward_iterator {
     private:
      // TFldlibIntegerBranchOption<int> uIndex;
      int uIndex;
      TFLDLibVectorBranchOption<T>& vContainer;

     public:
      forward_iterator(int index,
            TFLDLibVectorBranchOption<T>& container)
         :  uIndex(index), vContainer(container) {}
      forward_iterator(const forward_iterator&) = default;
      forward_iterator(forward_iterator&&) = default;
      forward_iterator& operator=(const forward_iterator&) = default;
      forward_iterator& operator=(forward_iterator&&) = default;

      forward_iterator& operator++() { ++uIndex; return *this; }
      bool operator==(const forward_iterator& source) const;
      bool operator!=(const forward_iterator& source) const;
      ElementProperty operator*() const
         {  return ElementProperty(vContainer, uIndex); }
      TFLDLibVectorBranchOption<T>& getContainer() const { return vContainer; }
      int getIndex() const { return uIndex; }
   };
   forward_iterator begin() { return forward_iterator(0, *this); }
   forward_iterator begin() const { return forward_iterator(0, const_cast<thisType&>(*this)); }
   forward_iterator end() { return forward_iterator(ibSizes.getMaxValue(), *this); }
   forward_iterator end() const { return forward_iterator(ibSizes.getMaxValue(), const_cast<thisType&>(*this)); }
   void erase(forward_iterator& iterator);
   void erase(forward_iterator&& iterator);
   // typedef thisType InstrumentedAffineType;
   // void cloneShareParts() {}
};

template<typename IntegerType> requires std::integral<IntegerType>
class TFLDLibIntegerVectorBranchOption : public TFLDLibVectorBranchOption<TFldlibIntegerBranchOption<IntegerType>> {
  private:
   typedef TFLDLibIntegerVectorBranchOption<IntegerType> thisType;
   typedef TFLDLibVectorBranchOption<TFldlibIntegerBranchOption<IntegerType>> inherited;

  public:
   TFLDLibIntegerVectorBranchOption() = default;
#if 0
   void reserve(size_t expected_size) { vibContent.reserve(expected_size); }
   void clear() { vibContent.clear(); ibSizes = 0; }

   const TFldlibIntegerBranchOption<int>& size() const { return ibSizes; }
   TFldlibIntegerBranchOption<IntegerType>& operator[](int index) { return vibContent(index); }
   const TFldlibIntegerBranchOption<IntegerType>& operator[](int index) const { return vibContent(index); }
   TFldlibIntegerBranchOption<IntegerType> operator[](const TFldlibIntegerBranchOption<int>& index) const
      {  if (!index.conditionalValue)
            return vibContent[index.value];

         TFldlibIntegerBranchOption<IntegerType> result;
         TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(index);
         while (indexCursor.setToNext())
            result.conditionalAssigns(indexCursor.getPathAt(), vibContent[indexCursor.elementAt()]);
         return result;
      }
   class ElementProperty {
      std::vector<TFldlibIntegerBranchOption<IntegerType> >& vibContent;
      TFldlibIntegerBranchOption<int> uIndex;

     public:
      ElementProperty(TFLDLibIntegerVectorBranchOption<IntegerType>& container,
            const TFldlibIntegerBranchOption<int>& index)
         :  vibContent(container.vibContent), uIndex(index) {}
      ElementProperty(const ElementProperty& source) = default;
      ElementProperty(ElementProperty&& source) = default;

      operator TFldlibIntegerBranchOption<IntegerType>() const
         {  if (!uIndex.conditionalValue)
               return vibContent[uIndex.value];

            TFldlibIntegerBranchOption<IntegerType> result;
            TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
            while (indexCursor.setToNext())
               result.conditionalAssigns(indexCursor.getPathAt(), vibContent[indexCursor.elementAt()]);
            return result;
         }
      ElementProperty& operator=(IntegerType value)
         {  if (!uIndex.conditionalValue)
               vibContent[uIndex.value] = value;
            else {
               TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
               while (indexCursor.setToNext())
                  vibContent[indexCursor.elementAt()].conditionalAssigns(
                     indexCursor.getPathAt(), value);
            }
            return *this;
         }
      ElementProperty& operator=(const TFldlibIntegerBranchOption<IntegerType>& value)
         {  if (!uIndex.conditionalValue)
               vibContent[uIndex.value] = value;
            else {
               TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
               while (indexCursor.setToNext())
                  vibContent[indexCursor.elementAt()].conditionalAssigns(
                     indexCursor.getPathAt(), value);
            }
            return *this;
         }
      ElementProperty& operator=(TFldlibIntegerBranchOption<IntegerType>&& value)
         {  if (!uIndex.conditionalValue)
               vibContent[uIndex.value] = std::move(value);
            else {
               TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
               while (indexCursor.setToNext())
                  vibContent[indexCursor.elementAt()].conditionalAssigns(
                     indexCursor.getPathAt(), std::move(value));
            }
            return *this;
         }
   };
   friend class ElementProperty;
   ElementProperty operator[](const TFldlibIntegerBranchOption<int>& index)
      {  return ElementProperty(*this, index); }
   void push_back(IntegerType value)
      {  vibContent.push_back(value);
         if (ibSizes.conditionalValue.get()) {
            TFldlibIntegerBranchOption<int>::BranchCursor sizeCursor(ibSizes);
            while (sizeCursor.setToNext())
               vibContent[sizeCursor.elementAt()].conditionalAssigns(sizeCursor.getPathAt(), value);
         }
         ++ibSizes;
      }
   void push_back(const TFldlibIntegerBranchOption<IntegerType>& value)
      {  vibContent.push_back(value);
         if (ibSizes.conditionalValue.get()) {
            TFldlibIntegerBranchOption<int>::BranchCursor sizeCursor(ibSizes);
            while (sizeCursor.setToNext())
               vibContent[sizeCursor.elementAt()].conditionalAssigns(sizeCursor.getPathAt(), value);
         }
         ++ibSizes;
      }
   void pop_back()
      {  --ibSizes; }
   void conditional_push_back(const std::vector<std::pair<int, bool>>& path, IntegerType value)
      {  vibContent.push_back(value);
         if (ibSizes.conditionalValue.get()) {
            TFldlibIntegerBranchOption<int>::BranchCursor sizeCursor(ibSizes);
            while (sizeCursor.setToNext()) {
               std::vector<std::pair<int, bool>> newPath(path);
               if (FldlibBase::mergePath(newPath, sizeCursor.getPathAt()))
                  vibContent[sizeCursor.elementAt()].conditionalAssigns(newPath, value);
            };
         }
         ibSizes.conditionalAssigns(path, ibSizes+1);
      }
   void conditional_push_back(const std::vector<std::pair<int, bool>>& path,
         const TFldlibIntegerBranchOption<IntegerType>& value)
      {  vibContent.push_back(value);
         if (ibSizes.conditionalValue.get()) {
            TFldlibIntegerBranchOption<int>::BranchCursor sizeCursor(ibSizes);
            while (sizeCursor.setToNext()) {
               std::vector<std::pair<int, bool>> newPath(path);
               if (FldlibBase::mergePath(newPath, sizeCursor.getPathAt()))
                  vibContent[sizeCursor.elementAt()].conditionalAssigns(newPath, value);
            }
         }
         ibSizes.conditionalAssigns(path, ibSizes+1);
      }
   
   void mergeWith(int mergeBranchIndex, TFLDLibIntegerVectorBranchOption<IntegerType>& source)
      {  AssumeUnimplemented
      }
   class forward_iterator {
     private:
      // TFldlibIntegerBranchOption<int> uIndex;
      int uIndex;
      TFLDLibIntegerVectorBranchOption<IntegerType>& vContainer;

     public:
      forward_iterator(int index,
            TFLDLibIntegerVectorBranchOption<IntegerType>& container)
         :  uIndex(index), vContainer(container) {}
      // forward_iterator(const TFldlibIntegerBranchOption<int> index,
      //       TFLDLibIntegerVectorBranchOption<IntegerType>& container)
      //    :  uIndex(index), vContainer(container) {}
      forward_iterator(const forward_iterator&) = default;
      forward_iterator(forward_iterator&&) = default;
      forward_iterator& operator=(const forward_iterator&) = default;
      forward_iterator& operator=(forward_iterator&&) = default;

      forward_iterator& operator++() { ++uIndex; return *this; }
      bool operator==(const forward_iterator& source) const;
      bool operator!=(const forward_iterator& source) const;
      ElementProperty operator*() const
         {  return ElementProperty(vContainer, uIndex); }
      TFLDLibIntegerVectorBranchOption<IntegerType>& getContainer() const { return vContainer; }
   };
   forward_iterator begin() { return forward_iterator(0, *this); }
   forward_iterator begin() const { return forward_iterator(0, const_cast<thisType&>(*this)); }
   forward_iterator end() { return forward_iterator(ibSizes.getMaxValue(), *this); }
   forward_iterator end() const { return forward_iterator(ibSizes.getMaxValue(), const_cast<thisType&>(*this)); }
   void erase(forward_iterator& iterator);
   void erase(forward_iterator&& iterator);
#endif
   // typedef thisType InstrumentedAffineType;
   // void cloneShareParts() {}
};

template<typename FloatingType> requires enhanced_floating_point<FloatingType>
class TFLDLibFloatingVectorBranchOption : public TFLDLibVectorBranchOption<TFldlibFloatingBranchOption<FloatingType>> {
  private:
   typedef TFLDLibFloatingVectorBranchOption<FloatingType> thisType;
   typedef TFLDLibVectorBranchOption<TFldlibFloatingBranchOption<FloatingType>> inherited;

  public:
   TFLDLibFloatingVectorBranchOption() = default;
#if 0
   void reserve(size_t expected_size) { vibContent.reserve(expected_size); }
   void clear() { vibContent.clear(); ibSizes = 0; }

   TFldlibFloatingBranchOption<FloatingType>& operator[](int index) { return vibContent(index); }
   const TFldlibFloatingBranchOption<FloatingType>& operator[](int index) const { return vibContent(index); }
   TFldlibFloatingBranchOption<FloatingType> operator[](const TFldlibIntegerBranchOption<int>& index) const
      {  if (!index.conditionalValue)
            return vibContent[index.value];

         TFldlibFloatingBranchOption<FloatingType> result;
         TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(index);
         while (indexCursor.setToNext())
            result.conditionalAssigns(indexCursor.getPathAt(), vibContent[indexCursor.elementAt()]);
         return result;
      }
   const TFldlibFloatingBranchOption<FloatingType>& at(int index) const { return vibContent(index); }
   TFldlibFloatingBranchOption<FloatingType> at(const TFldlibIntegerBranchOption<int>& index) const
      {  return operator[](index); }
   class ElementProperty {
      std::vector<TFldlibFloatingBranchOption<FloatingType> >& vibContent;
      TFldlibIntegerBranchOption<int> uIndex;

     public:
      ElementProperty(TFLDLibFloatingVectorBranchOption<FloatingType>& container,
            const TFldlibIntegerBranchOption<int>& index)
         :  vibContent(container.vibContent), uIndex(index) {}
      ElementProperty(const ElementProperty& source) = default;
      ElementProperty(ElementProperty&& source) = default;

      ElementProperty& operator=(FloatingType value)
         {  if (!uIndex.conditionalValue)
               vibContent[uIndex.value] = value;
            else {
               TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
               while (indexCursor.setToNext())
                  vibContent[indexCursor.elementAt()].conditionalAssigns(
                     indexCursor.getPathAt(), value);
            }
            return *this;
         }
      ElementProperty& operator=(const TFldlibFloatingBranchOption<FloatingType>& value)
         {  if (!uIndex.conditionalValue)
               vibContent[uIndex.value] = value;
            else {
               TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
               while (indexCursor.setToNext())
                  vibContent[indexCursor.elementAt()].conditionalAssigns(
                     indexCursor.getPathAt(), value);
            }
            return *this;
         }
      ElementProperty& operator=(TFldlibFloatingBranchOption<FloatingType>&& value)
         {  if (!uIndex.conditionalValue)
               vibContent[uIndex.value] = std::move(value);
            else {
               TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
               while (indexCursor.setToNext())
                  vibContent[indexCursor.elementAt()].conditionalAssigns(
                     indexCursor.getPathAt(), std::move(value));
            }
            return *this;
         }
   };
   friend class ElementProperty;
   const TFldlibIntegerBranchOption<int>& size() const { return ibSizes; }
   ElementProperty operator[](const TFldlibIntegerBranchOption<int>& index)
      {  return ElementProperty(*this, index); }
   void push_back(FloatingType value)
      {  ++ibSizes;
         vibContent.push_back(value);
         if (ibSizes.conditionalValue.get()) {
            TFldlibIntegerBranchOption<int>::BranchCursor sizeCursor(ibSizes);
            while (sizeCursor.setToNext())
               vibContent[sizeCursor.elementAt()].conditionalAssigns(sizeCursor.getPathAt(), value);
         }
      }
   void push_back(const TFldlibFloatingBranchOption<FloatingType>& value)
      {  ++ibSizes;
         vibContent.push_back(value);
         if (ibSizes.conditionalValue.get()) {
            TFldlibIntegerBranchOption<int>::BranchCursor sizeCursor(ibSizes);
            while (sizeCursor.setToNext())
               vibContent[sizeCursor.elementAt()].conditionalAssigns(sizeCursor.getPathAt(), value);
         }
      }
   
   void mergeWith(int mergeBranchIndex, TFLDLibFloatingVectorBranchOption<FloatingType>& source)
      {  AssumeUnimplemented
      }
#endif
   // typedef thisType InstrumentedAffineType;
   // void cloneShareParts() {}
};

template<class T>
class TReferenceContainer {
  private:
   typedef TReferenceContainer<T> thisType;
   const T& cContent;

  public:
   TReferenceContainer(const T& content) : cContent(content) {}
   const typename T::value_type& operator[](int index) const
      { return cContent[index]; }
   const typename T::value_type& at(int index) const
      { return cContent[index]; }
};

template<class T>
class TIndexedReferenceContainer {
  private:
   typedef TReferenceContainer<T> thisType;
   const T& cContent;
   int uIndex;

  public:
   TIndexedReferenceContainer(const T& content, int index) : cContent(content), uIndex(index) {}
   const typename T::value_type& operator[](int index) const
      { return cContent[uIndex][index]; }
   const typename T::value_type& at(int index) const
      { return cContent[uIndex][index]; }
};

template<integral_indexed_container T>
class TReferenceContainer<T> {
  private:
   typedef TReferenceContainer<T> thisType;
   const T& cContent;

  public:
   TReferenceContainer(const T& content) : cContent(content) {}
   const TFldlibIntegerBranchOption<typename T::value_type>& operator[](int index) const
      { return cContent[index]; }
   const TFldlibIntegerBranchOption<typename T::value_type>& operator[](unsigned index) const
      { return cContent[index]; }
   TFldlibIntegerBranchOption<typename T::value_type> operator[](const TFldlibIntegerBranchOption<int>& index) const
      {  if (!index.conditionalValue)
            return cContent[index.value];

         TFldlibIntegerBranchOption<typename T::value_type> result;
         TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(index);
         while (indexCursor.setToNext())
            result.conditionalAssigns(indexCursor.getPathAt(), cContent[indexCursor.elementAt()]);
         return result;
      }
   TFldlibIntegerBranchOption<typename T::value_type> operator[](const TFldlibIntegerBranchOption<unsigned>& index) const
      {  if (!index.conditionalValue)
            return cContent[index.value];

         TFldlibIntegerBranchOption<typename T::value_type> result;
         TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(index);
         while (indexCursor.setToNext())
            result.conditionalAssigns(indexCursor.getPathAt(), cContent[indexCursor.elementAt()]);
         return result;
      }
   const TFldlibIntegerBranchOption<typename T::value_type>& at(int index) const { return cContent(index); }
   TFldlibIntegerBranchOption<typename T::value_type> at(const TFldlibIntegerBranchOption<int>& index) const
      {  return operator[](index); }
   const TFldlibIntegerBranchOption<typename T::value_type>& at(unsigned index) const { return cContent(index); }
   TFldlibIntegerBranchOption<typename T::value_type> at(const TFldlibIntegerBranchOption<unsigned>& index) const
      {  return operator[](index); }
};

template<integral_indexed_container T>
class TIndexedReferenceContainer<T> {
  private:
   typedef TReferenceContainer<T> thisType;
   const T& cContent;
   TFldlibIntegerBranchOption<int> uIndex;

  public:
   TIndexedReferenceContainer(const T& content, const TFldlibIntegerBranchOption<int>& index)
      :  cContent(content), uIndex(index) {}
   const TFldlibIntegerBranchOption<typename T::value_type::value_type>& operator[](int index) const
      {  if (!uIndex.conditionalValue)
            return cContent[uIndex.value][index];

         TFldlibIntegerBranchOption<typename T::value_type::value_type> result;
         TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
         while (indexCursor.setToNext())
            result.conditionalAssigns(indexCursor.getPathAt(), cContent[indexCursor.elementAt()][index]);
         return result;
      }
   const TFldlibIntegerBranchOption<typename T::value_type::value_type>& operator[](unsigned index) const
      {  if (!uIndex.conditionalValue)
            return cContent[uIndex.value][index];

         TFldlibIntegerBranchOption<typename T::value_type::value_type> result;
         TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
         while (indexCursor.setToNext())
            result.conditionalAssigns(indexCursor.getPathAt(), cContent[indexCursor.elementAt()][index]);
         return result;
      }
   TFldlibIntegerBranchOption<typename T::value_type::value_type> operator[](const TFldlibIntegerBranchOption<int>& index) const
      {  if (!index.conditionalValue) {
            if (!uIndex.conditionalValue)
               return cContent[uIndex.value][index.value];
            TFldlibIntegerBranchOption<typename T::value_type::value_type> result;
            TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
            while (indexCursor.setToNext())
               result.conditionalAssigns(indexCursor.getPathAt(), cContent[indexCursor.elementAt()][index]);
            return result;
         }

         TFldlibIntegerBranchOption<typename T::value_type::value_type> result;
         TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
         while (indexCursor.setToNext()) {
            TFldlibIntegerBranchOption<int>::BranchCursor secondIndexCursor(index);
            while (secondIndexCursor.setToNext()) {
               std::vector<std::pair<int, bool>> path = indexCursor.getPathAt();
               path.insert(path.end(), secondIndexCursor.getPathAt().begin(),
                     secondIndexCursor.getPathAt().end());
               result.conditionalAssigns(path,
                  cContent[indexCursor.elementAt()][secondIndexCursor.elementAt()]);
            }
         }
         return result;
      }
   TFldlibIntegerBranchOption<typename T::value_type::value_type> operator[](const TFldlibIntegerBranchOption<unsigned>& index) const
      {  if (!index.conditionalValue) {
            if (!uIndex.conditionalValue)
               return cContent[uIndex.value][index.value];
            TFldlibIntegerBranchOption<typename T::value_type::value_type> result;
            TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
            while (indexCursor.setToNext())
               result.conditionalAssigns(indexCursor.getPathAt(), cContent[indexCursor.elementAt()][index]);
            return result;
         }

         TFldlibIntegerBranchOption<typename T::value_type::value_type> result;
         TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(uIndex);
         while (indexCursor.setToNext()) {
            TFldlibIntegerBranchOption<int>::BranchCursor secondIndexCursor(index);
            while (secondIndexCursor.setToNext()) {
               std::vector<std::pair<int, bool>> path = indexCursor.getPathAt();
               path.insert(path.end(), secondIndexCursor.getPathAt().begin(),
                     secondIndexCursor.getPathAt().end());
               result.conditionalAssigns(path,
                  cContent[indexCursor.elementAt()][secondIndexCursor.elementAt()]);
            }
         }
         return result;
      }
   const TFldlibIntegerBranchOption<typename T::value_type::value_type>& at(int index) const { return operator[](index); }
   TFldlibIntegerBranchOption<typename T::value_type::value_type> at(const TFldlibIntegerBranchOption<int>& index) const
      {  return operator[](index); }
   const TFldlibIntegerBranchOption<typename T::value_type::value_type>& at(unsigned index) const { return operator[](index); }
   TFldlibIntegerBranchOption<typename T::value_type::value_type> at(const TFldlibIntegerBranchOption<unsigned>& index) const
      {  return operator[](index); }
};

template<integral_indexed_double_container T>
class TReferenceContainer<T> {
  private:
   typedef TReferenceContainer<T> thisType;
   const T& cContent;

  public:
   TReferenceContainer(const T& content) : cContent(content) {};
   const TReferenceContainer<typename T::value_type>& operator[](int index) const
      { return getReferenceContainer(cContent(index)); }
   const TReferenceContainer<typename T::value_type>& operator[](unsigned index) const
      { return getReferenceContainer(cContent(index)); }
   TIndexedReferenceContainer<T> operator[](const TFldlibIntegerBranchOption<int>& index) const
      {  return TIndexedReferenceContainer<T>(cContent, index); }
   TIndexedReferenceContainer<T> operator[](const TFldlibIntegerBranchOption<unsigned>& index) const
      {  return TIndexedReferenceContainer<T>(cContent, index); }
   const TReferenceContainer<typename T::value_type>& at(int index) const { return cContent(index); }
   TIndexedReferenceContainer<T> at(const TFldlibIntegerBranchOption<int>& index) const
      {  return operator[](index); }
   const TReferenceContainer<typename T::value_type>& at(unsigned index) const { return cContent(index); }
   TIndexedReferenceContainer<T> at(const TFldlibIntegerBranchOption<unsigned>& index) const
      {  return operator[](index); }
};

template<enhanced_floating_point_container T>
class TReferenceContainer<T> {
  private:
   typedef TReferenceContainer<T> thisType;
   const T& cContent;

  public:
   TReferenceContainer(const T& content) : cContent(content) {};
   const TFldlibFloatingBranchOption<typename T::value_type>& operator[](int index) const { return cContent(index); }
   TFldlibFloatingBranchOption<typename T::value_type> operator[](const TFldlibIntegerBranchOption<int>& index) const
      {  if (!index.conditionalValue)
            return cContent[index.value];

         TFldlibFloatingBranchOption<typename T::value_type> result;
         TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(index);
         while (indexCursor.setToNext())
            result.conditionalAssigns(indexCursor.getPathAt(), cContent[indexCursor.elementAt()]);
         return result;
      }
   const TFldlibFloatingBranchOption<typename T::value_type>& at(int index) const { return cContent(index); }
   TFldlibFloatingBranchOption<typename T::value_type> at(const TFldlibIntegerBranchOption<int>& index) const
      {  return operator[](index); }
   const TFldlibFloatingBranchOption<typename T::value_type>& operator[](unsigned index) const { return cContent(index); }
   TFldlibFloatingBranchOption<typename T::value_type> operator[](const TFldlibIntegerBranchOption<unsigned>& index) const
      {  if (!index.conditionalValue)
            return cContent[index.value];

         TFldlibFloatingBranchOption<typename T::value_type> result;
         TFldlibIntegerBranchOption<int>::BranchCursor indexCursor(index);
         while (indexCursor.setToNext())
            result.conditionalAssigns(indexCursor.getPathAt(), cContent[indexCursor.elementAt()]);
         return result;
      }
   const TFldlibFloatingBranchOption<typename T::value_type>& at(unsigned index) const { return cContent(index); }
   TFldlibFloatingBranchOption<typename T::value_type> at(const TFldlibIntegerBranchOption<unsigned>& index) const
      {  return operator[](index); }
};

#ifdef INT_DOMAIN

template<class T>
TReferenceContainer<T>
getReferenceContainer(const T& source) { return TReferenceContainer<T>(source); }

#endif

} // end of namespace NumericalDomains

