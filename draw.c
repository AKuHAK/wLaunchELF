#include "launchelf.h"
#define tmpbuffersize	256*256
//----------------------------------------------------------
#if 1
static int alloced=0;
static void *X_malloc(size_t mallocsize)
{
	void *ret;
	ret = malloc(mallocsize);
	if (ret == NULL)
	//	printf("%s[%u] %s: malloc failed (ofs: %08X, size: %d)\n", __FILE__, __line__, __func__, (unsigned int) ret, mallocsize);
		printf("draw: malloc failed (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	else
	//	printf("%s[%u] %s: malloc vaild (ofs: %08X, size: %d) [%d]\n", __FILE__, __line__, __func__, (unsigned int) ret, mallocsize, ++alloced);
		printf("draw: malloc vaild (ofs: %08X, size: %d) [%d]\n", (unsigned int) ret, mallocsize, ++alloced);
	return ret;
}
static void X_free(void *mallocdata)
{
	if (mallocdata != NULL) {
		printf("draw: free vaild (ofs: %08X) [%d]\n", (unsigned int) mallocdata, --alloced);
	//	printf("%s[%u] %s: free vaild (ofs: %08X) [%d]\n", __FILE__, __line__, __func__, (unsigned int) mallocdata, --alloced);
		free(mallocdata);
	} else 
	//	printf("%s[%u] %s: free failed (ofs: %08X)\n", __FILE__, __line__, __func__, (unsigned int) mallocdata);
		printf("draw: free failed (ofs: %08X)\n", (unsigned int) mallocdata);
	mallocdata = NULL;
}
#define	malloc	X_malloc
#define free	X_free
#endif
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
	unsigned short dfVersion;		// 0x0200 => 2.0
	unsigned int   dfSize;
	unsigned char  dfCopyright[60];	// asciiz
	unsigned short dfType;
	unsigned short dfPoints;
	unsigned short dfVertRes;		// uLE=  96 dpi
	unsigned short dfHorizRes;		// uLE=  96 dpi
	unsigned short dfAscent;		// baseline
	unsigned short dfInternalLeading;
	unsigned short dfExternalLeading;
	unsigned char  dfItalic;
	unsigned char  dfUnderline;
	unsigned char  dfStrikeOut;
	unsigned short dfWeight;
	unsigned char  dfCharSet;
	unsigned short dfPixWidth;		// uLE=   8
	unsigned short dfPixHeight;		// uLE=  16
	unsigned char  dfPitchAndFamily;
	unsigned short dfAvgWidth;		// =Width
	unsigned short dfMaxWidth;		// =Width
	unsigned char  dfFirstChar;		// uLE=  32
	unsigned char  dfLastChar;		// uLE= 255
	unsigned char  dfDefaultChar;	// uLE=  32
	unsigned char  dfBreakChar;		// 
	unsigned short dfWidthbytes;
	unsigned int   dfDevice;		// (char*)
	unsigned int   dfFace;			// (char*)
	unsigned int   dfBitsPointer;	// (char*) 
	unsigned int   dfBitsOffset;	// (char*)
	unsigned char  dfReserved;
	struct {
		unsigned short charWidth;		// 
		unsigned short charOffset;		// for Version 2.0
	} dfCharTable[];
} __attribute__((packed)) CPE_FONT_HEADER;

typedef struct {
	char font_name[9];
	int width;
	int height;
	int size;	//1�������̃T�C�Y
	int Tnum;
	int offset;
} FONTX_DATA;

#ifdef ENABLE_ICON
extern uint8 *icon_iif[];
extern int size_icon_iif;
#endif

//----------------------------------------------------------
itoGsEnv screen_env;
GSREG gsregs[MAX_GSREG];
int SCANRATE=60;
int initbiosfont=0;
char *biosfont=NULL;
int SCREEN_LEFT;
int SCREEN_TOP;
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 448;
int SCREEN_MARGIN;
int FONT_WIDTH;
int FONT_HEIGHT;
int MAX_ROWS;
int MAX_ROWS_X;

int char_Margin;	//�����̊Ԋu
int line_Margin;	//�s�̊Ԋu
int font_bold;
int font_half, font_vhalf;
int fonthalfmode=0;
int fontchange=0;

int ffmode;
int interlace;
int flickerfilter;
int fieldnow;
/*
uint64 matrix[2] = {
	0x4026264040262640,
	0x2640402626404026,
};
*/
uint64 totalcount=0;
//ascii
int init_ascii=0;	//���������������Ă��Ȃ����̃t���O
int framebuffers=2;
int fieldbuffers=2;
char *font_ascii=NULL;	//�t�H���g�̃o�b�t�@
unsigned int font_ascii_size;
FONTX_DATA ascii_data;	//�t�H���g�̏��
int ascii_MarginTop;	//��̃}�[�W��
int ascii_MarginLeft;	//���̃}�[�W��
//kanji
int init_kanji=0;
char *font_kanji=NULL;
unsigned int font_kanji_size;
FONTX_DATA kanji_data;
int kanji_MarginTop;
int kanji_MarginLeft;
//font
//ptrdiff_t fontoffset[11536]={0};
typedef struct {
		unsigned char type;		// 0:point,1:line
		unsigned char alpha;	
		unsigned char left;		// point:(left,top),(right,bottom) (2points)
		unsigned char top;		// line:(left,top)-(right,bottom) (1line)
		unsigned char right;
		unsigned char bottom;
		unsigned char dummy[2];
} cachestruct;
typedef struct {
	ptrdiff_t offset;
	short status;
	short width;
	short height;
	short caches;
//	int left;
//	int top;
	cachestruct *cache;
} fontcache;
fontcache *font=NULL;

