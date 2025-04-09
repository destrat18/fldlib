/**************************************************************************/
/*                                                                        */
/*  Copyright (C) 2013-2025                                               */
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
// Library   : Pointer
// Unit      : Parser
// File      : DescentParse.h
// Description :
//   Definition of classes to implement recursive descent parsers.
//

#pragma once

#include "StandardClasses/Persistence.h"
#include "Pointer/PassPointer.h"
#include "Pointer/ImplArray.h"

namespace STG {

namespace Parser {

/******************************************/
/* Definition of the template TStateStack */
/******************************************/

template <class TypeArguments>
class TStateStack : public EnhancedObject, public Lexer::Base {
  private:
   typedef TStateStack<TypeArguments> thisType;
   typedef EnhancedObject inherited;
   char* pvContent; // pvContent=nullptr <=> uAllocatedSize == 0
   size_t uAllocatedSize;
   char* pvCurrentPointer; // pvCurrentPointer == nullptr <=> empty
   // else pvContent != nullptr && pvCurrentPointer points onto last element
   // pvCurrentPointer - pvContent < uAllocatedSize
   // pvCurrentPointer - pvContent + pvContent->getSize() <= uAllocatedSize

  public:
   typedef TypeArguments ParseArgument;
   class VirtualParseState : public EnhancedObject {
     private:
      typedef EnhancedObject inherited;
      int uPoint = 0;
      mutable int uPreviousSize = 0;
      friend class TStateStack<TypeArguments>;

     protected:
      virtual ComparisonResult _compare(const EnhancedObject& asource) const override
         {  ComparisonResult result = inherited::_compare(asource);
            const VirtualParseState& source = (const VirtualParseState&) asource;
            return (result == CREqual) ? fcompare(uPoint, source.uPoint) : result;
         }
      virtual int getSize() const { return sizeof(VirtualParseState); }
      virtual int getAlign() const { return alignof(VirtualParseState); }
#include "StandardClasses/UndefineNew.h"
      virtual void moveTo(void* dest)
         {  new (dest) VirtualParseState(std::move(*this)); }
      virtual void copyTo(void* dest)
         {  new (dest) VirtualParseState(*this); }
#include "StandardClasses/DefineNew.h"
      virtual void* initUnionResult(std::function<void(void*, void*)> mover, std::function<void(void*, const void*)> copyer, std::function<void(void*)> destroyer)
         {  return nullptr; }
      virtual void* getSUnionResult() { return nullptr; }
      virtual void* getFreeUnionResult() { return nullptr; }
      virtual void* getResultPlace() { return nullptr; }

     public:
      VirtualParseState() = default;
      VirtualParseState(const VirtualParseState& source)
         :  inherited(source), uPoint(source.uPoint) {}
      VirtualParseState& operator=(const VirtualParseState& source)
         {  inherited::operator=(source);
            uPoint = source.uPoint;
            return *this;
         }
      DefineCopy(VirtualParseState)
      // DDefineAssign(VirtualParseState)

      int& point() { return uPoint; }
      const int& point() const { return uPoint; }
      virtual auto operator()(TStateStack<TypeArguments>& parser, ParseArgument& args) 
         -> typename ParseArgument::ResultAction
         {  AssumeUncalled return (typename ParseArgument::ResultAction) 3; /* Finished */ }

      template <class TypeResult>
      void absorbUnionResult(TypeResult&& res) const
         // [TODO] a single index can replace the 3 std::functions, like for std::variant
         // use enable_if (SFINAE) to find the index from TypeResult and call initUnionResult
         //    with this index
         {  new (const_cast<VirtualParseState&>(*this).initUnionResult(
                  [](void* dst, void* src)
                     { new (dst) TypeResult(std::move(*reinterpret_cast<TypeResult*>(src))); },
                  [](void* dst, const void* src)
                     { new (dst) TypeResult(*reinterpret_cast<const TypeResult*>(src)); },
                  [](void* src)
                     { reinterpret_cast<const TypeResult*>(src)->~TypeResult(); }
               )) TypeResult(std::move(res));
         }
      template <class TypeResult>
      void freeUnionResult(TypeResult* res) const
         {  reinterpret_cast<TypeResult*>(const_cast<VirtualParseState&>(*this).getFreeUnionResult())->~TypeResult(); }
      template <class TypeResult>
      TypeResult& getUnionResult(TypeResult*) const
         {  return *reinterpret_cast<TypeResult*>(const_cast<VirtualParseState&>(*this).getSUnionResult()); }
   };

   template <size_t arg1, size_t ... others> struct static_max;
   template <size_t arg>
   struct static_max<arg> {
      static const size_t value = arg;
   };
   template <size_t arg1, size_t arg2, size_t ... others>
   struct static_max<arg1, arg2, others...> {
      static const size_t value = arg1 >= arg2 ? static_max<arg1, others...>::value :
            static_max<arg2, others...>::value;
   };
   template<typename... Ts>
   struct UnionResult {
      static const size_t data_size = static_max<sizeof(Ts)...>::value;
      static const size_t data_align = static_max<alignof(Ts)...>::value;
      using data_t = typename std::aligned_storage<data_size, data_align>::type;
      data_t data;

      UnionResult() { memset(&data, 0, sizeof(data_t)); }
   };

   template <class TypeResult>
   class TVirtualParseState : public VirtualParseState {
     private:
      typedef VirtualParseState inherited;
      typedef TVirtualParseState<TypeResult> thisType;
      TypeResult rResult;

     public:
      TVirtualParseState() : rResult() {}
      TVirtualParseState(const thisType& source) = default;
      TVirtualParseState(thisType&& source) = default;
      thisType& operator=(const thisType&) = default;
      TemplateDefineCopy(TVirtualParseState, TypeResult)
      // DTemplateDefineAssign(TVirtualParseState, TypeResult)

