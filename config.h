/* See LICENSE file for copyright and license details. */

#include "push.c"
#include "gaplessgrid.c"
#include "moveresize.c"

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;     /* 0 means no systray */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char buttonbar[]       = "Menu";
static const char *fonts[]          = { "GohuFont:size=12", "Terminus:size=12" };
static const char dmenufont[]       = "GohuFont:size=12";
static const char col_gray1[]       = "#2B2E37";
static const char col_gray2[]       = "#252730";
static const char col_gray3[]       = "#eeeeee";
static const char col_gray4[]       = "#dddddd";
static const char col_cyan[]        = "#4084D6";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray4, col_gray1, col_gray1 },
	[SchemeSel]  = { col_cyan, col_gray2,  col_gray2  },
};

/* tagging */
//static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static const char *tags[] = { "Main", "Term", "Code", "Math", "Doc", "Web", "Chat", "Media", "Sys" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class			instance	title	tags mask	iscentered	isfloating	monitor */
	{ "URxvt",			NULL,		NULL,	1 << 1,		1,		0,		-1 },
	{ "st-256color",	NULL,		NULL,	1 << 1,		1,		0,		-1 },
	{ "acme",			NULL,		NULL,	1 << 2,		1,		0,		-1 },
	{ "vim",			NULL,		NULL,	1 << 2,		1,		0,		-1 },
	{ "gvim",			NULL,		NULL,	1 << 2,		1,		0,		-1 },
	{ "vis",			NULL,		NULL,	1 << 2,		1,		0,		-1 },
	{ "neovim",			NULL,		NULL,	1 << 2,		1,		0,		-1 },
	{ "emacs",			NULL,		NULL,	1 << 2,		1,		0,		-1 },
	{ "lyx",			NULL,		NULL,	1 << 3,		1,		0,		-1 },
	{ "TeXstudio",		NULL,		NULL,	1 << 3,		1,		0,		-1 },
	{ "soffice",		NULL,		NULL,	1 << 4,		1,		0,		-1 },
	{ "LibreOffice",	NULL,		NULL,	1 << 4,		1,		0,		-1 },
	{ "zathura",		NULL,		NULL,	1 << 4,		1,		0,		-1 },
	{ "Firefox",		NULL,		NULL,	1 << 5,		1,		0,		-1 },
	{ "surf",			NULL,		NULL,	1 << 5,		1,		0,		-1 },
	{ "Chromium",		NULL,		"Authy",1 << 5,		1,		1,		-1 },
	{ "discord",		NULL,		NULL,	1 << 6,		1,		0,		-1 },
	{ "mpv",			NULL,		NULL,	1 << 7,		1,		0,		-1 },
	{ "feh",			NULL,		NULL,	1 << 7,		1,		0,		-1 },
	{ "Gimp",			NULL,		NULL,	1 << 7,		0,		1,		-1 },
	{ "virt-manager",	NULL,		NULL,	1 << 8,		0,		0,		-1 },
};

/* layout(s) */
static const float mfact     = 0.50; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol	arrange function */
	{ "Grid",	gaplessgrid },    /* first entry is default */
	{ "Float",	NULL },    /* no layout function means floating behavior */
	{ "Monocle",	monocle },
	{ "Tile",	tile },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

#define XF86AudioMute				0x1008ff12
#define XF86AudioLowerVolume		0x1008ff11
#define XF86AudioRaiseVolume		0x1008ff13

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run_history", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *dmlcmd[]  = { "dmenu-launch", NULL };
static const char *morccmd[]  = { "morc_menu", NULL };
static const char *viacmd[] = { "via", NULL };
static const char *termcmd[]  = { "st", NULL };
static const char scratchpadname[] = "scratchpad";
static const char *scratchpadcmd[] = { "st", "-t", scratchpadname, "-g", "120x34", NULL };
static const char *vup[] =           { "pactl", "set-sink-volume", "alsa_output.usb-Plantronics_Plantronics_GameCom_780-00.analog-stereo", "+5%", NULL };
static const char *vdown[] =         { "pactl", "set-sink-volume", "alsa_output.usb-Plantronics_Plantronics_GameCom_780-00.analog-stereo", "-5%", NULL };
static const char *vmute[] =         { "pactl", "set-sink-mute", "alsa_output.usb-Plantronics_Plantronics_GameCom_780-00.analog-stereo", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_p,      spawn,          {.v = dmlcmd } },
	{ MODKEY,                       XK_v,      spawn,          {.v = viacmd } },
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_grave,  togglescratch,  {.v = scratchpadcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_h,      setcfact,       {.f = +0.25} },
	{ MODKEY|ShiftMask,             XK_l,      setcfact,       {.f = -0.25} },
	{ MODKEY|ShiftMask,             XK_o,      setcfact,       {.f =  0.00} },
	{ MODKEY|ShiftMask,             XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
	{ MODKEY|ControlMask|ShiftMask, XK_q,      quit,           {1} },
	{ 0,                       XF86AudioMute,  spawn,          {.v = vmute } },
	{ 0,                XF86AudioRaiseVolume,  spawn,          {.v = vup } },
	{ 0,                XF86AudioLowerVolume,  spawn, 	       {.v = vdown } },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkButton,			0,				Button1,		spawn,			{.v = morccmd } },
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

