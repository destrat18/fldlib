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
// Unit      : Basic pointers
// File      : PassPointer.h
// Description :
//   Definition of the template PassPointer to tranfer control on the pointed element.
//   This class is especially interesting for the stack management, where PassPointer
//     are used for the arguments and for the result type of functions/methods.
//

#pragma once

#include "Pointer/Pointer.h"

namespace PNT {

// [TODO] Deprecated: use CPassPointer
template<class Element>
class PassPointer : public Pointer {
  protected:
   operator Element*() const { return (Element*) &Element::castFromCopyHandler(Pointer::getElement()); }

  public:
   typedef Element* PElement;
   typedef typename Pointer::Duplicate Duplicate;
   typedef typename Pointer::Init Init;

   PassPointer() = default;
   PassPointer(Element* element, Init) : Pointer(Element::castToCopyHandler(element), Init()) {}
   PassPointer(const Element& element) : Pointer(element.createCopy(), Init()) {}
   PassPointer(const PassPointer<Element>& source, Duplicate)
      {  if (source.Pointer::isValid())
            Pointer::setElement(source.Pointer::getElement().createCopy());
      }
   PassPointer(PassPointer<Element>&& source) : Pointer(source) { source.Pointer::release(); }
   PassPointer(const PassPointer<Element>& source)
      :  Pointer(source) { const_cast<PassPointer<Element>& >(source).Pointer::release(); }
   PassPointer<Element>& operator=(const PassPointer<Element>& source)
      {  Pointer::fullAssign(source, TPPass); return *this; }
   virtual ~PassPointer() { Pointer::release(true); }

   TemplateDefineCopy(PassPointer, Element)
   DTemplateDefineAssign(PassPointer, Element)
   
   void assign(Element* newElement, Init)
      {  Pointer::fullAssign(Element::castToCopyHandler(newElement), TPPass, Init()); }
   void fullAssign(const PassPointer<Element>& source)
      {  Pointer::fullAssign(source, TPAuto); }
   void setElement(const Element& element) { assign(element.createSCopy(), Init()); }
   void absorbElement(Element* element) { assign(element, Init()); }
   void swap(PassPointer<Element>& source) { Pointer::swap(source); }
   Element* extractElement()
      {  Element* result = (Element*) Element::castFromCopyHandler(&Pointer::getElement());
         Pointer::release();
         return result;
      }
   void release() { Pointer::release(true); }

   Element* key() const
      { return Pointer::isValid() ? (Element*) Element::castFromCopyHandler(&Pointer::getElement()) : nullptr; }
   Element* operator->() const { return (Element*) Element::castFromCopyHandler(Pointer::operator->()); }
   Element& operator*() const { return *(operator->()); }
   Element& getElement() const { return *(operator->()); }
};

template<class Element>
class CPassPointer : public Pointer {
  protected:
   operator Element*() const { return (Element*) &Element::castFromCopyHandler(Pointer::getElement()); }

  public:
   typedef Element* PElement;
   typedef typename Pointer::Duplicate Duplicate;
   typedef typename Pointer::Init Init;

   CPassPointer() = default;
   CPassPointer(Element* element, Init) : Pointer(Element::castToCopyHandler(element), Init()) {}
   CPassPointer(const Element& element) : Pointer(element.createCopy(), Init()) {}
   CPassPointer(const CPassPointer<Element>& source, Duplicate duplicate) 
      {  if (source.Pointer::isValid())
            Pointer::setElement(source.Pointer::getElement().createCopy());
      }
   CPassPointer(const CPassPointer<Element>& source) : Pointer()
      {  if (source.Pointer::isValid())
            Pointer::setElement(source.Pointer::getElement().createCopy());
      }
   CPassPointer(CPassPointer<Element>&& source) : Pointer(source) { source.Pointer::release(); }
   virtual ~CPassPointer() { Pointer::release(true); }
   CPassPointer<Element>& operator=(const CPassPointer<Element>& source)
      {  Pointer::fullAssign(source, TPAuto); return *this; }
   CPassPointer<Element>& operator=(CPassPointer<Element>&& source)
      {  Pointer::fullAssign(source, TPPass); return *this; }

