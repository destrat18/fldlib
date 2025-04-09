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
// Library   : Standard classes
// Unit      : Basic object
// File      : StandardClasses.hpp
// Description :
//   Definition of the class EnhancedObject, inherited by any object whose
//   derivations are open.
//

#pragma once

#include <iosfwd>
#include <typeinfo>
#include <functional>
#include <cstdint>

#include "StandardMessage.h"
#include "StandardClasses.macro"

#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

enum ComparisonResult { CRLess=-1, CREqual=0, CRGreater=1, CRNonComparable=2 };

#if defined(__clang__) && DefineDebugLevel == 1
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

class EnhancedObject {

#if DefineDebugLevel > 2

  public:
   static const char* pcReadFileSource;
   static unsigned int uReadLineSource;

  private:
   static int uCountInstances;
   static EnhancedObject* peoFirst;
   static EnhancedObject* peoMultipleRemoved;

   EnhancedObject* peoNext;
   EnhancedObject* peoPrevious;
   const char* pcFileSource;
   int uLineSource;

   class Empty {};
   EnhancedObject(const EnhancedObject& source, Empty)
      :  peoNext(this), peoPrevious(this), pcFileSource(source.pcFileSource), uLineSource(source.uLineSource) {}

   void registerMultipleRemovedObject()
      {  EnhancedObject* copy = new EnhancedObject(*this, Empty());
         if (peoMultipleRemoved == nullptr)
            peoMultipleRemoved = copy;
         else {
            peoMultipleRemoved->peoPrevious->peoNext = copy;
            copy->peoNext = peoMultipleRemoved;
            copy->peoPrevious = peoMultipleRemoved->peoPrevious;
            peoMultipleRemoved->peoPrevious = copy;
         };
      }
   void registerObject()
      {  if (peoFirst == nullptr)
            peoFirst = this;
         else {
            peoFirst->peoPrevious->peoNext = this;
            peoNext = peoFirst;
            peoPrevious = peoFirst->peoPrevious;
            peoFirst->peoPrevious = this;
         };
      }
   void removeObject()
      {  if (peoNext == nullptr) // removing an object that has soon been removed.
            registerMultipleRemovedObject();
         else if (peoFirst == this) { // remove the first element
            if (peoNext != this) {
               peoFirst = peoNext;
               peoNext->peoPrevious = peoPrevious;
               peoPrevious->peoNext = peoNext;
            }
            else
               peoFirst = nullptr;
         }
         else if (peoNext != this) {
            peoNext->peoPrevious = peoPrevious;
            peoPrevious->peoNext = peoNext;
         };
         peoNext = nullptr;
         peoPrevious = nullptr;
      }
  public:
   EnhancedObject() : peoNext(this), peoPrevious(this), pcFileSource(nullptr), uLineSource(0)
      {  if (pcReadFileSource != nullptr) {
            registerObject();
            pcFileSource = pcReadFileSource;
            uLineSource = uReadLineSource;
            pcReadFileSource = nullptr;
            uReadLineSource = 0;
         };
      }
   EnhancedObject(const EnhancedObject& source)
      :  peoNext(this), peoPrevious(this), pcFileSource(nullptr), uLineSource(0)
      {  if (pcReadFileSource != nullptr) {
            registerObject();
            pcFileSource = pcReadFileSource;
            uLineSource = uReadLineSource;
            pcReadFileSource = nullptr;
            uReadLineSource = 0;
         };
      }
   virtual ~EnhancedObject() { removeObject(); }

   static void writeMessages(std::ostream& osOut);
   static void removeAllRegisteredObjects() { uCountInstances = 0; peoFirst = nullptr; }
   static int queryAllocatedSize();
   virtual int queryDebugSize() const { return sizeof(EnhancedObject); }
   virtual const char* queryDebugClassName() const { return "EnhancedObject" ; }

#define DefNew new
#define new (EnhancedObject::uReadLineSource = __LINE__, (EnhancedObject::pcReadFileSource = __FILE__) == nullptr) ? nullptr : DefNew 

#elif DefineDebugLevel > 1
  private:
   static int uCountInstances;
   
  public:
   EnhancedObject() { uCountInstances++; }
   EnhancedObject(const EnhancedObject& source) { uCountInstances++; }
   virtual ~EnhancedObject() { uCountInstances--; }

   static int queryDebugCount() { return uCountInstances; }
   static void writeMessages(std::ostream& out);
   virtual int queryDebugSize() const { return sizeof(EnhancedObject); }
   virtual const char* queryDebugClassName() const { return "EnhancedObject" ; }

#else

  public:
   EnhancedObject() {}
   EnhancedObject(const EnhancedObject&) {}
   virtual ~EnhancedObject() {}

#endif