      virtual void* getResultPlace() override { return &rResult; }
      bool hasResult() const { return rResult.isValid(); }
      TypeResult& getSResult() { return rResult; }
      const TypeResult& getResult() const { return rResult; }
      TypeResult&& extractResult() { return std::move(rResult); }
      void setResult(TypeResult&& result) { rResult = std::move(result); }
   };

   template <typename... Ts>
   class TVirtualParseState<UnionResult<Ts...> > : public VirtualParseState {
     private:
      typedef VirtualParseState inherited;
      typedef TVirtualParseState<UnionResult<Ts...> > thisType;
      UnionResult<Ts...> rResult;
      // [TODO] a single index can replace these functions, like for std::variant
      std::function<void(void*, void*)> fnMover;
      std::function<void(void*, const void*)> fnCopyer;
      std::function<void(void*)> fnDestroyer;

     protected:
      virtual void* initUnionResult(std::function<void(void*, void*)> mover, std::function<void(void*, const void*)> copyer, std::function<void(void*)> destroyer) override
         {  fnMover = mover; fnCopyer = copyer; fnDestroyer = destroyer;
            return &rResult.data;
         }
      virtual void* getSUnionResult() override { return &rResult.data; }
      virtual void* getFreeUnionResult() override
         {  fnMover = nullptr; fnCopyer = nullptr; fnDestroyer = nullptr;
            return &rResult.data;
         }

     public:
      TVirtualParseState() : rResult(), fnMover(), fnCopyer() {}
      TVirtualParseState(const thisType& source)
         :  VirtualParseState(source), rResult(source.rResult), fnMover(source.fnMover), fnCopyer(source.fnCopyer), fnDestroyer(source.fnDestroyer)
         {  if (fnCopyer != nullptr)
               fnCopyer(&rResult.data, &source.rResult.data);
         }
      TVirtualParseState(thisType&& source)
         :  VirtualParseState(source), rResult(std::move(source.rResult)), fnMover(source.fnMover), fnCopyer(source.fnCopyer), fnDestroyer(source.fnDestroyer)
         {  if (fnMover != nullptr)
               fnMover(&rResult.data, &source.rResult.data);
         }
      ~TVirtualParseState()
         {  if (fnDestroyer != nullptr)
               fnDestroyer(&rResult.data);
         }
      thisType& operator=(const thisType&) = default;
      DefineCopy(thisType)
      // DDefineAssign(thisType)

      virtual void* getResultPlace() override { return &rResult; }
      UnionResult<Ts...>& getSResult() { return rResult; }
      const UnionResult<Ts...>& getResult() const { return rResult; }
   };

   // ReadResult ReadPointerMethod(TStateStack<TypeArguments>&, TypeArguments&)
   //   RRNeedChars if TypeArguments has a lexer access to read additional info
   //                 and info is not present in the buffer
   //   RRContinue  if event is not managed but has produced a state change
   //   RRHasToken  if event is completly managed
   //   RRFinished  if event is the last supported one (and event is completly managed)
   template <class TypeObject, typename ReadPointerMethod, class TypeResult>
   class TParseState : public TVirtualParseState<TypeResult> {
     private:
      typedef TVirtualParseState<TypeResult> inherited;
      typedef TParseState<TypeObject, ReadPointerMethod, TypeResult> thisType;
      ReadPointerMethod rpmReadMethod = nullptr;
      TypeObject* poObject = nullptr;

     protected:
      virtual ComparisonResult _compare(const EnhancedObject& asource) const override
         {  ComparisonResult result = inherited::_compare(asource);
            const thisType& source = (const thisType&) asource;
            return (rpmReadMethod != source.rpmReadMethod) ? CRNonComparable : result;
         }
      virtual int getSize() const override { return sizeof(thisType); }
      virtual int getAlign() const override { return alignof(thisType); }
#include "StandardClasses/UndefineNew.h"
      virtual void moveTo(void* dest) override
         {  new (dest) thisType(std::move(*this)); }
      virtual void copyTo(void* dest) override
         {  new (dest) thisType(*this); }
#include "StandardClasses/DefineNew.h"

     public:
      TParseState() = default;
      TParseState(const ReadPointerMethod& readMethod)
         :  rpmReadMethod(readMethod) {}
      TParseState(TypeObject& object, const ReadPointerMethod& readMethod)
         :  rpmReadMethod(readMethod), poObject(&object) {}
      TParseState(const thisType& source) = default;
      TParseState(thisType&& source) = default;
      thisType& operator=(const thisType&) = default;
      Template3DefineCopy(TParseState, TypeObject, ReadPointerMethod, TypeResult)
      // DTemplate3DefineAssign(TParseState, TypeObject, ReadPointerMethod, TypeResult)

      virtual auto operator()(TStateStack<TypeArguments>& stateStack, ParseArgument& arguments)
         -> typename ParseArgument::ResultAction override
         {  return (poObject->*rpmReadMethod)(stateStack, arguments); }
      void setObject(TypeObject& object)
         {  AssumeCondition(rpmReadMethod && !poObject)
            poObject = &object;
         }
      const ReadPointerMethod& getStateMethod() const { return rpmReadMethod; }
      void change(TypeObject& object, ReadPointerMethod readMethod, int point)
         {  poObject = &object;
            rpmReadMethod = readMethod;
            inherited::point() = point;
         }
      bool hasObjectRead(const TypeObject& object, ReadPointerMethod readMethod)
         {  return (poObject == &object) && (rpmReadMethod == readMethod); }
      bool hasMethodRead(ReadPointerMethod readMethod)
         {  return (rpmReadMethod == readMethod); }
   };
   template <class TypeObject, typename ReadPointerMethod, class TypeParseMultiState, class TypeResult>
   class TLevelParseState : public TVirtualParseState<TypeResult> {
     private:
      typedef TVirtualParseState<TypeResult> inherited;
      typedef TLevelParseState<TypeObject, ReadPointerMethod, TypeParseMultiState, TypeResult> thisType;
      ReadPointerMethod rpmReadMethod = nullptr;
      TypeObject* poObject = nullptr;

