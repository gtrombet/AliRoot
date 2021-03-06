//-*- Mode: C++ -*-
// $Id$
#ifndef ALIHLTTPCHWCFPROCESSORUNIT_H
#define ALIHLTTPCHWCFPROCESSORUNIT_H

//* This file is property of and copyright by the ALICE HLT Project        * 
//* ALICE Experiment at CERN, All rights reserved.                         *
//* See cxx source for full Copyright notice                               *


#include "AliHLTDataTypes.h"
#include "AliHLTTPCHWCFDataTypes.h"


//  @class AliHLTTPCHWCFProcessorUnit
//  @author Sergey Gorbunov <sergey.gorbunov@fias.uni-frankfurt.de>
//  @author Torsten Alt <talt@cern.ch> 
//  @brief  Channel Processor unit of FPGA ClusterFinder Emulator for TPC
//  @brief  ( see AliHLTTPCHWCFEmulator class )
//  @note
//
class AliHLTTPCHWCFProcessorUnit
{
 public:

  /** standard constructor */
  AliHLTTPCHWCFProcessorUnit();
  
  /** destructor */
  ~AliHLTTPCHWCFProcessorUnit();

  /** set debug level */
  void SetDebugLevel( int val ){ fDebug = val; }

  /** do cluster deconvolution in time direction */
  void SetDeconvolutionTime( bool val ){ fDeconvolute = val; }
  void SetImprovedDeconvolution( bool val) { fImprovedDeconvolution = val; }

  /** lower charge limit for isolated signals
   */
  void SetSingleSeqLimit( AliHLTUInt32_t val ){ 
    fSingleSeqLimit = val << AliHLTTPCHWCFDefinitions::kFixedPoint; 
  }

  /** limit size of the cluster in time bins to 5
   */
  void SetUseTimeBinWindow( bool val ){ 
    fUseTimeBinWindow = val;
  }

  /** initialise */
  int Init();
  
  /** input stream of data */
  int InputStream( const AliHLTTPCHWCFBunch *bunch );

  /** output stream of data */
  const AliHLTTPCHWCFClusterFragment *OutputStream();
  
 private: 

  /** copy constructor prohibited */
  AliHLTTPCHWCFProcessorUnit(const AliHLTTPCHWCFProcessorUnit&);
  /** assignment operator prohibited */
  AliHLTTPCHWCFProcessorUnit& operator=(const AliHLTTPCHWCFProcessorUnit&);  
  

  AliHLTTPCHWCFClusterFragment fOutput; // current output
  const AliHLTTPCHWCFBunch *fkBunch; // current input
  AliHLTUInt32_t fBunchIndex; // index in bunch
  bool fWasDeconvoluted; // tag shows if the current bunch has been split in several clusters
  bool fDeconvolute;    // do deconvolution in time direction
  bool fImprovedDeconvolution;    // Improved deconvolution taking into account the minimum flag
  AliHLTUInt64_t fSingleSeqLimit; // lower charge limit for isolated signals
  bool fUseTimeBinWindow; // set max. size of the cluster to 5 time bins   
  int fDebug; // debug level
};

#endif
