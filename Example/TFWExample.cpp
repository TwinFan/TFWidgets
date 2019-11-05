//
//  LiveTraffic API Example Plugin
//

/*
 * Copyright (c) 2019, Birger Hoppe
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// This example plugin is based on the Hello World SDK 3 plugin
// downloaded from https://developer.x-plane.com/code-sample/hello-world-sdk-3/
// and comes with no copyright notice.

// It is changed, however, to also work with SDK 2.10, i.e. XP10
// But you can also define all up to XPLM301, if you want.
#ifndef XPLM301
#error This is made to be compiled at least against the XPLM301 SDK
#endif

// include TFW header
#include "TFW.h"

// include other C++ headers
#include <memory>
#include <list>

// include X-Plane SDK headers
#include "XPLMMenus.h"
#include "XPLMPlugin.h"

//
// MARK: Declaration of example window
//

class TheWnd : public TFW::MainWnd
{
protected:
    // widgets in use by the window, in order of appearance,
    // as this is also the order of initialization in the constructor,
    // which is important when using `TFW::POS_AFTER_PRV`:
    TFW::Label lab1, lab2, lab3;
    TFW::ListBox list;
    TFW::ButtonArray barCheck;
    
    TFW::Line line;

    TFW::ButtonArray barRadio;
    TFW::EditField edit, pwd;
    TFW::Label pwdClearText;
    
    std::list< std::shared_ptr<TFW::Widget> > aWidgets;
    
    TFW::Line linButtons;
    TFW::ButtonPush btnOK, btnClear, btnDisabled;
public:
    /// Constructor
    TheWnd ();
    
    /// Enable OK when selected something
    virtual void MsgListSelChanged (TFW::ListBox&, TFW::ListRow&) { btnOK.SetEnabled(true); }
    
    /// Clear list or hide window
    virtual void MsgButtonClicked (TFW::ButtonPush& _btn);
    
    /// Update password clear text
    virtual void MsgEditFieldChanged (TFW::EditField& _edit);
};

//
// MARK: Implementation of example window
//

/// The global plane selection window
std::unique_ptr<TheWnd> pTheWnd;

TheWnd::TheWnd () :
TFW::MainWnd("TFWidget Example",    // Title of window
             0, 300, 550, 0,        // coordinates only used to define size
             xplm_WindowDecorationRoundRectangle,
             xplm_WindowLayerFloatingWindows),
// definition of included widgets in order of appearance in the window:
lab1("Standard Text", *this, TFW::POS_ABSOLUTE, 0, TFW::POS_AFTER_PRV, 0, TFW::POS_FIXED_LEN, 100),
lab2("Disabled Text", *this, TFW::POS_ABSOLUTE, 0, TFW::POS_AFTER_PRV, 0, TFW::POS_FIXED_LEN, 100),
lab3("Bold Text", *this,     TFW::POS_ABSOLUTE, 0, TFW::POS_AFTER_PRV, 0, TFW::POS_FIXED_LEN, 100),
// List box: Right-hand side, 20 boxels next to the above labels, extending to the bottom/right corner
list("List Box", *this,
        TFW::POS_AFTER_PRV, 20,        TFW::POS_ABSOLUTE,      0,
        TFW::POS_PERCENT, 80,          TFW::POS_BOTTOM_RIGHT, 90),
// Checkboxes to the right of the list box
barCheck("Checkboxes", *this,
        TFW::BTN_CHECK_BOXES, true,
        TFW::POS_AFTER_PRV, 20,        TFW::POS_SAME_PRV,       0,
        TFW::POS_BOTTOM_RIGHT, 0,      TFW::POS_SAME_PRV,       0),
// Divider Line under list box and check boxes
line("line", *this,
        TFW::POS_ABSOLUTE, 120,        TFW::POS_BOTTOM_RIGHT, 88,
        TFW::POS_BOTTOM_RIGHT, 0,      TFW::POS_FIXED_LEN,     0),
// Radio buttons under the list box
barRadio("Radio Buttons", *this,
         TFW::BTN_RADIO_BUTTONS, false,
         TFW::POS_ABSOLUTE, 120,        TFW::POS_FIXED_LEN,    15,
         TFW::POS_BOTTOM_RIGHT, 0,      TFW::POS_BOTTOM_RIGHT, 70),
// Edit field under the radio buttons
edit("Edit this text", "", 80, *this,
        TFW::POS_ABSOLUTE, 120,         TFW::POS_FIXED_LEN,    15,
        TFW::POS_BOTTOM_RIGHT, 0,       TFW::POS_BOTTOM_RIGHT, 50),
// Password field under the edit field
pwd("Password", "", 20, *this,
        TFW::POS_ABSOLUTE, 120,         TFW::POS_FIXED_LEN,    15,
        TFW::POS_BOTTOM_RIGHT, -100,    TFW::POS_BOTTOM_RIGHT, 30),
// Password clear text label to the right of password entry
pwdClearText("Password", *this,
        TFW::POS_AFTER_PRV, 20,         TFW::POS_SAME_PRV, 0,
        TFW::POS_BOTTOM_RIGHT, 0,       TFW::POS_SAME_PRV, 0),

// Row of buttons at the bottom
linButtons("Line Buttons", *this,
        TFW::POS_ABSOLUTE,     0,       TFW::POS_BOTTOM_RIGHT, 23,
        TFW::POS_BOTTOM_RIGHT, 0,       TFW::POS_FIXED_LEN,     0),
btnOK("OK", *this,
        TFW::POS_FIXED_LEN,     100,    TFW::POS_FIXED_LEN,     15,
        TFW::POS_BOTTOM_RIGHT,    0,    TFW::POS_BOTTOM_RIGHT,   0),
btnClear("Delete All", *this,
        TFW::POS_FIXED_LEN,     100,    TFW::POS_SAME_PRV,      0,
        TFW::POS_BOTTOM_RIGHT, -110,    TFW::POS_SAME_PRV,      0),
btnDisabled("Disabled & too long", *this,
        TFW::POS_FIXED_LEN,     100,    TFW::POS_SAME_PRV,      0,
        TFW::POS_BOTTOM_RIGHT, -220,    TFW::POS_SAME_PRV,      0)
{
    lab2.SetEnabled(false);
    //lab2.bBold = true;
    lab3.bBold = true;
    
    // Here we add a number of labels colored in the XP standard colors
    // to get an idea of what they looks like
    // They appear to the left (POS_ABSOLUTE, 0)
    for (TFW::XPStdColorsE eCol = TFW::COL_XP_BACKGROUND_RGB;
         eCol < TFW::COL_XP_COUNT;
         eCol = TFW::XPStdColorsE(int(eCol)+1))
    {
        // Label with text color set to XP's color
        aWidgets.emplace_back(new TFW::Label(std::to_string(eCol), *this, TFW::POS_ABSOLUTE, 0,
                                             // first of these color boxes is placed with absolute y coordinates, the other follow thereafter:
                                             !eCol ? TFW::POS_ABSOLUTE : TFW::POS_AFTER_PRV, !eCol ? -50 : 0,
                                             TFW::POS_FIXED_LEN, 50,                         TFW::POS_FIXED_LEN, 15));
        aWidgets.back()->SetFgColor(eCol,TFW::COL_XP_MENU_TEXT_DISABLED_RGB);
        // Label with Background color set to XP's color, so it becomes a coloured box
        aWidgets.emplace_back(new TFW::Label(std::to_string(eCol), *this, TFW::POS_AFTER_PRV, 0, TFW::POS_SAME_PRV, 0, TFW::POS_FIXED_LEN, 50, TFW::POS_SAME_PRV, 0));
        aWidgets.back()->SetBkColor(eCol);
    }
    
    // Some basic tests of Listbox code
    
    enum { COL_NAME=0, COL_LOCATION, COL_BOARDED, COL_PAX, COL_FUEL, COL_TYPE };
    list.AddCol(COL_NAME,       {"Nome",     50});
    list.AddCol(COL_PAX,        {"Pax",      30, TFW::DTY_INT});     // one empty in-between
    list.AddCol(COL_LOCATION,   {"Location", 60});                   // fill up the empty slot
    list.AddCol(COL_BOARDED,    {"Boarded",  40, TFW::DTY_CHECKBOX});
    list.AddCol(COL_FUEL,       {"Fuel",     60, TFW::DTY_DOUBLE});
    list.AddCol(COL_TYPE,       {"Type",     40});   // this is a string, i.e. a left-aligned columnd following a right-aligned column, which requires some extra space so that text doesn't directly connect
    list.AddCol(COL_NAME,       {"Name",    100});   // overwrite first element

    // Add some rows
    list.AddRow({(void*)1, {{"Cessna 152"},     {"EDDL"}, {true},   {2},  {123.456}, {"C152"}}});
    list.AddRow({(void*)2, {{"Airbus A320"},    {"EDDF"}, {true}, {186}, {4123.456}, {"A320"}}});
    list.AddRow({(void*)3, {{"ATR 72"},         {"KATL"}, {true},  {70},  {412.000}, {"AT72"}}});
    
    // set sorting
    list.SetSorting(1, TFW::SORT_DESCENDING);
    
    // add an item to sorted list
    list.AddRow({(void*)4, {{"Boing 737 MAX"},  {"KMHV"}, {false}, {189}, {5.000}, {"B737"}}});
    
    // change sorting
    list.SetSorting(1, TFW::SORT_ASCENDING);

    // update an existing item to sorted list
    list.AddRow({(void*)2, {{"Airbus A320neo"}, {"EDDF"}, {false}, {186}, {4123.456},{"A20N"}}});
    
    // add many rows for scrolling
    //                      COL_NAME     COL_LOCATION COL_PAX  COL_FUEL  COL_TYPE
    list.AddRow({(void*)5, {{"Cessna 172"}, {"EDLE"}, {true},   {4},  {123.456}, {"C172"}}});
    list.AddRow({(void*)6, {{"Airbus A380"},{"EDDF"}, {true}, {868}, {4123.456}, {"A380"}}});
    list.AddRow({(void*)7, {{"Very long a name"},      {"Too long text"},  {false}, {1699999},  {41299999.000}, {"Extended Type"}}});
    list.AddRow({(void*)8, {{"Eight"},      {"EDLE"}, {false},  {8},  {123.456}, {"red"}}});
    list.AddRow({(void*)9, {{"Nine"},       {"EDDF"}, {false},  {9}, {4123.456}, {"green"}}});
    TFW::ListRow& tenthRow = list.AddRow({(void*)10,{{"Ten"}, {"EDLE"}, {true}, {10},  {412.000}, {"blue"}}});
    
    // set color of some items
    (*list.FindRow((void*)8))[COL_TYPE].SetColor(TFW::COL_RED);
    (*list.FindRow((void*)8))[COL_NAME].SetColor(TFW::COL_BLUE);
    (*list.FindRow((void*)9))[COL_TYPE].SetColor(TFW::COL_GREEN);
    (*list.FindRow((void*)9))[COL_NAME].SetColor(TFW::COL_YELLOW);
    tenthRow[COL_TYPE].SetColor(TFW::COL_BLUE);
    tenthRow[COL_NAME].SetColor(TFW::COL_CYAN);

    
    // Check boxes
    barCheck.AddButton({(void*)1, "First"});
    barCheck.AddButton({(void*)2, "Second", TFW::BTN_SELECTED});
    barCheck.AddButton({(void*)3, "Third",  TFW::BTN_SELECTED});
    
    // Radio Buttons
    barRadio.AddButton({(void*)1, "Left"});
    barRadio.AddButton({(void*)2, "Center"});
    barRadio.AddButton({(void*)3, "Right"});
    barRadio.SetSelected((void*)3);
    
    // make password edit field hide its content
    pwd.bPwdMode = true;

    // inactive button
    btnOK.SetEnabled(false);
    btnDisabled.SetEnabled(false);

    // default upon [Enter] is OK
    SetDefaultButton(&btnOK);
}


void TheWnd::MsgButtonClicked (TFW::ButtonPush& _btn)
{
    // hide the window?
    if (_btn == btnOK) {
        SetVisible(false);
    }
    // clear the list?
    else if (_btn == btnClear) {
        list.Clear();
        btnOK.SetEnabled(false);
    }
}


// Update password clear text
void TheWnd::MsgEditFieldChanged (TFW::EditField& _edit)
{
    if (_edit == pwd) {
        pwdClearText.SetCaption(_edit.GetCaption());
    }
}

//
// MARK: Global Functions
//

void OpenAndDisplayTheWnd ()
{
    // Create the UI object if needed
    if (!pTheWnd)
        pTheWnd.reset(new TheWnd());
    
    // make visible and place in the middle of main screen
    pTheWnd->SetCenterFloat();
}

// Cleanup work when disabling, destroys selection UI
void RemoveTheWnd()
{
    pTheWnd.reset(nullptr);
}

// Move the window into and out of VR
void MoveWndVR (bool _bIntoVR)
{
    if (pTheWnd) {
        if (_bIntoVR)
            pTheWnd->MoveIntoVR();
        else
            pTheWnd->MoveOutOfVR();
    }
}


//
// MARK: Handle one menu entry
//

void CBMenuHandler(void * /*mRef*/, void * /*iRef*/)
{
    // we don't check any id here as we only do one thing:
    OpenAndDisplayTheWnd();
}

