/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 */

/* Image.h: Top level Class to manage all image rendering issues
 * Created: Radha Kulkarni <radha@netscape.com> 21-Aug-1998
 */

#ifndef _xfe_image_h
#define _xfe_image_h

#include "libimg.h"    
#include "ViewGlue.h"
#include "xfe.h"

extern "C"
{
 void fe_DisplayImage(MWContext * context, IL_Pixmap * image, 
                      IL_Pixmap * mask,PRInt32  x,PRInt32  y,
                      PRInt32 x_offset, PRInt32  y_offset, 
		      PRInt32   width, PRInt32   height);

void
Image_GetUrlExitRoutine(URL_Struct *pUrl, int iStatus, MWContext *pContext);

};

class XFE_Image{

public:

  XFE_Image(XFE_Component * frame, char * imageUrl, fe_colormap *, Widget);
  virtual ~XFE_Image();
  virtual Pixmap  getPixmap();
  virtual Pixmap  getMask();
  virtual PRInt32 getImageWidth();
  virtual PRInt32 getImageHeight();
  virtual Boolean  isImageLoaded();
  virtual void     loadImage();


protected:

  MWContext * m_imageContext; // Special MWContext
  fe_ContextData * fec;       // FE specific data for MWContext  
  char * m_urlString;         // Url string  
  IL_Pixmap * m_image;        // The image
  IL_Pixmap * m_mask;         // The mask
  Pixmap m_badImage;          // Bad image to use if the image loading fails
  PRInt32   imageWidth;
  PRInt32   imageHeight;
  XFE_Component * m_frame;

  Boolean     badImage;     // Indicates whether to use the bad bitmap
  Boolean     cxtInitSucceeded;  // Indicates if MWcontext is initialized 
  Boolean     completelyLoaded;  // Indicates if image is completely loaded 
};

#endif  /* _xfe_image_h */