  protected:
   virtual ComparisonResult _compare(const EnhancedObject&) const { return CREqual; }
   virtual void _assign(const EnhancedObject&) {}

  public:
   virtual EnhancedObject* createCopy() const
      {  return new EnhancedObject(*this); }
   virtual EnhancedObject* createMove()
      {  return new EnhancedObject(std::move(*this)); }
   EnhancedObject* createSCopy() const { return createCopy(); }
   EnhancedObject* createSMove() { return createMove(); }

   ComparisonResult compare(const EnhancedObject& source) const { return _compare(source); }
   bool isComparable(const EnhancedObject& typeObject) const { return _compare(typeObject) != CRNonComparable; }
   DAssign(EnhancedObject)

   static std::strong_ordering convertCompare(ComparisonResult comparisonResult)
      {  return (comparisonResult == CRLess
            ? std::strong_ordering::less : (comparisonResult == CRGreater
            ? std::strong_ordering::greater : std::strong_ordering::equal));
      }
   static std::partial_ordering convertComparePartial(ComparisonResult comparisonResult)
      {  return (comparisonResult == CRLess
            ? std::partial_ordering::less : (comparisonResult == CRGreater
            ? std::partial_ordering::greater : (comparisonResult == CREqual
            ? std::partial_ordering::equivalent : std::partial_ordering::unordered)));
      }
   static ComparisonResult convertToCompare(std::strong_ordering comparisonResult)
      {  return (comparisonResult < 0 ? CRLess : (comparisonResult > 0 ? CRGreater : CREqual)); }
   static ComparisonResult convertToCompare(std::partial_ordering comparisonResult)
      {  return (comparisonResult < 0 ? CRLess : (comparisonResult > 0 ? CRGreater
            : (comparisonResult == std::partial_ordering::equivalent ? CREqual : CRNonComparable)));
      }
   
   friend std::partial_ordering operator<=>(const EnhancedObject& first, const EnhancedObject& second)
      {  return convertComparePartial(first.compare(second)); }
   EnhancedObject& operator=(const EnhancedObject&) { return *this; }

   // Implementation of basic casts to avoid systematic dynamic_cast for the createSCopy() method
   static EnhancedObject* castToCopyHandler(EnhancedObject* source) { return source; }
   static const EnhancedObject* castToCopyHandler(const EnhancedObject* source) { return source; }
   static EnhancedObject& castToCopyHandler(EnhancedObject& source) { return source; }
   static const EnhancedObject& castToCopyHandler(const EnhancedObject& source) { return source; }

   static EnhancedObject* castFromCopyHandler(EnhancedObject* source) { return source; }
   static const EnhancedObject* castFromCopyHandler(const EnhancedObject* source) { return source; }
   static EnhancedObject& castFromCopyHandler(EnhancedObject& source) { return source; }
   static const EnhancedObject& castFromCopyHandler(const EnhancedObject& source) { return source; }

   virtual bool isValid() const { return true; }
};

typedef EnhancedObject* PEnhancedObject;
typedef const EnhancedObject* PCEnhancedObject;

template <typename Type>
inline ComparisonResult tcompare(Type fst, Type snd)
   { return fst < snd ? CRLess : (fst > snd) ? CRGreater : CREqual; }

inline ComparisonResult fcompare(int fst, int snd) { return tcompare<int>(fst, snd); }
inline ComparisonResult fcompare(const EnhancedObject& fst, const EnhancedObject& snd)
   { return fst.compare(snd); }

/******************************/
/* Definition of cast classes */
/******************************/

class SimpleCast {
  public:
   typedef EnhancedObject Base;
   static EnhancedObject* castFrom(EnhancedObject* source) { return source; }
   static const EnhancedObject* castFrom(const EnhancedObject* source) { return source; }
   static EnhancedObject& castFrom(EnhancedObject& source) { return source; }
   static const EnhancedObject& castFrom(const EnhancedObject& source) { return source; }

   static EnhancedObject* castTo(EnhancedObject* source) { return source; }
   static const EnhancedObject* castTo(const EnhancedObject* source) { return source; }
   static EnhancedObject& castTo(EnhancedObject& source) { return source; }
   static const EnhancedObject& castTo(const EnhancedObject& source) { return source; }
};

template <class Element, class TypeBase=EnhancedObject>
class HandlerCast {
  public:
   typedef TypeBase Base;
   static Element* castFrom(TypeBase* source) { return (Element*) source; }
   static const Element* castFrom(const TypeBase* source) { return (const Element*) source; }
   static Element& castFrom(TypeBase& source) { return (Element&) source; }
   static const Element& castFrom(const TypeBase& source) { return (const Element&) source; }

