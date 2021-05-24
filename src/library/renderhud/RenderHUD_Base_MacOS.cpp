/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "RenderHUD_Base_MacOS.h"
#ifdef LIBTAS_ENABLE_HUD

#include "../logging.h"
#include "../hook.h"
#include <CoreText/CoreText.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
//#include "../global.h" // shared_config

namespace libtas {

CTFontRef RenderHUD_Base_MacOS::font;
CGFloat RenderHUD_Base_MacOS::font_size = 20;

RenderHUD_Base_MacOS::~RenderHUD_Base_MacOS()
{
    if (font)
        CFRelease(font);
}

void RenderHUD_Base_MacOS::initFonts()
{
    font = CTFontCreateWithName(CFSTR("Times"), font_size, NULL);
    if (!font) {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "CTFontCreateWithName failed");
        shared_config.osd = 0;
    }
}

void RenderHUD_Base_MacOS::renderText(const char* text, Color fg_color, Color bg_color, int x, int y)
{
    if (!font) {
        initFonts();
        if (!font)
            return;
    }

    /* Create the color space and foreground color */
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    if (!colorSpace)
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "CGColorSpaceCreateWithName failed");

    CGFloat components[4];
    components[0] = static_cast<CGFloat>(fg_color.r)/255.0;
    components[1] = static_cast<CGFloat>(fg_color.g)/255.0;
    components[2] = static_cast<CGFloat>(fg_color.b)/255.0;
    components[3] = static_cast<CGFloat>(fg_color.a)/255.0;
    CGColorRef fg_col = CGColorCreate(colorSpace, components);
    if (!fg_col)
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "CGColorCreate failed");

    /* Create an attributed string */
    CFStringRef keys[] = {kCTFontAttributeName, kCTForegroundColorAttributeName};
    CFTypeRef values[] = {font, fg_col};
    CFDictionaryRef attr = CFDictionaryCreate(NULL, (const void **)&keys, (const void **)&values,
    					  sizeof(keys) / sizeof(keys[0]), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFStringRef textString = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
    CFAttributedStringRef attrString = CFAttributedStringCreate(NULL, textString, attr);
    CFRelease(attr);

    /* Draw the string */
    CTLineRef line = CTLineCreateWithAttributedString(attrString);
    if (!line)
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "CTLineCreateWithAttributedString failed");

    //CGContextSetTextMatrix(context, CGAffineTransformIdentity);  //Use this one when using standard view coordinates
    //CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1.0, -1.0)); //Use this one if the view's coordinates are flipped

    /* Get bounds */
    CGFloat ascent, descent, leading;
    CGFloat widthF = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);
    
    int width = static_cast<int>(ceil(widthF));
    int height = static_cast<int>(ceil(ascent+descent+leading));

    /* Create the surface */
    std::unique_ptr<SurfaceARGB> surf(new SurfaceARGB(width, height));
    surf->fill(bg_color);

    /* Create a context to hold the surface data */
    CGContextRef ctx = CGBitmapContextCreate(surf->pixels.data(), width, height, 8, 4*width, colorSpace, kCGImageAlphaPremultipliedLast);
    if (!ctx)
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "CGBitmapContextCreate failed");

    /* Render into the context */
    CGContextSetTextPosition(ctx, 0, 0);
    CTLineDraw(line, ctx);

    /* Change the coords so that the text fills on screen */
    int swidth, sheight;
    ScreenCapture::getDimensions(swidth, sheight);

    x = (x + width + 5) > swidth ? (swidth - width - 5) : x;    
    y = (y + height + 5) > sheight ? (sheight - height - 5) : y;

    /* Call the specific render function */
    renderSurface(std::move(surf), x, y);

    /* Clean up */
    CGColorRelease(fg_col);
    CGContextRelease(ctx);
    CGColorSpaceRelease(colorSpace);
    CFRelease(textString);
    CFRelease(line);
    CFRelease(attrString);
}

}

#endif
