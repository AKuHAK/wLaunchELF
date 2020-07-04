#include "launchelf.h"

#define NUM_CNF_KEY 44
//�ݒ�t�@�C���̃L�[
const char *cnf_keyname[NUM_CNF_KEY] = 
{
	//version
	"cnf_version",
	//Launcher
	"DEFAULT",
	"CIRCLE",
	"CROSS",
	"SQUARE",
	"TRIANGLE",
	"L1",
	"R1",
	"L2",
	"R2",
	"L3",
	"R3",
	"START",
	"SELECT",
	//color
	"color_background",
	"color_fream",
	"color_highlight_text",
	"color_normal_text",
	"color_folder",
	"color_file",
	"color_ps2_save",
	"color_elf_file",
	//font
	"ascii_font",
	"kanji_font",
	"char_margin",
	"line_margin",
	"font_bold",
	"ascii_margin_top",
	"ascii_margin_left",
	"kanji_margin_top",
	"kanji_margin_left",
	//
	"screen_pos_x",
	"screen_pos_y",
	"flicker_control",
	"language",
	"timeout",
	"disc_control",
	"only_filename",
	"file_icon",
	"ps2save_check",
	"elf_check",
	"export_dir",
	"interlace",
	"ffmode",
};

//�f�t�H���g�̐ݒ�̒l
enum
{
	DEF_TIMEOUT = 10,
	DEF_FILENAME = TRUE,
	DEF_COLOR1 = ITO_RGBA(30,30,50,0),		//�w�i
	DEF_COLOR2 = ITO_RGBA(64,64,80,0),		//�g
	DEF_COLOR3 = ITO_RGBA(192,192,192,0),	//�����̕����F
	DEF_COLOR4 = ITO_RGBA(128,128,128,0),	//�ʏ�̕����F
	DEF_COLOR5 = ITO_RGBA(128,128,0,0),	//�t�H���_
	DEF_COLOR6 = ITO_RGBA(128,128,128,0),	//�t�@�C��
	DEF_COLOR7 = ITO_RGBA(0,128,0,0),		//PS2save�t�H���_
	DEF_COLOR8 = ITO_RGBA(128,0,0,0),		//ELF�t�@�C��
	DEF_SCREEN_X = 160,
	DEF_SCREEN_Y = 55,
	DEF_FLICKERCONTROL = TRUE,
	DEF_INTERLACE = TRUE,	//FALSE=ITO_NON_INTERLACE TRUE=ITO_INTERLACE
	DEF_FFMODE = FALSE,	//FALSE=ITO_FIELD TRUE=ITO_FRAME

	DEF_CHAR_MARGIN = 2,
	DEF_LINE_MARGIN = 5,
	DEF_FONTBOLD = TRUE,
	DEF_ASCII_MARGINTOP = 0,
	DEF_ASCII_MARGINLEFT = 0,
	DEF_KANJI_MARGINTOP = 0,
	DEF_KANJI_MARGINLEFT = 0,

	DEF_DISCCONTROL = TRUE,
	DEF_FILEICON = TRUE,
	DEF_DISCPS2SAVECHECK = FALSE,
	DEF_DISCELFCHECK = FALSE,
	DEF_LANGUAGE = LANG_ENGLISH,
};

//CONFIG
enum
{
	BUTTONSETTING=0,
	SCREENSETTING,
	NETWORK,
	FONTSETTING,
	MISC,
	OK,
	CANCEL,
};

//BUTTON SETTING
enum
{
	DEFAULT=1,
	LAUNCHER1,
	LAUNCHER2,
	LAUNCHER3,
	LAUNCHER4,
	LAUNCHER5,
	LAUNCHER6,
	LAUNCHER7,
	LAUNCHER8,
	LAUNCHER9,
	LAUNCHER10,
	LAUNCHER11,
	LAUNCHER12,
	BUTTONINIT,
};

//SCREEN SETTING
enum
{
	COLOR1=1,
	COLOR2,
	COLOR3,
	COLOR4,
	COLOR5,
	COLOR6,
	COLOR7,
	COLOR8,
	INTERLACE,
	FFMODE,
	SCREEN_X,
	SCREEN_Y,
	FLICKERCONTROL,
	SCREENINIT,
};

//NETWORK SETTING
enum
{
	IPADDRESS=1,
	NETMASK,
	GATEWAY,
	NETWORKSAVE,
	NETWORKINIT,
};

//FONT SETTING
enum
{
	ASCIIFONT=1,
	KANJIFONT,
	CHARMARGIN,
	LINEMARGIN,
	FONTBOLD,
	ASCIIMARGINTOP,
	ASCIIMARGINLEFT,
	KANJIMARGINTOP,
	KANJIMARGINLEFT,
	FONTINIT,
};

//MISC SETTING
enum
{
	LANG=1,
	TIMEOUT,
	DISCCONTROL,
	FILENAME,
	FILEICON,
	PS2SAVECHECK,
	ELFCHECK,
	EXPORTDIR,
	MISCINIT,
};

SETTING *setting, *tmpsetting;

//-------------------------------------------------
// �������[�J�[�h�̏�Ԃ��`�F�b�N����B
// �߂�l�͗L���ȃ������[�J�[�h�X���b�g�̔ԍ��B
// �������[�J�[�h���}�����Ă��Ȃ��ꍇ��-11��Ԃ��B
int CheckMC(void)
{
	int ret;
	
	mcGetInfo(0, 0, NULL, NULL, NULL);
	mcSync(0, NULL, &ret);

	if( -1 == ret || 0 == ret) return 0;

	mcGetInfo(0, 0, NULL, NULL, NULL);
	mcSync(0, NULL, &ret);

	if( -1 == ret || 0 == ret ) return 1;

	return -11;
}

//-------------------------------------------------
// BUTTON SETTING��������
void InitButtonSetting(void)
{
	int i;

	for(i=0; i<12; i++) setting->dirElf[i][0] = 0;

	strcpy(setting->dirElf[1], "MISC/FileBrowser");
	strcpy(setting->dirElf[12], "MISC/CONFIG");
}

//-------------------------------------------------
// SCREEN SETTING��������
void InitScreenSetting(void)
{
	setting->color[0] = DEF_COLOR1;
	setting->color[1] = DEF_COLOR2;
	setting->color[2] = DEF_COLOR3;
	setting->color[3] = DEF_COLOR4;
	setting->color[4] = DEF_COLOR5;
	setting->color[5] = DEF_COLOR6;
	setting->color[6] = DEF_COLOR7;
	setting->color[7] = DEF_COLOR8;
	setting->screen_x = DEF_SCREEN_X;
	setting->screen_y = DEF_SCREEN_Y;
	setting->flickerControl = DEF_FLICKERCONTROL;
	setting->interlace = DEF_INTERLACE;
	setting->ffmode = DEF_FFMODE;
}

