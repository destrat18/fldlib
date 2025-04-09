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
// File      : StandardMessage.h
// Description :
//   Definition of the interface for the error messages.
//

#pragma once

#define DefineSTD_InvalidPrecondition "Invalid precondition"
#define DefineSTD_InvalidObject "Invalid object"
#define DefineSTD_InvalidParameter "Invalid parameter"
#define DefineSTD_NotImplemented "Unimplemented method"
#define DefineSTD_UserError "User error"
#define DefineSTD_PreconditionError "Precondition error"
#define DefineSTD_File "file"
#define DefineSTD_Line "line"

class EUserError {
  public:
   EUserError() {}
   virtual ~EUserError() {}
   virtual std::ostream& print(std::ostream& out) const;

   friend std::ostream& operator<<(std::ostream& osOut, const EUserError& error)
      {  return error.print(osOut); }
};

class ESPreconditionError : public EUserError {
  private:
   int uLine = 0;
   const char* szFile = nullptr;
   const char* szText = nullptr;

  protected:
   const int& getLine() const { return uLine; }
   const char* getFile() const { return szFile; }
   const char* getText() const { return szText; }

  public:
   ESPreconditionError(const char *text) : szFile(""), szText(text) {}
   ESPreconditionError(const char *text, int line, const char* file)
#if DefineDebugLevel <= 1
      :  uLine(line), szFile(file), szText(text) {}
#else
      ;
#endif
   virtual std::ostream& print(std::ostream& out) const;
};

class ENotImplemented : public EUserError {
  public:
   ENotImplemented() {}
   virtual std::ostream& print(std::ostream& out) const;
};

class EMemoryExhausted {};

