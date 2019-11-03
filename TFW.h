/// @file       TFW.h
/// @brief      Defines widget classes for XP11 without using XP's widgets but OpenGL drawing
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

#ifndef TFW_h
#define TFW_h

#include "TFWGraphics.h"

#include <algorithm>
#include <string>
#include <cstring>
#include <deque>
#include <vector>
#include "XPLMDisplay.h"

// Windows
// we prefer std::max/min of <algorithm>
#if max
#undef max
#endif
#ifdef min
#undef min
#endif

/// Encapsulates all TFW widget definitions for custom-drawn widgets in modern XP11 windows
namespace TFW {
    
    class Widget;
    class MainWnd;
    
    //
    // MARK: Useful colors
    //
    extern const float COL_NAN[4];                  ///< no color, special meaning per situation, like "use a default"
    extern const float COL_TRANSPARENT[4];          ///< fully transparent, will usually be skipped in drawing
    extern const float COL_WHITE[4];
    extern const float COL_WHITE_LIGHTTRANS[4];     ///< white with 90% transparency to light up background (like for alternating list box lines)
    extern const float COL_YELLOW[4];
    extern const float COL_RED[4];
    extern const float COL_GREEN[4];
    extern const float COL_BLUE[4];
    extern const float COL_BLUE_SELECT[4];          ///< selected item's background
    extern const float COL_GREY[4];
    extern const float COL_GREY_TRANSLUCENT[4];     ///< 55% opacity
    extern const float COL_GREY_DARK[4];            ///< dark grey
    extern const float COL_CYAN[4];
    
    /// XP standard colors, fetched from dataRefs provided by XP
    enum XPStdColorsE {
        COL_XP_BACKGROUND_RGB = 0,      ///< background color behind a modal window
        COL_XP_MENU_DARK_RGB,           ///< Dark tinging for menus
        COL_XP_MENU_HILITE_RGB,         ///< Menu color of a selected item
        COL_XP_MENU_LITE_RGB,           ///< Light tinging for menus
        COL_XP_MENU_TEXT_RGB,           ///< Menu Item Text Color
        COL_XP_MENU_TEXT_DISABLED_RGB,  ///< Menu Item Text Color When Disabled
        COL_XP_SUBTITLE_TEXT_RGB,       ///< Subtitle text colors
        COL_XP_TAB_FRONT_RGB,           ///< Color of text on tabs that are forward
        COL_XP_TAB_BACK_RGB,            ///< Color of text on tabs that are in the bkgnd
        COL_XP_CAPTION_TEXT_RGB,        ///< Caption text (for on a main window)
        COL_XP_LIST_TEXT_RGB,           ///< Text Color for scrolling lists
        COL_XP_GLASS_TEXT_RGB,          ///< Text color for on a 'glass' screen
        COL_XP_PLANE_PATH1_3D_RGB,      ///< Color for 3-d plane path
        COL_XP_PLANE_PATH2_3D_RGB,      ///< Striping color for 3-d plane path
        COL_XP_COUNT                    ///< always last, number of elements
    };
    
    //
    // MARK: Helper types
    //
    
    /// Defines how widget's coordinates get placed in relation to main window / other widgets
    enum WidgetPosE {
        POS_ABSOLUTE = 0,       ///< no automatic layout, just place where coordinates say
        POS_BOTTOM_RIGHT,       ///< relative to main window's bottom / right
        POS_AFTER_PRV,          ///< place after (below / to the right of) previous widget
        POS_SAME_PRV,           ///< place the same way as previous widget
        POS_FIXED_LEN,          ///< fixed offset to opposite point of same widget (causing fixed width/height)
        POS_PERCENT,            ///< relative to main windows width / height
    };
    
    /// structure defining widget positioning and the widget itself
    struct WidgetAddTy {
        WidgetPosE lR = POS_ABSOLUTE;           ///< left positioning rule
        int lO = 0;                             ///< offset for left pos
        WidgetPosE tR = POS_AFTER_PRV;          ///< top positioning rule
        int tO = 0;                             ///< offset for top pos
        WidgetPosE rR = POS_BOTTOM_RIGHT;       ///< right positioning rule
        int rO = 0;                             ///< offset for right pos
        WidgetPosE bR = POS_FIXED_LEN;          ///< bottom positioning rule
        int bO = -15;                           ///< offset for bottom pos
        Widget& w;                              ///< widget
    };
    
    /// Mouse button enumeration
    enum MouseBtnTy { MOUSE_LEFT, MOUSE_RIGHT };
    
    /// Data type, e.g. for values in list columns
    enum DataTypeTy {
        DTY_NULL = 0,                           ///< no data
        DTY_STRING,                             ///< data type "string"
        DTY_CHECKBOX,                           ///< data type "check box"
        DTY_INT,                                ///< data type "integer"
        DTY_DOUBLE,                             ///< data type "double"
    };
    
    /// Text Alignment
    enum TextAlignTy {
        TXA_LEFT = 0,                           ///< left aligned
        TXA_CENTER,                             ///< centered
        TXA_RIGHT                               ///< right aligned
    };
    
    /// Sorting, like for list box columns
    enum SortingTy {
        SORT_UNSUPPORTED = -1,                  ///< don't allow sorting
        SORT_UNSORTED = 0,                      ///< not sorted by this column
        SORT_ASCENDING,                         ///< sorted ascendingly
        SORT_DESCENDING                         ///< sorted descendingly
    };
    
    /// Button status types
    enum ButtonStateTy {
        BTN_NORMAL = 0,                         ///< normal state, released
        BTN_DOWN,                               ///< button actively pushed down (transient, while mouse down)
        BTN_SELECTED,                           ///< button selected (like checkbox checked)
        BTN_DOWN_SELECTED,                      ///< button actively pushed down while selected (transient, while mouse down)
    };
    
    /// Button array types
    enum ButtonArrayTy {
        BTN_RADIO_BUTTONS,                      ///< Radio buttons: one and only one is selected
        BTN_CHECK_BOXES,                        ///< Check boxes: anything between 0 and all buttons are selected
    };
    