//-------------------------------------------------
// FONT SETTING��������
void InitFontSetting(void)
{
	strcpy(setting->AsciiFont, "systemfont");
	strcpy(setting->KanjiFont, "systemfont");
	setting->CharMargin = DEF_CHAR_MARGIN;
	setting->LineMargin = DEF_LINE_MARGIN;
	setting->FontBold = DEF_FONTBOLD;
	setting->AsciiMarginTop = DEF_ASCII_MARGINTOP;
	setting->AsciiMarginLeft = DEF_ASCII_MARGINLEFT;
	setting->KanjiMarginTop = DEF_KANJI_MARGINTOP;
	setting->KanjiMarginLeft = DEF_KANJI_MARGINLEFT;
}

//-------------------------------------------------
// MISC SETTING��������
void InitMiscSetting(void)
{
	setting->timeout = DEF_TIMEOUT;
	setting->filename = DEF_FILENAME;
	setting->discControl = DEF_DISCCONTROL;
	setting->fileicon = DEF_FILEICON;
	setting->discPs2saveCheck = DEF_DISCPS2SAVECHECK;
	setting->discELFCheck = DEF_DISCELFCHECK;
	setting->Exportdir[0] = 0;
	setting->language = DEF_LANGUAGE;
}

//-------------------------------------------------
// �ݒ��������
void InitSetting(void)
{
	InitButtonSetting();
	InitScreenSetting();
	InitFontSetting();
	InitMiscSetting();
}

//-------------------------------------------------
void saveConfig(char *mainMsg)
{
	int fd, mcport;
	char path[MAX_PATH];
	char tmp[MAX_PATH];
	int i, ret, error;

	//cnf�t�@�C���̃p�X
	if(boot==CD_BOOT){
		//cd����N�����Ă���Ƃ��́A�ݒ�t�@�C����ۑ����Ȃ�
		mainMsg[0] = 0;
		return;
	}
	else if(boot==MC_BOOT || boot==MASS_BOOT){
		// LaunchELF�����s���ꂽ�p�X����ݒ�t�@�C�����J��
		sprintf(path, "%sLBF.CNF", LaunchElfDir);
		if((fd = fioOpen(path, O_RDONLY)) >= 0)
			fioClose(fd);
		else{
			//mcport
			if(boot==MC_BOOT)
				mcport = LaunchElfDir[2]-'0';
			else
				mcport = CheckMC();
			//path
			sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", mcport);
			if((fd = fioOpen(path, O_RDONLY)) >= 0)
				fioClose(fd);
			else
				path[0]=0;
		}
	}
	else{
		//�L����mc��SYS-CONF�t�H���_
		sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", CheckMC());
		if((fd = fioOpen(path, O_RDONLY)) >= 0)
			fioClose(fd);
		else
			path[0]=0;
	}

	cnf_init();

	//cnf�t�@�C���I�[�v��
	if(cnf_load(path)==FALSE){
		path[0]=0;
	}

	error=FALSE;
	for(i=0;i<NUM_CNF_KEY;i++){
		//version
		if(i==0)
			sprintf(tmp, "%d", 2);
		//Launcher
		if(i>=1 && i<=13)
			strcpy(tmp, setting->dirElf[i-1]);
		//color
		if(i>=14 && i<=21)
			sprintf(tmp, "%08lX", setting->color[i-14]);
		//font
		if(i==22)
			strcpy(tmp, setting->AsciiFont);
		if(i==23)
			strcpy(tmp, setting->KanjiFont);
		if(i==24)
			sprintf(tmp, "%d", setting->CharMargin);
		if(i==25)
			sprintf(tmp, "%d", setting->LineMargin);
		if(i==26)
			sprintf(tmp, "%d", setting->FontBold);
		if(i==27)
			sprintf(tmp, "%d", setting->AsciiMarginTop);
		if(i==28)
			sprintf(tmp, "%d", setting->AsciiMarginLeft);
		if(i==29)
			sprintf(tmp, "%d", setting->KanjiMarginTop);
		if(i==30)
			sprintf(tmp, "%d", setting->KanjiMarginLeft);
		//
		if(i==31)
			sprintf(tmp, "%d", setting->screen_x);
		if(i==32)
			sprintf(tmp, "%d", setting->screen_y);
		if(i==33)
			sprintf(tmp, "%d", setting->flickerControl);
		if(i==34)
			sprintf(tmp, "%d", setting->language);
		if(i==35)
			sprintf(tmp, "%d", setting->timeout);
		if(i==36)
			sprintf(tmp, "%d", setting->discControl);
		if(i==37)
			sprintf(tmp, "%d", setting->filename);
		if(i==38)
			sprintf(tmp, "%d", setting->fileicon);
		if(i==39)
			sprintf(tmp, "%d", setting->discPs2saveCheck);
		if(i==40)
			sprintf(tmp, "%d", setting->discELFCheck);
		if(i==41)
			strcpy(tmp, setting->Exportdir);
		if(i==42)
			sprintf(tmp, "%d", setting->interlace);
		if(i==43)
			sprintf(tmp, "%d", setting->ffmode);
		//
		ret = cnf_setstr(cnf_keyname[i], tmp);
		if(ret<0){
			error=TRUE;
			break;
		}
	}

	//�G���[��������
	if(error==TRUE){
		sprintf(mainMsg, "%s", lang->conf_savefailed);
		cnf_free();
		return;
	}

	//cnf�t�@�C���̃p�X
	if(boot==MC_BOOT || boot==MASS_BOOT){
		sprintf(path, "%sLBF.CNF", LaunchElfDir);
		if((fd = fioOpen(path, O_RDONLY)) >= 0)
			fioClose(fd);
		else{
			//mcport
			if(boot==MC_BOOT)
				mcport = LaunchElfDir[2]-'0';
			else
				mcport = CheckMC();
			//path
			sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", mcport);
			if((fd = fioOpen(path, O_RDONLY)) >= 0)
				fioClose(fd);
			else
				sprintf(path, "%sLBF.CNF", LaunchElfDir);
		}
	}
	else{
		sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", CheckMC());
	}

	//cnf�ۑ�
	ret = cnf_save(path);
	if(ret<0)
		sprintf(mainMsg, "%s (%s)", lang->conf_savefailed, path);
	else
		sprintf(mainMsg, "%s (%s)", lang->conf_saveconfig, path);

	cnf_free();
	return;
}

