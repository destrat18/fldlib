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
// File      : FloatAffineInterface.cpp
// Description :
//   Implementation of a class of affine relations.
//

#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "fldlib_config.h"
#include "NumericalAnalysis/FloatAffineExecutionPath.h"
#include "NumericalAnalysis/FloatAffineExecutionPath.template"

#include "Pointer/Vector.template"

#include "FloatAffineInterface.h"

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

#include "FloatInstrumentation/FloatAffine.inch"
#include "FloatInstrumentation/FloatAffine.incc"

namespace NumericalDomains { namespace DAffineInterface {

#include "StandardClasses/UndefineNew.h"
PathExplorer::PathExplorer() {
   AssumeCondition(sizeof(DAffine::PathExplorer) <= UPathExplorerSize*sizeof(AlignType))
   new (content) DAffine::PathExplorer();
}

PathExplorer::PathExplorer(Mode mode) {
   AssumeCondition(sizeof(DAffine::PathExplorer) <= UPathExplorerSize*sizeof(AlignType))
   new (content) DAffine::PathExplorer((DAffine::BaseExecutionPath::Mode) mode);
}

PathExplorer::PathExplorer(const PathExplorer& source) {
   new (content) DAffine::PathExplorer(*reinterpret_cast<const DAffine::PathExplorer*>(source.content));
}

PathExplorer::PathExplorer(PathExplorer&& source) {
   new (content) DAffine::PathExplorer(std::move(*reinterpret_cast<DAffine::PathExplorer*>(source.content)));
}
#include "StandardClasses/DefineNew.h"

PathExplorer::~PathExplorer() {
   reinterpret_cast<DAffine::PathExplorer*>(content)->~PathExplorer();
}

PathExplorer&
PathExplorer::operator=(const PathExplorer& asource) {
   const DAffine::PathExplorer& source = *reinterpret_cast<const DAffine::PathExplorer*>(asource.content);
   DAffine::PathExplorer& thisExplorer = *reinterpret_cast<DAffine::PathExplorer*>(content);
   thisExplorer = source;
   return *this;
}

PathExplorer&
PathExplorer::operator=(PathExplorer&& asource) {
   DAffine::PathExplorer& source = *reinterpret_cast<DAffine::PathExplorer*>(asource.content);
   DAffine::PathExplorer& thisExplorer = *reinterpret_cast<DAffine::PathExplorer*>(content);
   thisExplorer.operator=(std::move(source));
   return *this;
}

PathExplorer::Mode
PathExplorer::mode() const {
   return (Mode) const_cast<DAffine::PathExplorer*>(
         reinterpret_cast<const DAffine::PathExplorer*>(content))->mode();
}

bool
PathExplorer::isFinished(Mode mode) {
   return reinterpret_cast<DAffine::PathExplorer*>(content)->isFinished((DAffine::BaseExecutionPath::Mode) mode);
}

bool
PathExplorer::isFinished() {
   return reinterpret_cast<DAffine::PathExplorer*>(content)->isFinished();
}

void
ExecutionPath::splitBranches(const char* file, int line) {
   DAffine::BaseFloatAffine::splitBranches(file, line);
}

std::pair<const char*, int>
ExecutionPath::querySplitInfo() {
   return DAffine::BaseFloatAffine::querySplitInfo();
}

bool
ExecutionPath::hasMultipleBranches() const {
   return DAffine::ExecutionPath::hasMultipleBranches();
}

void
ExecutionPath::setSupportAtomic() {
   DAffine::ExecutionPath::setSupportAtomic();
}

void
ExecutionPath::setSupportUnstableInLoop(bool value) {
   DAffine::ExecutionPath::setSupportUnstableInLoop(value);
}

void
ExecutionPath::setSupportBacktrace() {
   DAffine::ExecutionPath::setSupportBacktrace();
}

void
ExecutionPath::setSupportVerbose() {
   DAffine::ExecutionPath::setSupportVerbose();
}

void
ExecutionPath::setSupportThreshold() {
   DAffine::ExecutionPath::setSupportThreshold();
}

void
ExecutionPath::setSupportFirstFollowFloat() {
   DAffine::ExecutionPath::setSupportFirstFollowFloat();
}

void
ExecutionPath::setSupportPureZonotope() {
   DAffine::ExecutionPath::setSupportPureZonotope();
}

void
ExecutionPath::setTrackErrorOrigin() {
   DAffine::ExecutionPath::setTrackErrorOrigin();
}

void
ExecutionPath::setSupportMapSymbols() {
   DAffine::ExecutionPath::setSupportMapSymbols();
}

void
ExecutionPath::setLimitNoiseSymbolsNumber(int limit) {
   DAffine::ExecutionPath::setLimitNoiseSymbolsNumber(limit);
}

void
ExecutionPath::setSimplificationTriggerPercent(double percent) {
   DAffine::ExecutionPath::setSimplificationTriggerPercent(percent);
}

bool
ExecutionPath::doesSupportUnstableInLoop() {
   return DAffine::ExecutionPath::doesSupportUnstableInLoop();
}

void
ExecutionPath::initializeGlobals(const char* fileSuffix) {
   DAffine::ExecutionPath::initializeGlobals(fileSuffix);
}

void
ExecutionPath::finalizeGlobals() {
   DAffine::ExecutionPath::finalizeGlobals();
}

void
ExecutionPath::flushOut() {
   DAffine::ExecutionPath::flushOut();
}

void
ExecutionPath::setSourceLine(const char* file, int line) {
   DAffine::ExecutionPath::setSourceLine(file, line);
}

void
ExecutionPath::clearSynchronizationBranches() {
   DAffine::ExecutionPath::clearSynchronizationBranches();
}

void
ExecutionPath::writeCurrentPath(std::ostream& out) {
   DAffine::ExecutionPath::writeCurrentPath(out);
}

DAffine::PathExplorer*
ExecutionPath::getCurrentPathExplorer() {
   return DAffine::ExecutionPath::getCurrentPathExplorer();
}

void
ExecutionPath::setCurrentPathExplorer(PathExplorer* pathExplorer) {
   if (pathExplorer)
      DAffine::ExecutionPath::currentPathExplorer = reinterpret_cast<DAffine::PathExplorer*>(pathExplorer->content);
   else
      DAffine::ExecutionPath::currentPathExplorer = nullptr;
}

void
ExecutionPath::setCurrentPathExplorer(DAffine::PathExplorer* pathExplorer) {
   DAffine::ExecutionPath::currentPathExplorer = pathExplorer;
}

DAffine::DiagnosisReadStream*
ExecutionPath::inputTraceFile() {
   return reinterpret_cast<DAffine::DiagnosisReadStream*>(DAffine::ExecutionPath::inputTraceFile());
}

const char*
ExecutionPath::synchronisationFile() {
   return DAffine::ExecutionPath::synchronisationFile();
}

int
ExecutionPath::synchronisationLine() {
   return DAffine::ExecutionPath::synchronisationLine();
}

bool
ExecutionPath::doesFollowFlow() {
   return DAffine::ExecutionPath::doesFollowFlow();
}

void
ExecutionPath::clearFollowFlow() {
   return DAffine::ExecutionPath::clearFollowFlow();
}

void
ExecutionPath::setFollowFlow(bool doesFollowFlow, DAffine::DiagnosisReadStream* inputTraceFile,
      const char* synchronizationFile, int synchronizationLine) {
   DAffine::ExecutionPath::setFollowFlow(doesFollowFlow,
         reinterpret_cast<STG::IOObject::ISBase*>(inputTraceFile),
         synchronizationFile, synchronizationLine);
}

void
ExecutionPath::setFollowFlow() {
   DAffine::ExecutionPath::setFollowFlow();
}

PathExplorer::Mode
ExecutionPath::queryMode(DAffine::PathExplorer* pathExplorer) {
   AssumeCondition(pathExplorer)
   return (PathExplorer::Mode) pathExplorer->mode();
}

void
ExecutionPath::throwEmptyBranch(bool isUnstable) {
   try {
      DAffine::ExecutionPath::throwEmptyBranch(isUnstable);
   }
   catch (DAffine::ExecutionPath::anticipated_termination) {
      throw DAffineInterface::ExecutionPath::anticipated_termination();
   };
}

#include "StandardClasses/UndefineNew.h"
MergeBranches::MergeBranches(const char* file, int line) {
   AssumeCondition(sizeof(DAffine::MergeBranches) <= UMergeBranchesSize*sizeof(AlignType))
   new (content) DAffine::MergeBranches(file, line);
}
#include "StandardClasses/DefineNew.h"

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
MergeBranches&
MergeBranches::operator<<(const TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>& value) {
   reinterpret_cast<DAffine::MergeBranches*>(content)->operator<<(
         *reinterpret_cast<DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation>*>(const_cast<TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>&>(value).content));
   return *this;
}

bool
MergeBranches::operator<<(end) {
   try {
   return reinterpret_cast<DAffine::MergeBranches*>(content)->operator<<(
         DAffine::BaseFloatAffine::end());
   }
   catch (DAffine::ExecutionPath::anticipated_termination) {
      throw DAffineInterface::ExecutionPath::anticipated_termination();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
const char*
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::queryDebugValue() const {
   return reinterpret_cast<const DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation>*>(content)
      ->queryDebugValue();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
const char*
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::queryLightDebugValue() const {
   return reinterpret_cast<const DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation>*>(content)
      ->queryLightDebugValue();
}

#include "StandardClasses/UndefineNew.h"
template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::TFloatZonotope() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   AssumeCondition(sizeof(Implementation) <= UFloatZonotopeSize*sizeof(AlignType))
   new (content) Implementation();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::TFloatZonotope(const char* value, ValueFromString) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   AssumeCondition(sizeof(Implementation) <= UFloatZonotopeSize*sizeof(AlignType))
   new (content) Implementation();
   STG::IOObject::ISBase* in = DAffine::ExecutionPath::acquireConstantStream(value);
   reinterpret_cast<Implementation*>(content)->initFrom(*in);
   DAffine::ExecutionPath::releaseConstantStream(in);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::initFrom(TypeImplementation value) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   if (!DAffine::ExecutionPath::doesSupportAtomic() && DAffine::ExecutionPath::outputTraceFile())
      reinterpret_cast<Implementation*>(content)->initFrom(value);
   else
      reinterpret_cast<Implementation*>(content)->initFromAtomic(value);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::initFrom(
      TypeImplementation min, TypeImplementation max) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   *reinterpret_cast<Implementation*>(content) = Implementation(min, max);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop())
      reinterpret_cast<Implementation*>(content)->getSRealDomain().setHolder(reinterpret_cast<Implementation*>(content)->currentPathExplorer);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::initFrom(
      TypeImplementation min, TypeImplementation max,
      TypeImplementation errmin, TypeImplementation errmax) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   *reinterpret_cast<Implementation*>(content) = Implementation(min, max);
   reinterpret_cast<Implementation*>(content)->setError(errmin, errmax);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      reinterpret_cast<Implementation*>(content)->getSRealDomain().setHolder(reinterpret_cast<Implementation*>(content)->currentPathExplorer);
      reinterpret_cast<Implementation*>(content)->getSError().setHolder(reinterpret_cast<Implementation*>(content)->currentPathExplorer);
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::initFrom(int64_t value) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   *reinterpret_cast<Implementation*>(content) = Implementation(value);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::initFrom(uint64_t value) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   *reinterpret_cast<Implementation*>(content) = Implementation(value);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::TFloatZonotope(const thisType& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& source = *reinterpret_cast<const Implementation*>(asource.content);
   new (content) Implementation(source);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      reinterpret_cast<Implementation*>(content)->getSRealDomain().setHolder(reinterpret_cast<Implementation*>(content)->currentPathExplorer);
      reinterpret_cast<Implementation*>(content)->getSError().setHolder(reinterpret_cast<Implementation*>(content)->currentPathExplorer);
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::setHolder(thisType& save) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      reinterpret_cast<Implementation*>(content)->getSRealDomain().setHolder(reinterpret_cast<Implementation*>(content)->currentPathExplorer);
      reinterpret_cast<Implementation*>(content)->getSError().setHolder(reinterpret_cast<Implementation*>(content)->currentPathExplorer);
      reinterpret_cast<Implementation*>(save.content)->getSRealDomain().clearHolder();
      reinterpret_cast<Implementation*>(save.content)->getSError().clearHolder();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::removeHolder() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      reinterpret_cast<Implementation*>(content)->getSRealDomain().clearHolder();
      reinterpret_cast<Implementation*>(content)->getSError().clearHolder();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::TFloatZonotope(thisType&& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   Implementation& source = *reinterpret_cast<Implementation*>(asource.content);
   new (content) Implementation(std::move(source));
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      reinterpret_cast<Implementation*>(content)->getSRealDomain().setHolder(reinterpret_cast<Implementation*>(content)->currentPathExplorer);
      reinterpret_cast<Implementation*>(content)->getSError().setHolder(reinterpret_cast<Implementation*>(content)->currentPathExplorer);
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::TFloatZonotope(
      const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument> ImplementationSource;
   const ImplementationSource& source = *reinterpret_cast<const ImplementationSource*>(asource.content);
   new (content) Implementation(source);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      reinterpret_cast<Implementation*>(content)->getSRealDomain().setHolder(reinterpret_cast<Implementation*>(content)->currentPathExplorer);
      reinterpret_cast<Implementation*>(content)->getSError().setHolder(reinterpret_cast<Implementation*>(content)->currentPathExplorer);
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::TFloatZonotope(
      TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument> ImplementationSource;
   ImplementationSource& source = *reinterpret_cast<ImplementationSource*>(asource.content);
   new (content) Implementation(std::move(source));
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      reinterpret_cast<Implementation*>(content)->getSRealDomain().setHolder(reinterpret_cast<Implementation*>(content)->currentPathExplorer);
      reinterpret_cast<Implementation*>(content)->getSError().setHolder(reinterpret_cast<Implementation*>(content)->currentPathExplorer);
   };
}
#include "StandardClasses/DefineNew.h"

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::~TFloatZonotope() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->~Implementation();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>&
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::operator=(const thisType& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& source = *reinterpret_cast<const Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   thisZonotope.operator=(source);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().setHolder(thisZonotope.currentPathExplorer);
      thisZonotope.getSError().setHolder(thisZonotope.currentPathExplorer);
   };
   return *this;
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::recordFrom(const thisType& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& source = *reinterpret_cast<const Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   // if (!const_cast<Implementation&>(source).optimizeValue()) throw anticipated_termination();
   thisZonotope.operator=(source);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().clearHolder();
      thisZonotope.getSError().clearHolder();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::mergeWith(const thisType& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& source = *reinterpret_cast<const Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   // if (!const_cast<Implementation&>(source).optimizeValue()) throw anticipated_termination();
   thisZonotope.mergeWith(source);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>&
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::operator=(thisType&& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   Implementation& source = *reinterpret_cast<Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   thisZonotope.operator=(std::move(source));
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().setHolder(thisZonotope.currentPathExplorer);
      thisZonotope.getSError().setHolder(thisZonotope.currentPathExplorer);
   };
   return *this;
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::recordFrom(thisType&& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   Implementation& source = *reinterpret_cast<Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   if (!source.optimizeValue()) throw anticipated_termination();
   thisZonotope.operator=(std::move(source));
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().clearHolder();
      thisZonotope.getSError().clearHolder();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::mergeWith(thisType&& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   Implementation& source = *reinterpret_cast<Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   if (!source.optimizeValue()) throw anticipated_termination();
   thisZonotope.mergeWith(std::move(source));
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>&
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::operator=(
      const TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument> ImplementationSource;
   const ImplementationSource& source = *reinterpret_cast<const ImplementationSource*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   thisZonotope.operator=(source);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().setHolder(thisZonotope.currentPathExplorer);
      thisZonotope.getSError().setHolder(thisZonotope.currentPathExplorer);
   };
   return *this;
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
template <int USizeMantissaArgument, int USizeExponentArgument, typename TypeImplementationArgument>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>&
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::operator=(
      TFloatZonotope<USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument>&& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissaArgument, USizeExponentArgument, TypeImplementationArgument> ImplementationSource;
   ImplementationSource& source = *reinterpret_cast<ImplementationSource*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   thisZonotope.operator=(std::move(source));
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().setHolder(thisZonotope.currentPathExplorer);
      thisZonotope.getSError().setHolder(thisZonotope.currentPathExplorer);
   };
   return *this;
}


template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::writeImplementation(
      std::ostream& out) const {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   // const auto& implementation = *reinterpret_cast<const Implementation*>(content);
   // out << '[';
   // typename Implementation::BuiltDouble::WriteParameters writeParameters;
   // implementation.getMinImplementation().write(out, writeParameters);
   // out << ", ";
   // implementation.getMaxImplementation().write(out, writeParameters);
   // out << ']';
#ifdef FLOAT_CONCRETE
   out << reinterpret_cast<const Implementation*>(content)->asImplementation();
#else
   TypeImplementation min=0, max=0;
   reinterpret_cast<const Implementation*>(content)->retrieveImplementationBounds(min, max);
   out << (min+max)/2;
#endif
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::readImplementation(
      std::istream& in) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
// auto& implementation = *reinterpret_cast<Implementation*>(content);
// int ch = in.get();
// while (ch >= 0 && std::isspace(ch))
//    ch = in.get();
// AssumeCondition(ch == '[');
// typename Implementation::BuiltDouble::ReadParameters readParameters;
// STG::
// implementation.getSMinImplementation().read(in, readParameters);
// ch = in.get();
// while (ch >= 0 && std::isspace(ch))
//    ch = in.get();
// AssumeCondition(ch == ',');
// ch = in.get();
// while (ch >= 0 && std::isspace(ch))
//    ch = in.get();
// in.putback(ch);
// implementation.getSMaxImplementation().read(in, readParameters);
   TypeImplementation result; 
   in >> result;
   *reinterpret_cast<Implementation*>(content) = Implementation(result, result);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
bool
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::optimizeValue() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   return thisZonotope.optimizeValue();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
bool
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::compareLess(const thisType& afirst, const thisType& asecond) {
   try {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& first = *reinterpret_cast<const Implementation*>(afirst.content);
   const Implementation& second = *reinterpret_cast<const Implementation*>(asecond.content);
   return first.operator<(second);
   }
   catch (DAffine::ExecutionPath::anticipated_termination) {
      throw DAffineInterface::ExecutionPath::anticipated_termination();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
bool
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::compareLessOrEqual(const thisType& afirst, const thisType& asecond) {
   try {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& first = *reinterpret_cast<const Implementation*>(afirst.content);
   const Implementation& second = *reinterpret_cast<const Implementation*>(asecond.content);
   return first.operator<=(second);
   }
   catch (DAffine::ExecutionPath::anticipated_termination) {
      throw DAffineInterface::ExecutionPath::anticipated_termination();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
bool
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::compareEqual(const thisType& afirst, const thisType& asecond) {
   try {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& first = *reinterpret_cast<const Implementation*>(afirst.content);
   const Implementation& second = *reinterpret_cast<const Implementation*>(asecond.content);
   return first.operator==(second);
   }
   catch (DAffine::ExecutionPath::anticipated_termination) {
      throw DAffineInterface::ExecutionPath::anticipated_termination();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
bool
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::compareDifferent(const thisType& afirst, const thisType& asecond) {
   try {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& first = *reinterpret_cast<const Implementation*>(afirst.content);
   const Implementation& second = *reinterpret_cast<const Implementation*>(asecond.content);
   return first.operator!=(second);
   }
   catch (DAffine::ExecutionPath::anticipated_termination) {
      throw DAffineInterface::ExecutionPath::anticipated_termination();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
bool
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::compareGreaterOrEqual(const thisType& afirst, const thisType& asecond) {
   try {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& first = *reinterpret_cast<const Implementation*>(afirst.content);
   const Implementation& second = *reinterpret_cast<const Implementation*>(asecond.content);
   return first.operator>=(second);
   }
   catch (DAffine::ExecutionPath::anticipated_termination) {
      throw DAffineInterface::ExecutionPath::anticipated_termination();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
bool
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::compareGreater(const thisType& afirst, const thisType& asecond) {
   try {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& first = *reinterpret_cast<const Implementation*>(afirst.content);
   const Implementation& second = *reinterpret_cast<const Implementation*>(asecond.content);
   return first.operator>(second);
   }
   catch (DAffine::ExecutionPath::anticipated_termination) {
      throw DAffineInterface::ExecutionPath::anticipated_termination();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>&
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::operator+=(const thisType& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& source = *reinterpret_cast<const Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   thisZonotope.plusAssign(source, DAffine::Equation::PCSourceRValue);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().setHolder(thisZonotope.currentPathExplorer);
      thisZonotope.getSError().setHolder(thisZonotope.currentPathExplorer);
   };
   return *this;
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>&
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::operator+=(thisType&& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   Implementation& source = *reinterpret_cast<Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   thisZonotope.plusAssign(source, DAffine::Equation::PCSourceXValue);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().setHolder(thisZonotope.currentPathExplorer);
      thisZonotope.getSError().setHolder(thisZonotope.currentPathExplorer);
   };
   return *this;
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>&
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::operator-=(const thisType& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& source = *reinterpret_cast<const Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   thisZonotope.minusAssign(source, DAffine::Equation::PCSourceRValue);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().setHolder(thisZonotope.currentPathExplorer);
      thisZonotope.getSError().setHolder(thisZonotope.currentPathExplorer);
   };
   return *this;
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>&
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::operator-=(thisType&& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   Implementation& source = *reinterpret_cast<Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   thisZonotope.minusAssign(source, DAffine::Equation::PCSourceXValue);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().setHolder(thisZonotope.currentPathExplorer);
      thisZonotope.getSError().setHolder(thisZonotope.currentPathExplorer);
   };
   return *this;
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>&
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::operator*=(const thisType& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& source = *reinterpret_cast<const Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   thisZonotope.multAssign(source, DAffine::Equation::PCSourceRValue);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().setHolder(thisZonotope.currentPathExplorer);
      thisZonotope.getSError().setHolder(thisZonotope.currentPathExplorer);
   };
   return *this;
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>&
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::operator*=(thisType&& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   Implementation& source = *reinterpret_cast<Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   thisZonotope.multAssign(source, DAffine::Equation::PCSourceXValue);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().setHolder(thisZonotope.currentPathExplorer);
      thisZonotope.getSError().setHolder(thisZonotope.currentPathExplorer);
   };
   return *this;
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>&
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::operator/=(const thisType& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& source = *reinterpret_cast<const Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   thisZonotope.divAssign(source, DAffine::Equation::PCSourceRValue);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().setHolder(thisZonotope.currentPathExplorer);
      thisZonotope.getSError().setHolder(thisZonotope.currentPathExplorer);
   };
   return *this;
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>&
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::operator/=(thisType&& asource) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   Implementation& source = *reinterpret_cast<Implementation*>(asource.content);
   Implementation& thisZonotope = *reinterpret_cast<Implementation*>(content);
   thisZonotope.divAssign(source, DAffine::Equation::PCSourceXValue);
   if (DAffine::ExecutionPath::doesSupportUnstableInLoop()) {
      thisZonotope.getSRealDomain().setHolder(thisZonotope.currentPathExplorer);
      thisZonotope.getSError().setHolder(thisZonotope.currentPathExplorer);
   };
   return *this;
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::oppositeAssign() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->oppositeAssign();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::plusAssign(const thisType& source) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->plusAssign(
         *reinterpret_cast<const Implementation*>(source.content), DAffine::Equation::PCSourceRValue);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::plusAssign(thisType&& source) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->plusAssign(
         *reinterpret_cast<Implementation*>(source.content), DAffine::Equation::PCSourceXValue);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::minusAssign(const thisType& source) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->minusAssign(
         *reinterpret_cast<const Implementation*>(source.content), DAffine::Equation::PCSourceRValue);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::minusAssign(thisType&& source) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->minusAssign(
         *reinterpret_cast<Implementation*>(source.content), DAffine::Equation::PCSourceXValue);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::multAssign(const thisType& source) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->multAssign(
         *reinterpret_cast<const Implementation*>(source.content), DAffine::Equation::PCSourceRValue);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::multAssign(thisType&& source) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->multAssign(
         *reinterpret_cast<Implementation*>(source.content), DAffine::Equation::PCSourceXValue);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::divAssign(const thisType& source) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->divAssign(
         *reinterpret_cast<const Implementation*>(source.content), DAffine::Equation::PCSourceRValue);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::divAssign(thisType&& source) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->divAssign(
         *reinterpret_cast<Implementation*>(source.content), DAffine::Equation::PCSourceXValue);
}

#ifdef FLOAT_CONCRETE
template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TypeImplementation
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::asImplementation() const {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   return reinterpret_cast<const Implementation*>(content)->asImplementation();
}
#endif

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::operator TypeImplementation() const {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
#ifdef FLOAT_CONCRETE
   return reinterpret_cast<const Implementation*>(content)->asImplementation();
#else
   TypeImplementation min = 0, max = 0;
   reinterpret_cast<const Implementation*>(content)->retrieveImplementationBounds(min, max);
   return (min+max)/2;
#endif
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
int32_t
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::asInt(RoundMode mode) const {
   try {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   typedef Numerics::DDouble::Access::ReadParameters ReadParametersBase;
   return reinterpret_cast<const Implementation*>(content)->asInt((ReadParametersBase::RoundMode) mode);
   }
   catch (DAffine::ExecutionPath::anticipated_termination) {
      throw DAffineInterface::ExecutionPath::anticipated_termination();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
uint32_t
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::asUnsigned(RoundMode mode) const {
   try {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   typedef Numerics::DDouble::Access::ReadParameters ReadParametersBase;
   return reinterpret_cast<const Implementation*>(content)->asUnsigned((ReadParametersBase::RoundMode) mode);
   }
   catch (DAffine::ExecutionPath::anticipated_termination) {
      throw DAffineInterface::ExecutionPath::anticipated_termination();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
int64_t
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::asLongInt(RoundMode mode) const {
   try {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   typedef Numerics::DDouble::Access::ReadParameters ReadParametersBase;
   return reinterpret_cast<const Implementation*>(content)->asLongInt((ReadParametersBase::RoundMode) mode);
   }
   catch (DAffine::ExecutionPath::anticipated_termination) {
      throw DAffineInterface::ExecutionPath::anticipated_termination();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
uint64_t
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::asUnsignedLong(RoundMode mode) const {
   try {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   typedef Numerics::DDouble::Access::ReadParameters ReadParametersBase;
   return reinterpret_cast<const Implementation*>(content)->asUnsignedLong((ReadParametersBase::RoundMode) mode);
   }
   catch (DAffine::ExecutionPath::anticipated_termination) {
      throw DAffineInterface::ExecutionPath::anticipated_termination();
   };
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::lightPersist(const char* prefix) const {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& thisZonotope = *reinterpret_cast<const Implementation*>(content);
   thisZonotope.lightPersist(thisZonotope, prefix);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::persist(const char* prefix) const {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const Implementation& thisZonotope = *reinterpret_cast<const Implementation*>(content);
   thisZonotope.persist(thisZonotope, prefix);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::inverseAssign() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->inverseAssign();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::sqrtAssign() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->sqrtAssign();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::sinAssign() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->sinAssign();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::cosAssign() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->cosAssign();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::asinAssign() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->asinAssign();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::acosAssign() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->acosAssign();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::tanAssign() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->tanAssign();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::atanAssign() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->atanAssign();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::expAssign() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->expAssign();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::logAssign() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->logAssign();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::log10Assign() {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->log10Assign();
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::powAssign(const thisType& value) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->powAssign(
         *reinterpret_cast<const Implementation*>(value.content), DAffine::Equation::PCSourceRValue);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::powAssign(thisType&& value) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->powAssign(
         *reinterpret_cast<const Implementation*>(value.content), DAffine::Equation::PCSourceXValue);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::atan2Assign(const thisType& value) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->atan2Assign(
         *reinterpret_cast<const Implementation*>(value.content), DAffine::Equation::PCSourceRValue);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
void
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::atan2Assign(thisType&& value) {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   reinterpret_cast<Implementation*>(content)->atan2Assign(
         *reinterpret_cast<const Implementation*>(value.content), DAffine::Equation::PCSourceXValue);
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
int
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::sfinite() const {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   return !(reinterpret_cast<const Implementation*>(content)->getMinImplementation()
         .isInftyExponent());
   // return finite(reinterpret_cast<const Implementation*>(content)->asImplementation());
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
int
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::sisfinite() const {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   return !(reinterpret_cast<const Implementation*>(content)->getMinImplementation()
         .isInftyExponent());
   // return std::isfinite(reinterpret_cast<const Implementation*>(content)->asImplementation());
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
int
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::sisnan() const {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   return reinterpret_cast<const Implementation*>(content)->getMinImplementation()
         .isNaN();
   // return std::isnan(reinterpret_cast<const Implementation*>(content)->asImplementation());
}

template <int USizeMantissa, int USizeExponent, typename TypeImplementation>
int
TFloatZonotope<USizeMantissa, USizeExponent, TypeImplementation>::sisinf() const {
   typedef DAffine::TFloatZonotope<DAffine::ExecutionPath, USizeMantissa, USizeExponent, TypeImplementation> Implementation;
   const auto& implementation = *reinterpret_cast<const Implementation*>(content);
   return implementation.getMinImplementation().isInftyExponent()
         && !implementation.getMinImplementation().isNaN();
   // return std::isinf(reinterpret_cast<const Implementation*>(content)->asImplementation());
}

typedef DAffine::FloatDigitsHelper::TFloatDigits<long double> LongDoubleFloatDigits;

template class TFloatZonotope<23, 8, float>;
template class TFloatZonotope<52, 11, double>;
template class TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>;

template TFloatZonotope<23, 8, float>::TFloatZonotope(TFloatZonotope<52, 11, double>&&);
template TFloatZonotope<23, 8, float>::TFloatZonotope(const TFloatZonotope<52, 11, double>&);
template TFloatZonotope<23, 8, float>::TFloatZonotope(TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>&&);
template TFloatZonotope<23, 8, float>::TFloatZonotope(const TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>&);
template TFloatZonotope<52, 11, double>::TFloatZonotope(TFloatZonotope<23, 8, float>&&);
template TFloatZonotope<52, 11, double>::TFloatZonotope(const TFloatZonotope<23, 8, float>&);
template TFloatZonotope<52, 11, double>::TFloatZonotope(TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>&&);
template TFloatZonotope<52, 11, double>::TFloatZonotope(const TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>&);
template TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>::TFloatZonotope(TFloatZonotope<23, 8, float>&&);
template TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>::TFloatZonotope(const TFloatZonotope<23, 8, float>&);
template TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>::TFloatZonotope(TFloatZonotope<52, 11, double>&&);
template TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>::TFloatZonotope(const TFloatZonotope<52, 11, double>&);

template TFloatZonotope<23, 8, float>& TFloatZonotope<23, 8, float>::operator=(const TFloatZonotope<52, 11, double>&);
template TFloatZonotope<23, 8, float>& TFloatZonotope<23, 8, float>::operator=(const TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>&);
template TFloatZonotope<52, 11, double>& TFloatZonotope<52, 11, double>::operator=(const TFloatZonotope<23, 8, float>&);
template TFloatZonotope<52, 11, double>& TFloatZonotope<52, 11, double>::operator=(const TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>&);
template TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>& TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>::operator=(const TFloatZonotope<23, 8, float>&);
template TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>& TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>::operator=(const TFloatZonotope<52, 11, double>&);

template TFloatZonotope<23, 8, float>& TFloatZonotope<23, 8, float>::operator=(TFloatZonotope<52, 11, double>&&);
template TFloatZonotope<23, 8, float>& TFloatZonotope<23, 8, float>::operator=(TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>&&);
template TFloatZonotope<52, 11, double>& TFloatZonotope<52, 11, double>::operator=(TFloatZonotope<23, 8, float>&&);
template TFloatZonotope<52, 11, double>& TFloatZonotope<52, 11, double>::operator=(TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>&&);
template TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>& TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>::operator=(TFloatZonotope<23, 8, float>&&);
template TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>& TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>::operator=(TFloatZonotope<52, 11, double>&&);

template MergeBranches& MergeBranches::operator<<(const TFloatZonotope<23, 8, float>&);
template MergeBranches& MergeBranches::operator<<(const TFloatZonotope<52, 11, double>&);
template MergeBranches& MergeBranches::operator<<(const TFloatZonotope<LongDoubleFloatDigits::UBitSizeMantissa, LongDoubleFloatDigits::UBitSizeExponent, long double>&);

}} // end of namespace NumericalDomains::DAffineInterface