    /// Focus change can happen in 3 ways
    enum FocusChangeTy {
        FCS_LOOSE = 0,                          ///< widget looses focus
        FCS_FIRST,                              ///< widget receives focus, shall activate first focus field if it manages several focus fields itself hierarchically
        FCS_NEXT,                               ///< widget shall activate next focus field (if it manage several focus fields) or release focus if it has none resp. reached the last self-managed field
        FCS_LAST,                               ///< widget receives focus "from the back", shall activate last focus field if it manages several focus fields itself hierarchically
        FCS_PREV,                               ///< widget shall activate previous focus field (if it manage several focus fields) or release focus if it has none resp. reached the first self-managed field
    };
    
    //
    // MARK: Croppable Text
    //
    
    /// @brief Holds a string plus its cropped version
    /// @details Avoids using expensive clipping operations during UI draw,
    ///          instead the clipped text is computed once only.
    class CroppedString
    {
    protected:
        std::string     sFull;          ///< full string
        std::string     sCropped;       ///< cropped string
        size_t          width;          ///< cropping width in pixels
        size_t          startPos = 0;   ///< starting position into sFull
    public:
        /// Default Constructor, sets width of UINT64_MAX, i.e. everything
        CroppedString () : width(UINT64_MAX) {}
        /// Constructor
        CroppedString (const std::string& _s, size_t _width=UINT64_MAX, size_t _start=0) :
        sFull(_s), width(_width), startPos(_start)
        { UpdateCroppedString(); }
        /// Constructor, which moves a string into `sFull`
        CroppedString (std::string&& _s, size_t _width, size_t _start=0) :
        sFull(std::move(_s)), width(_width), startPos(_start)
        { UpdateCroppedString(); }
        
        bool operator == (const CroppedString& _o) const { return sFull == _o.sFull; }    ///< Comparison bases on the full string only
        bool operator != (const CroppedString& _o) const { return sFull != _o.sFull; }    ///< Comparison bases on the full string only
        bool operator <  (const CroppedString& _o) const { return sFull <  _o.sFull; }    ///< Comparison bases on the full string only
        bool operator == (const std::string& _o) const { return sFull == _o; }            ///< Comparison bases on the full string only
        bool operator != (const std::string& _o) const { return sFull != _o; }            ///< Comparison bases on the full string only
        bool operator <  (const std::string& _o) const { return sFull <  _o; }            ///< Comparison bases on the full string only
        bool operator == (const char* _o) const { return sFull == _o; }                   ///< Comparison bases on the full string only
        bool operator != (const char* _o) const { return sFull != _o; }                   ///< Comparison bases on the full string only
        bool operator <  (const char* _o) const { return sFull <  _o; }                   ///< Comparison bases on the full string only

        /// Return the full string
        const std::string& full() const { return sFull; }
        /// Return the full string as C string
        const char* full_str() const { return sFull.c_str(); }
        /// Return the cropped string
        const std::string& cropped() const { return sCropped; }
        /// Return the cropped string as C string
        const char* cropped_str() const { return sCropped.c_str(); }

        /// Set a new string
        CroppedString& operator = (const std::string& _o)
        { sFull = _o; UpdateCroppedString(); return *this; }
        /// Move a new string
        CroppedString& operator = (std::string&& _o)
        { sFull = std::move(_o); UpdateCroppedString(); return *this; }
        
        // Pass on some std::string functions to sFull
        /// insert text
        CroppedString& insert(std::string::size_type index,
                              const std::string& str )
        { sFull.insert(index,str); UpdateCroppedString(); return *this; }
        /// insert text from substring
        CroppedString& insert(std::string::size_type index, const std::string& str,
                              std::string::size_type index_str,
                              std::string::size_type count = std::string::npos)
        { sFull.insert(index,str,index_str,count); UpdateCroppedString(); return *this; }
        /// append text
        CroppedString& operator+=(const char* str)
        { sFull += str; UpdateCroppedString(); return *this; }
        /// append text
        CroppedString& operator+=(const std::string& str)
        { sFull += str; UpdateCroppedString(); return *this; }
        /// append character
        CroppedString& operator+=(char ch)
        { sFull += ch; UpdateCroppedString(); return *this; }
        /// Erase characters from string
        CroppedString& erase(std::string::size_type index = 0,
                             std::string::size_type count = std::string::npos)
        { sFull.erase(index,count); UpdateCroppedString(); return *this; }
        /// Clear all text
        void clear() noexcept { sFull.clear(); sCropped.clear(); }
        /// Size
        std::string::size_type size() const noexcept { return sFull.size(); }
        
        /// Get current defined width
        size_t GetWidth () const { return width; }
        /// Set a new width
        void SetWidth (size_t _w) { width = _w; UpdateCroppedString(); }
        /// Get current defined starting position
        size_t GetStartPos () const { return startPos; }
        /// Set a new starting position, which can be, at maximum, the last character
        void SetStartPos (size_t _sp) { startPos = std::min(_sp,size()-1); UpdateCroppedString(); }
        
        /// How many characters missing to the right of the cropped string?
        size_t GetNumCharsCropped () const { return sFull.size() - startPos - sCropped.size(); }

        /// Recalculate `sCropped` from `sFull`
        void UpdateCroppedString ();
    };
    
    //
    // MARK: Widgets
    //

    /// @brief Base class for all widgets
    /// @details Bases on Rect, which represents the area covered by the widget
    class Widget : public Rect
    {
    protected:
        MainWnd&    mainWnd;            ///< the main window used to inform about events
        unsigned    widgetId = 0;       ///< unique id assigned by main window
        bool        bVisible = true;    ///< currently visible?
        bool        bEnabled = true;    ///< enabled / disabled
        CroppedString caption;          ///< the text to display
        float       colBk[4];           ///< background color
        float       colFg[4];           ///< foreground color
        float       colFgDisabled[4];   ///< foreground color when disabled
    public:
        /// constructor adds widget to main window
        /// @param _caption Caption, usually an identifying/displayed string
        /// @param main Reference to the main window controlling widget display and functionality
        Widget (const std::string& _caption,
                MainWnd& main,
                WidgetPosE lR = POS_ABSOLUTE,        int lO = 0,
                WidgetPosE tR = POS_AFTER_PRV,       int tO = 0,
                WidgetPosE rR = POS_BOTTOM_RIGHT,    int rO = 0,
                WidgetPosE bR = POS_FIXED_LEN,       int bO = 15);
        
        /// make sure destrcutor are virtual
        virtual ~Widget () {}
        