static const char *dwmfifo = "/tmp/dwm.fifo";
static Command commands[] = {
	{ "dmenu",           spawn,          {.v = dmenucmd} },
	{ "term",            spawn,          {.v = termcmd} },
	{ "quit",            quit,           {0} },
	{ "togglebar",       togglebar,      {0} },
	{ "focusstack+",     focusstack,     {.i = +1} },
	{ "focusstack-",     focusstack,     {.i = -1} },
	{ "incnmaster+",     incnmaster,     {.i = +1} },
	{ "incnmaster-",     incnmaster,     {.i = -1} },
	{ "setmfact+",       setmfact,       {.f = +0.05} },
	{ "setmfact-",       setmfact,       {.f = -0.05} },
	{ "zoom",            zoom,           {0} },
	{ "view",            view,           {0} },
	{ "killclient",      killclient,     {0} },
	{ "setlayout-grid",  setlayout,      {.v = &layouts[0]} },
	{ "setlayout-float", setlayout,      {.v = &layouts[1]} },
	{ "setlayout-mono",  setlayout,      {.v = &layouts[2]} },
	{ "setlayout-tiled", setlayout,      {.v = &layouts[3]} },
	{ "togglelayout",    setlayout,      {0} },
	{ "togglefloating",  togglefloating, {0} },
	{ "viewall",         view,           {.ui = ~0} },
	{ "tag",             tag,            {.ui = ~0} },
	{ "focusmon+",       focusmon,       {.i = +1} },
	{ "focusmon-",       focusmon,       {.i = -1} },
	{ "tagmon+",         tagmon,         {.i = +1} },
	{ "tagmon-",         tagmon,         {.i = -1} },
	{ "view1",           view,           {.ui = 1 << 0} },
	{ "view2",           view,           {.ui = 1 << 1} },
	{ "view3",           view,           {.ui = 1 << 2} },
	{ "view4",           view,           {.ui = 1 << 3} },
	{ "view5",           view,           {.ui = 1 << 4} },
	{ "view6",           view,           {.ui = 1 << 5} },
	{ "view7",           view,           {.ui = 1 << 6} },
	{ "view8",           view,           {.ui = 1 << 7} },
	{ "view9",           view,           {.ui = 1 << 8} },
	{ "toggleview1",     toggleview,     {.ui = 1 << 0} },
	{ "toggleview2",     toggleview,     {.ui = 1 << 1} },
	{ "toggleview3",     toggleview,     {.ui = 1 << 2} },
	{ "toggleview4",     toggleview,     {.ui = 1 << 3} },
	{ "toggleview5",     toggleview,     {.ui = 1 << 4} },
	{ "toggleview6",     toggleview,     {.ui = 1 << 5} },
	{ "toggleview7",     toggleview,     {.ui = 1 << 6} },
	{ "toggleview8",     toggleview,     {.ui = 1 << 7} },
	{ "toggleview9",     toggleview,     {.ui = 1 << 8} },
	{ "tag1",            tag,            {.ui = 1 << 0} },
	{ "tag2",            tag,            {.ui = 1 << 1} },
	{ "tag3",            tag,            {.ui = 1 << 2} },
	{ "tag4",            tag,            {.ui = 1 << 3} },
	{ "tag5",            tag,            {.ui = 1 << 4} },
	{ "tag6",            tag,            {.ui = 1 << 5} },
	{ "tag7",            tag,            {.ui = 1 << 6} },
	{ "tag8",            tag,            {.ui = 1 << 7} },
	{ "tag9",            tag,            {.ui = 1 << 8} },
	{ "toggletag1",      toggletag,      {.ui = 1 << 0} },
	{ "toggletag2",      toggletag,      {.ui = 1 << 1} },
	{ "toggletag3",      toggletag,      {.ui = 1 << 2} },
	{ "toggletag4",      toggletag,      {.ui = 1 << 3} },
	{ "toggletag5",      toggletag,      {.ui = 1 << 4} },
	{ "toggletag6",      toggletag,      {.ui = 1 << 5} },
	{ "toggletag7",      toggletag,      {.ui = 1 << 6} },
	{ "toggletag8",      toggletag,      {.ui = 1 << 7} },
	{ "toggletag9",      toggletag,      {.ui = 1 << 8} },
};