unsigned short krom_sjis_table[] = {
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

void drawChar_1bpp(void *src, int x, int y, uint64 color, int w, int h);
void drawChar_bilinear(void *src, int x, int y, uint64 color, int w, int h);
void drawChar_filter(void *src, int x, int y, uint64 color, int w, int h);
void drawChar_8bpp(void *src, int x, int y, uint64 color, uint64 back, int w, int h);
void drawChar_resize(void *dist, void *src, int dw, int dh, int sw, int sh);
void drawChar_unpack(void *dist, void *src, int w, int h);
void drawChar_resize2(void *dist, void *src, int w, int h, int x, int y);
void drawChar_normalize(void *src, int dw, int dh, int alpha);
void drawChar_cache(cachestruct *src, int prims, int x, int y, uint64 color);
static unsigned char bmpsrc[tmpbuffersize], bmpdst[tmpbuffersize];
//unsigned char tmpbuffer[tmpbuffersize*2];

void texDestroy(int fd);
int texGetFontSize(int fw, int fh, int fs, int *tw, int *th);
int texCreate(int fd, int tw, int th);

//-------------------------------------------------
// VSYNC
void int_vsync()
{
	fieldnow^=1;
	//if (fieldbuffers == 2)
	if (ffmode == ITO_FRAME)
		itoSetVisibleFrameBuffer(fieldnow^(screen_env.screen.y&1));
	totalcount++;
	itoEI();
}

void setup_vsync()
{
	itoDI();
	itoAddIntcHandler(ITO_INTC_VSYNC_START, int_vsync, 0);
	itoEnableIntc(ITO_INTC_VSYNC_START);
	fieldnow = itoGetVisibleFrameBuffer();
	itoEI();
}

//-------------------------------------------------
// setup ito
void setupito(int tvmode)
{
	int	vmode, height_t, gstop, vx, vy;
	int width, height, dither, depth;
	vmode = tvmode;
	if (!gsregs[tvmode].loaded) vmode = (ITO_VMODE_AUTO)-1;
	// screen resolution
	printf("vmode: %d: (%02X:%dx%d) %d,%d,%d,%d\n", vmode, gsregs[vmode].vmode, gsregs[vmode].width, gsregs[vmode].height, gsregs[vmode].dither, gsregs[vmode].interlace, gsregs[vmode].ffmode, gsregs[vmode].vesa);
	
	//ffmode			= gsregs[vmode].ffmode & 1;
	//interlace		= gsregs[vmode].interlace & 1;
	ffmode			= setting->screen_ffmode[vmode] > 0 ? (setting->screen_ffmode[vmode]-1):(gsregs[vmode].ffmode & 1);
	interlace		= setting->screen_interlace[vmode] > 0 ? (setting->screen_interlace[vmode]-1):(gsregs[vmode].interlace & 1);
	depth			= setting->screen_depth[vmode] > 0 ? (5-setting->screen_depth[vmode]):gsregs[vmode].psm;
	width			= setting->screen_scan[vmode] > 0 ? gsregs[vmode].defwidth:gsregs[vmode].width;
	height			= setting->screen_scan[vmode] > 0 ? gsregs[vmode].defheight:gsregs[vmode].height;
	dither			= setting->screen_dither[vmode] > 0 ? (setting->screen_dither[vmode]-1):gsregs[vmode].dither;
	height_t		= height;
	gstop			= 1;
	if ((gsregs[vmode].vmode == 2) || (gsregs[vmode].vmode == 3) || (gsregs[vmode].vmode == 81) || (gsregs[vmode].vmode == 130) || (gsregs[vmode].vmode == 131)) {
		if (!interlace){
			gstop = 2;
			height_t *= 2;
		} else if (ffmode)
			height_t *= 2;
		if (gsregs[vmode].ffmode != ffmode) {
			if (ffmode) {
				height /= 2;
				height_t /= 2;
			} else {
				height *= 2;
				height_t *= 2;
			}
		}
		if (gsregs[vmode].interlace != interlace) {
			if (!interlace && !ffmode) {
				height /= 2;
				height_t /= 2;
			}
		}
	}
	//SCREEN_LEFT		= 0;
	//SCREEN_TOP		= 0;
	SCREEN_WIDTH	= width;
	SCREEN_HEIGHT	= height;
	vx				= SCREEN_LEFT + gsregs[vmode].left - width*(gsregs[vmode].magx+1)/2;
	vy				= SCREEN_TOP  + (gsregs[vmode].top - height_t*(gsregs[vmode].magy+1)/2)/gstop;
	if (vx < 0) {
		SCREEN_LEFT-= vx;
		vx = 0;
	}
	if (vy < 0) {
		SCREEN_TOP -= vy;
		vy = 0;
	}
	screen_env.screen.width		= width;
	screen_env.screen.height	= height_t/gstop;
	screen_env.screen.psm		= depth;
	screen_env.screen.mag_x		= gsregs[vmode].magx;
	screen_env.screen.mag_y		= gsregs[vmode].magy;
	screen_env.screen.x			= vx;
	screen_env.screen.y			= vy;
	screen_env.doublebuffer		= gsregs[vmode].doublebuffer;
	framebuffers 				= gsregs[vmode].doublebuffer+1;
	fieldbuffers				= 1;
	if (ffmode) fieldbuffers = framebuffers;
	screen_env.zpsm				= gsregs[vmode].zpsm;
	// scissor 
	screen_env.scissor_x1		= 0;
	screen_env.scissor_x2		= width;
	screen_env.scissor_y1		= 0;
	screen_env.scissor_y2		= height;
	//if (setting->screen_scan[vmode] || (gsregs[vmode].ffmode != ffmode) || (gsregs[vmode].interlace != interlace))
	// misc
	screen_env.dither			= dither;
	screen_env.matrix			= ( 0x5D7F91B36E4CA280
								  & 0xEEEEEEEEEEEEEEEE
								  ) >> 1;
//	screen_env.matrix			= 0x5D7F91B36E4CA280;
	screen_env.interlace		= interlace;
	screen_env.ffmode			= ffmode;
	screen_env.vmode			= gsregs[vmode].vmode;
	screen_env.vesa				= gsregs[vmode].vesa;
	
	vmode = gsregs[vmode].vmode;
	if ((vmode == 2) || (vmode==130) || (vmode == 80) || (vmode == 81) || (vmode == 82) || (vmode == 114)) {
		SCANRATE = 60;
	} else if ((vmode == 3) || (vmode == 131)) {
		SCANRATE = 50;
	} else {
		int vm[32] = {26,27,28,29,42,43,44,45,46,59,60,61,62,74,75, 1};
		int rm[32] = {60,72,75,85,56,60,72,75,85,60,70,75,85,60,75,30};
		SCANRATE = 50;
		for (vx=0;vm[vx]!=1;vx++) {
			if (vmode == vm[vx]) {
				SCANRATE = rm[vx];
				break;
			}
		}
	}
	printf("\tRefresh Rate: %d Hz\n", SCANRATE);
	//printf("\tscreen: (%d,%d)\n", screen_env.screen.x, screen_env.screen.y);
#if 0
	printf("draw: screen setup:\n");
	printf("\tvmode: %02X\n", screen_env.vmode);
	printf("\tbuffer size: %dx%d\n", screen_env.screen.width, screen_env.screen.height);
	printf("\tvram size: %dx%d\n", buffer_width, buffer_height_t);
	//printf("\tscreen offset: (%d,%d)\n", SCREEN_LEFT, SCREEN_TOP);
	printf("\tframe buffer 1 position: (%d,%d)-(%d,%d)\n", screen_env.framebuffer1.x,screen_env.framebuffer1.y);
	printf("\tframe buffer 2 position: (%d,%d)-(%d,%d)\n", screen_env.framebuffer2.x,screen_env.framebuffer2.y);
	printf("\tZ buffer offset: (%d,%d)\n", screen_env.zbuffer.x,screen_env.zbuffer.y);
	printf("\tscissor: (%d,%d)-(%d,%d)\n", screen_env.scissor_x1, screen_env.scissor_y1, screen_env.scissor_x2, screen_env.scissor_y2);
	printf("\tdither,interlace,ffmode: %d,%d,%d\n",  screen_env.dither, screen_env.interlace, screen_env.ffmode);
#endif
	//printf("\tscissor: (%d,%d)-(%d,%d)\n", screen_env.scissor_x1, screen_env.scissor_y1, screen_env.scissor_x2, screen_env.scissor_y2);
	itoGsEnvSubmit(&screen_env);
	//�A���t�@�u�����h
	itoSetAlphaBlending(
		ITO_ALPHA_COLOR_SRC, // A = COLOR SOURCE
		ITO_ALPHA_COLOR_DST, // B = COLOR DEST
		ITO_ALPHA_VALUE_SRC, // C = ALPHA VALUE SOURCE
		ITO_ALPHA_COLOR_DST, // C = COLOR DEST
		0x80);				 // Fixed Value

	itoZBufferUpdate(FALSE);
	itoZBufferTest(FALSE, 0);
	itoSetTextureBufferBase( itoGetZBufferBase() );
	printf("\tframebuffer0 offset: %08X\n", itoGetFrameBufferBase(0));
	printf("\tframebuffer1 offset: %08X\n", itoGetFrameBufferBase(1));
	printf("\tTexture buffer base: %08X\n", itoGetTextureBufferBase());
	itoSetBgColor(setting->color[COLOR_OUTSIDE]);
	SetHeight();
	
	fieldnow = itoGetVisibleFrameBuffer()^1;
}
/*		�f�B�U�����O
	�g�D�I�f�B�U�@(4x4,bayer�^)
	 0, 8, 2,10,
	12, 4,14, 6,
	 3,11, 1, 9,
	15, 7,13, 5,
*/
//-------------------------------------------------
// �����߃J���[�擾�p 

uint64 half(uint64 color1, uint64 color2, int blend) {
	unsigned int l3,l2,l1,la;
	unsigned int r3,r2,r1,ra;
	unsigned int t3,t2,t1,ta;
	//uint64 lq,rq,tq;
	unsigned int lb,rb;
	if (blend <= 0) return color1;
	if (blend > 0xff) return color2;
	l3 = (color1 >> 16) & 0xff;
	l2 = (color1 >>  8) & 0xff;
	l1 =  color1        & 0xff;
	la = (color1 >> 24) & 0xff;
	//lq =  color1 >> 32;
	r3 = (color2 >> 16) & 0xff;
	r2 = (color2 >>  8) & 0xff;
	r1 =  color2        & 0xff;
	ra = (color2 >> 24) & 0xff;
	//rq =  color2 >> 32;
	rb = blend & 0xff;
	lb = 0x100 - rb;
	t3 = (l3 * lb + r3 * rb) >> 8;
	t2 = (l2 * lb + r2 * rb) >> 8;
	t1 = (l1 * lb + r1 * rb) >> 8;
	ta = (la * lb + ra * rb) >> 8;
	//tq = (lq * lb + rq * rb) >> 8;
	return /*(tq<<32)|*/(ta<<24)|(t3<<16)|(t2<<8)|t1;
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
		SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3.5), 0);
	//�A���t�@�u�����h����
	itoPrimAlphaBlending(FALSE);
}

int drawDarks(int ret)
{
	itoSetActiveFrameBuffer(itoGetActiveFrameBuffer()^1);
	drawDark();
	itoSetActiveFrameBuffer(itoGetActiveFrameBuffer()^1);
	drawDark();
	return ret;
}
//-------------------------------------------------
// �_�C�A���O�̔w�i
void drawDialogTmp(int x1, int y1, int x2, int y2, uint64 color1, uint64 color2)
{
	//
	itoSprite(color1, x1, y1, x2, y2, 0);
	drawFrame(x1+2, y1+2, x2-3, y2-3, color2);
}

//-------------------------------------------------
// ��ʕ\���̃e���v���[�g
int drawStringLimit(const unsigned char *s, int charset, int sx, int sy, uint64 fcol, uint64 scol, unsigned char *ctrlchars, int right);
void setScrTmp(const char *msg0, const char *msg1)
{
	uint64 color1,color;
	uint64 color2;	//�A���t�@�t��

	color = setting->color[COLOR_FRAME]&0x00FFFFFF;	//�����x�����O
	color1 = color|0x80000000;	//�s����
	color2 = color|(setting->flicker_alpha << 24);	//������
	//color2 = half(color, setting->color[COLOR_BACKGROUND], 0x80);
	itoSprite(setting->color[COLOR_BACKGROUND], 0, SCREEN_MARGIN,
		SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2 -line_Margin +flickerfilter, 0);
	itoSprite(setting->color[COLOR_BACKGROUND], 0, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3.5), SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	// �o�[�W�����\�L
	drawString(LBF_VER, TXT_ASCII, FONT_WIDTH*2, SCREEN_MARGIN, setting->color[COLOR_TEXT], setting->color[COLOR_HIGHLIGHTTEXT], 0);

	// ���b�Z�[�W
	drawStringLimit(msg0, TXT_SJIS, FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT, setting->color[COLOR_TEXT], setting->color[COLOR_HIGHLIGHTTEXT], 0, SCREEN_WIDTH - FONT_WIDTH*2);

	//��̉���
	itoLine(color1, 0, SCREEN_MARGIN+FONT_HEIGHT*2.5, 0,
		color1, SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2.5, 0);	
	//���̉���
	itoLine(color1, 0, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3.5), 0,
		color1, SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3.5), 0);	

	//FLICKER CONTROL: ON
	if(flickerfilter==TRUE){
		//�A���t�@�u�����h�L��
		itoPrimAlphaBlending( TRUE );
		//��̉���
		itoLine(color2, 0, SCREEN_MARGIN+FONT_HEIGHT*2.5+1, 0,
			color2, SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2.5+1, 0);	
		//���̉���
		itoLine(color2, 0, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3.5)+1, 0,
			color2, SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3.5)+1, 0);	
		//�A���t�@�u�����h����
		itoPrimAlphaBlending(FALSE);
	}

	// �������
	drawStringLimit(msg1, TXT_SJIS, FONT_WIDTH*1, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+4), setting->color[COLOR_TEXT], setting->color[COLOR_HIGHLIGHTTEXT], 0, SCREEN_WIDTH);
}