        /// Unique id assigned by main window, 0 for main wnd
        unsigned GetId () const { return widgetId; }
        /// comparison bases on unique id
        virtual bool operator == (const Widget& o) const { return widgetId && widgetId == o.widgetId; }
        
        /// show/hide the widget
        virtual void SetVisible (bool _bVisible = true) { bVisible = _bVisible; }
        /// Is the widget visible?
        virtual bool IsVisible () const { return bVisible; }
        
        /// enabled/dsiable the widget
        virtual void SetEnabled (bool _bEnabled = true) { bEnabled = _bEnabled; }
        /// Is the widget enabled=
        bool IsEnabled () const { return bEnabled; }
        
        /// Does the widget have focus?
        inline bool HasFocus () const;
        
        /// Is this the main window?
        inline bool IsMainWnd () const;
        
        /// Set the text to display
        virtual void SetCaption (const std::string& _c) { caption = _c; }
        /// Get current text
        const std::string& GetCaption () const { return caption.full(); }
        /// No caption defined?
        bool IsEmpty () const { return caption.full().empty(); }
        /// Get current visible part of text
        const std::string& GetVisibleCap () const { return caption.cropped(); }

        /// Clear the widget, here: remove text
        virtual void Clear() { SetCaption(""); }

        /// sets the background color
        virtual void SetBkColor (const float _col[4])
        { std::memmove (colBk, _col, sizeof(colBk)); }
        /// sets the background color to some standard XP color
        virtual void SetBkColor (XPStdColorsE _colId)
        { SetBkColor (COL_TRANSPARENT); colBk[0] = (float)_colId; }
        /// sets the foreground color for enabled and disabled case
        virtual void SetFgColor (const float _col[4], const float _colDisabled[4])
        {   std::memmove (colFg, _col, sizeof(colFg));
            std::memmove (colFgDisabled, _colDisabled, sizeof(colFgDisabled)); }
        /// sets the foreground color to some standard XP color
        virtual void SetFgColor (XPStdColorsE _colId, XPStdColorsE _colIdDisabled)
        {   SetFgColor (COL_TRANSPARENT, COL_TRANSPARENT);
            colFg[0] = (float)_colId; colFgDisabled[0] = (float)_colIdDisabled; }

        /// set geometry of widget, updates visible part of caption
        virtual void SetGeometry (const Rect& _r) { Rect::SetGeometry(_r); caption.SetWidth((size_t)_r.Width()); }

    protected:
        /// Called by MainWnd, this function does the drawing
        virtual void DoDraw (const Rect& r);
        /// Called by MainWnd, this function handles mouse clicks
        virtual void DoMouseClick (Point /*p*/, MouseBtnTy /*mouseBtn*/, XPLMMouseStatus /*inMouse*/) {}
        /// Called by MainWnd, this function handles mouse wheel
        virtual void DoMouseWheel (Point /*p*/, int /*wheel*/, int /*clicks*/) {}
        /// Called by MainWnd, this function handles keys
        virtual void DoHandleKey (char /*inKey*/, XPLMKeyFlags /*inFlags*/, unsigned char /*inVirtualKey*/) {}
        /// @brief Called by MainWnd when focus changes (standard widget deosn't accept focus)
        /// @return Does this widget take/keep the focus?
        virtual bool DoFocusChange (FocusChangeTy) { return false; }
        /// Called by MainWnd, this function sets the form of the cursor
        virtual XPLMCursorStatus DoHandleCursor (Point /*p*/) { return xplm_CursorDefault; }
        /// Called by a timer started with MainWnd::SetTimer()
        virtual void DoTimer () {}

        /// paint the background of the widget area
        virtual void DrawBackground (const Rect& r);
        
        /// Main window needs to call all the protected Do... functions
        friend class MainWnd;
    };

    
    /// Simple text label
    class Label : public Widget
    {
    public:
        bool    bBold = false;      ///< bold font? (Writes text twice with one pixel ofset)
    public:
        /// construct a label, for positioning params see struct WidgetAddTy
        /// @param _caption Label text to show
        /// @param main parent main window
        Label (const std::string& _caption,
               MainWnd& main,
               WidgetPosE lR = POS_ABSOLUTE,        int lO = 0,
               WidgetPosE tR = POS_AFTER_PRV,       int tO = 0,
               WidgetPosE rR = POS_BOTTOM_RIGHT,    int rO = 0,
               WidgetPosE bR = POS_FIXED_LEN,       int bO = 15);

        /// @brief Called by MainWnd, this function does the drawing of the label
        /// @param r Drawing rectangle for widget
        virtual void DoDraw (const Rect& r);
    };


    /// A divider line, just as optical separation, nothing fancy
    class Line : public Widget
    {
    public:
        float lnWidth = 1.0;
    public:
        /// construct a line from top/left to bottom/right, for positioning params see struct WidgetAddTy
        /// @param _caption Will not show, may serve internal identification purposes
        /// @param main parent main window
        /// @param _lnWidth Line width, defaults to 1.0
        Line (const std::string& _caption,
              MainWnd& main,
              WidgetPosE lR, int lO,
              WidgetPosE tR, int tO,
              WidgetPosE rR, int rO,
              WidgetPosE bR, int bO,
              float _lnWidth = 1.0f);
        
        /// @brief Called by MainWnd, this function does the drawing of the line
        /// @param _r Drawing rectangle for widget
        virtual void DoDraw (const Rect& _r);
    };
    
    //
    // MARK: Buttons
    //
    
    /// Push button
    class ButtonPush : public Widget
    {
    protected:
        ButtonStateTy btnState = BTN_NORMAL;    ///< current button's status
    public:
        /// construct a push button, for positioning params see struct WidgetAddTy
        /// @param _caption Button text to show
        /// @param main parent main window
        ButtonPush (const std::string& _caption,
                    MainWnd& main,
                    WidgetPosE lR = POS_ABSOLUTE,        int lO = 0,
                    WidgetPosE tR = POS_AFTER_PRV,       int tO = 0,
                    WidgetPosE rR = POS_BOTTOM_RIGHT,    int rO = 0,
                    WidgetPosE bR = POS_FIXED_LEN,       int bO = 15);
 
        /// What's the current state of the button
        ButtonStateTy GetState () const { return btnState; }

