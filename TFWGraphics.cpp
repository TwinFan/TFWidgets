/// @file       TFWGraphics.cpp
/// @brief      Implements some basic graphic primitives with OpenGL
///             The only file to use OpenGL directly (so could be exchanged later)
/// @author     Birger Hoppe
/// @copyright  (c) 2018 Birger Hoppe
/// @copyright  Permission is hereby granted, free of charge, to any person obtaining a
///             copy of this software and associated documentation files (the "Software"),
///             to deal in the Software without restriction, including without limitation
///             the rights to use, copy, modify, merge, publish, distribute, sublicense,
///             and/or sell copies of the Software, and to permit persons to whom the
///             Software is furnished to do so, subject to the following conditions:\n
///             The above copyright notice and this permission notice shall be included in
///             all copies or substantial portions of the Software.\n
///             THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///             IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///             FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///             AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///             LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///             OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
///             THE SOFTWARE.

#include "TFWGraphics.h"

// include platform-specific GL headers
#if IBM
#include <windows.h>
#endif

// Open GL
#if LIN
#include <GL/gl.h>
#endif
#if __GNUC__ && APL
#include <OpenGL/gl.h>
#endif
#if IBM
#include <GL/gl.h>
#endif


/// Encapsulates all TFW widget definitions
namespace TFW {

    /// Ensures that elements in `bl` have the smaller values compared to `tr`
    void Rect::Normalize()
    {
        if (bl.x() > tr.x()) std::swap(bl.x(), tr.x());
        if (bl.y() > tr.y()) std::swap(bl.y(), tr.y());
    }
    
    // Is a point within this rectangle?
    bool Rect::Contains(const Point& p) const
    {
        assert(isNormalized());
        return (Left()   <= p.x() && p.x() <= Right() &&
                Bottom() <= p.y() && p.y() <= Top());
    }
    
    // Do 2 rectangle overlap anywhere?
    bool Rect::Overlap (const Rect& r) const
    {
        // rectangles overlap if at least one corner lies in the other rectangle
        return
          Contains(r.Bl()) ||   Contains(r.Tr()) ||
        r.Contains(bl)     || r.Contains(tr);
    }
    
    // Sets the drawing color
    void DrawSetColor (const float color[4])
    {
        glColor4fv(color);
    }
    
    // Sets the line width, 1.0 is normal
    void DrawSetLineWidth (float width)
    {
        glLineWidth(width);
    }
    
    /// @brief Draws any GL mode with the passed in list of points
    /// @param _mode GL mode to draw
    /// @param _pts list of points, their meaning differs with _mode
    static void DrawGL (GLenum _mode, std::initializer_list<Point> _pts)
    {
        glBegin(_mode);
        for (const Point& pt: _pts)
            glVertex2iv(pt);
        glEnd();
    }
    
    // Draws lines between pairs of points
    void DrawLine (std::initializer_list<Point> _pts)
    {
        DrawGL(GL_LINES, _pts);
    }
    
    // Draws a number of connected lines from point to point
    void DrawLineStrip (std::initializer_list<Point> _pts)
    {
        DrawGL(GL_LINE_STRIP, _pts);
    }

    // Draws a rectangle
    void DrawRect (const Rect& _r)
    {
        glRecti(_r.Left(), _r.Top(), _r.Right(), _r.Bottom());
    }
    
    // Draws an unfilled rectangle
    void DrawFrame (const Rect& _r)
    {
        DrawLineStrip({_r.Tl(), _r.Tr(), _r.Br(), _r.Bl(), _r.Tl()});
    }
    
    // @brief Draws a filled polygon
    void DrawPolygon (std::initializer_list<Point> _pts)
    {
        DrawGL(GL_POLYGON, _pts);
    }

    // Draws a single check box
    void DrawCheckBox (const Rect& _r, bool _bSel, bool _bThick)
    {
        // the rectangle which makes up the box
        // (and do it with thick line if being clicked at the moment)
        if (_bThick)
            DrawSetLineWidth(2.0f);
        DrawFrame(_r);
        if (_bThick)
            DrawSetLineWidth(1.0f);
        // X in case it is selected
        if (_bSel) {
            DrawLine({
                _r.Bl() + Point(2, 2), _r.Tr() + Point(-2,-2),
                _r.Tl() + Point(2,-2), _r.Br() + Point(-2, 2)
            });
        }
    }

    // Draws a single Radio Button
    void DrawRadioButton (const Rect& _r, bool _bSel, bool _bThick)
    {
        // Outer diamond
        // (and do it with thick line if being clicked at the moment)
        if (_bThick)
            DrawSetLineWidth(2.0f);
        DrawLineStrip({
            { _r.Left() + _r.Width()/2, _r.Bottom() },  // bottom center
            { _r.Left()               , _r.Bottom() + _r.Height()/2 },
            { _r.Left() + _r.Width()/2, _r.Top() },     // top cneter
            { _r.Right()              , _r.Bottom() + _r.Height()/2 },
            { _r.Left() + _r.Width()/2, _r.Bottom() },  // bottom center
        });
        if (_bThick)
            DrawSetLineWidth(1.0f);

        // Inner filled diamond in case it is selected
        if (_bSel)
            DrawPolygon({
                { _r.Left() + _r.Width()/2, _r.Bottom() + 3 },  // bottom center
                { _r.Left() + 3           , _r.Bottom() + _r.Height()/2 },
                { _r.Left() + _r.Width()/2, _r.Top()    - 3 },     // top cneter
                { _r.Right()- 3           , _r.Bottom() + _r.Height()/2 },
                { _r.Left() + _r.Width()/2, _r.Bottom() + 3 },  // bottom center
            });
    }

    
} // namespace "TFW"
