//----------------------------------------------------------------------------------
//! The ML module class XMarkerListToFile.
/*!
// \file    mlXMarkerListToFile.h
// \author  Coert Metz, Erwin Vast
// \date    2007-07-06, 2015-11-20
//
// Save XMarkerList to a text file
*/
//----------------------------------------------------------------------------------

/* =================================================================================
   Copyright (c) 2010, Biomedical Imaging Group Rotterdam (BIGR),
   Departments of Radiology and Medical Informatics, Erasmus MC. All
   rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of BIGR nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL BIGR BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  =================================================================================*/

#ifndef __mlXMarkerListToFile_H
#define __mlXMarkerListToFile_H


// Local includes
#ifndef __MLXMarkerListCommunityModulesSystem_H
#include "MLXMarkerListCommunityModulesSystem.h"
#endif

// ML includes
#include "mlOperatorIncludes.h"
#include "mlXMarkerList.h"

ML_START_NAMESPACE

//! Read XMarkers from a text file
class MLXMARKERLISTCOMMUNITYMODULES_EXPORT XMarkerListToFile : public BaseOp
{
public:

  //! Constructor.
  XMarkerListToFile (void);

  //! Handle field changes of the field \c field.
  virtual void handleNotification (Field *field);

private:

  // ----------------------------------------------------------
  //@{ \name Module field declarations
  // ----------------------------------------------------------

  //! Filename of file to save markers to
  StringField *_filenameFld;

  //! Coordinate system of output markers (world or voxel)
  //! When voxel is selected, the module converts the markers to
  //! voxel coordinates before writing them to file
  //! The last option is pos:voxel, vec:world, which only convert
  //! the marker position to voxel coordinates before exporting the marker.
  EnumField *_outputCoordinateSystemFld;

  //! Field to chose between space or newline as coordinate separator
  EnumField *_coordinateSeparatorFld;

  //! Field for input XMarkerList
  BaseField *_inputXMarkerListFld;

  //! Fields to select elements to export
  BoolField *_positionXFld;
  BoolField *_positionYFld;
  BoolField *_positionZFld;
  BoolField *_positionSFld;
  BoolField *_positionTFld;
  BoolField *_positionUFld;
  BoolField *_vectorXFld;
  BoolField *_vectorYFld;
  BoolField *_vectorZFld;
  BoolField *_typeFld;
  BoolField *_nameFld;

  //! Int field to select writing precision
  IntField *_precisionFld;

  //! Bool field for transformix format (first line: number of points, second line: point or index)
  BoolField *_transformixFormatFld;

  //! Enable/disable output of only one marker per voxel coordinate
  BoolField *_maxOneMarkerPerVoxelFld;

  //! Save button
  NotifyField *_saveFld;

  //@}

  //! Implements interface for the runtime type system of the ML.
  ML_BASEOP_CLASS_HEADER(XMarkerListToFile)
};

ML_END_NAMESPACE

#endif // __mlXMarkerListToFile_H