    protected:
        /// Toggles button status
        virtual void DoMouseClick (Point /*p*/, MouseBtnTy mouseBtn, XPLMMouseStatus inMouse);
        /// [Space] is like click
        virtual void DoHandleKey (char /*inKey*/, XPLMKeyFlags inFlags, unsigned char inVirtualKey);
        /// Buttons accept focus, i.e. it keeps focus when receiving and release it when it looses focus
        virtual bool DoFocusChange (FocusChangeTy _fcs) { return _fcs == FCS_FIRST || _fcs == FCS_LAST; }
        /// @brief Called by MainWnd, this function does the drawing of the push button
        /// @param _r Drawing rectangle for button
        virtual void DoDraw (const Rect& _r);
        /// @brief Handle a complete click, i.e. after down and up events
        /// @return what the button state is now, BTN_NORMAL in case of ButtonPush
        virtual ButtonStateTy ButtonClicked ();
    };
    
    /// Represents an individual Radio button / check box within an ButtonArray
    class ButtonElem : public Rect
    {
    public:
        void*           refCon;                 ///< some unique refcon
        CroppedString   Caption;                ///< button text
        
    protected:
        ButtonStateTy   btnState = BTN_NORMAL;  ///< current button's status
        
    public:
        /// Constructor (rectangle is not needed here, will be filled in SetGeometry())
        ButtonElem (void* _refCon, const std::string& _caption,
                    ButtonStateTy _btnState = BTN_NORMAL,
                    int _left = 0, int _top = 0, int _right = 0, int _bottom = 0) :
        Rect(_left, _top, _right, _bottom),
        refCon(_refCon), Caption(_caption), btnState(_btnState) {}
        
        /// Equality bases solely on `refCon`
        bool operator == (const ButtonElem& o) const { return refCon == o.refCon; }
        /// Equality bases solely on `refCon`
        bool operator != (const ButtonElem& o) const { return refCon != o.refCon; }
        /// Is this button element selected?
        bool IsSelected () const { return btnState == BTN_SELECTED || btnState == BTN_DOWN_SELECTED; }
        /// Is this button pushed at the moment?
        bool IsDown () const     { return btnState == BTN_DOWN     || btnState == BTN_DOWN_SELECTED; }
        /// Define the buttons current state as selected or not
        void SetSelected (bool _bSel);
        /// Define the buttons current state as down or up
        void SetDown (bool _bDown);

        /// Resizing means we might need to crop the caption
        virtual void SetGeometry(const Rect &_r);

        /// Draw the button element
        virtual void DoDraw (const Rect& _r, ButtonArrayTy _type, float _col[4], bool _bFocus);
    };
    
    /// Array of button elements, which make up a ButtonArray
    typedef std::vector<ButtonElem> ButtonElemArr;
    
    /// Placeholder for an empty non-existing element returned if refCon not found
    extern ButtonElem BUTTON_ELEM_NULL;
    
    /// Radio buttons or Check boxes, organized in horizontal or vertical stacks
    class ButtonArray : public Widget
    {
    protected:
        ButtonArrayTy   type;               ///< Radio Buttons or Check Boxes?
        bool            bVertical;          ///< Vertical order of buttons? (Otherwise horizontally)
        ButtonElemArr   aBtns;              ///< array of buttons to be displayed
        ButtonElemArr::iterator iterFocus;
    public:
        unsigned        lnHeight = 15;      ///< for vertical arrangement we assume fixed height (not distributed over available space)
    public:
        /// construct a button array, for positioning params see struct WidgetAddTy
        /// @param _caption Identifier for button array, but not actually shown
        /// @param main parent main window
        /// @param _type Radio Buttons or Check Boxes?
        /// @param _bVertical Vertical order of buttons? (Otherwise horizontally)
        ButtonArray (const std::string& _caption,
                     MainWnd& main,
                     ButtonArrayTy _type,
                     bool _bVertical,
                     WidgetPosE lR = POS_ABSOLUTE,        int lO = 0,
                     WidgetPosE tR = POS_AFTER_PRV,       int tO = 0,
                     WidgetPosE rR = POS_BOTTOM_RIGHT,    int rO = 0,
                     WidgetPosE bR = POS_FIXED_LEN,       int bO = 15);

        /// Add a button to the array
        bool AddButton (ButtonElem&& _elem);
        /// @brief Finds an element by refCon
        /// @return A reference to that element, or to BUTTON_ELEM_NULL if not found
        ButtonElem& FindElem (void* _refCon);
        /// Find an element by refCon
        ButtonElem& operator[](void* _refCon) { return FindElem(_refCon); }
        /// Find an element by index
        ButtonElem& operator[](size_t i);
        
        /// Return (first) selected element
        const ButtonElem& GetSelected () const;
        /// Return (first) selected element's refCon
        void* GetSelectedRefCon () const { return GetSelected().refCon; }
        
        /// Set the selected element
        void SetSelected (void* _refCon);

        /// Resizing means we need to recalculate the ButtonElems' positions
        virtual void SetGeometry(const Rect &_r);

    protected:
        /// Toggles button status
        virtual void DoMouseClick (Point /*p*/, MouseBtnTy mouseBtn, XPLMMouseStatus inMouse);
        /// Handle a complete click, i.e. after down and up events
        virtual void ButtonClicked (ButtonElem& _selBe);
        /// [Space] is like click
        virtual void DoHandleKey (char /*inKey*/, XPLMKeyFlags /*inFlags*/, unsigned char /*inVirtualKey*/);
        /// Button arrays accept focus and pass it down to ButtomElems:
        virtual bool DoFocusChange (FocusChangeTy _fcs);
        /// @brief Called by MainWnd, this function does the drawing of the push button
        /// @param _r Drawing rectangle for button
        virtual void DoDraw (const Rect& _r);
        /// Identify which button element a point falls into, NULL if none
        bool FindButtonByPos (const Point& _p, ButtonElemArr::iterator& _iter);
    };
    
    //
    // MARK: Edit field
    //
    
#define CHR_ALPHA_UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define CHR_ALPHA_LOWER "abcdefghijklmnopqrstuvwxyz"
#define CHR_ALPHA CHR_ALPHA_UPPER CHR_ALPHA_LOWER
#define CHR_NUM        "1234567890"
#define CHR_ALPHA_NUM CHR_ALPHA CHR_NUM
#define CHR_ALPHA_NUM_SPACE CHR_ALPHA CHR_NUM " "