//-------------------------------------------------
// ���b�Z�[�W�`��
void drawMsg(const char *msg)
{
	itoSprite(setting->color[COLOR_BACKGROUND], 0, SCREEN_MARGIN+FONT_HEIGHT,
		SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2 -line_Margin +flickerfilter, 0);
	//���b�Z�[�W
	drawStringLimit(msg, TXT_SJIS, FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT, setting->color[COLOR_TEXT], setting->color[COLOR_TEXT], 0, SCREEN_WIDTH - FONT_WIDTH*2);
	itoGsFinish();
	if (framebuffers == 2) {
		itoSetActiveFrameBuffer(itoGetActiveFrameBuffer()^1);
		itoSprite(setting->color[COLOR_BACKGROUND], 0, SCREEN_MARGIN+FONT_HEIGHT,
			SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2 -line_Margin +flickerfilter, 0);
		//���b�Z�[�W
		drawStringLimit(msg, TXT_SJIS, FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT, setting->color[COLOR_TEXT], setting->color[COLOR_TEXT], 0, SCREEN_WIDTH - FONT_WIDTH*2);
		itoGsFinish();
		itoSetActiveFrameBuffer(itoGetActiveFrameBuffer()^1);
	}
}

//-------------------------------------------------
// ��ʂ̃N���A
void clrScr(uint64 color)
{
	itoSprite(color, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}

//-------------------------------------------------
// ��ʂ̕`��
void drawScr(void)
{
	itoGsFinish();
	itoVSync();
	//if (ffmode * interlace * (framebuffers-1))
	if (ffmode)
		itoSetActiveFrameBuffer(itoGetActiveFrameBuffer()^1);
	else
		itoSwitchFrameBuffers();
	
}

//-------------------------------------------------
// �g�̕`��
void drawFrame(int x1, int y1, int x2, int y2, uint64 color)
{
	uint64 color0,color1,color2;	//�A���t�@�t��

	color0 = color&0x00FFFFFF;	//�����x�����O
	color1 = color0|0x80000000;	//�s����
	color2 = color0|(setting->flicker_alpha << 24);	//������
	//color2 = half(color, setting->color[COLOR_BACKGROUND], 0x80);

	//FLICKER CONTROL: ON
	if(flickerfilter==TRUE){
		//�A���t�@�u�����h�L��
		itoPrimAlphaBlending( TRUE );
		//��̉���
		itoLine(color2, x1+1, y1+1, 0, color2, x2, y1+1, 0);
		//���̉���
		itoLine(color2, x1, y2+1, 0, color2, x2+1, y2+1, 0);
		//�A���t�@�u�����h����
		itoPrimAlphaBlending(FALSE);
	}

	//��̉���
	itoLine(color1, x1, y1, 0, color, x2, y1, 0);
	//�E�̏c��
	itoLine(color1, x2, y1, 0, color, x2, y2, 0);	
	//���̉���
	itoLine(color1, x2, y2, 0, color, x1, y2, 0);
	//���̏c��
	itoLine(color1, x1, y2, 0, color, x1, y1, 0);
}

// �X�N���[���o�[�̕`��
void drawBar(int x1, int y1, int x2, int y2, uint64 color, int ofs, int len, int size)
{
	long long z0,z1,z2;
	itoSprite(setting->color[COLOR_BACKGROUND], x1, y1, x2+1, y2+2, 0);
	drawFrame(x1, y1, x2, y2, color);
	if (x2-x1 > y2-y1) {
		z0 = x2 - x1;
		z1 = z0 * ofs / size;
		z2 = z0 * (ofs + len) / size;
		if (z1 < 0) z1 = 0;
		if (z2 < 0) z2 = 0;
		if (z1 > z0) z1 = z0;
		if (z2 > z0) z2 = z0;
		if (z1 == z2) z2++;
		itoSprite(color, z1+x1, y1, z2+x1, y2, 0);
	} else {
		z0 = y2 - y1;
		z1 = z0 * ofs / size;
		z2 = z0 * (ofs + len) / size;
		if (z1 < 0) z1 = 0;
		if (z2 < 0) z2 = 0;
		if (z1 > z0) z1 = z0;
		if (z2 > z0) z2 = z0;
		if (z1 == z2) z2++;
		itoSprite(color, x1, z1+y1, x2, z2+y1, 0);
	}
}

//-------------------------------------------------
//MAX_ROWS�Ȃǂ̐ݒ�
void SetHeight(void)
{
	//FONT_WIDTH
	if (font_half > 0) 
		FONT_WIDTH = (ascii_data.width+font_half) / (font_half+1) + char_Margin;
	else if (font_half == 0)
		FONT_WIDTH = ascii_data.width + char_Margin;
	else
		FONT_WIDTH = ascii_data.width * (-font_half+1) + char_Margin;

	//FONT_HEIGHT
	if(ascii_data.height>=kanji_data.height)
		FONT_HEIGHT = ascii_data.height;// + line_Margin;
	else
		FONT_HEIGHT = kanji_data.height;// + line_Margin;
	if (ffmode == ITO_FRAME) {
		if (font_vhalf >= 0)
			FONT_HEIGHT = (FONT_HEIGHT+1)/2;
		else
			FONT_HEIGHT /= 2;
	}
	if (font_vhalf > 0)
		FONT_HEIGHT = (FONT_HEIGHT+font_vhalf) / (font_vhalf+1);
	else if (font_vhalf < 0)
		FONT_HEIGHT*= -font_vhalf+1;
	FONT_HEIGHT+= line_Margin;
	
	if (FONT_HEIGHT < 1) FONT_HEIGHT = 1;
	//if (FONT_WIDTH < 1) FONT_WIDTH = 1;
	
	//MAX_ROWS
	MAX_ROWS = SCREEN_HEIGHT/FONT_HEIGHT-6;
	MAX_ROWS_X = SCREEN_WIDTH/FONT_WIDTH-11;

	//SCREEN_MARGIN
	SCREEN_MARGIN = (SCREEN_HEIGHT - ((MAX_ROWS+5) * FONT_HEIGHT))/2;
}

//------------------------------------------------------------
// �L���b�V���p�̍œK��
int mkfontcache(int c, void *dist, int ofs, int limit) 
{
	unsigned int x, y, z, w, h, wb, hf, i, pa, pb, pc, pd, pe, pf, pg, ph, pi;
//	unsigned int avx, avy, avxp, avyp, avl, ahx, ahy, ahxp, ahyp, ahl;
	unsigned char av, ac, ax, ay, axp, ayp, axl, ayl, *s, *d, ovr;
	uint64 msks, msk, cc;
	//if (c > 255) return ofs;
	if ((c == 0) || ((font[c].status & 1) == 0) || font[c].cache) return ofs;
	
	w = font[c].width;
	h = font[c].height;
	wb = font[c].width + font_bold;
	hf = font[c].height + flickerfilter;
	// �t�H���g�p�^�[���̓W�J
	if (c < 256)
		s = font_ascii + font[c].offset;
	else
		s = font_kanji + font[c].offset;
	d = bmpsrc;
	msks = msk = (uint64)1 << (((w+7)&-8)-1);
	memset(d, 0, wb*hf);
	for (y=0; y<h; y++) {
		for (x=cc=0; x<((w+7)&-8); x+=8) {
			cc <<= 8;
			cc |= (uint64)*s++;
		}
		msk = msks;
		for (x=0; x<w; x++,msk>>=1) {
			if (cc & msk) {
				d[y*wb+x] = 0x81;
				if (font_bold) d[y*wb+x+1] = 0x81;
				if (flickerfilter) {
					d[y*wb+x+wb] = 0x82;
					if (font_bold) d[y*wb+x+wb+1] = 0x82;
				}
			}
		}
	}

	// �L���b�V���ݒ�
	s = (unsigned char *)bmpsrc;
	font[c].cache = (cachestruct*)(dist + ofs);
	font[c].caches = 0;
	// �œK��
	for (i=y=pa=pb=pc=pd=pe=pf=pg=ph=pi=ovr=0; y<hf; y++) {
		if (ofs + i * sizeof(cachestruct) >= limit) break;
		for (x=0; x<wb; x++) {
			if (ofs + i * sizeof(cachestruct) >= limit) break;
			if ((av=s[y*wb+x]) & 0x80) {
				av &= 3;
				// ���������̒����̒���
				for (ax=axp=axl=z=0; z<wb-x; z++) {
					if ((s[y*wb+x+z] & 3) == av) {
						ax++;
						if (s[y*wb+x+z] & 0x80) {
							axp++;
							axl = ax;
						}
					} else break;
				}
				// ���������̒����̒���
				for (ay=ayp=ayl=z=0; z<hf-y; z++) {
					if ((s[(y+z)*wb+x] & 3) == av) {
						ay++;
						if (s[(y+z)*wb+x] & 0x80) {
							ayp++;
							ayl = ay;
						}
					} else break;
				}
				// �����̒����`�̑傫��
				// �c���̒����`�̑傫��
				if (av == 1) av = 0x80; else av = setting->flicker_alpha;
				if (axp < ayp) {
					for (z=0; z<ay; z++) s[(y+z)*wb+x] &= 3;
					ax = 0; ac = ay; ay = ayl;
				} else if (axp > 1) {
					for (z=0; z<ax; z++) s[y*wb+x+z] &= 3;
					ay = 0; ac = ax; ax = axl;
				} else {
					ac = 1;
				}
				if (ac > 1) {
					// line
					font[c].cache[i].type = 2;
					font[c].cache[i].alpha = av;
					font[c].cache[i].left = x;
					font[c].cache[i].top = y;
					font[c].cache[i].right = x+ax;
					font[c].cache[i].bottom = y+ay;
					pi = 1;
				} else {
					// point
					if (pa && (av & 0x80)) {
						font[c].cache[pb].type = 3;
						font[c].cache[pb].right = x;
						font[c].cache[pb].bottom = y;
						pa = 0;
						pi = 0;
					} else if (pc && (av < 0x80)) {
						font[c].cache[pd].type = 3;
						font[c].cache[pd].right = x;
						font[c].cache[pd].bottom = y;
						pc = 0;
						pi = 0;
					} else {
						font[c].cache[i].type = 1;
						font[c].cache[i].alpha = av;
						font[c].cache[i].left = x;
						font[c].cache[i].top = y;
						font[c].cache[i].right = 0;
						font[c].cache[i].bottom = 0;
						if (av & 0x80) {
							pa = 1; pb = i;
						} else {
							pc = 1; pd = i;
						}
						pi = 1;
					}
				}
				if (pi) {
					font[c].cache[i].dummy[0] = c >> 8;
					font[c].cache[i].dummy[1] = c & 255;
					font[c].caches++;
					if (ofs + (i+1) * sizeof(cachestruct) >= limit) {
						ovr = 1;
						break;
					}
					//else font[c].cache[++i].alpha = 0;
					i++;
				}
			}
		}
	}
	if (ovr) {
		i = 0;
		font[c].cache = NULL;
		font[c].caches = 0;
	}
	//if (ofs + i * sizeof(cachestruct) < limit) i++;
	return ofs + i * sizeof(cachestruct);
}

int mkfontcaches(int start, int chars, void *dist, int ofs, int limit)
{
	int i,k,m;
	// �t�H���g�L���b�V���쐬
	for(i=start,k=m=ofs; i<start+chars; i++) {
		if (font[i].status & 1) {
			m = mkfontcache(i, dist, k, limit);
		//	if (i < 256)
		//		printf("mkfontcache: 0x%2X [%c] = %04X-%04X (%4d bytes)\n", i, i, k, m, m-k);
		//	else if (k != m)
		//		printf("mkfontcache: %02d-%02d = %04X-%04X (%4d bytes)\n", ((i-256)/94)+1, ((i-256)%94)+1, k, m, m-k);
			k = m;
			if (k+8 >= limit) break;
		}
	}
	
	return m;
}
#define	cjk(ku, ten)	((ku-1)*94+(ten-1))
int mkfontcacheset(void)
{
	if (!setting->fontcache)  fontchange = 0;
	else if (fonthalfmode == 0) fontchange = 1; 
	else fontchange = 0;
	return fontchange;
}

int mkfontcachereset(void)
{
	int i;
	if (!font) return 0;
	mkfontcacheclear();
	fontchange = 0;
	if (fonthalfmode) return 0;
	if (!setting->fontcache) {
		printf("mkfontcache: fontcache disabled\n");
		return 0;
	}
	i = mkfontcaches(33, 94, bmpdst, 0, sizeof(bmpdst));	// ASCII
	i = mkfontcache(256 + cjk( 1,91), bmpdst, i, sizeof(bmpdst));	// ��
	i = mkfontcache(256 + cjk( 1,63), bmpdst, i, sizeof(bmpdst));	// �~
	i = mkfontcache(256 + cjk( 2, 4), bmpdst, i, sizeof(bmpdst));	// ��
	i = mkfontcache(256 + cjk( 2, 2), bmpdst, i, sizeof(bmpdst));	// ��
	i = mkfontcache(256 + cjk( 1,28), bmpdst, i, sizeof(bmpdst));	// �[
	i = mkfontcaches(256 + cjk( 2,10),   4, bmpdst, i, sizeof(bmpdst));	// ���
	i = mkfontcaches(256 + cjk( 8, 1),  32, bmpdst, i, sizeof(bmpdst));	// �r��
	i = mkfontcaches(256 + cjk( 5, 1),  86, bmpdst, i, sizeof(bmpdst));	// �J�^�J�i
	i = mkfontcaches(256 + cjk( 4, 1),  86, bmpdst, i, sizeof(bmpdst));	// �Ђ炪��
//	i =	mkfontcaches(128, 128, bmpdst, i, sizeof(bmpdst));	// ANK/SBCS
	i = mkfontcaches(256 + cjk( 1, 2), 281, bmpdst, i, sizeof(bmpdst));	// �L����+�S�p����+�S�p�p��
	printf("mkfontcache: used %d bytes, remain %d bytes\n", i, sizeof(bmpdst)-i-8);
	/*//
	int fd;
	fd=fioOpen("host:mkfcache.bin",O_WRONLY | O_TRUNC | O_CREAT);
	if (fd>=0) {
		fioWrite(fd, bmpdst, sizeof(bmpdst));
		fioClose(fd);
	}
	//*/
	return i;
}
#undef	cjk
void mkfontcacheclear(void)
{
	int i;
	memset(&bmpdst, 0, sizeof(bmpdst));
	if (font) {
		for (i=0; i<11536; i++) {
			font[i].cache = NULL;
			font[i].caches = 0;
		}
	}
}

//------------------------------------------------------------
// �C�ӂ̃T�C�Y�̃t�H���g�O���t��Ԃ�(GIF�p)

//------------------------------------------------------------
// ���[�U�[�p�^�[���̕`��(GIF�p?)


//------------------------------------------------------------
//�A�X�L�[�t�H���g�����[�h
int InitFontAscii(const char *path)
{
	int fd=0, dsize, i, k;
	size_t size;
	FONTX_HEADER *fontx_header_ascii;
	CPE_FONT_HEADER *font_header;
	char fullpath[MAX_PATH];
	char *tekbuff=NULL;

	if(init_ascii==1) FreeFontAscii();
	//fnthinited=FALSE;
	if (font == NULL) font = (fontcache*)malloc(11536*sizeof(fontcache));

	if(strcmp(path, "rom0:KROM")==0 || strcmp(path, "systemfont")==0){
		//BIOSFont
		//�t�H���g�t�@�C���I�[�v��
		fd = fioOpen("rom0:KROM", O_RDONLY);
		if(fd<0) return -1;
	
		//���������m�� ���zFONTX2 KROM
		size = 17 + 16*256;	//�w�b�_�T�C�Y + 1�����̃T�C�Y*256����
		font_ascii = (char*)malloc(size);
		memset(font_ascii, 0, size);
		if(font_ascii==NULL){
			fioClose(fd);
			return -1;
		}

		//�������ɓǂݍ���
		fioLseek(fd, 0x198DE, SEEK_SET);
		//fioRead(fd, font_ascii + 17 + 15*33, 15*95);//�w�b�_�T�C�Y + 1�����̃T�C�Y*33����, 1�����̃T�C�Y*95����
		int i,p;
		for (i=0; i<95; i++) {
			p = 17 + 16*33 + i*16;
			fioRead(fd, font_ascii + p, 15);
			//font_ascii[p+15] = font_ascii[p+13] & font_ascii[p+14];
		}

		//�N���[�Y
		fioClose(fd);

		//�w�b�_�̃|�C���^
		fontx_header_ascii = (FONTX_HEADER*)font_ascii;

		//�w�b�_�쐬
		strncpy(fontx_header_ascii->Identifier, "FONTX2", 6);
		strncpy(fontx_header_ascii->FontName, "KROM", 8);
		fontx_header_ascii->XSize = 8;
		fontx_header_ascii->YSize =16;
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
			if(checkFONTX2header(fullpath)<0 && checkMSWinheader(fullpath)<0){
				fullpath[2]='1';
				if(checkFONTX2header(fullpath)<0 && checkMSWinheader(fullpath)<0)
					fullpath[0]=0;
			}
		}
		else
			strcpy(fullpath, path);

		//�t�H���g�t�@�C���I�[�v��
		fd = fioOpen(fullpath, O_RDONLY);
		if(fd<0) return -1;
	
		//���k����p
		fioRead(fd, fullpath, 32);

		//�T�C�Y�𒲂ׂ�
		size = fioLseek(fd, 0, SEEK_END);
		fioLseek(fd, 0, SEEK_SET);	//�V�[�N��0�ɖ߂�

		if ((dsize = tek_getsize(fullpath))>=0) {
			//tek�W�J
			//���������m��
			tekbuff = (char*)malloc(size);
			if(tekbuff==NULL){
				fioClose(fd);
				return -1;
			}
			//���f�[�^��ǂݍ���
			fioRead(fd, tekbuff, (size_t)size);
			
			//�N���[�Y
			fioClose(fd);
			
			//�W�J��o�b�t�@���m��
			font_ascii = (char*)malloc(dsize);
			if(font_ascii==NULL){
				free(tekbuff);
				return -1;
			}
			//tek�W�J
			if(tek_decomp(tekbuff, font_ascii, size)<0){
				free(tekbuff);
				free(font_ascii);
				return -1;
			}
			free(tekbuff);
		} else {
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
	}

	//�w�b�_�̃|�C���^
	fontx_header_ascii = (FONTX_HEADER*)font_ascii;
	font_header = (CPE_FONT_HEADER*)font_ascii;

	//�w�b�_�`�F�b�N
	if(strncmp(fontx_header_ascii->Identifier, "FONTX2", 6) || fontx_header_ascii->CodeType) {
		if ((font_header->dfVersion != 0x0200) ||
			(font_header->dfType != 0) ||
			(font_header->dfReserved != 0) ||
			((font_header->dfItalic |
			  font_header->dfUnderline |
			  font_header->dfStrikeOut) & 0xFE) )
			return -1;
		// �t�H���g�̏��
		strncpy(ascii_data.font_name, font_ascii+font_header->dfFace, 8);
		ascii_data.font_name[8] = 0;
		// 1�����̃T�C�Y
		ascii_data.width = font_header->dfPixWidth;
		ascii_data.height = font_header->dfPixHeight;
		if (ascii_data.width == 0) ascii_data.width = font_header->dfMaxWidth;
		// 1�����̃T�C�Y�Z�o
		ascii_data.size = ((ascii_data.width+7)/8) * ascii_data.height;
		//
		ascii_data.Tnum = 0;
		//
		ascii_data.offset = font_header->dfBitsOffset;

		// �t�H���g�L���b�V���p
		if (font) {
			int m,x,y;
			char temp[2048];
			memset(font, 0, 256*sizeof(fontcache)); // ���p�����̏�����
			for(i=font_header->dfFirstChar; i<=font_header->dfLastChar; i++) {
				// �t�H���g�`��p�̃f�[�^����������
				font[i].offset = font_header->dfCharTable[i-font_header->dfFirstChar].charOffset;
				font[i].width = font_header->dfCharTable[i-font_header->dfFirstChar].charWidth; /* �ϕ��t�H���g�ɑΉ��� */
				font[i].height = ascii_data.height;
				font[i].status = 0;
				font[i].cache = NULL;
				for (k=0; k<((font[i].width+7)/8)*ascii_data.height; k++) {
					// �t�H���g�O���t���L���ł���ꍇ�̓t���O�𗧂Ă�
					if (font_ascii[font[i].offset+k]) {
						font[i].status |= 1;
						break;
					}
				}
				if (font[i].width>8 && font[i].status&1) {
					// ����9�h�b�g�ȏ�̏ꍇ�̓o�C�g�����̕��ёւ����K�v
					m = (font[i].width+7)/8;
					for(y=0; y<font[i].height; y++)
						for(x=0; x<m; x++)
							temp[y*m+x] = font_ascii[font[i].offset+x*ascii_data.height+y];
					memcpy(&font_ascii[font[i].offset], temp, m*ascii_data.height);
				}
			}
		} else {
			// ���s�������Ƃɂ���
			init_ascii=1;
			FreeFontAscii();
			return -1;
		}
	} else {
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

		// �t�H���g�L���b�V���p
		if (font) {
			memset(font, 0, 256*sizeof(fontcache)); // ���p�����̏�����
			for(i=0; i<256; i++) {
				// �t�H���g�`��p�̃|�C���^�Ȃǂ����炩���ߏ������Ă���
				font[i].offset = ascii_data.offset + i*ascii_data.size;
				font[i].width = ascii_data.width;
				font[i].height = ascii_data.height;
				font[i].status = 0;
				font[i].cache = NULL;
				for (k=0; k<ascii_data.size; k++) {
					if (font_ascii[font[i].offset+k]) {
						font[i].status |= 1;
						break;
					}
				}
			}
		}
	}

	// �t�H���g�L���b�V���쐬
	mkfontcacheset();

	SetHeight();

	//�t�H���g���[�h����
	init_ascii=1;
	
	return 0;
}

//------------------------------------------------------------
//�����t�H���g�����[�h
int InitFontKnaji(const char *path)
{
	int fd=0, dsize;
	size_t size;
	FONTX_HEADER *fontx_header_kanji;
	char fullpath[MAX_PATH];
	char *tekbuff=NULL;

	if(init_kanji==1) FreeFontKanji();

	if(strcmp(path,"rom0:KROM")==0 || strcmp(path,"systemfont")==0){
		//BIOSFont
		//�t�H���g�t�@�C���I�[�v��
		fd = fioOpen("rom0:KROM", O_RDONLY);
		if(fd<0) return -1;

		//���������m�� ���zFONTX2 KROM
		size = 18 + 51*4 + 32*3489;	//�w�b�_�T�C�Y + �e�[�u���̐�*4 + 1�����̃T�C�Y*3489����
		font_kanji = (char*)malloc(size);
		memset(font_kanji, 0, size);
		if(font_kanji==NULL){
			fioClose(fd);
			return -1;
		}

		//�������ɓǂݍ���
		//fioRead(fd, font_kanji + 18 + 51*4, 30*3489);//�w�b�_�T�C�Y + �e�[�u���̐�*4 ,1�����̃T�C�Y*3489����
		int i,p;
		for (i=0; i<3489; i++) {
			p = 18 + 51*4 + i*32;
			fioRead(fd, font_kanji + p, 30);
			//font_kanji[p+30] = font_kanji[p+26] & font_kanji[p+28];
			//font_kanji[p+31] = font_kanji[p+27] & font_kanji[p+29];
		}

		//�N���[�Y
		fioClose(fd);

		//�w�b�_�̃|�C���^
		fontx_header_kanji = (FONTX_HEADER*)font_kanji;

		//�w�b�_�쐬
		strncpy(fontx_header_kanji->Identifier, "FONTX2", 6);
		strncpy(fontx_header_kanji->FontName, "KROM_k", 8);
		fontx_header_kanji->XSize = 16;
		fontx_header_kanji->YSize = 16;
		fontx_header_kanji->CodeType = 1;
		fontx_header_kanji->Tnum = 51;
		//�e�[�u��
		memcpy(font_kanji+18,krom_sjis_table,51*4);

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

		//���k����p
		fioRead(fd, fullpath, 32);

		//�T�C�Y�𒲂ׂ�
		size = fioLseek(fd, 0, SEEK_END);
		fioLseek(fd, 0, SEEK_SET);	//�V�[�N��0�ɖ߂�

		if ((dsize = tek_getsize(fullpath))>=0) {
			//tek�W�J
			//���������m��
			tekbuff = (char*)malloc(size);
			if(tekbuff==NULL){
				fioClose(fd);
				return -1;
			}
			//���f�[�^��ǂݍ���
			fioRead(fd, tekbuff, (size_t)size);
			
			//�N���[�Y
			fioClose(fd);
			
			//�W�J��o�b�t�@���m��
			font_kanji = (char*)malloc(dsize);
			if(font_kanji==NULL){
				free(tekbuff);
				return -1;
			}
			//tek�W�J
			if(tek_decomp(tekbuff, font_kanji, size)<0){
				free(tekbuff);
				free(font_kanji);
				return -1;
			}
			free(tekbuff);
		} else {
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
	kanji_data.size = ((kanji_data.width+7)/8) * kanji_data.height;
	//
	kanji_data.Tnum = fontx_header_kanji->Tnum;
	//
	kanji_data.offset = 18 + kanji_data.Tnum*4;

	SetHeight();

	//�t�H���g���[�h����
	init_kanji=1;
	
	//�t�H���g�L���b�V���p
	if (font == NULL) font = (fontcache*)malloc(11536*sizeof(fontcache));
	if (font) {
		//printf("fontcache: %08X, size:%6d (%3d x11536)\n", (int)font, 11536*sizeof(fontcache), sizeof(fontcache));
		unsigned char l,h;
		unsigned short t,c,i,f,s,n;
		unsigned int a;
		//for(c = 256; c < 11536; c++)
		//	fontoffset[c] = 0;
		memset(&font[256], 0, 11280*sizeof(fontcache)); // �S�p�t�H���g�����̏�����
		/*
				c = i*94+k;
				1 = 0x81 + (i >> 1) + 0x40 * (i > 61);
				2 = 0x40 + k + 94 * (i & 1) + ((i & 1) || (k >= 63));
				w = (f << 8) + s;
		*/
		for(t = 0, c = 0; t < kanji_data.Tnum; t++) {
			f = fontx_header_kanji->Block[t].Start;
			s = fontx_header_kanji->Block[t].End;
			for(i = f; i <= s; i++) {
				h = i >> 8; l = i & 255;
				if ((((h > 0x80) && (h < 0xA0)) || ((h >= 0xE0) && (h < 0xFD))) &&
					(l >= 0x40) && (l != 0x7F) && (l < 0xFD)) { /* SJIS��p */
					h = h - 0x81 - 0x40*(h > 0xA0);
					l = l - 0x40 - (l>0x7F);
					n = h * 188 + l + 256;
					// �t�H���g�����`��p�Ƀ|�C���^�Ȃǂ̏����������Ă���
					font[n].offset = kanji_data.offset + kanji_data.size * c;
					font[n].width = kanji_data.width;
					font[n].height = kanji_data.height;
					//printf("fontcache_full: font[%d].offset=%08X, .width=%3d, .height=%3d\n", n, font[n].offset, font[n].width, font[n].height);
					//printf("fontcahce_full: c=%5d, font_kanji=%08X, size=%d\n", c, (int)font_kanji, kanji_data.size);
					for(a = 0; a < kanji_data.size; a++) {
						if (font_kanji[font[n].offset+a]) {
							// �t�H���g�̃O���t�͗L��
							font[n].status |= 1;
							break;
						}
					}
				}
				c++;
			}
		}
	}
	
	// �t�H���g�L���b�V���쐬
	mkfontcacheset();
	
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
			if (font_half > 0) 
				return (ascii_data.width+font_half) / (font_half+1);
			else if (font_half == 0)
				return ascii_data.width;
			else
				return ascii_data.width * (-font_half+1);
		case ASCII_FONT_HEIGHT:
			if (font_vhalf > 0) 
				return (ascii_data.height+font_vhalf) / (font_vhalf+1) / fieldbuffers;
			else if (font_vhalf == 0)
				return ascii_data.height / fieldbuffers;
			else
				return ascii_data.height * (-font_vhalf+1) / fieldbuffers;
		case KANJI_FONT_WIDTH:
			if (font_half > 0) 
				return (kanji_data.width+font_half) / (font_half+1);
			else if (font_half == 0)
				return kanji_data.width;
			else
				return kanji_data.width * (-font_half+1);
		case KANJI_FONT_HEIGHT:
			if (font_vhalf > 0) 
				return (kanji_data.height+font_vhalf) / (font_vhalf+1) / fieldbuffers;
			else if (font_vhalf == 0)
				return kanji_data.height / fieldbuffers;
			else
				return kanji_data.height * (-font_vhalf+1) / fieldbuffers;
	}
	return 0;
}

//------------------------------------------------------------
//�t�H���g�{�[���h��ݒ�
void SetFontBold(int flag)
{
	if (font_bold != flag) {
		font_bold = flag;
		mkfontcacheset();
	}
	return;
}

//------------------------------------------------------------
//�t�H���g�{�[���h�̎擾
int GetFontBold(void)
{
	return font_bold;
}

//------------------------------------------------------------
//�����t�H���g�T�C�Y�␳��ݒ�
void SetFontHalf(int flag)
{
	font_half = flag;
	//mkfontcacheset();
	if (flag > 0)
		FONT_WIDTH = (ascii_data.width+flag) / (flag+1) + char_Margin;
	else if (flag == 0)
		FONT_WIDTH = ascii_data.width + char_Margin;
	else
		FONT_WIDTH = ascii_data.width * (-flag+1) + char_Margin;
	return;
}

//------------------------------------------------------------
//�����t�H���g�T�C�Y�␳�l�̎擾
int GetFontHalf(void)
{
	return font_half;
}

//------------------------------------------------------------
//�����t�H���g�T�C�Y�␳��ݒ�
void SetFontVHalf(int flag)
{
	font_vhalf = flag;
	//mkfontcacheset();
	if (flag > 0)
		FONT_HEIGHT = (ascii_data.height+flag) / (flag+1) + line_Margin;
	else if (flag == 0)
		FONT_HEIGHT = ascii_data.height + line_Margin;
	else
		FONT_HEIGHT = ascii_data.height * (-flag+1) + line_Margin;
	return;
}

//------------------------------------------------------------
//�����t�H���g�T�C�Y�␳�l�̎擾
int GetFontVHalf(void)
{
	return font_vhalf;
}

//------------------------------------------------------------
//MS Win �t�H���g�t�H�[�}�b�g�̃w�b�_�`�F�b�N
int checkMSWinheader(const char *path)
{
	int fd, size=0;
	char fullpath[MAX_PATH], tmp[MAX_PATH], *p;
	char buf[126];
	CPE_FONT_HEADER *font_header;

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
		//buf = (char*)malloc(32);
		fileXioRead(fd, buf, 126);
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
		//buf = (char*)malloc(32);
		fioRead(fd, buf, 126);
		fioClose(fd);
	}
	else {
		return 0;
	}

	//�w�b�_�̃|�C���^
	font_header = (CPE_FONT_HEADER*)buf;

	//�w�b�_�`�F�b�N
	if ((font_header->dfVersion != 0x0200) ||
		(font_header->dfType != 0) ||
		(font_header->dfReserved != 0) ||
		(font_header->dfItalic +
		 font_header->dfUnderline +
		 font_header->dfStrikeOut) ) {
		if(tek_getsize(buf)<0){
			//free(buf);
			goto error;
		}
	}

	//free(buf);
	return 1;
error:
	return -1;
}

//------------------------------------------------------------
//FONTX2�t�@�C���̃w�b�_�`�F�b�N
int checkFONTX2header(const char *path)
{
	//char *buf;
	int fd, size=0;
	char fullpath[MAX_PATH], tmp[MAX_PATH], *p;
	char buf[32];
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
		//buf = (char*)malloc(32);
		fileXioRead(fd, buf, 32);
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
		//buf = (char*)malloc(32);
		fioRead(fd, buf, 32);
		fioClose(fd);
	}
	else {
		return 0;
	}

	//�w�b�_�̃|�C���^
	fontx_header = (FONTX_HEADER*)buf;

	//�w�b�_�`�F�b�N
	if(strncmp(fontx_header->Identifier, "FONTX2", 6)!=0){
		if(tek_getsize(buf)<0){
			//free(buf);
			goto error;
		}
	}

	//free(buf);
	return 1;
error:
	return -1;
}