     protected:
      virtual ComparisonResult _compare(const EnhancedObject& asource) const
         {  ComparisonResult result = inherited::_compare(asource);
            const thisType& source = (const thisType&) asource;
            return (rpmReadMethod != source.rpmReadMethod) ? CRNonComparable : result;
         }
      virtual int getSize() const override { return sizeof(thisType); }
      virtual int getAlign() const override { return alignof(thisType); }
#include "StandardClasses/UndefineNew.h"
      virtual void moveTo(void* dest) override
         {  new (dest) thisType(std::move(*this)); }
      virtual void copyTo(void* dest) override
         {  new (dest) thisType(*this); }
#include "StandardClasses/DefineNew.h"

     public:
      TLevelParseState() = default;
      TLevelParseState(TypeObject& object, const ReadPointerMethod& readMethod)
         :  rpmReadMethod(readMethod), poObject(&object) {}
      TLevelParseState(const thisType& source) = default;
      TLevelParseState(thisType&& source) = default;
      Template4DefineCopy(TLevelParseState, TypeObject, ReadPointerMethod, TypeParseMultiState, TypeResult)
      DTemplate4DefineAssign(TLevelParseState, TypeObject, ReadPointerMethod, TypeParseMultiState, TypeResult)

      virtual auto operator()(TStateStack<TypeArguments>& stateStack, ParseArgument& arguments) 
         -> typename ParseArgument::ResultAction override
         { return (poObject->*rpmReadMethod)((TypeParseMultiState&) stateStack,
             (typename TypeParseMultiState::ParseArgument&) arguments, (void*) &inherited::getSResult());
         }
      const ReadPointerMethod& getStateMethod() const { return rpmReadMethod; }
      void change(TypeObject& object, ReadPointerMethod readMethod, int point)
         {  poObject = &object;
            rpmReadMethod = readMethod;
            inherited::point() = point;
         }
      bool hasObjectRead(const TypeObject& object, ReadPointerMethod readMethod)
         {  return (poObject == &object) && (rpmReadMethod == readMethod); }
      bool hasMethodRead(ReadPointerMethod readMethod)
         {  return (rpmReadMethod == readMethod); }
   };

  protected:
#include "StandardClasses/UndefineNew.h"
   void realloc(size_t newSize)
      {  if (uAllocatedSize * 3 / 2 > newSize)
            newSize = (uAllocatedSize * 3 / 2);
         char* newMemoryChunk = new char[newSize];
         char* newCurrentPointer = pvCurrentPointer
            ? (newMemoryChunk + (pvCurrentPointer - pvContent)) : nullptr;
         try {
         char* newThisIter = newCurrentPointer, *newSourceIter = pvCurrentPointer;
         while (newSourceIter) {
            AssumeCondition(dynamic_cast<const VirtualParseState*>(reinterpret_cast<EnhancedObject*>(newSourceIter)))
            reinterpret_cast<VirtualParseState*>(newSourceIter)->moveTo(newThisIter);
            int shift = reinterpret_cast<VirtualParseState*>(newSourceIter)->uPreviousSize;
            reinterpret_cast<VirtualParseState*>(newSourceIter)->~VirtualParseState();
            reinterpret_cast<VirtualParseState*>(newThisIter)->uPreviousSize = shift;
            if (newSourceIter <= pvContent) {
               AssumeCondition(shift == 0)
               newSourceIter = nullptr;
               newThisIter = nullptr;
            }
            else {
               AssumeCondition(shift > 0)
               newSourceIter -= shift;
               newThisIter -= shift;
               AssumeCondition(newSourceIter >= pvContent)
            }
         }
         } catch (...) {
            delete [] newMemoryChunk;
            throw;
         }
         if (pvContent)
            delete [] pvContent;
         pvContent = newMemoryChunk;
         if (pvCurrentPointer)
            pvCurrentPointer = newCurrentPointer;
         uAllocatedSize = newSize;
      }
   template <class TypeObject, typename ReadPointerMethod, class SpecializedThis, class TypeResult>
   thisType& _shift(TypeObject& object, ReadPointerMethod parseMethod, SpecializedThis* thisState)
      {  typedef TLevelParseState<TypeObject, ReadPointerMethod, SpecializedThis, TypeResult> ParseState;
         int previousSize = pvCurrentPointer ? reinterpret_cast<VirtualParseState*>
               (pvCurrentPointer)->getSize() : 0;
         if ((pvCurrentPointer ? (pvCurrentPointer - pvContent) : 0) + previousSize
                  + sizeof(ParseState) + (uint64_t) pvCurrentPointer % alignof(ParseState)
               >= uAllocatedSize)
            realloc(uAllocatedSize + sizeof(ParseState) + (uint64_t) pvCurrentPointer % alignof(ParseState));
         pvCurrentPointer = pvCurrentPointer ? pvCurrentPointer+previousSize : pvContent;
         if ((uint64_t) pvCurrentPointer % alignof(ParseState) != 0) {
            int shift = ((uint64_t) pvCurrentPointer % alignof(ParseState));
            pvCurrentPointer += shift;
            previousSize += shift;
         }
         AssumeCondition(!pvCurrentPointer || dynamic_cast<const VirtualParseState*>(reinterpret_cast<EnhancedObject*>(pvCurrentPointer)))
         new (pvCurrentPointer) ParseState(object, parseMethod);
         reinterpret_cast<ParseState*>(pvCurrentPointer)->uPreviousSize = previousSize;
         return *this;
      }
#include "StandardClasses/DefineNew.h"