    /// An edit field, for text or number entry
    class EditField : public Widget
    {
    public:
        std::string     allowedChars;           ///< list of allowed chars, or empty = any
        size_t          maxLen = 1024;          ///< max number of characters
        bool            bPwdMode = false;       ///< if `true` displays asterisks only
    protected:
        size_t          caretPos = 0;           ///< where is the caret, i.e. the current insert position?
        std::string     prevTxt;                ///< last text informed to main window, compare-bases for "has changed?"
    public:
        /// construct an edit field, for positioning params see struct WidgetAddTy
        /// @param _caption Initial value to be displayed for editing
        /// @param _allowed list of allowed characters, empty = any
        /// @param _maxLen Maximum allowed number of characters, 0 for unlimited
        /// @param main parent main window
        EditField (const std::string& _caption,
                   const std::string& _allowed,
                   unsigned _maxLen,
                   MainWnd& main,
                   WidgetPosE lR = POS_ABSOLUTE,        int lO = 0,
                   WidgetPosE tR = POS_AFTER_PRV,       int tO = 0,
                   WidgetPosE rR = POS_BOTTOM_RIGHT,    int rO = 0,
                   WidgetPosE bR = POS_FIXED_LEN,       int bO = 15);
        
        /// Set integer value
        virtual void Set (long _l) { SetCaption(std::to_string(_l)); }
        /// Set floating point value
        virtual void Set (double _d, int precision = 2);
        /// Get current value converted to integer
        virtual long GetLong () const       { return std::stol(GetCaption()); }
        /// Get current value converted to double
        virtual double GetDouble () const   { return std::stod(GetCaption()); }

        /// Copy into clipboard/pasteboard (currently just the entire caption)
        virtual void ClipCopy ();
        /// Copy into clipboard/pasteboard and clear
        virtual void ClipCut ();
        /// Paste text from clipboard/pasteboard at current caret position
        virtual void ClipPaste ();
        
    protected:
        /// Entry into the fields
        virtual void DoHandleKey (char /*inKey*/, XPLMKeyFlags /*inFlags*/, unsigned char /*inVirtualKey*/);
        /// Places the caret
        virtual void DoMouseClick (Point p, MouseBtnTy mouseBtn, XPLMMouseStatus inMouse);
        /// Wheel scrolls the text left and right
        virtual void DoMouseWheel (Point /*p*/, int /*wheel*/, int clicks);
        /// Edit fields accept focus, on lost focus it informs main window on changed text
        virtual bool DoFocusChange (FocusChangeTy _fcs);
        /// @brief Called by MainWnd, this function does the drawing of the field
        /// @param _r Drawing rectangle for field
        virtual void DoDraw (const Rect& _r);
        /// returns the current string to draw, which depends on scrolling and on conversions like password-*
        virtual std::string GetDrawString () const;
    };
    
    //
    // MARK: List box
    //
    
    /// Represents an actual value of a list box row
    class ListItem
    {
    protected:
        CroppedString   sVal;           ///< the value to display
        int             iVal = 0;       ///< the last integer or boolean (Checkbox) value stored
        double          dVal = 0.0;     ///< the last double value stored
        bool            bEnabled = true;///< relevant for checkboxes: enabled or disabled?
    public:
        /// Text color to use, defaults to "use standard text color"
        float           col[4] = { NAN, NAN, NAN, 0.00f};
    public:
        ListItem() {}
        ListItem(const char* _s) : sVal(_s) {}
        ListItem(const std::string& _s) : sVal(_s) {}
        ListItem(std::string&& _s) : sVal(_s) {}
        ListItem(int _i) : sVal(std::to_string(_i)), iVal(_i) {}
        ListItem(bool _b) : sVal(_b ? "X" : " "), iVal(_b) {}
        ListItem(double _d, int _prec = 2) : dVal(_d) { Set(_d,_prec); }
        
        /// compare against a string
        bool operator == (const char* _s) const { return sVal == _s; }
        /// compare against a string
        bool operator == (const std::string& _s) const { return sVal == _s; }
        /// compare against an int
        bool operator == (int _i) const { return iVal == _i; }
        /// compare against a string
        bool operator != (const char* _s) const { return sVal != _s; }
        /// compare against a string
        bool operator != (const std::string& _s) const { return sVal != _s; }
        /// compare against an int
        bool operator != (int _i) const { return iVal != _i; }

        /// Sets current value to a string value
        void Set (const char* _s) { sVal = _s; }
        /// Sets current value to a string value
        void Set (const std::string& _s) { sVal = _s; }
        /// Sets current value to a string value
        void Set (std::string&& _s) { sVal = _s; }
        /// Sets current value to an integer value
        void Set (int _i) { sVal = std::to_string(iVal=_i); }
        /// Sets current value to a boolean value
        void Set (bool _b) { sVal = (iVal=_b) ? "X" : " "; }
        /// @brief Sets current value to a double value
        /// @param _d The value
        /// @param _prec Precision, with with the value is to be displayed
        void Set (double _d, int _prec = 2);
        
        const std::string& GetS () const { return sVal.full(); }    ///< get current string representation
        int                GetI () const { return iVal; }           ///< get last set integer value
        bool               IsSet () const { return iVal != 0; }     ///< get last set boolean value
        double             GetD () const { return dVal; }           ///< get last set double value
        
        /// returns the string representation of the drawing as a char pointer
        char* GetCroppedS () const { return (char*)sVal.cropped_str(); }
        /// Sets the columns width
        void SetWidth (size_t _w) { sVal.SetWidth(_w); }
        
        /// Define text color
        void SetColor (const float _c[4]) { memmove(col, _c, sizeof(col)); }
        
        /// Set checkbox enabled or disabled
        void SetEnable (bool _b) { bEnabled = _b; }
        /// Is checkbox enabled?
        bool IsEnabled () const { return bEnabled; }

        /// @brief compare function, used for sorting
        /// @return Is `*this` less than `o`?
        /// @param o The other ListItem to compare with
        /// @param _dataType Data type to use for comparison
        /// @param _thisBeforeOnEqual For creating a unique order even in case this column contains duplicates this parameter defines if this < o if column value is equal
        bool less (const ListItem& o, DataTypeTy _dataType, bool _thisBeforeOnEqual) const;
    };
    
