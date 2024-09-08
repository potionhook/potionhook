/*
 * xoverlay.cpp
 *
 *  Created on: Nov 9, 2017
 *      Author: nullifiedcat
 * 
 *  Converted to C++ by: rosne-gamingyt
 */
#include "internal/drawglx.hpp"
#include <GL/glx.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xfixes.h>
#include <cstring>
#include <cstdio>
#include "xoverlay.hpp"
#include <dlfcn.h> // dlsym

int event_ShapeNotify;
int event_ShapeError;
struct xoverlay_library xoverlay_library;
typedef void (*glXSwapBuffers_t)(Display *dpy, GLXDrawable drawable);
glXSwapBuffers_t glXSwapBuffersfn;

int xoverlay_init()
{
    std::memset(&xoverlay_library, 0, sizeof(struct xoverlay_library));

    xoverlay_library.display = XOpenDisplay(nullptr);
    if (xoverlay_library.display == nullptr)
        return -2;

    xoverlay_library.screen = DefaultScreen(xoverlay_library.display);
    xoverlay_library.width = DisplayWidth(xoverlay_library.display, xoverlay_library.screen);
    xoverlay_library.height = DisplayHeight(xoverlay_library.display, xoverlay_library.screen);

    if (!XShapeQueryExtension(xoverlay_library.display, &event_ShapeNotify, &event_ShapeError))
        return -3;

    if (xoverlay_glx_init() < 0)
        return -4;

    if (xoverlay_glx_create_window() < 0)
        return -5;

    glXSwapBuffersfn = (glXSwapBuffers_t)dlsym(reinterpret_cast<void*>(0xFFFFFFFF), "glXSwapBuffers");

    if (!glXSwapBuffersfn)
        return -6;

    xoverlay_library.init = 1;

    return 0;
}

void xoverlay_destroy()
{
    if (!xoverlay_library.init)
        return;

    XDestroyWindow(xoverlay_library.display, xoverlay_library.window);
    XCloseDisplay(xoverlay_library.display);
    xoverlay_glx_destroy();
    xoverlay_library.init = 0;
}

void xoverlay_show()
{
    if (xoverlay_library.window && !xoverlay_library.visible) {
        XMapWindow(xoverlay_library.display, xoverlay_library.window);
        xoverlay_library.visible = 1;
    }
}

void xoverlay_hide()
{
    if (xoverlay_library.window && xoverlay_library.visible) {
        XUnmapWindow(xoverlay_library.display, xoverlay_library.window);
        xoverlay_library.visible = 0;
    }
}

void xoverlay_draw_begin()
{
    if (!xoverlay_library.init || xoverlay_library.drawing)
        return;

    xoverlay_library.drawing = 1;
    glXMakeCurrent(xoverlay_library.display, xoverlay_library.window, glx_state.context);
}

void xoverlay_draw_end()
{
    if (!xoverlay_library.init || !xoverlay_library.drawing)
        return;

    glXSwapBuffersfn(xoverlay_library.display, xoverlay_library.window);
    xoverlay_library.drawing = 0;
}