   template <class TypeObject, typename ReadPointerMethod, class SpecializedThis, class TypeResult>
   SpecializedThis& _change(TypeObject& object, ReadPointerMethod parseMethod, int newPoint, SpecializedThis* thisState)
      {  typedef TLevelParseState<TypeObject, ReadPointerMethod, SpecializedThis, TypeResult> ParseState;
         AssumeCondition(dynamic_cast<const ParseState*>(reinterpret_cast<EnhancedObject*>(pvCurrentPointer)))
         ParseState* lastState = static_cast<const ParseState*>(reinterpret_cast<EnhancedObject*>(pvCurrentPointer));
         lastState->change(object, parseMethod, newPoint);
         return (SpecializedThis&) *this;
      }
   template <class TypeObject, typename ReadPointerMethod, class SpecializedThis, class TypeResult>
   bool _tisAlive(TypeObject& object, ReadPointerMethod parseMethod, int level, SpecializedThis* thisState)
      {  typedef TLevelParseState<TypeObject, ReadPointerMethod, SpecializedThis, TypeResult> ParseState;
         char* currentPointer = pvCurrentPointer;
         while (currentPointer && level > 0) {
            AssumeCondition(dynamic_cast<const VirtualParseState*>(reinterpret_cast<EnhancedObject*>(currentPointer)))
            int shift = reinterpret_cast<VirtualParseState*>(currentPointer)->uPreviousSize;
            if (currentPointer <= pvContent) {
               AssumeCondition(shift == 0)
               currentPointer = nullptr;
            }
            else {
               AssumeCondition(shift > 0)
               currentPointer -= shift;
               AssumeCondition(currentPointer >= pvContent)
            }
            --level;
         }
         VirtualParseState* stateAtLevel = currentPointer ? static_cast<const VirtualParseState*>
            (reinterpret_cast<EnhancedObject*>(currentPointer)) : nullptr;
         return dynamic_cast<ParseState*>(stateAtLevel)
            && ((ParseState&) *stateAtLevel).hasObjectRead(object, parseMethod);
      }
   template <class TypeObject, typename ReadPointerMethod, class SpecializedThis, class TypeResult>
   bool _tisAlive(TypeObject* object, ReadPointerMethod parseMethod, int level, SpecializedThis* thisState)
      {  typedef TLevelParseState<TypeObject, ReadPointerMethod, SpecializedThis, TypeResult> ParseState;
         char* currentPointer = pvCurrentPointer;
         while (currentPointer && level > 0) {
            AssumeCondition(dynamic_cast<const VirtualParseState*>(reinterpret_cast<EnhancedObject*>(currentPointer)))
            int shift = reinterpret_cast<VirtualParseState*>(currentPointer)->uPreviousSize;
            if (currentPointer <= pvContent) {
               AssumeCondition(shift == 0)
               currentPointer = nullptr;
            }
            else {
               AssumeCondition(shift > 0)
               currentPointer -= shift;
               AssumeCondition(currentPointer >= pvContent)
            }
            --level;
         }
         VirtualParseState* stateAtLevel = currentPointer ? static_cast<const VirtualParseState*>
            (reinterpret_cast<EnhancedObject*>(currentPointer)) : nullptr;
         return dynamic_cast<ParseState*>(stateAtLevel)
            && ((ParseState&) *stateAtLevel).hasMethodRead(parseMethod);
      }
   template <class TypeObject, typename ReadPointerMethod, class SpecializedThis, class TypeResult>
   bool _tisParentAlive(TypeObject* object, ReadPointerMethod parseMethod, SpecializedThis* thisState)
      {  typedef TLevelParseState<TypeObject, ReadPointerMethod, SpecializedThis, TypeResult> ParseState;
         char* currentPointer = pvCurrentPointer;
         if (currentPointer) {
            AssumeCondition(dynamic_cast<const VirtualParseState*>(reinterpret_cast<EnhancedObject*>(currentPointer)))
            int shift = reinterpret_cast<VirtualParseState*>(currentPointer)->uPreviousSize;
            if (currentPointer <= pvContent) {
               AssumeCondition(shift == 0)
               currentPointer = nullptr;
            }
            else {
               AssumeCondition(shift > 0)
               currentPointer -= shift;
               AssumeCondition(currentPointer >= pvContent)
            }
         }
         VirtualParseState* stateAtLevel = currentPointer ? static_cast<const VirtualParseState*>
            (reinterpret_cast<EnhancedObject*>(currentPointer)) : nullptr;
         return dynamic_cast<ParseState*>(stateAtLevel)
            && ((ParseState&) *stateAtLevel).hasMethodRead(parseMethod);
      }

