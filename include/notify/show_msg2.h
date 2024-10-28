#ifndef LITE_EDITION

#define XMB2					0x584d4232

// ~0 = L"\uF880" = Dualshock circle button-Button
// ~1 = L"\uF881" = Dualshock cross button-Button
// ~2 = L"\uF882" = Dualshock square button-Button
// ~3 = L"\uF883" = Dualshock triangle button-Button
// ~4 = L"\uF884" = D-Pad UP Button-Button
// ~5 = L"\uF885" = D-Pad DOWN Button-Button
// ~6 = L"\uF886" = D-Pad LEFT Button-Button
// ~7 = L"\uF887" = D-Pad RIGHT Button-Button
// ~8 = L"\uF888" = Dualshock L1 button-Button
// ~9 = L"\uF889" = Dualshock L2 button-Button
// ~: = L"\uF88A" = Dualshock L3 button-Button
// ~; = L"\uF88B" = Dualshock R1 button-Button
// ~< = L"\uF88C" = Dualshock R2 button-Button
// ~= = L"\uF88D" = Dualshock R3 button-Button
// ~> = L"\uF88E" = Dualshock select button-Button
// ~? = L"\uF88F" = Dualshock start button-Button
// ~B = L"\uF892" = Dualshock PS button-Button
// ~C = L"\uF893" = Dualshock L2 buttonD-Pad LEFT Button-Combo
// ~D = L"\uF894" = Dualshock R2 buttonD-Pad RIGHT Button-Combo

static void show_navigation_msg(const char* msg)
{
	u32 system_plugin_handle = View_Find("system_plugin");
	if (system_plugin_handle == 0)
		return;

	u32 widget = FindWidget(system_plugin_handle, "page_autooff_guide");
	if (widget == 0)
		return;

	int len = strlen(msg) + 1;

	wchar_t wmsg[len];
	mbstowcs((wchar_t*)wmsg, msg, len);  //size_t stdc_FCAC2E8E(wchar_t *dest, const char *src, size_t max)

	for (int i = 0; i < len; i++) if (wmsg[i] == 0x7E) { wmsg[i] = 0xF850 + (u8)wmsg[i + 1], wmsg[++i] = 0x20; }

	ShowButtonNavigationText(widget, wmsg, 4, 0);
}

/*
static void show_navigation_msg(const char* msg)
{
	int view = View_Find("xmb_plugin");
	if(view)
	{
		int len = strlen(msg) + 1;

		wchar_t wmsg[len];
		mbstowcs((wchar_t *)wmsg, (const char *)msg, len);  //size_t stdc_FCAC2E8E(wchar_t *dest, const char *src, size_t max)

		for(int i = 0; i < len; i++) if(wmsg[i] == 0x7E) {wmsg[i] = 0xF850 + (u8)wmsg[i+1], wmsg[++i] = 0x20;}

		xmb2_interface = (xmb_plugin_xmb2 *)plugin_GetInterface(view, XMB2);
		xmb2_interface->showMsg(wmsg); // usage: xmb2_interface->showMsg(L"text");
	}
}
*/
#endif // #ifndef LITE_EDITION