//-------------------------------------------------
// �����\��
void drawChar_JIS(unsigned int c, int x, int y, uint64 fcol, uint64 scol, unsigned char *k)
{
	unsigned char *pc;
	uint64 color = fcol;
	if (!font) return;
	if (c > 11535) return;
	if (fontchange) mkfontcachereset();
	if (c > 255) {
		if (!init_kanji) return;
		if (!font[c].offset) return;
		pc = font_kanji;
	} else {
		if (!init_ascii) return;
		if (c < 32) {
			// if (setting->disablectrl) return;
			if (c == 0) return;
			if (setting->disablectrl || (((font[c].status & 1)==0) && ((font[c+32].status & 1)!=0) && ((scol != fcol) || ((scol == fcol) && (scol == ((setting->color[COLOR_HIGHLIGHTTEXT]&0x00FFFFFF)|0x80000000)))))) {
				c = k[c];
				color = scol;
			}
		}
		pc = font_ascii;
	}
	if (font[c].status & 1) {
		pc += font[c].offset;
		if (fonthalfmode == 0) {
			if (font[c].cache)
				drawChar_cache(font[c].cache, font[c].caches, x, y, color);
			else
				drawChar_1bpp(pc, x, y, color, font[c].width, font[c].height);
		} else if (fonthalfmode == 1)
			drawChar_bilinear(pc, x, y, color, font[c].width, font[c].height);
		else if (fonthalfmode == 2)
			drawChar_filter(pc, x, y, color, font[c].width, font[c].height);
	}
}

