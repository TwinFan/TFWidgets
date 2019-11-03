/// @file       TFW.cpp
/// @brief      Implements widget classes for XP11 without using XP's widgets but OpenGL drawing
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

#include "TFW.h"
#include "TFWGraphics.h"

#if IBM
// we prefer std::max/min
#undef max
#undef min
#endif

#include <cassert>
#include <algorithm>
#include <chrono>
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
// clipboard/pasteboard support
#include "clip.h"

/// Encapsulates all TFW widget definitions
namespace TFW {
    
    constexpr int TEXT_OFS = 3;         ///< offset from bottom when using XPLMDrawString
    int gCHAR_AVG_WIDTH = 8;            ///< Avergae char width in pixel, also defines space between a right-aligned and a left-aligned list column
    
    //
    // MARK: Useful colors
    //
    const float COL_NAN[4]              = {  NAN,   NAN,   NAN, 0.00f};
    const float COL_TRANSPARENT[4]      = {-1.0f, -1.0f, -1.0f, 0.00f};
    const float COL_WHITE[4]            = {1.00f, 1.00f, 1.00f, 1.00f};
    const float COL_WHITE_LIGHTTRANS[4] = {1.00f, 1.00f, 1.00f, 0.10f};
    const float COL_YELLOW[4]           = {1.00f, 1.00f, 0.00f, 1.00f};
    const float COL_RED[4]              = {1.00f, 0.00f, 0.00f, 1.00f};
    const float COL_GREEN[4]            = {0.00f, 1.00f, 0.00f, 1.00f};
    const float COL_BLUE[4]             = {0.00f, 0.00f, 1.00f, 1.00f};
    const float COL_BLUE_SELECT[4]      = {0.00f, 0.40f, 0.80f, 1.00f};
    const float COL_GREY[4]             = {0.75f, 0.75f, 0.75f, 1.00f};
    const float COL_GREY_TRANSLUCENT[4] = {0.23f, 0.23f, 0.26f, 0.55f};
    const float COL_GREY_DARK[4]        = {0.50f, 0.50f, 0.50f, 1.00f};
    const float COL_CYAN[4]             = {0.50f, 1.00f, 1.00f, 1.00f};
    
    /// XP standard colors are provided as dataRefs
    const char* XP_COLOR_DR[COL_XP_COUNT] = {
        "sim/graphics/colors/background_rgb",           // float[3]    n    RGB    Background color behind a modal window
        "sim/graphics/colors/menu_dark_rgb",            // float[3]    n    RGB    Dark tinging for menus
        "sim/graphics/colors/menu_hilite_rgb",          // float[3]    n    RGB    Menu color of a selected item
        "sim/graphics/colors/menu_lite_rgb",            // float[3]    n    RGB    Light tinging for menus
        "sim/graphics/colors/menu_text_rgb",            // float[3]    n    RGB    Menu Item Text Color
        "sim/graphics/colors/menu_text_disabled_rgb",   // float[3]    n    RGB    Menu Item Text Color When Disabled
        "sim/graphics/colors/subtitle_text_rgb",        // float[3]    n    RGB    Subtitle text colors
        "sim/graphics/colors/tab_front_rgb",            // float[3]    n    RGB    Color of text on tabs that are forward
        "sim/graphics/colors/tab_back_rgb",             // float[3]    n    RGB    Color of text on tabs that are in the bkgnd
        "sim/graphics/colors/caption_text_rgb",         // float[3]    n    RGB    Caption text (for on a main window)
        "sim/graphics/colors/list_text_rgb",            // float[3]    n    RGB    Text Color for scrolling lists
        "sim/graphics/colors/glass_text_rgb",           // float[3]    n    RGB    Text color for on a 'glass' screen
        "sim/graphics/colors/plane_path1_3d_rgb",       // float[3]    y    RGB    Color for 3-d plane path
        "sim/graphics/colors/plane_path2_3d_rgb",       // float[3]    y    RGB    Striping color for 3-d plane path
    };
    
    /// This array contains the resolved datarefs
    static XPLMDataRef aXPColorDR[COL_XP_COUNT];
    
    // some global color definitions
    static float gCOL_STD[COL_NUMBER_OF_COLORS][4] = {
        {-1.0f, -1.0f, -1.0f, 0.00f}, ///<  COL_STD_BG,                      standard background, default: fully transparent
        {0.90f, 0.90f, 0.90f, 1.00f}, ///<  COL_STD_FG,                      standard foreground, default: dark white
        {0.75f, 0.75f, 0.75f, 1.00f}, ///<  COL_STD_FG_DISABLED,  standard foreground deactivated, default: gray
        {1.00f, 0.00f, 0.00f, 1.00f}, ///<  COL_FOCUS_BORDER,      border around a focus widget
        {0.00f, 0.40f, 0.80f, 1.00f}, ///<  COL_BUTTON_BG,              Button's background
        {1.00f, 1.00f, 1.00f, 1.00f}, ///<  COL_BUTTON_FG,              button's text
        {1.00f, 1.00f, 1.00f, 1.00f}, ///<  COL_COLUMN_HEADER,   List box's column header
        {0.00f, 0.40f, 0.80f, 1.00f}, ///<  COL_SELECTION,                selected item
        {1.00f, 1.00f, 1.00f, 0.10f}, ///<  COL_ALTERNATE_ROW,     alternate list box rows, usually high transparency for lighting up the background
        {1.00f, 1.00f, 1.00f, 0.10f}, ///<  COL_SCROLLBAR_BG,       scrollbar's background, default: lighter than standard bg
        {1.00f, 1.00f, 1.00f, 0.15f}, ///<  COL_SCROLLBAR_FG,       scrollbar's foreground, default: even lighter
        {0.90f, 0.90f, 0.90f, 1.00f}, ///<  COL_EDIT_BG,                    edit field's background, default: very light grey
        {0.00f, 0.00f, 0.00f, 1.00f}, ///<  COL_EDIT_FG,                    edit field's foreground, default: black
        {0.50f, 0.50f, 0.50f, 1.00f}, ///<  COL_LINE,                           divider line's color, default: darker gray
    };

    // Change globally defined colors for all widgets
    void SetGlobalColor (ColCodeE _code, const float _col[4])
    {
        std::memmove(gCOL_STD[_code], _col, sizeof(float[4]));
    }
    
    // GetGlobalColor
    const float* GetGlobalColor (ColCodeE _code)
    {
        return gCOL_STD[_code];
    }
    
    /// @brief Pick one of XP's standard colors
    /// @param inColorID defines the color to be picked, see `XP_COLOR_DR`
    /// @param outColor array of 4 floats to receive the color, or NULL if the OpenGL color shall be set immediately instead
    /// @param inAlphaLevel (optional) alpha level to be added to the color (XP returns none), defaults to 1.0
    /// @see Taken from https://developer.x-plane.com/code-sample/testwidgets/
    void SetupAmbientColor (XPStdColorsE inColorID,
                            float outColor[4] = nullptr,
                            float inAlphaLevel = 1.0f)
    {
        // If we're running the first time, resolve all of our datarefs just once.
        static    bool    firstTime = true;
        if (firstTime)
        {
            firstTime = false;
            for (int n = COL_XP_BACKGROUND_RGB; n < COL_XP_COUNT; ++n)
                aXPColorDR[n] = XPLMFindDataRef(XP_COLOR_DR[n]);
        }
        
        // If being asked to set the color immediately, allocate some storage.
        float    theColor[4];
        float * target = outColor ? outColor : theColor;
        
        // If we have a dataref, just fetch the color from the ref.
        if (aXPColorDR[inColorID])
            XPLMGetDatavf(aXPColorDR[inColorID], target, 0, 3);
        else
            // otherwise revert to white:
            std::memmove (target, COL_WHITE, sizeof(float[4]));
        target[3] = inAlphaLevel;

        // If the user passed NULL, set the color now using the alpha level.
        if (!outColor)
            DrawSetColor(theColor);
    }
    
    /// Sets the color, handles request for XP standard colors
    /// @return Shall drawing continue, i.e. not fully transparent?
    bool SetColor (const float inColor[4])
    {
        // alpha level says: fully transparent?
        if (inColor[3] < 0.01f) {
            // is the R value actually a standard XP color id?
            if (inColor[0] >= 0.0f)
                SetupAmbientColor (XPStdColorsE((int)std::lround(inColor[0])));
            else
                // fully transparent: don't draw
                return false;
        } else {
            // no standard color, so just set what has been passed in
            DrawSetColor(inColor);
        }
        // continue drawing
        return true;
    }
    
    
    /// @brief Gets the color, handles request for XP standard colors
    /// @param[in] inColor input color, might coded with XP standard color id
    /// @param[out] outColor output color, either copy if inColor or the fetched XP standard color
    /// @return Shall drawing continue, i.e. not fully transparent?
    bool TranslateColor (const float* inColor, float* outColor)
    {
        // alpha level says: fully transparent?
        if (inColor[3] < 0.01f) {
            // is the R value actually a standard XP color id?
            if (inColor[0] >= 0.0f)
                SetupAmbientColor (XPStdColorsE((int)std::lround(inColor[0])), outColor);
            else
                // fully transparent: don't draw
                return false;
        } else {
            // no standard color, so just copy what has been passed in
            std::memmove(outColor, inColor, sizeof(float[4]));
        }
        // continue drawing
        return true;
    }
    
    /// @brief Draws text with alignment: vertically centered, horizontally as specified
    void DrawAlignedString (float *             inColorRGB,
                            const Rect&         inRect,
                            TextAlignTy         inAlign,
                            const std::string&  inText,
                            XPLMFontID          inFontID)
    {
        // center text vertically in rect, calculate the text's bottom
        const Point c = inRect.Center();
        int fontHeight = 10;
        XPLMGetFontDimensions(inFontID, &gCHAR_AVG_WIDTH, &fontHeight, nullptr);
        const int y = c.y() - fontHeight/2 + TEXT_OFS;      // text baseline

        // Alignment needs to consider pixel-width of the text
        int x = inRect.Left();
        if (inAlign != TXA_LEFT) {
            const int width = (int)std::lround(XPLMMeasureString(inFontID,
                                                                 inText.c_str(),
                                                                 (int)inText.length()));
            if (inAlign == TXA_CENTER)
                // start to the left of center point
                x = c.x() - width/2;
            else if (inAlign == TXA_RIGHT)
                // start to the left of the right border
                x = inRect.Right() - width;
        }
        
        // get the correct translated color and draw the label
        float col[4];
        if (TranslateColor(inColorRGB, col))
            XPLMDrawString(col, x, y,
                           (char*)inText.c_str(), nullptr, inFontID);
    }
    