//-------------------------------------------------
void loadConfig(char *mainMsg)
{
	int fd, mcport;
	char path[MAX_PATH];
	char tmp[MAX_PATH];
	int cnf_version=0;
	int ret=0;
	int i;

	setting = (SETTING*)malloc(sizeof(SETTING));

	//cnf�t�@�C���̃p�X
	if(boot==CD_BOOT || boot==MC_BOOT || boot==MASS_BOOT){
		// LaunchELF�����s���ꂽ�p�X����ݒ�t�@�C�����J��
		sprintf(path, "%sLBF.CNF", LaunchElfDir);
		if((fd = fioOpen(path, O_RDONLY)) >= 0)
			fioClose(fd);
		else{
			//mcport
			if(boot==MC_BOOT)
				mcport = LaunchElfDir[2]-'0';
			else
				mcport = CheckMC();
			//path
			sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", mcport);
			if((fd = fioOpen(path, O_RDONLY)) >= 0)
				fioClose(fd);
			else
				path[0]=0;
		}
	}
	else{
		//�L����mc��SYS-CONF�t�H���_
		sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", CheckMC());
		if((fd = fioOpen(path, O_RDONLY)) >= 0)
			fioClose(fd);
		else
			path[0]=0;
	}

	//�ݒ������������
	InitSetting();

	cnf_init();

	//cnf�t�@�C���I�[�v��
	if(cnf_load(path)<0){
		ret=0;
	}
	else{
		ret=1;
		cnf_version = 0;
		for(i=0;i<NUM_CNF_KEY;i++){
			if(cnf_getstr(cnf_keyname[i], tmp, "")>=0){
				//version
				if(i==0)
					cnf_version = atoi(tmp);
				//Launcher
				if(i>=1 && i<=13)
					strcpy(setting->dirElf[i-1], tmp);
				//color
				if(i>=14 && i<=21)
					setting->color[i-14] = strtoul(tmp, NULL, 16);
				//font
				if(i==22)
					strcpy(setting->AsciiFont, tmp);
				if(i==23)
					strcpy(setting->KanjiFont, tmp);
				if(i==24)
					setting->CharMargin = atoi(tmp);
				if(i==25)
					setting->LineMargin = atoi(tmp);
				if(i==26)
					setting->FontBold = atoi(tmp);
					if(setting->FontBold<0 || setting->FontBold>1)
						setting->FontBold = DEF_FONTBOLD;
				if(i==27)
					setting->AsciiMarginTop = atoi(tmp);
				if(i==28)
					setting->AsciiMarginLeft = atoi(tmp);
				if(i==29)
					setting->KanjiMarginTop = atoi(tmp);
				if(i==30)
					setting->KanjiMarginLeft = atoi(tmp);
				//
				if(i==31)
					setting->screen_x = atoi(tmp);
				if(i==32)
					setting->screen_y = atoi(tmp);
				if(i==33){
					setting->flickerControl = atoi(tmp);
					if(setting->flickerControl<0 || setting->flickerControl>1)
						setting->flickerControl = DEF_FLICKERCONTROL;
				}
				if(i==34){
					setting->language = atoi(tmp);
					if(setting->language<0 || setting->flickerControl>=NUM_LANG)
						setting->language = DEF_LANGUAGE;
				}
				if(i==35){
					setting->timeout = atoi(tmp);
					if(setting->timeout<0)
						setting->timeout = DEF_TIMEOUT;
				}
				if(i==36){
					setting->discControl = atoi(tmp);
					if(setting->discControl<0 || setting->discControl>1)
						setting->discControl = DEF_DISCCONTROL;
				}
				if(i==37){
					setting->filename = atoi(tmp);
					if(setting->filename<0 || setting->filename>1)
						setting->filename = DEF_FILENAME;
				}
				if(i==38){
					setting->fileicon = atoi(tmp);
					if(setting->fileicon<0 || setting->fileicon>1)
						setting->fileicon = DEF_FILEICON;
				}
				if(i==39){
					setting->discPs2saveCheck = atoi(tmp);
					if(setting->discPs2saveCheck<0 || setting->discPs2saveCheck>1)
						setting->discPs2saveCheck = DEF_DISCPS2SAVECHECK;
				}
				if(i==40){
					setting->discELFCheck = atoi(tmp);
					if(setting->discELFCheck<0 || setting->discELFCheck>1)
						setting->discELFCheck = DEF_DISCELFCHECK;
				}
				if(i==41)
					strcpy(setting->Exportdir, tmp);
				if(i==42){
					setting->interlace = atoi(tmp);
					if(setting->interlace<0 || setting->interlace>1)
						setting->interlace = DEF_INTERLACE;
				}
				if(i==43){
					setting->ffmode = atoi(tmp);
					if(setting->ffmode<0 || setting->ffmode>1)
						setting->ffmode = DEF_FFMODE;
				}
			}
		}
		//�o�[�W�����`�F�b�N
		if(cnf_version!=2){
			//Setting������
			InitSetting();
			//�t�@�C���T�C�Y��0�ɂ���
			fd = fioOpen(path, O_WRONLY | O_TRUNC | O_CREAT);
			fioClose(fd);
			ret=2;
		}
	}

	SetLanguage(setting->language);

	if(ret==0){
		//�ݒ�t�@�C���J���Ȃ�����
		mainMsg[0] = 0;
	}
	else if(ret==1){
		//���[�h����
		sprintf(mainMsg, "%s (%s)", lang->conf_loadconfig, path);
	}
	else if(ret==2){
		//CNF�̃o�[�W�������Â�
		sprintf(mainMsg, "%s (%s)", lang->conf_initializeconfig, path);
	}
	cnf_free();
	return;
}