//-------------------------------------------------
// 1bpp�r�b�g�}�b�v�t�H���g�̕`��(�������[�h�p)(��64�h�b�g�܂�)
void drawChar_1bpp(void *src, int x, int y, uint64 color, int w, int h)
{
	int	i, j;
	//unsigned char cc;
	unsigned char *pc, *cp;
	int btz;
	int bts, xl, xw, bty, xp, yp, n;
	uint64 msks, msk, cc, color2;
	unsigned int rpx, rpy;
	int xxl, xxr, yy;
	int wb=(w+7)>>3;	// 1�o�C�g�A���C��
	pc = (unsigned char*) src;
	
	color2 = half(setting->color[COLOR_BACKGROUND], color, setting->flicker_alpha<<1);
	//	msk:	�}�X�N�r�b�g
	//	bts:	�V�t�g�r�b�g��
	//	rpx:	�����{��
	// GetFontHalf() < 0: �g�� (-n �{)
	//				 = 0: �W�� (���{)
	//				 > 0: �k�� (1/(n+1)�{)
	msk = 0x8000000000000000;
	if (GetFontHalf() == 0) {
		// �W��
		bts = 1; rpx = 1;
	} else if (GetFontHalf() > 0) {
		// �k��
		bts = GetFontHalf() + 1; rpx = 1;
		for (i = 1; i < bts; i++) msk |= 0x8000000000000000 >> i;
	} else {
		bts = 1; rpx = -GetFontHalf() + 1;
	}
	msks = msk;
	if (GetFontVHalf() == 0) {
		// �W��
		rpy = 1;	// rpy: ���s�[�g���C����(1:�W��)
		bty = 1;	// bty: �X�L�b�v���C����(1:�W��)
	} else if (GetFontVHalf() > 0) {
		// �k��
		rpy = 1; bty = GetFontVHalf()+1;
	} else {
		rpy = -GetFontVHalf()+1; bty = 1;
	}
	btz = bty;
	if (ffmode == ITO_FRAME) {
		if (rpy == 1){
			bty*=2;
			if (itoGetActiveFrameBuffer())
				pc+= wb*btz;
		} else if (rpy > 1) {
			rpy /= 2;
		}
	}
	for(i=0,yp=0; i<h; i+=bty,yp++) {
		// i: �W����, yp: �g�厞�̐����ʒu
		// ����64�h�b�g�ȓ��Ȃ�1��œǂݍ��݂��s��
		if (btz>1) {
			cc = 0;
			if (w > 32) {
				for(j = 0; j < btz; j++){
					if (i+j >= h) break;
					cp = pc+j*wb;
					cc |= ((uint64) cp[0] << 56)|((uint64) cp[1] << 48)|((uint64) cp[2] << 40)|((uint64) cp[3] << 32)|
							((uint64) cp[4] << 24)|((uint64) cp[5] << 16)|((uint64) cp[6] << 8)|((uint64) cp[7]);
				}
			} else {
				for(j = 0; j < btz; j++){
					if (i+j >= h) break;
					cp = pc+j*wb;
					cc |= ((uint64) cp[0] << 56)|((uint64) cp[1] << 48)|((uint64) cp[2] << 40)|((uint64) cp[3] << 32);
				}
			}
		} else if (w > 32) {
			cc = ((uint64) pc[0] << 56)|((uint64) pc[1] << 48)|((uint64) pc[2] << 40)|((uint64) pc[3] << 32)|
					((uint64) pc[4] << 24)|((uint64) pc[5] << 16)|((uint64) pc[6] << 8)|((uint64) pc[7]);
		} else {
			cc = ((uint64) pc[0] << 56)|((uint64) pc[1] << 48)|((uint64) pc[2] << 40)|((uint64) pc[3] << 32);
		}
		// �`��J�n
		xl = -1; xw = 0;
		if (flickerfilter) {
			// �t���b�J�[�R���g���[�����L���̏ꍇ
			for(j=0,xp=0; j<w; j+=bts,xp++) {
				if (cc & msk) {
					if (xl < 0) xl = xp;
					xw++;
				} else if (xl >= 0) {
					if ((xw > 1) || font_bold || (rpx > 1)) {
						xxl = x+xl*rpx; xxr = x+(xl+xw)*rpx+font_bold;
						if (rpy > 1) {
							yy = y+i*rpy;
							for (n = 0; n < rpy; n++) {
								itoLine(color, xxl, yy+n, 0, color, xxr, yy+n, 0);
								itoLine(color2, xxl, yy+n+1, 0, color2, xxr, yy+n+1, 0);
							}
						} else {
							itoLine(color, xxl, y+yp, 0, color, xxr, y+yp, 0);
							itoLine(color2, xxl, y+yp+1, 0, color2, xxr, y+yp+1, 0);
						}
					} else {
						if (rpy > 1) {
							xxl = x+xl; yy = y+i*rpy;
							itoLine(color, xxl, yy, 0, color, xxl, yy+rpy, 0);
							itoPoint(color2, xxl, yy+rpy, 0);
						} else {
							itoPoint(color, x+xl, y+yp, 0);
							itoPoint(color2, x+xl, y+yp+1, 0);
						}
					}
					xl = -1; xw = 0;
				}
				msk = msk >> bts;
			}
			if (xw > 0) {
				if ((xw > 1) || font_bold || (rpx > 1)) {
					xxl = x+xl*rpx; xxr = x+(xl+xw)*rpx+font_bold;
					if (rpy > 1) {
						yy = y+i*rpy;
						for (n = 0; n < rpy; n++) {
							itoLine(color, xxl, yy+n, 0, color, xxr, yy+n, 0);
							itoLine(color2, xxl, yy+n+1, 0, color2, xxr, yy+n+1, 0);
						}
					} else {
						itoLine(color, xxl, y+yp, 0, color, xxr, y+yp, 0);
						itoLine(color2, xxl, y+yp+1, 0, color2, xxr, y+yp+1, 0);
					}
				} else {
					if (rpy > 1) {
						xxl = x+xl; yy = y+i*rpy;
						itoLine(color, xxl, yy, 0, color, xxl, yy+rpy, 0);
						itoPoint(color2, xxl, yy+rpy, 0);
					} else {
						itoPoint(color, x+xl, y+yp, 0);
						itoPoint(color2, x+xl, y+yp+1, 0);
					}
				}
			}
		} else {
			// �ʏ펞
			for(j=0,xp=0; j<w; j+=bts,xp++) {
				if (cc & msk) {
					if (xl < 0) xl = xp;
					xw++;
				} else if (xl >= 0) {
					if ((xw > 1) || font_bold || (rpx > 1)) {
						xxl = x+xl*rpx; xxr = x+(xl+xw)*rpx+font_bold;
						if (rpy > 1) {
							yy = y+i*rpy;
							for (n = 0; n < rpy; n++)
								itoLine(color, xxl, yy+n, 0, color, xxr, yy+n, 0);
						} else
							itoLine(color, xxl, y+yp, 0, color, xxr, y+yp, 0);
					} else {
						if (rpy > 1)
							itoLine(color, x+xl, y+i*rpy, 0, color, x+xl, y+i*rpy+rpy, 0);
						else
							itoPoint(color, x+xl, y+yp, 0);
					}
					xl = -1; xw = 0;
				}
				msk = msk >> bts;
			}
			if (xw > 0) {
				if ((xw > 1) || font_bold || (rpx > 1)) {
					xxl = x+xl*rpx; xxr = x+(xl+xw)*rpx+font_bold;
					if (rpy > 1) {
						yy = y+i*rpy;
						for (n = 0; n < rpy; n++)
							itoLine(color, xxl, yy+n, 0, color, xxr, yy+n, 0);
					} else
						itoLine(color, xxl, y+yp, 0, color, xxr, y+yp, 0);
				} else {
					if (rpy > 1)
						itoLine(color, x+xl, y+i*rpy, 0, color, x+xl, y+i*rpy+rpy, 0);
					else
						itoPoint(color, x+xl, y+yp, 0);
				}
			}
		}
		pc+= wb * bty;
		msk = msks;
	}
	return;
}