    /// @brief Crops a text to fit into a maximum width when drawn
    /// @details Cropping the actual text to draw (called only when
    ///          text or geometry change) avoids using costly
    ///          cropping frame techniques in each drawing frame.
    /// @param _s The text to crop
    /// @param _width The width in pixel the resulting text needs to fit into
    /// @param _startChar (optional) Resulting text starts at _s[_startChar], defaults to `0`
    /// @param _fontId (optional) Font to use for rendering, defaults to `xplmFont_Proportional`
    std::string CropTextToWidth (const std::string& _s, float _width,
                                 size_t _startChar = 0,
                                 XPLMFontID _fontId = xplmFont_Proportional)
    {
        // max number of characters to care about
        const size_t maxChars = _s.size() - _startChar;

        // Sanity checks
        if (maxChars <= 0 || _width < 1.0f) return "";
        if (gCHAR_AVG_WIDTH <= 0) gCHAR_AVG_WIDTH = 8;
        
        // Educated guess about resulting number of characters
        size_t numChars = size_t(_width / gCHAR_AVG_WIDTH);
        
        // If it is too much we keep reducing until it fits
        if (XPLMMeasureString(_fontId, _s.c_str() + _startChar, int(numChars)) > _width)
            while (numChars > 0 &&
                   XPLMMeasureString(_fontId, _s.c_str() + _startChar, int(--numChars)) > _width);
        // else we keep incresing it until it's too much
        else
            while (numChars < maxChars &&
                   XPLMMeasureString(_fontId, _s.c_str() + _startChar, int(numChars+1)) <= _width)
                numChars++;
        
        // return the resulting string
        return _s.substr(_startChar, numChars);
    }

    /// @brief Computes a fitting square for check boxes / radio buttons
    /// @details Takes of `_r` height as given, returns a square which starts one pixel down/right and
    ///          also ends one pixel up, so that there is (at least) one pixel of room
    Rect MakeCheckboxSquare (const Rect& _r)
    {
        Rect dr = _r;
        dr.Top()--;                         // move one pixel down and right
        dr.Left()++;
        // an even height is nicer for drawing the Radio Buttons
        if (dr.Height() % 2 > 0)
            dr.Bottom() += 1;
        else
            dr.Bottom() += 2;
        // make it a proper square
        dr.SetWidth(dr.Height());
        
        return dr;
    }
    
    //
    // MARK: Global callback functions
    //       They just pass on the call to the MainWnd member functions
    //
    
    /// Draw function
    static void CBDraw (XPLMWindowID /*inWindowID*/, void* inRefcon)
    {
        Rect noRect;
        // the refcon is the pointer to our object
        assert(inRefcon);
        return reinterpret_cast<MainWnd*>(inRefcon)->
        DoDraw(noRect);
    }
    
    /// Left Mouse click function
    static int CBMouseClickLeft (XPLMWindowID         /*inWindowID*/,
                                 int                  x,
                                 int                  y,
                                 XPLMMouseStatus      inMouse,
                                 void *               inRefcon)
    {
        // the refcon is the pointer to our object
        assert(inRefcon);
        reinterpret_cast<MainWnd*>(inRefcon)->
        DoMouseClick({x,y}, MOUSE_LEFT, inMouse);
        // always consume the click as per recommendation
        return 1;
    }
    
    /// Right Mouse click function
    static int CBMouseClickRight (XPLMWindowID         /*inWindowID*/,
                                  int                  x,
                                  int                  y,
                                  XPLMMouseStatus      inMouse,
                                  void *               inRefcon)
    {
        // the refcon is the pointer to our object
        assert(inRefcon);
        reinterpret_cast<MainWnd*>(inRefcon)->
        DoMouseClick({x,y}, MOUSE_RIGHT, inMouse);
        // always consume the click as per recommendation
        return 1;
    }

    /// Handle Mouse Wheel function
    static int CBMouseWheel (XPLMWindowID         /*inWindowID*/,
                             int                  x,
                             int                  y,
                             int                  wheel,
                             int                  clicks,
                             void *               inRefcon)
    {
        // the refcon is the pointer to our object
        assert(inRefcon);
        reinterpret_cast<MainWnd*>(inRefcon)->
        DoMouseWheel({x,y}, wheel, clicks);
        // always consume the click as per recommendation
        return 1;
    }

    /// Handle key function
    static void CBKey (XPLMWindowID         /*inWindowID*/,
                       char                 inKey,
                       XPLMKeyFlags         inFlags,
                       char                 inVirtualKey,
                       void *               inRefcon,
                       int                  losingFocus)
    {
        // the refcon is the pointer to our object
        assert(inRefcon);
        // we split up handling of keys and losing focus right here already
        if (!losingFocus)
            reinterpret_cast<MainWnd*>(inRefcon)->
            DoHandleKey(inKey, inFlags, (unsigned char)inVirtualKey);
        else
            reinterpret_cast<MainWnd*>(inRefcon)->
            DoFocusChange(FCS_LOOSE);
    }

    /// Handle cursor function
    static XPLMCursorStatus CBCursor (XPLMWindowID         /*inWindowID*/,
                                      int                  x,
                                      int                  y,
                                      void *               inRefcon)
    {
        // the refcon is the pointer to our object
        assert(inRefcon);
        return reinterpret_cast<MainWnd*>(inRefcon)->
        DoHandleCursor({x,y});
    }
    
    //
    // MARK: Croppable Text
    //

    // Recalculate `sCropped` from `sFull`
    void CroppedString::UpdateCroppedString ()
    {
        sCropped = CropTextToWidth(sFull, (float)width, startPos);
    }

    //
    // MARK: Widget classes
    //
    
    // constructor adds widget to main window
    /// @param lR left positioning rule
    /// @param lO offset for left pos
    /// @param tR top positioning rule
    /// @param tO offset for top pos
    /// @param rR right positioning rule
    /// @param rO offset for right pos
    /// @param bR bottom positioning rule
    /// @param bO offset for bottom pos
    Widget::Widget (const std::string& _caption,
                    MainWnd& main,
                    WidgetPosE lR, int lO,
                    WidgetPosE tR, int tO,
                    WidgetPosE rR, int rO,
                    WidgetPosE bR, int bO) :
    mainWnd(main)
    {
        // set some standard colors
        SetBkColor(gCOL_STD[COL_STD_BG]);
        SetFgColor(gCOL_STD[COL_STD_FG], gCOL_STD[COL_STD_FG_DISABLED]);

        // set own caption
        SetCaption(_caption);

        // add to main window (if I'm not the main window myself)
        if (!IsMainWnd())
            main.Add({lR, lO, tR, tO, rR, rO, bR, bO, *this});
    }
    
    // Does the widget have focus?
    bool Widget::HasFocus () const
    {
        return mainWnd.GetFocusWidget() == this;
    }
    
    // Is this the main window?
    bool Widget::IsMainWnd () const
    {
        return this == &mainWnd;
    }

    // Called by MainWnd, this function does the drawing
    void Widget::DoDraw(const Rect& r)
    {
        DrawBackground(r);
    }
    
    // paint the background of the widget area
    void Widget::DrawBackground(const Rect& r)
    {
        if (SetColor(colBk))
        {
            DrawRect(r);
        }
    }
    
    //
    // Label
    //
             
    // constructor adds widget to main window
    /// @param lR left positioning rule
    /// @param lO offset for left pos
    /// @param tR top positioning rule
    /// @param tO offset for top pos
    /// @param rR right positioning rule
    /// @param rO offset for right pos
    /// @param bR bottom positioning rule
    /// @param bO offset for bottom pos
    Label::Label (const std::string& _caption,
                  MainWnd& main,
                  WidgetPosE lR, int lO,
                  WidgetPosE tR, int tO,
                  WidgetPosE rR, int rO,
                  WidgetPosE bR, int bO) :
    Widget(_caption, main, lR, lO, tR, tO, rR, rO, bR, bO)
    {}
    
    // Called by MainWnd, this function does the drawing of the label
    void Label::DoDraw (const Rect& r)
    {
        // draw the background
        Widget::DoDraw(r);
        // draw the text
        DrawAlignedString(IsEnabled() ? colFg : colFgDisabled, r, TXA_LEFT,
                          caption.cropped(), xplmFont_Proportional);
        // Bold text? Requires one more write operation with ofset 1 pt
        if (bBold)
            DrawAlignedString(IsEnabled() ? colFg : colFgDisabled, r + Point(1,0), TXA_LEFT,
                              caption.cropped(), xplmFont_Proportional);
    }

    //
    // Line
    //

    // construct a line, for positioning params see struct WidgetAddTy
    /// @param lR left positioning rule (top/left define the start point of the line)
    /// @param lO offset for left pos
    /// @param tR top positioning rule (top/left define the start point of the line)
    /// @param tO offset for top pos
    /// @param rR right positioning rule (bottom/right define the end point of the line)
    /// @param rO offset for right pos
    /// @param bR bottom positioning rule (bottom/right define the end point of the line)
    /// @param bO offset for bottom pos
    Line::Line (const std::string& _caption,
                MainWnd& main,
                WidgetPosE lR, int lO,
                WidgetPosE tR, int tO,
                WidgetPosE rR,  int rO,
                WidgetPosE bR,  int bO,
                float _lnWidth) :
    Widget(_caption, main, lR, lO, tR, tO, rR, rO, bR, bO),
    lnWidth (_lnWidth)
    {
        // set standard color for a divider line
        SetFgColor(gCOL_STD[COL_LINE], gCOL_STD[COL_STD_FG_DISABLED]);
    }

    // Called by MainWnd, this function does the drawing of the line
    void Line::DoDraw (const Rect& _r)
    {
        // draw a line from top/left to bottom/right
        DrawSetColor(colFg);
        DrawLine({_r.Tl(), _r.Br()});
    }

    
    //
    // MARK: Buttons
    //

    // constructor adds widget to main window
    /// @param lR left positioning rule
    /// @param lO offset for left pos
    /// @param tR top positioning rule
    /// @param tO offset for top pos
    /// @param rR right positioning rule
    /// @param rO offset for right pos
    /// @param bR bottom positioning rule
    /// @param bO offset for bottom pos
    ButtonPush::ButtonPush (const std::string& _caption,
                            MainWnd& main,
                            WidgetPosE lR, int lO,
                            WidgetPosE tR, int tO,
                            WidgetPosE rR, int rO,
                            WidgetPosE bR, int bO) :
    Widget(_caption, main, lR, lO, tR, tO, rR, rO, bR, bO)
    {
        SetBkColor(gCOL_STD[COL_BUTTON_BG]);
        SetFgColor(gCOL_STD[COL_BUTTON_FG], gCOL_STD[COL_STD_FG_DISABLED]);
    }
    

    // Toggles button status
    void ButtonPush::DoMouseClick (Point /*p*/, MouseBtnTy mouseBtn, XPLMMouseStatus inMouse)
    {
        // we only deal with left clicks
        if (mouseBtn != MOUSE_LEFT) return;
        
        // a mouse down event changes the status to...well...down
        if (inMouse == xplm_MouseDown)
            btnState = BTN_DOWN;
        else if (inMouse == xplm_MouseUp)
            // a mouse up event ends the operation
            btnState = ButtonClicked();
    }
    