  public:
   TStateStack() : pvContent(nullptr), uAllocatedSize(0), pvCurrentPointer(nullptr) {}
   TStateStack(thisType&& source)
      :  pvContent(source.pvContent), uAllocatedSize(source.uAllocatedSize), pvCurrentPointer(source.pvCurrentPointer)
      {  source.pvContent = nullptr; source.uAllocatedSize = 0; source.pvCurrentPointer = nullptr; }
   TStateStack(const thisType& source)
      :  EnhancedObject(source), pvContent(nullptr), uAllocatedSize(0), pvCurrentPointer(nullptr)
      {  if (source.pvContent && source.pvCurrentPointer) {
            AssumeCondition(dynamic_cast<const VirtualParseState*>(reinterpret_cast<EnhancedObject*>(source.pvCurrentPointer)))
            uAllocatedSize = (source.pvCurrentPointer - source.pvContent)
               + reinterpret_cast<VirtualParseState*>(source.pvCurrentPointer)->getSize();
            pvContent = new char[uAllocatedSize];
            char* newThisIter = pvCurrentPointer, *newSourceIter = source.pvCurrentPointer;
            while (newSourceIter) {
               AssumeCondition(dynamic_cast<const VirtualParseState*>(reinterpret_cast<EnhancedObject*>(newSourceIter)))
               reinterpret_cast<VirtualParseState*>(newSourceIter)->copyTo(newThisIter);
               int shift = reinterpret_cast<VirtualParseState*>(newSourceIter)->uPreviousSize;
               reinterpret_cast<VirtualParseState*>(newThisIter)->uPreviousSize = shift;
               if (newSourceIter <= source.pvContent) {
                  AssumeCondition(shift == 0)
                  newSourceIter = nullptr;
                  newThisIter = nullptr;
               }
               else {
                  AssumeCondition(shift > 0)
                  newSourceIter -= shift;
                  newThisIter -= shift;
                  AssumeCondition(newSourceIter >= source.pvContent && newThisIter >= pvContent)
               }
            }
         }
      }
   thisType& operator=(thisType&& source)
      {  inherited::operator=(source);
         if (this != &source) {
            clear();
            pvContent = source.pvContent;
            uAllocatedSize = source.uAllocatedSize;
            pvCurrentPointer = source.pvCurrentPointer;
            source.pvContent = nullptr;
            source.uAllocatedSize = 0;
            source.pvCurrentPointer = nullptr;
         }
         return *this;
      }
   thisType& operator=(const thisType& source)
      {  return operator=(thisType(source)); }
   virtual ~TStateStack() { clear(); }
   TemplateDefineCopy(TStateStack, TypeArguments)

   void clear()
      {  while (pvCurrentPointer) {
            AssumeCondition(dynamic_cast<const VirtualParseState*>(reinterpret_cast<EnhancedObject*>(pvCurrentPointer)))
            int shift = reinterpret_cast<VirtualParseState*>(pvCurrentPointer)->uPreviousSize;
            reinterpret_cast<VirtualParseState*>(pvCurrentPointer)->~VirtualParseState();
            if (pvCurrentPointer <= pvContent) {
               AssumeCondition(shift == 0)
               pvCurrentPointer = nullptr;
            }
            else {
               AssumeCondition(shift > 0)
               pvCurrentPointer -= shift;
            }
         }
         if (pvContent)
            delete [] pvContent;
         pvContent = nullptr;
         uAllocatedSize = 0;
      }
   void swap(thisType& source)
      {  char* tmp = source.pvContent;
         source.pvContent = pvContent;
         pvContent = tmp;
         tmp = source.pvCurrentPointer;
         source.pvCurrentPointer = pvCurrentPointer;
         pvCurrentPointer = tmp;
         int allocTmp = source.uAllocatedSize;
         source.uAllocatedSize = uAllocatedSize;
         uAllocatedSize = allocTmp;
      }