    /// Represents an actual row in a list box, primarily a vector of ListItem objects
    class ListRow : public std::vector<ListItem>
    {
    protected:
        unsigned sequNr = 0;            ///< order, in which rows had been added, sorting order of "unsorted" data
    public:
        void* refCon = nullptr;         ///< user-defined refCon tied to this row, uniquely identifies the data
    public:
        /// @brief Constructor creates a row
        /// @param _refCon Application-defined refCon, unique for this row
        /// @param _fields A list of field values / elements in the row
        ListRow (void* _refCon,
                 std::initializer_list<ListItem> _fields = {});
        /// Get row's sequence number
        unsigned GetSequNr () const { return sequNr; }

        /// equality is defined via `refCon`. So for identifying the row it should be set uniquely
        bool operator == (const ListRow& o) const { return refCon == o.refCon; }
        
        /// @brief compare function on any element, used for sorting
        /// @return Is `(*this)[_col]` less than `o[_col]`?
        /// @param o The other ListRow to compare with
        /// @param _col Column id (vector idx) of element to compare
        /// @param _sorted List is sorted in which way?
        /// @param _dataType Data type to use for comparison
        bool elemLess (const ListRow& o, unsigned _col, SortingTy _sorted, DataTypeTy _dataType) const;
    protected:
        /// Get row's sequence number
        void SetSequNr (unsigned _s) { sequNr = _s; }
        
        friend class ListBox;
    };
    
    /// Type of vector to use for storing rows
    typedef std::vector<ListRow> ListRowVecTy;
    
    /// Defines what a list box column looks like
    class ListColumnDef
    {
    public:
        CroppedString name;                 ///< column header
        int width = 50;                     ///< column width in boxels
        DataTypeTy dataType = DTY_STRING;   ///< type of data, primarily importand for sorting
        bool bVisible = true;               ///< column currently shown?
    public:
        /// Standard constructor creates a non-existing column
        ListColumnDef () : width(0), dataType(DTY_NULL), bVisible(false) {}
        /// Constructor defines a column with name, width, type, and visibility
        ListColumnDef (const std::string& _name,
                       int _w,
                       DataTypeTy _dt = DTY_STRING,
                       bool _v = true) :
        name(_name), width(_w), dataType(_dt), bVisible(_v) {}
        
        /// return cropped name as char pointer
        char* GetCroppedName () const { return (char*)name.cropped_str(); }
    };
    
    /// Type of vector to use for storing column definitions
    typedef std::vector<ListColumnDef> ListColDefVecTy;
    
    /// @brief List box
    ///
    /// Idea:
    ///
    /// - Columns are identified by small but self-assigned unsigneds. These
    ///   numbers are used as index into a vector, so keep them small.
    ///   But this way access is fast, and we need to access both column
    ///   definitions and actual values (see below) with each drawing cycle.
    ///   Read: _Very_ fast.
    ///   The order, in which columns appear, is configurable and can differ
    ///   from the ids. Columns can also be hidden.
    ///
    /// - Rows include a vector, indexed by the same column ids, which hold
    ///   the values. Eventually, each value is a string and stored like this.
    ///   For convience number formatting functons are available.
    class ListBox : public Widget
    {
    protected:
        ListColDefVecTy cols;               ///< column definitions
        // sorting definitions:
        unsigned  sortedBy = 0;             ///< column, by which we sort the data
        SortingTy sorted = SORT_UNSORTED;   ///< list sorted by any column?
        
        ListRowVecTy rows;                  ///< rows, actual data to be displayed
        unsigned sequRow = 0;               ///< last sequence number assinged to a row
        
        unsigned lnHeightFactor = 200;      ///< line height in % of font height, defaults to 150%
        unsigned lnHeight = 15;             ///< current aboslute line height

        void* selRefCon = nullptr;          ///< `refCon` of the selected line if any
        
        unsigned rowFirst = 0;              ///< scroll position: this is the first row to show
        unsigned rowMax = 0;                ///< how many rows can be shown with current list box size?
        unsigned rowFirstMaxDown = 0;       ///< what would be the first row to show when fully scrolled down?
        
        int sbThumbTop = 0;                 ///< if there is a scrollbar then this is the thumb's top position
        int sbThumbBottom = 0;              ///< if there is a scrollbar then this is the thumb's bottom position

    public:
        /// @brief construct a listbox, for positioning params see struct WidgetAddTy
        /// @param _caption Label is not actually shown in case of a list box, but might be useful for internal identification
        /// @param main parent main window
        ListBox (const std::string& _caption,
                 MainWnd& main,
                 WidgetPosE lR = POS_ABSOLUTE,        int lO = 0,
                 WidgetPosE tR = POS_AFTER_PRV,       int tO = 0,
                 WidgetPosE rR = POS_BOTTOM_RIGHT,    int rO = 0,
                 WidgetPosE bR = POS_FIXED_LEN,       int bO = 15);
        
        /// @brief Adds or overwrites a column definition
        /// @param colId Column id this definition refers to
        /// @param def Column definition
        virtual void AddCol (unsigned colId, ListColumnDef&& def);
        /// Get a column by id
        virtual ListColumnDef& GetCol (unsigned colId) { return cols.at(colId); }
        
        /// @brief Define sorting, will immediately resort the available data
        /// @param _col Column id by which the data is to be sorted
        /// @param _sorted Sorting order, or SORT_UNSORTED if revert order, in which rows had been added originally
        virtual void SetSorting (unsigned _col, SortingTy _sorted);
        
        /// @brief set geometry of list box
        virtual void SetGeometry (const Rect& _r);

        /// Set line height
        virtual void SetLnHeightFactor (unsigned _lhf) { lnHeightFactor = _lhf; }
        /// Get current line height factor
        unsigned GetLnHeightFactor () const { return lnHeightFactor; }
        
        /// @brief Add/update an actual row to the list box, which is to be displayed.
        /// @param row The row data to add.
        /// @return The row as stored by the list
        /// @note The `row`'s `refCon` acts as a unique identifier per row.
        ///       If the same `refCon` is sent in again AddRow()
        ///       overwrites the existing data.
        virtual ListRow& AddRow (ListRow&& row);
        /// Return row by index (which is unsafe as sorting can change the order)
        virtual ListRow* GetRow (size_t index);
        /// Return row by index (which is unsafe as sorting can change the order)
        virtual const ListRow* GetRow (size_t index) const;
        /// Number of entries in the list
        virtual size_t GetSize() const { return rows.size(); }
        /// Remove a row, identified by ListRow::refCon
        virtual bool RemoveRow (void* _refCon);
        /// Clear a list: Remove all rows
        virtual void Clear ();
        