    // [Space] is like click
    void ButtonPush::DoHandleKey (char /*inKey*/, XPLMKeyFlags inFlags, unsigned char inVirtualKey)
    {
        // only handle [Space]
        if (inVirtualKey != XPLM_VK_SPACE) return;
        
        // a key down event changes the status to...well...down
        if (inFlags == xplm_DownFlag)       // no modifiers allowed!
            btnState = BTN_DOWN;
        else if (inFlags == xplm_UpFlag)
            // a key up event ends the operation
            btnState = ButtonClicked();
    }
    
    // Called by MainWnd, this function does the drawing of the push button
    void ButtonPush::DoDraw (const Rect& _r)
    {
        // blue box
        if (SetColor(IsEnabled() ? colBk : colFgDisabled))
            DrawRect(_r);
        
        // if "pushed" then we lighten up the button
        if (btnState == BTN_DOWN) {
            SetColor(COL_WHITE_LIGHTTRANS);
            DrawRect(_r);
        }
        
        // if we have focus draw a red border, otherwise a white one
        SetColor(HasFocus() ? gCOL_STD[COL_FOCUS_BORDER] : colFg);
        DrawFrame(_r);
        
        // finally add the button's text
        DrawAlignedString(colFg, _r, TXA_CENTER, GetVisibleCap(), xplmFont_Proportional);
    }
    
    // Handle a complete click, i.e. after down and up events
    ButtonStateTy ButtonPush::ButtonClicked ()
    {
        // inform MainWnd
        mainWnd.MsgButtonClicked(*this);
        // return to normal state
        return BTN_NORMAL;
    }
    
    //
    // ButtonElem
    //
    
    // placeholder for an empty non-existing element returned if refCon not found
    ButtonElem BUTTON_ELEM_NULL = ButtonElem(nullptr, "NULL");

    // Define the buttons current state as selected or not
    void ButtonElem::SetSelected (bool _bSel)
    {
        if (_bSel)
            btnState = IsDown() ? BTN_DOWN_SELECTED : BTN_SELECTED;
        else
            btnState = IsDown() ? BTN_DOWN          : BTN_NORMAL;
    }

    // Define the buttons current state as down or up
    void ButtonElem::SetDown (bool _bDown)
    {
        if (_bDown)
            btnState = IsSelected() ? BTN_DOWN_SELECTED : BTN_DOWN;
        else
            btnState = IsSelected() ? BTN_SELECTED      : BTN_NORMAL;
    }
        
    /// Resizing means we might need to crop the caption
    void ButtonElem::SetGeometry(const Rect &_r)
    {
        // pass up the class hierarchy
        Rect::SetGeometry(_r);
        // pass on to the caption, but leave room for symbol
        Caption.SetWidth(size_t(std::max(_r.Width() - _r.Height() - 3, 0)));
    }
    
    // Draw the button element
    /// @param _r Drawing rectangle of the entire button array in display coordinates
    /// @param _type Check Box or Radio Button
    /// @param _col Color for drawing (text and symbol)
    /// @param _bFocus Element has focus?
    void ButtonElem::DoDraw (const Rect& _r, ButtonArrayTy _type, float _col[4], bool _bFocus)
    {
        // We need 2 rectangles: One for the symbol in front,
        // one for the caption
        Rect dr (*this);                // copy of ButtonElem's dimension
        dr += _r.Tl();                  // move to actual drawing pos on display
        
        // If requested draw a focus rectangle
        if (_bFocus && _r.Contains(dr)) {
            SetColor(gCOL_STD[COL_FOCUS_BORDER]);
            DrawFrame(dr);
        }
        
        // Caption rectangle: Just move the left border inward:
        dr.SetLeft(dr.Left() + dr.Height() + 3);
        // Draw the buttons text
        if (_r.Contains(dr))
            DrawAlignedString(_col, dr, TXA_LEFT,
                              Caption.cropped(),
                              xplmFont_Proportional);
        
        // Symbol rectangle
        dr = MakeCheckboxSquare(*this);
        dr += _r.Tl();                  // move to actual drawing pos on display

        // Draw symbol in front
        if (_r.Contains(dr)) {
            SetColor(_col);
            switch (_type) {
                case BTN_CHECK_BOXES:
                    DrawCheckBox(dr, IsSelected(), IsDown());
                    break;
                case BTN_RADIO_BUTTONS:
                    DrawRadioButton(dr, IsSelected(), IsDown());
                    break;
            }
        }
    }
        
    //
    // ButtonArray
    //
    
    // constructor adds widget to main window
    /// @param lR left positioning rule
    /// @param lO offset for left pos
    /// @param tR top positioning rule
    /// @param tO offset for top pos
    /// @param rR right positioning rule
    /// @param rO offset for right pos
    /// @param bR bottom positioning rule
    /// @param bO offset for bottom pos
    ButtonArray::ButtonArray (const std::string& _caption,
                              MainWnd& main,
                              ButtonArrayTy _type,
                              bool _bVertical,
                              WidgetPosE lR, int lO,
                              WidgetPosE tR, int tO,
                              WidgetPosE rR, int rO,
                              WidgetPosE bR, int bO) :
    Widget(_caption, main, lR, lO, tR, tO, rR, rO, bR, bO),
    type (_type), bVertical(_bVertical)
    {
        // init to "no focus element"
        iterFocus = aBtns.end();
    }
    
    // Add a button to the array
    bool ButtonArray::AddButton (ButtonElem&& _elem)
    {
        // Need a defined refCon, otherwise indistringuishable from BUTTON_ELEM_NULL!
        if (_elem == BUTTON_ELEM_NULL)
            return false;
        
        if (FindElem(_elem.refCon) == BUTTON_ELEM_NULL) {
            aBtns.emplace_back(std::move(_elem));
            return true;
        }
        return false;
    }
    
    // Finds an element by refCon
    ButtonElem& ButtonArray::FindElem (void* _refCon)
    {
        ButtonElemArr::iterator _iter =
        std::find_if(aBtns.begin(), aBtns.end(),
                     [_refCon](const ButtonElem& be){return be.refCon == _refCon;});
        return _iter != aBtns.cend() ? *_iter : BUTTON_ELEM_NULL;
    }
    
    // Find an element by index
    ButtonElem& ButtonArray::operator[](size_t i)
    {
        if (i < aBtns.size())
            return aBtns[i];
        return BUTTON_ELEM_NULL;
    }

    // Return (first) selected element
    const ButtonElem& ButtonArray::GetSelected () const
    {
        for (const ButtonElem& elem: aBtns)
            if (elem.IsSelected())
                return elem;
        return BUTTON_ELEM_NULL;
    }

    // Set the selected element
    void ButtonArray::SetSelected (void* _refCon)
    {
        ButtonElem& elem = FindElem(_refCon);
        if (elem != BUTTON_ELEM_NULL)
            ButtonClicked(elem);
    }

    // Resizing means we need to recalculate the ButtonElems' positions
    void ButtonArray::SetGeometry(const Rect &_r)
    {
        // pass up the class hierarchy
        Rect::SetGeometry(_r);
        
        // Update geometry of button elements
        
        // rectangle defining one button element, localized to the button array,
        // i.e. starting with top-left corner = 0/0
        Rect btnRect = _r - _r.Tl();
        btnRect.Bottom() = btnRect.Top() - int(lnHeight);
        
        // in case of horizontal alignment: reduce width
        int horzSize = btnRect.Width();
        if (!bVertical && aBtns.size() > 1) {
            horzSize /= int(aBtns.size());
            btnRect.SetRight(btnRect.Left() + horzSize);
        }

        // The above calcs the first buttons dimensions,
        // loop now over all button elements and assign to them
        for (ButtonElem& elem: aBtns)
        {
            elem.SetGeometry(btnRect);
            
            // move the rectangle downward / to the right
            if (bVertical)
                btnRect    += Point(0, -(int)lnHeight);
            else
                btnRect    += Point(horzSize, 0);
        }
    }
    
    // Toggles button status
    void ButtonArray::DoMouseClick (Point p, MouseBtnTy mouseBtn, XPLMMouseStatus inMouse)
    {
        // We only handle left clicks
        if (mouseBtn != MOUSE_LEFT) return;
        
        // We save the button, which got clicked during button-down event,
        // because that is the one to handle the button-up event, too
        static ButtonElem* pMouseFocusElem = nullptr;
        
        if (inMouse == xplm_MouseDown)
        {
            ButtonElemArr::iterator iter;
            if (FindButtonByPos(p, iter)) {
                pMouseFocusElem = &(*iter);
                iterFocus = iter;           // this is now also the one having regular focus
            }
        }
        
        // no mouse focus element? bail
        if (!pMouseFocusElem) return;
        
        // What happens now to the focus element?
        switch (inMouse) {
                // While we click on the element it is "down"
            case xplm_MouseDown:
                pMouseFocusElem->SetDown(true);
                break;

                // While we move on element it keeps being down,
                // but if we move out it goes back to what it was before
            case xplm_MouseDrag:
                pMouseFocusElem->SetDown(pMouseFocusElem->Contains(p));
                break;
                
                // Mouse-up only has a lasting effect if inside the element
            case xplm_MouseUp:
                pMouseFocusElem->SetDown(false);
                if (pMouseFocusElem->Contains(p))
                    ButtonClicked(*pMouseFocusElem);
                break;
        }
    }
    
    // Handle a complete click, i.e. after down and up events
    void ButtonArray::ButtonClicked (ButtonElem& _selBe)
    {
        switch (type) {
            case BTN_CHECK_BOXES:
                // just toggle the element's individual state
                _selBe.SetSelected(!_selBe.IsSelected());
                break;
                
            case BTN_RADIO_BUTTONS:
                // set all buttons to unselected except for `be`
                for (ButtonElem& be: aBtns)
                    be.SetSelected(be == _selBe);
                break;
        }
        
        // inform MainWnd
        mainWnd.MsgButtonArrClicked(*this, _selBe);
    }

    // [Space] is like click
    void ButtonArray::DoHandleKey (char /*inKey*/, XPLMKeyFlags inFlags, unsigned char inVirtualKey)
    {
        switch (inVirtualKey) {
            case XPLM_VK_SPACE:             // Space toggles button
                // for Space to work we need a focus element
                if (iterFocus == aBtns.end()) return;
                
                if (inFlags == xplm_DownFlag)
                    iterFocus->SetDown(true);
                else if (inFlags == xplm_UpFlag) {
                    iterFocus->SetDown(false);
                    ButtonClicked(*iterFocus);
                }
                break;
                
            case XPLM_VK_LEFT:              // left/up move the focus towards beginning
            case XPLM_VK_UP:
                if (inFlags == xplm_DownFlag && !DoFocusChange(FCS_PREV))
                    // but we'll never loose focus this way
                    iterFocus = aBtns.begin();
                break;
                
            case XPLM_VK_RIGHT:             // right/down move the focus towards end
            case XPLM_VK_DOWN:
                if (inFlags == xplm_DownFlag && !DoFocusChange(FCS_NEXT))
                    // but we'll never loose focus this way
                    iterFocus = std::prev(aBtns.end());
                break;
        }
    }
    
