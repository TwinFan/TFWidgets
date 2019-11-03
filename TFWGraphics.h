/// @file       TFWGraphics.h
/// @brief      Defines some basic graphic primitives with OpenGL
///             The only file to use OpenGL directly (so could be exchanged later)
/// @author     Birger Hoppe
/// @copyright  (c) 2019 Birger Hoppe
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

#ifndef TFWGraphics_h
#define TFWGraphics_h

#include <cassert>
#include <valarray>

/// Encapsulates all TFW widget definitions
namespace TFW {
    
    //
    // MARK: Geometry
    //
    
    /// @brief Groups x,y coordinate as an valarray to define a point in a coordinate system
    ///
    /// `valarray` has same memory storage as a C array, but also provides
    /// mathematical vector operations
    class Point
    {
    public:
        std::valarray<int> pt;              ///< the point's coordinates, x and y
    public:
        Point (int _x = 0, int _y = 0) : pt({_x,_y}) {}
        Point (const std::valarray<int> _va) : pt(_va) { assert (_va.size() == 2); }

        // coordinate access
        int  x () const { return pt[0]; }   ///< x coordinate (const)
        int& x ()       { return pt[0]; }   ///< x coordinate
        int  y () const { return pt[1]; }   ///< y coordinate (const)
        int& y ()       { return pt[1]; }   ///< y coordinate
        
        /// move by an offset
        Point  operator +  (const Point& o) const { return Point(pt + o.pt); }
        /// move by an offset
        Point& operator += (const Point& o)       { pt += o.pt; return *this; }
        /// move by an offset
        Point  operator -  (const Point& o) const { return Point(pt - o.pt); }
        /// move by an offset
        Point& operator -= (const Point& o)       { pt -= o.pt; return *this; }

        /// compare for equality
        bool operator == (const Point& o) const { return x()==o.x() && y()==o.y(); }
        /// compare for inequality
        bool operator != (const Point& o) const { return x()!=o.x() || y()!=o.y();; }

        /// access as an array of two ints, return the address of the first (valarray is guaranteed to have adjacent elements in memory)
        operator const int* () const { return &(pt[0]); }
        /// access as an array of two ints, returnathe address of the first
        operator       int* ()       { return &(pt[0]); }
    };
    
    /// Rectangle defined by two points bottom-left and top-right, normalized so the `bl` < `tr`
    class Rect
    {
    protected:
        Point bl;               ///< bottom-left corner
        Point tr;               ///< top-right cornder
    public:
        ///
        
        /// Normalizes coordinates, so that `bl` < `tr`
        void Normalize ();
        
        /// Constructor
        Rect (int _left = 0, int _top = 0, int _right = 0, int _bottom = 0) :
            bl(_left,_bottom), tr(_right,_top)
            { Normalize(); }
        
        Rect (const Point& _bl, const Point& _tr) :
            bl(_bl), tr(_tr)
            { Normalize(); }
        
        /// Make sure destructor is virtual
        virtual ~Rect() {}

        /// access
        int  Left ()   const  { return bl.x(); }        ///< left coordinate
        int& Left ()          { return bl.x(); }        ///< left coordinate
        int  Bottom () const  { return bl.y(); }        ///< bottom coordinate
        int& Bottom ()        { return bl.y(); }        ///< bottom coordinate
        int  Right ()  const  { return tr.x(); }        ///< right coordinate
        int& Right ()         { return tr.x(); }        ///< right coordinate
        int  Top ()    const  { return tr.y(); }        ///< top coordinate
        int& Top ()           { return tr.y(); }        ///< top coordinate
        const Point& Bl () const { return bl; }         ///< bottom-left point
        const Point& Tr () const { return tr; }         ///< top-right point
        Point Tl () const { return Point(Left(),Top()); }       ///< top-left point
        Point Br () const { return Point(Right(),Bottom()); }   ///< bottom-right point
        Point Center () const { return Point((bl.x()+tr.x())/2, (bl.y()+tr.y())/2); }   ///< returns the center point of the rect
        int Width ()  const  { return Right() - Left(); }       ///< Width
        int Height () const  { return Top() - Bottom(); }       ///< Height
        
        /// Are coordinates normalized? (mainly used in `assert`s
        bool isNormalized () const { return Left() <= Right() && Bottom() <= Top(); }