   static TypeBase* castTo(Element* source) { return (TypeBase*) source; }
   static const TypeBase* castTo(const Element* source) { return (const TypeBase*) source; }
   static TypeBase& castTo(Element& source) { return (TypeBase&) source; }
   static const TypeBase& castTo(const Element& source) { return (const TypeBase&) source; }
};

template <class Element, class TypeBase=EnhancedObject>
class HandlerDynamicCast {
  public:
   typedef TypeBase Base;
   static Element* castFrom(TypeBase* source) { return dynamic_cast<Element*>(source); }
   static const Element* castFrom(const TypeBase* source) { return dynamic_cast<const Element*>(source); }
   static Element& castFrom(TypeBase& source) { return (Element&) source; }
   static const Element& castFrom(const TypeBase& source) { return (const Element&) source; }

   static TypeBase* castTo(Element* source) { return source; }
   static const TypeBase* castTo(const Element* source) { return source; }
   static TypeBase& castTo(Element& source) { return source; }
   static const TypeBase& castTo(const Element& source) { return source; }
};

template <class Element>
class HandlerCopyCast {
  public:
   typedef EnhancedObject Base;
   static Element* castFrom(EnhancedObject* source) { return (Element*) Element::castFromCopyHandler(source); }
   static const Element* castFrom(const EnhancedObject* source) { return (const Element*) Element::castFromCopyHandler(source); }
   static Element& castFrom(EnhancedObject& source) { return (Element&) Element::castFromCopyHandler(source); }
   static const Element& castFrom(const EnhancedObject& source) { return (const Element&) Element::castFromCopyHandler(source); }

   static EnhancedObject* castTo(Element* source) { return Element::castToCopyHandler(source); }
   static const EnhancedObject* castTo(const Element* source) { return Element::castToCopyHandler(source); }
   static EnhancedObject& castTo(Element& source) { return Element::castToCopyHandler(source); }
   static const EnhancedObject& castTo(const Element& source) { return Element::castToCopyHandler(source); }
};

template <class Element, class Intermediate, class IntermediateCast=HandlerCast<Intermediate> >
class HandlerMultiCast {
  public:
   typedef typename IntermediateCast::Base Base;
   static Base& castTo(Element& source)
      { return IntermediateCast::castTo((Intermediate&) source); }
   static Base* castTo(Element* source)
      { return IntermediateCast::castTo((Intermediate*) source); }
   static const Base& castTo(const Element& source)
      { return IntermediateCast::castTo((const Intermediate&) source); }
   static const Base* castTo(const Element* source)
      { return IntermediateCast::castTo((const Intermediate*) source); }

   static Element& castFrom(Base& source)
      { return (Element&) (Intermediate&) IntermediateCast::castFrom(source); }
   static Element* castFrom(Base* source)
      { return (Element*) (Intermediate*) IntermediateCast::castFrom(source); }
   static const Element& castFrom(const Base& source)
      { return (const Element&) (const Intermediate&) IntermediateCast::castFrom(source); }
   static const Element* castFrom(const Base* source)
      { return (const Element*) (const Intermediate*) IntermediateCast::castFrom(source); }
};

template <class Element, class IntermediateCast, class BaseCast>
class HandlerLinkedCast {
  public:
   typedef typename BaseCast::Base Base;
   static Base& castTo(Element& source)
      { return BaseCast::castTo(IntermediateCast::castTo(source)); }
   static Base* castTo(Element* source)
      { return BaseCast::castTo(IntermediateCast::castTo(source)); }
   static const Base& castTo(const Element& source)
      { return BaseCast::castTo(IntermediateCast::castTo(source)); }
   static const Base* castTo(const Element* source)
      { return BaseCast::castTo(IntermediateCast::castTo(source)); }

   static Element& castFrom(Base& source)
      { return (Element&) IntermediateCast::castFrom((typename IntermediateCast::Base&) BaseCast::castFrom(source)); }
   static Element* castFrom(Base* source)
      { return (Element*) IntermediateCast::castFrom((typename IntermediateCast::Base*) BaseCast::castFrom(source)); }
   static const Element& castFrom(const Base& source)
      { return (const Element&) IntermediateCast::castFrom((const typename IntermediateCast::Base&) BaseCast::castFrom(source)); }
   static const Element* castFrom(const Base* source)
      { return (const Element*) IntermediateCast::castFrom((const typename IntermediateCast::Base*) BaseCast::castFrom(source)); }
};

template <class Element, class Intermediate, class TypeBase>
class HandlerIntermediateCast {
  public:
   typedef TypeBase Base;
   static Base& castTo(Element& source) { return (Intermediate&) source; }
   static Base* castTo(Element* source) { return (Intermediate*) source; }
   static const Base& castTo(const Element& source) { return (const Intermediate&) source; }
   static const Base* castTo(const Element* source) { return (const Intermediate*) source; }

