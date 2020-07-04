#include "launchelf.h"

LANGUAGE *lang;

//-------------------------------------------------
void InitLanguage(void)
{
	lang = (LANGUAGE*)malloc(sizeof(LANGUAGE));
	SetLanguage(LANG_ENGLISH);
}

//-------------------------------------------------
void FreeLanguage(void)
{
	free(lang);
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
		strcpy(lang->filer_exportfailed, "Export psu Failed");
		strcpy(lang->filer_exportto, "Export to");
		strcpy(lang->filer_importfailed, "Import psu Failed");
		strcpy(lang->filer_importto, "Import to");
		strcpy(lang->filer_keyboard_hint, "��:OK �~:Back L1:Left R1:Right START:Enter");
		strcpy(lang->filer_anyfile_hint1, "��:OK ��:Up �~:Mark ��:RevMark L1:TitleOFF R1:Menu R2:GetSize");
		strcpy(lang->filer_anyfile_hint2, "��:OK ��:Up �~:Mark ��:RevMark L1:TitleON  R1:Menu R2:GetSize");
		strcpy(lang->filer_elffile_hint1, "��:OK �~:Cancel ��:Up ��:*->ELF");
		strcpy(lang->filer_elffile_hint2, "��:OK �~:Cancel ��:Up ��:ELF->*");
		strcpy(lang->filer_fntfile_hint1, "��:OK �~:Cancel ��:Up ��:*->FNT");
		strcpy(lang->filer_fntfile_hint2, "��:OK �~:Cancel ��:Up ��:FNT->*");
		strcpy(lang->filer_dir_hint, "��:OK �~:Cancel ��:Up Start:Choose");
		strcpy(lang->filer_export_files, "files");
		strcpy(lang->filer_export_header, " num: attr:     size: filename");
		strcpy(lang->filer_import_files, "files");
		strcpy(lang->filer_import_header, " num: attr:     size: filename");
		strcpy(lang->filer_l2popup_detail, "Detail Mode");
		strcpy(lang->filer_l2popup_dirsize, "Get DirSize");
		strcpy(lang->filer_l2popup_icon, "Icon");
		strcpy(lang->filer_l2popup_flicker, "Flicker Control");
		}
		//config
		{
		strcpy(lang->conf_savefailed, "Save Failed");
		strcpy(lang->conf_saveconfig, "Save Config");
		strcpy(lang->conf_loadconfig, "Load Config");
		strcpy(lang->conf_initializeconfig, "Initialize Config");

		strcpy(lang->conf_setting_button,  "BUTTON SETTING");
		strcpy(lang->conf_setting_screen,  "SCREEN SETTING");
		strcpy(lang->conf_setting_network, "NETWORK SETTING");
		strcpy(lang->conf_setting_font,    "FONT SETTING");
		strcpy(lang->conf_setting_misc,    "MISC SETTING");
		strcpy(lang->conf_ok, "OK");
		strcpy(lang->conf_cancel, "CANCEL");

		//button
		strcpy(lang->conf_buttonsettinginit, "BUTTON SETTING INIT");
		//screen
		strcpy(lang->conf_background, "BACK GROUND   ");
		strcpy(lang->conf_frame, "FRAME         ");
		strcpy(lang->conf_highlighttext, "HIGHLIGHT TEXT");
		strcpy(lang->conf_normaltext, "NORMAL TEXT   ");
		strcpy(lang->conf_folder, "FOLDER        ");
		strcpy(lang->conf_file, "FILE          ");
		strcpy(lang->conf_ps2save, "PS2 SAVE      ");
		strcpy(lang->conf_elffile, "ELF FILE      ");
		strcpy(lang->conf_ps1save, "PS1 SAVE      ");
		strcpy(lang->conf_disabletext, "DISABLE TEXT  ");
		strcpy(lang->conf_tvmode, "TV MODE");
		strcpy(lang->conf_interlace, "INTERLACE");
		strcpy(lang->conf_ffmode, "FFMODE");
		strcpy(lang->conf_ffmode_field, "FIELD");
		strcpy(lang->conf_ffmode_frame, "FRAME");
		strcpy(lang->conf_screen_x, "SCREEN X");
		strcpy(lang->conf_screen_y, "SCREEN Y");
		strcpy(lang->conf_flickercontrol, "FLICKER CONTROL");
		strcpy(lang->conf_screensettinginit, "SCREEN SETTING INIT");
		//network
		strcpy(lang->conf_ipaddress, "IP ADDRESS");
		strcpy(lang->conf_netmask, "NETMASK   ");
		strcpy(lang->conf_gateway, "GATEWAY   ");
		strcpy(lang->conf_ipoverwrite, "SAVE IPCONFIG.DAT");
		strcpy(lang->conf_ipsettinginit, "NETWORK SETTING INIT");
		strcpy(lang->conf_ipsaved, "Saved");
		strcpy(lang->conf_ipsavefailed, "Save Failed ");
		//font
		strcpy(lang->conf_AsciiFont, "ASCII FONT");
		strcpy(lang->conf_KanjiFont, "KANJI FONT");
		strcpy(lang->conf_CharMargin, "CHAR MARGIN");
		strcpy(lang->conf_LineMargin, "LINE MARGIN");
		strcpy(lang->conf_FontBold, "FONT BOLD");
		strcpy(lang->conf_AsciiMarginTop,  "ASCII MARGIN TOP ");
		strcpy(lang->conf_AsciiMarginLeft, "ASCII MARGIN LEFT");
		strcpy(lang->conf_KanjiMarginTop,  "KANJI MARGIN TOP ");
		strcpy(lang->conf_KanjiMarginLeft, "KANJI MARGIN LEFT");
		strcpy(lang->conf_fontsettinginit, "FONT SETTING INIT");
		//misc
		strcpy(lang->conf_language, "LANGUAGE");
		strcpy(lang->conf_language_us, "ENGLISH");
		strcpy(lang->conf_language_jp, "���{��");
		strcpy(lang->conf_timeout, "TIME OUT");
		strcpy(lang->conf_disc_control, "DISC CONTROL");
		strcpy(lang->conf_print_only_filename, "PRINT ONLY FILENAME");
		strcpy(lang->conf_fileicon, "FILEICON");
		strcpy(lang->conf_disc_ps2save_check, "DISC PS2SAVE CHECK");
		strcpy(lang->conf_disc_elf_check, "DISC ELF CHECK");
		strcpy(lang->conf_export_dir, "EXPORT DIR");
		strcpy(lang->conf_miscsettinginit, "MISC SETTING INIT");

		strcpy(lang->conf_on, "ON");
		strcpy(lang->conf_off, "OFF");
		strcpy(lang->conf_edit, "Edit");
		strcpy(lang->conf_clear, "Clear");
		strcpy(lang->conf_add, "Add");
		strcpy(lang->conf_away, "Away");
		strcpy(lang->conf_change, "Change");
		strcpy(lang->conf_up, "Up");
		}
	}
	if(l==LANG_JAPANESE){
		//general
		strcpy(lang->gen_ok, "����");
		strcpy(lang->gen_cancel, "�L�����Z��");
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
		strcpy(lang->filer_exportfailed, "�G�N�X�|�[�g���s���܂���");
		strcpy(lang->filer_exportto, "�G�N�X�|�[�g�����ꏊ");
		strcpy(lang->filer_importfailed, "�C���|�[�g���s���܂���");
		strcpy(lang->filer_importto, "�C���|�[�g�����ꏊ");
		strcpy(lang->filer_keyboard_hint, "��:���� �~:�폜 L1:���� R1:�E��");
		strcpy(lang->filer_anyfile_hint1, "��:���� ��:��� �~:�}�[�N ��:�}�[�N���] L1:�^�C�g�� R1:���j���[");
		strcpy(lang->filer_anyfile_hint2, "��:���� ��:��� �~:�}�[�N ��:�}�[�N���] L1:�^�C�g�� R1:���j���[");
		strcpy(lang->filer_elffile_hint1, "��:���� �~:�L�����Z�� ��:��� ��:*->ELF");
		strcpy(lang->filer_elffile_hint2, "��:���� �~:�L�����Z�� ��:��� ��:ELF->*");
		strcpy(lang->filer_fntfile_hint1, "��:���� �~:�L�����Z�� ��:��� ��:*->FNT");
		strcpy(lang->filer_fntfile_hint2, "��:���� �~:�L�����Z�� ��:��� ��:FNT->*");
		strcpy(lang->filer_dir_hint, "��:���� �~:�L�����Z�� ��:��� Start:�t�H���_�I��");
		strcpy(lang->filer_export_files, "�̃t�@�C��");
		strcpy(lang->filer_export_header, "�ԍ�: ����:   �T�C�Y: �t�@�C���̖��O");
		strcpy(lang->filer_import_files, "�̃t�@�C��");
		strcpy(lang->filer_import_header, "�ԍ�: ����:   �T�C�Y: �t�@�C���̖��O");
		strcpy(lang->filer_l2popup_detail, "�ڍו\\�� �؂�ւ�");
		strcpy(lang->filer_l2popup_dirsize, "�t�H���_�T�C�Y�\\��");
		strcpy(lang->filer_l2popup_icon, "�A�C�R���\\��");
		strcpy(lang->filer_l2popup_flicker, "�t���b�J�[�R���g���[��");
		}
		//config
		{
		strcpy(lang->conf_savefailed, "�ݒ�̕ۑ��Ɏ��s���܂���");
		strcpy(lang->conf_saveconfig, "�ݒ�ۑ�");
		strcpy(lang->conf_loadconfig, "�ݒ�ǂݍ���");
		strcpy(lang->conf_initializeconfig, "�ݒ菉����");

		strcpy(lang->conf_setting_button,  "�����`���[");
		strcpy(lang->conf_setting_screen,  "��ʐݒ�");
		strcpy(lang->conf_setting_network, "IP�ݒ�");
		strcpy(lang->conf_setting_font,    "�t�H���g�ݒ�");
		strcpy(lang->conf_setting_misc,    "���̑�");
		strcpy(lang->conf_ok, "�ۑ����Ė߂�");
		strcpy(lang->conf_cancel, "�L�����Z��");

		//button
		strcpy(lang->conf_buttonsettinginit, "�u�����`���[�ݒ�v������������");
		//screen
		strcpy(lang->conf_background, "�w�i�̐F      ");
		strcpy(lang->conf_frame, "�g�̐F        ");
		strcpy(lang->conf_highlighttext, "��������      ");
		strcpy(lang->conf_normaltext, "����          ");
		strcpy(lang->conf_folder, "�t�H���_      ");
		strcpy(lang->conf_file, "�t�@�C��      ");
		strcpy(lang->conf_ps2save, "PS2 �Z�[�u    ");
		strcpy(lang->conf_elffile, "ELF �t�@�C��  ");
		strcpy(lang->conf_ps1save, "PS1 �Z�[�u    ");
		strcpy(lang->conf_disabletext, "��������      ");
		strcpy(lang->conf_screen_x, "��ʈʒu X");
		strcpy(lang->conf_screen_y, "��ʈʒu Y");
		strcpy(lang->conf_tvmode, "TV MODE");
		strcpy(lang->conf_interlace, "�C���^�[���[�X");
		strcpy(lang->conf_ffmode, "FFMODE");
		strcpy(lang->conf_ffmode_field, "FIELD");
		strcpy(lang->conf_ffmode_frame, "FRAME");
		strcpy(lang->conf_flickercontrol, "�t���b�J�[����");
		strcpy(lang->conf_screensettinginit, "�u��ʐݒ�v������������");
		//network
		strcpy(lang->conf_ipaddress, "IP�A�h���X  ");
		strcpy(lang->conf_netmask, "�l�b�g�}�X�N");
		strcpy(lang->conf_gateway, "�Q�[�g�E�F�C");
		strcpy(lang->conf_ipoverwrite, "IPCONFIG.DAT���㏑���ۑ�����");
		strcpy(lang->conf_ipsettinginit, "�uIP�ݒ�v������������");
		strcpy(lang->conf_ipsaved, "�ۑ����܂���");
		strcpy(lang->conf_ipsavefailed, "�ۑ����s���܂��� ");
		//font
		strcpy(lang->conf_AsciiFont, "�A�X�L�[�t�H���g");
		strcpy(lang->conf_KanjiFont, "�����t�H���g    ");
		strcpy(lang->conf_CharMargin, "�����̊Ԋu");
		strcpy(lang->conf_LineMargin, "�s�̊Ԋu  ");
		strcpy(lang->conf_FontBold, "�����ɂ���");
		strcpy(lang->conf_AsciiMarginTop, "�A�X�L�[�t�H���g�̏�̊Ԋu");
		strcpy(lang->conf_AsciiMarginLeft, "�A�X�L�[�t�H���g�̍��̊Ԋu");
		strcpy(lang->conf_KanjiMarginTop, "�����t�H���g�̏�̊Ԋu");
		strcpy(lang->conf_KanjiMarginLeft, "�����t�H���g�̍��̊Ԋu");
		strcpy(lang->conf_fontsettinginit, "�u�t�H���g�ݒ�v������������");
		//misc
		strcpy(lang->conf_language, "LANGUAGE");
		strcpy(lang->conf_language_us, "ENGLISH");
		strcpy(lang->conf_language_jp, "���{��");
		strcpy(lang->conf_timeout, "�I�[�g���[�h�܂ł̕b��");
		strcpy(lang->conf_disc_control, "�f�B�X�N���~����");
		strcpy(lang->conf_print_only_filename, "�����`���[�A�C�e�����t�@�C�����݂̂ɂ���");
		strcpy(lang->conf_fileicon, "FileBrowser�̃A�C�R��");
		strcpy(lang->conf_disc_ps2save_check, "CD/DVD�̂Ƃ��Z�[�u�f�[�^�����ׂ�");
		strcpy(lang->conf_disc_elf_check, "CD/DVD�̂Ƃ�ELF�����ׂ�");
		strcpy(lang->conf_export_dir, "�G�N�X�|�[�g�t�H���_");
		strcpy(lang->conf_miscsettinginit, "�u���̑��ݒ�v������������");

		strcpy(lang->conf_on, "ON");
		strcpy(lang->conf_off, "OFF");
		strcpy(lang->conf_edit, "�ҏW");
		strcpy(lang->conf_clear, "�폜");
		strcpy(lang->conf_add, "���₷");
		strcpy(lang->conf_away, "���炷");
		strcpy(lang->conf_change, "�ύX");
		strcpy(lang->conf_up, "���");
		}
	}
}
