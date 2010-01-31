/****************************************************************************
**
** Copyright (C) 2010 Hugues Luc BRUANT aka fullmetalcoder 
**                    <non.deterministic.finite.organism@gmail.com>
** 
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and appearing in the
** file GPL.txt included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

extern "C" {
#include <scancodes.h>
}

#include <QKeyEvent>

#include "keymaploader.h"

/*!
	\file tilemkeymap.cpp
	\brief Tilem keymap loading data
*/

const char* pc_prefixes[] = {
	"PCKEY_",		// TiEmu/VTI compat
	0
};

const KeyName pc_keys[] = {
	{"Escape", Qt::Key_Escape},
	{"Tab", Qt::Key_Tab},
	{"Backtab", Qt::Key_Backtab},
	{"Backspace", Qt::Key_Backspace},
	{"Return", Qt::Key_Return},
	{"Enter", Qt::Key_Enter},
	{"Insert", Qt::Key_Insert},
	{"Delete", Qt::Key_Delete},
	{"Pause", Qt::Key_Pause},
	{"Print", Qt::Key_Print},
	{"SysReq", Qt::Key_SysReq},
	{"Clear", Qt::Key_Clear},
	{"Home", Qt::Key_Home},
	{"End", Qt::Key_End},
	{"Left", Qt::Key_Left},
	{"Up", Qt::Key_Up},
	{"Right", Qt::Key_Right},
	{"Down", Qt::Key_Down},
	{"PageUp", Qt::Key_PageUp},
	{"PageDown", Qt::Key_PageDown},
	{"Shift", Qt::Key_Shift},
	{"Control", Qt::Key_Control},
	{"Meta", Qt::Key_Meta},
	{"Alt", Qt::Key_Alt},
	{"AltGr", Qt::Key_AltGr},
	{"CapsLock", Qt::Key_CapsLock},
	{"NumLock", Qt::Key_NumLock},
	{"ScrollLock", Qt::Key_ScrollLock},
	{"F1", Qt::Key_F1},
	{"F2", Qt::Key_F2},
	{"F3", Qt::Key_F3},
	{"F4", Qt::Key_F4},
	{"F5", Qt::Key_F5},
	{"F6", Qt::Key_F6},
	{"F7", Qt::Key_F7},
	{"F8", Qt::Key_F8},
	{"F9", Qt::Key_F9},
	{"F10", Qt::Key_F10},
	{"F11", Qt::Key_F11},
	{"F12", Qt::Key_F12},
	{"F13", Qt::Key_F13},
	{"F14", Qt::Key_F14},
	{"F15", Qt::Key_F15},
	{"F16", Qt::Key_F16},
	{"F17", Qt::Key_F17},
	{"F18", Qt::Key_F18},
	{"F19", Qt::Key_F19},
	{"F20", Qt::Key_F20},
	{"F21", Qt::Key_F21},
	{"F22", Qt::Key_F22},
	{"F23", Qt::Key_F23},
	{"F24", Qt::Key_F24},
	{"F25", Qt::Key_F25},
	{"F26", Qt::Key_F26},
	{"F27", Qt::Key_F27},
	{"F28", Qt::Key_F28},
	{"F29", Qt::Key_F29},
	{"F30", Qt::Key_F30},
	{"F31", Qt::Key_F31},
	{"F32", Qt::Key_F32},
	{"F33", Qt::Key_F33},
	{"F34", Qt::Key_F34},
	{"F35", Qt::Key_F35},
	{"Super_L", Qt::Key_Super_L},
	{"Super_R", Qt::Key_Super_R},
	{"Menu", Qt::Key_Menu},
	{"Hyper_L", Qt::Key_Hyper_L},
	{"Hyper_R", Qt::Key_Hyper_R},
	{"Help", Qt::Key_Help},
	{"Direction_L", Qt::Key_Direction_L},
	{"Direction_R", Qt::Key_Direction_R},
	{"Space", Qt::Key_Space},
	{"Any", Qt::Key_Any},
	{"Exclam", Qt::Key_Exclam},
	{"QuoteDbl", Qt::Key_QuoteDbl},
	{"NumberSign", Qt::Key_NumberSign},
	{"Dollar", Qt::Key_Dollar},
	{"Percent", Qt::Key_Percent},
	{"Ampersand", Qt::Key_Ampersand},
	{"Apostrophe", Qt::Key_Apostrophe},
	{"ParenLeft", Qt::Key_ParenLeft},
	{"ParenRight", Qt::Key_ParenRight},
	{"Asterisk", Qt::Key_Asterisk},
	{"Plus", Qt::Key_Plus},
	{"Comma", Qt::Key_Comma},
	{"Minus", Qt::Key_Minus},
	{"Period", Qt::Key_Period},
	{"Slash", Qt::Key_Slash},
	{"0", Qt::Key_0},
	{"1", Qt::Key_1},
	{"2", Qt::Key_2},
	{"3", Qt::Key_3},
	{"4", Qt::Key_4},
	{"5", Qt::Key_5},
	{"6", Qt::Key_6},
	{"7", Qt::Key_7},
	{"8", Qt::Key_8},
	{"9", Qt::Key_9},
	{"Colon", Qt::Key_Colon},
	{"Semicolon", Qt::Key_Semicolon},
	{"Less", Qt::Key_Less},
	{"Equal", Qt::Key_Equal},
	{"Greater", Qt::Key_Greater},
	{"Question", Qt::Key_Question},
	{"At", Qt::Key_At},
	{"A", Qt::Key_A},
	{"B", Qt::Key_B},
	{"C", Qt::Key_C},
	{"D", Qt::Key_D},
	{"E", Qt::Key_E},
	{"F", Qt::Key_F},
	{"G", Qt::Key_G},
	{"H", Qt::Key_H},
	{"I", Qt::Key_I},
	{"J", Qt::Key_J},
	{"K", Qt::Key_K},
	{"L", Qt::Key_L},
	{"M", Qt::Key_M},
	{"N", Qt::Key_N},
	{"O", Qt::Key_O},
	{"P", Qt::Key_P},
	{"Q", Qt::Key_Q},
	{"R", Qt::Key_R},
	{"S", Qt::Key_S},
	{"T", Qt::Key_T},
	{"U", Qt::Key_U},
	{"V", Qt::Key_V},
	{"W", Qt::Key_W},
	{"X", Qt::Key_X},
	{"Y", Qt::Key_Y},
	{"Z", Qt::Key_Z},
	{"BracketLeft", Qt::Key_BracketLeft},
	{"Backslash", Qt::Key_Backslash},
	{"BracketRight", Qt::Key_BracketRight},
	{"AsciiCircum", Qt::Key_AsciiCircum},
	{"Underscore", Qt::Key_Underscore},
	{"QuoteLeft", Qt::Key_QuoteLeft},
	{"BraceLeft", Qt::Key_BraceLeft},
	{"Bar", Qt::Key_Bar},
	{"BraceRight", Qt::Key_BraceRight},
	{"AsciiTilde", Qt::Key_AsciiTilde},
	{"nobreakspace", Qt::Key_nobreakspace},
	{"exclamdown", Qt::Key_exclamdown},
	{"cent", Qt::Key_cent},
	{"sterling", Qt::Key_sterling},
	{"currency", Qt::Key_currency},
	{"yen", Qt::Key_yen},
	{"brokenbar", Qt::Key_brokenbar},
	{"section", Qt::Key_section},
	{"diaeresis", Qt::Key_diaeresis},
	{"copyright", Qt::Key_copyright},
	{"ordfeminine", Qt::Key_ordfeminine},
	{"guillemotleft", Qt::Key_guillemotleft},
	{"notsign", Qt::Key_notsign},
	{"hyphen", Qt::Key_hyphen},
	{"registered", Qt::Key_registered},
	{"macron", Qt::Key_macron},
	{"degree", Qt::Key_degree},
	{"plusminus", Qt::Key_plusminus},
	{"twosuperior", Qt::Key_twosuperior},
	{"threesuperior", Qt::Key_threesuperior},
	{"acute", Qt::Key_acute},
	{"mu", Qt::Key_mu},
	{"paragraph", Qt::Key_paragraph},
	{"periodcentered", Qt::Key_periodcentered},
	{"cedilla", Qt::Key_cedilla},
	{"onesuperior", Qt::Key_onesuperior},
	{"masculine", Qt::Key_masculine},
	{"guillemotright", Qt::Key_guillemotright},
	{"onequarter", Qt::Key_onequarter},
	{"onehalf", Qt::Key_onehalf},
	{"threequarters", Qt::Key_threequarters},
	{"questiondown", Qt::Key_questiondown},
	{"Agrave", Qt::Key_Agrave},
	{"Aacute", Qt::Key_Aacute},
	{"Acircumflex", Qt::Key_Acircumflex},
	{"Atilde", Qt::Key_Atilde},
	{"Adiaeresis", Qt::Key_Adiaeresis},
	{"Aring", Qt::Key_Aring},
	{"AE", Qt::Key_AE},
	{"Ccedilla", Qt::Key_Ccedilla},
	{"Egrave", Qt::Key_Egrave},
	{"Eacute", Qt::Key_Eacute},
	{"Ecircumflex", Qt::Key_Ecircumflex},
	{"Ediaeresis", Qt::Key_Ediaeresis},
	{"Igrave", Qt::Key_Igrave},
	{"Iacute", Qt::Key_Iacute},
	{"Icircumflex", Qt::Key_Icircumflex},
	{"Idiaeresis", Qt::Key_Idiaeresis},
	{"ETH", Qt::Key_ETH},
	{"Ntilde", Qt::Key_Ntilde},
	{"Ograve", Qt::Key_Ograve},
	{"Oacute", Qt::Key_Oacute},
	{"Ocircumflex", Qt::Key_Ocircumflex},
	{"Otilde", Qt::Key_Otilde},
	{"Odiaeresis", Qt::Key_Odiaeresis},
	{"multiply", Qt::Key_multiply},
	{"Ooblique", Qt::Key_Ooblique},
	{"Ugrave", Qt::Key_Ugrave},
	{"Uacute", Qt::Key_Uacute},
	{"Ucircumflex", Qt::Key_Ucircumflex},
	{"Udiaeresis", Qt::Key_Udiaeresis},
	{"Yacute", Qt::Key_Yacute},
	{"THORN", Qt::Key_THORN},
	{"ssharp", Qt::Key_ssharp},
	{"division", Qt::Key_division},
	{"ydiaeresis", Qt::Key_ydiaeresis},
	{"Multi_key", Qt::Key_Multi_key},
	{"Codeinput", Qt::Key_Codeinput},
	{"SingleCandidate", Qt::Key_SingleCandidate},
	{"MultipleCandidate", Qt::Key_MultipleCandidate},
	{"PreviousCandidate", Qt::Key_PreviousCandidate},
	{"Mode_switch", Qt::Key_Mode_switch},
	{"Kanji", Qt::Key_Kanji},
	{"Muhenkan", Qt::Key_Muhenkan},
	{"Henkan", Qt::Key_Henkan},
	{"Romaji", Qt::Key_Romaji},
	{"Hiragana", Qt::Key_Hiragana},
	{"Katakana", Qt::Key_Katakana},
	{"Hiragana_Katakana", Qt::Key_Hiragana_Katakana},
	{"Zenkaku", Qt::Key_Zenkaku},
	{"Hankaku", Qt::Key_Hankaku},
	{"Zenkaku_Hankaku", Qt::Key_Zenkaku_Hankaku},
	{"Touroku", Qt::Key_Touroku},
	{"Massyo", Qt::Key_Massyo},
	{"Kana_Lock", Qt::Key_Kana_Lock},
	{"Kana_Shift", Qt::Key_Kana_Shift},
	{"Eisu_Shift", Qt::Key_Eisu_Shift},
	{"Eisu_toggle", Qt::Key_Eisu_toggle},
	{"Hangul", Qt::Key_Hangul},
	{"Hangul_Start", Qt::Key_Hangul_Start},
	{"Hangul_End", Qt::Key_Hangul_End},
	{"Hangul_Hanja", Qt::Key_Hangul_Hanja},
	{"Hangul_Jamo", Qt::Key_Hangul_Jamo},
	{"Hangul_Romaja", Qt::Key_Hangul_Romaja},
	{"Hangul_Jeonja", Qt::Key_Hangul_Jeonja},
	{"Hangul_Banja", Qt::Key_Hangul_Banja},
	{"Hangul_PreHanja", Qt::Key_Hangul_PreHanja},
	{"Hangul_PostHanja", Qt::Key_Hangul_PostHanja},
	{"Hangul_Special", Qt::Key_Hangul_Special},
	{"Dead_Grave", Qt::Key_Dead_Grave},
	{"Dead_Acute", Qt::Key_Dead_Acute},
	{"Dead_Circumflex", Qt::Key_Dead_Circumflex},
	{"Dead_Tilde", Qt::Key_Dead_Tilde},
	{"Dead_Macron", Qt::Key_Dead_Macron},
	{"Dead_Breve", Qt::Key_Dead_Breve},
	{"Dead_Abovedot", Qt::Key_Dead_Abovedot},
	{"Dead_Diaeresis", Qt::Key_Dead_Diaeresis},
	{"Dead_Abovering", Qt::Key_Dead_Abovering},
	{"Dead_Doubleacute", Qt::Key_Dead_Doubleacute},
	{"Dead_Caron", Qt::Key_Dead_Caron},
	{"Dead_Cedilla", Qt::Key_Dead_Cedilla},
	{"Dead_Ogonek", Qt::Key_Dead_Ogonek},
	{"Dead_Iota", Qt::Key_Dead_Iota},
	{"Dead_Voiced_Sound", Qt::Key_Dead_Voiced_Sound},
	{"Dead_Semivoiced_Sound", Qt::Key_Dead_Semivoiced_Sound},
	{"Dead_Belowdot", Qt::Key_Dead_Belowdot},
	{"Dead_Hook", Qt::Key_Dead_Hook},
	{"Dead_Horn", Qt::Key_Dead_Horn},
	{"Back", Qt::Key_Back},
	{"Forward", Qt::Key_Forward},
	{"Stop", Qt::Key_Stop},
	{"Refresh", Qt::Key_Refresh},
	{"VolumeDown", Qt::Key_VolumeDown},
	{"VolumeMute", Qt::Key_VolumeMute},
	{"VolumeUp", Qt::Key_VolumeUp},
	{"BassBoost", Qt::Key_BassBoost},
	{"BassUp", Qt::Key_BassUp},
	{"BassDown", Qt::Key_BassDown},
	{"TrebleUp", Qt::Key_TrebleUp},
	{"TrebleDown", Qt::Key_TrebleDown},
	{"MediaPlay", Qt::Key_MediaPlay},
	{"MediaStop", Qt::Key_MediaStop},
	{"MediaPrevious", Qt::Key_MediaPrevious},
	{"MediaNext", Qt::Key_MediaNext},
	{"MediaRecord", Qt::Key_MediaRecord},
	{"HomePage", Qt::Key_HomePage},
	{"Favorites", Qt::Key_Favorites},
	{"Search", Qt::Key_Search},
	{"Standby", Qt::Key_Standby},
	{"OpenUrl", Qt::Key_OpenUrl},
	{"LaunchMail", Qt::Key_LaunchMail},
	{"LaunchMedia", Qt::Key_LaunchMedia},
	{"Launch0", Qt::Key_Launch0},
	{"Launch1", Qt::Key_Launch1},
	{"Launch2", Qt::Key_Launch2},
	{"Launch3", Qt::Key_Launch3},
	{"Launch4", Qt::Key_Launch4},
	{"Launch5", Qt::Key_Launch5},
	{"Launch6", Qt::Key_Launch6},
	{"Launch7", Qt::Key_Launch7},
	{"Launch8", Qt::Key_Launch8},
	{"Launch9", Qt::Key_Launch9},
	{"LaunchA", Qt::Key_LaunchA},
	{"LaunchB", Qt::Key_LaunchB},
	{"LaunchC", Qt::Key_LaunchC},
	{"LaunchD", Qt::Key_LaunchD},
	{"LaunchE", Qt::Key_LaunchE},
	{"LaunchF", Qt::Key_LaunchF},
	{"MediaLast", Qt::Key_MediaLast},
	{"unknown", Qt::Key_unknown},
	{"Call", Qt::Key_Call},
	{"Context1", Qt::Key_Context1},
	{"Context2", Qt::Key_Context2},
	{"Context3", Qt::Key_Context3},
	{"Context4", Qt::Key_Context4},
	{"Flip", Qt::Key_Flip},
	{"Hangup", Qt::Key_Hangup},
	{"No", Qt::Key_No},
	{"Select", Qt::Key_Select},
	{"Yes", Qt::Key_Yes},
	{0, 0}
};