//-------------------------------------------------
void drawChar_filter(void *src, int x, int y, uint64 color, int w, int h)
{
	int dw, dh;
	int i,j;
	
	i = -GetFontHalf();
	j = -GetFontVHalf();
	if ((i == 0) && (j == 0) && (w <= 64)) {
		drawChar_1bpp(src, x, y, color, w, h);
		return;
	}
	if (i > 0)	dw = (i+1)*w;
	else if (i < 0)	dw = w/(-i+1);
	else		dw = w;
	if (j > 0)	dh = (j+1)*h;
	else if (j < 0)	dh = h/(-j+1);
	else		dh = h;
	
	if (dw*dh <= tmpbuffersize) {
	} else
		drawChar_1bpp(src, x, y, color, w, h);
}

void drawChar_bilinear(void *src, int x, int y, uint64 color, int w, int h)
{
	int dw, dh;
	int i,j;
	
	i = -GetFontHalf();
	j = -GetFontVHalf();
/*	if ((i == 0) && (j == 0) && (w <= 64) && (boot != PCSX_BOOT)) {
		drawChar_1bpp(src, x, y, color, w, h);
		return;
	}
*/	if (i > 0)	dw = (i+1)*w;
	else if (i < 0)	dw = w/(-i+1);
	else		dw = w;
	if (j > 0)	dh = (j+1)*h;
	else if (j < 0)	dh = h/(-j+1);
	else		dh = h;
	
	if (dw*dh <= tmpbuffersize) {
		drawChar_unpack(bmpsrc, src, w, h);
		if ((i != 0) || (j != 0)) {
			drawChar_resize(bmpdst, bmpsrc, dw, dh, w, h);
			if ((i < 0) || (j < 0))
				drawChar_normalize(bmpdst, dw, dh, 0x80);
			drawChar_8bpp(bmpdst, x, y, color, setting->color[COLOR_BACKGROUND], dw, dh);
		} else 
			drawChar_8bpp(bmpsrc, x, y, color, setting->color[COLOR_BACKGROUND], w, h);
	} else
		drawChar_1bpp(src, x, y, color, w, h);
}