//-------------------------------------------------
//�����`���[�ݒ�
void config_button(SETTING *setting)
{
	char c[MAX_PATH];
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) pushed=TRUE;
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0)
					break;
				if(sel>=DEFAULT && sel<=LAUNCHER12){
					getFilePath(setting->dirElf[sel-1], ELF_FILE);
					if(!strncmp(setting->dirElf[sel-1], "mc", 2)){
						sprintf(c, "mc%s", &setting->dirElf[sel-1][3]);
						strcpy(setting->dirElf[sel-1], c);
					}
				}
				else if(sel==BUTTONINIT){
					InitButtonSetting();
					//sprintf(msg0, "%s", "Initialize Button Setting");
					//pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//�~
				if(sel>=DEFAULT && sel<=LAUNCHER12)
					setting->dirElf[sel-1][0]=0;
			}
		}
		for(i=0;i<=BUTTONINIT;i++){
			if(i==0){
				strcpy(config[i], "..");
			}
			else if(i==DEFAULT){
				sprintf(config[i], "DEFAULT: %s", setting->dirElf[0]);
			}
			else if(i==LAUNCHER1){
				sprintf(config[i], "��     : %s", setting->dirElf[1]);
			}
			else if(i==LAUNCHER2){
				sprintf(config[i], "�~     : %s", setting->dirElf[2]);
			}
			else if(i==LAUNCHER3){
				sprintf(config[i], "��     : %s", setting->dirElf[3]);
			}
			else if(i==LAUNCHER4){
				sprintf(config[i], "��     : %s", setting->dirElf[4]);
			}
			else if(i==LAUNCHER5){
				sprintf(config[i], "L1     : %s", setting->dirElf[5]);
			}
			else if(i==LAUNCHER6){
				sprintf(config[i], "R1     : %s", setting->dirElf[6]);
			}
			else if(i==LAUNCHER7){
				sprintf(config[i], "L2     : %s", setting->dirElf[7]);
			}
			else if(i==LAUNCHER8){
				sprintf(config[i], "R2     : %s", setting->dirElf[8]);
			}
			else if(i==LAUNCHER9){
				sprintf(config[i], "L3     : %s", setting->dirElf[9]);
			}
			else if(i==LAUNCHER10){
				sprintf(config[i], "R3     : %s", setting->dirElf[10]);
			}
			else if(i==LAUNCHER11){
				sprintf(config[i], "START  : %s", setting->dirElf[11]);
			}
			else if(i==LAUNCHER12){
				sprintf(config[i], "SELECT : %s", setting->dirElf[12]);
			}
			else if(i==BUTTONINIT){
				strcpy(config[i], lang->conf_buttonsettinginit);
			}
		}
		nList=15;

		// ���X�g�\���p�ϐ��̐��K��
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// ��ʕ`��J�n
		clrScr(setting->color[0]);

		// ���X�g
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nList) break;
			//�F
			if(top+i == sel)
				color = setting->color[2];
			else
				color = setting->color[3];
			//�J�[�\���\��
			if(top+i == sel)
				printXY(">", x, y, color, TRUE);
			//���X�g�\��
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			y += FONT_HEIGHT;
		}

		// �X�N���[���o�[
		if(nList > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// ���b�Z�[�W
		if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_button);
		// �������
		if(sel==0)
			sprintf(msg1, "��:%s ��:%s", lang->gen_ok, lang->conf_up);
		else if (sel>=DEFAULT && sel<=LAUNCHER12)
			sprintf(msg1, "��:%s �~:%s ��:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
		else if(sel==BUTTONINIT)
			sprintf(msg1, "��:%s ��:%s", lang->gen_ok, lang->conf_up);
		setScrTmp(msg0, msg1);
		drawScr();
	}

	for(i=0;i<=12;i++){
		if(setting->dirElf[i][0]) return;
	}
	//�����`���[�ݒ肪�����Ȃ��Ƃ�SELECT��CONFIG���Z�b�g
	strcpy(setting->dirElf[12], "MISC/CONFIG");
	return;
}