    // Button arrays accept focus and pass it down to ButtomElems:
    bool ButtonArray::DoFocusChange (FocusChangeTy _fcs)
    {
        // sanity checks
        if (aBtns.empty())
            return false;
        
        // handle the focus types
        switch (_fcs) {
            case FCS_FIRST:             // focus the first element
                iterFocus = aBtns.begin();
                break;
            case FCS_NEXT:              // focus the next element if any
                if (iterFocus != aBtns.end())
                    iterFocus++;
                break;
            case FCS_LAST:              // focus the last element
                iterFocus = aBtns.end();
                [[fallthrough]];
            case FCS_PREV:              // focus to the previous element if any
                if (iterFocus != aBtns.begin()) {
                    iterFocus--;
                    return true;
                }
                // reached beginning, so we give up focus
                [[fallthrough]];
            case FCS_LOOSE:
                iterFocus = aBtns.end();
                break;
        }
        
        // we keep focus if we still have a focus element
        return iterFocus != aBtns.end();
    }

    // Called by MainWnd, this function does the drawing of the push button
    void ButtonArray::DoDraw (const Rect& _r)
    {
        for (ButtonElemArr::iterator iterElem = aBtns.begin();
             iterElem != aBtns.end();
             iterElem++)
        {
            iterElem->DoDraw(_r, type,
                             IsEnabled() ? colFg : colFgDisabled,
                             iterElem == iterFocus);
        }
    }
  
    // Identify which button element a point falls into, NULL if none
    bool ButtonArray::FindButtonByPos (const Point& _p, ButtonElemArr::iterator& _iter)
    {
        _iter = std::find_if(aBtns.begin(), aBtns.end(),
                             [_p](const ButtonElem& be){return be.Contains(_p);});
        return _iter != aBtns.end();
    }
    
    //
    // MARK: Edit Field
    //
    
    // construct a edit field
    /// @param lR left positioning rule
    /// @param lO offset for left pos
    /// @param tR top positioning rule
    /// @param tO offset for top pos
    /// @param rR right positioning rule
    /// @param rO offset for right pos
    /// @param bR bottom positioning rule
    /// @param bO offset for bottom pos
    EditField::EditField (const std::string& _caption,
                          const std::string& _allowed,
                          unsigned _maxLen,
                          MainWnd& main,
                          WidgetPosE lR, int lO,
                          WidgetPosE tR, int tO,
                          WidgetPosE rR, int rO,
                          WidgetPosE bR, int bO) :
    Widget(_caption, main, lR, lO, tR, tO, rR, rO, bR, bO),
    allowedChars(_allowed), maxLen(_maxLen)
    {
        // set up standard colors
        SetBkColor(gCOL_STD[COL_EDIT_BG]);
        SetFgColor(gCOL_STD[COL_EDIT_FG], gCOL_STD[COL_STD_FG_DISABLED]);
        
        // caret is at end of string
        caretPos = _caption.size();
    }

    /// Set floating point value
    void EditField::Set (double _d, int precision)
    {
        char buf[100];
        snprintf(buf, sizeof(buf), "%.*f", precision, _d);
        SetCaption(buf);
    }

    // Copy into clipboard/pasteboard (currently just the entire caption)
    void EditField::ClipCopy ()
    {
        clip::set_text(caption.full());
    }

    // Copy into clipboard/pasteboard and clear
    void EditField::ClipCut ()
    {
        ClipCopy();
        caption.clear();
        caretPos = 0;
    }
    
    // Paste text from clipboard/pasteboard at current caret position
    void EditField::ClipPaste ()
    {
        std::string s;
        if (clip::get_text(s)) {
            // watch maximum allowed size, copy only until full
            const std::string::size_type cpyCnt = std::min(maxLen - caption.size(),
                                                           s.length());
            if (cpyCnt > 0) {
                // safeguard insert position
                if (caretPos > caption.size())
                    caretPos = caption.size();
                // insert the actual characters
                caption.insert(caretPos, s, 0, cpyCnt);
                caretPos += cpyCnt;             // advance the caret accordingly
            }
        }
    }

    // Entry into the fields
    void EditField::DoHandleKey (char inKey, XPLMKeyFlags inFlags, unsigned char inVirtualKey)
    {
        // Ignore the up event
        if (!(inFlags & xplm_UpFlag)) {
            switch (inVirtualKey) {
                case XPLM_VK_BACK:              // Backspace deletes one character to the left
                    if (caretPos > 0)
                        caption.erase(--caretPos, 1);
                    return;
                    
                case XPLM_VK_DELETE:            // deletes one character to the right
                    if (caretPos < caption.size())
                        caption.erase(caretPos, 1);
                    return;
                    
                case XPLM_VK_CLEAR:             // removes everything
                    caption.clear();
                    caretPos = 0;
                    return;
                    
                case XPLM_VK_LEFT:              // moves caret left
                    if (caretPos > 0)
                        caretPos--;
                    return;
                    
                case XPLM_VK_RIGHT:             // moves caret right
                    if (caretPos < caption.size())
                        caretPos++;
                    return;
                    
                case XPLM_VK_HOME:              // moves caret to beginning
                    caretPos = 0;
                    return;

                case XPLM_VK_END:               // moves caret to the end
                    caretPos = caption.size();
                    return;
            }
        }
        
        // Copy and paste keyboard combinations
        if (inFlags == (xplm_DownFlag | xplm_ControlFlag))
        {
            switch (inVirtualKey) {
                case XPLM_VK_INSERT:
                case XPLM_VK_C:         ClipCopy();     return;
                case XPLM_VK_X:         ClipCut();      return;
                case XPLM_VK_V:         ClipPaste();    return;
            }
        }
        if (inFlags == (xplm_DownFlag | xplm_ShiftFlag))
        {
            switch (inVirtualKey) {
                case XPLM_VK_INSERT:    ClipPaste();    return;
            }
        }

        // Just add any printable standard char to the end and forward the caret
        // if it is an allowed character, that is
        if (!(inFlags & xplm_UpFlag) &&         // ignore the up event
            inKey >= ' ' &&                     // printable
            (allowedChars.empty() || allowedChars.find(inKey) != std::string::npos) &&
            (caption.size() < maxLen))
        {
            // safeguard insert position
            if (caretPos > caption.size())
                caretPos = caption.size();
            // insert the actual character
            caption.insert(caretPos++, std::string(1, inKey));
        }
    }
    
    // Places the caret
    void EditField::DoMouseClick (Point p, MouseBtnTy mouseBtn, XPLMMouseStatus inMouse)
    {
        // Only handle left mouse click down event
        if (mouseBtn != MOUSE_LEFT || inMouse != xplm_MouseDown)
            return;
        
        // determine the character that is hit
        size_t pos = 0;
        const std::string drawTxt = GetDrawString();
        for (; pos <= drawTxt.length(); pos++)
        {
            if (XPLMMeasureString(xplmFont_Proportional, drawTxt.c_str(), (int)pos) > p.x())
                break;
        }
        
        // set caret pos
        caretPos = caption.GetStartPos() + (pos > 0 ? pos-1 : 0);
    }
    
    // Wheel scrolls the text left and right
    void EditField::DoMouseWheel (Point /*p*/, int /*wheel*/, int clicks)
    {
        // scrolling left
        if (clicks < 0) {
            clicks *= -1;
            if (caption.GetStartPos() > (size_t)clicks)
                caption.SetStartPos(caption.GetStartPos() - (size_t)clicks);
            else
                caption.SetStartPos(0);
        } else {
            // scrolling right, only if the cropped part is missing some text to the right
            if (caption.GetNumCharsCropped() > 0)
                caption.SetStartPos(caption.GetStartPos() + (size_t)clicks);
        }
    }
    
    // Edit fields accept focus
    bool EditField::DoFocusChange (FocusChangeTy _fcs)
    {
        // are we losing focus and has the text changed?
        if (_fcs != FCS_FIRST && _fcs != FCS_LAST &&
            caption != prevTxt)
        {
            mainWnd.MsgEditFieldChanged(*this);
            prevTxt = caption.full();
        }
        
        // We accept focus if we got it
        return _fcs == FCS_FIRST || _fcs == FCS_LAST;
    }

    
    // Called by MainWnd, this function does the drawing of the field
    void EditField::DoDraw (const Rect& _r)
    {
        static int x = 0;

        // Background
        SetColor(colBk);
        DrawRect(_r);
        
        // Focus?
        if (HasFocus()) {
            // Caret

            // mini cache for measured caret string
            static EditField* pLastEditField = nullptr;
            static size_t lastCaretPos = 0;
            static size_t lastStartPos = 0;
            size_t currStartPos = caption.GetStartPos();
            if (pLastEditField != this || lastCaretPos != caretPos || lastStartPos != currStartPos)
            {
                pLastEditField = this;
                lastCaretPos = caretPos;
                lastStartPos = currStartPos;
                // calculate where the caret would be
                x = (int)std::lround(XPLMMeasureString(xplmFont_Proportional,
                                                       GetDrawString().c_str(),
                                                       (int)(caretPos - currStartPos)));
            }
                
            // HERE WE CALCULATE IF THE TEXT NEEDS TO SCROLL
            // yea...during drawing...bad hack...but we happen to know where the caret is
            
            // if caret would be close to right edge
            if (x >= _r.Width() - gCHAR_AVG_WIDTH)
            {
                // shift the text left by advancing its starting position
                while ((x >= _r.Width() - gCHAR_AVG_WIDTH) &&
                       currStartPos < caption.size()-1)
                {
                    caption.SetStartPos(currStartPos += 5);
                    x = (int)std::lround(XPLMMeasureString(xplmFont_Proportional,
                                                           GetDrawString().c_str(),
                                                           (int)(caretPos - currStartPos)));
                }
            }
            // if care is close to the left edge
            else if (x <= gCHAR_AVG_WIDTH)
            {
                // shoft the text right by reducing its starting position
                while ((x <= gCHAR_AVG_WIDTH) &&
                       currStartPos > 0)
                {
                    caption.SetStartPos(currStartPos > 5 ? currStartPos - 5 : 0);
                    currStartPos = caption.GetStartPos();
                    x = (int)std::lround(XPLMMeasureString(xplmFont_Proportional,
                                                           GetDrawString().c_str(),
                                                           (int)(caretPos - currStartPos)));
                }
            }
        }
        
        // Text
        DrawAlignedString(IsEnabled() ? colFg : colFgDisabled, _r,
                          TXA_LEFT, GetDrawString(),
                          xplmFont_Proportional);
        
        // Focus?
        if (HasFocus()) {
            // Caret
            // this makes the caret blink: Only draw it in the first half of any second
            long long ms;
            {
                using namespace std::chrono;
                ms = time_point_cast<milliseconds>(system_clock::now()).time_since_epoch().count();
            }
            if (ms % 1000 < 500) {
                SetColor(gCOL_STD[COL_SELECTION]);
                DrawLine({ {_r.Left() + x, _r.Top()-1},
                           {_r.Left() + x, _r.Bottom()+3} });
            }

            // Focus rect
            SetColor(gCOL_STD[COL_FOCUS_BORDER]);
            DrawFrame(_r);
        }
        
    }
    
    
    // returns the current string to draw, which depends on scrolling and on conversions like password-*
    std::string EditField::GetDrawString () const
    {
        return bPwdMode ? std::string(caption.cropped().length(), '*') : caption.cropped();
    }
    