//-------------------------------------------------
// 8bpp�r�b�g�}�b�v�̐��K��
void drawChar_normalize(void *src, int w, int h, int alpha)
{
	int i,j,k,maxalpha=0;
	double bai;
	unsigned char *pc;
	pc = src;
	// 1st pass
	for(i=0;i<h;i++)
		for(j=0;j<w;j++)
			if ((k=*pc++) > maxalpha) maxalpha = k;
	if ((maxalpha == 0) || (maxalpha >= alpha)) return;
	// 2nd pass
	bai = (double) alpha / maxalpha;
	//printf("bai: %.4f\n", bai);
	pc = src;
	for(i=0;i<h;i++)
		for(j=0;j<w;j++)
			*pc++ *= bai;
	return;
}

//-------------------------------------------------
// 8bpp�r�b�g�}�b�v�t�H���g�̕`��(���掿���[�h�p)
void drawChar_8bpp(void *src, int x, int y, uint64 color0, uint64 back, int w, int h)
{
	int i,j;
	unsigned char *pc,k;
	uint64 color=color0 & 0x00FFFFFF;
	pc = src;
	//�A���t�@�u�����h�L��
	itoPrimAlphaBlending( TRUE );
	// �������͌��
	if (ffmode * interlace * (framebuffers-1)) {
		if (itoGetActiveFrameBuffer()) pc+=w;
		for(i=0;i<h/2;i++,pc+=w)
			for(j=0;j<w;j++)
				if ((k=*pc++) > 0)
					itoPoint(color|((uint64)k<<24), x+j, y+i, 0);
				//	itoPoint(half(back, color, k<<1), x+j, y+i, 0);
	} else {
		for(i=0;i<h;i++)
			for(j=0;j<w;j++)
				if ((k=*pc++) > 0)
					itoPoint(color|((uint64)k<<24), x+j, y+i, 0);
				//	itoPoint(half(back, color, k<<1), x+j, y+i, 0);
	}
	//�A���t�@�u�����h����
	itoPrimAlphaBlending(FALSE);
}

unsigned char pget8bpp(unsigned char *src, int x, int y, int w, int h)
{
	if ((x >= 0) && (y >= 0) && (x < w) && (y < h))
		return src[y*w+x];
	return 0;
}
void pset8bpp(unsigned char *dst, int x, int y, unsigned char c, int w, int h)
{
	if ((x >= 0) && (y >= 0) && (x < w) && (y < h))
		dst[y*w+x] = c;
}
//-------------------------------------------------
// �o�C���j�A���T�C�Y
void drawChar_resize(void *dist, void *src, int dw, int dh, int sw, int sh)
{
	int xy, x, y, z, b, l, r;
	unsigned char *s, *d;
	unsigned char yx[8];
	s = src; d = dist;
	// 1st pass: ���������̃��T�C�Y(�����{�g��/�k������)
	if (sw > dw) {
		// �k��
		xy = sw / dw;
		for (y=0; y<sh; y++)
			for (x=0; x<dw; x++) {
				b = 0;
				for (z=0; z<xy; z++)
					b += pget8bpp(s, x*xy+z, y, sw, sh);
				pset8bpp(d, x, y, b/xy, dw, sh);
			}
	} else if (sw < dw) {
		// �g��
		xy = dw / sw;
		for (z=0; z<xy; z++)
			yx[z] = (z<<8)/xy;
		for (y=0; y<sh; y++)
			for (x=0; x<sw; x++) {
				l = pget8bpp(s, x, y, sw, sh);
				r = pget8bpp(s, x+1, y, sw, sh);
				for (z=0; z<xy; z++) {
					b = (l*(256-yx[z]) + r*yx[z])>>8;
					pset8bpp(d, x*xy+z, y, b, dw, sh);
				}
			}
	} else {
		// ���̂܂�
		for (y=0; y<sh; y++)
			for (x=0; x<sw; x++)
				pset8bpp(d, x, y, pget8bpp(s, x, y, sw, sh), dw, sh);
	}
	// 2nd pass: ���������̃��T�C�Y
	if (sh > dh) {
		// �k��
		xy = sh / dh;
		for (y=0; y<dh; y++)
			for (x=0; x<dw; x++) {
				b = 0;
				for (z=0; z<xy; z++)
					b += pget8bpp(d, x, y*xy+z, dw, sh);
				pset8bpp(d, x, y, b/xy, dw, dh);
			}
	} else if (sh < dh) {
		// �g��
		xy = dh / sh;
		for (z=0; z<xy; z++)
			yx[z] = (z<<8)/xy;
		for (y=sh-1; y>=0; y--)
			for (x=0; x<dw; x++) {
				l = pget8bpp(d, x, y, dw, sh);
				r = pget8bpp(d, x, y+1, dw, sh);
				for (z=0; z<xy; z++) {
					b = (l*(256-yx[z]) + r*yx[z])>>8;
					pset8bpp(d, x, y*xy+z, b, dw, dh);
				}
			}
	}
}

