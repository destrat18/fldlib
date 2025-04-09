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
// Unit      : NumericalDomains
// File      : TypesSizes.cpp
// Description :
//   Definition of a class of floating point comparison
//

#include "FloatAffine.h"
#include "FloatInterval.h"
#include "FloatExact.h"
#include "NumericalAnalysis/FloatAffineExecutionPath.template"
#include <vector>
#include <iostream>
#include "FloatInstrumentation/FloatAffine.incc"

int main(int argc, char** argv) {
   std::string headerAffineFile = "./obj_interface/AffineTypesSize.h";
   std::string headerIntervalFile = "./obj_interface/IntervalTypesSize.h";
   std::string headerExactFile = "./obj_interface/ExactTypesSize.h";
   static const int LDBL_EXPONENT_DIG
      = NumericalDomains::DDoubleExact::FloatDigitsHelper::TFloatDigits<long double>::UBitSizeExponent;
   // static const int LDBL_FULL_EXPONENT_DIG
   //    = NumericalDomains::DDoubleExact::FloatDigitsHelper::TFloatDigits<long double>::UBitFullSizeExponent;

   {  std::ofstream fileAffine(headerAffineFile.c_str());
      fileAffine << "namespace NumericalDomains { namespace DAffineInterface {\n\n";
      fileAffine << "static const int PathExplorerSize = "
                 << sizeof(NumericalDomains::DAffine::PathExplorer) << ";\n";
      fileAffine << "static const int MergeBranchesSize = "
                 << sizeof(NumericalDomains::DAffine::MergeBranches) << ";\n";

      int sizeZonotopeConstant;
      int sizeZonotopeVariableExponent, sizeZonotopeVariableMantissa;
      
#if !defined(FLOAT_GENERIC_BASE_UNSIGNED) && !defined(FLOAT_GENERIC_BASE_LONG)
      static const int sizeCell = sizeof(Numerics::UnsignedBaseStoreTraits::BaseType);
#elif defined(FLOAT_GENERIC_BASE_LONG)
      static const int sizeCell = sizeof(Numerics::UnsignedLongBaseStoreTraits::BaseType);
#else // defined(FLOAT_GENERIC_BASE_UNSIGNED)
      static const int sizeCell = sizeof(Numerics::UnsignedBaseStoreTraits::BaseType);
#endif

      // sizeof(NumericalDomains::DAffine::TInstrumentedFloatZonotope<23, 8, float>)
      //    = sizeZonotopeConstant + sizeof(float)
      //    + sizeZonotopeVariableExponent*((8+8*sizeCell-1)/(8*sizeCell))
      //    + sizeZonotopeVariableMantissa*((23+8*sizeCell-1)/(8*sizeCell))
      //    + sizeZonotopeRealMantissa*((FLOAT_REAL_BITS_NUMBER+8*sizeCell-1)/(8*sizeCell));
      // sizeof(NumericalDomains::DAffine::TInstrumentedFloatZonotope<52, 11, double>)
      //    = sizeZonotopeConstant + sizeof(double)
      //    + sizeZonotopeVariableExponent*((11+8*sizeCell-1)/(8*sizeCell))
      //    + sizeZonotopeVariableMantissa*((52+8*sizeCell-1)/(8*sizeCell))
      //    + sizeZonotopeRealMantissa*((FLOAT_REAL_BITS_NUMBER+8*sizeCell-1)/(8*sizeCell));
      // sizeof(NumericalDomains::DAffine::TInstrumentedFloatZonotope<LDBL_MANT_DIG-1, LDBL_EXPONENT_DIG, long double>)
      //    = sizeZonotopeConstant + sizeof(long double)
      //    + sizeZonotopeVariableExponent*((LDBL_EXPONENT_DIG+8*sizeCell-1)/(8*sizeCell))
      //    + sizeZonotopeVariableMantissa*((LDBL_MANT_DIG-1+8*sizeCell-1)/(8*sizeCell))
      //    + sizeZonotopeRealMantissa*((FLOAT_REAL_BITS_NUMBER+8*sizeCell-1)/(8*sizeCell));

#ifdef FLOAT_CONCRETE
#if !defined(FLOAT_GENERIC_BASE_UNSIGNED) && !defined(FLOAT_GENERIC_BASE_LONG)
      typedef NumericalDomains::DAffine::TBaseFloatZonotope<FLOAT_REAL_BITS_NUMBER, NumericalDomains::DAffine::TBaseFloatAffine<NumericalDomains::DAffine::ExecutionPath> > AlignType;
#elif defined(FLOAT_GENERIC_BASE_LONG)
      typedef NumericalDomains::DAffine::TGBaseFloatZonotope<Numerics::UnsignedLongBaseStoreTraits, FLOAT_REAL_BITS_NUMBER, NumericalDomains::DAffine::TBaseFloatAffine<NumericalDomains::DAffine::ExecutionPath> > AlignType;
#else // defined(FLOAT_GENERIC_BASE_UNSIGNED)
      typedef NumericalDomains::DAffine::TGBaseFloatZonotope<Numerics::UnsignedBaseStoreTraits, FLOAT_REAL_BITS_NUMBER, NumericalDomains::DAffine::TBaseFloatAffine<NumericalDomains::DAffine::ExecutionPath> > AlignType;
#endif
#endif
      int a0 = sizeof(NumericalDomains::DAffine::TInstrumentedFloatZonotope<23, 8, float>); // 288 - 4
#ifdef FLOAT_CONCRETE
      a0 -= sizeof(float);
      a0 += (alignof(float) < alignof(AlignType) ? sizeof(float) % alignof(AlignType) : 0); // (288-4) + 4
      a0 -= (alignof(float) > alignof(AlignType) ? (alignof(float) - sizeof(AlignType)%alignof(float))%alignof(float) : 0);
#endif
      int b0 = ((8+8*sizeCell-1)/(8*sizeCell));    // 1
      int c0 = ((23+8*sizeCell-1)/(8*sizeCell));   // 1
      int a1 = sizeof(NumericalDomains::DAffine::TInstrumentedFloatZonotope<52, 11, double>); // 296 - 8
#ifdef FLOAT_CONCRETE
      a1 -= sizeof(double);
      a1 += (alignof(double) < alignof(AlignType) ? sizeof(double) % alignof(AlignType) : 0);
      a1 -= (alignof(double) > alignof(AlignType) ? (alignof(double) - sizeof(AlignType)%alignof(double))%alignof(double) : 0);
#endif
      int b1 = ((11+8*sizeCell-1)/(8*sizeCell));   // 1
      int c1 = ((52+8*sizeCell-1)/(8*sizeCell));   // 1
      int a2 = sizeof(NumericalDomains::DAffine::TInstrumentedFloatZonotope<LDBL_MANT_DIG-1, LDBL_EXPONENT_DIG, long double>); // 304 - 16
#ifdef FLOAT_CONCRETE
      a2 -= sizeof(long double);
      a2 += (alignof(long double) < alignof(AlignType) ? sizeof(long double) % alignof(AlignType) : 0);
      a2 -= (alignof(long double) > alignof(AlignType) ? (alignof(long double) - sizeof(AlignType)%alignof(long double))%alignof(long double) : 0);
#endif
      int b2 = ((LDBL_EXPONENT_DIG+8*sizeCell-1)/(8*sizeCell));   // 1
      int c2 = ((LDBL_MANT_DIG-1+8*sizeCell-1)/(8*sizeCell));     // 1

      // a0 = 288 = sizeZonotopeConstant + sizeZonotopeVariableExponent*b0 + sizeZonotopeVariableMantissa*c0;
      // a1 = 288 = sizeZonotopeConstant + sizeZonotopeVariableExponent*b1 + sizeZonotopeVariableMantissa*c1;
      // a2 = 288 = sizeZonotopeConstant + sizeZonotopeVariableExponent*b2 + sizeZonotopeVariableMantissa*c2;

      // a1-a0 = sizeZonotopeVariableExponent*(b1-b0) + sizeZonotopeVariableMantissa*(c1-c0);
      // a2-a0 = sizeZonotopeVariableExponent*(b2-b0) + sizeZonotopeVariableMantissa*(c2-c0);

      // (c1-c0)*(a2-a0) - (c2-c0)*(a1-a0) = sizeZonotopeVariableExponent*((b2-b0)*(c1-c0) - (b1-b0)*(c2-c0));

      int exponentNumerator = (c1-c0)*(a2-a0) - (c2-c0)*(a1-a0),
          exponentDenominator = ((b2-b0)*(c1-c0) - (b1-b0)*(c2-c0));
      AssumeCondition((exponentDenominator == 0 && exponentNumerator == 0)
            || (exponentDenominator > 0 && exponentNumerator % exponentDenominator == 0))

      if (exponentDenominator == 0)
         sizeZonotopeVariableExponent = 0;
      else
         sizeZonotopeVariableExponent = exponentNumerator / exponentDenominator;

      AssumeCondition(((c2 == c0) && (a2-a0 == sizeZonotopeVariableExponent*(b2-b0)))
            || c2 > c0)
      if (c2 == c0)
         sizeZonotopeVariableMantissa = 0;
      else
         sizeZonotopeVariableMantissa = (a2-a0 - sizeZonotopeVariableExponent*(b2-b0))/(c2 - c0);

      sizeZonotopeConstant = a0 - sizeZonotopeVariableExponent*b0 - sizeZonotopeVariableMantissa*c0;
      // size = sizeZonotopeConstant + sizeof(TypeImplementation) /* warning: alignof */
      //    + sizeZonotopeVariableExponent*((USizeExponent+8*sizeCell-1)/(8*sizeCell))
      //    + sizeZonotopeVariableMantissa*((USizeMantissa+8*sizeCell-1)/(8*sizeCell));
      fileAffine << "template <int USizeMantissa, int USizeExponent, typename TypeImplementation>\n"
                 << "struct TFloatZonotopeSizeTraits {\n"
                 << "   static const int USize = " << sizeZonotopeConstant
#ifdef FLOAT_CONCRETE
                     << " + sizeof(TypeImplementation) /* warning: alignof */\n"
                     << "         - (alignof(TypeImplementation) < " << alignof(AlignType)
                        << " ? sizeof(TypeImplementation) % " << alignof(AlignType) << " : 0)\n"
                     << "         + (alignof(TypeImplementation) > " << alignof(AlignType)
                        << " ? (alignof(TypeImplementation)-1\n"
                        << "            - " << (sizeof(AlignType)-1)
                     << " % alignof(TypeImplementation)) : 0)"
#endif
                     << "\n         + " << sizeZonotopeVariableExponent << " * ((USizeExponent+"
                           << 8*sizeCell-1 << ") / (" << 8*sizeCell << "))\n"
                     << "         + " << sizeZonotopeVariableMantissa << " * ((USizeMantissa+"
                           << 8*sizeCell-1 << ") / (" << 8*sizeCell << "));\n"
                 << "};\n\n";

      fileAffine << "}} // end of namespace NumericalDomains::DAffineInterface\n";
   };

   {  std::ofstream fileInterval(headerIntervalFile.c_str());
      fileInterval << "namespace NumericalDomains { namespace DDoubleIntervalInterface {\n\n";
      fileInterval << "static const int PathExplorerSize = "
                 << sizeof(NumericalDomains::DDoubleInterval::PathExplorer) << ";\n";

      int sizeIntervalConstant;
      int sizeIntervalVariableExponent, sizeIntervalVariableMantissa;
      
// #if !defined(FLOAT_GENERIC_BASE_UNSIGNED) && !defined(FLOAT_GENERIC_BASE_LONG)
      static const int sizeCell = sizeof(Numerics::UnsignedBaseStoreTraits::BaseType);
// #elif defined(FLOAT_GENERIC_BASE_LONG)
//       static const int sizeCell = sizeof(Numerics::UnsignedLongBaseStoreTraits::BaseType);
// #else // defined(FLOAT_GENERIC_BASE_UNSIGNED)
//       static const int sizeCell = sizeof(Numerics::UnsignedBaseStoreTraits::BaseType);
// #endif

      // sizeof(NumericalDomains::DDoubleInterval::TInstrumentedFloatInterval<NumericalDomains::DDoubleInterval::TBuiltFloat<LDBL_MANT_DIG, 23, 8>, float>)
      //    = sizeIntervalConstant + sizeof(float)
      //    + sizeIntervalVariableExponent*((8+8*sizeCell-1)/(8*sizeCell))
      //    + sizeIntervalVariableMantissa*((23+8*sizeCell-1)/(8*sizeCell))
      // sizeof(NumericalDomains::DDoubleInterval::TInstrumentedFloatInterval<NumericalDomains::DDoubleInterval::TBuiltFloat<LDBL_MANT_DIG, 52, 11>, double>)
      //    = sizeIntervalConstant + sizeof(double)
      //    + sizeIntervalVariableExponent*((11+8*sizeCell-1)/(8*sizeCell))
      //    + sizeIntervalVariableMantissa*((52+8*sizeCell-1)/(8*sizeCell))
      // sizeof(NumericalDomains::DDoubleInterval::TInstrumentedFloatInterval<NumericalDomains::DDoubleInterval::TBuiltFloat<LDBL_MANT_DIG, LDBL_MANT_DIG-1, LDBL_EXPONENT_DIG>, long double>)
      //    = sizeIntervalConstant + sizeof(long double)
      //    + sizeIntervalVariableExponent*((LDBL_EXPONENT_DIG+8*sizeCell-1)/(8*sizeCell))
      //    + sizeIntervalVariableMantissa*((LDBL_MANT_DIG-1+8*sizeCell-1)/(8*sizeCell))

      // typedef void* AlignType;
      int a0 = sizeof(NumericalDomains::DDoubleInterval::TInstrumentedFloatInterval<NumericalDomains::DDoubleInterval::TBuiltFloat<LDBL_MANT_DIG, 23, 8>, float>);
#ifdef FLOAT_CONCRETE
      a0 -= sizeof(float);
      a0 -= 2*(((23 + 31) / 32) % 2 == 0 ? 8 : 4); // 28 - 4
#endif
//       + (alignof(float) < alignof(AlignType) ? sizeof(float) % alignof(AlignType) : 0)
//       - (alignof(float) > alignof(AlignType) ? (alignof(float) - sizeof(AlignType)%alignof(float))%alignof(float) : 0);
      int b0 = ((8+8*sizeCell-1)/(8*sizeCell));    // 1
      int c0 = ((23+8*sizeCell-1)/(8*sizeCell));   // 1
      int a1 = sizeof(NumericalDomains::DDoubleInterval::TInstrumentedFloatInterval<NumericalDomains::DDoubleInterval::TBuiltFloat<LDBL_MANT_DIG, 52, 11>, double>);
#ifdef FLOAT_CONCRETE
      a1 -= sizeof(double);
      a1 -= 2*(((52 + 31) / 32) % 2 == 0 ? 8 : 4); // 40 - 8
#endif
//       + (alignof(double) < alignof(AlignType) ? sizeof(double) % alignof(AlignType) : 0)
//       - (alignof(double) > alignof(AlignType) ? (alignof(double) - sizeof(AlignType)%alignof(double))%alignof(double) : 0);
      int b1 = ((11+8*sizeCell-1)/(8*sizeCell));   // 1
      int c1 = ((52+8*sizeCell-1)/(8*sizeCell));   // 2
      int a2 = sizeof(NumericalDomains::DDoubleInterval::TInstrumentedFloatInterval<NumericalDomains::DDoubleInterval::TBuiltFloat<LDBL_MANT_DIG, LDBL_MANT_DIG-1, LDBL_EXPONENT_DIG>, long double>);
#ifdef FLOAT_CONCRETE
      a2 -= sizeof(long double);
      a2 -= 2*(((LDBL_MANT_DIG-1 + 31) / 32) % 2 == 0 ? 8 : 4); // 48 - 16
#endif
//       + (alignof(long double) < alignof(AlignType) ? sizeof(long double) % alignof(AlignType) : 0)
//       - (alignof(long double) > alignof(AlignType) ? (alignof(long double) - sizeof(AlignType)%alignof(long double))%alignof(long double) : 0);
      int b2 = ((LDBL_EXPONENT_DIG+8*sizeCell-1)/(8*sizeCell)); // 1
      int c2 = ((LDBL_MANT_DIG-1+8*sizeCell-1)/(8*sizeCell));   // 2

      // a0 = sizeIntervalConstant + sizeIntervalVariableExponent*b0 + sizeIntervalVariableMantissa*c0;
      // a1 = sizeIntervalConstant + sizeIntervalVariableExponent*b1 + sizeIntervalVariableMantissa*c1;
      // a2 = sizeIntervalConstant + sizeIntervalVariableExponent*b2 + sizeIntervalVariableMantissa*c2;

      // a1-a0 = sizeIntervalVariableExponent*(b1-b0) + sizeIntervalVariableMantissa*(c1-c0);
      // a2-a0 = sizeIntervalVariableExponent*(b2-b0) + sizeIntervalVariableMantissa*(c2-c0);

      // (c1-c0)*(a2-a0) - (c2-c0)*(a1-a0) = sizeIntervalVariableExponent*((b2-b0)*(c1-c0) - (b1-b0)*(c2-c0));

      int exponentNumerator = (c1-c0)*(a2-a0) - (c2-c0)*(a1-a0),
          exponentDenominator = ((b2-b0)*(c1-c0) - (b1-b0)*(c2-c0));
      AssumeCondition((exponentDenominator == 0 && exponentNumerator == 0)
            || (exponentDenominator > 0 && exponentNumerator % exponentDenominator == 0))

      if (exponentDenominator == 0)
         sizeIntervalVariableExponent = 0;
      else
         sizeIntervalVariableExponent = exponentNumerator / exponentDenominator;

      AssumeCondition(((c2 == c0) && (a2-a0 == sizeIntervalVariableExponent*(b2-b0)))
            || c2 > c0)
      if (c2 == c0)
         sizeIntervalVariableMantissa = 0;
      else
         sizeIntervalVariableMantissa = (a2-a0 - sizeIntervalVariableExponent*(b2-b0))/(c2 - c0);

      sizeIntervalConstant = a0 - sizeIntervalVariableExponent*b0 - sizeIntervalVariableMantissa*c0;
      // size = sizeIntervalConstant + sizeof(TypeImplementation) /* warning: alignof */
      //    + sizeIntervalVariableExponent*((USizeExponent+8*sizeCell-1)/(8*sizeCell))
      //    + sizeIntervalVariableMantissa*((USizeMantissa+8*sizeCell-1)/(8*sizeCell));
      fileInterval << "template <int USizeMantissa, int USizeExponent, typename TypeImplementation>\n"
                 << "struct TFloatIntervalSizeTraits {\n"
                 << "   static const int USize = " << sizeIntervalConstant
#ifdef FLOAT_CONCRETE
                     << " + sizeof(TypeImplementation) /* warning: alignof */"
                     << " + 2*(((USizeMantissa + 31) / 32) % 2 == 0 ? 8 : 4)"
//                   << "         - (alignof(TypeImplementation) < " << alignof(AlignType)
//                      << " ? sizeof(TypeImplementation) % " << alignof(AlignType) << " : 0)\n"
//                   << "         + (alignof(TypeImplementation) > " << alignof(AlignType)
//                      << " ? (alignof(TypeImplementation)-1\n"
//                      << "            - " << (sizeof(AlignType)-1)
//                   << " % alignof(TypeImplementation)) : 0)\n"
#endif
                     << "\n         + " << sizeIntervalVariableExponent << " * ((USizeExponent+"
                           << 8*sizeCell-1 << ") / (" << 8*sizeCell << "))\n"
                     << "         + " << sizeIntervalVariableMantissa << " * ((USizeMantissa+"
                           << 8*sizeCell-1 << ") / (" << 8*sizeCell << "));\n"
                 << "};\n\n";

      fileInterval << "}} // end of namespace NumericalDomains::DDoubleIntervalInterface\n";
   };

   {  std::ofstream fileExact(headerExactFile.c_str());
      fileExact << "namespace NumericalDomains { namespace DDoubleExactInterface {\n\n";
      fileExact << "static const int MergeBranchesSize = "
                 << sizeof(NumericalDomains::DDoubleExact::MergeBranches) << ";\n";

      int sizeExactConstant;
      int sizeExactVariableExponent, sizeExactVariableMantissa;
      
#if !defined(FLOAT_GENERIC_BASE_UNSIGNED) && !defined(FLOAT_GENERIC_BASE_LONG)
      static const int sizeCell = sizeof(Numerics::UnsignedBaseStoreTraits::BaseType);
#ifdef FLOAT_CONCRETE
      typedef Numerics::UnsignedBaseStoreTraits::BaseType AlignType;
#endif
#elif defined(FLOAT_GENERIC_BASE_LONG)
      static const int sizeCell = sizeof(Numerics::UnsignedLongBaseStoreTraits::BaseType);
#ifdef FLOAT_CONCRETE
      typedef Numerics::UnsignedLongBaseStoreTraits::BaseType AlignType;
#endif
#else // defined(FLOAT_GENERIC_BASE_UNSIGNED)
      static const int sizeCell = sizeof(Numerics::UnsignedBaseStoreTraits::BaseType);
#ifdef FLOAT_CONCRETE
      typedef Numerics::UnsignedBaseStoreTraits::BaseType AlignType;
#endif
#endif

      int a0 = sizeof(NumericalDomains::DDoubleExact::TInstrumentedFloat<NumericalDomains::DDoubleExact::BuiltFloat, float>); // 64 - 4 - 4
      int size0 = a0;
#ifdef FLOAT_CONCRETE
      a0 +=
         - sizeof(float)
         - (alignof(float) < alignof(AlignType) ? sizeof(float) % alignof(AlignType) : 0)
         - (alignof(float) > alignof(AlignType) ? (alignof(float) - sizeof(AlignType)%alignof(float))%alignof(float) : 0);
#endif
      int b0 = ((8+8*sizeCell-1)/(8*sizeCell));    // 1
      int c0 = ((23+8*sizeCell-1)/(8*sizeCell));   // 1
      int a1 = sizeof(NumericalDomains::DDoubleExact::TInstrumentedFloat<NumericalDomains::DDoubleExact::BuiltDouble, double>); // 64 - 8
      int size1 = a1;
#ifdef FLOAT_CONCRETE
      a1 +=
         - sizeof(double)
         - (alignof(double) < alignof(AlignType) ? sizeof(double) % alignof(AlignType) : 0)
         - (alignof(double) > alignof(AlignType) ? (alignof(double) - sizeof(AlignType)%alignof(double))%alignof(double) : 0);
#endif
      int b1 = ((11+8*sizeCell-1)/(8*sizeCell));   // 1
      int c1 = ((52+8*sizeCell-1)/(8*sizeCell));   // 1
      int a2 = sizeof(NumericalDomains::DDoubleExact::TInstrumentedFloat<NumericalDomains::DDoubleExact::BuiltLongDouble, long double>); // 80 - 16
      int size2 = a2;
#ifdef FLOAT_CONCRETE
      a2 +=
         - sizeof(long double)
         - (alignof(long double) < alignof(AlignType) ? sizeof(long double) % alignof(AlignType) : 0)
         - (alignof(long double) > alignof(AlignType) ? (alignof(long double) - sizeof(AlignType)%alignof(long double))%alignof(long double) : 0);
#endif
      int b2 = ((LDBL_EXPONENT_DIG+8*sizeCell-1)/(8*sizeCell));   // 1
      int c2 = ((LDBL_MANT_DIG-1+8*sizeCell-1)/(8*sizeCell));     // 1

      // a0 = sizeExactConstant + sizeExactVariableExponent*b0 + sizeExactVariableMantissa*c0;
      // a1 = sizeExactConstant + sizeExactVariableExponent*b1 + sizeExactVariableMantissa*c1;
      // a2 = sizeExactConstant + sizeExactVariableExponent*b2 + sizeExactVariableMantissa*c2;

      // a1-a0 = sizeExactVariableExponent*(b1-b0) + sizeExactVariableMantissa*(c1-c0);
      // a2-a0 = sizeExactVariableExponent*(b2-b0) + sizeExactVariableMantissa*(c2-c0);

      // (c1-c0)*(a2-a0) - (c2-c0)*(a1-a0) = sizeExactVariableExponent*((b2-b0)*(c1-c0) - (b1-b0)*(c2-c0));

      int exponentNumerator = (c1-c0)*(a2-a0) - (c2-c0)*(a1-a0),
          exponentDenominator = ((b2-b0)*(c1-c0) - (b1-b0)*(c2-c0));
      AssumeCondition((exponentDenominator == 0 && exponentNumerator == 0)
            || (exponentDenominator > 0 && exponentNumerator % exponentDenominator == 0))

      if (exponentDenominator == 0)
         sizeExactVariableExponent = 0;
      else
         sizeExactVariableExponent = exponentNumerator / exponentDenominator;

      AssumeCondition(((c2 == c0) && (a2-a0 == sizeExactVariableExponent*(b2-b0)))
            || c2 > c0)
      if (c2 == c0)
         sizeExactVariableMantissa = 0;
      else
         sizeExactVariableMantissa = (a2-a0 - sizeExactVariableExponent*(b2-b0))/(c2 - c0);

      sizeExactConstant = a0 - sizeExactVariableExponent*b0 - sizeExactVariableMantissa*c0;
      // size = sizeExactConstant + sizeof(TypeImplementation) /* warning: alignof */
      //    + sizeExactVariableExponent*((USizeExponent+8*sizeCell-1)/(8*sizeCell))
      //    + sizeExactVariableMantissa*((USizeMantissa+8*sizeCell-1)/(8*sizeCell));
      fileExact << "template <int USizeMantissa, int USizeExponent, typename TypeImplementation>\n"
                 << "struct TFloatExactSizeTraits {};\n\n";
      fileExact << "template <>\n"
                 << "struct TFloatExactSizeTraits<23, 8, float> {\n"
                 << "   static const int USize = " << size0 << ";\n"
                 << "   static const int SizeOfImplementation = " << sizeof(float) << ";\n"
                 << "   static const int AlignOfImplementation = " << alignof(float) << ";\n"
                 << "};\n\n";
      fileExact << "template <>\n"
                 << "struct TFloatExactSizeTraits<52, 11, double> {\n"
                 << "   static const int USize = " << size1 << ";\n"
                 << "   static const int SizeOfImplementation = " << sizeof(double) << ";\n"
                 << "   static const int AlignOfImplementation = " << alignof(double) << ";\n"
                 << "};\n\n";
      fileExact << "template <>\n"
                 << "struct TFloatExactSizeTraits<63, 15, long double> {\n"
                 << "   static const int USize = " << size2 << ";\n"
                 << "   static const int SizeOfImplementation = " << sizeof(long double) << ";\n"
                 << "   static const int AlignOfImplementation = " << alignof(long double) << ";\n"
                 << "};\n\n";

      fileExact << "// template <int USizeMantissa, int USizeExponent, typename TypeImplementation>\n"
                 << "// struct TFloatExactSizeTraits {\n"
                 << "//   static const int USize = " << sizeExactConstant
#ifdef FLOAT_CONCRETE
                     << " + sizeof(TypeImplementation) /* warning: alignof */\n"
//                   << " + (((USizeMantissa + 63) / 64) % 2 == 0 ? 16 : 8)\n"
                     << "//         - (alignof(TypeImplementation) < " << alignof(AlignType)
                        << " ? sizeof(TypeImplementation) % " << alignof(AlignType) << " : 0)\n"
                     << "//         - (alignof(TypeImplementation) > " << alignof(AlignType)
                        << " ? (alignof(TypeImplementation)\n"
                        << "//            - " << sizeof(AlignType)
                     << " % alignof(TypeImplementation)) : 0)"
#endif
                     << "\n//         + " << sizeExactVariableExponent << " * ((USizeExponent+"
                           << 8*sizeCell-1 << ") / (" << 8*sizeCell << "))\n"
                     << "//         + " << sizeExactVariableMantissa << " * ((USizeMantissa+"
                           << 8*sizeCell-1 << ") / (" << 8*sizeCell << "));\n"
                 << "// };\n\n";

      fileExact << "}} // end of namespace NumericalDomains::DDoubleExactInterface\n";
   };
   return 0;
}