    //
    // MARK: List Box
    //
    
    /// multi-purpose text buffer, e.g. for snprintf operations
    static char lbBuf[1024];
    
    // Sets current value to a double value
    void ListItem::Set (double _d, int _prec)
    {
        snprintf(lbBuf, sizeof(lbBuf), "%.*f", _prec, dVal = _d);
        sVal = lbBuf;
    }
    
    // compare function, used for sorting
    bool ListItem::less (const ListItem& o, DataTypeTy _dataType,
                         bool _thisBeforeOnEqual) const
    {
        switch(_dataType) {
            case DTY_STRING:
                return sVal < o.sVal ? true :
                sVal == o.sVal ? _thisBeforeOnEqual : false;
            case DTY_CHECKBOX:
            case DTY_INT:
                return iVal < o.iVal ? true :
                iVal == o.iVal ? _thisBeforeOnEqual : false;
            case DTY_DOUBLE:    return dVal < o.dVal;
            case DTY_NULL:      return _thisBeforeOnEqual;
        }
        return true;
    }
    
    
    // Constructor creates a row
    ListRow::ListRow (void* _refCon,
                      std::initializer_list<ListItem> _fields) :
    std::vector<ListItem>(_fields),
    refCon(_refCon)
    {}
    
    
    // compare function on any element, used for sorting
    bool ListRow::elemLess (const ListRow& o,
                            unsigned _col,
                            SortingTy _sorted,
                            DataTypeTy _dataType) const
    {
        if (_col >= size()) return true;        // I don't have enough elements, and 'empty' always comes first
        if (_col >= o.size()) return false;     // The other doesn't have enough elements
        
        // consider if sorting shall be ascending or descending
        switch (_sorted) {
            case SORT_ASCENDING:    return  operator[](_col).less(o[_col], _dataType, sequNr < o.sequNr);
            case SORT_DESCENDING:   return !operator[](_col).less(o[_col], _dataType, sequNr < o.sequNr);
            default:
                return sequNr < o.sequNr;
        }
    }
    
    
    // constructor adds widget to main window
    /// @param lR left positioning rule
    /// @param lO offset for left pos
    /// @param tR top positioning rule
    /// @param tO offset for top pos
    /// @param rR right positioning rule
    /// @param rO offset for right pos
    /// @param bR bottom positioning rule
    /// @param bO offset for bottom pos
    ListBox::ListBox (const std::string& _caption,
                      MainWnd& main,
                      WidgetPosE lR, int lO,
                      WidgetPosE tR, int tO,
                      WidgetPosE rR, int rO,
                      WidgetPosE bR, int bO) :
    Widget(_caption, main, lR, lO, tR, tO, rR, rO, bR, bO)
    {}
    
    // Adds or overwrites a column definition
    void ListBox::AddCol (unsigned colId, ListColumnDef&& def)
    {
        // extend vector to needed size
        if (colId >= cols.size())
            cols.resize(colId+1);
        
        // set the element as passed in
        // (if this throws an exception then the code above for
        //  vector resizing is wrong, not supposed to happen!)
        cols.at(colId) = std::move(def);
    }
    
    
    // Define sorting, will immediately resort the available data
    void ListBox::SetSorting (unsigned _col, SortingTy _sorted)
    {
        // Special case unsupported
        if (_sorted == SORT_UNSUPPORTED)
            _col = 0;
        
        // short-cut for "no change"
        if (sortedBy == _col && sorted == _sorted)
            return;
        
        // check _col is valid
        if (_col >= cols.size() ||
            cols.at(_col).dataType == DTY_NULL)
            return;
        
        // save new values and re-sort the rows
        sortedBy = _col;
        sorted = _sorted;
        DoSort();
    }
    
    
    // set geometry of list box
    /// Recalculates number of rows shown and
    /// ensures that the selected row stays visible
    void ListBox::SetGeometry (const Rect& _r)
    {
        // pass up in class hierarchy
        Widget::SetGeometry(_r);
        
        // line height is font height...
        int fontHeight = 0;
        XPLMGetFontDimensions(xplmFont_Proportional, nullptr, &fontHeight, nullptr);
        // ...multiplied by the factor
        lnHeight = (unsigned(fontHeight) * lnHeightFactor)/100;
        
        // how many rows can we draw with this size?
        rowMax = unsigned(_r.Height()) / lnHeight;
        // one row is for the column headers, remove it from calculations
        if (rowMax > 0) --rowMax;
        
        // Maybe we got resized since last drawing, so let's check if we need
        // to adapt scroll position
        if (rows.size() <= rowMax) {
            rowFirst = rowFirstMaxDown = 0;           // no issue, all rows fit
        }
        else {
            // what would be the first row to show when fully scrolled down?
            rowFirstMaxDown = unsigned(rows.size()) - rowMax;
            if (rowFirst > rowFirstMaxDown)
                rowFirst = rowFirstMaxDown;
        }
        
        // we might need to crop strings in the last visible column
        DoCropTexts();
        
        // Ensure that the selected row, if any, is scrolled into visibility
        MakeSelVisible();
    }
    
    
    // Add an actual row to the list box, which is to be displayed
    ListRow& ListBox::AddRow (ListRow&& row)
    {
        // first we check for duplicates
        ListRowVecTy::iterator iter;
        if (FindRowIdx(row.refCon, iter)) {
            // found, so overwrite existing data, but re-use same `sequNr`
            row.SetSequNr(iter->GetSequNr());
            *iter = std::move(row);
        } else {
            // not found, new data to be added:
            // set the always increasing sequence number to this row
            row.SetSequNr(++sequRow);
            // add the row to the end of the row vector
            rows.emplace_back(std::move(row));
            iter = std::prev(rows.end());
        }
        // if sorting is other than none: re-sort
        if (sorted > SORT_UNSORTED)
            DoSort();
        // return reference to the row
        return *iter;
    }
    
    // Return row by index (which is unsafe as sorting can change the order)
    ListRow* ListBox::GetRow (size_t index)
    {
        if (index < rows.size())
            return &rows.at(index);
        else
            return nullptr;
    }
    
    // Return row by index (which is unsafe as sorting can change the order)
    const ListRow* ListBox::GetRow (size_t index) const
    {
        if (index < rows.size())
            return &rows.at(index);
        else
            return nullptr;
    }
    
    // Remove a row, identified by ListRow::refCon
    bool ListBox::RemoveRow (void* _refCon)
    {
        ListRowVecTy::const_iterator iter;
        if (FindRowIdx(_refCon, iter))
        {
            // if it is the selected row, then no row is selected now
            if (selRefCon == iter->refCon)
                selRefCon = nullptr;
            
            // remove the row
            rows.erase(iter);
            return true;
        }
        // not found
        return false;
    }
    
    
    // Clear a list: Remove all rows
    void ListBox::Clear ()
    {
        selRefCon = nullptr;            // no selection any longer
        rows.clear();                   // remove all rows
    }
    
    
    // Set selection, returns if the row is available
    bool ListBox::SelectRow (void* _refCon)
    {
        ListRowVecTy::const_iterator iter;
        if (FindRowIdx(_refCon, iter))
        {
            selRefCon = _refCon;
            return true;
        }
        selRefCon = nullptr;
        return false;
    }
    
    
    // find a row by refCon (const version)
    const ListRow* ListBox::FindRow (void* _refCon) const
    {
        // shortcut
        if (!_refCon)
            return nullptr;
        // loop over rows and compare against passed in _refCon
        ListRowVecTy::const_iterator iter;
        if (FindRowIdx(_refCon, iter))
            return &(*iter);
        // not found
        return nullptr;
    }
    