        /// setting the rectangle's bottom-left corner, then normalizing
        void SetBl (const Point& _bl) { bl = _bl; Normalize(); }
        /// setting the rectangle's top-right corner, then normalizing
        void SetTr (const Point& _tr) { tr = _tr; Normalize(); }
        
        /// setting the rectangle's top, then normalizing
        void SetTop (int _top) { tr.y() = _top; Normalize(); }
        /// setting the rectangle's bottom, then normalizing
        void SetBottom (int _bottom) { bl.y() = _bottom; Normalize(); }
        /// setting the rectangle's left, then normalizing
        void SetLeft (int _left) { bl.x() = _left; Normalize(); }
        /// setting the rectangle's right, then normalizing
        void SetRight (int _right) { tr.x() = _right; Normalize(); }
        
        /// Sets the width, i.e. the right
        void SetWidth (int _w) { SetRight (Left() + _w); }
        /// Sets the height, i.e. the bottom
        void SetHeight (int _h) { SetBottom(Top() - _h); }

        /// compute a rectangle, which is moved by given x/y offset
        Rect operator + (const Point& ofs) const { return Rect(bl+ofs, tr+ofs); }
        /// move rectangle by given x/y offset
        Rect& operator += (const Point& ofs) { bl+=ofs; tr+=ofs; return *this; }
        /// compute a rectangle, which is moved by given -x/-y offset
        Rect operator - (const Point& ofs) const { return Rect(bl-ofs, tr-ofs); }
        /// move rectangle by given -x/-y offset
        Rect& operator -= (const Point& ofs) { bl-=ofs; tr-=ofs; return *this; }
        
        /// Expand/Shrink a rectangle, i.e. all edges move outward/inward
        void Expand (int by) { bl -= Point(by,by); tr += Point(by,by); }

        /// compare two rects for equality
        bool operator == (const Rect& o) const { return bl==o.bl && tr==o.tr; }
        /// compare two rects for inequality
        bool operator != (const Rect& o) const { return bl!=o.bl || tr!=o.tr; }

        /// set geometry of widget
        virtual void SetGeometry (const Rect& _r) { bl=_r.Bl(); tr=_r.Tr(); Normalize(); }
        /// set geometry of widget
        Rect& operator = (const Rect& _r) { SetGeometry(_r); return *this; }
        
        /// Is a point within this rectangle?
        bool Contains (const Point& p) const;
        /// Is the entire other rectangle within this rectangle?
        bool Contains (const Rect& r) const { return Contains(r.bl) && Contains(r.tr); }
        /// Do 2 rectangle overlap anywhere?
        bool Overlap (const Rect& r) const;
    };
    
    /// Sets the drawing color
    void DrawSetColor (const float color[4]);
    /// Sets the line width, 1.0 is normal
    void DrawSetLineWidth (float width);
    
    /// @brief Draws lines between pairs of points
    /// @param _pts list of points, two points define a line, requires even number of elements
    void DrawLine (std::initializer_list<Point> _pts);
    /// @brief Draws a number of connected lines from point to point
    /// @param _pts list of points to connect, at least 2 needed
    void DrawLineStrip (std::initializer_list<Point> _pts);
    /// @brief Draws a filled rectangle
    /// @param _r The rectangle to draw
    void DrawRect (const Rect& _r);
    /// @brief Draws an unfilled rectangle
    /// @param _r The rectangle to draw
    void DrawFrame (const Rect& _r);
    /// @brief Draws a filled polygon
    /// @param _pts list of points defining the polygon
    void DrawPolygon (std::initializer_list<Point> _pts);

    /// @brief Draw a checkbox
    /// @param _r Outer drawing rectangle, size of the check box
    /// @param _bSel Selected? `true` for drawing an inside X
    /// @param _bThick Draw a thick frame around the box instead of a thin?
    void DrawCheckBox (const Rect& _r, bool _bSel, bool _bThick);
    /// @brief Draw a radio button
    /// @param _r Outer drawing rectangle, size of the radio button
    /// @param _bSel Selected? `true` for drawing an inside diamond
    /// @param _bThick Draw a thick frame around the radio button instead of a thin?
    void DrawRadioButton (const Rect& _r, bool _bSel, bool _bThick);

} // namespace "TFW"

#endif /* TFWGraphics_h */