        /// Set selection, returns true if the row is available
        virtual bool SelectRow (void* _refCon);
        /// find a row by refCon
        virtual const ListRow* FindRow (void* _refCon) const;
        /// find a row by refCon
        virtual ListRow* FindRow (void* _refCon);
        /// Return selected row or NULL if none is selected / available
        virtual const ListRow* GetSelectedRow () const { return FindRow (selRefCon); }

        
    protected:
        /// @brief Crops texts of header and content to match geometry
        virtual void DoCropTexts ();
        
        /// @brief Called by MainWnd, this function does the drawing of the list
        /// @param r Drawing rectangle for widget
        virtual void DoDraw (const Rect& r);

        /// @brief Draw column headers.
        /// @param rowRect Dimensions of the header row
        /// @param txtColor The color to use for text
        virtual void DoDrawHeader (const Rect& rowRect, const float txtColor[4]);

        /// @brief Draw a row's content (text)
        /// @param rowRect Dimensions of the row
        /// @param row The row to draw.
        /// @param txtColor The standard color to use for text (can be overriden in `ListItem.col`)
        virtual void DoDrawRow (const Rect& rowRect, const ListRow& row, const float txtColor[4]);
        
        /// @brief Draw a list box's vertical scrollbar
        /// @param _r Drawing rectangle for listbox
        virtual void DoDrawVertScrollbar (const Rect& _r);

        /// @brief ListBox accepts focus
        /// @return Processed? In case of receiving focus also means: Do we accept focus?
        virtual bool DoFocusChange (FocusChangeTy _fcs) { return _fcs == FCS_FIRST || _fcs == FCS_LAST; }
        
        /// Re-sort the existing list of rows according to sorting criteria
        virtual void DoSort ();
        
        /// define the new selected row and inform the main window
        virtual void DoSelectRow (ListRow& _selRow);
        
        /// Find row by refCon
        /// @param[in] _refCon The refCon to search for
        /// @param[out] iter The iterator, which will point to the found elment
        /// @return Found?
        virtual bool FindRowIdx (void* _refCon, ListRowVecTy::const_iterator& iter) const;
        virtual bool FindRowIdx (void* _refCon, ListRowVecTy::iterator& iter);
        /// Find row by y coordinate, relative to top of list box
        virtual bool FindRowIdx (int _y, ListRowVecTy::iterator& iter);
        /// Find column by x coordinate, relative to top of list box
        virtual unsigned FindColIdx (int _x);

        /// Ensure that the selected row, if any, is scrolled into visibility
        virtual void MakeSelVisible ();

        /// Change selected row upon mouse click
        virtual void DoMouseClick (Point p, MouseBtnTy mouseBtn, XPLMMouseStatus inMouse);
        /// Handle cursor keys to change selection and scroll list
        virtual void DoHandleKey (char inKey, XPLMKeyFlags inFlags, unsigned char inVirtualKey);
        /// Scroll up/down
        virtual void DoMouseWheel (Point p, int wheel, int clicks);

    };
    
    //
    // MARK: Main Window
    //
    
    /// Main window, this is the one communicating with X-Plane
    class MainWnd : public Widget
    {
    protected:
        XPLMWindowID wnd = NULL;    ///< XP's window id for this window
        /// In the next drawing callback (when we know current pos) reset positioning mode to:
        XPLMWindowPositioningMode resetPosModeTo = -1;
        /// internal frame, ie. space left empty
        int frameThick = 5;
        /// timer interval as set with SetTimer()
        float timerInterval = 0.0f;
    public:
        /// Main window coordinates are global coordinates
        MainWnd (const std::string& _caption,
                 int _left = 0, int _top = 0, int _right = 0, int _bottom = 0,
                 XPLMWindowDecoration _deco = xplm_WindowDecorationRoundRectangle,
                 XPLMWindowLayer _layer = xplm_WindowLayerFloatingWindows,
                 int _frame = 5);
        /// destroys the window
        virtual ~MainWnd ();
        
        /// access to underlying window id
        XPLMWindowID GetWnd() const { return wnd; }
        /// use the object like an XPLMWindowID
        operator XPLMWindowID() const { return wnd; }
        
        /// show/hide the window
        virtual void SetVisible (bool _bVisible = true);
        /// Is the widget visible?
        virtual bool IsVisible () const
        { return XPLMGetWindowIsVisible(wnd) > 0; }
        
        /// @brief Take/release keyboard focus
        /// @see https://developer.x-plane.com/sdk/XPLMDisplay/#XPLMTakeKeyboardFocus
        void TakeKeyboardFocus (bool _bTake = true) { XPLMTakeKeyboardFocus(_bTake ? wnd : 0); }
        /// @brief Do I have the keyboard focus?
        /// @see https://developer.x-plane.com/sdk/XPLMDisplay/#XPLMHasKeyboardFocus
        bool HasKeyboardFocus () const { return XPLMHasKeyboardFocus(wnd); }
        
        /// @brief Defines which button's click is simulated upon [Enter] by calling MsgButtonClicked()
        /// @param _pDefBtn Pointer to default button (essentially the parameter for calling MsgButtonClicked()) or NULL for no default handling
        virtual void SetDefaultButton (ButtonPush* _pDefBtn) { pDefBtn = _pDefBtn; }
        
        /// Set the window title
        virtual void SetCaption (const std::string& _c);
        
        /// Reads the current window's geometry and stores it in the `Rect` members
        virtual void FetchGeometry ()
        { XPLMGetWindowGeometry (wnd, &Left(), &Top(), &Right(), &Bottom()); }
        
        /// @brief Center the window on the main screen, then switch to floating type
        /// @param _bVisible Shall window be visible or not at the end?
        /// @param _monitorIdx Monitor index the window shall appear on, negative for XP's main window
        virtual void SetCenterFloat (bool _bVisible = true,
                                     int  _monitorIdx = -1);
        
