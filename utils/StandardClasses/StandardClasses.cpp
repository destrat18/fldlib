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
// File      : StandardClasses.cpp
// Description :
//   Implementation of the class EnhancedObject, inherited by any object whose
//   derivations are open.
//

#include <iostream>
#include "StandardClasses/StandardClasses.hpp"

namespace {}

auto
EUserError::print(std::ostream& out) const -> std::ostream&
   {  return out << DefineSTD_UserError << std::endl; }

auto
ESPreconditionError::print(std::ostream& out) const -> std::ostream&
   {  return out << DefineSTD_PreconditionError << ' ' << szText << '\n' << DefineSTD_File << ' '
         << szFile << ", " << DefineSTD_Line << " " << uLine << std::endl;
   }

#if DefineDebugLevel > 1
ESPreconditionError::ESPreconditionError(const char *textSource, int lineSource, const char* fileSource)
   :  uLine(lineSource), szFile(fileSource), szText(textSource) {}
#endif

auto
ENotImplemented::print(std::ostream& out) const -> std::ostream&
   {  return out << DefineSTD_NotImplemented << ' ' << std::endl; }


#if DefineDebugLevel == 2

int EnhancedObject::uCountInstances = 0;

void
EnhancedObject::writeMessages(std::ostream& out)
{  if (uCountInstances > 0)
      out << uCountInstances << " memory leaks" << std::endl;
   else if (uCountInstances < 0)
      out << -uCountInstances << " objects multiply destroyed" << std::endl;
}

#elif DefineDebugLevel > 2

const char* EnhancedObject::pcReadFileSource = nullptr;
unsigned int EnhancedObject::uReadLineSource = 0;

int EnhancedObject::uCountInstances = 0;
EnhancedObject* EnhancedObject::peoFirst = nullptr;
EnhancedObject* EnhancedObject::peoMultipleRemoved = nullptr;

void
EnhancedObject::writeMessages(std::ostream& out) {
   if (peoFirst != nullptr) {
      EnhancedObject* cursor = peoFirst;
      do {
         out << "memory leak of " << cursor->pcFileSource << ", "
            << cursor->uLineSource << std::endl;
      } while ((cursor = cursor->peoNext) != peoFirst);
   };
   if (peoMultipleRemoved != nullptr) {
      EnhancedObject* cursor = peoMultipleRemoved;
      do {
         out << "multiple memory destruction of " << cursor->pcFileSource << ", " << cursor->uLineSource << std::endl;
      } while ((cursor = cursor->peoNext) != peoMultipleRemoved);
   };
}

namespace {

class BasicObject {
  public:
   virtual ~BasicObject() {}
};

}

int
EnhancedObject::queryAllocatedSize() {
   int uResult = 0;
   if (peoFirst != nullptr) {
      EnhancedObject* peoCursor = peoFirst;
      do {
         uResult += peoCursor->queryDebugSize() - sizeof(EnhancedObject) + sizeof(BasicObject);
      } while ((peoCursor = peoCursor->peoNext) != peoFirst);
   };
   return uResult;
}

#endif