   auto parse(ParseArgument& arguments)
         -> typename ParseArgument::ResultAction
      {  return (!pvCurrentPointer)
            ? (typename ParseArgument::ResultAction) 3 /* Finished */
            : (*reinterpret_cast<VirtualParseState*>(pvCurrentPointer))(*this, arguments);
      }
   template <class TypeObject, typename ReadPointerMethod, class TypeResult>
   thisType& shift(TypeObject& object, ReadPointerMethod parseMethod, TypeResult*)
      {  typedef TParseState<TypeObject, ReadPointerMethod, TypeResult> ParseState;
         int previousSize = pvCurrentPointer ? reinterpret_cast<VirtualParseState*>
               (pvCurrentPointer)->getSize() : 0;
         if ((pvCurrentPointer ? (pvCurrentPointer - pvContent) : 0) + previousSize
                  + sizeof(ParseState) + (uint64_t) pvCurrentPointer % alignof(ParseState)
               >= uAllocatedSize)
            realloc(uAllocatedSize + sizeof(ParseState) + (uint64_t) pvCurrentPointer % alignof(ParseState));
         pvCurrentPointer = pvCurrentPointer ? pvCurrentPointer+previousSize : pvContent;
         if ((uint64_t) pvCurrentPointer % alignof(ParseState) != 0) {
            int shift = ((uint64_t) pvCurrentPointer % alignof(ParseState));
            pvCurrentPointer += shift;
            previousSize += shift;
         }
         new (pvCurrentPointer) ParseState(object, parseMethod);
         reinterpret_cast<ParseState*>(pvCurrentPointer)->uPreviousSize = previousSize;
         return *this;
      }
   template <class TypeObject, typename ReadPointerMethod, class TypeResult>
   TParseState<TypeObject, ReadPointerMethod, TypeResult>& shiftResult(TypeObject* nullObject,
         ReadPointerMethod parseMethod, TypeResult&& result)
      {  typedef TParseState<TypeObject, ReadPointerMethod, TypeResult> ParseState;
         int previousSize = pvCurrentPointer ? reinterpret_cast<VirtualParseState*>
               (pvCurrentPointer)->getSize() : 0;
         if ((pvCurrentPointer ? (pvCurrentPointer - pvContent) : 0) + previousSize
                  + sizeof(ParseState) + (uint64_t) pvCurrentPointer % alignof(ParseState)
               >= uAllocatedSize)
            realloc(uAllocatedSize + sizeof(ParseState) + (uint64_t) pvCurrentPointer % alignof(ParseState));
         pvCurrentPointer = pvCurrentPointer ? pvCurrentPointer+previousSize : pvContent;
         if ((uint64_t) pvCurrentPointer % alignof(ParseState) != 0) {
            int shift = ((uint64_t) pvCurrentPointer % alignof(ParseState));
            pvCurrentPointer += + shift;
            previousSize += shift;
         }
         new (pvCurrentPointer) ParseState(parseMethod);
         reinterpret_cast<ParseState*>(pvCurrentPointer)->uPreviousSize = previousSize;
         auto& res = *reinterpret_cast<TParseState<TypeObject, ReadPointerMethod, TypeResult>*>(pvCurrentPointer);
         res.setResult(std::move(result));
         return res;
      }
   void* allocateResult(size_t sizeResult, int& previousSize)
      {  previousSize = pvCurrentPointer ? reinterpret_cast<VirtualParseState*>
               (pvCurrentPointer)->getSize() : 0;
         if ((pvCurrentPointer ? (pvCurrentPointer - pvContent) : 0) + previousSize
                  + sizeResult + (uint64_t) pvCurrentPointer % alignof(VirtualParseState)
               >= uAllocatedSize)
            realloc(uAllocatedSize + sizeResult + (uint64_t) pvCurrentPointer % alignof(VirtualParseState));
         pvCurrentPointer = pvCurrentPointer ? pvCurrentPointer+previousSize : pvContent;
         if ((uint64_t) pvCurrentPointer % alignof(VirtualParseState) != 0) {
            int shift = ((uint64_t) pvCurrentPointer % alignof(VirtualParseState));
            pvCurrentPointer += + shift;
            previousSize += shift;
         }
         return pvCurrentPointer;
      }
   void setResultPreviousSize(int previousSize)
      {  reinterpret_cast<VirtualParseState*>(pvCurrentPointer)->uPreviousSize = previousSize; }
   template <class TypeObject, typename ReadPointerMethod, class TypeResult>
   thisType& change(TypeObject& object, ReadPointerMethod parseMethod, int newPoint, TypeResult*)
      {  typedef TParseState<TypeObject, ReadPointerMethod, TypeResult> ParseState;
         ParseState* lastState = static_cast<const ParseState*>(reinterpret_cast<EnhancedObject*>(pvCurrentPointer));
         lastState->change(object, parseMethod, newPoint);
         return *this;
      }
   template <class TypeObject, typename ReadPointerMethod, class TypeResult>
   bool tisAlive(TypeObject& object, ReadPointerMethod parseMethod, int level, TypeResult*)
      {  typedef TParseState<TypeObject, ReadPointerMethod, TypeResult> ParseState;
         char* currentPointer = pvCurrentPointer;
         while (currentPointer && level > 0) {
            AssumeCondition(dynamic_cast<const VirtualParseState*>(reinterpret_cast<EnhancedObject*>(currentPointer)))
            int shift = reinterpret_cast<VirtualParseState*>(currentPointer)->uPreviousSize;
            if (currentPointer <= pvContent) {
               AssumeCondition(shift == 0)
               currentPointer = nullptr;
            }
            else {
               AssumeCondition(shift > 0)
               currentPointer -= shift;
               AssumeCondition(currentPointer >= pvContent)
            }
            --level;
         }
         VirtualParseState* stateAtLevel = currentPointer ? static_cast<const VirtualParseState*>
            (reinterpret_cast<EnhancedObject*>(currentPointer)) : nullptr;
         return dynamic_cast<ParseState*>(stateAtLevel)
            && ((ParseState&) *stateAtLevel).hasObjectRead(object, parseMethod);
      }
   template <class TypeObject, typename ReadPointerMethod, class TypeResult>
   bool tisAlive(TypeObject* object, ReadPointerMethod parseMethod, int level, TypeResult*)
      {  typedef TParseState<TypeObject, ReadPointerMethod, TypeResult> ParseState;
         char* currentPointer = pvCurrentPointer;
         while (currentPointer && level > 0) {
            AssumeCondition(dynamic_cast<const VirtualParseState*>(reinterpret_cast<EnhancedObject*>(currentPointer)))
            int shift = reinterpret_cast<VirtualParseState*>(currentPointer)->uPreviousSize;
            if (currentPointer <= pvContent) {
               AssumeCondition(shift == 0)
               currentPointer = nullptr;
            }
            else {
               AssumeCondition(shift > 0)
               currentPointer -= shift;
               AssumeCondition(currentPointer >= pvContent)
            }
            --level;
         }
         VirtualParseState* stateAtLevel = currentPointer ? static_cast<const VirtualParseState*>
            (reinterpret_cast<EnhancedObject*>(currentPointer)) : nullptr;
         return dynamic_cast<ParseState*>(stateAtLevel)
            && ((ParseState&) *stateAtLevel).hasMethodRead(parseMethod);
      }
   template <class TypeObject, typename ReadPointerMethod, class TypeResult>
   bool tisParentAlive(TypeObject* object, ReadPointerMethod parseMethod, TypeResult*)
      {  typedef TParseState<TypeObject, ReadPointerMethod, TypeResult> ParseState;
         char* currentPointer = pvCurrentPointer;
         if (currentPointer) {
            int shift = reinterpret_cast<VirtualParseState*>(currentPointer)->uPreviousSize;
            if (currentPointer <= pvContent) {
               AssumeCondition(shift == 0)
               currentPointer = nullptr;
            }
            else {
               AssumeCondition(shift > 0)
               currentPointer -= shift;
               AssumeCondition(currentPointer >= pvContent)
            }
         }
         VirtualParseState* stateAtLevel = currentPointer ? static_cast<const VirtualParseState*>
            (reinterpret_cast<EnhancedObject*>(currentPointer)) : nullptr;
         return dynamic_cast<ParseState*>(stateAtLevel)
            && ((ParseState&) *stateAtLevel).hasMethodRead(parseMethod);
      }
   bool isAlive(int level, int point) const
      {  char* currentPointer = pvCurrentPointer;
         while (currentPointer && level > 0) {
            AssumeCondition(dynamic_cast<const VirtualParseState*>(reinterpret_cast<EnhancedObject*>(currentPointer)))
            int shift = reinterpret_cast<VirtualParseState*>(currentPointer)->uPreviousSize;
            if (currentPointer <= pvContent) {
               AssumeCondition(shift == 0)
               currentPointer = nullptr;
            }
            else {
               AssumeCondition(shift > 0)
               currentPointer -= shift;
               AssumeCondition(currentPointer >= pvContent)
            }
            --level;
         }
         VirtualParseState* stateAtLevel = currentPointer ? static_cast<const VirtualParseState*>
            (reinterpret_cast<EnhancedObject*>(currentPointer)) : nullptr;
         return stateAtLevel && stateAtLevel->point() == point;
      }
   bool isLessThan(int level, int point) const
      {  char* currentPointer = pvCurrentPointer;
         while (currentPointer && level > 0) {
            AssumeCondition(dynamic_cast<const VirtualParseState*>(reinterpret_cast<EnhancedObject*>(currentPointer)))
            int shift = reinterpret_cast<VirtualParseState*>(currentPointer)->uPreviousSize;
            if (currentPointer <= pvContent) {
               AssumeCondition(shift == 0)
               currentPointer = nullptr;
            }
            else {
               AssumeCondition(shift > 0)
               currentPointer -= shift;
               AssumeCondition(currentPointer >= pvContent)
            }
            --level;
         }
         const VirtualParseState* stateAtLevel = currentPointer ? static_cast<const VirtualParseState*>
            (reinterpret_cast<EnhancedObject*>(currentPointer)) : nullptr;
         return stateAtLevel && stateAtLevel->point() < point;
      }
   thisType& reduce()
      {  VirtualParseState* oldState = reinterpret_cast<VirtualParseState*>(pvCurrentPointer);
         char* currentPointer = pvCurrentPointer;
         if (currentPointer) {
            int shift = reinterpret_cast<VirtualParseState*>(currentPointer)->uPreviousSize;
            if (currentPointer <= pvContent) {
               AssumeCondition(shift == 0)
               currentPointer = nullptr;
            }
            else {
               AssumeCondition(shift > 0)
               currentPointer -= shift;
               AssumeCondition(currentPointer >= pvContent)
            }
         }
         oldState->~VirtualParseState();
         pvCurrentPointer = currentPointer;
         return *this;
      }