const char* ti_prefixes[] = {
	"TIKEY_",		// TiEmu/VTI compat
	"TILEM_KEY_",
	0
};

const KeyName ti_keys[] = {
	{"DOWN", TILEM_KEY_DOWN},
	{"LEFT", TILEM_KEY_LEFT},
	{"RIGHT", TILEM_KEY_RIGHT},
	{"UP", TILEM_KEY_UP},
	{"ENTER", TILEM_KEY_ENTER},
	{"ADD", TILEM_KEY_ADD},
	{"SUB", TILEM_KEY_SUB},
	{"MUL", TILEM_KEY_MUL},
	{"DIV", TILEM_KEY_DIV},
	{"POWER", TILEM_KEY_POWER},
	{"CLEAR", TILEM_KEY_CLEAR},
	{"CHS", TILEM_KEY_CHS},
	{"3", TILEM_KEY_3},
	{"6", TILEM_KEY_6},
	{"9", TILEM_KEY_9},
	{"RPAREN", TILEM_KEY_RPAREN},
	{"TAN", TILEM_KEY_TAN},
	{"VARS", TILEM_KEY_VARS},
	{"DECPNT", TILEM_KEY_DECPNT},
	{"2", TILEM_KEY_2},
	{"5", TILEM_KEY_5},
	{"8", TILEM_KEY_8},
	{"LPAREN", TILEM_KEY_LPAREN},
	{"COS", TILEM_KEY_COS},
	{"PRGM", TILEM_KEY_PRGM},
	{"STAT", TILEM_KEY_STAT},
	{"0", TILEM_KEY_0},
	{"1", TILEM_KEY_1},
	{"4", TILEM_KEY_4},
	{"7", TILEM_KEY_7},
	{"COMMA", TILEM_KEY_COMMA},
	{"SIN", TILEM_KEY_SIN},
	{"MATRIX", TILEM_KEY_MATRIX},
	{"GRAPHVAR", TILEM_KEY_GRAPHVAR},
	{"ON", TILEM_KEY_ON},
	{"STORE", TILEM_KEY_STORE},
	{"LN", TILEM_KEY_LN},
	{"LOG", TILEM_KEY_LOG},
	{"SQUARE", TILEM_KEY_SQUARE},
	{"RECIP", TILEM_KEY_RECIP},
	{"MATH", TILEM_KEY_MATH},
	{"ALPHA", TILEM_KEY_ALPHA},
	{"GRAPH", TILEM_KEY_GRAPH},
	{"TRACE", TILEM_KEY_TRACE},
	{"ZOOM", TILEM_KEY_ZOOM},
	{"WINDOW", TILEM_KEY_WINDOW},
	{"YEQU", TILEM_KEY_YEQU},
	{"2ND", TILEM_KEY_2ND},
	{"MODE", TILEM_KEY_MODE},
	{"DEL", TILEM_KEY_DEL},
	
	// VTI/Tiemu compat
	{"PLUS", TILEM_KEY_ADD},
	{"MULTIPLY", TILEM_KEY_MUL},
	{"DIVIDE", TILEM_KEY_DIV},
	{"MINUS", TILEM_KEY_SUB},
	
	{0, 0}
};

extern const KeySpace ti_space = {
	ti_prefixes,
	ti_keys
};

extern const KeySpace pc_space = {
	pc_prefixes,
	pc_keys
};