    // find a row by refCon (non-const version)
    ListRow* ListBox::FindRow (void* _refCon)
    {
        // shortcut
        if (!_refCon)
            return nullptr;
        // loop over rows and compare against passed in _refCon
        ListRowVecTy::iterator iter;
        if (FindRowIdx(_refCon, iter))
            return &(*iter);
        // not found
        return nullptr;
    }

    
    // Crops texts of header and content to match geometry
    void ListBox::DoCropTexts ()
    {
        // Width to calculate with
        int width = Width();
        // reserve space for scrollbar if needed
        if (rows.size() > rowMax)
            width -= gCHAR_AVG_WIDTH + 1;

        
        // current column's starting position
        size_t x = 0;
        
        // loop over all columns
        for (unsigned colId = 0;
             colId < cols.size();
             colId++)
        {
            ListColumnDef& def = cols[colId];
            // visible width of the column
            const size_t w =
                !def.bVisible ? 0 :
                int(x) >= width ? 0 :
                std::min(size_t(width) - x, (size_t)def.width);
            
            // set header text's cropped width
            def.name.SetWidth(w);
            
            // also crop all data elements to that width
            for (ListRow& row: rows)
                row[colId].SetWidth(w);
            
            // increment starting pos of next column
            x += w;
        }
    }

    
    // Called by MainWnd, this function does the drawing of the list
    void ListBox::DoDraw (const Rect& r)
    {
        // draw the background
        Widget::DoDraw(r);
        
        // get the correct foreground color
        float txtColor[4];
        TranslateColor(colFg, txtColor);

        // represents the area of one row, starting with the header
        Rect rowRect (r.Tl(),
                      {r.Right(), r.Top()-int(lnHeight)});
        // row doesn't fit into widget's area?
        if (rowRect.Bottom() < r.Bottom())
            return;

        // Draw column headers first
        DoDrawHeader(rowRect, GetGlobalColor(COL_COLUMN_HEADER));

        // reserve space for scrollbar if needed
        if (rows.size() > rowMax)
            rowRect.SetRight(rowRect.Right() - gCHAR_AVG_WIDTH - 1);
        
        // move on to next row
        rowRect += Point(0,-int(lnHeight));
        
        // Draw each item of each row of the list
        bool bAlternateRow = false;         // every other row gets lighter background
        int rowsToSkip = (int)rowFirst;     // number of rows to skip due to scrolling
        for (const ListRow& row: rows) {
            // Skip those rows which are scrolled away at beginning
            if (--rowsToSkip >= 0)
                continue;
            
            // Don't draw beyond our widget space
            if (rowRect.Bottom() < r.Bottom())
                break;
            
            // if this is the selected row then draw a highlite background
            if (selRefCon == row.refCon) {
                SetColor(gCOL_STD[COL_SELECTION]);
                DrawRect(rowRect);
                // if we also have focus add a darker frame around the selection
                if (HasFocus()) {
                    SetColor(gCOL_STD[COL_FOCUS_BORDER]);
                    DrawFrame(rowRect);
                }
            }
            // for alternating row make background a bit lighter
            else if (bAlternateRow) {
                SetColor(COL_WHITE_LIGHTTRANS);
                DrawRect(rowRect);
            }
            bAlternateRow = !bAlternateRow;
            
            // draw the content of the row
            DoDrawRow(rowRect, row, txtColor);
            
            // move on to next row
            rowRect += Point(0,-int(lnHeight));
        }
        
        // draw scrollbar if needed
        if (rows.size() > rowMax)
            DoDrawVertScrollbar(r);
    }

    
    // Draw column headers.
    void ListBox::DoDrawHeader (const Rect& rowRect, const float txtColor[4])
    {
        std::string buf;
        Rect cellRect(rowRect);
        DataTypeTy prevDataType = DTY_NULL;         // data type of previous column
        unsigned colId = 0;
        for (ListColumnDef& def: cols) {
            // completely skip invisible columns
            if (!def.bVisible) {
                colId++;
                continue;
            }
            
            // the cell of this column header
            cellRect.SetRight(cellRect.Left() + def.width);
            // don't draw beyond limits
            if (cellRect.Left() >= rowRect.Right())
                break;
            // clip rectangle if not enough space for entire column
            if (cellRect.Right() > rowRect.Right())
                cellRect.SetRight(rowRect.Right());
            
            // if a left-aligned column directly follows a right-aligned column we need some space inbetween
            if (def.dataType == DTY_STRING && prevDataType >= DTY_INT)
                cellRect.SetLeft(cellRect.Left() + gCHAR_AVG_WIDTH);
            
            // for the sorted column we also print ^ or v
            if (colId == sortedBy && sorted > SORT_UNSORTED) {
                buf = def.name.full();
                def.name += (sorted == SORT_DESCENDING ? " v" : " ^");
            }
            
            // draw column header
            DrawAlignedString(const_cast<float*>(txtColor), cellRect,
                              def.dataType < DTY_CHECKBOX ? TXA_LEFT :
                              def.dataType > DTY_CHECKBOX ? TXA_RIGHT : TXA_CENTER,
                              def.name.cropped(), xplmFont_Proportional);
//            // once again, one point to the right, to create something like "bold" text
//            cellRect += Point(1,0);
//            DrawAlignedString(txtColor, cellRect,
//                              def.dataType < DTY_CHECKBOX ? TXA_LEFT :
//                              def.dataType > DTY_CHECKBOX ? TXA_RIGHT : TXA_CENTER,
//                              def.name.cropped(), xplmFont_Proportional);

            // for the sorted column we restore previous text
            if (colId == sortedBy && sorted > SORT_UNSORTED) {
                def.name = std::move(buf);
            }
            
            // if a left-aligned column directly follows a right-aligned column we need some space inbetween
            if (def.dataType == DTY_STRING && prevDataType >= DTY_INT)
                cellRect.SetLeft(cellRect.Left() - gCHAR_AVG_WIDTH);

            // move cell rect's left border to next column, thereby removing the 1 point move for "bold" drawing
            cellRect += Point(def.width-1);
            prevDataType = def.dataType;
            colId++;
        }

        // Draw line under header text as divider
        SetColor(gCOL_STD[COL_LINE]);
        DrawLine({rowRect.Bl(), rowRect.Br()});
    }

    
    // Draw the text items of a row
    void ListBox::DoDrawRow (const Rect& rowRect, const ListRow& row, const float txtColor[4])
    {
        // size of a cell (space per item in a column)
        Rect cell = rowRect;
        DataTypeTy prevDataType = DTY_NULL;         // data type of previous column

        // loop over all columns
        for (unsigned colId = 0;
             colId < cols.size();
             colId++)
        {
            const ListColumnDef& def = cols[colId];
            if (def.bVisible) {
                // set cell's width to match column width
                cell.SetTr({cell.Left() + def.width, cell.Top()});
                
                // don't draw beyond limits
                if (cell.Left() >= rowRect.Right())
                    break;
                // clip rectangle if not enough space for entire column
                if (cell.Right() > rowRect.Right())
                    cell.SetRight(rowRect.Right());

                // if a left-aligned column directly follows a right-aligned column we need some space inbetween
                if (def.dataType == DTY_STRING && prevDataType >= DTY_INT)
                    cell.SetLeft(cell.Left() + gCHAR_AVG_WIDTH);
                
                // draw row item's text
                if (colId < row.size()) {
                    // Text color overriden in ListItem?
                    const ListItem& item = row[colId];
                    const float* pCol = std::isnan(item.col[0]) ? txtColor : item.col;
                    if (def.dataType != DTY_CHECKBOX) {
                        DrawAlignedString(const_cast<float*>(pCol), cell,
                                          def.dataType >= DTY_INT ? TXA_RIGHT : TXA_LEFT,
                                          item.GetCroppedS(), xplmFont_Proportional);
                    } else {
                        // Checkbox type is somewhat special
                        Rect cb = MakeCheckboxSquare(cell);
                        cb.Expand(-3);
                        // Center the checkbox in the item cell
                        cb += Point((cell.Width() - cb.Width())/2, 0);
                        // Draw in proper color, potentially "disabled" color
                        DrawSetColor(item.IsEnabled() ? pCol : GetGlobalColor(COL_STD_FG_DISABLED));
                        DrawCheckBox(cb, item.IsSet(), false);
                    }
                }

                // if a left-aligned column directly follows a right-aligned column we need some space inbetween
                if (def.dataType == DTY_STRING && prevDataType >= DTY_INT)
                    cell.SetLeft(cell.Left() - gCHAR_AVG_WIDTH);
                
                // move cell rect to next column's beginning
                cell += Point(def.width,0);
                prevDataType = def.dataType;
            }
        }
    }
    
    
    // Draw a list box's vertical scrollbar
    void ListBox::DoDrawVertScrollbar (const Rect& _r)
    {
        // Scrollbar is on right hand side, one character spacing thick
        Rect sbRect(_r);
        sbRect.Left() = sbRect.Right() - gCHAR_AVG_WIDTH;
        sbRect.Top() -= int(lnHeight);
        
        // background
        SetColor(gCOL_STD[COL_SCROLLBAR_BG]);
        DrawRect(sbRect);
        
        // thumb, depends on visible portion of list items
        const int height = sbRect.Height();
        const int top = _r.Top();
        sbThumbTop      = - int(lnHeight) - (height * int(rowFirst       )) / int(rows.size());
        sbThumbBottom   = - int(lnHeight) - (height * int(rowFirst+rowMax)) / int(rows.size());
        sbRect.Top()   = top + sbThumbTop;
        sbRect.SetBottom(top + sbThumbBottom);
        
        // reduce size of all edges by 1 to leave a tiny frame around
        // where the background stays visible
        sbRect.Expand(-1);
        
        SetColor(gCOL_STD[COL_SCROLLBAR_FG]);
        DrawRect(sbRect);
    }
    

    // Re-sort the existing list of rows according to sorting criteria
    void ListBox::DoSort ()
    {
        // sanity check that column-to-sort-by exists
        if (sortedBy >= cols.size()) return;
        const DataTypeTy _dataType = cols[sortedBy].dataType;

        // re-sort the list of rows
        std::sort(rows.begin(), rows.end(),
                  [this,_dataType](const ListRow& r1, const ListRow& r2)->bool
                  { return r1.elemLess(r2, sortedBy, sorted, _dataType); });
        
        // Make selected row still visible
        MakeSelVisible();
    }
    
    
    // define the new selected row and inform the main window
    void ListBox::DoSelectRow (ListRow& _selRow)
    {
        selRefCon = _selRow.refCon;
        mainWnd.MsgListSelChanged(*this, _selRow);

    }

    
    // Find vector index by refCon
    bool ListBox::FindRowIdx (void* _refCon,
                              ListRowVecTy::const_iterator& iter) const
    {
        // search the vector by refCon
        iter = std::find_if (rows.cbegin(), rows.cend(),
                             [_refCon](const ListRow& r)
                             { return r.refCon == _refCon; });
        // found something?
        return iter != rows.cend();
    }
    
    // Find vector index by refCon
    bool ListBox::FindRowIdx (void* _refCon,
                              ListRowVecTy::iterator& iter)
    {
        // search the vector by refCon
        iter = std::find_if (rows.begin(), rows.end(),
                             [_refCon](const ListRow& r)
                             { return r.refCon == _refCon; });
        // found something?
        return iter != rows.end();
    }
    
    
    // Find row by y coordinate, relative to top of list box
    bool ListBox::FindRowIdx (int _y, ListRowVecTy::iterator& iter)
    {
        // _y needs to be negative: it's relative from top, and downward is negative
        if (_y > 0) { iter = rows.end(); return false; }
        
        // which visible row did we hit?
        unsigned rowHit = unsigned(-_y) / lnHeight;
        
        // Hit the header row?
        if (rowHit == 0) {
            iter = rows.end();
            return false;
        }

        // remove the header from calculation
        rowHit--;
        // add the lines hidden due to scrolling into the calculation
        rowHit += rowFirst;
        
        // If this now is in range of number of rows we have, then we found it
        if (rowHit < rows.size()) {
            iter = rows.begin();
            std::advance(iter, rowHit);
            return true;
        }

        // clicked after all rows
        iter = rows.end();
        return false;
    }


