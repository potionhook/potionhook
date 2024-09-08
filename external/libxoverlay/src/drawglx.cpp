/*
 * drawglx.c
 *
 *  Created on: Nov 9, 2017
 *      Author: nullifiedcat
 * 
 *  Converted to C++ by: rosne-gamingyt
 */

#include "internal/drawglx.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <xoverlay.hpp>
#include <GL/glx.h>
#include <iostream>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xfixes.h>

typedef GLXContext (*glXCreateContextAttribsARBfn)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

xoverlay_glx_state glx_state;

int xoverlay_glx_create_window() {
    GLint attribs[] = {
        GLX_X_RENDERABLE, GL_TRUE,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, GL_TRUE,
        None
    };

    int fbc_count;
    GLXFBConfig* fbc = glXChooseFBConfig(xoverlay_library.display, xoverlay_library.screen, attribs, &fbc_count);
    if (!fbc)
        return -1;

    int fbc_best = -1;
    int fbc_best_samples = -1;

    for (int i = 0; i < fbc_count; ++i) {
        XVisualInfo* info = glXGetVisualFromFBConfig(xoverlay_library.display, fbc[i]);
        if (info->depth >= 32)
            continue;

        int samples;
        glXGetFBConfigAttrib(xoverlay_library.display, fbc[i], GLX_SAMPLES, &samples);

        if (fbc_best < 0 || samples > fbc_best_samples) {
            fbc_best = i;
            fbc_best_samples = samples;
        }

        XFree(info);
    }

    if (fbc_best == -1)
        return -1;

    GLXFBConfig fbconfig = fbc[fbc_best];
    XFree(fbc);

    XVisualInfo* info = glXGetVisualFromFBConfig(xoverlay_library.display, fbconfig);
    if (!info)
        return -1;

    Window root = DefaultRootWindow(xoverlay_library.display);
    xoverlay_library.colormap = XCreateColormap(xoverlay_library.display, root, info->visual, AllocNone);
    XSetWindowAttributes attr;
    attr.background_pixel = 0x0;
    attr.border_pixel = 0;
    attr.save_under = 1;
    attr.override_redirect = 1;
    attr.colormap = xoverlay_library.colormap;
    unsigned long mask = CWBackPixel | CWBorderPixel | CWSaveUnder | CWOverrideRedirect | CWColormap;
    xoverlay_library.window = XCreateWindow(xoverlay_library.display, root, 0, 0, xoverlay_library.width, xoverlay_library.height, 0, info->depth, InputOutput, info->visual, mask, &attr);
    if (!xoverlay_library.window)
        return -1;

    XShapeCombineMask(xoverlay_library.display, xoverlay_library.window, ShapeInput, 0, 0, None, ShapeSet);
    XShapeSelectInput(xoverlay_library.display, xoverlay_library.window, ShapeNotifyMask);

    XserverRegion region = XFixesCreateRegion(xoverlay_library.display, NULL, 0);
    XFixesSetWindowShapeRegion(xoverlay_library.display, xoverlay_library.window, ShapeInput, 0, 0, region);
    XFixesDestroyRegion(xoverlay_library.display, region);

    XStoreName(xoverlay_library.display, xoverlay_library.window, "OverlayWindow");

    xoverlay_show();

    const char* extensions = glXQueryExtensionsString(xoverlay_library.display, xoverlay_library.screen);
    glXCreateContextAttribsARBfn glXCreateContextAttribsARB = (glXCreateContextAttribsARBfn)glXGetProcAddressARB(reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB"));

    int ctx_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
        None
    };

    glx_state.context = glXCreateContextAttribsARB(xoverlay_library.display, fbconfig, nullptr, GL_TRUE, ctx_attribs);
    if (!glx_state.context)
        return -1;

    glXMakeCurrent(xoverlay_library.display, xoverlay_library.window, glx_state.context);
    glewInit();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    return 0;
}

void xoverlay_glx_swap_buffers() {
    glXSwapBuffers(xoverlay_library.display, xoverlay_library.window);
}