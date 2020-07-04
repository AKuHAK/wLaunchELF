#include "launchelf.h"

//----------------------------------------------------------
typedef struct {
	char Identifier[6];	// "FONTX2"
	char FontName[8];		// Font��
	unsigned char XSize;
	unsigned char YSize;
	unsigned char CodeType;
	unsigned char Tnum;	// �e�[�u���̃G���g����
	struct {
		unsigned short Start;	// �̈�̎n�܂�̕����R�[�h
		unsigned short End;	// �̈�̏I���̕����R�[�h
	} Block[];
} FONTX_HEADER;

typedef struct {
	char font_name[9];
	int width;
	int height;
	int size;	//1�������̃T�C�Y
	int Tnum;
	int offset;
} FONTX_DATA;

//----------------------------------------------------------
itoGsEnv screen_env;
uint16 buffer_width;
uint16 buffer_height;

int initbiosfont=0;
char *biosfont=NULL;
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 448;
int SCREEN_MARGIN;
int FONT_WIDTH;
int FONT_HEIGHT;
int MAX_ROWS;

int char_Margin;	//�����̊Ԋu
int line_Margin;	//�s�̊Ԋu
int font_bold;

//ascii
int init_ascii=0;	//���������������Ă��Ȃ����̃t���O
char *font_ascii=NULL;	//�t�H���g�̃o�b�t�@
FONTX_DATA ascii_data;	//�t�H���g�̏��
int ascii_MarginTop;	//��̃}�[�W��
int ascii_MarginLeft;	//���̃}�[�W��
//kanji
int init_kanji=0;
char *font_kanji=NULL;
FONTX_DATA kanji_data;
int kanji_MarginTop;
int kanji_MarginLeft;

unsigned short font_sjis_table[] = {
0x8140,0x817e,
0x8180,0x81ac,
0x81b8,0x81bf,
0x81c8,0x81ce,
0x81da,0x81e8,
0x81f0,0x81f7,
0x81fc,0x81fc,
0x824f,0x8258,
0x8260,0x8279,
0x8281,0x829a,
0x829f,0x82f1,
0x8340,0x837e,
0x8380,0x8396,
0x839f,0x83b6,
0x83bf,0x83d6,
0x8440,0x8460,
0x8470,0x847e,
0x8480,0x8491,
0x849f,0x84be,
0x889f,0x88fc,
0x8940,0x897e,
0x8980,0x89fc,
0x8a40,0x8a7e,
0x8a80,0x8afc,
0x8b40,0x8b7e,
0x8b80,0x8bfc,
0x8c40,0x8c7e,
0x8c80,0x8cfc,
0x8d40,0x8d7e,
0x8d80,0x8dfc,
0x8e40,0x8e7e,
0x8e80,0x8efc,
0x8f40,0x8f7e,
0x8f80,0x8ffc,
0x9040,0x907e,
0x9080,0x90fc,
0x9140,0x917e,
0x9180,0x91fc,
0x9240,0x927e,
0x9280,0x92fc,
0x9340,0x937e,
0x9380,0x93fc,
0x9440,0x947e,
0x9480,0x94fc,
0x9540,0x957e,
0x9580,0x95fc,
0x9640,0x967e,
0x9680,0x96fc,
0x9740,0x977e,
0x9780,0x97fc,
0x9840,0x9872
};