    // Find column by x coordinate, relative to top of list box
    unsigned ListBox::FindColIdx (int _x)
    {
        if (cols.empty())               // sanity check
            return 0;
        
        int right = 0;                  // will become the columns right x coordinate
        size_t lastVisibleCol = 0;
        for (size_t col = 0;
             col < cols.size();
             col++)
        {
            if (cols.at(col).bVisible) {
                right += cols.at(lastVisibleCol=col).width;
                if (_x <= right)        // if x is now left of right border return this column
                    return (unsigned)col;
            }
        }
        // default: return last visible column
        return (unsigned)lastVisibleCol;
    }
    
    
    // Ensure that the selected row, if any, is scrolled into visibility
    void ListBox::MakeSelVisible ()
    {
        if (selRefCon) {
            ListRowVecTy::const_iterator iter;
            if (FindRowIdx(selRefCon, iter)) {
                const unsigned rowSel = (unsigned)std::distance(rows.cbegin(), iter);
                if (rowSel < rowFirst)          // selected before first visible row
                    rowFirst = rowSel;
                    else if (rowSel >= rowFirst + rowMax) // selected past last visible row
                        rowFirst = rowSel - rowMax + 1;
            }
        }
    }
    
    
    // Change selected row upon mouse click
    void ListBox::DoMouseClick (Point p, MouseBtnTy mouseBtn, XPLMMouseStatus inMouse)
    {
        // we only handle the left-button-mouse-down-event
        if (mouseBtn != MOUSE_LEFT || inMouse != xplm_MouseDown)
            return;
        
        // Into which column was the click?
        unsigned col = FindColIdx(p.x());
        
        // clicked into the header?
        if (p.y() >= -int(lnHeight)) {
            if (sorted != SORT_UNSUPPORTED) {
                // Sort by the column we hit
                SortingTy direction = SORT_ASCENDING;
                if (col == sortedBy) {          // clicked the currently sorted column again?
                    switch(sorted) {            // -> cycle the sorting direction
                        case SORT_UNSUPPORTED:  direction = SORT_UNSUPPORTED;   break;
                        case SORT_UNSORTED:     direction = SORT_ASCENDING;     break;
                        case SORT_ASCENDING:    direction = SORT_DESCENDING;    break;
                        case SORT_DESCENDING:   direction = SORT_UNSORTED;      break;
                    }
                }
                SetSorting(col, direction);
            }
            return;
        }
        
        // if there is a scrollbar then we need to check scrollbar clicks, too
        if (rows.size() > rowMax && p.x() >= Width() - gCHAR_AVG_WIDTH)
        {
            if (p.y() >= sbThumbTop)            // scroll one page up
                DoMouseWheel(p, 0, rowMax > 1 ? -int(rowMax)+1 : -1);
            else if (p.y() <= sbThumbBottom)    // scroll one page down
                DoMouseWheel(p, 0, rowMax > 1 ? int(rowMax)-1  :  1);
            // TODO: Handle dragging of thumb
            return;
        }
        
        // find the affected row of the list box
        ListRowVecTy::iterator iter;
        if (FindRowIdx(p.y(), iter)) {
            // if we hit a Checkbox column in that row
            if (col < cols.size() && cols.at(col).dataType == DTY_CHECKBOX &&
                col < iter->size())
            {
                // toggle the "checked" status if that item is enabled
                ListItem& li = iter->at(col);
                if (li.IsEnabled())
                    li.Set(!li.IsSet());
            }
            // Select the row
            DoSelectRow(*iter);
        }
    }
    
    
    // Handle cursor keys to change selection and scroll list
    void ListBox::DoHandleKey (char /*inKey*/, XPLMKeyFlags inFlags, unsigned char inVirtualKey)
    {
        // with no rows I can't do much
        if (rows.empty()) return;
        
        // currently selected item
        ListRowVecTy::iterator iter = rows.end();
        if (selRefCon)
            FindRowIdx(selRefCon, iter);
        
        // select next line?
        if ( inVirtualKey == XPLM_VK_DOWN && inFlags == xplm_DownFlag ) {
            if (iter == rows.end())             // none selected yet
                iter = rows.begin();            // -> select first row
            else if (std::next(iter) != rows.cend())
                ++iter;                         // next, if there is a next
        }

        // select previous line?
        else if ( inVirtualKey == XPLM_VK_UP && inFlags == xplm_DownFlag ) {
            if (iter == rows.end())             // none selected yet
                iter = rows.begin();            // -> select first row
            else if (iter != rows.cbegin())
                --iter;                         // previous, if there is a next
        }
        
        // select the selected row
        if (iter != rows.end())
            DoSelectRow(*iter);
        else
            selRefCon = nullptr;
        MakeSelVisible();
    }
    
    
    // Scroll up/down
    void ListBox::DoMouseWheel (Point /*p*/, int wheel, int clicks)
    {
        // we only handle the vertical wheel axis yet
        if (wheel != 0) return;
        
        // adjust first shown row accordingly
        int fr = int(rowFirst) + clicks;
        if (fr < 0)
            rowFirst = 0;
        else if (fr > int(rowFirstMaxDown))
            rowFirst = rowFirstMaxDown;
        else
            rowFirst = unsigned(fr);
    }

    //
    // Main window, this is the one communicating with X-Plane
    //
    
    // Actually creates the window (hidden)
    MainWnd::MainWnd (const std::string& _caption,
                      int _left, int _top, int _right, int _bottom,
                      XPLMWindowDecoration _deco, XPLMWindowLayer _layer,
                      int _frame) :
    Widget(_caption, *this,
           POS_ABSOLUTE, _left,  POS_ABSOLUTE, _top,
           POS_ABSOLUTE, _right, POS_ABSOLUTE, _bottom),
    frameThick(_frame)
    {
        // Widget doesn't set the rectangle coordinates (but would rely on Layout()).
        // As this is a main window it won't get layouted, so set Rect manually
        Left() = _left;
        Top() = _top;
        Right() = _right;
        Bottom() = _bottom;
        
        // prepare a windows creation structure
        XPLMCreateWindow_t wndDef = {
            sizeof(XPLMCreateWindow_t),
            _left, _top, _right, _bottom,
            0,                  // not immediately visible
            CBDraw,
            CBMouseClickLeft, CBKey, CBCursor, CBMouseWheel,
            this,               // refcon
            _deco, _layer,
            CBMouseClickRight
        };
        
        // create the window
        wnd = XPLMCreateWindowEx(&wndDef);
        assert(wnd != NULL);
        
        SetCaption(_caption);
    }
    
    
    // destructor destroys the window
    MainWnd::~MainWnd ()
    {
        // stop the timer
        SetTimer(0.0f);
        
        // remove the window also from XP
        if (wnd)
            XPLMDestroyWindow(wnd);
        wnd = NULL;
    }
    
    // Set the window title
    void MainWnd::SetCaption (const std::string& _c)
    {
        // call base class first
        Widget::SetCaption(_c);
        // set the window's title
        XPLMSetWindowTitle(wnd, caption.full_str());
    }

    // Center the window on the main screen, then switch to floating type
    void MainWnd::SetCenterFloat (bool _bVisible, int _monitorIdx)
    {
        // Let XPlane figure out the main window and the position there
        SetPositioningMode(xplm_WindowCenterOnMonitor, _monitorIdx);
        // now fetch the current coordinates
        FetchGeometry();
        // Set floating mode in next drawing callback
        resetPosModeTo = xplm_WindowPositionFree;
        // Finally set visibility
        SetVisible(_bVisible);
    }

    // Move the window to the same position (top/left corner) as the window passed in
    void MainWnd::SetPosAs (const MainWnd& _o)
    {
        // Positioning mode
        const XPLMWindowPositioningMode oMode = _o.GetPositioningMode();
        SetPositioningMode(oMode,-1);
        
        // Geometry depends on positioning mode
        int l, t, w, h;
        switch (oMode) {
            case xplm_WindowPopOut:
                // find out my width/height in OS coordinates
                XPLMGetWindowGeometryOS(*this, &l, &t, &w, &h);
                w -= l;
                h = t - h;
                
                // Left/Top of other window:
                XPLMGetWindowGeometryOS(_o, &l, &t, nullptr, nullptr);
                
                // Position myself in OS coordinates, using my width/height:
                XPLMSetWindowGeometryOS(*this, l, t, l+w, t-h);
                
                break;
                
            case xplm_WindowVR:
                // in VR we could only set width/height...which we don't do here
                break;
                
            default:
                XPLMGetWindowGeometry(_o, &l, &t, nullptr, nullptr);
                XPLMSetWindowGeometry(*this, l, t, l+Width(), t-Height());
        }
    }

    // Tries to figure out the positioning mode
    /// There is no direct call in the XP SDK to figure out the positioning mode,
    /// but only function to check for popped out or VR
    XPLMWindowPositioningMode MainWnd::GetPositioningMode () const
    {
        if (XPLMWindowIsPoppedOut(*this))
            return xplm_WindowPopOut;
        else if (XPLMWindowIsInVR(*this))
            return xplm_WindowVR;
        else
            return xplm_WindowPositionFree;
    }
    
    // Start a timer, which calls DoTimer() repeatedly
    /// @see https://developer.x-plane.com/sdk/XPLMProcessing/#XPLMRegisterFlightLoopCallback
    void MainWnd::SetTimer (float _interval)
    {
        // set the timer the first time
        if (timerInterval <= 0.01f && _interval > 0.01f)
            XPLMRegisterFlightLoopCallback(CBTimer, _interval, this);
        // change the timer
        else if (timerInterval > 0.01f && _interval > 0.01f)
            XPLMSetFlightLoopCallbackInterval(CBTimer, _interval, 0, this);
        // cancel an active timer
        else if (timerInterval > 0.01f && _interval <= 0.01f)
            XPLMUnregisterFlightLoopCallback(CBTimer, this);
        
        // save current setting
        timerInterval = _interval;
    }
    
    
    // Add a widget to the window
    void MainWnd::Add (WidgetAddTy&& _wa)
    {
        _wa.w.widgetId = ++lastWidgetId;    // assign unique id
        widgets.emplace_back(_wa);          // add to our list of widget
        bLayoutDirty = true;                // will need to recalc layout
    }
    
    // based on the positioning rules recalculates the current window's layout
    /// The widgets' coordinates are recalculated with the top left corner
    /// being (0|0), ie. we don't care about the main window's current
    /// position. That offset will be added during drawing only.
    void MainWnd::Layout ()
    {
        // the order is the order in the deque
        const WidgetAddTy* prevWa = nullptr;
        for (WidgetAddTy& wa: widgets)
        {
            int _left = 0, _top = 0, _right = 0, _bottom = 0;
            
            // window are in use - deduct frame
            const int w = Width() - 2 * frameThick;
            const int h = Height() - 2 * frameThick;
            
            // left: calculation basis
            _left = wa.lO;              // add positioning offset
            switch (wa.lR) {
                case POS_ABSOLUTE:      break;
                case POS_BOTTOM_RIGHT:  _left += w; break;
                case POS_AFTER_PRV:     _left += prevWa ? prevWa->w.Right() : 0; break;
                case POS_SAME_PRV:      _left += prevWa ? prevWa->w.Left() : 0; break;
                case POS_FIXED_LEN:     break;   // we know _right only later
                case POS_PERCENT:       _left = w * wa.lO / 100; break;     // override
            }
            
            // top: calculation basis
            _top = wa.tO;               // add positioning offset
            switch (wa.tR) {
                case POS_ABSOLUTE:      break;
                case POS_BOTTOM_RIGHT:  _top -= h; break;
                case POS_AFTER_PRV:     _top += prevWa ? prevWa->w.Bottom() : 0; break;
                case POS_SAME_PRV:      _top += prevWa ? prevWa->w.Top() : 0; break;
                case POS_FIXED_LEN:     break;    // we know _bottom only later
                case POS_PERCENT:       _top = -h * wa.tO / 100; break;     // override
            }
            
            // right: calculation basis
            _right = wa.rO;             // add positioning offset
            switch (wa.rR) {
                case POS_ABSOLUTE:      break;
                case POS_BOTTOM_RIGHT:  _right += w; break;
                case POS_AFTER_PRV:
                case POS_SAME_PRV:      _right += prevWa ? prevWa->w.Right() : 0; break;
                case POS_FIXED_LEN:     _right += _left; break;
                case POS_PERCENT:       _right = w * wa.rO / 100; break;     // override
            }
            
            // bottom: calculation basis
            _bottom = wa.bO;            // add positioning offset
            switch (wa.bR) {
                case POS_ABSOLUTE:      break;
                case POS_BOTTOM_RIGHT:  _bottom -= h; break;
                case POS_AFTER_PRV:
                case POS_SAME_PRV:      _bottom += prevWa ? prevWa->w.Bottom() : 0; break;
                case POS_FIXED_LEN:     _bottom = _top - wa.bO; break;
                case POS_PERCENT:       _bottom = -h * wa.bO / 100; break;     // override
            }
            
            // if left/top are fixed offset to right/bottom then we know them now only
            if (wa.lR == POS_FIXED_LEN)  _left = _right     - wa.lO;
            if (wa.tR == POS_FIXED_LEN)  _top  = _bottom    + wa.tO;
            
            // result might be invalid, e.g. if window is too small
            if (_left > _right || _bottom > _top)
                wa.w.SetGeometry({-1,-1,-1,-1});
            else
                // reposition the widget
                wa.w.SetGeometry({_left, _top, _right, _bottom});
            
            // remember this one as the `previous`
            prevWa = &wa;
        }
        
        bLayoutDirty = false;
    }
    
