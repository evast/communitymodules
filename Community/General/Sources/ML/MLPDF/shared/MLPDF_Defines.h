//----------------------------------------------------------------------------------
//! Defines for PDF file creation.
/*!
// \file    MLPDF_Defines.h
// \author  Axel Newe
// \date    2015-10-16
//
//
*/
//----------------------------------------------------------------------------------


#ifndef _PDF_Defines_H
#define _PDF_Defines_H 

#include "mlModuleIncludes.h"

// Thirdparty includes
#include "hpdf.h"


ML_START_NAMESPACE

namespace mlPDF 
{
  typedef HPDF_Page       PAGE;
  typedef HPDF_Image      IMAGE;
  typedef HPDF_U3D        MODEL3D;
  typedef HPDF_Dict       VIEW3D;
  typedef HPDF_Annotation SCENE3D;

  static bool IGNORE_MARGINS = true;

  static std::string LOREM_IPSUM = "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Maecenas porttitor congue massa. Fusce posuere, magna sed pulvinar ultricies, purus lectus malesuada libero, sit amet commodo magna eros quis urna.\nNunc viverra imperdiet enim. Fusce est. Vivamus a tellus.\nPellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Proin pharetra nonummy pede. Mauris et orci.\n";

  enum PAGESIZES {
    PAGESIZE_LETTER       = HPDF_PAGE_SIZE_LETTER,       // 612     x 792       pixels
    PAGESIZE_LEGAL        = HPDF_PAGE_SIZE_LEGAL,        // 612     x 1008      pixels
    PAGESIZE_A3           = HPDF_PAGE_SIZE_A3,           // 841.89  x 1199.551  pixels
    PAGESIZE_A4           = HPDF_PAGE_SIZE_A4,           // 595.276 x 841.89    pixels
    PAGESIZE_A5           = HPDF_PAGE_SIZE_A5,           // 419.528 x 595.276   pixels
    PAGESIZE_B4           = HPDF_PAGE_SIZE_B4,           // 708.661 x 1000.63   pixels
    PAGESIZE_B5           = HPDF_PAGE_SIZE_B5,           // 498.898 x 708.661   pixels
    PAGESIZE_EXECUTIVE    = HPDF_PAGE_SIZE_EXECUTIVE,    // 522     x 756       pixels
    PAGESIZE_US4x6        = HPDF_PAGE_SIZE_US4x6,        // 288     x 432       pixels
    PAGESIZE_US4x8        = HPDF_PAGE_SIZE_US4x8,        // 288     x 576       pixels
    PAGESIZE_US5x7        = HPDF_PAGE_SIZE_US5x7,        // 360     x 504       pixels
    PAGESIZE_COMM10       = HPDF_PAGE_SIZE_COMM10,       // 297     x 684       pixels

    NUM_PAGESIZES         = 12
  };

  const char* const PageSizeStrings[NUM_PAGESIZES] = {
    "PAGESIZE_A4",
    "PAGESIZE_LETTER",
    "PAGESIZE_LEGAL",
    "PAGESIZE_A3",
    "PAGESIZE_A5",
    "PAGESIZE_B4",
    "PAGESIZE_B5",
    "PAGESIZE_EXECUTIVE",
    "PAGESIZE_US4x6",
    "PAGESIZE_US4x8",
    "PAGESIZE_US5x7",
    "PAGESIZE_COMM10"
  };

  struct PageSizeInfoStruct
  {
    _HPDF_PageSizes PageSize;
    HPDF_REAL       Width;
    HPDF_REAL       Height;
    HPDF_REAL       MarginX;
    HPDF_REAL       MarginY;
  };

  enum PAGEDIRECTIONS {
    PAGEDIRECTION_PORTRAIT  = HPDF_PAGE_PORTRAIT,
    PAGEDIRECTION_LANDSCAPE = HPDF_PAGE_LANDSCAPE,

    NUM_PAGEDIRECTIONS      = 3
  };

  const char* const PageDirectionStrings[NUM_PAGEDIRECTIONS] = {
    "PAGEDIRECTION_PORTRAIT",
    "PAGEDIRECTION_LANDSCAPE"
  };

  struct BuiltInFontsStruct
  {
    HPDF_Font Times;
    HPDF_Font TimesBold;
    HPDF_Font TimesItalic;
    HPDF_Font TimesBoldItalic;
    HPDF_Font Courier;
    HPDF_Font CourierBold;
    HPDF_Font CourierOblique;
    HPDF_Font CourierBoldOblique;
    HPDF_Font Helvetica;
    HPDF_Font HelveticaBold;
    HPDF_Font HelveticaOblique;
    HPDF_Font HelveticaBoldOblique;
    HPDF_Font Symbol;
    HPDF_Font ZapfDingbats;
  };


  enum ACTIVATION_MODES {
    ACTIVATION_MODE_EXPLICIT_ACTIVATE = 0,
    ACTIVATION_MODE_PAGE_OPEN         = 1,
    ACTIVATION_MODE_PAGE_VISIBLE      = 2,
    NUM_ACTIVATIONMODES               = 3
  };

  const char* const ACTIVATION_MODE_STRINGS[NUM_ACTIVATIONMODES] = {
    "ExplicitActivate",
    "PageOpen",
    "PageVisible"
  };

  enum DEACTIVATION_MODES {
    DEACTIVATION_MODE_EXPLICIT_DEACTIVATE = 0,
    DEACTIVATION_MODE_PAGE_CLOSED         = 1,
    DEACTIVATION_MODE_PAGE_INVISIBLE      = 2,
    NUM_DEACTIVATIONMODES                 = 3
  };

  const char* const DEACTIVATION_MODE_STRINGS[NUM_DEACTIVATIONMODES] = {
    "ExplicitDeactivate",
    "PageClosed",
    "PageInvisible"
  };

  enum LIGHTING_SCHEMES {
    LIGHTING_SCHEME_NONE       = 0,
    LIGHTING_SCHEME_WHITE,
    LIGHTING_SCHEME_DAY,
    LIGHTING_SCHEME_NIGHT,
    LIGHTING_SCHEME_HARD,
    LIGHTING_SCHEME_PRIMARY,
    LIGHTING_SCHEME_BLUE,
    LIGHTING_SCHEME_RED,
    LIGHTING_SCHEME_CUBE,
    LIGHTING_SCHEME_CAD,
    LIGHTING_SCHEME_HEADLAMP,

    NUM_LIGHTING_SCHEMES
  };

  const char* const LightingSchemeStrings[NUM_LIGHTING_SCHEMES] = {
    "None", 
    "White", 
    "Day", 
    "Night", 
    "Hard", 
    "Primary", 
    "Blue", 
    "Red", 
    "Cube", 
    "CAD", 
    "HeadLamp"
  };








} // namespace mlPDF 

ML_END_NAMESPACE

#endif // _PDF_Defines_H