   const int& point() const
      {  AssumeCondition(pvCurrentPointer)
         return reinterpret_cast<VirtualParseState*>(pvCurrentPointer)->point();
      }
   int& point()
      {  AssumeCondition(pvCurrentPointer)
         return reinterpret_cast<VirtualParseState*>(pvCurrentPointer)->point();
      }
   int getLevel() const
      {  char* currentPointer = pvCurrentPointer;
         int level = 0;
         while (currentPointer) {
            AssumeCondition(dynamic_cast<const VirtualParseState*>(reinterpret_cast<EnhancedObject*>(currentPointer)))
            int shift = reinterpret_cast<VirtualParseState*>(currentPointer)->uPreviousSize;
            if (currentPointer <= pvContent) {
               AssumeCondition(shift == 0)
               currentPointer = nullptr;
            }
            else {
               AssumeCondition(shift > 0)
               currentPointer -= shift;
               AssumeCondition(currentPointer >= pvContent)
            }
            ++level;
         }
         return level;
      }
   size_t getTotalSize() const { return pvCurrentPointer ? (pvCurrentPointer - pvContent) : 0; }

   VirtualParseState& last() { return *reinterpret_cast<VirtualParseState*>(pvCurrentPointer); }
   const VirtualParseState& last() const { return *reinterpret_cast<VirtualParseState*>(pvCurrentPointer); }
   bool isEmpty() const { return pvCurrentPointer == nullptr; }
   bool hasUpLast() const { return pvCurrentPointer && pvCurrentPointer > pvContent; }
   VirtualParseState& upLast() const
      {  char* currentPointer = pvCurrentPointer;
         if (currentPointer) {
            int shift = reinterpret_cast<VirtualParseState*>(currentPointer)->uPreviousSize;
            if (currentPointer <= pvContent) {
               AssumeCondition(shift == 0)
               currentPointer = nullptr;
            }
            else {
               AssumeCondition(shift > 0)
               currentPointer -= shift;
               AssumeCondition(currentPointer >= pvContent)
            }
         }
         AssumeCondition(currentPointer)
         return *reinterpret_cast<VirtualParseState*>(currentPointer);
      }

   template <class TypeResult>
   void absorbUnionResult(TypeResult&& result)
      {  AssumeCondition(pvCurrentPointer)
         reinterpret_cast<VirtualParseState*>(pvCurrentPointer)->absorbUnionResult(std::move(result));
      }
   template <class TypeResult>
   void freeUnionResult(TypeResult* typeResult)
      {  AssumeCondition(pvCurrentPointer)
         return reinterpret_cast<VirtualParseState*>(pvCurrentPointer)->freeUnionResult(typeResult);
      }
   template <class TypeResult>
   TypeResult& getUnionResult(TypeResult* typeResult)
      {  AssumeCondition(pvCurrentPointer)
         return reinterpret_cast<VirtualParseState*>(pvCurrentPointer)->getUnionResult(typeResult);
      }
   template <class TypeResult>
   TypeResult& getParentUnionResult(TypeResult* typeResult)
      {  return reinterpret_cast<VirtualParseState&>(upLast()).getUnionResult(typeResult); }
   // bool hasUnionResult() const
   //    {  AssumeCondition(pvCurrentPointer)
   //       return reinterpret_cast<VirtualParseState*>(pvCurrentPointer)->hasUnionResult();
   //    }
   // bool hasParentUnionResult() const
   //    {  return reinterpret_cast<VirtualParseState&>(upLast()).hasUnionResult(); }