//-------------------------------------------------
//��ʐݒ�
void config_screen(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0 ,sel_x=0;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];
	int r,g,b;

	int font_h;

	font_h = FONT_HEIGHT - GetFontMargin(LINE_MARGIN);

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) pushed=TRUE;
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT){
				if(sel>=COLOR1 && sel<=COLOR8){
					sel_x--;
					if(sel_x<0){
						sel_x=2;
						sel--;
					}
				}
				else if(sel>INTERLACE)
					sel=INTERLACE;
				else
					sel=0;
			}
			else if(new_pad & PAD_RIGHT){
				if(sel>=COLOR1 && sel<=COLOR8){
					sel_x++;
					if(sel_x>2){
						sel_x=0;
						sel++;
					}
				}
				else if(sel==0)
					sel=INTERLACE;
				else
					sel+=MAX_ROWS/2;
			}
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0) break;
				if(sel>=COLOR1 && sel<=COLOR8){
					r = setting->color[sel-1] & 0xFF;
					g = setting->color[sel-1] >> 8 & 0xFF;
					b = setting->color[sel-1] >> 16 & 0xFF;
					if(sel_x==0 && r<255) r++;
					if(sel_x==1 && g<255) g++;
					if(sel_x==2 && b<255) b++;
					setting->color[sel-1] = ITO_RGBA(r, g, b, 0);
				}
				else if(sel==INTERLACE){	//�C���^�[���[�X
					setting->interlace = !setting->interlace;
					if(setting->interlace) setting->screen_y+=30;
					else setting->screen_y-=30;
					itoGsReset();
					setupito(ITO_INIT_DISABLE);
					SetHeight();
				}
				else if(sel==FFMODE){	//ffmode
					setting->ffmode = !setting->ffmode;
					itoGsReset();
					setupito(ITO_INIT_DISABLE);
					SetHeight();
				}
				else if(sel==SCREEN_X){	//SCREEN X
					setting->screen_x++;
					screen_env.screen.x = setting->screen_x;
					itoSetScreenPos(setting->screen_x, setting->screen_y);
				}
				else if(sel==SCREEN_Y){	//SCREEN Y
					setting->screen_y++;
					screen_env.screen.y = setting->screen_y;
					itoSetScreenPos(setting->screen_x, setting->screen_y);
				}
				else if(sel==FLICKERCONTROL)	//�t���b�J�[�R���g���[��
					setting->flickerControl = !setting->flickerControl;
				else if(sel==SCREENINIT){	//SCREEN SETTING INIT
					//init
					InitScreenSetting();
					itoGsReset();
					setupito(ITO_INIT_DISABLE);
					SetHeight();
					//sprintf(msg0, "%s", "Initialize Screen Setting");
					//pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//�~
				if(sel>=COLOR1 && sel<=COLOR8){
					r = setting->color[sel-1] & 0xFF;
					g = setting->color[sel-1] >> 8 & 0xFF;
					b = setting->color[sel-1] >> 16 & 0xFF;
					if(sel_x==0 && r>0) r--;
					if(sel_x==1 && g>0) g--;
					if(sel_x==2 && b>0) b--;
					setting->color[sel-1] = ITO_RGBA(r, g, b, 0);
				}
				else if(sel==SCREEN_X){	//SCREEN X
					if(setting->screen_x > 0){
						setting->screen_x--;
						screen_env.screen.x = setting->screen_x;
						itoSetScreenPos(setting->screen_x, setting->screen_y);
					}
				}
				else if(sel==SCREEN_Y){	//SCREEN Y
					if(setting->screen_y > 0){
						setting->screen_y--;
						screen_env.screen.y = setting->screen_y;
						itoSetScreenPos(setting->screen_x, setting->screen_y);
					}
				}
			}
			else if(new_pad & PAD_L3){
				static int preset=0;
				setting->color[4] = ITO_RGBA(128,128,0,0);	//�t�H���_
				setting->color[5] = ITO_RGBA(128,128,128,0);	//�t�@�C��
				setting->color[6] = ITO_RGBA(0,128,0,0);		//PS2save�t�H���_
				setting->color[7] = ITO_RGBA(128,0,0,0);		//ELF�t�@�C��
				//�f�t�H���g
				if(preset==0){
					setting->color[0] = ITO_RGBA(30,30,50,0);		//�w�i
					setting->color[1] = ITO_RGBA(64,64,80,0);		//�g
					setting->color[2] = ITO_RGBA(192,192,192,0);	//�����̕����F
					setting->color[3] = ITO_RGBA(128,128,128,0);	//�ʏ�̕����F
				}
				//Unofficial LaunchELF
				if(preset==1){
					setting->color[0] = ITO_RGBA(128,128,128,0);		//�w�i
					setting->color[1] = ITO_RGBA(64,64,64,0);		//�g
					setting->color[2] = ITO_RGBA(96,0,0,0);	//�����̕����F
					setting->color[3] = ITO_RGBA(0,0,0,0);	//�ʏ�̕����F
					setting->color[5] = ITO_RGBA(96,96,96,0);	//�t�@�C��
				}
				//���F�̔w�i
				if(preset==2){
					setting->color[0] = ITO_RGBA(160,160,128,0);		//�w�i
					setting->color[1] = ITO_RGBA(128,128,80,0);		//�g
					setting->color[2] = ITO_RGBA(0,0,0,0);	//�����̕����F
					setting->color[3] = ITO_RGBA(64,64,64,0);	//�ʏ�̕����F
				}
				//�����w�i
				if(preset==3){
					setting->color[0] = ITO_RGBA(0,0,0,0);		//�w�i
					setting->color[1] = ITO_RGBA(32,32,32,0);		//�g
					setting->color[2] = ITO_RGBA(192,192,192,0);	//�����̕����F
					setting->color[3] = ITO_RGBA(128,128,128,0);	//�ʏ�̕����F
				}
				//�ΐF�̔w�i
				if(preset==4){
					setting->color[0] = ITO_RGBA(0,32,0,0);		//�w�i
					setting->color[1] = ITO_RGBA(0,64,0,0);		//�g
					setting->color[2] = ITO_RGBA(192,192,192,0);	//�����̕����F
					setting->color[3] = ITO_RGBA(128,128,128,0);	//�ʏ�̕����F
				}
				//�ԐF�̔w�i
				if(preset==5){
					setting->color[0] = ITO_RGBA(32,0,0,0);		//�w�i
					setting->color[1] = ITO_RGBA(64,24,24,0);		//�g
					setting->color[2] = ITO_RGBA(192,192,192,0);	//�����̕����F
					setting->color[3] = ITO_RGBA(128,128,128,0);	//�ʏ�̕����F
				}
				preset++;
				if(preset>5) preset=0;
			}
		}

		//
		for(i=0;i<=SCREENINIT;i++){
			if(i==0){
				sprintf(config[i], "..");
			}
			else if(i>=COLOR1 && i<=COLOR8){	//COLOR
				r = setting->color[i-1] & 0xFF;
				g = setting->color[i-1] >> 8 & 0xFF;
				b = setting->color[i-1] >> 16 & 0xFF;
				if(i==COLOR1) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_background, r, g, b);
				if(i==COLOR2) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_frame, r, g, b);
				if(i==COLOR3) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_highlighttext, r, g, b);
				if(i==COLOR4) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_normaltext, r, g, b);
				if(i==COLOR5) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_folder, r, g, b);
				if(i==COLOR6) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_file, r, g, b);
				if(i==COLOR7) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_ps2save, r, g, b);
				if(i==COLOR8) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_elffile, r, g, b);
			}
			else if(i==INTERLACE){	//INTERLACE
				sprintf(config[i], "%s: ", lang->conf_interlace);
				if(setting->interlace)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==FFMODE){	//FFMODE
				sprintf(config[i],"%s: ", lang->conf_ffmode);
				if(setting->ffmode)
					strcat(config[i], lang->conf_ffmode_frame);
				else
					strcat(config[i], lang->conf_ffmode_field);
			}
			else if(i==SCREEN_X){	//SCREEN X
				sprintf(config[i],"%s: %3d", lang->conf_screen_x, setting->screen_x);
			}
			else if(i==SCREEN_Y){	//SCREEN Y
				sprintf(config[i],"%s: %3d", lang->conf_screen_y, setting->screen_y);
			}
			else if(i==FLICKERCONTROL){	//FLICKER CONTROL
				sprintf(config[i],"%s: ", lang->conf_flickercontrol);
				if(setting->flickerControl)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==SCREENINIT){	//INIT
				strcpy(config[i], lang->conf_screensettinginit);
			}
		}
		nList=15;

		// ���X�g�\���p�ϐ��̐��K��
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// ��ʕ`��J�n
		clrScr(setting->color[0]);

		// ���X�g
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nList) break;
			//�F
			if(top+i == sel)
				color = setting->color[2];
			else
				color = setting->color[3];
			//�J�[�\���\��
			if(top+i == sel){
				if(sel>=COLOR1 && sel<=COLOR8)
					printXY(">", FONT_WIDTH*21 + FONT_WIDTH*sel_x*8, y, color, TRUE);
				else
					printXY(">", x, y, color, TRUE);
			}
			//���X�g�\��
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			//�F�̃v���r���[
			if(top+i>=COLOR1 && top+i<=COLOR8){
				itoSprite(setting->color[top+i-1],
					x+FONT_WIDTH*42, y,
					x+FONT_WIDTH*42+font_h, y+font_h, 0);
			}
			y += FONT_HEIGHT;
		}

		// �X�N���[���o�[
		if(nList > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// ���b�Z�[�W
		if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_screen);
		// �������
		if(sel==0)
			sprintf(msg1, "��:%s ��:%s", lang->gen_ok, lang->conf_up);
		else if(sel>=COLOR1 && sel<=COLOR8)
			sprintf(msg1, "��:%s �~:%s ��:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==INTERLACE)
			sprintf(msg1, "��:%s ��:%s", lang->conf_change, lang->conf_up);
		else if(sel==FFMODE)
			sprintf(msg1, "��:%s ��:%s", lang->conf_change, lang->conf_up);
		else if(sel==FLICKERCONTROL)
			sprintf(msg1, "��:%s ��:%s", lang->conf_change, lang->conf_up);
		else if(sel==SCREENINIT)
			sprintf(msg1, "��:%s ��:%s", lang->gen_ok, lang->conf_up);
		setScrTmp(msg0, msg1);
		drawScr();
	}
	return;
}