        /// Move the window to the same position (top/left corner) as the window passed in
        virtual void SetPosAs (const MainWnd& _o);
        
        /// Set positioning mode
        /// @see https://developer.x-plane.com/sdk/XPLMDisplay/#XPLMSetWindowPositioningMode
        virtual void SetPositioningMode (XPLMWindowPositioningMode inPositioningMode,
                                         int                       inMonitorIndex)
        { XPLMSetWindowPositioningMode(wnd, inPositioningMode, inMonitorIndex); }
        
        /// Tries to figure out the positioning mode
        virtual XPLMWindowPositioningMode GetPositioningMode () const;
        
        /// @brief Start a timer, which calls DoTimer() repeatedly
        /// @param _interval Positive: in seconds, Negative: in cycles, 0: stop
        virtual void SetTimer (float _interval);
        
    public:
        /// Add a widget to the window
        virtual void Add (WidgetAddTy&& _wa);
        
        /// based on the positioning rules recalculates the current window's layout
        virtual void Layout ();
        
    protected:
        /// last unique widget id assigned
        unsigned lastWidgetId = 0;
        /// type of list of widgets
        typedef std::deque<WidgetAddTy> WidgetAddDequeTy;
        /// list of widgets owned by this main window
        WidgetAddDequeTy widgets;
        /// recalc of widget layout needed? (due to resizing or added widgets)
        bool bLayoutDirty = true;
        /// The widget which currently has input focus, i.e. receives keys
        WidgetAddTy* pFocusWidget = nullptr;
        /// The default push button when pressing [Enter]
        ButtonPush* pDefBtn = nullptr;
        
    public:
        // functions performing callback functionality
        /// @see https://developer.x-plane.com/sdk/XPLMDrawWindow_f/
        virtual void DoDraw (const Rect& /*r*/);
        /// @see https://developer.x-plane.com/sdk/XPLMHandleMouseClick_f/
        virtual void DoMouseClick (Point p, MouseBtnTy mouseBtn, XPLMMouseStatus inMouse);
        /// @see https://developer.x-plane.com/sdk/XPLMHandleMouseWheel_f/
        virtual void DoMouseWheel (Point p, int wheel, int clicks);
        /// @see https://developer.x-plane.com/sdk/XPLMHandleKey_f/
        virtual void DoHandleKey (char inKey, XPLMKeyFlags inFlags, unsigned char inVirtualKey);
        /// @see https://developer.x-plane.com/sdk/XPLMHandleKey_f/
        virtual bool DoFocusChange (FocusChangeTy _fcs);
        /// @see https://developer.x-plane.com/sdk/XPLMHandleCursor_f/
        virtual XPLMCursorStatus DoHandleCursor (Point p);
        
        /// Return the current focus window, or `nullptr`
        Widget* GetFocusWidget () const { return pFocusWidget ? &pFocusWidget->w : nullptr; }
        
    protected:
        /// @brief Finds the next widget to receive focus, either forward or backward
        /// @tparam InputIt Either a forward or a backward iterator over list of `widgets`
        /// @param first Iterator to the beginning of the widget list to iterate
        /// @param last Iterator to its end
        /// @param _fcs Indicates direction, should be only FCS_NEXT or FCS_PREV
        template<class InputIt> void DoFocusNextWidget (InputIt first, InputIt last, FocusChangeTy _fcs);
        
        /// @brief Find the widget, which contains given global p
        /// @param[in,out] _p Global position on input, hit position relative to widget on output
        /// @param[out] _iter Iterator pointing to the widget in `widgets`
        /// @param _bVisibleActiveOnly Only return widgets, which are marked visible and active
        /// @return Found a widget?
        virtual bool FindWidgetByPos (Point& _p, WidgetAddDequeTy::iterator& _iter,
                                      bool _bVisibleActiveOnly = true);
        
        /// Localize global coordinates to given widget so they become relative to the widget's top left corner
        virtual void LocalizePoint (Point& _p, const WidgetAddTy& _wa);
        
        /// @brief callback for SetTimer()
        /// @see https://developer.x-plane.com/sdk/XPLMProcessing/#XPLMFlightLoop_f
        static float CBTimer (float, float, int, void* _refCon);
        
        //
        // Override the following notification methods if you want to
        // handle any such event from you derived main window class
        //
    public:
        /// The user selected a row in a list box
        virtual void MsgListSelChanged (ListBox&, ListRow&) {}
        /// The user pressed a button
        virtual void MsgButtonClicked (ButtonPush&) {}
        /// The user pressed a button element in a button array
        virtual void MsgButtonArrClicked (ButtonArray&, ButtonElem&) {}
        /// The user changed text of an edit field
        virtual void MsgEditFieldChanged (EditField&) {}
    };
    
    //
    // MARK: Global Functions
    //
    
    /// Enumerates global colors, which can be changed using SetGlobalColor()
    enum ColCodeE {
        COL_STD_BG = 0,             ///< standard background (can be adjusted for each widget, see Widget::SetBgColor())
        COL_STD_FG,                 ///< standard foreground (can be adjusted for each widget, see Widget::SetFgColor())
        COL_STD_FG_DISABLED,        ///< standard foreground for disabled text (can be adjusted for each widget, see Widget::SetFgColor())
        COL_FOCUS_BORDER,           ///< thin border around focus widgets, default: red
        COL_BUTTON_BG,              ///< button's background, default: light blue
        COL_BUTTON_FG,              ///< button's text color, default: white
        COL_COLUMN_HEADER,          ///< List box's column header, default: white
        COL_SELECTION,              ///< selected items, default: light blue
        COL_ALTERNATE_ROW,          ///< applied to alternate list box rows, default: white with 90% transparency to just light up the background a bit
        COL_SCROLLBAR_BG,           ///< scrollbar's background
        COL_SCROLLBAR_FG,           ///< scrollbar's foreground
        COL_EDIT_BG,                ///< edit field's background
        COL_EDIT_FG,                ///< edit field's foreground
        COL_LINE,                   ///< divider line's color
        COL_NUMBER_OF_COLORS
    };
    
    /// Change globally defined colors for all widgets
    void SetGlobalColor (ColCodeE _code, const float _col[4]);

    /// Get globally defined color
    const float* GetGlobalColor (ColCodeE _code);

} // namespace "TFW"

#endif /* TFW_h */