void RegisterMenuEntry ()
{
    // submenu in "Plugins" menu
    int mainItemId = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "TFWidgets", NULL, 1);
    XPLMMenuID mainMenuId = XPLMCreateMenu("TFWidgets", XPLMFindPluginsMenu(), mainItemId, CBMenuHandler, NULL);
    
    // item to open the window
    XPLMAppendMenuItem(mainMenuId, "Open Window...", NULL,1 );
}

//
// MARK: Plugin Main Functions
//       in typical order of execution
//

PLUGIN_API int XPluginStart(
							char *		outName,
							char *		outSig,
							char *		outDesc)
{
    // tell the world who we are
	strcpy(outName, "TFWidgets Example");
	strcpy(outSig, "TwinFan.plugin.TFWExample");
	strcpy(outDesc, "Example plugin using TFWidgets");
    
    // register a menu item to open the example window in case it got closed
    RegisterMenuEntry();

    // success
	return 1;
}

PLUGIN_API int  XPluginEnable(void)
{
    // we open the window upon startup
    OpenAndDisplayTheWnd();
    
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * /*inParam*/)
{
    // we only process msgs from X-Plane
    if (inFrom != XPLM_PLUGIN_XPLANE)
        return;
        
    switch (inMsg) {
        // *** entering VR mode ***
        case XPLM_MSG_ENTERED_VR:
            MoveWndVR(true);
            break;
            
        // *** existing from VR mode ***
        case XPLM_MSG_EXITING_VR:
            MoveWndVR(false);
            break;
    }
}

PLUGIN_API void XPluginDisable(void)
{
    RemoveTheWnd();
}

PLUGIN_API void    XPluginStop(void)
{
}


