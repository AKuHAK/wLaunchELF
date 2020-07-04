#include "launchelf.h"

LANGUAGE *lang=NULL;

//-------------------------------------------------
void InitLanguage(void)
{
	if (lang != NULL) free(lang);
	lang = (LANGUAGE*)malloc(sizeof(LANGUAGE));
	SetLanguage(LANG_ENGLISH);
}

//-------------------------------------------------
void FreeLanguage(void)
{
	if (lang != NULL) free(lang);
}

//-------------------------------------------------
void SetLanguage(const int langID)
{
	int l;

	l=langID;
	if(l<LANG_ENGLISH) l=LANG_ENGLISH;
	if(l>=NUM_LANG) l=LANG_ENGLISH;
	memset(lang, 0, sizeof(LANGUAGE));

	if(l==LANG_ENGLISH){
		//general
		strcpy(lang->gen_ok, "OK");
		strcpy(lang->gen_cancel, "CANCEL");
		strcpy(lang->gen_yes, "YES");
		strcpy(lang->gen_no, "NO");
		strcpy(lang->gen_loading, "Loading...");
		strcpy(lang->gen_decoding, "Processing...");
		//main
		{
		strcpy(lang->main_launch_hint, "PUSH ANY BUTTON or D-PAD!");
		strcpy(lang->main_loadhddmod, "Loading HDD Modules...");
		strcpy(lang->main_loadftpmod, "Loading FTP Modules...");
		strcpy(lang->main_notfound, " is Not Found.");
		strcpy(lang->main_readsystemcnf, "Reading SYSTEM.CNF...");
		strcpy(lang->main_failed, "Failed");
		strcpy(lang->main_nodisc, "No Disc");
		strcpy(lang->main_detectingdisc, "Detecting Disc");
		strcpy(lang->main_stopdisc, "Stop Disc");
		}
		//filer
		{
		strcpy(lang->filer_menu_copy, "Copy");
		strcpy(lang->filer_menu_cut, "Cut");
		strcpy(lang->filer_menu_paste, "Paste");
		strcpy(lang->filer_menu_delete, "Delete");
		strcpy(lang->filer_menu_rename, "Rename");
		strcpy(lang->filer_menu_newdir, "New Dir");
		strcpy(lang->filer_menu_getsize, "Get Size");
		strcpy(lang->filer_menu_exportpsu, "Export psu");
		strcpy(lang->filer_menu_importpsu, "Import psu");
		strcpy(lang->filer_menu_compress, "Compression");
		strcpy(lang->filer_menu_editor, "Look");
		strcpy(lang->filer_overwrite, "Overwrite?");
		strcpy(lang->filer_not_elf, "This file isn't ELF.");
#ifdef ENABLE_PSB
		strcpy(lang->filer_execute_psb, "Execute?");
#endif
		strcpy(lang->filer_not_fnt, "This file isn't FNT.");
		strcpy(lang->filer_copy_to_clip, "Copied to the Clipboard");
		strcpy(lang->filer_delete, "Delete?");
		strcpy(lang->filer_deletemarkfiles, "Mark Files Delete?");
		strcpy(lang->filer_deleting, "deleting...");
		strcpy(lang->filer_deletefailed, "Delete Failed");
		strcpy(lang->filer_renamefailed, "Rename Failed");
		strcpy(lang->filer_pasting, "Pasting...");
		strcpy(lang->filer_pastefailed, "Paste Failed");
		strcpy(lang->filer_direxists, "directory already exists");
		strcpy(lang->filer_newdirfailed, "NewDir Failed");
		strcpy(lang->filer_checkingsize, "Checking Size...");
		strcpy(lang->filer_getsizefailed, "Get Size Failed");
		strcpy(lang->filer_export, "Export?");
		strcpy(lang->filer_exportmarkfiles, "Mark Files Export?");
		strcpy(lang->filer_exporting, "Exporting...");
		strcpy(lang->filer_exportfailed, "Export psu Failed");
		strcpy(lang->filer_exportto, "Export to");
		strcpy(lang->filer_import, "Import?");
		strcpy(lang->filer_importmarkfiles, "Mark Files Import?");
		strcpy(lang->filer_importing, "Importing...");
		strcpy(lang->filer_importfailed, "Import psu Failed");
		strcpy(lang->filer_importto, "Import to");
		strcpy(lang->filer_keyboard_hint, "��:OK �~:Back L1:Left R1:Right START:Enter");
		strcpy(lang->filer_anyfile_hint1, "��:OK ��:Up �~:Mark ��:RevMark L1:TitleOFF R1:Menu R2:GetSize");
		strcpy(lang->filer_anyfile_hint2, "��:OK ��:Up �~:Mark ��:RevMark L1:TitleON  R1:Menu R2:GetSize");
		strcpy(lang->filer_elffile_hint1, "��:OK �~:Cancel ��:Up ��:*->ELF");
		strcpy(lang->filer_elffile_hint2, "��:OK �~:Cancel ��:Up ��:ELF->*");
		strcpy(lang->filer_fntfile_hint1, "��:OK �~:Cancel ��:Up ��:*->FNT");
		strcpy(lang->filer_fntfile_hint2, "��:OK �~:Cancel ��:Up ��:FNT->*");
		strcpy(lang->filer_irxfile_hint1, "��:OK �~:Cancel ��:Up ��:*->IRX");
		strcpy(lang->filer_irxfile_hint2, "��:OK �~:Cancel ��:Up ��:IRX->*");
		strcpy(lang->filer_dir_hint, "��:OK �~:Cancel ��:Up START:Choose");
		strcpy(lang->filer_l2popup_detail, "Detail Mode");
		strcpy(lang->filer_l2popup_dirsize, "Get DirSize");
		strcpy(lang->filer_l2popup_icon, "Icon");
		strcpy(lang->filer_l2popup_flicker, "Flicker Control");
		strcpy(lang->filer_l2popup_sort, "Sort Mode");
		strcpy(lang->kbd_page[0], "ASCII");
		strcpy(lang->kbd_page[1], "Hiragana");
		strcpy(lang->kbd_page[2], "Katakana");
		strcpy(lang->kbd_page[3], "Num,Marks");
		strcpy(lang->kbd_page[4], "Kanji-on");
		strcpy(lang->kbd_page[5], "Kanji-kun");
		strcpy(lang->kbd_page[6], "History");
		strcpy(lang->kbd_page[7], "Raw");
		strcpy(lang->kbd_page[8], "Custom");
		strcpy(lang->kbd_page[9], "Extended");
		strcpy(lang->kbd_enter,    "Enter");
		strcpy(lang->kbd_abort,    "Abort");
		strcpy(lang->kbd_helpl, "��:Change �~:BS L1:Left R1:Right R2:Chars");
		strcpy(lang->kbd_helpr, "��:Insert �~:BS ��:Return L1:Left R1:Right L2:Type R2:Regist");
		}
		//editor
		{
		strcpy(lang->editor_viewer_help, "��:Count ��:TAB(%d) ��/�~:Exit L1:Left R1:Right START:TAB/CR/LF");
		strcpy(lang->editor_viewer_error1, "Can not open the file.");
		strcpy(lang->editor_viewer_error2, "Out of memory.");
		strcpy(lang->editor_l2popup_tabmode, "TAB Space Mode");
		strcpy(lang->editor_l2popup_charset, "Charactor Set");
		strcpy(lang->editor_l2popup_linenum, "Line Number Count");
		strcpy(lang->editor_l2popup_flicker, "Flicker Control");
		strcpy(lang->editor_l2popup_wordwrap, "Auto New Line Mode");
		strcpy(lang->editor_image_help, "��:FullScreen ��/�~:Exit  SIZE:%d�~%d");
		strcpy(lang->editor_image_help2, "��:FullScreen L1:Prev R1:Next ��/�~:Exit SIZE:%d�~%d");
		}
		//config
		{
		strcpy(lang->conf_savefailed, "Save Failed");
		strcpy(lang->conf_saveconfig, "Save Config");
		strcpy(lang->conf_loadconfig, "Load Config");
		strcpy(lang->conf_initializeconfig, "Initialize Config");

		strcpy(lang->conf_setting_button,  "BUTTON SETTING");
		strcpy(lang->conf_setting_filer,   "FILER SETTING");
		strcpy(lang->conf_setting_color,   "COLOR SETTING");
		strcpy(lang->conf_setting_screen,  "SCREEN SETTING");
		strcpy(lang->conf_setting_font,    "FONT SETTING");
		strcpy(lang->conf_setting_device,  "DEVICE SETTING");
		strcpy(lang->conf_setting_view,    "VIEWER SETTING");
		strcpy(lang->conf_setting_misc,    "MISC SETTING");
		strcpy(lang->conf_ok, "OK");
		strcpy(lang->conf_cancel, "CANCEL");

		//button
		strcpy(lang->conf_button_copied, "copied");
		strcpy(lang->conf_button_deleted, "deleted");
		strcpy(lang->conf_button_pasted, "pasted");
		strcpy(lang->conf_launch_btnnum, "BUTTON SETTING OF #%s");
		strcpy(lang->conf_launch_name, "DISPLAY NAME");
		strcpy(lang->conf_launch_padmsk, "PAD");
		strcpy(lang->conf_launch_path, "PATH%d");
		strcpy(lang->conf_launch_list, "CHECK ALL PAD SETTING");
		strcpy(lang->conf_buttonsettinginit, "LAUNCHER SETTING INIT");
		strcpy(lang->conf_launch_pad0, "Push the button(s) please(Arrow key is cancel).");
		strcpy(lang->conf_launch_pad2, "DEFAULT is can't change the pad.");
		strcpy(lang->conf_insert, "Insert");
		strcpy(lang->conf_delete, "Delete");
		//color
		strcpy(lang->conf_background, "BACK GROUND   ");
		strcpy(lang->conf_frame, "FRAME         ");
		strcpy(lang->conf_normaltext, "NORMAL TEXT   ");
		strcpy(lang->conf_highlighttext, "HIGHLIGHT TEXT");
		strcpy(lang->conf_disabletext, "GRAY TEXT     ");
		strcpy(lang->conf_folder, "FOLDER        ");
		strcpy(lang->conf_file, "FILE          ");
		strcpy(lang->conf_ps2save, "PS2 SAVE      ");
		strcpy(lang->conf_ps1save, "PS1 SAVE      ");
		strcpy(lang->conf_elffile, "ELF FILE      ");
		strcpy(lang->conf_psufile, "PSU FILE      ");
		strcpy(lang->conf_outside, "OUTSIDE       ");
		strcpy(lang->conf_flicker_alpha, "FLICKER ALPHA");
		strcpy(lang->conf_presetcolor, "LOAD PRESET COLOR");
		//screen
		strcpy(lang->conf_tvmode, "TV MODE");
		strcpy(lang->conf_displayname,		"NAME       ");
		strcpy(lang->conf_screen_scan,		"SCREEN SIZE");
		strcpy(lang->conf_screen_scan_crop, "NORMAL");
		strcpy(lang->conf_screen_scan_full, "FULL");
		strcpy(lang->conf_resolution,		"RESOLUTION ");
		strcpy(lang->conf_depth,			"COLOR DEPTH");
		strcpy(lang->conf_dither,			"DITHER     ");
		strcpy(lang->conf_interlace,		"INTERLACE  ");
		strcpy(lang->conf_ffmode,			"FFMODE     ");
		strcpy(lang->conf_gsedit_default,	"LOAD DEFAULT");
		strcpy(lang->conf_screen_x, "SCREEN X");
		strcpy(lang->conf_screen_y, "SCREEN Y");
		strcpy(lang->conf_flickercontrol, "FLICKER CONTROL");
		strcpy(lang->conf_screensettinginit, "SCREEN SETTING INIT");
		strcpy(lang->conf_screenmodemsg1, "You want to change the setting.\nIf screen isn't displayed,\nreturn to old setting\n10 seconds later.");
		strcpy(lang->conf_screenmodemsg2, "Changed the setting.\nDo you apply\nthe this screen setting?\n(return to old setting\nwithout reply 10 seconds later.)");
		//network
		strcpy(lang->conf_ipaddress, "IP ADDRESS");
		strcpy(lang->conf_netmask, "NETMASK   ");
		strcpy(lang->conf_gateway, "GATEWAY   ");
		strcpy(lang->conf_ipsettinginit, "LOAD DEFAULT IPCONFIG");
		strcpy(lang->conf_ipsaved, "Saved");
		strcpy(lang->conf_ipsavefailed, "Save Failed");
		//font
		strcpy(lang->conf_AsciiFont, "ASCII FONT");
		strcpy(lang->conf_KanjiFont, "KANJI FONT");
		strcpy(lang->conf_CharMargin, "CHAR MARGIN");
		strcpy(lang->conf_LineMargin, "LINE MARGIN");
		strcpy(lang->conf_FontBold, "FONT BOLD");
		strcpy(lang->conf_FontHalf, "FIX FONT WIDTH ");
		strcpy(lang->conf_FontVHalf, "FIX FONT HEIGHT");
		strcpy(lang->conf_FontScaler, "FIX FONT MODE");
		strcpy(lang->conf_FontScaler_A, "Faster (nearest/composite)");
		strcpy(lang->conf_FontScaler_B, "Normal (bilinear)");
		strcpy(lang->conf_FontScaler_C, "4x AA (and bilinear)");
		strcpy(lang->conf_AsciiMarginTop,  "ASCII MARGIN TOP ");
		strcpy(lang->conf_AsciiMarginLeft, "ASCII MARGIN LEFT");
		strcpy(lang->conf_KanjiMarginTop,  "KANJI MARGIN TOP ");
		strcpy(lang->conf_KanjiMarginLeft, "KANJI MARGIN LEFT");
		strcpy(lang->conf_fontsettinginit, "FONT SETTING INIT");
		//viewer
		strcpy(lang->conf_linenumber, "DISPLAY LINE NUMBER IN TEXT");
		strcpy(lang->conf_tabspaces,  "WIDTH OF TAB IN TEXT");
		strcpy(lang->conf_chardisp,   "DISPLAY RETURN AND TAB CODE IN TEXT");
		strcpy(lang->conf_wordwrap,   "AUTO NEW LINE IN TEXT");
		strcpy(lang->conf_fullscreen, "FULLSCREEN MODE IN IMAGE");
		strcpy(lang->conf_imageresize,"RESIZE IN IMAGE");
		strcpy(lang->conf_autodecode, "AUTO DECODE OF TEK COMPRESSION");
		strcpy(lang->conf_viewerinit, "VIEWER SETTING INIT");
		strcpy(lang->conf_imageresizes[0], "OFF");
		strcpy(lang->conf_imageresizes[1], "ONNNNNN");
		strcpy(lang->conf_imageresizes[2], "OFF");
		strcpy(lang->conf_imageresizes[3], "OFF");
		strcpy(lang->conf_imageresizes[4], "OFF");
		strcpy(lang->conf_imageresizes[5], "OFF");
		strcpy(lang->conf_imageresizes[6], "OFF");
		strcpy(lang->conf_imageresizes[7], "OFF");
		//misc
		strcpy(lang->conf_language, "LANGUAGE");
		strcpy(lang->conf_language_us, "ENGLISH");
		strcpy(lang->conf_language_jp, "���{��");
		strcpy(lang->conf_timeout, "TIME OUT");
		strcpy(lang->conf_disc_control, "DISC CONTROL");
		strcpy(lang->conf_print_only_filename, "PRINT ONLY FILENAME");
		strcpy(lang->conf_print_all_filename, "DISPLAY ALL FILES");
		strcpy(lang->conf_fileicon, "FILEICON");
		strcpy(lang->conf_disc_ps2save_check, "DISC PS2SAVE CHECK");
		strcpy(lang->conf_disc_elf_check, "DISC ELF CHECK");
		strcpy(lang->conf_file_ps2save_check, "FILE PS2SAVE CHECK");
		strcpy(lang->conf_file_elf_check, "FILE ELF CHECK");
		strcpy(lang->conf_export_dir, "EXPORT DIR");
		strcpy(lang->conf_defaulttitle, "DEFAULT SHOW TITLE");
		strcpy(lang->conf_defaultdetail, "DEFAULT SHOW DETAIL");
		strcpy(lang->conf_defaultdetail_none, "NONE");
		strcpy(lang->conf_defaultdetail_size, "SIZE");
		strcpy(lang->conf_defaultdetail_modifytime, "MODIFYTIME");
		strcpy(lang->conf_defaultdetail_both, "SIZE AND MODIFYTIME");
		strcpy(lang->conf_sort_type, "FILELIST SORT");
		strcpy(lang->conf_sort_types[0], "NONE");
		strcpy(lang->conf_sort_types[1], "FILENAME");
		strcpy(lang->conf_sort_types[2], "EXTENSION");
		strcpy(lang->conf_sort_types[3], "GAMETITLE");
		strcpy(lang->conf_sort_types[4], "SIZE");
		strcpy(lang->conf_sort_types[5], "TIMESTAMP");
		strcpy(lang->conf_sort_dir, "LIST TOP FOLDER");// always display folder in the list top
		strcpy(lang->conf_sort_ext, "LIST TOP ELF"); // always display ELF in the file top
		strcpy(lang->conf_usbmass_use, "USE EXTEND USB_MASS");
		strcpy(lang->conf_usbmass_path, "USB_MASS.IRX");
		strcpy(lang->conf_usbd_use, "USE EXTEND USB_driver");
		strcpy(lang->conf_usbd_path, "USBD.IRX");
		strcpy(lang->conf_usbmass_devs, "ADD USB_MASS DEVICES");
		strcpy(lang->conf_usbkbd_use, "USE USB KEYBOARD");
		strcpy(lang->conf_usbkbd_path, "PS2KBD.IRX");
		strcpy(lang->conf_usbmouse_use, "USE USB MOUSE");
		strcpy(lang->conf_usbmouse_path, "PS2MOUSE.IRX");
		strcpy(lang->conf_miscsettinginit, "MISC SETTING INIT");
		strcpy(lang->conf_filersettinginit, "FILER SETTING INIT");
		strcpy(lang->conf_devicesettinginit, "DEVICE SETTING INIT");
		//gsconfig
		{
		strcpy(lang->gs_easymode,		"EASY MODE");
		strcpy(lang->gs_detailmode,		"DETAIL MODE");
		strcpy(lang->gs_autoapply,		"CHANGE WITH APPLY");
		strcpy(lang->gs_gsinit,			"INIT GSCONFIG");
		strcpy(lang->gs_ok,				"OK");
		strcpy(lang->gs_cancel,			"CANCEL");

		strcpy(lang->gs_number,			"NUMBER OF SETTING");
		strcpy(lang->gs_name,			"DISPLAY NAME");
		strcpy(lang->gs_width,			"SCREEN WIDTH ");
		strcpy(lang->gs_height,			"SCREEN HEIGHT");
		strcpy(lang->gs_left,			"HORIZONTAL OFFSET");
		strcpy(lang->gs_top,			"VERTICAL OFFSET  ");
		strcpy(lang->gs_mag_x,			"X RESIZE");
		strcpy(lang->gs_mag_y,			"Y RESIZE");
		strcpy(lang->gs_depth,			"COLOR DEPTH");
		strcpy(lang->gs_bufferwidth,	"BUFFER WIDTH");
		strcpy(lang->gs_x1,				"X1");
		strcpy(lang->gs_y1,				"Y1");
		strcpy(lang->gs_x2,				"X2");
		strcpy(lang->gs_y2,				"Y2");
		strcpy(lang->gs_zleft,			"Z BUFFER LEFT");
		strcpy(lang->gs_ztop,			"Z BUFFER TOP");
		strcpy(lang->gs_zdepth,			"Z DEPTH");
		strcpy(lang->gs_dither,			"DITHER");
		strcpy(lang->gs_interlace,		"INTERLACE");
		strcpy(lang->gs_ffmode,			"FFMODE");
		strcpy(lang->gs_vmode,			"VMODE");
		strcpy(lang->gs_vesa,			"VESA");
		strcpy(lang->gs_double,			"DOUBLE BUFFER");
		strcpy(lang->gs_f0_left,		"1ST FRAME LEFT");
		strcpy(lang->gs_f0_top,			"1ST FRAME TOP ");
		strcpy(lang->gs_f1_left,		"2ND FRAME LEFT");
		strcpy(lang->gs_f1_top,			"2ND FRAME TOP ");
		strcpy(lang->gs_preset,			"INIT THIS SETTING");
		strcpy(lang->gs_init,			"LOAD DEFAULT SETTING");
		strcpy(lang->gse_convert,		"OVERRIDE WRITE TO %s");
		strcpy(lang->gs_vramsize,		"ABOUT THE VRAM SIZE");

		strcpy(lang->gs_msg_0,			"In case of setting to system crash. continue the setting?");
		
		strcpy(lang->gs_prev,			"Prev");
		strcpy(lang->gs_next,			"Next");
		strcpy(lang->gs_copy,			"Copy");
		strcpy(lang->gs_paste,			"Paste");
		strcpy(lang->gs_apply,			"Apply");
		strcpy(lang->gs_default,		"Default");
		}
		//	FMCB_CONFIG
		{
		strcpy(lang->fmcb[ 0], "NULL");
		strcpy(lang->fmcb[ 1], "Load CNF from: %s");
		strcpy(lang->fmcb[ 2], "Save CNF to: %s");
		strcpy(lang->fmcb[ 3], "Configutre launcher buttons...");
		strcpy(lang->fmcb[ 4], "Configutre OSDSYS options...");
		strcpy(lang->fmcb[ 5], "Configutre ESR path...");
		strcpy(lang->fmcb[ 6], "FastBoot: %s");
		strcpy(lang->fmcb[ 7], "Debug Screen: %s");
		strcpy(lang->fmcb[ 8], "Pad Delay: %d.%d");
		strcpy(lang->fmcb[ 9], "Return");
		strcpy(lang->fmcb[10], "Pad: %s");
		strcpy(lang->fmcb[11], "Check the all launcher buttons");
		strcpy(lang->fmcb[12], "Hacked OSDSYS: %s");
		strcpy(lang->fmcb[13], "Configutre Items...");
		strcpy(lang->fmcb[14], "Configutre Scrolling Options...");
		strcpy(lang->fmcb[15], "Video Mode: %s");
		strcpy(lang->fmcb[16], "Skip MC update check: %s");
		strcpy(lang->fmcb[17], "Skip HDD update check: %s");
		strcpy(lang->fmcb[18], "Skip Disc Boot: %s");
		strcpy(lang->fmcb[19], "Skip Sony Logo: %s");
		strcpy(lang->fmcb[20], "Go to Browser: %s");
		strcpy(lang->fmcb[21], "Selected Color:   ");
		strcpy(lang->fmcb[22], "Unselected Color: ");
		strcpy(lang->fmcb[23], "Menu X: %3d");
		strcpy(lang->fmcb[24], "Menu y: %3d");
		strcpy(lang->fmcb[25], "Enter:            ");
		strcpy(lang->fmcb[26], "Version:          ");
		strcpy(lang->fmcb[40], "Number of Item: %3d");
		strcpy(lang->fmcb[29], "Name: %s");
		strcpy(lang->fmcb[30], "Path%d: %s");
		strcpy(lang->fmcb[31], "Scroll Menu: %s");
		strcpy(lang->fmcb[32], "Displayed Items: %3d");
		strcpy(lang->fmcb[33], "Cursor Max Velocity:%6d");
		strcpy(lang->fmcb[34], "Cursor Acceleration:%6d");
		strcpy(lang->fmcb[35], "Left Cursor: %s");
		strcpy(lang->fmcb[36], "Right Cursor: %s");
		strcpy(lang->fmcb[37], "Top Deimiter: %s");
		strcpy(lang->fmcb[38], "Bottom Delimiter: %s");
		strcpy(lang->fmcb[39], "..");
		strcpy(lang->fmcb[41], "Loaded %s");
		strcpy(lang->fmcb[42], "Failed to load %s");
		strcpy(lang->fmcb[43], "Saved %s");
		strcpy(lang->fmcb[44], "Failed to open %s");
		strcpy(lang->fmcb[51], "Free McBoot Settings");
		strcpy(lang->fmcb[52], "Button Settings");
		strcpy(lang->fmcb[53], "OSD Settings");
		strcpy(lang->fmcb[54], "Item Settings");
		strcpy(lang->fmcb[55], "Scroll Settings");
		strcpy(lang->fmcb[56], "ESR Path");
		strcpy(lang->fmcb[27], "��:OK L1/R1:Change L2:Init R2:Default");// CNF
		strcpy(lang->fmcb[28], "��:OK L1/R1:Change");
		strcpy(lang->fmcb[59], "��:Add �~:Sub +��:Fast");//public
		strcpy(lang->fmcb[60], "��:OK");
		strcpy(lang->fmcb[61], "��:Edit �~:Clear");
		strcpy(lang->fmcb[62], "��:Change");
		strcpy(lang->fmcb[63], "��:Edit");
		strcpy(lang->fmcb[57], "��:Edit �~:Clear L1:Any R1:Special L2:Copy R2:Paste"); //dir
		strcpy(lang->fmcb[58], "��:Edit �~:Clear L2:Copy R2:Paste");	//title
		strcpy(lang->fmcb[45], "��:Add �~:Sub +��:Fast L2:Copy R2:Paste");
		strcpy(lang->fmcb[46], "��:Next �~:Prev L2:Copy R2:Paste");
		strcpy(lang->fmcb[47], "��:Add �~:Sub");
		strcpy(lang->fmcb[48], "��:Next �~:Prev");
		strcpy(lang->fmcb[49], "Setting was initialized");
		strcpy(lang->fmcb[50], "All set to default");
		}
		strcpy(lang->conf_on, "ON");
		strcpy(lang->conf_off, "OFF");
		strcpy(lang->conf_edit, "Edit");
		strcpy(lang->conf_clear, "Clear");
		strcpy(lang->conf_add, "Add");
		strcpy(lang->conf_away, "Sub");
		strcpy(lang->conf_change, "Change");
		strcpy(lang->conf_up, "Up");
		strcpy(lang->conf_detail, "Detail");
		strcpy(lang->conf_fast, "Fast");
		strcpy(lang->conf_default, "Default");
		}
	}
	if(l==LANG_JAPANESE){
		//general
		strcpy(lang->gen_ok, "����");
		strcpy(lang->gen_cancel, "�L�����Z��");
		strcpy(lang->gen_yes, "�͂�");
		strcpy(lang->gen_no, "������");
		strcpy(lang->gen_loading, "�ǂݍ��ݒ��ł�...");
		strcpy(lang->gen_decoding, "�������ł�...");
		//main
		{
		strcpy(lang->main_launch_hint, "�{�^���ŋN�� or �\\���L�[�őI��");
		strcpy(lang->main_loadhddmod, "HDD ���N�����Ă��܂�");
		strcpy(lang->main_loadftpmod, "FTP ���N�����Ă��܂�");
		strcpy(lang->main_notfound, " ��������܂���");
		strcpy(lang->main_readsystemcnf, "SYSTEM.CNF ��ǂݍ��ݒ�");
		strcpy(lang->main_failed, "SYSTEM.CNF ��ǂݍ��ݎ��s���܂���");
		strcpy(lang->main_nodisc, "�f�B�X�N������܂���");
		strcpy(lang->main_detectingdisc, "�f�B�X�N���o��");
		strcpy(lang->main_stopdisc, "�f�B�X�N��~");
		}
		//filer
		{
		strcpy(lang->filer_menu_copy, "�R�s�[");
		strcpy(lang->filer_menu_cut, "�؂���");
		strcpy(lang->filer_menu_paste, "�\\��t��");
		strcpy(lang->filer_menu_delete, "�폜");
		strcpy(lang->filer_menu_rename, "���l�[��");
		strcpy(lang->filer_menu_newdir, "�t�H���_�쐬");
		strcpy(lang->filer_menu_getsize, "�T�C�Y�擾");
		strcpy(lang->filer_menu_exportpsu, "�G�N�X�|�[�g");
		strcpy(lang->filer_menu_importpsu, "�C���|�[�g");
		strcpy(lang->filer_menu_compress, "���k");
		strcpy(lang->filer_menu_editor, "�\\��");
		strcpy(lang->filer_overwrite, "�㏑�����܂���?");
		strcpy(lang->filer_not_elf, "ELF�t�@�C���ł͂���܂���");
#ifdef ENABLE_PSB
		strcpy(lang->filer_execute_psb, "���s���܂���?");
#endif
		strcpy(lang->filer_not_fnt, "FNT�t�@�C���ł͂���܂���");
		strcpy(lang->filer_copy_to_clip, "�N���b�v�{�[�h�փR�s�[���܂���");
		strcpy(lang->filer_delete, "�폜���܂���?");
		strcpy(lang->filer_deletemarkfiles, "�}�[�N�����t�@�C�����폜���܂���?");
		strcpy(lang->filer_deleting, "�폜���Ă��܂�");
		strcpy(lang->filer_deletefailed, "�폜�����s���܂���");
		strcpy(lang->filer_renamefailed, "���l�[�������s���܂���");
		strcpy(lang->filer_pasting, "�\\��t�����Ă��܂�");
		strcpy(lang->filer_pastefailed, "�\\��t�����s���܂���");
		strcpy(lang->filer_direxists, "�t�H���_�͊��ɑ��݂��Ă��܂�");
		strcpy(lang->filer_newdirfailed, "�t�H���_�쐬�����s���܂���");
		strcpy(lang->filer_checkingsize, "�T�C�Y���v�Z���Ă��܂�");
		strcpy(lang->filer_getsizefailed, "�T�C�Y�擾���s���܂���");
		strcpy(lang->filer_export, "�G�N�X�|�[�g���܂���?");
		strcpy(lang->filer_exportmarkfiles, "�}�[�N�����t�@�C�����G�N�X�|�[�g���܂���?");
		strcpy(lang->filer_exporting, "�G�N�X�|�[�g���Ă��܂�");
		strcpy(lang->filer_exportfailed, "�G�N�X�|�[�g���s���܂���");
		strcpy(lang->filer_exportto, "�G�N�X�|�[�g�����ꏊ");
		strcpy(lang->filer_import, "�C���|�[�g���܂���?");
		strcpy(lang->filer_importmarkfiles, "�}�[�N�����t�@�C�����C���|�[�g���܂���?");
		strcpy(lang->filer_importing, "�C���|�[�g���Ă��܂�");
		strcpy(lang->filer_importfailed, "�C���|�[�g���s���܂���");
		strcpy(lang->filer_importto, "�C���|�[�g�����ꏊ");
		strcpy(lang->filer_keyboard_hint, "��:���� �~:�폜 L1:���� R1:�E��");
		strcpy(lang->filer_anyfile_hint1, "��:���� ��:��� �~:�}�[�N ��:�}�[�N���] L1:�^�C�g�� R1:���j���[");
		strcpy(lang->filer_anyfile_hint2, "��:���� ��:��� �~:�}�[�N ��:�}�[�N���] L1:�^�C�g�� R1:���j���[");
		strcpy(lang->filer_elffile_hint1, "��:���� �~:�L�����Z�� ��:��� ��:*->ELF");
		strcpy(lang->filer_elffile_hint2, "��:���� �~:�L�����Z�� ��:��� ��:ELF->*");
		strcpy(lang->filer_fntfile_hint1, "��:���� �~:�L�����Z�� ��:��� ��:*->FNT");
		strcpy(lang->filer_fntfile_hint2, "��:���� �~:�L�����Z�� ��:��� ��:FNT->*");
		strcpy(lang->filer_irxfile_hint1, "��:���� �~:�L�����Z�� ��:��� ��:*->IRX");
		strcpy(lang->filer_irxfile_hint2, "��:���� �~:�L�����Z�� ��:��� ��:IRX->*");
		strcpy(lang->filer_dir_hint, "��:���� �~:�L�����Z�� ��:��� Start:�t�H���_�I��");
		strcpy(lang->filer_l2popup_detail, "�ڍו\\�� �؂�ւ�");
		strcpy(lang->filer_l2popup_dirsize, "�t�H���_�T�C�Y�\\��");
		strcpy(lang->filer_l2popup_icon, "�A�C�R���\\��");
		strcpy(lang->filer_l2popup_flicker, "�t���b�J�[�R���g���[��");
		strcpy(lang->filer_l2popup_sort, "���X�g����");
		strcpy(lang->kbd_page[0], "ASCII");
		strcpy(lang->kbd_page[1], "�Ђ炪��");
		strcpy(lang->kbd_page[2], "�J�^�J�i");
		strcpy(lang->kbd_page[3], "�p���L��");
		strcpy(lang->kbd_page[4], "����(��)");
		strcpy(lang->kbd_page[5], "����(�P)");
		strcpy(lang->kbd_page[6], "��������");
		strcpy(lang->kbd_page[7], "�����ꗗ");
		strcpy(lang->kbd_page[8], "�J�X�^��");
		strcpy(lang->kbd_page[9], "�O���g��");
		strcpy(lang->kbd_enter,    "�I��");
		strcpy(lang->kbd_abort,    "���~");
		strcpy(lang->kbd_helpl, "��:���� �~:�폜 L1:���� R1:�E�� R2:���͂�");
		strcpy(lang->kbd_helpr, "��:���� �~:�폜 ��:�߂� L1:���� R1:�E�� L2:��ނ� R2:�o�^");
		}
		//editor
		{
		strcpy(lang->editor_viewer_help, "��:�s�ԍ� ��:TAB(%d) ��/�~:�߂� L1:���� R1:�E�� Start:TAB/���s");
		strcpy(lang->editor_viewer_error1, "�t�@�C���̃I�[�v���Ɏ��s���܂���");
		strcpy(lang->editor_viewer_error2, "������������܂���");
		strcpy(lang->editor_l2popup_tabmode, "TAB���[�h�ύX");
		strcpy(lang->editor_l2popup_charset, "�����R�[�h�ύX");
		strcpy(lang->editor_l2popup_linenum, "�s�ԍ��\\��");
		strcpy(lang->editor_l2popup_flicker, "�t���b�J�[�R���g���[��");
		strcpy(lang->editor_l2popup_wordwrap, "�E�[�Ő܂�Ԃ��ύX");
		strcpy(lang->editor_image_help, "��:�t���X�N���[�� ��/�~:�߂� �T�C�Y:%d�~%d");
		strcpy(lang->editor_image_help2, "��:�t���X�N���[�� L1:�O�� R1:���� ��/�~:�߂� �T�C�Y:%d�~%d");
		}
		//config
		{
		strcpy(lang->conf_savefailed, "�ݒ�̕ۑ��Ɏ��s���܂���");
		strcpy(lang->conf_saveconfig, "�ݒ�ۑ�");
		strcpy(lang->conf_loadconfig, "�ݒ�ǂݍ���");
		strcpy(lang->conf_initializeconfig, "�ݒ菉����");

		strcpy(lang->conf_setting_button,  "�����`���[");
		strcpy(lang->conf_setting_filer,   "�t�@�C���[");
		strcpy(lang->conf_setting_color,   "�z�F�ݒ�");
		strcpy(lang->conf_setting_screen,  "��ʐݒ�");
		strcpy(lang->conf_setting_font,    "�t�H���g�ݒ�");
		strcpy(lang->conf_setting_device,  "�f�o�C�X�ݒ�");
		strcpy(lang->conf_setting_view,    "�r���[�A�ݒ�");
		strcpy(lang->conf_setting_misc,    "���̑�");
		strcpy(lang->conf_ok, "�ۑ����Ė߂�");
		strcpy(lang->conf_cancel, "�L�����Z��");

		//button
		strcpy(lang->conf_button_copied, "�R�s�[���܂���");
		strcpy(lang->conf_button_deleted, "�N���A���܂���");
		strcpy(lang->conf_button_pasted, "�y�[�X�g���܂���");
		strcpy(lang->conf_launch_btnnum, "[�o�^�ԍ�:%s] �̃{�^���ݒ�");
		strcpy(lang->conf_launch_name, "�ݒ薼");
		strcpy(lang->conf_launch_padmsk, "�{�^��");
		strcpy(lang->conf_launch_path, "�o�^%d");
		strcpy(lang->conf_launch_list, "�S�Ẵ{�^���ݒ���m�F����");
		strcpy(lang->conf_buttonsettinginit, "�u�����`���[�ݒ�v������������");
		strcpy(lang->conf_launch_pad0, "�Ή�������{�^���������ĉ�����(�����L�[�ŃL�����Z��)");
		strcpy(lang->conf_launch_pad2, "DEFAULT �͕ύX�ł��܂���");
		strcpy(lang->conf_insert, "�}��");
		strcpy(lang->conf_delete, "����");
		//color
		strcpy(lang->conf_background,    "�w�i�̐F      ");
		strcpy(lang->conf_frame,         "�t���[���̐F  ");
		strcpy(lang->conf_normaltext,    "�e�L�X�g      ");
		strcpy(lang->conf_highlighttext, "�e�L�X�g����  ");
		strcpy(lang->conf_disabletext,   "�e�L�X�g����  ");
		strcpy(lang->conf_folder,        "�t�H���_      ");
		strcpy(lang->conf_file,          "�t�@�C��      ");
		strcpy(lang->conf_ps2save,       "PS2 �Z�[�u    ");
		strcpy(lang->conf_ps1save,       "PS1 �Z�[�u    ");
		strcpy(lang->conf_elffile,       "ELF �t�@�C��  ");
		strcpy(lang->conf_psufile,       "PSU �t�@�C��  ");
		strcpy(lang->conf_outside,       "��ʊO�̐F    ");
		strcpy(lang->conf_flicker_alpha, "�t���b�J�[�R���g���[���̕s�����x");
		strcpy(lang->conf_presetcolor,   "�z�F�ݒ�̏�����");
		//screen
		strcpy(lang->conf_tvmode, "TV MODE");
		strcpy(lang->conf_displayname,		"�\\����        ");
		strcpy(lang->conf_screen_scan,		"��ʃT�C�Y    ");
		strcpy(lang->conf_screen_scan_crop, "�m�[�}��");
		strcpy(lang->conf_screen_scan_full, "�t��");
		strcpy(lang->conf_resolution,		"��ʉ𑜓x    ");
		strcpy(lang->conf_depth,			"�F�[�x        ");
		strcpy(lang->conf_dither,			"�f�B�U�����O  ");
		strcpy(lang->conf_interlace,		"�C���^�[���[�X");
		strcpy(lang->conf_ffmode,			"FFMODE        ");
		strcpy(lang->conf_gsedit_default, 	"�f�t�H���g�ɖ߂�");
		strcpy(lang->conf_screen_x, "��ʈʒu X");
		strcpy(lang->conf_screen_y, "��ʈʒu Y");
		strcpy(lang->conf_flickercontrol, "�t���b�J�[�R���g���[��");
		strcpy(lang->conf_screensettinginit, "�u��ʐݒ�v������������");
		strcpy(lang->conf_screenmodemsg1, "��ʐݒ��ύX���܂��B\n�\\������Ȃ��ꍇ�ł��A\n��10�b��Ɍ��̉�ʂɖ߂�܂��B");
		strcpy(lang->conf_screenmodemsg2, "��ʐݒ��ύX���܂����B\n���̐ݒ��K�p���Ă�낵���ł����H\n(�������Ȃ��ꍇ�͖�10�b���\n���̉�ʂɖ߂�܂��B)");
		//network
		strcpy(lang->conf_ipaddress, "IP�A�h���X  ");
		strcpy(lang->conf_netmask, "�l�b�g�}�X�N");
		strcpy(lang->conf_gateway, "�Q�[�g�E�F�C");
		strcpy(lang->conf_ipsettinginit, "�f�t�H���g�ɖ߂�");
		strcpy(lang->conf_ipsaved, "�ۑ�����");
		strcpy(lang->conf_ipsavefailed, "�ۑ����s");
		//font
		strcpy(lang->conf_AsciiFont, "�A�X�L�[�t�H���g");
		strcpy(lang->conf_KanjiFont, "�����t�H���g    ");
		strcpy(lang->conf_CharMargin, "�����̊Ԋu");
		strcpy(lang->conf_LineMargin, "�s�̊Ԋu  ");
		strcpy(lang->conf_FontBold, "�����ɂ���");
		strcpy(lang->conf_FontHalf, "�t�H���g�̕���␳����@");
		strcpy(lang->conf_FontVHalf, "�t�H���g�̍�����␳����");
		strcpy(lang->conf_FontScaler, "�t�H���g�␳���[�h");
		strcpy(lang->conf_FontScaler_A, "����");
		strcpy(lang->conf_FontScaler_B, "�W��");
		strcpy(lang->conf_FontScaler_C, "���掿");
		strcpy(lang->conf_AsciiMarginTop, "�A�X�L�[�t�H���g�̏�̊Ԋu");
		strcpy(lang->conf_AsciiMarginLeft, "�A�X�L�[�t�H���g�̍��̊Ԋu");
		strcpy(lang->conf_KanjiMarginTop, "�����t�H���g�̏�̊Ԋu");
		strcpy(lang->conf_KanjiMarginLeft, "�����t�H���g�̍��̊Ԋu");
		strcpy(lang->conf_fontsettinginit, "�u�t�H���g�ݒ�v������������");
		//viewer
		strcpy(lang->conf_linenumber, "�e�L�X�g�̍s�ԍ��\\��");
		strcpy(lang->conf_tabspaces,  "�e�L�X�g��TAB�̕�����(���p���Z)");
		strcpy(lang->conf_chardisp,   "�e�L�X�g�̉��s��TAB�̃}�[�N�\\��");
		strcpy(lang->conf_wordwrap,   "�e�L�X�g�̍s���̐܂�Ԃ�����");
		strcpy(lang->conf_fullscreen, "�C���[�W�̃t���X�N���[�����[�h");
		strcpy(lang->conf_imageresize,"�C���[�W�̃��T�C�Y����");
		strcpy(lang->conf_autodecode, "tek���k�̎����L��");
		strcpy(lang->conf_viewerinit, "�u�r���[�A�ݒ�v������������");
		strcpy(lang->conf_imageresizes[0], "OFF");
		strcpy(lang->conf_imageresizes[1], "�摜�S�̂�`��̈�S�̂Ƀ��T�C�Y");
		strcpy(lang->conf_imageresizes[2], "OFF");
		strcpy(lang->conf_imageresizes[3], "OFF");
		strcpy(lang->conf_imageresizes[4], "OFF");
		strcpy(lang->conf_imageresizes[5], "OFF");
		strcpy(lang->conf_imageresizes[6], "OFF");
		strcpy(lang->conf_imageresizes[7], "OFF");
		//misc
		strcpy(lang->conf_language, "LANGUAGE");
		strcpy(lang->conf_language_us, "ENGLISH");
		strcpy(lang->conf_language_jp, "���{��");
		strcpy(lang->conf_timeout, "�I�[�g���[�h�܂ł̕b��");
		strcpy(lang->conf_disc_control, "�f�B�X�N���~����");
		strcpy(lang->conf_print_only_filename, "�t�@�C�����̂ݕ\\��");
		strcpy(lang->conf_print_all_filename, "���ׂẴt�@�C����\\��");
		strcpy(lang->conf_fileicon, "FileBrowser�̃A�C�R��");
		strcpy(lang->conf_disc_ps2save_check, "CD/DVD�̂Ƃ��Z�[�u�f�[�^�����ׂ�");
		strcpy(lang->conf_disc_elf_check, "CD/DVD�̂Ƃ�ELF�����ׂ�");
		strcpy(lang->conf_file_ps2save_check, "CD/DVD/MC�ȊO�̂Ƃ��Z�[�u�f�[�^�����ׂ�");
		strcpy(lang->conf_file_elf_check, "CD/DVD�ȊO�̂Ƃ�ELF�����ׂ�");
		strcpy(lang->conf_export_dir, "�G�N�X�|�[�g�t�H���_");
		strcpy(lang->conf_defaulttitle, "�Q�[���^�C�g���\\�����f�t�H���g�ɂ���");
		strcpy(lang->conf_defaultdetail, "�ڍו\\���̃f�t�H���g");
		strcpy(lang->conf_defaultdetail_none, "�Ȃ�");
		strcpy(lang->conf_defaultdetail_size, "�T�C�Y");
		strcpy(lang->conf_defaultdetail_modifytime, "�X�V����");
		strcpy(lang->conf_defaultdetail_both, "�T�C�Y�ƍX�V����");
		strcpy(lang->conf_sort_type, "�t�@�C�����X�g�̕��я�");
		strcpy(lang->conf_sort_types[0], "�������Ȃ�");
		strcpy(lang->conf_sort_types[1], "�t�@�C����");
		strcpy(lang->conf_sort_types[2], "�g���q");
		strcpy(lang->conf_sort_types[3], "�Q�[����");
		strcpy(lang->conf_sort_types[4], "�T�C�Y");
		strcpy(lang->conf_sort_types[5], "�X�V����");
		strcpy(lang->conf_sort_dir, "��Ƀt�H���_����ɂ���");
		strcpy(lang->conf_sort_ext, "ELF�t�@�C������ɂ���");
		strcpy(lang->conf_usbmass_use, "�O��USB_MASS�h���C�o���g�p����");
		strcpy(lang->conf_usbmass_path, "USB_MASS.IRX");
		strcpy(lang->conf_usbd_use, "�O��USB�h���C�o���g�p����");
		strcpy(lang->conf_usbd_path, "USBD.IRX");
		strcpy(lang->conf_usbmass_devs, "USB_MASS�f�o�C�X���g��");
		strcpy(lang->conf_usbkbd_use, "USB�L�[�{�[�h���g�p����");
		strcpy(lang->conf_usbkbd_path, "PS2KBD.IRX");
		strcpy(lang->conf_usbmouse_use, "USB�}�E�X���g�p����");
		strcpy(lang->conf_usbmouse_path, "PS2MOUSE.IRX");
		strcpy(lang->conf_miscsettinginit, "�u���̑��ݒ�v������������");
		strcpy(lang->conf_filersettinginit, "�u�t�@�C���[�ݒ�v������������");
		strcpy(lang->conf_devicesettinginit, "�u�f�o�C�X�ݒ�v������������");
		//gsconfig
		{
		strcpy(lang->gs_easymode,		"�ȒP�ݒ�");
		strcpy(lang->gs_detailmode,		"�ڍאݒ�");
		strcpy(lang->gs_autoapply,		"�ύX�Ɠ����ɓK�p");
		strcpy(lang->gs_gsinit,			"GSCONFIG������������");
		strcpy(lang->gs_ok,				"�ۑ����Ė߂�");
		strcpy(lang->gs_cancel,			"�L�����Z��");
		strcpy(lang->gs_number,			"�ݒ�ԍ�");
		strcpy(lang->gs_name,			"�ݒ薼");
		strcpy(lang->gs_width,			"�����𑜓x");
		strcpy(lang->gs_height,			"�����𑜓x");
		strcpy(lang->gs_left,			"���������ʒu");
		strcpy(lang->gs_top,			"���������ʒu");
		strcpy(lang->gs_mag_x,			"�����g��l");
		strcpy(lang->gs_mag_y,			"�����g��l");
		strcpy(lang->gs_depth,			"�F�ʐ[�x");
		strcpy(lang->gs_bufferwidth,	"1���C���̃s�N�Z����");
		strcpy(lang->gs_x1,				"�����J�n�ʒu");
		strcpy(lang->gs_y1,				"�����J�n�ʒu");
		strcpy(lang->gs_x2,				"�����I���ʒu");
		strcpy(lang->gs_y2,				"�����I���ʒu");
		strcpy(lang->gs_zleft,			"����Z�ʒu");
		strcpy(lang->gs_ztop,			"����Z�ʒu");
		strcpy(lang->gs_zdepth,			"Z�[�x");
		strcpy(lang->gs_dither,			"�f�B�U�����O");
		strcpy(lang->gs_interlace,		"�C���^���[�X");
		strcpy(lang->gs_ffmode,			"FFMODE");
		strcpy(lang->gs_vmode,			"VMODE");
		strcpy(lang->gs_vesa,			"VESA");
		strcpy(lang->gs_double,			"�_�u���o�b�t�@");
		strcpy(lang->gs_f0_left,		"�t���[��0�̐����ʒu");
		strcpy(lang->gs_f0_top,			"�t���[��0�̐����ʒu");
		strcpy(lang->gs_f1_left,		"�t���[��1�̐����ʒu");
		strcpy(lang->gs_f1_top,			"�t���[��1�̐����ʒu");
		strcpy(lang->gs_preset,			"�v���Z�b�g�l�ɖ߂�");
		strcpy(lang->gs_init,			"�W���l�̓ǂݍ���");
		strcpy(lang->gse_convert,		"%s �֏㏑���ۑ�����");
		strcpy(lang->gs_vramsize,		"�K�v��VRAM�̃T�C�Y");
		strcpy(lang->gs_msg_0,			"�ݒ�ɂ���Ă̓V�X�e������~����\\��������܂��B\n���s���Ă���낵���ł���?");

		strcpy(lang->gs_prev,			"�O��");
		strcpy(lang->gs_next,			"����");
		strcpy(lang->gs_copy,			"�R�s�[");
		strcpy(lang->gs_paste,			"�y�[�X�g");
		strcpy(lang->gs_apply,			"�K�p");
		strcpy(lang->gs_default,		"�W��");
		}
		//	FMCB_CONFIG
		{
		strcpy(lang->fmcb[ 0], "NULL");
		strcpy(lang->fmcb[ 1], "���[�h: %s");
		strcpy(lang->fmcb[ 2], "�Z�[�u: %s");
		strcpy(lang->fmcb[ 3], "�����`���[�ݒ�...");
		strcpy(lang->fmcb[ 4], "OSDSYS���j���[�̐ݒ�...");
		strcpy(lang->fmcb[ 5], "ESR�ݒ�...");
		strcpy(lang->fmcb[ 6], "�����N��(FASTBOOT): %s");
		strcpy(lang->fmcb[ 7], "�f�o�b�O�X�N���[��: %s");
		strcpy(lang->fmcb[ 8], "�{�^�����͑҂�����: %d.%d");
		strcpy(lang->fmcb[ 9], "�߂�");
		strcpy(lang->fmcb[10], "�{�^��: %s");
		strcpy(lang->fmcb[11], "���ׂẴ����`���[�ݒ���m�F����");
		strcpy(lang->fmcb[12], "OSDSYS���n�b�N����: %s");
		strcpy(lang->fmcb[13], "���j���[�A�C�e���̐ݒ�...");
		strcpy(lang->fmcb[14], "�X�N���[���ݒ�...");
		strcpy(lang->fmcb[15], "�f���o��: %s");
		strcpy(lang->fmcb[16], "�������[�J�[�h���������Ȃ�: %s");
		strcpy(lang->fmcb[17], "�n�[�h�f�B�X�N���������Ȃ�: %s");
		strcpy(lang->fmcb[18], "CD/DVD�̋N�����X�L�b�v: %s");
		strcpy(lang->fmcb[19], "�N������SONY���S��\\��: %s");
		strcpy(lang->fmcb[20], "�N��������u���E�U���: %s");
		strcpy(lang->fmcb[21], "�J�[�\\���̐F:   ");
		strcpy(lang->fmcb[22], "�A�C�e���̐F:   ");
		strcpy(lang->fmcb[23], "���j���[�̐����ʒu: %3d");
		strcpy(lang->fmcb[24], "���j���[�̐����ʒu: %3d");
		strcpy(lang->fmcb[25], "Enter:          ");
		strcpy(lang->fmcb[26], "Version:        ");
		strcpy(lang->fmcb[40], "���j���[�A�C�e���ԍ�: %3d");
		strcpy(lang->fmcb[29], "�A�C�e����: %s");
		strcpy(lang->fmcb[30], "�o�^%d: %s");
		strcpy(lang->fmcb[31], "���j���[���X�N���[�����ɂ���: %s");
		strcpy(lang->fmcb[32], "�\\�����郁�j���[�A�C�e����: %3d");
		strcpy(lang->fmcb[33], "�J�[�\\���ő�ړ����x:%7d");
		strcpy(lang->fmcb[34], "�J�[�\\���̍������܂�:%7d");
		strcpy(lang->fmcb[35], "�����̃J�[�\\��: %s");
		strcpy(lang->fmcb[36], "�E���̃J�[�\\��: %s");
		strcpy(lang->fmcb[37], "�㕔�̑����\\��: %s");
		strcpy(lang->fmcb[38], "�����̑����\\��: %s");
		strcpy(lang->fmcb[39], "..");
		strcpy(lang->fmcb[41], "%s ��ǂݍ��݂܂���");
		strcpy(lang->fmcb[42], "%s �̓ǂݍ��݂Ɏ��s���܂���");
		strcpy(lang->fmcb[43], "%s �֕ۑ����܂���");
		strcpy(lang->fmcb[44], "%s �ւ̕ۑ��Ɏ��s���܂���");
		strcpy(lang->fmcb[51], "FMCBCONFIG");
		strcpy(lang->fmcb[52], "�����`���[�ݒ�");
		strcpy(lang->fmcb[53], "���j���[�ݒ�");
		strcpy(lang->fmcb[54], "�A�C�e���ݒ�");
		strcpy(lang->fmcb[55], "�X�N���[���ݒ�");
		strcpy(lang->fmcb[56], "ESR�ݒ�");
		strcpy(lang->fmcb[27], "��:OK L1/R1:�ύX L2:������ R2:�W����");// CNF
		strcpy(lang->fmcb[28], "��:OK L1/R1:�ύX");
		strcpy(lang->fmcb[59], "��:���₷ �~:���炷 +��:����");//public
		strcpy(lang->fmcb[60], "��:OK");
		strcpy(lang->fmcb[61], "��:�ҏW �~:�폜");
		strcpy(lang->fmcb[62], "��:�ύX");
		strcpy(lang->fmcb[63], "��:�ҏW");
		strcpy(lang->fmcb[57], "��:�ҏW �~:�폜 L1:MC�ύX R1:�@�\\�o�^ L2:�R�s�[ R2:�y�[�X�g"); //dir
		strcpy(lang->fmcb[58], "��:�ҏW �~:�폜 L2:�R�s�[ R2:�y�[�X�g");	//title
		strcpy(lang->fmcb[45], "��:���₷ �~:���炷 +��:���� L2:�R�s�[ R2:�y�[�X�g");
		strcpy(lang->fmcb[46], "��:���� �~:�O�� L2:�R�s�[ R2:�y�[�X�g");
		strcpy(lang->fmcb[47], "��:���₷ �~:���炷");
		strcpy(lang->fmcb[48], "��:���� �~:�O��");
		strcpy(lang->fmcb[49], "�ݒ�����������܂���");
		strcpy(lang->fmcb[50], "�W���l�����[�h���܂���");
		}
		strcpy(lang->conf_on, "ON");
		strcpy(lang->conf_off, "OFF");
		strcpy(lang->conf_edit, "�ҏW");
		strcpy(lang->conf_clear, "�폜");
		strcpy(lang->conf_add, "���₷");
		strcpy(lang->conf_away, "���炷");
		strcpy(lang->conf_change, "�ύX");
		strcpy(lang->conf_up, "���");
		strcpy(lang->conf_detail, "�ڍ�");
		strcpy(lang->conf_fast, "����");
		strcpy(lang->conf_default, "�W��");
		}
	}
}