    // functions performing callback functionality
    
    // @see https://developer.x-plane.com/sdk/XPLMDrawWindow_f/
    void MainWnd::DoDraw (const Rect& /*r*/)
    {
        // update my own geometry before painting refers to it
        const Rect oldGeo = *this;
        FetchGeometry();
        if (oldGeo != *this)
            bLayoutDirty = true;
        
        // if needed recalc the layout
        if (bLayoutDirty)
            Layout();
        
        // apparently I'm visible, otherwise I wouldn't get called
        bVisible = true;
        
        // if requested now that we know coordinates set a (different) positioning mode
        if (resetPosModeTo >= 0) {
            SetPositioningMode(resetPosModeTo, -1);
            resetPosModeTo = -1;
        }
        
        // define graphic state for all drawing once
        XPLMSetGraphicsState(0,             // fog
                             0,             // number tex units
                             0,             // lighting
                             0,             // alpha test
                             1,             // alpha blend
                             0,             // depth test
                             0);            // depth write
        
        // draw my own background
        DrawBackground(*this);
        
        // Offset all drawing starts from
        const Point Ofs = Tl() + Point(frameThick,-frameThick);
        // draw all widgets
        for (WidgetAddTy& wa: widgets) {
            if (wa.w.IsVisible()) {                 // is widget visible?
                const Rect drawRect (wa.w + Ofs);   // actual coordinate to draw
                if (Contains(drawRect))             // and enough space to draw complete widget?
                    wa.w.DoDraw(drawRect);
            }
        }
    }
    
    // @see https://developer.x-plane.com/sdk/XPLMHandleMouseClick_f/
    void MainWnd::DoMouseClick (Point p, MouseBtnTy mouseBtn, XPLMMouseStatus inMouse)
    {
        // this window got clicked, so it will also take keyboard focus
        TakeKeyboardFocus();
        
        // Search for the affected widget only in case of xplm_MouseDown.
        // All future events must go to the same widget, no matter where the mouse is
        static WidgetAddTy* pMouseFocusWidget = nullptr;
        if (inMouse == xplm_MouseDown || !pMouseFocusWidget) {
            // find the affected widget and inform it
            WidgetAddDequeTy::iterator iter;
            if (FindWidgetByPos(p, iter)) {         // this also "localizes" p in relation to found widget
                // the widget also gets focus if it wants it
                pMouseFocusWidget = &(*iter);
                if (!pMouseFocusWidget->w.HasFocus() &&
                     pMouseFocusWidget->w.DoFocusChange(FCS_FIRST)) {
                    // take it away from previous focus widget if different
                    if (pFocusWidget && pFocusWidget != pMouseFocusWidget)
                        pFocusWidget->w.DoFocusChange(FCS_LOOSE);
                    // assign it to found widget
                    pFocusWidget = pMouseFocusWidget;
                } // wants focus
            } // found a widget
        } // mouseDown event
        // other than mouse up, and we know the widget to send it to
        else {
            // still need to "localize" the coordinates
            LocalizePoint(p, *pMouseFocusWidget);
        }

        // now perform the click in the widget
        if (pMouseFocusWidget)
            pMouseFocusWidget->w.DoMouseClick(p, mouseBtn, inMouse);
        
        // in case of mouse up: clear the focus widget
        if (inMouse == xplm_MouseUp)
            pMouseFocusWidget = nullptr;
    }
    
    // @see https://developer.x-plane.com/sdk/XPLMHandleMouseWheel_f/
    void MainWnd::DoMouseWheel (Point p, int wheel, int clicks)
    {
        // find the affected widget and inform it
        WidgetAddDequeTy::iterator iter;
        if (FindWidgetByPos(p, iter))
            iter->w.DoMouseWheel(p, wheel, clicks);
    }
    
    // @see https://developer.x-plane.com/sdk/XPLMHandleKey_f/
    void MainWnd::DoHandleKey (char inKey, XPLMKeyFlags inFlags, unsigned char inVirtualKey)
    {
        // Handle the [Tab] key for focus change between our widgets
        if (inVirtualKey == XPLM_VK_TAB) {
            // pressing TAB without any modifiers -> forward
            if (inFlags == xplm_DownFlag)
                DoFocusNextWidget(widgets.begin(), widgets.end(), FCS_NEXT);
            // pressing TAB with Shift modifier -> backward
            else if (inFlags == (xplm_DownFlag | xplm_ShiftFlag))
                DoFocusNextWidget(widgets.rbegin(), widgets.rend(), FCS_PREV);
        }
        // Handle the [Esc] key for losing widget focus
        else if (inVirtualKey == XPLM_VK_ESCAPE) {
            if (pFocusWidget && inFlags == xplm_DownFlag)
                pFocusWidget->w.DoFocusChange(FCS_LOOSE);
            pFocusWidget = nullptr;
        }
        // Handle the [Enter] key for applying default push button
        else if (inVirtualKey == XPLM_VK_RETURN || inVirtualKey == XPLM_VK_ENTER) {
            if (pDefBtn && inFlags == xplm_DownFlag && pDefBtn->IsEnabled())
                MsgButtonClicked(*pDefBtn);
        }
        // forward all other keys to the focus widget
        else if (pFocusWidget) {
            pFocusWidget->w.DoHandleKey(inKey, inFlags, inVirtualKey);
        }
    }
    
    // show/hide the window
    void MainWnd::SetVisible (bool _bVisible)
    {
        if (!_bVisible)
            DoFocusChange(FCS_LOOSE);
        XPLMSetWindowIsVisible ( wnd, bVisible = _bVisible );
        TakeKeyboardFocus(_bVisible);
    }
    
    // @see https://developer.x-plane.com/sdk/XPLMHandleKey_f/
    bool MainWnd::DoFocusChange (FocusChangeTy _fcs)
    {
        // Being called means that the entire main window lost focus.
        // TODO: Find a way of calling this from outside when main window gains focus
        if (_fcs == FCS_LOOSE && pFocusWidget)
            // inform focus widget that we (all) just lost it.
            // (But don't nullify the pointer: When main window gets focus back
            //  we continue with the same focus widget.)
            pFocusWidget->w.DoFocusChange(FCS_LOOSE);
        
        // processed
        return true;
    }
    
    // @see https://developer.x-plane.com/sdk/XPLMHandleCursor_f/
    XPLMCursorStatus MainWnd::DoHandleCursor (Point p)
    {
        // find the affected widget and inform it
        WidgetAddDequeTy::iterator iter;
        if (FindWidgetByPos(p, iter))
            return iter->w.DoHandleCursor(p);
        
        // just return the default
        return xplm_CursorDefault;
    }
    
    
    // Finds the next widget to receive focus, either forward or backward
    template< class InputIt >
    void MainWnd::DoFocusNextWidget (InputIt _first, InputIt _last, FocusChangeTy _fcs)
    {
        InputIt iter = _last;
        // if there is a current focus widget
        if (pFocusWidget) {
            // If the focus widget is now invisible or deactivate it will loose focus
            if (!pFocusWidget->w.IsVisible() ||
                !pFocusWidget->w.IsEnabled())
                pFocusWidget->w.DoFocusChange(FCS_LOOSE);
            // Still visible/enabled: Let's see if the widget handels multiple focus fields internally (like a button array)
            else if (pFocusWidget->w.DoFocusChange(_fcs))
                // it keeps the focus, so we are done
                return;

            // That widget returned or lost focus,
            // so find the deque position of it
            iter = std::find_if(_first, _last,
                                [this](const WidgetAddTy& wa)
                                { return pFocusWidget == &wa; });
            // if we found the current focus widget (we should!)
            // then start looking for the next widget at the next position:
            if (iter != _last)
                ++iter;
            
            // right now, there is no focus widget
            pFocusWidget = nullptr;
        }
        
        // we do 2 loops max, so we get one chance for round-robin start over
        for (int loop = 0; loop < 2; loop++)
        {
            while (iter != _last) {
                // Will this widget accept input focus?
                WidgetAddTy& wa = *iter;
                if (wa.w.IsVisible() &&
                    wa.w.IsEnabled() &&
                    wa.w.DoFocusChange(_fcs == FCS_NEXT ? FCS_FIRST : FCS_LAST)) {
                    // yea, then we are done
                    pFocusWidget = &wa;
                    return;
                }
                // try next widget
                ++iter;
            }
            
            // start over from the beginning
            iter = _first;
        }
    }
    
    
    // Find the widget, which contains given global p
    bool MainWnd::FindWidgetByPos (Point& _p, WidgetAddDequeTy::iterator& _iter,
                                   bool _bVisibleActiveOnly)
    {
        // p is global, make it relative to main window
        _p -= Tl();
        // then take the frame out of consideration
        _p.x() -= frameThick;
        _p.y() += frameThick;
        
        // walk the widgets and find one which contains the point
        // (and is visible and active, if so requested)
        _iter = std::find_if(widgets.begin(), widgets.end(),
                            [&_p,_bVisibleActiveOnly](const WidgetAddTy& wa)
                            { return
                                (!_bVisibleActiveOnly || (wa.w.IsVisible() && wa.w.IsEnabled()))
                                && wa.w.Contains(_p); });
        
        // if we found a widget we change p further to widget-local coordinates
        if (_iter != widgets.end()) {
            _p -= _iter->w.Tl();
            return true;
        }
        
        // else not found anything
        return false;
    }
    
    
    /// Localize global coordinates to given widget so they become relative to the widget's top left corner
    void MainWnd::LocalizePoint (Point& _p, const WidgetAddTy& _wa)
    {
        // p is global, make it relative to main window
        _p -= Tl();
        // the take the frame out of consideration
        _p.x() -= frameThick;
        _p.y() += frameThick;
        // from main-window-local go further to widget-local
        _p -= _wa.w.Tl();
    }
    
    
    // callback for SetTimer()
    float MainWnd::CBTimer (float, float, int, void* _refCon)
    {
        // call main window's and each widget's DoTimer function
        MainWnd& mw = *reinterpret_cast<MainWnd*>(_refCon);
        mw.DoTimer();                           // main window
        for (WidgetAddTy& wa: mw.widgets)       // all widgets
            wa.w.DoTimer();
        return mw.timerInterval;                // call me again
    }
    
} // namespace "TFW"