//-------------------------------------------------
// setup ito
void setupito(int tvmode)
{
	uint8 vmode;
	uint8 psm;

	if(tvmode==0)
		vmode = ITO_VMODE_AUTO;
	else if(tvmode==1)
		vmode = ITO_VMODE_NTSC;
	else if(tvmode==2)
		vmode = ITO_VMODE_PAL;
	else if(tvmode==3)
		vmode = 0x50;
	else if(tvmode==4)
		vmode = 0x52;
	else
		vmode = ITO_VMODE_NTSC;

	switch(vmode)
	{
		case ITO_VMODE_NTSC:
		{
			buffer_width = 640;
			buffer_height= 448;
			psm = ITO_RGBA32;
			break;
		}
		case ITO_VMODE_PAL:
		{
			buffer_width = 640;
			buffer_height= 512;
			psm = ITO_RGBA32;
			break;
		}
		case 0x50://480p
		{
			buffer_width = 720;
			buffer_height= 480;
			psm = ITO_RGBA32;
			//setting->interlace = ITO_NON_INTERLACE;
			//setting->ffmode = ITO_FIELD;
			break;
		}
		case 0x52://720p:
		{
			buffer_width = 1280;
			buffer_height= 720;
			psm = ITO_RGBA16;
			//setting->interlace = ITO_NON_INTERLACE;
			//setting->ffmode = ITO_FIELD;
			break;
		}
		default:
		{
			//NTSC
			buffer_width = 640;
			buffer_height= 448;
			psm = ITO_RGBA32;
			vmode = ITO_VMODE_NTSC;
			break;
		}
	}

	// screen resolution
	screen_env.screen.width		= buffer_width;
	screen_env.screen.height	= buffer_height;
	screen_env.screen.psm		= psm;

	// These setting work best with my tv, experiment for youself
	screen_env.screen.x			= setting->screen_x; 
	screen_env.screen.y			= setting->screen_y;
	
	screen_env.framebuffer1.x	= 0;
	screen_env.framebuffer1.y	= 0;
	
	screen_env.framebuffer2.x	= 0;
	screen_env.framebuffer2.y	= buffer_height;

	// zbuffer
	screen_env.zbuffer.x		= 0;
	screen_env.zbuffer.y		= buffer_height*2;
	screen_env.zbuffer.psm		= ITO_ZBUF32;
	
	// scissor 
	screen_env.scissor_x1		= 0;
	screen_env.scissor_y1		= 0;
	screen_env.scissor_x2		= buffer_width;
	screen_env.scissor_y2		= buffer_height;
	
	// misc
	screen_env.dither			= TRUE;
	screen_env.interlace		= setting->interlace;
	screen_env.ffmode			= setting->ffmode;
	screen_env.vmode			= vmode;
	
	itoGsEnvSubmit(&screen_env);

	//�A���t�@�u�����h
	itoSetAlphaBlending(
		ITO_ALPHA_COLOR_SRC, // A = COLOR SOURCE
		ITO_ALPHA_COLOR_DST, // B = COLOR DEST
		ITO_ALPHA_VALUE_SRC, // C = ALPHA VALUE SOURCE
		ITO_ALPHA_COLOR_DST, // C = COLOR DEST
		0x80);				 // Fixed Value
}

//-------------------------------------------------
// �Â�����(�������̍����l�p)
void drawDark(void)
{
	//�A���t�@�u�����h�L��
	itoPrimAlphaBlending( TRUE );
	//
	itoSprite(ITO_RGBA(0,0,0,0x10),
		0, SCREEN_MARGIN+FONT_HEIGHT*2.5,
		SCREEN_WIDTH, SCREEN_HEIGHT-SCREEN_MARGIN-FONT_HEIGHT*1.5, 0);
	//�A���t�@�u�����h����
	itoPrimAlphaBlending(FALSE);
}

//-------------------------------------------------
// �_�C�A���O�̔w�i
void drawDialogTmp(int x1, int y1, int x2, int y2, uint64 color1, uint64 color2)
{
	//
	itoSprite(color1, x1, y1, x2, y2, 0);
	drawFrame(x1+2, y1+2, x2-2, y2-2, color2);
}

//-------------------------------------------------
// ��ʕ\���̃e���v���[�g
void setScrTmp(const char *msg0, const char *msg1)
{
	uint64 color;
	uint64 color2;	//�A���t�@�t��

	color = setting->color[1]&0x00FFFFFF;	//�����x�����O
	color = color|0x80000000;	//�s����
	color2 = color|0x10000000;	//������

	// �o�[�W�����\�L
	printXY(LBF_VER, FONT_WIDTH*2, SCREEN_MARGIN, setting->color[3], TRUE);

	// ���b�Z�[�W
	printXY(msg0, FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT, setting->color[3], TRUE);

	//FLICKER CONTROL: ON
	if( (setting->flickerControl)||(SCREEN_HEIGHT==448) ){
		//�A���t�@�u�����h�L��
		itoPrimAlphaBlending( TRUE );
		itoLine(color2, 0, SCREEN_MARGIN+FONT_HEIGHT*2.5+1, 0,
			color2, SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2.5+1, 0);	
		itoLine(color2, 0, SCREEN_MARGIN+(MAX_ROWS+3.5)*FONT_HEIGHT+1, 0,
			color2, SCREEN_WIDTH, SCREEN_MARGIN+(MAX_ROWS+3.5)*FONT_HEIGHT+1, 0);	
		//�A���t�@�u�����h����
		itoPrimAlphaBlending(FALSE);
	}

	itoLine(color, 0, SCREEN_MARGIN+FONT_HEIGHT*2.5, 0,
		color, SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2.5, 0);	
	itoLine(color, 0, SCREEN_MARGIN+(MAX_ROWS+3.5)*FONT_HEIGHT, 0,
		color, SCREEN_WIDTH, SCREEN_MARGIN+(MAX_ROWS+3.5)*FONT_HEIGHT, 0);	

	// �������
	printXY(msg1, FONT_WIDTH*1, SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT, setting->color[3], TRUE);
}