//-------------------------------------------------
//IP�ݒ�
void config_network(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];

	int fd;
	extern char ip[16];
	extern char netmask[16];
	extern char gw[16];
	char tmp[16*3];

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) pushed=TRUE;
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0) break;
				if(sel==IPADDRESS){
					drawDark();
					itoGsFinish();
					itoSwitchFrameBuffers();
					drawDark();
					strcpy(tmp,ip);
					if(keyboard(tmp, 15)>=0) strcpy(ip,tmp);
				}
				else if(sel==NETMASK){
					drawDark();
					itoGsFinish();
					itoSwitchFrameBuffers();
					drawDark();
					strcpy(tmp,netmask);
					if(keyboard(tmp, 15)>=0) strcpy(netmask,tmp);
				}
				else if(sel==GATEWAY){
					drawDark();
					itoGsFinish();
					itoSwitchFrameBuffers();
					drawDark();
					strcpy(tmp,gw);
					if(keyboard(tmp, 15)>=0) strcpy(gw,tmp);
				}
				else if(sel==NETWORKSAVE){
					//save
					sprintf(tmp, "%s %s %s", ip, netmask, gw);
					//�t�H���_�쐬
					newdir("mc0:/","SYS-CONF");
					// ��������
					if((fd=fioOpen("mc0:/SYS-CONF/IPCONFIG.DAT",O_CREAT|O_WRONLY|O_TRUNC)) >= 0){
						fioWrite(fd, tmp, strlen(tmp));
						fioClose(fd);
						sprintf(tmp, "mc0:/SYS-CONF/IPCONFIG.DAT\n%s",lang->conf_ipsaved);
					}
					else{
						sprintf(tmp, "mc0:/SYS-CONF/IPCONFIG.DAT\n%s",lang->conf_ipsavefailed);
					}
					drawDark();
					itoGsFinish();
					itoSwitchFrameBuffers();
					drawDark();
					MessageDialog(tmp);
				}
				else if(sel==NETWORKINIT){
					//init
					strcpy(ip, "192.168.0.10");
					strcpy(netmask, "255.255.255.0");
					strcpy(gw, "192.168.0.1");
					//sprintf(msg0, "%s", "Initialize Network Setting");
					//pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//�~
			}
		}

		//
		for(i=0;i<=NETWORKINIT;i++){
			if(i==0){
				sprintf(config[i], "..");
			}
			else if(i==IPADDRESS){	//IPADDRESS
				sprintf(config[i], "%s: %s", lang->conf_ipaddress, ip);
			}
			else if(i==NETMASK){	//NETMASK
				sprintf(config[i], "%s: %s", lang->conf_netmask, netmask);
			}
			else if(i==GATEWAY){	//GATEWAY
				sprintf(config[i], "%s: %s", lang->conf_gateway, gw);
			}
			else if(i==NETWORKSAVE){	//NETWORKSAVE
				strcpy(config[i],lang->conf_ipoverwrite);
			}
			else if(i==NETWORKINIT){	//NETWORKINIT
				strcpy(config[i],lang->conf_ipsettinginit);
			}
		}
		nList=6;

		// ���X�g�\���p�ϐ��̐��K��
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// ��ʕ`��J�n
		clrScr(setting->color[0]);

		// ���X�g
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nList) break;
			//�F
			if(top+i == sel)
				color = setting->color[2];
			else
				color = setting->color[3];
			//�J�[�\���\��
			if(top+i == sel)
				printXY(">", x, y, color, TRUE);
			//���X�g�\��
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			y += FONT_HEIGHT;
		}

		// �X�N���[���o�[
		if(nList > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// ���b�Z�[�W
		if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_network);
		// �������
		if(sel==0)
			sprintf(msg1, "��:%s ��:%s", lang->gen_ok, lang->conf_up);
		else if(sel>=IPADDRESS && sel<=GATEWAY)
			sprintf(msg1, "��:%s ��:%s", lang->conf_edit, lang->conf_up);
		else if(sel==NETWORKINIT)
			sprintf(msg1, "��:%s ��:%s", lang->gen_ok, lang->conf_up);
		setScrTmp(msg0, msg1);
		drawScr();
	}
	return;
}