   template <class TypeResult>
   void setResult(TypeResult&& result)
      {  AssumeCondition(pvCurrentPointer)
         reinterpret_cast<TVirtualParseState<TypeResult>*>(pvCurrentPointer)->setResult(std::move(result));
      }
   template <class TypeResult>
   TypeResult&& extractResult(TypeResult*)
      {  AssumeCondition(pvCurrentPointer)
         return reinterpret_cast<TVirtualParseState<TypeResult>*>(pvCurrentPointer)->extractResult();
      }
   template <class TypeResult>
   const TypeResult& getResult(TypeResult*)
      {  AssumeCondition(pvCurrentPointer)
         return reinterpret_cast<TVirtualParseState<TypeResult>*>(pvCurrentPointer)->getResult();
      }
   template <class TypeResult>
   TypeResult& getSResult(TypeResult*)
      {  AssumeCondition(pvCurrentPointer)
         return reinterpret_cast<TVirtualParseState<TypeResult>*>(pvCurrentPointer)->getSResult();
      }
   void* getResultPlace()
      {  AssumeCondition(pvCurrentPointer)
         return reinterpret_cast<VirtualParseState*>(pvCurrentPointer)->getResultPlace();
      }
   template <class TypeResult>
   const TypeResult& getParentResult(TypeResult*)
      {  return reinterpret_cast<TVirtualParseState<TypeResult>&>(upLast()).getResult(); }
   template <class TypeResult>
   TypeResult& getSParentResult(TypeResult*)
      {  return reinterpret_cast<TVirtualParseState<TypeResult>&>(upLast()).getSResult(); }
   template <class TypeResult>
   bool hasResult(TypeResult*) const
      {  AssumeCondition(pvCurrentPointer)
         return reinterpret_cast<TVirtualParseState<TypeResult>*>(pvCurrentPointer)->getResult().isValid();
      }
   template <class TypeResult>
   bool hasParentResult(TypeResult*) const
      {  return reinterpret_cast<TVirtualParseState<TypeResult>&>(upLast()).getResult().isValid(); }

   bool doesContainAddress(void* pointer) const
      {  return pointer && pvCurrentPointer && pointer >= pvContent
               && pointer < (pvCurrentPointer
                     + reinterpret_cast<VirtualParseState*>(pvCurrentPointer)->getSize());
      }
   size_t convertAddressToRelative(void* pointer) const
      {  return static_cast<char*>(pointer) - pvContent; }
   void* convertRelativeToAddress(size_t shift) const
      {  return reinterpret_cast<void*>(shift + reinterpret_cast<size_t>(pvContent)); }

   template <class Type>
   class TPureStackPointer;
   template <class Type>
   class TStackPointer {
     private:
      size_t ptPointer;
      bool fIsStack;
      friend class TPureStackPointer<Type>;

     public:
      TStackPointer(TStateStack<TypeArguments>& state, Type* pointer=nullptr)
         :  ptPointer(reinterpret_cast<size_t>(pointer)), fIsStack(false)
         {  if (state.doesContainAddress(pointer)) {
               ptPointer = state.convertAddressToRelative(pointer);
               fIsStack = true;
            }
         }
      TStackPointer(const TStackPointer<Type>& source) = default;
      TStackPointer(const TPureStackPointer<Type>& source)
         :  ptPointer(source.ptPointer), fIsStack(true) {}
      TStackPointer<Type>& operator=(const TStackPointer<Type>& source) = default;
      TStackPointer<Type>& operator=(const TPureStackPointer<Type>& source)
         {  ptPointer = source.ptPointer;
            fIsStack = true;
            return *this;
         }

      bool isStackPointer() const { return fIsStack; }
      Type& deref(TStateStack<TypeArguments>& state) const
         {  if (!fIsStack)
               return *reinterpret_cast<Type*>(ptPointer);
            else
               return *reinterpret_cast<Type*>(state.convertRelativeToAddress(ptPointer));
         }
      Type* value(TStateStack<TypeArguments>& state) const
         {  if (!fIsStack)
               return reinterpret_cast<Type*>(ptPointer);
            else
               return reinterpret_cast<Type*>(state.convertRelativeToAddress(ptPointer));
         }
   };

   template <class Type>
   class TPureStackPointer {
     private:
      size_t ptPointer;
      friend class TStackPointer<Type>;

     public:
      TPureStackPointer(TStateStack<TypeArguments>& state, Type* pointer)
         {  AssumeCondition(state.doesContainAddress(pointer))
            ptPointer = state.convertAddressToRelative(pointer);
         }
      TPureStackPointer(const TPureStackPointer<Type>& source) = default;
      TPureStackPointer<Type>& operator=(const TPureStackPointer<Type>& source) = default;

      Type& deref(TStateStack<TypeArguments>& state) const
         {  return *reinterpret_cast<Type*>(state.convertRelativeToAddress(ptPointer)); }
      Type* value(TStateStack<TypeArguments>& state) const
         {  return reinterpret_cast<Type*>(state.convertRelativeToAddress(ptPointer)); }
   };
   template <class Type> TStackPointer<Type> acquirePointer(Type* pointer)
      {  return TStackPointer<Type>(*this, pointer); }
   template <class Type> TStackPointer<Type> acquirePurePointer(Type* pointer)
      {  return TPureStackPointer<Type>(*this, pointer); }
};

}} // end of namespace STG::Parser