//-------------------------------------------------
// ���b�Z�[�W�`��
void drawMsg(const char *msg)
{
	itoSprite(setting->color[0], 0, SCREEN_MARGIN+FONT_HEIGHT,
		SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2, 0);
	//���b�Z�[�W
	printXY(msg, FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT,
		setting->color[3], TRUE);
	drawScr();
}

//-------------------------------------------------
// ��ʂ̃N���A
void clrScr(uint64 color)
{
	//type A
	itoSprite(color, 0, 0, buffer_width, buffer_height, 0);

	//type B
//	itoSprite(ITO_RGBA(0x00,0x00,0x00,0x00), 0, 0, buffer_width, buffer_height-32, 0);//������h�~�̂��߂�buffer_height��-32����
//	itoSprite(color, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}

//-------------------------------------------------
// ��ʂ̕`��
void drawScr(void)
{
	itoGsFinish();
	itoVSync();
	itoSwitchFrameBuffers();
}

//-------------------------------------------------
// �g�̕`��
void drawFrame(int x1, int y1, int x2, int y2, uint64 color)
{
	uint64 color2;	//�A���t�@�t��

	color = color&0x00FFFFFF;	//�����x�����O
	color = color|0x80000000;	//�s����
	color2 = color|0x10000000;	//������

	//FLICKER CONTROL: ON
	if((setting->flickerControl)|(SCREEN_HEIGHT==448)){
		//�A���t�@�u�����h�L��
		itoPrimAlphaBlending( TRUE );
		//��̉���
		itoLine(color2, x1, y1+1, 0, color2, x2, y1+1, 0);
		//���̉���
		itoLine(color2, x2, y2+1, 0, color2, x1, y2+1, 0);
		//�A���t�@�u�����h����
		itoPrimAlphaBlending(FALSE);
	}

	//��̉���
	itoLine(color, x1, y1, 0, color, x2, y1, 0);
	//�E�̏c��
	itoLine(color, x2, y1, 0, color, x2, y2, 0);	
	//���̉���
	itoLine(color, x2, y2, 0, color, x1, y2, 0);
	//���̏c��
	itoLine(color, x1, y2, 0, color, x1, y1, 0);
}

//-------------------------------------------------
//MAX_ROWS�Ȃǂ̐ݒ�
void SetHeight(void)
{
	//SCREEN_WIDTH��SCREEN_HEIGHT
	if(setting->tvmode==3){	//480p
		SCREEN_WIDTH = 640;
		SCREEN_HEIGHT = 480-32;
	}
	else if(setting->tvmode==4){	//720p
		SCREEN_WIDTH = 1280-64;
		SCREEN_HEIGHT = 720-32;
	}
	else{
		SCREEN_WIDTH = 640;
		if(ITO_VMODE_AUTO==ITO_VMODE_NTSC){
			//NTSC
			if(setting->ffmode==FALSE && setting->interlace==TRUE)
				SCREEN_HEIGHT = 448;
			else
				SCREEN_HEIGHT = 224;
		}
		else{
			//PAL
			if(setting->ffmode==FALSE && setting->interlace==TRUE)
				SCREEN_HEIGHT = 512;
			else
				SCREEN_HEIGHT = 256;
		}
	}

	//FONT_WIDTH
	FONT_WIDTH = ascii_data.width + char_Margin;

	//FONT_HEIGHT
	if(ascii_data.height>=kanji_data.height)
		FONT_HEIGHT = ascii_data.height + line_Margin;
	else
		FONT_HEIGHT = kanji_data.height + line_Margin;

	//MAX_ROWS
	MAX_ROWS = SCREEN_HEIGHT/FONT_HEIGHT-6;

	//SCREEN_MARGIN
	SCREEN_MARGIN = (SCREEN_HEIGHT - ((MAX_ROWS+5) * FONT_HEIGHT))/2;
}

//------------------------------------------------------------
//�A�X�L�[�t�H���g�����[�h
int InitFontAscii(const char *path)
{
	int fd=0;
	size_t size;
	FONTX_HEADER *fontx_header_ascii;
	char fullpath[MAX_PATH];

	if(init_ascii==1) FreeFontAscii();

	if(strcmp(path, "rom0:KROM")==0 || strcmp(path, "systemfont")==0){
		//BIOSFont
		//�t�H���g�t�@�C���I�[�v��
		fd = fioOpen("rom0:KROM", O_RDONLY);
		if(fd<0) return -1;
	
		//���������m�� ���zFONTX2 KROM
		size = 17 + 15*256;	//�w�b�_�T�C�Y + 1�����̃T�C�Y*256����
		font_ascii = (char*)malloc(size);
		memset(font_ascii, 0, size);
		if(font_ascii==NULL){
			fioClose(fd);
			return -1;
		}

		//�������ɓǂݍ���
		fioLseek(fd, 0x198DE, SEEK_SET);
		fioRead(fd, font_ascii + 17 + 15*33, 15*95);//�w�b�_�T�C�Y + 1�����̃T�C�Y*33����, 1�����̃T�C�Y*95����

		//�N���[�Y
		fioClose(fd);

		//�w�b�_�̃|�C���^
		fontx_header_ascii = (FONTX_HEADER*)font_ascii;

		//�w�b�_�쐬
		strncpy(fontx_header_ascii->Identifier, "FONTX2", 6);
		strncpy(fontx_header_ascii->FontName, "KROM", 8);
		fontx_header_ascii->XSize = 8;
		fontx_header_ascii->YSize =15;
		fontx_header_ascii->CodeType = 0;
/*
		KROM�̃A�X�L�[�t�H���g��FONTX2�Ń_���v
		{
			fd=fioOpen("host:KROM.fnt",O_WRONLY | O_TRUNC | O_CREAT);
			fioWrite(fd, font_ascii, size);
			fioClose(fd);
		}
*/
	}
	else{
		//FONTX2
		if(!strncmp(path, "mc:", 3)){
			strcpy(fullpath, "mc0:");
			strcat(fullpath, path+3);
			if(checkFONTX2header(fullpath)<0){
				fullpath[2]='1';
				if(checkFONTX2header(fullpath)<0)
					fullpath[0]=0;
			}
		}
		else
			strcpy(fullpath, path);

		//�t�H���g�t�@�C���I�[�v��
		fd = fioOpen(fullpath, O_RDONLY);
		if(fd<0) return -1;
	
		//�T�C�Y�𒲂ׂ�
		size = fioLseek(fd, 0, SEEK_END);
		fioLseek(fd, 0, SEEK_SET);	//�V�[�N��0�ɖ߂�

		//���������m��
		font_ascii = (char*)malloc(size);
		if(font_ascii==NULL){
			fioClose(fd);
			return -1;
		}
		
		//�������ɓǂݍ���
		fioRead(fd, font_ascii, (size_t)size);

		//�N���[�Y
		fioClose(fd);
	}

	//�w�b�_�̃|�C���^
	fontx_header_ascii = (FONTX_HEADER*)font_ascii;

	//�w�b�_�`�F�b�N
	if(strncmp(fontx_header_ascii->Identifier, "FONTX2", 6)!=0)
		return -1;
	if(fontx_header_ascii->CodeType!=0)
		return -1;

	//�t�H���g�̏��
	strncpy(ascii_data.font_name, fontx_header_ascii->FontName, 8);
	ascii_data.font_name[8] = '\0';
	//1�����̃T�C�Y
	ascii_data.width = fontx_header_ascii->XSize;
	ascii_data.height = fontx_header_ascii->YSize;
	//1�����̃T�C�Y�Z�o
	ascii_data.size = ((ascii_data.width-1)/8+1) * ascii_data.height;
	//
	ascii_data.Tnum = 0;
	//
	ascii_data.offset = 17;

	SetHeight();

	//�t�H���g���[�h����
	init_ascii=1;
	return 0;
}
//------------------------------------------------------------
//�����t�H���g�����[�h
int InitFontKnaji(const char *path)
{
	int fd=0;
	size_t size;
	FONTX_HEADER *fontx_header_kanji;
	char fullpath[MAX_PATH];

	if(init_kanji==1) FreeFontKanji();

	if(strcmp(path,"rom0:KROM")==0 || strcmp(path,"systemfont")==0){
		//BIOSFont
		//�t�H���g�t�@�C���I�[�v��
		fd = fioOpen("rom0:KROM", O_RDONLY);
		if(fd<0) return -1;

		//���������m�� ���zFONTX2 KROM
		size = 18 + 51*4 + 30*3489;	//�w�b�_�T�C�Y + �e�[�u���̐�*4 + 1�����̃T�C�Y*3489����
		font_kanji = (char*)malloc(size);
		memset(font_kanji, 0, size);
		if(font_kanji==NULL){
			fioClose(fd);
			return -1;
		}

		//�������ɓǂݍ���
		fioRead(fd, font_kanji + 18 + 51*4, 30*3489);//�w�b�_�T�C�Y + �e�[�u���̐�*4 ,1�����̃T�C�Y*3489����

		//�N���[�Y
		fioClose(fd);

		//�w�b�_�̃|�C���^
		fontx_header_kanji = (FONTX_HEADER*)font_kanji;

		//�w�b�_�쐬
		strncpy(fontx_header_kanji->Identifier, "FONTX2", 6);
		strncpy(fontx_header_kanji->FontName, "KROM_k", 8);
		fontx_header_kanji->XSize = 16;
		fontx_header_kanji->YSize = 15;
		fontx_header_kanji->CodeType = 1;
		fontx_header_kanji->Tnum = 51;
		//�e�[�u��
		memcpy(font_kanji+18,font_sjis_table,51*4);

/*
		KROM�̊����t�H���g��FONTX2�Ń_���v
		{
			fd=fioOpen("host:KROM_k.fnt",O_WRONLY | O_TRUNC | O_CREAT);
			fioWrite(fd, font_kanji, size);
			fioClose(fd);
		}
*/
	}
	else{
		//FONTX2
		if(!strncmp(path, "mc:", 3)){
			strcpy(fullpath, "mc0:");
			strcat(fullpath, path+3);
			if(checkFONTX2header(fullpath)<0){
				fullpath[2]='1';
				if(checkFONTX2header(fullpath)<0)
					fullpath[0]=0;
			}
		}
		else
			strcpy(fullpath, path);

		//�t�H���g�t�@�C���I�[�v��
		fd = fioOpen(fullpath, O_RDONLY);
		if(fd<0) return -1;

		//�T�C�Y�𒲂ׂ�
		size = fioLseek(fd, 0, SEEK_END);
		fioLseek(fd, 0, SEEK_SET);	//�V�[�N��0�ɖ߂�

		//���������m��
		font_kanji = (char*)malloc(size);
		if(font_kanji==NULL){
			fioClose(fd);
			return -1;
		}

		//�������ɓǂݍ���
		fioRead(fd, font_kanji, (size_t)size);

		//�N���[�Y
		fioClose(fd);
	}
	
	//�w�b�_�̃|�C���^
	fontx_header_kanji = (FONTX_HEADER*)font_kanji;

	//�w�b�_�`�F�b�N
	if(strncmp(fontx_header_kanji->Identifier, "FONTX2", 6)!=0)
		return -1;
	if(fontx_header_kanji->CodeType!=1)
		return -1;

	//�t�H���g�̏��
	strncpy(kanji_data.font_name, fontx_header_kanji->FontName, 8);
	kanji_data.font_name[8] = '\0';
	//1�����̃T�C�Y
	kanji_data.width = fontx_header_kanji->XSize;
	kanji_data.height = fontx_header_kanji->YSize;
	//1�����̃T�C�Y�Z�o
	kanji_data.size = ((kanji_data.width-1)/8+1) * kanji_data.height;
	//
	kanji_data.Tnum = fontx_header_kanji->Tnum;
	//
	kanji_data.offset = 18 + kanji_data.Tnum*4;

	SetHeight();

	//�t�H���g���[�h����
	init_kanji=1;
	return 0;
}
//------------------------------------------------------------
//���p�t�H���g���J��
void FreeFontAscii(void)
{
	free(font_ascii);
	memset(&ascii_data, 0, sizeof(FONTX_DATA));
	init_ascii=0;
	return;
}
//------------------------------------------------------------
//�����t�H���g���J��
void FreeFontKanji(void)
{
	free(font_kanji);
	memset(&kanji_data, 0, sizeof(FONTX_DATA));
	init_kanji=0;
	return;
}

//------------------------------------------------------------
//�t�H���g�̃}�[�W����ݒ�
int SetFontMargin(int type, int Margin)
{
	if(type<CHAR_MARGIN || type>KANJI_FONT_MARGIN_LEFT) return -1;

	switch(type)
	{
		case CHAR_MARGIN:
			char_Margin = Margin;
			SetHeight();
			break;
		case LINE_MARGIN:
			line_Margin=Margin;
			SetHeight();
			break;
		case ASCII_FONT_MARGIN_TOP:
			ascii_MarginTop=Margin;
			break;
		case ASCII_FONT_MARGIN_LEFT:
			ascii_MarginLeft=Margin;
			break;
		case KANJI_FONT_MARGIN_TOP:
			kanji_MarginTop=Margin;
			break;
		case KANJI_FONT_MARGIN_LEFT:
			kanji_MarginLeft=Margin;
			break;
	}

	return 0;
}
//------------------------------------------------------------
//�t�H���g�̃}�[�W�����擾
int GetFontMargin(int type)
{
	if(type<CHAR_MARGIN || type>KANJI_FONT_MARGIN_LEFT) return -1;

	switch(type)
	{
		case CHAR_MARGIN:
			return char_Margin;
		case LINE_MARGIN:
			return line_Margin;
		case ASCII_FONT_MARGIN_TOP:
			return ascii_MarginTop;
		case ASCII_FONT_MARGIN_LEFT:
			return ascii_MarginLeft;
		case KANJI_FONT_MARGIN_TOP:
			return kanji_MarginTop;
		case KANJI_FONT_MARGIN_LEFT:
			return kanji_MarginLeft;
	}

	return 0;
}

//------------------------------------------------------------
//�t�H���g�T�C�Y�̎擾
int GetFontSize(int type)
{
	switch(type)
	{
		case ASCII_FONT_WIDTH:
			return ascii_data.width;
		case ASCII_FONT_HEIGHT:
			return ascii_data.height;
		case KANJI_FONT_WIDTH:
			return kanji_data.width;
		case KANJI_FONT_HEIGHT:
			return kanji_data.height;
	}
	return 0;
}

//------------------------------------------------------------
//�t�H���g�{�[���h��ݒ�
void SetFontBold(int flag)
{
	font_bold = flag;
	return;
}

//------------------------------------------------------------
//�t�H���g�{�[���h�̎擾
int GetFontBold(void)
{
	return font_bold;
}

//------------------------------------------------------------
//FONTX2�t�@�C���̃w�b�_�`�F�b�N
int checkFONTX2header(const char *path)
{
	char *buf=NULL;
	int fd, size=0;
	char fullpath[MAX_PATH], tmp[MAX_PATH], *p;
	FONTX_HEADER *fontx_header;

	strcpy(fullpath,path);

	if(!strncmp(fullpath, "hdd0", 4)) {
		sprintf(tmp, "hdd0:%s", &path[6]);
		p = strchr(tmp, '/');
		sprintf(fullpath, "pfs0:%s", p);
		*p = 0;
		fileXioMount("pfs0:", tmp, FIO_MT_RDONLY);
		if ((fd = fileXioOpen(fullpath, O_RDONLY, FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH)) < 0){
			fileXioUmount("pfs0:");
			goto error;
		}
		size = fileXioLseek(fd, 0, SEEK_END);
		if (!size){
			fileXioClose(fd);
			fileXioUmount("pfs0:");
			goto error;
		}
		fileXioLseek(fd, 0, SEEK_SET);
		buf = (char*)malloc(17);
		fileXioRead(fd, buf, 17);
		fileXioClose(fd);
		fileXioUmount("pfs0:");
	}
	else if(!strncmp(fullpath, "mc", 2) || !strncmp(fullpath, "mass", 4) || !strncmp(fullpath, "cdfs", 4)) {
		if ((fd = fioOpen(fullpath, O_RDONLY)) < 0) 
			goto error;
		size = fioLseek(fd, 0, SEEK_END);
		if (!size){
			fioClose(fd);
			goto error;
		}
		fioLseek(fd, 0, SEEK_SET);
		buf = (char*)malloc(17);
		fioRead(fd, buf, 17);
		fioClose(fd);
	}
	else {
		return 0;
	}

	//�w�b�_�̃|�C���^
	fontx_header = (FONTX_HEADER*)buf;

	//�w�b�_�`�F�b�N
	if(strncmp(fontx_header->Identifier, "FONTX2", 6)!=0){
		free(buf);
		goto error;
	}

	free(buf);
	return 1;
error:
	return -1;
}

//-------------------------------------------------
//���p�����̕\��
void drawChar(unsigned char c, int x, int y, uint64 color)
{
	unsigned int i, j;
	unsigned char cc;
	unsigned char *pc=0;
	int n;

	//���������Ă��Ȃ����A���������s���Ă���
	if(!init_ascii) return;

	//���p�X�y�[�X�̂Ƃ��́A�������Ȃ�
	if(c==' ') return;

	pc = &font_ascii[ascii_data.offset + c * ascii_data.size];
	cc = *pc++;

	for(i=0; i<ascii_data.height; i++){
		n = ascii_data.width;
		if(n>8) n=8;
		for(j=0; j<n; j++){
			if(cc & 0x80){
				if(setting->FontBold)
					itoLine(color, x+j, y+i, 0, color, x+j+2, y+i, 0);
				else
					itoPoint(color, x+j, y+i, 0);
			}
			cc = cc << 1;
		}
		cc = *pc++;
		if(ascii_data.width>8){
			n = ascii_data.width-8;
			if(n>8) n=8;
			for(j=0; j<n; j++) {
				if(cc & 0x80){
					if(setting->FontBold)
						itoLine(color, x+8+j, y+i, 0, color, x+8+j+2, y+i, 0);
					else
						itoPoint(color, x+8+j, y+i, 0);
				}
				cc = cc << 1;
			}
			cc = *pc++;
		}
	}
	return;
}

//-------------------------------------------------
//�S�p�����̕\��
void drawChar_SJIS(unsigned int c, int x, int y, uint64 color)
{
	FONTX_HEADER *fontx_header_kanji;
	int i, j, a;
	int ret, sum;
	unsigned char cc;
	unsigned char *pc;
	int n;

	//���������Ă��Ȃ����A���������s���Ă���
	if(!init_kanji) return;

	//�w�b�_�̃|�C���^
	fontx_header_kanji = (FONTX_HEADER*)font_kanji;

	//���Ԗڂ̃e�[�u���ɂ��邩���ׂ�
	ret=-1;
	for(i=0;i<kanji_data.Tnum;i++){
		if((fontx_header_kanji->Block[i].Start <= c) && (fontx_header_kanji->Block[i].End >= c)){
			ret=i;
			break;
		}
	}
	//������Ȃ��Ƃ��́A�Ȃɂ����Ȃ�
	if (ret==-1) return;

	//�A�h���X�Z�o
	sum = 0;
	for(i=0;i<ret;i++){
		sum += fontx_header_kanji->Block[i].End - fontx_header_kanji->Block[i].Start;
	}

	//
	a = sum + ret + ( c - fontx_header_kanji->Block[ret].Start );
	pc = &font_kanji[kanji_data.offset + a * kanji_data.size];
	cc = *pc++;

	for(i=0; i<kanji_data.height; i++) {
		n = kanji_data.width;
		if(n>8) n=8;
		for(j=0; j<n; j++) {
			if(cc & 0x80){
				if(setting->FontBold)
					itoLine(color, x+j, y+i, 0, color, x+j+2, y+i, 0);
				else
					itoPoint(color, x+j, y+i, 0);
			}
			cc = cc << 1;
		}
		cc = *pc++;
		if(kanji_data.width>8){
			n = kanji_data.width-8;
			if(n>8) n=8;
			for(j=0; j<n; j++) {
				if(cc & 0x80){
					if(setting->FontBold)
						itoLine(color, x+8+j, y+i, 0, color, x+8+j+2, y+i, 0);
					else
						itoPoint(color, x+8+j, y+i, 0);
				}
				cc = cc << 1;
			}
			cc = *pc++;
			if(kanji_data.width>16){
				n = kanji_data.width-16;
				if(n>8) n=8;
				for(j=0; j<n; j++) {
					if(cc & 0x80){
						if(setting->FontBold)
							itoLine(color, x+16+j, y+i, 0, color, x+16+j+2, y+i, 0);
						else
							itoPoint(color, x+16+j, y+i, 0);
					}
					cc = cc << 1;
				}
				cc = *pc++;
			}
		}
	}
}

//-------------------------------------------------
// ���W���w�肵�ĕ������\��
int printXY(const unsigned char *s, int x, int y, uint64 color, int draw)
{
	uint64 color2;	//�A���t�@�t��
	uint16 code;
	int i;

	color = color&0x00FFFFFF;	//�����x�����O
	color = color|0x80000000;	//�s����
	color2 = color|0x10000000;	//������

	i=0;
	while(s[i]){
		if (( s[i]>=0x81 && s[i]<=0x9f ) || ( s[i]>=0xe0 && s[i]<=0xff )){	//SJIS
			code = s[i++];
			code = (code<<8) + s[i++];
			if(draw){
				if(setting->flickerControl){
					//�A���t�@�u�����h�L��
					itoPrimAlphaBlending( TRUE );
					drawChar_SJIS(code, x+kanji_MarginLeft, y+kanji_MarginTop+1, color2);
					//�A���t�@�u�����h����
					itoPrimAlphaBlending( FALSE );
				}
				drawChar_SJIS(code, x+kanji_MarginLeft, y+kanji_MarginTop, color);
			}
			x += kanji_data.width + char_Margin * 2;
		}
		else{
			if(draw){
				if(setting->flickerControl){
					//�A���t�@�u�����h�L��
					itoPrimAlphaBlending( TRUE );
					drawChar(s[i], x+ascii_MarginLeft, y+ascii_MarginTop+1, color2);
					//�A���t�@�u�����h����
					itoPrimAlphaBlending( FALSE );
				}
				drawChar(s[i], x+ascii_MarginLeft, y+ascii_MarginTop, color);
			}
			i++;
			x += ascii_data.width + char_Margin;
		}
	}

	return x;
}
/*
//int CurrentPos_x;	//�J�����g�|�W�V����x
//int CurrentPos_y;	//�J�����g�|�W�V����y

//------------------------------------------------------------
//�J�����g�|�W�V������ݒ�
int SetCurrentPos(int x, int y)
{
	CurrentPos_x = x;
	CurrentPos_y = y;
	return 0;
}

//------------------------------------------------------------
//�J�����g�|�W�V�������擾
int GetCurrentPos(int type)
{
	if(type==CURRENTPOS_X)
		return CurrentPos_x;
	if(type==CURRENTPOS_Y)
		return CurrentPos_y;
	return 0;
}

//------------------------------------------------------------
// �J�����g�|�W�V�����̍��W�ɕ������\��
int printXY2(const unsigned char *s, uint64 color, int draw)
{
	uint64 color2;	//�A���t�@�t��
	uint16 code;
	int i;
	int x,y;

	x=CurrentPos_x;
	y=CurrentPos_y;

	color = color&0x00FFFFFF;	//�����x�����O
	color = color|0x80000000;	//�s����
	color2 = color|0x10000000;	//������

	i=0;
	while(s[i]){
		if (( s[i]>=0x81 && s[i]<=0x9f ) || ( s[i]>=0xe0 && s[i]<=0xff )){	//SJIS
			code = s[i++];
			code = (code<<8) + s[i++];
			if(draw){
				if(setting->flickerControl){
					//�A���t�@�u�����h�L��
					itoPrimAlphaBlending( TRUE );
					drawChar_SJIS(code, x+kanji_MarginLeft, y+kanji_MarginTop+1, color2);
					//�A���t�@�u�����h����
					itoPrimAlphaBlending( FALSE );
				}
				drawChar_SJIS(code, x+kanji_MarginLeft, y+kanji_MarginTop, color);
			}
			x += kanji_data.width + char_Margin * 2;
		}
		else{
			if(draw){
				if(setting->flickerControl){
					//�A���t�@�u�����h�L��
					itoPrimAlphaBlending( TRUE );
					drawChar(s[i], x+ascii_MarginLeft, y+ascii_MarginTop+1, color2);
					//�A���t�@�u�����h����
					itoPrimAlphaBlending( FALSE );
				}
				drawChar(s[i], x+ascii_MarginLeft, y+ascii_MarginTop, color);
			}
			i++;
			x += ascii_data.width + char_Margin;
		}
	}

	CurrentPos_y += FONT_HEIGHT;

	return x;
}
*/