   TemplateDefineCopy(CPassPointer, Element)
   DTemplateDefineAssign(CPassPointer, Element)
   void assign(Element* newElement, Init)
      {  Pointer::fullAssign(Element::castToCopyHandler(newElement), TPPass, Init()); }
   void fullAssign(const CPassPointer<Element>& source)
      {  Pointer::fullAssign(source, TPAuto); }
   void setElement(const Element& element) { assign(element.createSCopy(), Init()); }
   void absorbElement(Element* element) { assign(element, Init()); }
   void swap(CPassPointer<Element>& source) { Pointer::swap(source); }
   Element* extractElement()
      {  Element* result = (Element*) Element::castFromCopyHandler(&Pointer::getElement());
         Pointer::release();
         return result;
      }
   void release() { Pointer::release(true); }

   Element* key() const
      {  return Pointer::isValid() ? (Element*) Element::castFromCopyHandler(&Pointer::getElement()) : nullptr; }
   Element* operator->() const { return (Element*) Element::castFromCopyHandler(Pointer::operator->()); }
   Element& operator*() const { return *(operator->()); }
   Element& getElement() const { return *(operator->()); }
};

template<class Element>
class PPassPointer : public CPassPointer<Element> {
  private:
   typedef CPassPointer<Element> inherited;

  protected:
   PPassPointer(const PPassPointer<Element>& source) = delete;
   PPassPointer<Element>& operator=(const PPassPointer<Element>& source) = delete;

  public:
   PPassPointer() = default;
   PPassPointer(Element* element, typename inherited::Init) : inherited(element, typename inherited::Init()) {}
   PPassPointer(const Element& element) : inherited(element) {}
   PPassPointer(const PPassPointer<Element>& source, typename inherited::Duplicate duplicate) 
      :  inherited(source, duplicate) {}
   PPassPointer(PPassPointer<Element>&& source) = default;
   PPassPointer<Element>& operator=(PPassPointer<Element>&& source) = default;

   // TemplateDefineMCopy(PPassPointer, Element)
   virtual EnhancedObject* createMove() override
      {  return PPassPointer<Element>::castToCopyHandler(new PPassPointer<Element>(std::move(*this))); }
   PPassPointer<Element>* createSMove()
      {  return (PPassPointer<Element>*) PPassPointer<Element>::castFromCopyHandler(createMove()); }
};

template<class Element, class PassDerived>
class TPassPointer : public PassDerived {
  private:
   typedef TPassPointer<Element, PassDerived> thisType;

  public:
   typedef Element* PElement;

   TPassPointer() = default;
   TPassPointer(Element* element, Pointer::Init) : PassDerived(element, Pointer::Init()) {}
   TPassPointer(const Element& element) : PassDerived(element) {}
   TPassPointer(PassDerived&& source) : PassDerived(std::move(source)) {}
   TPassPointer(const PassDerived& source) : PassDerived(source) {}
   TPassPointer(const PassDerived& source, typename PassDerived::Duplicate duplicate)
      : PassDerived(source, duplicate) {}
   TPassPointer(thisType&& source) = default;
   TPassPointer(const thisType& source) = default;
   TPassPointer& operator=(const thisType& source) = default;
   TPassPointer& operator=(PassDerived&& source) { return (thisType&) PassDerived::operator=(std::move(source)); }
   TPassPointer& operator=(const PassDerived& source) { return (thisType&) PassDerived::operator=(source); }
   TPassPointer& operator=(thisType&& source) = default;

   // DTemplate2DefineMCopy(TPassPointer, Element, PassDerived)
   thisType* createSMove()
      {  return (thisType*) PassDerived::castFromCopyHandler(((PassDerived&) *this).createMove()); }
   thisType* createSCopy()
      {  return (thisType*) PassDerived::castFromCopyHandler(((const PassDerived&) *this).createCopy()); }

   void assign(const PassDerived& source) { PassDerived::assign(source); }
   void assign(Element* newElement, Pointer::Init) { PassDerived::assign(newElement, Pointer::Init()); }
   void setElement(const Element& element) { PassDerived::setElement(element); }
   void absorbElement(Element* element) { PassDerived::absorbElement(element); }
   Element* extractElement() { return (Element*) PassDerived::extractElement(); }

   Element* key() const { return (Element*) PassDerived::key(); }
   Element* operator->() const { return (Element*) PassDerived::operator->(); }
   Element& operator*() const { return (Element&) PassDerived::operator*(); }
   Element& getElement() const { return (Element&) PassDerived::operator*(); }
};

} // end of namespace PNT