//-------------------------------------------------
//�t�H���g�ݒ�
void config_font(SETTING *setting)
{
	char c[MAX_PATH];
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];
	char newFontName[MAX_PATH];

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) pushed=TRUE;
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0) break;
				if(sel==ASCIIFONT){
					getFilePath(newFontName, FNT_FILE);
					//�t�H���g��K�p���Ă݂�
					if(InitFontAscii(newFontName)<0){
						//���s�����Ƃ����ɖ߂�
						InitFontAscii(setting->AsciiFont);
					}
					else{
						//���������Ƃ�
						strcpy(setting->AsciiFont, newFontName);
						if(!strncmp(setting->AsciiFont, "mc", 2)){
							sprintf(c, "mc%s", &setting->AsciiFont[3]);
							strcpy(setting->AsciiFont, c);
						}
					}
				}
				else if(sel==KANJIFONT){
					getFilePath(newFontName, FNT_FILE);
					//�t�H���g��K�p���Ă݂�
					if(InitFontKnaji(newFontName)<0){
						//���s�����Ƃ����ɖ߂�
						InitFontKnaji(setting->KanjiFont);
					}
					else{
						//���������Ƃ�
						strcpy(setting->KanjiFont, newFontName);
						if(!strncmp(setting->KanjiFont, "mc", 2)){
							sprintf(c, "mc%s", &setting->KanjiFont[3]);
							strcpy(setting->KanjiFont, c);
						}
					}
				}
				else if(sel==CHARMARGIN){
					setting->CharMargin++;
					SetFontMargin(CHAR_MARGIN, setting->CharMargin);
				}
				else if(sel==LINEMARGIN){
					setting->LineMargin++;
					SetFontMargin(LINE_MARGIN, setting->LineMargin);
				}
				else if(sel==FONTBOLD){
					setting->FontBold = !setting->FontBold ;
					SetFontBold(setting->FontBold);
				}
				else if(sel==ASCIIMARGINTOP){
					setting->AsciiMarginTop++;
					SetFontMargin(ASCII_FONT_MARGIN_TOP, setting->AsciiMarginTop);
				}
				else if(sel==ASCIIMARGINLEFT){
					setting->AsciiMarginLeft++;
					SetFontMargin(ASCII_FONT_MARGIN_LEFT, setting->AsciiMarginLeft);
				}
				else if(sel==KANJIMARGINTOP){
					setting->KanjiMarginTop++;
					SetFontMargin(KANJI_FONT_MARGIN_TOP, setting->KanjiMarginTop);
				}
				else if(sel==KANJIMARGINLEFT){
					setting->KanjiMarginLeft++;
					SetFontMargin(KANJI_FONT_MARGIN_LEFT, setting->KanjiMarginLeft);
				}
				else if(sel==FONTINIT){
					//init
					InitFontSetting();
					InitFontAscii(setting->AsciiFont);
					InitFontKnaji(setting->KanjiFont);
					SetFontMargin(CHAR_MARGIN, setting->CharMargin);
					SetFontMargin(LINE_MARGIN, setting->LineMargin);
					SetFontBold(setting->FontBold);
					SetFontMargin(ASCII_FONT_MARGIN_TOP, setting->AsciiMarginTop);
					SetFontMargin(ASCII_FONT_MARGIN_LEFT, setting->AsciiMarginLeft);
					SetFontMargin(KANJI_FONT_MARGIN_TOP, setting->KanjiMarginTop);
					SetFontMargin(KANJI_FONT_MARGIN_LEFT, setting->KanjiMarginLeft);
					//sprintf(msg0, "%s", "Initialize Font Setting");
					//pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//�~
				if(sel==ASCIIFONT){
					strcpy(setting->AsciiFont, "systemfont");
					InitFontAscii(setting->AsciiFont);
				}
				else if(sel==KANJIFONT){
					strcpy(setting->KanjiFont, "systemfont");
					InitFontKnaji(setting->KanjiFont);
				}
				else if(sel==CHARMARGIN){
					setting->CharMargin--;
					SetFontMargin(CHAR_MARGIN, setting->CharMargin);
				}
				else if(sel==LINEMARGIN){
					if(FONT_HEIGHT>1) setting->LineMargin--;
					SetFontMargin(LINE_MARGIN, setting->LineMargin);
				}
				else if(sel==ASCIIMARGINTOP){
					setting->AsciiMarginTop--;
					SetFontMargin(ASCII_FONT_MARGIN_TOP, setting->AsciiMarginTop);
				}
				else if(sel==ASCIIMARGINLEFT){
					setting->AsciiMarginLeft--;
					SetFontMargin(ASCII_FONT_MARGIN_LEFT, setting->AsciiMarginLeft);
				}
				else if(sel==KANJIMARGINTOP){
					setting->KanjiMarginTop--;
					SetFontMargin(KANJI_FONT_MARGIN_TOP, setting->KanjiMarginTop);
				}
				else if(sel==KANJIMARGINLEFT){
					setting->KanjiMarginLeft--;
					SetFontMargin(KANJI_FONT_MARGIN_LEFT, setting->KanjiMarginLeft);
				}
			}
		}

		//
		for(i=0;i<=FONTINIT;i++){
			if(i==0){
				sprintf(config[i], "..");
			}
			else if(i==ASCIIFONT){	//ASCIIFONT
				sprintf(config[i], "%s: %s", lang->conf_AsciiFont, setting->AsciiFont);
			}
			else if(i==KANJIFONT){	//KANJIFONT
				sprintf(config[i], "%s: %s", lang->conf_KanjiFont, setting->KanjiFont);
			}
			else if(i==CHARMARGIN){	//CHARMARGIN
				sprintf(config[i], "%s: %d", lang->conf_CharMargin, setting->CharMargin);
			}
			else if(i==LINEMARGIN){	//LINEMARGIN
				sprintf(config[i], "%s: %d", lang->conf_LineMargin, setting->LineMargin);
			}
			else if(i==FONTBOLD){	//FONTBOLD
				sprintf(config[i], "%s: ", lang->conf_FontBold);
				if(setting->FontBold)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==ASCIIMARGINTOP){	//ASCIIMARGINTOP
				sprintf(config[i], "%s: %d", lang->conf_AsciiMarginTop, setting->AsciiMarginTop);
			}
			else if(i==ASCIIMARGINLEFT){	//ASCIIMARGINLEFT
				sprintf(config[i], "%s: %d", lang->conf_AsciiMarginLeft, setting->AsciiMarginLeft);
			}
			else if(i==KANJIMARGINTOP){	//KANJIMARGINTOP
				sprintf(config[i], "%s: %d", lang->conf_KanjiMarginTop, setting->KanjiMarginTop);
			}
			else if(i==KANJIMARGINLEFT){	//KANJIMARGINLEFT
				sprintf(config[i], "%s: %d", lang->conf_KanjiMarginLeft, setting->KanjiMarginLeft);
			}
			else if(i==FONTINIT){	//FONT INIT
				strcpy(config[i], lang->conf_fontsettinginit);
			}
		}
		nList=11;

		// ���X�g�\���p�ϐ��̐��K��
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// ��ʕ`��J�n
		clrScr(setting->color[0]);

		// ���X�g
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nList) break;
			//�F
			if(top+i == sel)
				color = setting->color[2];
			else
				color = setting->color[3];
			//�J�[�\���\��
			if(top+i == sel)
				printXY(">", x, y, color, TRUE);
			//���X�g�\��
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			y += FONT_HEIGHT;
		}

		// �X�N���[���o�[
		if(nList > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// ���b�Z�[�W
		if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_font);
		// �������
		if(sel==0)
			sprintf(msg1, "��:%s ��:%s", lang->gen_ok, lang->conf_up);
		else if(sel==ASCIIFONT)
			sprintf(msg1, "��:%s �~:%s ��:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
		else if(sel==KANJIFONT)
			sprintf(msg1, "��:%s �~:%s ��:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
		else if(sel==CHARMARGIN)
			sprintf(msg1, "��:%s �~:%s ��:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==LINEMARGIN)
			sprintf(msg1, "��:%s �~:%s ��:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==FONTBOLD)
			sprintf(msg1, "��:%s ��:%s", lang->conf_change, lang->conf_up);
		else if(sel==ASCIIMARGINTOP)
			sprintf(msg1, "��:%s �~:%s ��:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==ASCIIMARGINLEFT)
			sprintf(msg1, "��:%s �~:%s ��:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==KANJIMARGINTOP)
			sprintf(msg1, "��:%s �~:%s ��:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==KANJIMARGINLEFT)
			sprintf(msg1, "��:%s �~:%s ��:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==FONTINIT)
			sprintf(msg1, "��:%s ��:%s", lang->gen_ok, lang->conf_up);
		else if(sel==FONTINIT)
			sprintf(msg1, "��:%s ��:%s", lang->gen_ok, lang->conf_up);
		else if(sel==FONTINIT)
			sprintf(msg1, "��:%s ��:%s", lang->gen_ok, lang->conf_up);
		setScrTmp(msg0, msg1);
		drawScr();
	}
	return;
}

//-------------------------------------------------
//���̑��ݒ�
void config_misc(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) pushed=TRUE;
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0) break;
				if(sel==LANG){
					setting->language++;
					if(setting->language==NUM_LANG) setting->language=LANG_ENGLISH;
					SetLanguage(setting->language);
				}
				else if(sel==TIMEOUT)
					setting->timeout++;
				else if(sel==DISCCONTROL)
					setting->discControl = !setting->discControl;
				else if(sel==FILENAME)
					setting->filename = !setting->filename;
				else if(sel==FILEICON)
					setting->fileicon = !setting->fileicon;
				else if(sel==PS2SAVECHECK)
						setting->discPs2saveCheck = !setting->discPs2saveCheck;
				else if(sel==ELFCHECK)
						setting->discELFCheck = !setting->discELFCheck;
				else if(sel==EXPORTDIR)
					getFilePath(setting->Exportdir, DIR);
				else if(sel==MISCINIT){
					//init
					InitMiscSetting();
					SetLanguage(setting->language);
					//sprintf(msg0, "%s", "Initialize Misc Setting");
					//pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//�~
				if(sel==TIMEOUT)
					setting->timeout--;
				if(sel==EXPORTDIR)
					setting->Exportdir[0]='\0';
			}
		}

		//
		for(i=0;i<=MISCINIT;i++){
			if(i==0){
				sprintf(config[i], "..");
			}
			else if(i==LANG){	//LANG
				sprintf(config[i], "%s: ", lang->conf_language);
				if(setting->language==LANG_ENGLISH)
					strcat(config[i], lang->conf_language_us);
				else if(setting->language==LANG_JAPANESE)
					strcat(config[i], lang->conf_language_jp);
			}
			else if(i==TIMEOUT){	//TIMEOUT
				sprintf(config[i], "%s: %d", lang->conf_timeout, setting->timeout);
			}
			else if(i==DISCCONTROL){	//DISC CONTROL
				sprintf(config[i], "%s: " ,lang->conf_disc_control);
				if(setting->discControl)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==FILENAME){	//PRINT ONLY FILENAME
				sprintf(config[i], "%s: " ,lang->conf_print_only_filename);
				if(setting->filename)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==FILEICON){	//FILEICON
				sprintf(config[i], "%s: " ,lang->conf_fileicon);
				if(setting->fileicon)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==PS2SAVECHECK){	//DISC PS2SAVE CHECK
				sprintf(config[6], "%s: " ,lang->conf_disc_ps2save_check);
				if(setting->discPs2saveCheck)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==ELFCHECK){	//DISC ELF CHECK
				sprintf(config[i], "%s: " ,lang->conf_disc_elf_check);
				if(setting->discELFCheck)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==EXPORTDIR){	//EXPORT DIR
				sprintf(config[i], "%s: %s", lang->conf_export_dir, setting->Exportdir);
			}
			else if(i==MISCINIT){	//INIT
				strcpy(config[i], lang->conf_miscsettinginit);
			}
		}
		nList=10;

		// ���X�g�\���p�ϐ��̐��K��
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// ��ʕ`��J�n
		clrScr(setting->color[0]);

		// ���X�g
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nList) break;
			//�F
			if(top+i == sel)
				color = setting->color[2];
			else
				color = setting->color[3];
			//�J�[�\���\��
			if(top+i == sel)
				printXY(">", x, y, color, TRUE);
			//���X�g�\��
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			y += FONT_HEIGHT;
		}

		// �X�N���[���o�[
		if(nList > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// ���b�Z�[�W
		if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_misc);
		// �������
		if(sel==0)
			sprintf(msg1, "��:%s ��:%s", lang->gen_ok, lang->conf_up);
		else if(sel==LANG)
			sprintf(msg1, "��:%s ��:%s", lang->conf_change, lang->conf_up);
		else if(sel==TIMEOUT)
			sprintf(msg1, "��:%s �~:%s ��:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==DISCCONTROL)
			sprintf(msg1, "��:%s ��:%s", lang->conf_change, lang->conf_up);
		else if(sel==FILENAME)
			sprintf(msg1, "��:%s ��:%s", lang->conf_change, lang->conf_up);
		else if(sel==FILEICON)
			sprintf(msg1, "��:%s ��:%s", lang->conf_change, lang->conf_up);
		else if(sel==PS2SAVECHECK)
			sprintf(msg1, "��:%s ��:%s", lang->conf_change, lang->conf_up);
		else if(sel==ELFCHECK)
			sprintf(msg1, "��:%s ��:%s", lang->conf_change, lang->conf_up);
		else if(sel==EXPORTDIR)
			sprintf(msg1, "��:%s �~:%s ��:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
		else if(sel==MISCINIT)
			sprintf(msg1, "��:%s ��:%s", lang->gen_ok, lang->conf_up);
		setScrTmp(msg0, msg1);
		drawScr();
	}
	return;
}

//-------------------------------------------------
//�ݒ�
void config(char *mainMsg)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList, sel=0, top=0;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];

	extern char ip[16];
	extern char netmask[16];
	extern char gw[16];
	char tmpip[16];
	char tmpnetmask[16];
	char tmpgw[16];

	tmpsetting = setting;
	setting = (SETTING*)malloc(sizeof(SETTING));
	*setting = *tmpsetting;

	strcpy(tmpip,ip);
	strcpy(tmpnetmask,netmask);
	strcpy(tmpgw,gw);

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				sel=OK;
			else if(new_pad & PAD_CIRCLE){
				if(sel==BUTTONSETTING) config_button(setting);
				if(sel==SCREENSETTING) config_screen(setting);
				if(sel==NETWORK) config_network(setting);
				if(sel==FONTSETTING) config_font(setting);
				if(sel==MISC) config_misc(setting);
				if(sel==OK){
					free(tmpsetting);
					saveConfig(mainMsg);
					SetFontMargin(LINE_MARGIN, setting->LineMargin);
					break;
				}
				if(sel==CANCEL){	//cansel
					free(setting);
					setting = tmpsetting;
					strcpy(ip,tmpip);
					strcpy(netmask,tmpnetmask);
					strcpy(gw,tmpgw);
					SetLanguage(setting->language);
					InitFontAscii(setting->AsciiFont);
					InitFontKnaji(setting->KanjiFont);
					SetFontMargin(CHAR_MARGIN, setting->CharMargin);
					SetFontMargin(LINE_MARGIN, setting->LineMargin);
					SetFontBold(setting->FontBold);
					SetFontMargin(ASCII_FONT_MARGIN_TOP, setting->AsciiMarginTop);
					SetFontMargin(ASCII_FONT_MARGIN_LEFT, setting->AsciiMarginLeft);
					SetFontMargin(KANJI_FONT_MARGIN_TOP, setting->KanjiMarginTop);
					SetFontMargin(KANJI_FONT_MARGIN_LEFT, setting->KanjiMarginLeft);
					itoGsReset();
					setupito(ITO_INIT_DISABLE);
					mainMsg[0] = 0;
					break;
				}
			}
			else if(new_pad & PAD_START)
				sel=OK;
			else if(new_pad & PAD_SELECT)
				sel=CANCEL;
		}
		//
		strcpy(config[0], lang->conf_setting_button);
		strcpy(config[1], lang->conf_setting_screen);
		strcpy(config[2], lang->conf_setting_network);
		strcpy(config[3], lang->conf_setting_font);
		strcpy(config[4], lang->conf_setting_misc);
		strcpy(config[5], lang->conf_ok);
		strcpy(config[6], lang->conf_cancel);
		nList=7;

		// ���X�g�\���p�ϐ��̐��K��
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// ��ʕ`��J�n
		clrScr(setting->color[0]);

		// ���X�g
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nList) break;
			//�F
			if(top+i == sel)
				color = setting->color[2];
			else
				color = setting->color[3];
			//�J�[�\���\��
			if(top+i == sel)
				printXY(">", x, y, color, TRUE);
			//���X�g�\��
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			y += FONT_HEIGHT;
		}

		// �X�N���[���o�[
		if(nList > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// ���b�Z�[�W
		strcpy(msg0, "CONFIG/");
		// �������
		sprintf(msg1, "��:%s", lang->gen_ok);
		setScrTmp(msg0, msg1);
		drawScr();
	}
	return;
}