   static Element& castFrom(Base& source) { return (Element&) (Intermediate&) source; }
   static Element* castFrom(Base* source) { return (Element*) (Intermediate*) source; }
   static const Element& castFrom(const Base& source) { return (const Element&) (const Intermediate&) source; }
   static const Element* castFrom(const Base* source) { return (const Element*) (const Intermediate*) source; }
};

template <class TypeElement, class TypeCast>
class TemplateElementCastParameters {
  public:
   typedef TypeElement Element;
   typedef TypeCast Cast;
};

template <class TypeElement, class TypeKey, class TypeCast>
class TemplateElementKeyCastParameters {
  public:
   typedef TypeElement Element;
   typedef TypeCast Cast;
   typedef TypeKey Key;

   static typename TypeKey::ControlKeyType key(const TypeElement& element) { return TypeKey::key(element); }
   static ComparisonResult keyCompare(typename TypeKey::KeyType fstKey, typename TypeKey::KeyType sndKey)
      {  return TypeKey::compare(fstKey, sndKey); }
};

/**********************************************/
/* Definition of the class ExtendedParameters */
/**********************************************/
// only valid if END_OF_MASK <= 32. To extend it, it is necessary to add fields
// either by multiple inheritance or by hand.
//

class ExtendedParameters {
  protected:
   typedef uint32_t ParametersContent;

  private:
   uint32_t uParams;

  protected:
   uint32_t& params() { return uParams; }
   const uint32_t& queryParams() const { return uParams; }
   static const int END_OF_MASK = 0;
   static const uint32_t MASK = 0;

  public:
   ExtendedParameters() : uParams(0) {}
   ExtendedParameters(const ExtendedParameters& source) = default;
   ExtendedParameters& operator=(const ExtendedParameters& source) = default;
};

class EExtendedParameters {
  protected:
   typedef uint64_t ParametersContent;

  private:
   uint64_t uParams;

  protected:
   uint64_t& params() { return uParams; }
   const uint64_t& queryParams() const { return uParams; }
   static const int END_OF_MASK = 0;
   static const uint64_t MASK = 0;

  public:
   EExtendedParameters() : uParams(0) {}
   EExtendedParameters(const EExtendedParameters& source) = default;
   EExtendedParameters& operator=(const EExtendedParameters& source) = default;
};

/***************************************/
/* Definition of the class VirtualCast */
/***************************************/

class VirtualCast : public ExtendedParameters {
  protected:
   virtual EnhancedObject* _castFrom(EnhancedObject* /* copyObject */) const { AssumeUncalled return nullptr; }
   virtual EnhancedObject* _castTo(EnhancedObject* /* collectionObject */) const { AssumeUncalled return nullptr; }

  public:
   VirtualCast() = default;
   VirtualCast(const VirtualCast& source) = default;
   virtual ~VirtualCast() {}

   EnhancedObject* castFrom(EnhancedObject* copyObject) const { return _castFrom(copyObject); }
   EnhancedObject& castFrom(EnhancedObject& copyObject) const { return *_castFrom(&copyObject); }
   const EnhancedObject* castFrom(const EnhancedObject* copyObject) const
      {  return _castFrom(const_cast<EnhancedObject*>(copyObject)); }
   const EnhancedObject& castFrom(const EnhancedObject& copyObject) const
      {  return *_castFrom(const_cast<EnhancedObject*>(&copyObject)); }

   EnhancedObject* castTo(EnhancedObject* collectionObject) const { return _castTo(collectionObject); }
   EnhancedObject& castTo(EnhancedObject& collectionObject) const { return *_castTo(&collectionObject); }
   const EnhancedObject* castTo(const EnhancedObject* collectionObject) const
      {  return _castTo(const_cast<EnhancedObject*>(collectionObject)); }
   const EnhancedObject& castTo(const EnhancedObject& collectionObject) const
      {  return *_castTo(const_cast<EnhancedObject*>(&collectionObject)); }
};

template <class TypeElement, class TypeCast>
class TVirtualCast : public VirtualCast {
  protected:
   virtual EnhancedObject* _castFrom(EnhancedObject* copyObject) const
      {  return TypeElement::castToCopyHandler((TypeElement*) 
            TypeCast::castFrom((typename TypeCast::Base*) TypeCast::Base::castFromCopyHandler(copyObject)));
      }
   virtual EnhancedObject* _castTo(EnhancedObject* collectionObject) const
      {  return TypeCast::Base::castToCopyHandler((typename TypeCast::Base*) 
            TypeCast::castTo((TypeElement*) TypeElement::castFromCopyHandler(collectionObject)));
      }

  public:
   TVirtualCast() = default;
   TVirtualCast(const TVirtualCast<TypeElement, TypeCast>& source) = default;
};