//-------------------------------------------------
// 1bpp�r�b�g�}�b�v�t�H���g�̃f�R�[�h(�t�H���g�̍��掿�`��p)
void drawChar_unpack(void *dist, void *src, int w, int h)
{
	int i,j;
	unsigned char msk, msks, *s, *d;
	msks = 0x80;
	for (j=0;j<h;j++){
		s = src + ((w+7)>>3)*j;
		d = dist + w*j;
		msk = msks;
		for (i=0;i<w;i++){
			if (!msk) {
				msk = msks;
				s++;
			}
			if (*s & msk)
				d[i] = 0x80;
			else if (font_bold && i>0 && d[i-1]==0x80)
				d[i] = 0x7F;
			else if (flickerfilter && j>0 && (*(d-w+i))>=0x7F)
				d[i] = setting->flicker_alpha - (setting->flicker_alpha>=0x7F)*2;
			else
				d[i] = 0x00;
			msk >>= 1;
		}
	}
		
	return;
}
//-------------------------------------------------
// �t�H���g�L���b�V���`��p
void drawChar_cache(cachestruct *src, int prims, int x, int y, uint64 color)
{
	int i;
	if (!prims) return;
	color &= 0x00FFFFFF;
	if (font_half || font_vhalf || ffmode) {
		// GetFontHalf() < 0: �g�� (-n �{)
		//				 = 0: �W�� (���{)
		//				 > 0: �k�� (1/(n+1)�{)
		int xp, yp;
		if (font_half < 0) xp = (-font_half +1) << 4;
		else if (font_half == 0) xp = 16;
		else xp = 16 / (font_half + 1);
		if (font_vhalf < 0) yp = (-font_vhalf +1) << 4;
		else if (font_vhalf == 0) yp = 16;
		else yp = 16 / (font_vhalf + 1);
		x <<= 4; y <<= 4;
		if (ffmode) {
			yp >>= 1;
			if (itoGetActiveFrameBuffer()) y -= 8;
		}
		itoPrimAlphaBlending( TRUE );
		for (i=0; i<prims; i++) {
			switch(src[i].type) {
				case 1:	// Point
					//itoPointX(color | ((uint64)src[i].alpha << 24), x+src[i].left*xp, y+src[i].top*yp);
					itoSpriteX(color | ((uint64)src[i].alpha << 24), x+src[i].left*xp, y+src[i].top*yp, x+src[i].left*xp+xp, y+src[i].top*yp+yp);
					break;
				case 2:	// Line
					//itoLineX(color | ((uint64)src[i].alpha << 24), x+src[i].left*xp, y+src[i].top*yp, x+src[i].right*xp, y+src[i].bottom*yp);
					if (src[i].left == src[i].right)
						itoSpriteX(color | ((uint64)src[i].alpha << 24), x+src[i].left*xp, y+src[i].top*yp, x+src[i].right*xp+xp, y+src[i].bottom*yp);
					else
						itoSpriteX(color | ((uint64)src[i].alpha << 24), x+src[i].left*xp, y+src[i].top*yp, x+src[i].right*xp, y+src[i].bottom*yp+yp);
					break;
				case 3:	// Point x2
					//itoPoint2X(color | ((uint64)src[i].alpha << 24), x+src[i].left*xp, y+src[i].top*yp, x+src[i].right*xp, y+src[i].bottom*yp);
					itoSpriteX(color | ((uint64)src[i].alpha << 24), x+src[i].left*xp, y+src[i].top*yp, x+src[i].left*xp+xp, y+src[i].top*yp+yp);
					itoSpriteX(color | ((uint64)src[i].alpha << 24), x+src[i].right*xp, y+src[i].bottom*yp, x+src[i].right*xp+xp, y+src[i].bottom*yp+yp);
					break;
			}
		}
		itoPrimAlphaBlending( FALSE );
	} else if (flickerfilter) {
		itoPrimAlphaBlending( TRUE );
		for (i=0; i<prims; i++) {
			switch(src[i].type) {
				case 1:	// Point
					itoPoint(color | ((uint64)src[i].alpha << 24), x+src[i].left, y+src[i].top, 0);
					break;
				case 2:	// Line
					itoLine(color | ((uint64)src[i].alpha << 24), x+src[i].left, y+src[i].top, 0,
							0, x+src[i].right, y+src[i].bottom, 0);
					break;
				case 3:	// Point x2
					itoPoint2(color | ((uint64)src[i].alpha << 24), x+src[i].left, y+src[i].top, x+src[i].right, y+src[i].bottom);
					break;
			}
		}
		itoPrimAlphaBlending( FALSE );
	} else {
		for (i=0; i<prims; i++) {
			switch(src[i].type) {
				case 1:	// Point
					itoPoint(color, x+src[i].left, y+src[i].top, 0);
					break;
				case 2:	// Line
					itoLine(color, x+src[i].left, y+src[i].top, 0,
							0, x+src[i].right, y+src[i].bottom, 0);
					break;
				case 3:	// Point x2
					itoPoint2(color, x+src[i].left, y+src[i].top, x+src[i].right, y+src[i].bottom);
					break;
			}
		}
	}
}

//-------------------------------------------------
// ���b�Z�[�W�\���p
int drawStringLimit(const unsigned char *s, int charset, int sx, int sy, uint64 fcol, uint64 scol, unsigned char *ctrlchars, int right)
{
	unsigned char *k;
	uint64 fclr = (fcol & 0x00FFFFFF) | 0x80000000;
	uint64 sclr = (scol & 0x00FFFFFF) | 0x80000000;
	int cs = charset;
	int i=0,x=sx,y=sy,width=right-sx;
	extern unsigned char ctrlchar[32];
	if (ctrlchars == NULL) k=&ctrlchar[0]; else k=ctrlchars;
	if (charset == TXT_AUTO) {
		if (setting->language == LANG_ENGLISH)
			cs = TXT_ASCII;
		else
			cs = TXT_SJIS;
	}
	if (cs == TXT_ASCII) {
		int xp;
		x=sx+ascii_MarginLeft;
		y=sy+ascii_MarginTop;
		if (font_half > 0)
			xp = (ascii_data.width+font_half) / (font_half+1) + char_Margin;
		else if (font_half == 0)
			xp = ascii_data.width + char_Margin;
		else
			xp = ascii_data.width * (-font_half+1) + char_Margin;
		for(i = 0; s[i]; i++) {
			drawChar_JIS(s[i], x, y, fclr, sclr, k);
			x += xp;
		}
	} else {
		uint16 code;
		unsigned char a,b;
		int xa,xk;

		if (font_half > 0) {
			xa = (ascii_data.width+font_half) / (font_half+1) + char_Margin;
			xk = (kanji_data.width+font_half) / (font_half+1) + char_Margin * 2;
		} else if (font_half == 0) {
			xa = ascii_data.width + char_Margin;
			xk = kanji_data.width + char_Margin * 2;
		} else {
			xa = ascii_data.width * (-font_half+1) + char_Margin;
			xk = kanji_data.width * (-font_half+1) + char_Margin * 2;
		}
		if (right && *s) {
			if ((width / strlen(s)) < xa) {
				xa = width / strlen(s);
				xk = xa * 2;
			}
		}
		while(s[i]){
			if ((( s[i]>=0x81 && s[i]<=0x9f ) || ( s[i]>=0xe0 && s[i]<=0xfc )) &&
				s[i+1]>=0x40 && s[i+1]!=0x7f && s[i+1]<=0xfc ) {	//SJIS
				a = s[i++]; b = s[i++];
				code = (a - 0x81 - 0x40*(a>0x9f)) *188 + (b - 0x40 - (b>0x7f)) +256;
				if((x >= 0) && (x < SCREEN_WIDTH))
						drawChar_JIS(code, x+kanji_MarginLeft, y+kanji_MarginTop, fclr, sclr, k);
				x+=xk;
			}
			else{
				if((x >= 0) && (x < SCREEN_WIDTH))
					drawChar_JIS(s[i], x+ascii_MarginLeft, y+ascii_MarginTop, fclr, sclr, k);
				i++;
				x+=xa;
			}
		}

	}
	return x;
}
int drawString(const unsigned char *s, int charset, int sx, int sy, uint64 fcol, uint64 scol, unsigned char *ctrlchars)
{	return drawStringLimit(s, charset, sx, sy, fcol, scol, ctrlchars, 0);	}

//-------------------------------------------------
// �]���݊��p
int printXY(const unsigned char *s, int x, int y, uint64 color, int draw)
{
	if (color != setting->color[COLOR_GRAYTEXT])
		return drawString(s, TXT_SJIS, x, y, color, setting->color[COLOR_HIGHLIGHTTEXT], 0);
	else
		return drawString(s, TXT_SJIS, x, y, color, color, 0);
}

#ifdef ENABLE_ICON
//-------------------------------------------------
void loadIcon(void)
{
	itoLoadIIF(icon_iif, 0, 256, 0, 0);

	return;
}

//-------------------------------------------------
int drawIcon(int x, int y, int w, int h, int id)
{
	//�A���t�@�u�����h�L��
	itoPrimAlphaBlending( TRUE );

	itoSetTexture(0, 256, ITO_RGBA32, ITO_TEXTURE_256, ITO_TEXTURE_32);
	itoTextureSprite(
		ITO_RGBA(0x80,0x80,0x80,0xFF),
		x, y,
		id*16, 0, 
		x+16*w/16, y+16*h/16,
		id*16+16, 16,
		0);

	//�A���t�@�u�����h����
	itoPrimAlphaBlending( FALSE );

	return 0;
}
#endif

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
	uint64 color0,color1,color2;	//�A���t�@�t��
	uint16 code;
	int i;
	int x,y;

	x=CurrentPos_x;
	y=CurrentPos_y;

	color0 = color&0x00FFFFFF;	//�����x�����O
	color1 = color0|0x80000000;	//�s����
	color2 = color0|(setting->flicker_alpha << 24);	//������
	//color2 = half(color0, setting->color[COLOR_BACKGROUND], 0x80);

	i=0;
	while(s[i]){
		if (( s[i]>=0x81 && s[i]<=0x9f ) || ( s[i]>=0xe0 && s[i]<=0xff )){	//SJIS
			code = s[i++];
			code = (code<<8) + s[i++];
			if(draw){
				if(flickerfilter){
					//�A���t�@�u�����h�L��
					itoPrimAlphaBlending( TRUE );
					drawChar_SJIS(code, x+kanji_MarginLeft, y+kanji_MarginTop+1, color2);
					//�A���t�@�u�����h����
					itoPrimAlphaBlending( FALSE );
				}
				drawChar_SJIS(code, x+kanji_MarginLeft, y+kanji_MarginTop, color1);
			}
			x += kanji_data.width + char_Margin * 2;
		}
		else{
			if(draw){
				if(flickerfilter){
					//�A���t�@�u�����h�L��
					itoPrimAlphaBlending( TRUE );
					drawChar(s[i], x+ascii_MarginLeft, y+ascii_MarginTop+1, color2);
					//�A���t�@�u�����h����
					itoPrimAlphaBlending( FALSE );
				}
				drawChar(s[i], x+ascii_MarginLeft, y+ascii_MarginTop, color1);
			}
			i++;
			x += ascii_data.width + char_Margin;
		}
	}

	CurrentPos_y += FONT_HEIGHT;

	return x;
}
*/

