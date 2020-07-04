#include "launchelf.h"

//PS2TIME uLaunchELF
typedef struct
{
	unsigned char unknown;
	unsigned char sec;	// date/time (second)
	unsigned char min;	// date/time (minute)
	unsigned char hour;	// date/time (hour)
	unsigned char day;	// date/time (day)
	unsigned char month;	// date/time (month)
	unsigned short year;	// date/time (year)
} PS2TIME __attribute__((aligned (2)));

typedef struct{
	PS2TIME createtime;
	PS2TIME modifytime;
	unsigned fileSizeByte;
	unsigned short attr;
	char title[16*4+1];
	char name[256];
	int type;
} FILEINFO;

// psu�t�@�C���w�b�_�\����
typedef struct { // 512 bytes
	unsigned short attr;
	unsigned short unknown1;
	unsigned int size;	//file size, 0 for directory
	unsigned char createtime[8];	//0x00:sec:min:hour:day:month:year
	unsigned int unknown2;
	unsigned int unknown3;
	unsigned char modifytime[8];	//0x00:sec:min:hour:day:month:year
	unsigned char unknown4[32];
	unsigned char name[32];
	unsigned char unknown5[416];
} PSU_HEADER;

//FILEINFO type
enum
{
	TYPE_DEVICE_MC,
	TYPE_DEVICE_HDD,
	TYPE_DEVICE_CD,
	TYPE_DEVICE_MASS,
	TYPE_MISC,
	TYPE_FILE,
	TYPE_ELF,
	TYPE_DIR,
	TYPE_PS2SAVE,
	TYPE_PS1SAVE,
	TYPE_PSU,
	TYPE_OTHER=15,
};

//menu
enum
{
	COPY,
	CUT,
	PASTE,
	DELETE,
	RENAME,
	NEWDIR,
	GETSIZE,
	EXPORT,
	IMPORT,
	NUM_MENU
};

// ASCII��SJIS�̕ϊ��p�z��
const unsigned char sjis_lookup_81[256] = {
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x00
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x10
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x20
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x30
   ' ', ',', '.', ',', '.',0xFF, ':', ';', '?', '!',0xFF,0xFF,'\'', '`',0xFF, '^',  // 0x40
  0xFF, '_',0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, '-', '-', '/',0xFF,  // 0x50
  0xFF,0xFF,0xFF,0xFF,0xFF,'\'','\'', '"', '"', '(', ')', '[', ']', '[', ']', '{',  // 0x60
   '}',0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, '+', '-',0xFF,'*', 0xFF,  // 0x70
   '/', '=',0xFF, '<', '>',0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,'\\',  // 0x80
   '$',0xFF,0xFF, '%', '#', '&', '*', '@',0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x90
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xA0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xB0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xC0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xD0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xE0
  0xFF,0xFF, '#',0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xF0
};
const unsigned char sjis_lookup_82[256] = {
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x00
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x10
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x20
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x30
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,'0',   // 0x40
  '1', '2', '3', '4', '5', '6', '7', '8', '9', 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x50
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',   // 0x60
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x70
  0xFF,'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',   // 0x80
  'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x90
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xA0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xB0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xC0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xD0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xE0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xF0
};

int cut;
int nclipFiles, nmarks, nparties;
int title;
char mountedParty[2][MAX_NAME];
char parties[MAX_PARTITIONS][MAX_NAME];
char clipPath[MAX_PATH], LastDir[MAX_NAME], marks[MAX_ENTRY];
FILEINFO clipFiles[MAX_ENTRY];
int fileMode = FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH;

#ifdef ENABLE_PSB
#define MAX_ARGC 3
int psb_argc;
char psb_argv[MAX_ARGC][MAX_PATH+2];
#endif

//�v���g�^�C�v�錾
void sjis2ascii(const unsigned char *in, unsigned char *out);

//-------------------------------------------------
//���b�Z�[�W�{�b�N�X
int MessageBox(const char *Text, const char *Caption, int type)
{
	char MessageText[2048];
	int i,n;
	char *p;
	int tw;
	int ret=0;
	char CaptionText[256];
	char ButtonText[256];
	int DialogType;
	int len;
	int dialog_x;		//�_�C�A���Ox�ʒu
	int dialog_y;		//�_�C�A���Oy�ʒu
	int dialog_width;	//�_�C�A���O��
	int dialog_height;	//�_�C�A���O����
	int sel;
	int x, y;
	char tmp[256];		//�\���p

	//
	sel=0;
	if(type&MB_DEFBUTTON1) sel=0;
	if(type&MB_DEFBUTTON2) sel=1;
	if(type&MB_DEFBUTTON3) sel=2;
	//���b�Z�[�W
	strncpy(MessageText, Text, 2048);
	//\n��؂��\0��؂�ɕϊ� n:���s�̐�
	for(i=0,n=1; MessageText[i]!=0; i++)
		if(MessageText[i]=='\n'){MessageText[i]='\0';n++;}
	//���b�Z�[�W�̈�Ԓ����s�̕������𒲂ׂ� tw:������
	p = MessageText;
	tw = 0;
	for(i=0;i<n;i++){
		len = strlen(p);
		if(len>tw) tw=len;
		p += len+1;
	}
	//�L���v�V����
	if(Caption==NULL)
		strcpy(CaptionText, "error");
	else{
		//\n�܂ł��L���v�V�����ɂ���
		strncpy(CaptionText, Caption, 256);
		p = strchr(CaptionText, '\n');
		if(p!=NULL) *p='\0';
	}
	//�_�C�A���O�̃{�^��
	DialogType = type&0xf;
	if(DialogType==MB_OK)
		sprintf(ButtonText, "%s", lang->gen_ok);
	else if(DialogType==MB_OKCANCEL)
		sprintf(ButtonText, " %-10s %-10s", lang->gen_ok, lang->gen_cancel);
	else if(DialogType==MB_YESNOCANCEL)
		sprintf(ButtonText, " %-10s %-10s %-10s", lang->gen_yes, lang->gen_no, lang->gen_cancel);
	else if(DialogType==MB_YESNO)
		sprintf(ButtonText, " %-10s %-10s", lang->gen_yes, lang->gen_no);
	else if(DialogType==MB_MC0MC1CANCEL)
		sprintf(ButtonText, " %-10s %-10s %-10s", "mc0:/", "mc1:/", lang->gen_cancel);
	else
		return 0;
	//�_�C�A���O�ɕ\������ő�̕�����
	if(tw<strlen(ButtonText)) tw=strlen(ButtonText);
	if(tw<strlen(CaptionText)) tw=strlen(CaptionText);
	
	dialog_width = FONT_WIDTH*(tw+2);
	dialog_height = FONT_HEIGHT*(n+5);
	dialog_x = (SCREEN_WIDTH-dialog_width)/2;
	dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad & PAD_LEFT){
				sel--;
				if(sel<0) sel=0; 
			}
			else if(new_pad & PAD_RIGHT){
				sel++;
				if(DialogType==MB_OKCANCEL||DialogType==MB_YESNO){
					if(sel>1) sel=1;
				}
				if(DialogType==MB_YESNOCANCEL||DialogType==MB_MC0MC1CANCEL){
					if(sel>2) sel=2;
				}
			}
			else if(new_pad & PAD_CROSS){	//�L�����Z��
				ret=0;
				break;
			}
			else if(new_pad & PAD_CIRCLE){
				if(DialogType==MB_OK)
					ret=IDOK;
				if(DialogType==MB_OKCANCEL){
					if(sel==0) ret=IDOK;
					if(sel==1) ret=IDCANCEL;
				}
				if(DialogType==MB_YESNOCANCEL){
					if(sel==0) ret=IDYES;
					if(sel==1) ret=IDNO;
					if(sel==2) ret=IDCANCEL;
				}
				if(DialogType==MB_YESNO){
					if(sel==0) ret=IDYES;
					if(sel==1) ret=IDNO;
				}
				if(DialogType==MB_MC0MC1CANCEL){
					if(sel==0) ret=IDMC0;
					if(sel==1) ret=IDMC1;
					if(sel==2) ret=IDCANCEL;
				}
				break;
			}
			else if(new_pad & PAD_SELECT){	//�L�����Z���ɃJ�[�\�����ړ�
				if(DialogType==MB_OKCANCEL||DialogType==MB_YESNO) sel=1;
				if(DialogType==MB_YESNOCANCEL||DialogType==MB_MC0MC1CANCEL) sel=2;
			}
			else if(new_pad & PAD_START){	//OK�ɃJ�[�\�����ړ�
				sel=0;
			}
			//���{�^���Ō���
			if(new_pad & PAD_TRIANGLE){
				if(type&MB_USETRIANGLE){
					if(DialogType==MB_OK)
						ret=IDOK|IDTRIANGLE;
					if(DialogType==MB_OKCANCEL){
						if(sel==0) ret=IDOK|IDTRIANGLE;
						if(sel==1) ret=IDCANCEL|IDTRIANGLE;
					}
					if(DialogType==MB_YESNOCANCEL){
						if(sel==0) ret=IDYES|IDTRIANGLE;
						if(sel==1) ret=IDNO|IDTRIANGLE;
						if(sel==2) ret=IDCANCEL|IDTRIANGLE;
					}
					if(DialogType==MB_YESNO){
						if(sel==0) ret=IDYES|IDTRIANGLE;
						if(sel==1) ret=IDNO|IDTRIANGLE;
					}
					if(DialogType==MB_MC0MC1CANCEL){
						if(sel==0) ret=IDMC0|IDTRIANGLE;
						if(sel==1) ret=IDMC1|IDTRIANGLE;
						if(sel==2) ret=IDCANCEL|IDTRIANGLE;
					}
					break;
				}
			}
			//���{�^���Ō���
			if(new_pad & PAD_SQUARE){
				if(type&MB_USESQUARE){
					if(DialogType==MB_OK)
						ret=IDOK|IDSQUARE;
					if(DialogType==MB_OKCANCEL){
						if(sel==0) ret=IDOK|IDSQUARE;
						if(sel==1) ret=IDCANCEL|IDSQUARE;
					}
					if(DialogType==MB_YESNOCANCEL){
						if(sel==0) ret=IDYES|IDSQUARE;
						if(sel==1) ret=IDNO|IDSQUARE;
						if(sel==2) ret=IDCANCEL|IDSQUARE;
					}
					if(DialogType==MB_YESNO){
						if(sel==0) ret=IDYES|IDSQUARE;
						if(sel==1) ret=IDNO|IDSQUARE;
					}
					if(DialogType==MB_MC0MC1CANCEL){
						if(sel==0) ret=IDMC0|IDSQUARE;
						if(sel==1) ret=IDMC1|IDSQUARE;
						if(sel==2) ret=IDCANCEL|IDSQUARE;
					}
					break;
				}
			}
		}

		// �`��J�n
		drawDialogTmp(dialog_x, dialog_y,
			dialog_x+dialog_width, dialog_y+dialog_height,
			setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
		itoLine(setting->color[COLOR_FRAME], dialog_x+FONT_WIDTH, dialog_y+FONT_WIDTH*3.5, 0,
			setting->color[COLOR_FRAME], dialog_x+dialog_width-FONT_WIDTH, dialog_y+FONT_WIDTH*3.5, 0);
		//�L���v�V����
		x = dialog_x+FONT_WIDTH*1;
		y = dialog_y+FONT_HEIGHT*0.5;
		printXY(CaptionText, x, y, setting->color[COLOR_TEXT], TRUE);
		//���b�Z�[�W
		x = dialog_x+FONT_WIDTH*1;
		y = dialog_y+FONT_HEIGHT*2.5;
		p = MessageText;
		for(i=0;i<n;i++){
			printXY(p, x, y, setting->color[COLOR_TEXT], TRUE);
			p += strlen(p)+1;
			y += FONT_HEIGHT;
		}
		y += FONT_HEIGHT;	//��s
		//�{�^��
		x = dialog_x+(dialog_width-(strlen(ButtonText)*FONT_WIDTH))/2;
		printXY(ButtonText, x, y, setting->color[COLOR_TEXT], TRUE);
		//�J�[�\��
		if(DialogType!=MB_OK){
			x = dialog_x+(dialog_width-(strlen(ButtonText)*FONT_WIDTH))/2 + (sel*FONT_WIDTH*11);
			printXY(">", x, y, setting->color[COLOR_TEXT], TRUE);
		}
		// �������
		x = FONT_WIDTH*1;
		y = SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT;
		itoSprite(setting->color[COLOR_BACKGROUND],
			0, y,
			SCREEN_WIDTH, y+FONT_HEIGHT, 0);
		sprintf(tmp,"��:%s �~:%s", lang->gen_ok, lang->gen_cancel);
		printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE);
		drawScr();
	}
	return ret;
}

//-------------------------------------------------
//�g���q���擾
char* getExtension(const char *path)
{
	return strrchr(path,'.');
}

//-------------------------------------------------
// HDD�̃p�[�e�B�V�����ƃp�X���擾
int getHddParty(const char *path, const FILEINFO *file, char *party, char *dir)
{
	char fullpath[MAX_PATH], *p;

	if(strncmp(path,"hdd",3)) return -1;

	//fullpath���쐬
	strcpy(fullpath, path);
	if(file!=NULL){
		strcat(fullpath, file->name);
		if(file->attr & MC_ATTR_SUBDIR)	//�t�H���_�̂Ƃ��X���b�V������
			strcat(fullpath,"/");
	}
	//�p�[�e�B�V���������Ȃ��Ƃ��̓G���[
	if((p=strchr(&fullpath[6], '/'))==NULL)
		return -1;
	//dir��pfs0:����n�܂�p�X���R�s�[ ��:pfs0:/BOOT.ELF
	if(dir!=NULL)
		sprintf(dir, "pfs0:%s", p);
	//�p�X���p�[�e�B�V�������܂łɂ���
	*p=0;
	//party��hdd0:�ƃp�[�e�B�V���������R�s�[ ��:hdd0:__boot
	if(party!=NULL)
		sprintf(party, "hdd0:%s", &fullpath[6]);

	return 0;
}

//-------------------------------------------------
// �p�[�e�B�V�����̃}�E���g
int mountParty(const char *party)
{
	//�}�E���g���Ă�����ԍ���Ԃ�
	if(!strcmp(party, mountedParty[0]))
		return 0;
	else if(!strcmp(party, mountedParty[1]))
		return 1;

	//�}�E���g���Ă��Ȃ��Ƃ�pfs0�Ƀ}�E���g
	fileXioUmount("pfs0:");
	mountedParty[0][0]=0;
	if(fileXioMount("pfs0:", party, FIO_MT_RDWR) < 0)
		return -1;
	strcpy(mountedParty[0], party);
	return 0;
}

//-------------------------------------------------
// �N�C�b�N�\�[�g
int cmpFile(FILEINFO *a, FILEINFO *b)
{
	unsigned char *p, ca, cb;
	int i, n, ret, aElf=FALSE, bElf=FALSE, t=title;

	if((a->attr & MC_ATTR_SUBDIR)==(b->attr & MC_ATTR_SUBDIR)){
		if(!(a->attr & MC_ATTR_SUBDIR)){
			p = strrchr(a->name, '.');
			if(p!=NULL && !stricmp(p+1, "ELF")) aElf=TRUE;
			p = strrchr(b->name, '.');
			if(p!=NULL && !stricmp(p+1, "ELF")) bElf=TRUE;
			if(aElf && !bElf)		return -1;
			else if(!aElf && bElf)	return 1;
			t=FALSE;
		}
		if(t){
			if(a->title[0]!=0 && b->title[0]==0) return -1;
			else if(a->title[0]==0 && b->title[0]!=0) return 1;
			else if(a->title[0]==0 && b->title[0]==0) t=FALSE;
		}
		if(t) n=strlen(a->title);
		else n=strlen(a->name);
		for(i=0; i<=n; i++){
			if(t){
				ca=a->title[i]; cb=b->title[i];
			}else{
				ca=a->name[i]; cb=b->name[i];
				if(ca>='a' && ca<='z') ca-=0x20;
				if(cb>='a' && cb<='z') cb-=0x20;
			}
			ret = ca-cb;
			if(ret!=0) return ret;
		}
		return 0;
	}

	if(a->attr & MC_ATTR_SUBDIR)	return -1;
	else						return 1;
}
void sort(FILEINFO *a, int left, int right)
{
	FILEINFO tmp, pivot;
	int i, p;

	if (left < right) {
		pivot = a[left];
		p = left;
		for (i=left+1; i<=right; i++) {
			if (cmpFile(&a[i],&pivot)<0){
				p=p+1;
				tmp=a[p];
				a[p]=a[i];
				a[i]=tmp;
			}
		}
		a[left] = a[p];
		a[p] = pivot;
		sort(a, left, p-1);
		sort(a, p+1, right);
	}
}

//-------------------------------------------------
// �������[�J�[�h�ǂݍ���
int readMC(const char *path, FILEINFO *info, int max)
{
	static mcTable mcDir[MAX_ENTRY] __attribute__((aligned(64)));
	char dir[MAX_PATH];
	int i, j, ret;

	mcSync(MC_WAIT, NULL, NULL);

	strcpy(dir, &path[4]); strcat(dir, "*");
	mcGetDir(path[2]-'0', 0, dir, 0, MAX_ENTRY-2, mcDir);
	mcSync(MC_WAIT, NULL, &ret);

	for(i=j=0; i<ret; i++)
	{
		if(mcDir[i].attrFile & MC_ATTR_SUBDIR && (!strcmp(mcDir[i].name, ".") || !strcmp(mcDir[i].name, "..")))
			continue;

		strcpy(info[j].name, mcDir[i].name);
		info[j].attr = mcDir[i].attrFile;
		info[j].fileSizeByte = mcDir[i].fileSizeByte;
		memcpy(&info[j].createtime, &mcDir[i]._create, 8);
		memcpy(&info[j].modifytime, &mcDir[i]._modify, 8);
		j++;
	}

	return j;
}

//-------------------------------------------------
// CD�ǂݍ���
int readCD(const char *path, FILEINFO *info, int max)
{
	static struct TocEntry TocEntryList[MAX_ENTRY];
	char dir[MAX_PATH];
	int i, j, n;

	loadCdModules();

	strcpy(dir, &path[5]);
	CDVD_FlushCache();
	n = CDVD_GetDir(dir, NULL, CDVD_GET_FILES_AND_DIRS, TocEntryList, MAX_ENTRY, dir);

	for(i=j=0; i<n; i++)
	{
		if(TocEntryList[i].fileProperties & 0x02 && (!strcmp(TocEntryList[i].filename, ".") || !strcmp(TocEntryList[i].filename, "..")))
			continue;

		if(TocEntryList[i].fileProperties & 0x02){
			info[j].attr = MC_ATTR_SUBDIR;
		}
		else{
			info[j].attr = 0;
			info[j].fileSizeByte = TocEntryList[i].fileSize;
		}
		strcpy(info[j].name, TocEntryList[i].filename);
		memset(&info[j].createtime, 0, sizeof(PS2TIME)); //�擾���Ȃ�
		memset(&info[j].modifytime, 0, sizeof(PS2TIME)); //�擾�ł��Ȃ�
		j++;
	}

	return j;
}

//-------------------------------------------------
// �p�[�e�B�V�������X�g�ݒ�
void setPartyList(void)
{
	iox_dirent_t dirEnt;
	int hddFd;

	nparties=0;
	
	if((hddFd=fileXioDopen("hdd0:")) < 0)
		return;
	while(fileXioDread(hddFd, &dirEnt) > 0)
	{
		if(nparties >= MAX_PARTITIONS)
			break;
		if((dirEnt.stat.attr & ATTR_SUB_PARTITION) 
				|| (dirEnt.stat.mode == FS_TYPE_EMPTY))
			continue;
		if(!strncmp(dirEnt.name, "PP.HDL.", 7))
			continue;
		if(!strncmp(dirEnt.name, "__", 2) && strcmp(dirEnt.name, "__boot"))
			continue;
		strcpy(parties[nparties++], dirEnt.name);
	}
	fileXioDclose(hddFd);
}

//-------------------------------------------------
// HDD�ǂݍ���
int readHDD(const char *path, FILEINFO *info, int max)
{
	iox_dirent_t dirbuf;
	char party[MAX_PATH], dir[MAX_PATH];
	int i=0, fd, ret;

	if(nparties==0){
		loadHddModules();
		setPartyList();
	}

	if(!strcmp(path, "hdd0:/")){
		for(i=0; i<nparties; i++){
			strcpy(info[i].name, parties[i]);
			info[i].attr = MC_ATTR_SUBDIR;
			info[i].fileSizeByte = 0;
			memset(&info[i].createtime, 0, sizeof(PS2TIME));
			memset(&info[i].modifytime, 0, sizeof(PS2TIME));
		}
		return nparties;
	}

	getHddParty(path, NULL, party, dir);
	ret = mountParty(party);
	if(ret<0) return 0;
	dir[3] = ret+'0';

	if((fd=fileXioDopen(dir)) < 0) return 0;

	while(fileXioDread(fd, &dirbuf)){
		if(dirbuf.stat.mode & FIO_S_IFDIR && (!strcmp(dirbuf.name, ".") || !strcmp(dirbuf.name, "..")))
			continue;

		if(dirbuf.stat.mode & FIO_S_IFDIR){
			info[i].attr = MC_ATTR_SUBDIR;
		}
		else{
			info[i].attr = 0;
			info[i].fileSizeByte = dirbuf.stat.size;
		}
		strcpy(info[i].name, dirbuf.name);
		memset(&info[i].createtime, 0, sizeof(PS2TIME)); //�擾���Ȃ�
		info[i].modifytime.unknown = dirbuf.stat.mtime[0];
		info[i].modifytime.sec = dirbuf.stat.mtime[1];
		info[i].modifytime.min = dirbuf.stat.mtime[2];
		info[i].modifytime.hour = dirbuf.stat.mtime[3];
		info[i].modifytime.day = dirbuf.stat.mtime[4];
		info[i].modifytime.month = dirbuf.stat.mtime[5];
		info[i].modifytime.year = dirbuf.stat.mtime[6]+dirbuf.stat.mtime[7]*256;
		i++;
		if(i==max) break;
	}

	fileXioDclose(fd);

	return i;
}

//-------------------------------------------------
// USB�}�X�X�g���[�W�ǂݍ���
int readMASS(const char *path, FILEINFO *info, int max)
{
	fio_dirent_t record;
	int n=0, dd=-1;

	loadUsbModules();
	
	if ((dd = fioDopen(path)) < 0) goto exit;

	while(fioDread(dd, &record) > 0){
		if((FIO_SO_ISDIR(record.stat.mode)) && (!strcmp(record.name,".") || !strcmp(record.name,"..")))
			continue;

		if(FIO_SO_ISDIR(record.stat.mode)){
			info[n].attr = MC_ATTR_SUBDIR;
		}
		else if(FIO_SO_ISREG(record.stat.mode)){
			info[n].attr = 0;
			info[n].fileSizeByte = record.stat.size;
		}
		else
			continue;

		strcpy(info[n].name, record.name);
		strncpy(info[n].name, info[n].name, 32);
		memset(&info[n].createtime, 0, sizeof(PS2TIME)); //�擾���Ȃ�
		info[n].modifytime.unknown = 0;
		info[n].modifytime.sec = record.stat.mtime[1];
		info[n].modifytime.min = record.stat.mtime[2];
		info[n].modifytime.hour = record.stat.mtime[3];
		info[n].modifytime.day = record.stat.mtime[4];
		info[n].modifytime.month = record.stat.mtime[5];
		info[n].modifytime.year = record.stat.mtime[6] + record.stat.mtime[7]*256;
		n++;
		if(n==max) break;
	}

exit:
	if(dd >= 0) fioDclose(dd);
	return n;
}

//-------------------------------------------------
// �t�@�C�����X�g�擾
int getDir(const char *path, FILEINFO *info)
{
	int max=MAX_ENTRY-2;
	int n;

	if(!strncmp(path, "mc", 2))
		n=readMC(path, info, max);
	else if(!strncmp(path, "hdd", 3))
		n=readHDD(path, info, max);
	else if(!strncmp(path, "mass", 4))
		n=readMASS(path, info, max);
	else if(!strncmp(path, "cdfs", 4))
		n=readCD(path, info, max);
	else
		return 0;

	return n;
}

//-------------------------------------------------
//psu�t�@�C������Q�[���^�C�g���擾
//�߂�l
//0�ȉ�:���s
//    0:����
int getGameTitlePsu(const char *path, const FILEINFO *file, char *out)
{
	char party[MAX_NAME], dir[MAX_PATH];
	int hddin=FALSE;
	int fd;
	int ret;
	int psuSize;
	PSU_HEADER psu_header_dir, psu_header;
	int n,i;
	int seek;
	char tmp[65];
	int fileSize;

	//���[�g���p�[�e�B�V�������X�g
	if(path[0]==0 || !strcmp(path, "hdd0:/")) return -1;

	//�t���p�X
	if(!strncmp(path, "hdd", 3)){
		getHddParty(path, file, party, dir);
		ret = mountParty(party);
		if(ret<0) return -1;
		dir[3]=ret+'0';
		hddin=TRUE;
	}
	else{
		sprintf(dir, "%s%s", path, file->name);
		if(file->attr & MC_ATTR_SUBDIR) strcat(dir, "/");
	}

	//psu�t�@�C���I�[�v���ƃT�C�Y�擾
	if(!strncmp(path, "hdd", 3)){
		hddin=TRUE;
		fd = fileXioOpen(dir, O_RDONLY, fileMode);
		if(fd<0){
			ret=-1;
			goto error;
		}
		psuSize = fileXioLseek(fd, 0, SEEK_END);
		fileXioLseek(fd, 0, SEEK_SET);
	}
	else{
		fd = fioOpen(dir, O_RDONLY);
		if(fd<0){
			ret=-1;
			goto error;
		}
		psuSize = fioLseek(fd, 0, SEEK_END);
		fioLseek(fd, 0, SEEK_SET);
	}

	//psu�w�b�_�ǂݍ���
	if(psuSize<sizeof(PSU_HEADER)){
		ret=-2;
		goto error;
	}
	memset(&psu_header_dir, 0, sizeof(PSU_HEADER));
	if(hddin)
		fileXioRead(fd, (char*)&psu_header_dir, sizeof(PSU_HEADER));
	else
		fioRead(fd, &psu_header_dir, sizeof(PSU_HEADER));
	n = psu_header_dir.size;	//�t�@�C����
	seek = sizeof(PSU_HEADER);	//�t�@�C���̃V�[�N

	ret=-3;
	//psu_header[0]����ǂݍ���
	for(i=0;i<n;i++){
		//�t�@�C���w�b�_�ǂݍ���
		if(psuSize<seek+sizeof(PSU_HEADER)){
			ret=-4;
			goto error;
		}
		memset(&psu_header, 0, sizeof(PSU_HEADER));
		if(hddin)
			fileXioRead(fd, (char*)&psu_header, sizeof(PSU_HEADER));
		else
			fioRead(fd, &psu_header, sizeof(PSU_HEADER));
		seek += sizeof(PSU_HEADER);
		//�Q�[���^�C�g��
		if(!strcmp(psu_header.name, "icon.sys")){
			if(hddin){
				fileXioLseek(fd, seek+0xC0, SEEK_SET);
				fileXioRead(fd, tmp, 64);
				out[64]=0;
				fileXioLseek(fd, seek, SEEK_SET);
			}
			else{
				fioLseek(fd, seek+0xC0, SEEK_SET);
				fioRead(fd, tmp, 64);
				out[64]=0;
				fioLseek(fd, seek, SEEK_SET);
			}
			sjis2ascii(tmp, out);
			ret=0;	//����
			break;
		}
		//���̃t�@�C���w�b�_�̈ʒu�ɃV�[�N
		if(psu_header.size>0){
			fileSize = (((psu_header.size-1)/0x400)+1)*0x400;
			if(psuSize<seek + fileSize){
				ret=-4;
				goto error;
			}
			seek += fileSize;
			if(hddin)
				fileXioLseek(fd, seek, SEEK_SET);
			else
				fioLseek(fd, seek, SEEK_SET);
		}
	}
error:
	//psu�t�@�C���N���[�Y
	if(hddin)
		fileXioClose(fd);
	else
		fioClose(fd);

	return ret;
}

//-------------------------------------------------
// �Z�[�u�f�[�^�^�C�g���̎擾
//�߂�l
//0�ȉ�:���s
//    0:PS2�̃Z�[�u�f�[�^�^�C�g���擾����
//    1:PS1�̃Z�[�u�f�[�^�^�C�g���擾����
int getGameTitle(const char *path, const FILEINFO *file, char *out)
{
	char party[MAX_NAME], dir[MAX_PATH];
	int fd=-1, hddin=FALSE, ret;
	char *ext;

	//���[�g���p�[�e�B�V�������X�g
	if(path[0]==0 || !strcmp(path, "hdd0:/")) return -1;

	//�t���p�X
	if(!strncmp(path, "hdd", 3)){
		getHddParty(path, file, party, dir);
		ret = mountParty(party);
		if(ret<0) return -1;
		dir[3]=ret+'0';
		hddin=TRUE;
	}
	else{
		sprintf(dir, "%s%s", path, file->name);
		if(file->attr & MC_ATTR_SUBDIR) strcat(dir, "/");
	}

	ret = -1;
	if(file->attr & MC_ATTR_SUBDIR){	//�t�H���_�̂Ƃ�
		if(hddin){
			//HDD
			strcat(dir, "icon.sys");
			if((fd=fileXioOpen(dir, O_RDONLY, fileMode)) >= 0){
				//icon.sys����擾
				if(fileXioLseek(fd,0,SEEK_END) <= 0x100)
					goto error;
				fileXioLseek(fd, 0xC0, SEEK_SET);
				fileXioRead(fd, out, 16*4);
				out[16*4] = 0;
				fileXioClose(fd); fd=-1;
				ret = 0;	//PS2
			}
		}
		else{
			//HDD�ȊO
			strcat(dir, "icon.sys");
			if((fd=fioOpen(dir, O_RDONLY)) >= 0){
				//icon.sys����擾
				if(fioLseek(fd,0,SEEK_END) <= 0x100)
					goto error;
				fioLseek(fd, 0xC0, SEEK_SET);
				fioRead(fd, out, 16*4);
				out[16*4] = 0;
				fioClose(fd); fd=-1;
				ret = 0;	//PS2
			}
		}
		//icon.sys���Ȃ��Ƃ�PS1�擾���Ă݂�
		if(ret!=0){
			char ps1dir[MAX_PATH];
			FILEINFO fi;
			sprintf(ps1dir, "%s%s/", path, file->name);
			strcpy(fi.name, file->name);
			fi.attr = 0;	//�����̓t�@�C��
			if(getGameTitle(ps1dir, &fi, out)>=0)
				ret = 1;	//PS1
			else
				out[0]=0;
		}
	}
	else{	//�t�@�C���̂Ƃ�
		ext = getExtension(file->name);
		if(ext!=NULL&&!stricmp(ext, ".psu")){
			//psu�t�@�C���̂Ƃ�
			if(getGameTitlePsu(path, file, out)>=0)
				ret = 0;	//PS2
			else
				out[0]=0;
		}
		else{
			//psu�t�@�C������Ȃ��Ƃ�PS1�̃Q�[���^�C�g���擾���Ă݂�
			if(hddin){
				if((fd=fileXioOpen(dir, O_RDONLY, fileMode)) < 0) goto error;
				if(fileXioLseek(fd, 0, SEEK_END) < 0x2000) goto error;
				fileXioLseek(fd, 0, SEEK_SET);
				fileXioRead(fd, out, 2);
				if(strncmp(out, "SC", 2)) goto error;
				fileXioLseek(fd, 4, SEEK_SET);
				fileXioRead(fd, out, 16*4);
				out[16*4] = 0;
				fileXioClose(fd); fd=-1;
				ret=1;	//PS1
			}
			else{
				if((fd=fioOpen(dir, O_RDONLY)) < 0) goto error;
				if(fioLseek(fd, 0, SEEK_END) < 0x2000) goto error;
				fioLseek(fd, 0, SEEK_SET);
				fioRead(fd, out, 2);
				if(strncmp(out, "SC", 2)) goto error;
				fioLseek(fd, 4, SEEK_SET);
				fioRead(fd, out, 16*4);
				out[16*4] = 0;
				fioClose(fd); fd=-1;
				ret=1;	//PS1
			}
		}
	}
error:
	if(fd>=0){
		if(hddin)
			fileXioClose(fd);
		else
			fioClose(fd);
	}
	return ret;
}

//-------------------------------------------------
// ���j���[
int menu(const char *path, const char *file)
{
	uint64 color;
	char enable[NUM_MENU], tmp[MAX_PATH];
	int x, y, i, sel;

	int menu_x = SCREEN_WIDTH-FONT_WIDTH*19;
	int menu_y = FONT_HEIGHT*4;
	int menu_w = FONT_WIDTH*15;
	int menu_h = FONT_HEIGHT*(NUM_MENU+1);

	// ���j���[���ڗL���E�����ݒ�
	memset(enable, TRUE, NUM_MENU);	//�S��TRUE�ɂ���

	if(!strcmp(path,"hdd0:/") || path[0]==0){
		enable[COPY] = FALSE;
		enable[CUT] = FALSE;
		enable[PASTE] = FALSE;
		enable[DELETE] = FALSE;
		enable[RENAME] = FALSE;
		enable[NEWDIR] = FALSE;
		enable[GETSIZE] = FALSE;
		enable[EXPORT] = FALSE;
		enable[IMPORT] = FALSE;
	}

	if(!strncmp(path, "mc", 2))
		enable[RENAME] = FALSE;

	if(!strncmp(path, "hdd", 3))
		enable[EXPORT] = FALSE;

	if(!strncmp(path,"cdfs",4)){
		enable[CUT] = FALSE;
		enable[PASTE] = FALSE;
		enable[DELETE] = FALSE;
		enable[RENAME] = FALSE;
		enable[NEWDIR] = FALSE;
		enable[EXPORT] = FALSE;
	}
	if(!strncmp(path, "mass", 4)){
		enable[RENAME] = FALSE;
		enable[EXPORT] = FALSE;
	}

	//�}�[�N�����t�@�C�����Ȃ�
	if(nmarks==0){
		//R1�{�^�����������Ƃ��̃J�[�\���̈ʒu��".."�̂Ƃ�
		if(!strcmp(file, "..")){
			enable[COPY] = FALSE;
			enable[CUT] = FALSE;
			enable[DELETE] = FALSE;
			enable[RENAME] = FALSE;
			enable[GETSIZE] = FALSE;
			enable[EXPORT] = FALSE;
			enable[IMPORT] = FALSE;
		}
	}
	else{
		//�}�[�N�����t�@�C��������
		enable[RENAME] = FALSE;
	}

	//�N���b�v�{�[�h�ɋL�������t�@�C�����Ȃ�
	if(nclipFiles==0)
		enable[PASTE] = FALSE;

	// �����I�����ݒ�
	for(sel=0; sel<NUM_MENU; sel++)
		if(enable[sel]==TRUE) break;

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad & PAD_UP && sel<NUM_MENU){
				do{
					sel--;
					if(sel<0) sel=NUM_MENU-1;
				}while(!enable[sel]);
			}else if(new_pad & PAD_DOWN && sel<NUM_MENU){
				do{
					sel++;
					if(sel==NUM_MENU) sel=0;
				}while(!enable[sel]);
			}else if(new_pad & PAD_CROSS)
				return -1;
			else if(new_pad & PAD_CIRCLE)
				break;
		}

		// �`��J�n
		drawDialogTmp(menu_x, menu_y, menu_x+menu_w, menu_y+menu_h, setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
		for(i=0,y=74; i<NUM_MENU; i++){
			if(i==COPY) strcpy(tmp, lang->filer_menu_copy);
			else if(i==CUT) strcpy(tmp, lang->filer_menu_cut);
			else if(i==PASTE) strcpy(tmp, lang->filer_menu_paste);
			else if(i==DELETE) strcpy(tmp, lang->filer_menu_delete);
			else if(i==RENAME) strcpy(tmp, lang->filer_menu_rename);
			else if(i==NEWDIR) strcpy(tmp, lang->filer_menu_newdir);
			else if(i==GETSIZE) strcpy(tmp, lang->filer_menu_getsize);
			else if(i==EXPORT) strcpy(tmp, lang->filer_menu_exportpsu);
			else if(i==IMPORT) strcpy(tmp, lang->filer_menu_importpsu);

			if(enable[i]){
				if(sel==i)
					color = setting->color[COLOR_HIGHLIGHTTEXT];	//����
				else
					color = setting->color[COLOR_TEXT];	//�m�[�}��
			}
			else
				color = setting->color[COLOR_GRAYTEXT];	//����

			printXY(tmp, menu_x+FONT_WIDTH*2, menu_y+FONT_HEIGHT/2+i*FONT_HEIGHT, color, TRUE);
			y+=FONT_HEIGHT;
		}
		if(sel<NUM_MENU)
			printXY(">", menu_x+FONT_WIDTH, menu_y+FONT_HEIGHT/2+sel*FONT_HEIGHT, setting->color[COLOR_HIGHLIGHTTEXT], TRUE);	//����

		// �������
		x = FONT_WIDTH*1;
		y = SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT;
		itoSprite(setting->color[COLOR_BACKGROUND],
			0, y,
			SCREEN_WIDTH, y+FONT_HEIGHT, 0);
		sprintf(tmp,"��:%s �~:%s", lang->gen_ok, lang->gen_cancel);
		printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE);
		drawScr();
	}

	return sel;
}

//-------------------------------------------------
// �t�@�C���T�C�Y�擾
size_t getFileSize(const char *path, const FILEINFO *file)
{
	size_t size;
	FILEINFO files[MAX_ENTRY];
	char dir[MAX_PATH], party[MAX_NAME];
	int nfiles, i, ret, fd;

	if(file->attr & MC_ATTR_SUBDIR){
		sprintf(dir, "%s%s/", path, file->name);
		// �Ώۃt�H���_���̑S�t�@�C���E�t�H���_�T�C�Y�����v
		nfiles = getDir(dir, files);
		for(i=size=0; i<nfiles; i++){
			ret=getFileSize(dir, &files[i]);
			if(ret < 0) size = -1;
			else		size+=ret;
		}
	}
	else{
		// �p�[�e�B�V�����}�E���g
		if(!strncmp(path, "hdd", 3)){
			getHddParty(path,file,party,dir);
			ret = mountParty(party);
			if(ret<0) return 0;
			dir[3] = ret+'0';
		}else
			sprintf(dir, "%s%s", path, file->name);
		// �t�@�C���T�C�Y�擾
		if(!strncmp(path, "hdd", 3)){
			fd = fileXioOpen(dir, O_RDONLY, fileMode);
			size = fileXioLseek(fd,0,SEEK_END);
			fileXioClose(fd);
		}else{
			fd = fioOpen(dir, O_RDONLY);
			size = fioLseek(fd,0,SEEK_END);
			fioClose(fd);
		}
	}
	return size;
}

//-------------------------------------------------
// �t�@�C���E�t�H���_�폜
int delete(const char *path, const FILEINFO *file)
{
	FILEINFO files[MAX_ENTRY];
	char party[MAX_NAME], dir[MAX_PATH], hdddir[MAX_PATH];
	int nfiles, i, ret;

	// �p�[�e�B�V�����}�E���g
	if(!strncmp(path, "hdd", 3)){
		getHddParty(path,file,party,hdddir);
		ret = mountParty(party);
		if(ret<0) return 0;
		hdddir[3] = ret+'0';
	}
	sprintf(dir, "%s%s", path, file->name);

	if(file->attr & MC_ATTR_SUBDIR){
		strcat(dir,"/");
		// �Ώۃt�H���_���̑S�t�@�C���E�t�H���_���폜
		nfiles = getDir(dir, files);
		for(i=0; i<nfiles; i++){
			ret=delete(dir, &files[i]);
			if(ret < 0) return -1;
		}
		// �Ώۃt�H���_���폜
		if(!strncmp(dir, "mc", 2)){
			mcSync(MC_WAIT, NULL, NULL);
			mcDelete(dir[2]-'0', 0, &dir[4]);
			mcSync(MC_WAIT, NULL, &ret);
		}else if(!strncmp(path, "hdd", 3)){
			ret = fileXioRmdir(hdddir);
		}else if(!strncmp(path, "mass", 4)){
			sprintf(dir, "mass0:%s%s", &path[5], file->name);
			ret = fioRmdir(dir);
			if (ret < 0){
				dir[4] = 1 + '0';
				ret = fioRmdir(dir);
			}
		}
	} else {
		// �Ώۃt�@�C�����폜
		if(!strncmp(path, "mc", 2)){
			mcSync(MC_WAIT, NULL, NULL);
			mcDelete(dir[2]-'0', 0, &dir[4]);
			mcSync(MC_WAIT, NULL, &ret);
		}else if(!strncmp(path, "hdd", 3)){
			ret = fileXioRemove(hdddir);
		}else if(!strncmp(path, "mass", 4)){
			ret = fioRemove(dir);
		}
	}
	return ret;
}

//-------------------------------------------------
// �t�@�C���E�t�H���_���l�[��
int Rename(const char *path, const FILEINFO *file, const char *name)
{
	char party[MAX_NAME], oldPath[MAX_PATH], newPath[MAX_PATH];
	int ret=0;

	if(!strncmp(path, "hdd", 3)){
		sprintf(party, "hdd0:%s", &path[6]);
		*strchr(party, '/')=0;
		sprintf(oldPath, "pfs0:%s", strchr(&path[6], '/')+1);
		sprintf(newPath, "%s%s", oldPath, name);
		strcat(oldPath, file->name);

		ret = mountParty(party);
		if(ret<0) return -1;
		oldPath[3] = newPath[3] = ret+'0';

		ret=fileXioRename(oldPath, newPath);
	}else
		return -1;

	return ret;
}

//-------------------------------------------------
// �V�K�t�H���_�쐬
int newdir(const char *path, const char *name)
{
	char party[MAX_NAME], dir[MAX_PATH];
	int ret=0;

	if(!strncmp(path, "hdd", 3)){
		getHddParty(path,NULL,party,dir);
		ret = mountParty(party);
		if(ret<0) return -1;
		dir[3] = ret+'0';
		//fileXioChdir(dir);
		strcat(dir, name);
		ret = fileXioMkdir(dir, fileMode);
	}else if(!strncmp(path, "mc", 2)){
		sprintf(dir, "%s%s", path+4, name);
		mcSync(MC_WAIT, NULL, NULL);
		mcMkDir(path[2]-'0', 0, dir);
		mcSync(MC_WAIT, NULL, &ret);
		if(ret == -4)
			ret = -17;
	}else if(!strncmp(path, "mass", 4)){
		strcpy(dir, path);
		strcat(dir, name);
		ret = fioMkdir(dir);
	}

	return ret;
}

//-------------------------------------------------
// �t�@�C���R�s�[
int copy(const char *outPath, const char *inPath, FILEINFO file, int n)
{
	FILEINFO files[MAX_ENTRY];
	char out[MAX_PATH], in[MAX_PATH], tmp[MAX_PATH],
		*buff=NULL, inParty[MAX_NAME], outParty[MAX_NAME];
	int hddout=FALSE, hddin=FALSE, nfiles, i;
	size_t size, outsize;
	int ret=-1, pfsout=-1, pfsin=-1, in_fd=-1, out_fd=-1, buffSize;
	int copyret;
	mcTable mcDir __attribute__((aligned(64)));	//mcSetFileInfo()�p

	//�t�H���_���܂��̓t�@�C�����̕�����
	if(!strncmp(outPath, "mc", 2)){
		if(strlen(file.name)>32){
			return -1;
		}
	}
	else if(!strncmp(outPath, "mass", 4)){
		if(strlen(file.name)>128){
			return -1;
		}
	}
	else if(!strncmp(outPath, "hdd", 3)){
		if(strlen(file.name)>256){
			return -1;
		}
	}
	sprintf(out, "%s%s", outPath, file.name);
	sprintf(in, "%s%s", inPath, file.name);
	
	//���̓p�X�̃p�[�e�B�V�������}�E���g���Ă��邩�`�F�b�N����
	if(!strncmp(inPath, "hdd", 3)){
		hddin = TRUE;
		getHddParty(inPath, &file, inParty, in);
		if(!strcmp(inParty, mountedParty[0]))
			pfsin=0;	//pfs0�Ƀ}�E���g���Ă���
		else if(!strcmp(inParty, mountedParty[1]))
			pfsin=1;	//pfs1�Ƀ}�E���g���Ă���
		else
			pfsin=-1;	//�}�E���g���Ă��Ȃ�
	}
	//�o�̓p�X�̃p�[�e�B�V�������}�E���g���Ă��邩�`�F�b�N����
	if(!strncmp(outPath, "hdd", 3)){
		hddout = TRUE;
		getHddParty(outPath, &file, outParty, out);
		if(!strcmp(outParty, mountedParty[0]))
			pfsout=0;	//pfs0�Ƀ}�E���g���Ă���
		else if(!strcmp(outParty, mountedParty[1]))
			pfsout=1;	//pfs1�Ƀ}�E���g���Ă���
		else
			pfsout=-1;	//�}�E���g���Ă��Ȃ�
	}

	//���̓p�X�̐ݒ�ƃ}�E���g
	if(hddin){
		if(pfsin<0){	//�}�E���g���Ă��Ȃ��Ƃ�
			if(pfsout==0)
				pfsin=1;	//�}�E���g����ԍ�
			else
				pfsin=0;	//�}�E���g����ԍ�
			//�A���}�E���g
			sprintf(tmp, "pfs%d:", pfsin);
			if(mountedParty[pfsin][0]!=0){
				fileXioUmount(tmp);
				mountedParty[pfsin][0]=0;
			}
			//�}�E���g
			printf("%s mounting\n", inParty);
			if(fileXioMount(tmp, inParty, FIO_MT_RDWR) < 0)
				return -1;
			strcpy(mountedParty[pfsin], inParty);
		}
		in[3]=pfsin+'0';	//���̓p�X
	}
	else
		sprintf(in, "%s%s", inPath, file.name);	//���̓p�X
	//�o�̓p�X�̐ݒ�ƃ}�E���g
	if(hddout){
		if(pfsout<0){	//�}�E���g���Ă��Ȃ��Ƃ�
			if(pfsin==0)
				pfsout=1;	//�}�E���g����ԍ�
			else
				pfsout=0;	//�}�E���g����ԍ�
			//�A���}�E���g
			sprintf(tmp, "pfs%d:", pfsout);
			if(mountedParty[pfsout][0]!=0){
				fileXioUmount(tmp);
				mountedParty[pfsout][0]=0;
			}
			//�}�E���g
			printf("%s mounting\n", outParty);
			if(fileXioMount(tmp, outParty, FIO_MT_RDWR) < 0)
				return -1;
			strcpy(mountedParty[pfsout], outParty);
		}
		out[3]=pfsout+'0';	//�o�̓p�X
	}
	else
		sprintf(out, "%s%s", outPath, file.name);	//�o�̓p�X

	// �t�H���_�̏ꍇ
	if(file.attr & MC_ATTR_SUBDIR){
		// �t�H���_�쐬
		ret = newdir(outPath, file.name);
		if(ret == -17){
			drawDark();
			itoGsFinish();
			itoSwitchFrameBuffers();
			drawDark();
			ret=-1;
			if(title) ret=getGameTitle(outPath, &file, tmp);
			if(ret<0) sprintf(tmp, "%s%s/", outPath, file.name);
			strcat(tmp, "\n");
			strcat(tmp, lang->filer_overwrite);
			ret = MessageBox(tmp, LBF_VER, MB_YESNO);
			if(ret!=IDYES) return -1;
			drawMsg(lang->filer_pasting);
		}
		else if(ret < 0)
			return -1;

		// �t�H���_�̒��g��S�R�s�[
		sprintf(out, "%s%s/", outPath, file.name);
		sprintf(in, "%s%s/", inPath, file.name);
		nfiles = getDir(in, files);
		for(i=0; i<nfiles; i++){
			copyret = copy(out, in, files[i], n+1);
			if(copyret < 0) return -1;
		}
		//�t�H���_��mcSetFileInfo
		if(!strncmp(inPath, "mc", 2) && !strncmp(outPath, "mc", 2)){
			memset(&mcDir, 0, sizeof(mcTable));
			memcpy(&mcDir._create, &file.createtime, 8);
			memcpy(&mcDir._modify, &file.modifytime, 8);
			mcDir.fileSizeByte = file.fileSizeByte;
			mcDir.attrFile = file.attr;
			strcpy(mcDir.name, file.name);

			//
			sprintf(out, "%s%s", outPath, file.name);
			mcGetInfo(out[2]-'0', 0, NULL, NULL, NULL);	//Wakeup call
			mcSync(MC_WAIT, NULL, NULL);
			mcSetFileInfo(out[2]-'0', 0, &out[4], (char*)&mcDir, 0xFFFF); //Fix file stats
			mcSync(MC_WAIT, NULL, NULL);
		}
		return 0;
	}

	// ���̓t�@�C���I�[�v���ƃt�@�C���T�C�Y�擾
	if(hddin){
		in_fd = fileXioOpen(in, O_RDONLY, fileMode);
		if(in_fd<0) goto error;
		size = fileXioLseek(in_fd,0,SEEK_END);
		fileXioLseek(in_fd,0,SEEK_SET);
	}
	else{
		in_fd = fioOpen(in, O_RDONLY);
		if(in_fd<0) goto error;
		size = fioLseek(in_fd,0,SEEK_END);
		fioLseek(in_fd,0,SEEK_SET);
	}
	// �o�̓t�@�C���I�[�v��
	if(hddout){
		// O_TRUNC �������Ȃ����߁A�I�[�v���O�Ƀt�@�C���폜
		fileXioRemove(out);
		out_fd = fileXioOpen(out,O_WRONLY|O_TRUNC|O_CREAT,fileMode);
		if(out_fd<0) goto error;
	}
	else{
		out_fd=fioOpen(out, O_WRONLY | O_TRUNC | O_CREAT);
		if(out_fd<0) goto error;
	}

	// �������Ɉ�x�œǂݍ��߂�t�@�C���T�C�Y�������ꍇ
	buff = (char*)malloc(size);
	if(buff==NULL){
		buff = (char*)malloc(32768);
		buffSize = 32768;
	}
	else
		buffSize = size;

	while(size>0){
		// ����
		if(hddin) buffSize = fileXioRead(in_fd, buff, buffSize);
		else buffSize = fioRead(in_fd, buff, buffSize);
		// �o��
		if(hddout){
			outsize = fileXioWrite(out_fd,buff,buffSize);
			if(buffSize!=outsize){
				fileXioClose(out_fd); out_fd=-1;
				fileXioRemove(out);
				goto error;
			}
		}
		else{
			outsize = fioWrite(out_fd,buff,buffSize);
			if(buffSize!=outsize){
				fioClose(out_fd); out_fd=-1;
				mcSync(MC_WAIT, NULL, NULL);
				mcDelete(out[2]-'0', 0, &out[4]);
				mcSync(MC_WAIT, NULL, NULL);
				goto error;
			}
		}
		size -= buffSize;
	}
	//�t�@�C����mcSetFileInfo
	if(!strncmp(inPath, "mc", 2) && !strncmp(outPath, "mc", 2)){
		memset(&mcDir, 0, sizeof(mcTable));
		memcpy(&mcDir._create, &file.createtime, 8);
		memcpy(&mcDir._modify, &file.modifytime, 8);
		mcDir.fileSizeByte = file.fileSizeByte;
		mcDir.attrFile = file.attr;
		strcpy(mcDir.name, file.name);

		//
		mcGetInfo(out[2]-'0', 0, NULL, NULL, NULL);	//Wakeup call
		mcSync(MC_WAIT, NULL, NULL);
		mcSetFileInfo(out[2]-'0', 0, &out[4], (char*)&mcDir, 0xFFFF); //Fix file stats
		mcSync(MC_WAIT, NULL, NULL);
	}
	ret=0;
error:
	free(buff);
	if(in_fd>0){
		if(hddin) fileXioClose(in_fd);
		else fioClose(in_fd);
	}
	if(out_fd>0){	//�C������
		if(hddout) fileXioClose(out_fd);
		else fioClose(out_fd);
	}
	return ret;
}

//-------------------------------------------------
// �y�[�X�g
int paste(const char *path)
{
	char tmp[MAX_PATH];
	int i, ret=-1;

	//�R�s�[��ƃR�s�[��������
	if(!strcmp(path,clipPath)) return -1;

	//�y�[�X�g����
	for(i=0; i<nclipFiles; i++){
		strcpy(tmp, clipFiles[i].name);
		if(clipFiles[i].attr & MC_ATTR_SUBDIR) strcat(tmp,"/");
		strcat(tmp, " ");
		strcat(tmp, lang->filer_pasting);
		drawMsg(tmp);
		ret=copy(path, clipPath, clipFiles[i], 0);
		//�R�s�[���s�����璆�f
		if(ret < 0) break;
		//�؂���t���O�������Ă�����폜����
		if(cut){
			ret=delete(clipPath, &clipFiles[i]);
			//�폜���s�����璆�f
			if(ret<0) break;
		}
	}
	//HDD�A���}�E���g
	if(mountedParty[0][0]!=0){
		fileXioUmount("pfs0:");
		mountedParty[0][0]=0;
	}
	if(mountedParty[1][0]!=0){
		fileXioUmount("pfs1:");
		mountedParty[1][0]=0;
	}

	return ret;
}

#ifdef ENABLE_PSB
//-------------------------------------------------
//psbCommand psb�R�}���h���s
int psbCommand(void)
{
	int cmd;
	char path[2][MAX_PATH];
	char dir[MAX_PATH];
	char pathtmp[MAX_PATH];
	int len;
	FILEINFO file;
	char *p;
	int fd;
	int ret=0;
	char message[2048];
	FILEINFO clipFilesBackup;
	int nclipFilesBackup;

	//����������Ȃ�
	if(psb_argc<2)
		return 0;

	//�R�}���h�`�F�b�N
	cmd=-1;
	if(!strnicmp(psb_argv[0], "copy", 4))
		cmd=0;
	else if(!strnicmp(psb_argv[0], "move", 4))
		cmd=1;
	else if(!strnicmp(psb_argv[0], "del", 3))
		cmd=2;
	else if(!strnicmp(psb_argv[0], "mkdir", 5))
		cmd=3;
	else if(!strnicmp(psb_argv[0], "rmdir", 5))
		cmd=4;

	if(cmd==-1) return 0;

	strcpy(path[0], psb_argv[1]);
	strcpy(path[1], psb_argv[2]);

	//Module
	if(!strncmp(path[0], "cdfs", 4)||!strncmp(path[1], "cdfs", 4))
		loadCdModules();
	else if(!strncmp(path[0], "hdd", 3)||!strncmp(path[1], "hdd", 3)){
		if(nparties==0){
			loadHddModules();
			setPartyList();
		}
	}
	else if(!strncmp(path[0], "mass", 4)||!strncmp(path[1], "mass", 4))
		loadUsbModules();

	//path[0]�̍Ōオ�X���b�V���̂Ƃ��폜
	len = strlen(path[0]);
	if(len>0){
		if(path[0][len-1]=='/')
			path[0][len-1]='\0';
	}
	//path[1]�̍Ō�ɃX���b�V�������Ƃ�����
	if(psb_argc>1){
		len = strlen(path[1]);
		if(len>0){
			if(path[1][len-1]!='/')
				strcat(path[1], "/");
		}
	}

	//path[0]����FILEINFO���쐬����
	//file.name
	if((p = strrchr(path[0], '/')))
		strcpy(file.name, p+1);
	//file.attr
	ret=0;
	if(!strncmp(path[0], "mc", 2)){
		mcTable mcDir __attribute__((aligned(64)));
		int mcret;
		//�t�H���_�Ƃ��ăI�[�v�����Ă݂�
		strcpy(pathtmp, path[0]+4); strcat(pathtmp,"/*");
		mcGetDir(path[0][2]-'0', 0, pathtmp, 0, 1, &mcDir);
		mcSync(MC_WAIT, NULL, &mcret);
		if(mcret<0){
			//���s������t�@�C���Ƃ��ăI�[�v�����Ă݂�
			fd = fioOpen(path[0], O_RDONLY);
			if(fd<0)
				ret=-1;	//���s
			else{
				fioClose(fd);
				file.attr=0;	//�t�@�C��
			}
		}
		else{
			file.attr=MC_ATTR_SUBDIR;	//�t�H���_
		}
	}
	else if(!strncmp(path[0], "cdfs", 4)){
		//�t�H���_�Ƃ��ăI�[�v�����Ă݂�
		struct TocEntry TocEntryList;
		char cdfsdir[MAX_PATH];
		int n;
		strcpy(cdfsdir, path[0]+5);
		CDVD_FlushCache();
		n = CDVD_GetDir(cdfsdir, NULL, CDVD_GET_FILES_AND_DIRS, &TocEntryList, 1, cdfsdir);
		if(n<0){
			//���s������t�@�C���Ƃ��ăI�[�v�����Ă݂�
			fd = fioOpen(path[0], O_RDONLY);
			if(fd<0)
				ret=-1;	//���s
			else{
				fioClose(fd);
				file.attr=0;	//�t�@�C��
			}
		}
		else{
			file.attr=MC_ATTR_SUBDIR;	//�t�H���_
		}
		if(setting->discControl) CDVD_Stop();
	}
	else if(!strncmp(path[0], "hdd", 3)){
		char party[MAX_NAME];
		int r;
		//�t�@�C����hdd�ɂ���Ƃ��p�X��ύX
		getHddParty(path[0], NULL, party, pathtmp);
		//�}�E���g
		r = mountParty(party);
		pathtmp[3] = r+'0';

		//�t�H���_�Ƃ��ăI�[�v�����Ă݂�
		fd = fileXioDopen(pathtmp);
		if(fd<0){
			//���s������t�@�C���Ƃ��ăI�[�v�����Ă݂�
			fd = fileXioOpen(pathtmp, O_RDONLY, fileMode);
			if(fd<0){
				ret=-1;	//���s
			}
			else{
				fileXioClose(fd);
				file.attr=0;	//�t�@�C��
			}
		}
		else{
			fileXioDclose(fd);
			file.attr=MC_ATTR_SUBDIR;
		}
	}
	else if(!strncmp(path[0], "mass", 4)){
		//�t�H���_�Ƃ��ăI�[�v�����Ă݂�
		strcpy(pathtmp, path[0]);
		fd = fioDopen(pathtmp);
		if(fd<0){
			//���s������t�@�C���Ƃ��ăI�[�v�����Ă݂�
			fd = fioOpen(path[0], O_RDONLY);
			if(fd<0)
				ret=-1;	//���s
			else{
				fioClose(fd);
				file.attr=0;	//�t�@�C��
			}
		}
		else{
			file.attr=MC_ATTR_SUBDIR;	//�t�H���_
			fioDclose(fd);
		}
	}
	if(cmd!=3 && ret==-1) return 0;	//file.attr�擾���s

	//path[0]������e�f�B���N�g����
	strcpy(dir, path[0]);
	if((p=strrchr(dir, '/'))){
		p++;
		*p=0;
	}

	//�N���b�v�{�[�h�����o�b�N�A�b�v
	clipFilesBackup = clipFiles[0];
	nclipFilesBackup = nclipFiles;

	//�R�}���h
	switch(cmd)
	{
		case 0:	//�R�s�[
		{
			//
			clipFiles[0]=file;
			nclipFiles = 1;
			//�R�s�[���̃f�B���N�g����(clipPath)
			strcpy(clipPath, dir);
			//
			cut=FALSE;	//�R�s�[
			//�y�[�X�g�J�n
			ret=paste(path[1]);
			if(ret) MessageBox("copy Failed", LBF_VER, MB_OK);
			break;
		}
		case 1:	//�ړ�
		{
			//
			clipFiles[0]=file;
			nclipFiles = 1;
			//�R�s�[���̃f�B���N�g����(clipPath)
			strcpy(clipPath, dir);
			//
			cut=TRUE;	//�ړ�
			//�y�[�X�g�J�n
			ret=paste(path[1]);
			if(ret) MessageBox("move Failed", LBF_VER, MB_OK);
			break;
		}
		case 2:	//�폜
		{
			int ynret;
			sprintf(message, "%s", file.name);
			if(file.attr & MC_ATTR_SUBDIR) strcat(message, "/");
			strcat(message, "\n");
			strcat(message, lang->filer_delete);
			ynret = MessageBox(message, LBF_VER, MB_YESNO);
			if(ynret==IDYES){
				//�폜�J�n
				ret=delete(dir, &file);
			}
			if(ret) MessageBox("del Failed", LBF_VER, MB_OK);
			break;
		}
		case 3:	//�t�H���_���쐬
		{
			//�쐬�J�n
			ret=newdir(dir, file.name);
			if(ret) MessageBox("mkdir Failed", LBF_VER, MB_OK);
			break;
		}
		case 4:	//�t�H���_���폜
		{
			int ynret;
			//�f�B���N�g���폜
			if(file.attr==MC_ATTR_SUBDIR){
				sprintf(message, "%s", file.name);
				if(file.attr & MC_ATTR_SUBDIR) strcat(message, "/");
				strcat(message, "\n");
				strcat(message, lang->filer_delete);
				ynret = MessageBox(message, LBF_VER, MB_YESNO);
				if(ynret==IDYES){
					//�폜�J�n
					ret=delete(dir, &file);
				}
			}
			else{
				ret=-1;
			}
			if(ret) MessageBox("rmdir Failed", LBF_VER, MB_OK);
			break;
		}
	}

	//�N���b�v�{�[�h�������ɖ߂�
	clipFiles[0] = clipFilesBackup;
	nclipFiles = nclipFilesBackup;

	return ret;
}

//-------------------------------------------------
//psbParse psb�t�@�C���p�[�X
void psbParse(const char *str)
{
	char *p;
	int len;
	int i;
	int flag;
	int l;
	char strtmp[(MAX_PATH+2)*MAX_ARGC];
	char tmp[MAX_PATH+2];

	strcpy(strtmp, str);

	//������
	psb_argc=0;
	memset(psb_argv, 0, sizeof(psb_argv));

	//���s�R�[�h������
	p=strrchr(strtmp,'\n');
	if(p!=NULL) *p='\0';
	p=strrchr(strtmp,'\r');
	if(p!=NULL) *p='\0';

	len=strlen(strtmp);

	//
	if(len==0) return;

	//�X�y�[�X��\0�ɕϊ�
	flag=0;
	for(i=0;i<len;i++){
		if(strtmp[i]=='\"')	//�_�u���N�H�[�e�[�V�����t���O
			flag=1-flag;
		else if((strtmp[i]==' ')&&(flag==0))
			strtmp[i]='\0';
	}

	//\0��؂��ǂ݂Ƃ�
	for(i=0;i<len;i++){
		if(strtmp[i]=='\0'){
			//�������Ȃ�
		}
		else{
			l=strlen(strtmp+i);
			if(psb_argc<MAX_ARGC){
				if(l<MAX_PATH+2)
					strcpy(psb_argv[psb_argc], strtmp+i);
				psb_argc++;
			}
			i+=l;
		}
	}

	//�_�u���N�H�[�e�[�V����������
	for(i=0;i<psb_argc;i++){
		if(psb_argv[i][0]=='\"'){
			//�O�̃_�u���N�H�[�e�[�V����������
			strcpy(tmp, &psb_argv[i][1]);
			strcpy(psb_argv[i],tmp);
			//���̃_�u���N�H�[�e�[�V����������
			p=strrchr(psb_argv[i],'\"');
			if(p!=NULL) *p='\0';
		}
	}
}

//-------------------------------------------------
//psb�t�@�C�����s
//�߂�l     0:����
//          -1:�t�@�C���I�[�v�����s
//       0�ȏ�:�G���[���o���s�ԍ�
int psb(const char *psbpath)
{
	int fd;
	char buffer[(MAX_PATH+2)*MAX_ARGC];
	int lineno;
	int ret;

	//
	if(!strncmp(psbpath, "cdfs", 4))
		loadCdModules();
	else if(!strncmp(psbpath, "mass", 4))
		loadUsbModules();

	//
	fd=fioOpen(psbpath, O_RDONLY);
	if(fd<0) return -1;

	lineno=0;
	while(1){
		memset(buffer, 0, sizeof(buffer));
		if(fioGets(fd, buffer, (MAX_PATH+2)*MAX_ARGC)==0)
			break;
		if(buffer[0]=='\n'){
		}
		else{
			lineno++;
			//�p�[�X
			psbParse(buffer);
			//���s
			if(psbCommand()<0){
				ret=lineno;
				goto psberror;	//�G���[�ł����~
			}
		}
	}
	ret=0;	//�G���[�Ȃ�
psberror:
	fioClose(fd);
	return ret;
}
#endif

//-------------------------------------------------
//�Q�[���^�C�g�����t�@�C�����ɕϊ�
void title2filename(const unsigned char *in, unsigned char *out)
{
	int len;
	int i=0;

	len = strlen(in);
	memcpy(out, in, len);
	for(i=0;i<len;i++){
		//windows�Ńt�@�C�����Ɏg���Ȃ������́u_�v�ɕϊ�
		if(out[i]==0x22) out[i]='_';	// '"'
		if(out[i]==0x2A) out[i]='_';	// '*'
		if(out[i]==0x2C) out[i]='_';	// ','
		if(out[i]==0x2F) out[i]='_';	// '/'
		if(out[i]==0x3A) out[i]='_';	// ':'
		if(out[i]==0x3B) out[i]='_';	// ';'
		if(out[i]==0x3C) out[i]='_';	// '<'
		if(out[i]==0x3E) out[i]='_';	// '>'
		if(out[i]==0x3F) out[i]='_';	// '?'
		if(out[i]==0x5c){	// '\'
			if(i>0){
				if((out[i-1]&0x80)==0) out[i]='_';
			}
		}
		if(out[i]==0x7C) out[i]='_';	// '|'
	}
}

//-------------------------------------------------
//�Q�[���^�C�g����sjis�̉p�����ƋL����ASCII�ɕϊ�
void sjis2ascii(const unsigned char *in, unsigned char *out)
{
	int i=0;
	int code;
	unsigned char ascii;
	int n=0;

	while(in[i]){
		if(in[i] & 0x80){
			// SJIS�R�[�h�̐���
			code = in[i++];
			code = (code<<8) + in[i++];

			ascii=0xFF;
			if(code>>8==0x81)
				ascii = sjis_lookup_81[code & 0x00FF];
			else if(code>>8==0x82)
				ascii = sjis_lookup_82[code & 0x00FF];

			if(ascii!=0xFF){
				out[n]=ascii;
				n++;
			}
			else{
				//ASCII�ɕϊ��ł��Ȃ�����
				out[n]=code>>8&0xFF;
				out[n+1]=code&0xFF;
				n=n+2;
			}
		}
		else{
			out[n]=in[i];
			n++;
			i++;
		}
	}
}

//-------------------------------------------------
//psu�t�@�C������C���|�[�g
//�߂�l
//0�ȉ�:���s
//    0:mc0�ɃC���|�[�g����
//    1:mc1�ɃC���|�[�g����
int psuImport(const char *path, const FILEINFO *file, int outmc)
{
	//
	int ret = -1;	//�߂�l
	int n = 0;
	PSU_HEADER psu_header[MAX_ENTRY];
	PSU_HEADER psu_header_dir;
	char title[16*4+1]="";
	char *buff=NULL;

	int in_fd = -1, out_fd = -1;
	int hddin = FALSE;
	int i;

	int dialog_x;		//�_�C�A���Ox�ʒu
	int dialog_y;		//�_�C�A���Oy�ʒu
	int dialog_width;	//�_�C�A���O�� 
	int dialog_height;	//�_�C�A���O����

	//�t�H���_�̂Ƃ��́Apsu����C���|�[�g�ł��Ȃ�
	if(file->attr & MC_ATTR_SUBDIR){
		return -1;
	}

	//step1 psu�w�b�_�ǂݍ���
	{
		char inpath[MAX_PATH];	//�I�����ꂽ�t�H���_�܂��̓t�@�C���̃t���p�X
		char tmp[2048];		//�G�p
		char party[MAX_NAME];
		int r;
		int psuSize;
		int seek;
		int fileSize;	//�t�@�C���̃T�C�Y

		//psu�t�@�C����hdd�̂���Ƃ��p�X��ύX
		if(!strncmp(path, "hdd", 3)){
			hddin = TRUE;
			getHddParty(path, NULL, party, inpath);
			//pfs0�Ƀ}�E���g
			r = mountParty(party);
			if(r<0) return -2;
			inpath[3] = r+'0';
			//psu�t�@�C���̃t���p�X
			strcat(inpath, file->name);
		}
		else{
			//psu�t�@�C���̃t���p�X
			sprintf(inpath, "%s%s", path, file->name);
		}

		//psu�t�@�C���I�[�v���ƃT�C�Y�擾
		if(hddin){
			in_fd = fileXioOpen(inpath, O_RDONLY, fileMode);
			if(in_fd<0){
				ret=-3;
				goto error;
			}
			psuSize = fileXioLseek(in_fd, 0, SEEK_END);
			fileXioLseek(in_fd, 0, SEEK_SET);
		}
		else{
			in_fd = fioOpen(inpath, O_RDONLY);
			if(in_fd<0){
				ret=-3;
				goto error;
			}
			psuSize = fioLseek(in_fd, 0, SEEK_END);
			fioLseek(in_fd, 0, SEEK_SET);
		}

		//psu�w�b�_�ǂݍ���
		if(psuSize<sizeof(PSU_HEADER)){
			ret=-4;
			goto error;
		}
		//psu�w�b�_��ǂݍ���
		memset(&psu_header_dir, 0, sizeof(PSU_HEADER));
		if(hddin) fileXioRead(in_fd, (char*)&psu_header_dir, sizeof(PSU_HEADER));
		else fioRead(in_fd, &psu_header_dir, sizeof(PSU_HEADER));
		n = psu_header_dir.size;	//�t�@�C����
		seek = sizeof(PSU_HEADER);	//�t�@�C���̃V�[�N

		//psu_header[0]����ǂݍ���
		for(i=0;i<n;i++){
			if(psuSize<seek+sizeof(PSU_HEADER)){
				ret=-5;
				goto error;
			}
			memset(&psu_header[i], 0, sizeof(PSU_HEADER));
			if(hddin) fileXioRead(in_fd, (char*)&psu_header[i], sizeof(PSU_HEADER));
			else fioRead(in_fd, &psu_header[i], sizeof(PSU_HEADER));
			seek += sizeof(PSU_HEADER);
			//�Q�[���^�C�g��
			if(!strcmp(psu_header[i].name,"icon.sys")){
				if(hddin){
					fileXioLseek(in_fd, seek+0xC0, SEEK_SET);
					fileXioRead(in_fd, tmp, 64);
					title[64]=0;
					fileXioLseek(in_fd, seek, SEEK_SET);
				}
				else{
					fioLseek(in_fd, seek+0xC0, SEEK_SET);
					fioRead(in_fd, tmp, 64);
					title[64]=0;
					fioLseek(in_fd, seek, SEEK_SET);
				}
				sjis2ascii(tmp, title);
			}
			//���̃t�@�C���w�b�_�̈ʒu�ɃV�[�N
			if(psu_header[i].size>0){
				fileSize = (((psu_header[i].size-1)/0x400)+1)*0x400;
				if(psuSize<seek + fileSize){
					ret=-6;
					goto error;
				}
				seek += fileSize;
				if(hddin) fileXioLseek(in_fd, seek, SEEK_SET);
				else fioLseek(in_fd, seek, SEEK_SET);
			}
		}
		//psu�t�@�C���N���[�Y
		if(hddin){
			hddin = FALSE;
			fileXioClose(in_fd);
		}
		else
			fioClose(in_fd);
		in_fd = -1;
	}
	//step2 �C���|�[�g�J�n
	{
		char inpath[MAX_PATH];	//psu�t�@�C���̃t���p�X
		char outpath[MAX_PATH];//�Z�[�u�f�[�^�̃t�H���_���o�͂���t�H���_�̃t���p�X
		int seek;
		char tmp[2048];		//�G�p �\���p
		char out[MAX_PATH];	//�Z�[�u�f�[�^�̃t�H���_�̃t���p�X �Z�[�u�f�[�^�̃t�H���_�̒��̃t�@�C���̃t���p�X
		size_t outsize;
		char party[MAX_NAME];
		int r;
		mcTable mcDir __attribute__((aligned(64)));	//mcSetFileInfo()�p

		//�Z�[�u�f�[�^�̃t�H���_���o�͂���t�H���_�̃t���p�X
		if(outmc==0)
			strcpy(outpath, "mc0:/");
		else if(outmc==1)
			strcpy(outpath, "mc1:/");

		//psu�t�@�C��
		if(!strncmp(path, "hdd", 3)){
			hddin = TRUE;
			getHddParty(path, NULL, party, inpath);
			//pfs0�Ƀ}�E���g
			r = mountParty(party);
			if(r<0) return -8;
			inpath[3] = r+'0';
			//psu�t�@�C���̃t���p�X
			strcat(inpath, file->name);
		}
		else{
			//psu�t�@�C���̃t���p�X
			sprintf(inpath, "%s%s", path, file->name);
		}

		//�Z�[�u�f�[�^�̃t�H���_�쐬
		r = newdir(outpath, psu_header_dir.name);
		if(r == -17){	//�t�H���_�����łɂ���Ƃ��㏑�����m�F����
			drawDark();
			itoGsFinish();
			itoSwitchFrameBuffers();
			drawDark();
			sprintf(tmp, "%s%s/\n%s", outpath, psu_header_dir.name, lang->filer_overwrite);
			ret = MessageBox(tmp, LBF_VER, MB_YESNO);
			if(ret!=IDYES){
				ret = -7;
				goto error;
			}
		}
		else if(r < 0){//�t�H���_�쐬���s
			ret = -9;
			goto error;
		}

		//psu�t�@�C���I�[�v��
		if(hddin){
			in_fd = fileXioOpen(inpath, O_RDONLY, fileMode);
			if(in_fd<0){
				ret=-10;
				goto error;
			}
		}
		else{
			in_fd = fioOpen(inpath, O_RDONLY);
			if(in_fd<0){
				ret=-10;
				goto error;
			}
		}


		//
		dialog_width = FONT_WIDTH*40;
		dialog_height = FONT_HEIGHT*6;
		dialog_x = (SCREEN_WIDTH-dialog_width)/2;
		dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
		drawDark();
		itoGsFinish();
		itoSwitchFrameBuffers();
		drawDark();

		//
		seek = sizeof(PSU_HEADER);
		for(i=0;i<n;i++){
			//�_�C�A���O
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
			//���b�Z�[�W
			printXY(lang->filer_importing, dialog_x+FONT_WIDTH*0.5, dialog_y+FONT_HEIGHT*0.5, setting->color[COLOR_TEXT], TRUE);
			printXY(file->name, dialog_x+FONT_WIDTH*0.5, dialog_y+FONT_HEIGHT*1.5, setting->color[COLOR_TEXT], TRUE);
			sprintf(tmp, "%2d / %2d", i, n);
			printXY(tmp, dialog_x+FONT_WIDTH*0.5, dialog_y+FONT_HEIGHT*2.5, setting->color[COLOR_TEXT], TRUE);
			//�v���O���X�o�[�̘g
			drawDialogTmp(dialog_x+FONT_WIDTH*0.5, dialog_y+FONT_HEIGHT*3.5,
				dialog_x+dialog_width-FONT_WIDTH*0.5, dialog_y+FONT_HEIGHT*5.5,
				setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
			//�v���O���X�o�[
			itoSprite(setting->color[COLOR_FRAME],
				dialog_x+FONT_WIDTH, dialog_y+FONT_HEIGHT*4,
				dialog_x+FONT_WIDTH+(dialog_width-FONT_WIDTH)*(i*100/n)/100, dialog_y+FONT_HEIGHT*5, 0);
			drawScr();
			//
			seek += sizeof(PSU_HEADER);
			if(psu_header[i].attr&MC_ATTR_SUBDIR){
				//�t�H���_�̂Ƃ��͉������Ȃ�
			}
			else{
				//�������ރf�[�^�̃��������m��
				buff = (char*)malloc(psu_header[i].size);
				if(buff==NULL){
					ret=-11;
					goto error;
				}
				//�o�͂���t�@�C���I�[�v��
				sprintf(out, "%s%s/%s", outpath, psu_header_dir.name, psu_header[i].name);
				out_fd = fioOpen(out, O_WRONLY | O_TRUNC | O_CREAT);
				if(out_fd<0){
					ret=-12;
					goto error;
				}
				//�ǂݍ���
				memset(buff, 0, psu_header[i].size);
				if(hddin){
					fileXioLseek(in_fd, seek, SEEK_SET);
					fileXioRead(in_fd, buff, psu_header[i].size);
				}
				else{
					fioLseek(in_fd, seek, SEEK_SET);
					fioRead(in_fd, buff, psu_header[i].size);
				}
				//��������
				outsize = fioWrite(out_fd, buff, psu_header[i].size);
				if(outsize!=psu_header[i].size){
					ret=-13;
					goto error;
				}
				//�N���[�Y
				fioClose(out_fd); out_fd=-1;
				free(buff);
			
				{	//original source uLaunchELF 4.12
					memset(&mcDir, 0, sizeof(mcTable));
					memcpy(&mcDir._create, &psu_header[i].createtime, 8);
					memcpy(&mcDir._modify, &psu_header[i].modifytime, 8);
					mcDir.fileSizeByte = psu_header[i].size;
					mcDir.attrFile = psu_header[i].attr;
					strcpy(mcDir.name, psu_header[i].name);

					mcGetInfo(out[2]-'0', 0, NULL,NULL, NULL);	//Wakeup call
					mcSync(MC_WAIT, NULL, NULL);
					mcSetFileInfo(out[2]-'0', 0, &out[4], (char*)&mcDir, 0xFFFF);	//Fix file stats
					mcSync(MC_WAIT, NULL, NULL);
				}
				//
				fioLseek(in_fd, seek, SEEK_SET);	//�V�[�N���t�@�C���̐擪�ɂɖ߂�
				seek += (((psu_header[i].size-1)/0x400)+1)*0x400;
				fioLseek(in_fd, seek, SEEK_SET);	//�V�[�N�����̃t�@�C���w�b�_�̐擪�Ɉړ�
			}
		}
		//psu�t�@�C�����N���[�Y
		if(hddin) fileXioClose(in_fd);
		else fioClose(in_fd);
		in_fd=-1;

		//�t�H���_��mcSetFileInfo
		{
			memset(&mcDir, 0, sizeof(mcTable));
			memcpy(&mcDir._create, &psu_header_dir.createtime, 8);
			memcpy(&mcDir._modify, &psu_header_dir.modifytime, 8);
			mcDir.fileSizeByte = psu_header_dir.size;
			mcDir.attrFile = psu_header_dir.attr;
			strcpy(mcDir.name, psu_header_dir.name);

			//
			sprintf(out, "%s%s", outpath, psu_header_dir.name);
			mcGetInfo(out[2]-'0', 0, NULL, NULL, NULL);	//Wakeup call
			mcSync(MC_WAIT, NULL, NULL);
			mcSetFileInfo(out[2]-'0', 0, &out[4], (char*)&mcDir, 0xFFFF); //Fix file stats
			mcSync(MC_WAIT, NULL, NULL);
		}
	}
	//
	ret=outmc;
error:
	free(buff);
	if(in_fd>=0){
		if(hddin) fileXioClose(in_fd);
		else fioClose(in_fd);
	}
	if(out_fd>=0){
		fioClose(out_fd);
	}

	return ret;
}

//-------------------------------------------------
// psu�t�@�C���ɃG�N�X�|�[�g
int psuExport(const char *path, const FILEINFO *file, int sjisout)
{
	int ret = -1;	//�߂�l
	int n = 0;

	mcTable mcDir[MAX_ENTRY] __attribute__((aligned(64)));
	int mcret;
	int r;

	char outpath[MAX_PATH];	//�o�͂���psu�t�@�C����
	char *buff=NULL;
	int out_fd = -1;
	int in_fd = -1;
	int hddout = FALSE;

	int dialog_x;		//�_�C�A���Ox�ʒu
	int dialog_y;		//�_�C�A���Oy�ʒu
	int dialog_width;	//�_�C�A���O��
	int dialog_height;	//�_�C�A���O����

	//�t�@�C���̂Ƃ��́Apsu�ɃG�N�X�|�[�g�ł��Ȃ�
	if(!(file->attr & MC_ATTR_SUBDIR)){	//�t�@�C��
		return -1;
	}

	//step1 �G�N�X�|�[�g����Z�[�u�f�[�^�𒲂ׂ�
	{
		char inpath[MAX_PATH];		//�I�����ꂽ�t�H���_�̃t���p�X
		char Pattern[MAX_PATH];	//�񋓗p�p�^�[��

		//�I�����ꂽ�t�H���_�̃t���p�X
		sprintf(inpath, "%s%s", path, file->name);

		//���X�g�ǂݍ���
		sprintf(Pattern, "%s/*", &inpath[4]);
		mcSync(MC_WAIT, NULL, &mcret);
		mcGetDir(inpath[2]-'0', 0, Pattern, 0, MAX_ENTRY-2, mcDir);
		mcSync(MC_WAIT, NULL, &n);	//�t�@�C����
		//mcDir[0]�̏��
		mcDir[0].fileSizeByte=0;
		mcDir[0].attrFile=0x8427;
		strcpy(mcDir[0].name,".");
	}
	//step2
	{
		char inpath[MAX_PATH];		//�I�����ꂽ�t�H���_�̃t���p�X
		char party[MAX_NAME];
		int r;
		int i;
		size_t outsize;
		int readSize;
		int writeSize;
		unsigned char tmp[2048]="";	//�\���p
		int code;
		char tmppath[MAX_PATH];
		PSU_HEADER psu_header;

		//�I�����ꂽ�t�H���_�̃t���p�X
		sprintf(inpath, "%s%s", path, file->name);

		//�o�͂���t�H���_��
		if(setting->Exportdir[0])
			strcpy(outpath, setting->Exportdir);
		else
			strcpy(outpath, path);

		//�o�͂���psu�t�@�C����
		strcpy(tmp, file->name);
		if(sjisout){
			if(file->title[0]){
				//�t�@�C�����Ɏg���Ȃ�������ϊ�
				title2filename(file->title, tmp);
			}
		}

		//�o�͐悪mc�̂Ƃ��Ƀt�@�C�����̕������𒲂ׂ�
		if(!strncmp(outpath, "mc", 2)){
			//�t�@�C�����̍ő� mc:32byte mass:128byte hdd:256byte
			if(strlen(tmp)>28){	//�t�@�C�����������Ƃ��ɒZ������
				tmp[28] = 0;
				code=tmp[27];
				//sjis��1byte�ڂ����������
				if( (code>=0x81)&&(code<=0x9F) ) tmp[27] = 0;
				if( (code>=0xE0)&&(code<=0xFF) ) tmp[27] = 0;
			}
		}
		//�o�͂���psu�t�@�C���̃t���p�X
		strcat(outpath, tmp);
		strcat(outpath, ".psu");

		if(!strncmp(outpath, "mc", 2)){
			int type;
			//
			mcGetInfo(outpath[2]-'0', 0, &type, NULL, NULL);
			mcSync(MC_WAIT, NULL, NULL);
			if(type!=MC_TYPE_PS2){
				ret=-3;
				goto error;
			}
		}
		//�o�͂���psu�t�@�C����hdd�̂Ƃ��p�X��ύX
		else if(!strncmp(outpath, "hdd", 3)){
			if(nparties==0){
				loadHddModules();
				setPartyList();
			}
			hddout = TRUE;
			getHddParty(outpath, NULL, party, tmp);
			//pfs0�Ƀ}�E���g
			r = mountParty(party);
			if(r<0){
				ret=-4;
				goto error;
			}
			strcpy(outpath, tmp);
			outpath[3] = r+'0';
		}
		else if(!strncmp(outpath, "cdfs", 2)){
			ret=-5;
			goto error;
		}
		else if(!strncmp(outpath, "mass", 4))
			loadUsbModules();

		//psu�t�@�C���I�[�v�� �V�K�쐬
		if(hddout){
			// O_TRUNC �������Ȃ����߁A�I�[�v���O�Ƀt�@�C���폜
			fileXioRemove(outpath);
			out_fd = fileXioOpen(outpath, O_WRONLY|O_TRUNC|O_CREAT, fileMode);
			if(out_fd<0){
				ret=-6;
				goto error;
			}
		}
		else{	//mc mass
			out_fd = fioOpen(outpath, O_WRONLY | O_TRUNC | O_CREAT);
			if(out_fd<0){
				ret=-6;
				goto error;
			}
		}

		//psu�w�b�_��������
		memset(&psu_header, 0, sizeof(PSU_HEADER));
		psu_header.attr = file->attr;
		psu_header.size = n;
		memcpy(&psu_header.createtime, &file->createtime, 8);
		memcpy(&psu_header.modifytime, &file->modifytime, 8);
		strcpy(psu_header.name, file->name);
		if(hddout){
			outsize = fileXioWrite(out_fd, (char*)&psu_header, sizeof(PSU_HEADER));
			if(outsize!=sizeof(PSU_HEADER)){
				ret=-7;
				goto error;
			}
		}
		else{
			outsize = fioWrite(out_fd, &psu_header, sizeof(PSU_HEADER));
			if(outsize!=sizeof(PSU_HEADER)){
				ret=-7;
				goto error;
			}
		}

		//
		dialog_width = FONT_WIDTH*40;
		dialog_height = FONT_HEIGHT*6;
		dialog_x = (SCREEN_WIDTH-dialog_width)/2;
		dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
		drawDark();
		itoGsFinish();
		itoSwitchFrameBuffers();
		drawDark();

		//
		for(i=0;i<n;i++){
			//�_�C�A���O
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
			//���b�Z�[�W
			printXY(lang->filer_exporting, dialog_x+FONT_WIDTH*0.5, dialog_y+FONT_HEIGHT*0.5, setting->color[COLOR_TEXT], TRUE);
			printXY(file->name, dialog_x+FONT_WIDTH*0.5, dialog_y+FONT_HEIGHT*1.5, setting->color[COLOR_TEXT], TRUE);
			sprintf(tmp, "%2d / %2d", i, n);
			printXY(tmp, dialog_x+FONT_WIDTH*0.5, dialog_y+FONT_HEIGHT*2.5, setting->color[COLOR_TEXT], TRUE);
			//�v���O���X�o�[�̘g
			drawDialogTmp(dialog_x+FONT_WIDTH*0.5, dialog_y+FONT_HEIGHT*3.5,
				dialog_x+dialog_width-FONT_WIDTH*0.5, dialog_y+FONT_HEIGHT*5.5,
				setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
			//�v���O���X�o�[
			itoSprite(setting->color[COLOR_FRAME],
				dialog_x+FONT_WIDTH, dialog_y+FONT_HEIGHT*4,
				dialog_x+FONT_WIDTH+(dialog_width-FONT_WIDTH)*(i*100/n)/100, dialog_y+FONT_HEIGHT*5, 0);
			drawScr();
			//�t�@�C���w�b�_���쐬
			memset(&psu_header, 0, sizeof(PSU_HEADER));
			psu_header.attr = mcDir[i].attrFile;	//�t�@�C�������̓������[�J�[�h�Ɠ����ɂ���
			psu_header.size = mcDir[i].fileSizeByte;
			memcpy(&psu_header.createtime, &mcDir[i]._create, 8);
			memcpy(&psu_header.modifytime, &mcDir[i]._modify, 8);
			strncpy(psu_header.name, mcDir[i].name,32);
			//�t�@�C���w�b�_��������
			if(hddout){
				outsize = fileXioWrite(out_fd, (char*)&psu_header, sizeof(PSU_HEADER));
				if(outsize!=sizeof(PSU_HEADER)){
					ret=-8;
					goto error;
				}
			}
			else{
				outsize = fioWrite(out_fd, &psu_header, sizeof(PSU_HEADER));
				if(outsize!=sizeof(PSU_HEADER)){
					ret=-9;
					goto error;
				}
			}
			//�t�@�C����������
			if(mcDir[i].fileSizeByte>0){
				sprintf(tmppath, "%s/%s", inpath, mcDir[i].name);
				writeSize = (((mcDir[i].fileSizeByte-1)/0x400)+1)*0x400;
				buff = (char*)malloc(writeSize);
				if(buff==NULL){
					ret=-10;
					goto error;
				}
				memset(buff, 0, writeSize);
				//�t�@�C���I�[�v��
				in_fd = fioOpen(tmppath, O_RDONLY);
				if(in_fd<0){
					ret=-11;
					goto error;
				}
				//�ǂݍ���
				readSize = fioRead(in_fd, buff, mcDir[i].fileSizeByte);
				if(readSize!=mcDir[i].fileSizeByte){
					ret=-12;
					goto error;
				}
				//�N���[�Y
				fioClose(in_fd); in_fd=-1;
				//psu�t�@�C���ɏ�������
				if(hddout){
					outsize = fileXioWrite(out_fd, buff, writeSize);
					if(outsize!=writeSize){
						ret=-13;
						goto error;
					}
				}
				else{
					outsize = fioWrite(out_fd, buff, writeSize);
					if(outsize!=writeSize){
						ret=-13;
						goto error;
					}
				}
				free(buff);
			}
		}
	}
	//psu�t�@�C���N���[�Y
	if(hddout) fileXioClose(out_fd);
	else fioClose(out_fd);
	out_fd=-1;
	ret=0;
error:
	free(buff);
	if(in_fd>=0) fioClose(in_fd);
	if(out_fd>=0){
		if(hddout) fileXioClose(out_fd);
		else fioClose(out_fd);
	}

	if(ret<0){
		// �G�N�X�|�[�g���s�����Ƃ�psu�t�@�C�����폜
		if(!strncmp(outpath, "mc", 2)){
			mcDelete(outpath[2]-'0', 0, &outpath[4]);
			mcSync(MC_WAIT, NULL, &r);
		}
		else if(!strncmp(outpath, "pfs", 3)){
			r = fileXioRemove(outpath);
		}
		else if(!strncmp(outpath, "mass", 4)){
			r = fioRemove(outpath);
		}
	}
	return ret;
}

//-------------------------------------------------
// �X�N���[���L�[�{�[�h
/*
�� �g�p�s����
 : * " | < > \ / ?
�� ���C�A�E�g
A B C D E F G H I J K L M
N O P Q R S T U V W X Y Z
a b c d e f g h i j k l m
n o p q r s t u v w x y z
0 1 2 3 4 5 6 7 8 9      
( ) [ ] ! # $ % & @ ;    
= + - ' ^ . , _          
OK                  CANCEL
*/
int keyboard(char *out, int max)
{
	int	WFONTS,	//�L�[�{�[�h�̉��̕�����
		HFONTS,	//�L�[�{�[�h�̏c�̕�����
		KEY_W,	//�L�[�{�[�h�̉��̃T�C�Y
		KEY_H,	//�L�[�{�[�h�̏c�̃T�C�Y
		KEY_X,	//�L�[�{�[�h��x���W
		KEY_Y;	//�L�[�{�[�h��y���W
	char *KEY="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789   ()[]!#$%&@;  =+-'^.,_     ";
	int KEY_LEN;
	int cur=0, sel=0, i, x, y, t=0;
	char tmp[MAX_PATH];//, *p;

	WFONTS=13;
	HFONTS=7;
	KEY_W=(WFONTS*3+4)*FONT_WIDTH;
	KEY_H=(HFONTS+4.5)*FONT_HEIGHT;
	KEY_X=(SCREEN_WIDTH-KEY_W)/2;
	KEY_Y=(SCREEN_HEIGHT-KEY_H)/2;

/*
	//�L�����b�g���g���q�̑O�Ɉړ�
	p=strrchr(out, '.');
	if(p==NULL)
		cur=strlen(out);
	else
		cur=(int)(p-out);
*/
	//�L�����b�g�𕶎���̐擪�Ɉړ�
	cur=0;
	KEY_LEN = strlen(KEY);

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad & PAD_UP){
				if(sel<WFONTS*HFONTS){
					if(sel>=WFONTS) sel-=WFONTS;
				}
				else{
					if(sel==WFONTS*HFONTS) sel=82;	//�J�[�\����OK�ɂ���Ƃ��ɏ��������
					else sel=86;					//�J�[�\���KCANCEL�ɂ���Ƃ��ɏ��������
				}
			}
			else if(new_pad & PAD_DOWN){
				if(sel/WFONTS == HFONTS-1){
					if(sel%WFONTS < 6)		//�J�[�\�������S��荶�ɂ���Ƃ�OK�Ɉړ�
						sel=WFONTS*HFONTS;
					else					//�J�[�\�������S���E�ɂ���Ƃ�CANCEL�Ɉړ�
						sel=WFONTS*HFONTS+1;
				}else if(sel/WFONTS <= HFONTS-2)
					sel+=WFONTS;
			}
			else if(new_pad & PAD_LEFT){
				if(sel>0) sel--;
			}
			else if(new_pad & PAD_RIGHT){
				if(sel<=WFONTS*HFONTS) sel++;
			}
			else if(new_pad & PAD_START){
				sel = WFONTS*HFONTS;
			}
			else if(new_pad & PAD_SELECT){
				sel = WFONTS*HFONTS+1;
			}
			else if(new_pad & PAD_L1){
				if(cur>0) cur--;
				t=0;
			}
			else if(new_pad & PAD_R1){
				if(cur<strlen(out)) cur++;
				t=0;
			}
			else if(new_pad & PAD_CROSS){
				if(cur>0){
					strcpy(tmp, out);
					out[cur-1]=0;
					strcat(out, &tmp[cur]);
					cur--;
					t=0;
				}
			}
			else if(new_pad & PAD_CIRCLE){
				i=strlen(out);
				if(sel < WFONTS*HFONTS){
					if(i<max && i<33){
						strcpy(tmp, out);
						out[cur]=KEY[sel];
						out[cur+1]=0;
						strcat(out, &tmp[cur]);
						cur++;
						t=0;
					}
				}else if(sel == WFONTS*HFONTS && i>0){
					break;
				}else
					return -1;
			}
		}
		// �`��J�n
		drawDialogTmp(KEY_X, KEY_Y, KEY_X+KEY_W, KEY_Y+KEY_H, setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
		//�L�[�{�[�h�����̘g
		drawFrame(KEY_X+FONT_WIDTH, KEY_Y+FONT_HEIGHT*1.5,
			KEY_X+KEY_W-FONT_WIDTH, KEY_Y+FONT_HEIGHT*9.5, setting->color[COLOR_FRAME]);
		//���͒��̕�����̕\��
		printXY(out,
			KEY_X+FONT_WIDTH*2, KEY_Y+FONT_HEIGHT*0.5,
			setting->color[COLOR_TEXT], TRUE);
		t++;
		//�L�����b�g
		if(t<SCANRATE/2){
			printXY("|",
				KEY_X+FONT_WIDTH*0.5+(cur+1)*FONT_WIDTH,
				KEY_Y+FONT_HEIGHT*0.5,
				setting->color[COLOR_TEXT], TRUE);
		}
		else{
			if(t==SCANRATE) t=0;
		}

		//�J�[�\���\��
		//�A���t�@�u�����h�L��
		itoPrimAlphaBlending( TRUE );
		if(sel<WFONTS*HFONTS){	//OK��CANCEL�ȊO
			x = KEY_X+FONT_WIDTH*2 + (sel%WFONTS)*FONT_WIDTH*3;
			y = KEY_Y+FONT_HEIGHT*2 + (sel/WFONTS)*FONT_HEIGHT;
			itoSprite(setting->color[COLOR_HIGHLIGHTTEXT]|0x10000000,
				x, y-2,
				x+FONT_WIDTH*3, y+GetFontSize(ASCII_FONT_HEIGHT)+2, 0);
			drawFrame(x, y-2, x+FONT_WIDTH*3, y+GetFontSize(ASCII_FONT_HEIGHT)+2, setting->color[COLOR_HIGHLIGHTTEXT]);
		}
		else{
			if(sel==WFONTS*HFONTS)
				x = KEY_X+KEY_W/4;	//OK
			else
				x = KEY_X+KEY_W/2;	//CANCEL
			y = KEY_Y+FONT_HEIGHT*10;
			itoSprite(setting->color[COLOR_HIGHLIGHTTEXT]|0x10000000,
				x, y-2,
				x+KEY_W/4, y+GetFontSize(ASCII_FONT_HEIGHT)+2, 0);
			drawFrame(x, y-2, x+KEY_W/4, y+GetFontSize(ASCII_FONT_HEIGHT)+2, setting->color[COLOR_HIGHLIGHTTEXT]);
		}
		//�A���t�@�u�����h����
		itoPrimAlphaBlending(FALSE);

		//�L�[�{�[�h�\��
		for(i=0; i<KEY_LEN; i++){
			sprintf(tmp,"%c",KEY[i]);
			printXY(tmp,
				KEY_X+FONT_WIDTH*3 + (i%WFONTS)*FONT_WIDTH*3,
				KEY_Y+FONT_HEIGHT*2 + (i/WFONTS)*FONT_HEIGHT,
				setting->color[COLOR_TEXT], TRUE);
		}
		//OK�\��
		x=((KEY_W/4)-FONT_WIDTH*strlen(lang->gen_ok))/2;
		sprintf(tmp, "%s",lang->gen_ok);
		printXY(tmp, KEY_X+KEY_W/4+x, KEY_Y+FONT_HEIGHT*10, setting->color[COLOR_TEXT], TRUE);
		//CANCEL�\��
		x=((KEY_W/4)-FONT_WIDTH*strlen(lang->gen_cancel))/2;
		sprintf(tmp, "%s",lang->gen_cancel);
		printXY(tmp, KEY_X+KEY_W/2+x, KEY_Y+FONT_HEIGHT*10, setting->color[COLOR_TEXT], TRUE);
		// �������
		x = FONT_WIDTH*1;
		y = SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT;
		itoSprite(setting->color[COLOR_BACKGROUND],
			0, y,
			SCREEN_WIDTH, y+FONT_HEIGHT, 0);
		printXY(lang->filer_keyboard_hint, x, y, setting->color[COLOR_TEXT], TRUE);
		drawScr();
	}
	return 0;
}

//-------------------------------------------------
// �t�@�C�����X�g�ݒ�
int setFileList(const char *path, const char *ext, FILEINFO *files, int cnfmode)
{
	char *p;
	int nfiles, i, j, ret=0;
	char fullpath[MAX_PATH];

	int checkELFret;
	char party[MAX_NAME];
	char tmp[16*4+1];

	// �t�@�C�����X�g�ݒ�
	if(path[0]==0){
		for(i=0;i<5;i++){
			memset(&files[i].createtime, 0, sizeof(PS2TIME));
			memset(&files[i].modifytime, 0, sizeof(PS2TIME));
			files[i].fileSizeByte = 0;
			files[i].attr = MC_ATTR_SUBDIR;
			files[i].title[0] = 0;
			if(i==0){
				strcpy(files[i].name, "mc0:");
				files[i].type = TYPE_DEVICE_MC;
			}
			if(i==1){
				strcpy(files[i].name, "mc1:");
				files[i].type = TYPE_DEVICE_MC;
			}
			if(i==2){
				strcpy(files[i].name, "hdd0:");
				files[i].type = TYPE_DEVICE_HDD;
			}
			if(i==3){
				strcpy(files[i].name, "cdfs:");
				files[i].type = TYPE_DEVICE_CD;
			}
			if(i==4){
				strcpy(files[i].name, "mass:");
				files[i].type = TYPE_DEVICE_MASS;
			}
		}
		nfiles = 5;
		if(cnfmode==ELF_FILE){
			memset(&files[5].createtime, 0, sizeof(PS2TIME));
			memset(&files[5].modifytime, 0, sizeof(PS2TIME));
			files[5].fileSizeByte = 0;
			files[5].attr = MC_ATTR_SUBDIR;
			strcpy(files[5].name, "MISC");
			files[5].type = TYPE_MISC;
			files[5].title[0] = 0;
			nfiles = 6;
		}
	}
	else if(!strcmp(path, "MISC/")){
		for(i=0;i<8;i++){
			memset(&files[i].createtime, 0, sizeof(PS2TIME));
			memset(&files[i].modifytime, 0, sizeof(PS2TIME));
			files[i].fileSizeByte = 0;
			if(i==0)
				files[i].attr = MC_ATTR_SUBDIR;
			else
				files[i].attr = 0;
			if(i==0) strcpy(files[i].name, "..");
			if(i==1) strcpy(files[i].name, "FileBrowser");
			if(i==2) strcpy(files[i].name, "PS2Browser");
			if(i==3) strcpy(files[i].name, "PS2Disc");
			if(i==4) strcpy(files[i].name, "PS2Ftpd");
			if(i==5) strcpy(files[i].name, "PowerOff");
			if(i==6) strcpy(files[i].name, "INFO");
			if(i==7) strcpy(files[i].name, "CONFIG");
			files[i].title[0] = 0;
			if(i==0)
				files[i].type = TYPE_OTHER;
			else
				files[i].type = TYPE_MISC;
		}
		nfiles = 8;
	}
	else{
		//files[0]��������
		memset(&files[0].createtime, 0, sizeof(PS2TIME));
		memset(&files[0].modifytime, 0, sizeof(PS2TIME));
		files[0].fileSizeByte = 0;
		files[0].attr = MC_ATTR_SUBDIR;
		strcpy(files[0].name, "..");
		files[0].title[0] = 0;
		files[0].type=TYPE_OTHER;

		//�t�@�C�����X�g�ƃt�@�C�������擾
		nfiles = getDir(path, &files[1]) + 1;
		if(strcmp(ext, "*")){	//�t�@�C���}�X�N
			for(i=j=1; i<nfiles; i++){
				if(files[i].attr & MC_ATTR_SUBDIR)
					files[j++] = files[i];
				else{
					p = strrchr(files[i].name, '.');
					if(p!=NULL && !stricmp(ext,p+1))
						files[j++] = files[i];
				}
			}
			nfiles = j;
		}

		//�Q�[���^�C�g���ƃt�@�C���^�C�v���擾
		for(i=1; i<nfiles; i++){
			memset(tmp, 0, 65);
			//�Q�[���^�C�g���擾
			if(!strncmp(path, "cdfs", 4)){
				//cdfs
				if(setting->discPs2saveCheck){
					ret = getGameTitle(path, &files[i], tmp);
					if(ret<0) tmp[0]=0;
				}
				else{
					ret=-1;
					tmp[0]=0;
				}
			}
			else{
				//mc��hdd��mass
				ret = getGameTitle(path, &files[i], tmp);
				if(ret<0) tmp[0]=0;
			}
			//sjis�̉p�����ƋL����ASCII�ɕϊ�
			memset(files[i].title, 0, 65);
			sjis2ascii(tmp, files[i].title);

			//�^�C�v�擾
			if(files[i].attr & MC_ATTR_SUBDIR){	//�t�H���_
				if(ret<0)
					files[i].type=TYPE_DIR;
				else if(ret==0){
					files[i].type=TYPE_PS2SAVE;	//PS2SAVE
				}
				else if(ret==1)
					files[i].type=TYPE_PS1SAVE;	//PS1SAVE
			}
			else if(!(files[i].attr & MC_ATTR_SUBDIR)){	//�t�@�C��
				sprintf(fullpath, "%s%s", path, files[i].name);
				//ELF�w�b�_�𒲂ׂ�
				if(!strncmp(path, "cdfs", 4)){
					if(setting->discELFCheck){
						checkELFret = checkELFheader(fullpath);
						//mountedParty[0][0]=0;
						if(checkELFret==1)
							files[i].type=TYPE_ELF;
						else
							files[i].type=TYPE_FILE;
					}
					else{
						files[i].type=TYPE_FILE;
					}
				}
				else{
					checkELFret = checkELFheader(fullpath);
					//mountedParty[0][0]=0;
					if(checkELFret==1)
						files[i].type=TYPE_ELF;
					else
						files[i].type=TYPE_FILE;
					if(!strncmp(path, "hdd", 3)){
						//HDD�̂Ƃ��ă}�E���g
						mountedParty[0][0]=0;
						getHddParty(path, NULL, party, NULL);
						mountParty(party);
					}
				}
				//psu�t�@�C�������ׂ�
				if(files[i].type==TYPE_FILE){
					char *ext;
					ext = getExtension(files[i].name);
					if(ext!=NULL&&!stricmp(ext, ".psu"))
						files[i].type=TYPE_PSU;
				}
			}
		}
		//�\�[�g
		if(nfiles>1)
			sort(&files[1], 0, nfiles-2);
	}

	return nfiles;
}

//-------------------------------------------------
// �C�ӂ̃t�@�C���p�X��Ԃ�
void getFilePath(char *out, int cnfmode)
{
	char path[MAX_PATH], oldFolder[MAX_PATH],
		msg0[MAX_PATH], msg1[MAX_PATH],
		tmp[MAX_PATH], ext[8], *p;
	uint64 color;
	FILEINFO files[MAX_ENTRY];
	int nfiles=0, sel=0, top=0;
	int cd=TRUE, up=FALSE, pushed=TRUE;
	int x, y, y0, y1;
	int i, ret;//, fd;
	size_t size;
	int code;
	int detail=0;	//�ڍו\�� 0:�Ȃ� 1:�T�C�Y 2:�X�V����
	size_t freeSpace=0;
	int mcfreeSpace=0;
	int vfreeSpace=FALSE;	//�󂫗e�ʕ\���t���O
	int l2button=FALSE;
	int showdirsize=FALSE;	//�t�H���_�T�C�Y�\���t���O

	if(cnfmode==ANY_FILE)
		strcpy(ext, "*");
	else if(cnfmode==ELF_FILE)
		strcpy(ext, "elf");
	else if(cnfmode==DIR)
		strcpy(ext, "");
	else if(cnfmode==FNT_FILE)
		strcpy(ext, "fnt");

	strcpy(path, LastDir);
	mountedParty[0][0]=0;
	mountedParty[1][0]=0;
	clipPath[0] = 0;
	nclipFiles = 0;
	cut = 0;
	title=FALSE;

	title = setting->defaulttitle;
	detail = setting->defaultdetail;
#ifdef ENABLE_ICON
	loadIcon();
#endif

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			l2button=FALSE;
			if(paddata&PAD_L2) l2button=TRUE;
			if(l2button){
				if(new_pad & PAD_CIRCLE){
					detail++;
					if(detail==3) detail=0;
				}
				else if(new_pad & PAD_TRIANGLE){
					setting->fileicon = !setting->fileicon;
				}
				else if(new_pad & PAD_CROSS){
					setting->flickerControl = !setting->flickerControl;
				}
				else if(new_pad & PAD_SQUARE){
					if(!strncmp(path, "mc", 2)){
						for(i=1; i<nfiles; i++){
							//�t�H���_�̃T�C�Y�擾
							if(files[i].attr & MC_ATTR_SUBDIR){
								int s;
								s=getFileSize(path, &files[i]);
								if(s<0)
									files[i].fileSizeByte=0;
								else
									files[i].fileSizeByte=s;
							}
						}
						showdirsize=TRUE;
						detail=1;	//�ڍו\��
					}
				}
			}
			else{
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
					up=TRUE;
				else if(new_pad & PAD_CIRCLE){	//change dir
					if(files[sel].attr & MC_ATTR_SUBDIR){
						if(!strcmp(files[sel].name,".."))
							up=TRUE;
						else{
							strcat(path, files[sel].name);
							strcat(path, "/");
							cd=TRUE;
						}
					}
				}
				else if(new_pad & PAD_SELECT){	//�߂�
					break;
				}
				else if(new_pad & PAD_L1) {	// �^�C�g���\���؂�ւ�
					title = !title;
					//�\�[�g
					if(path[0]==0 || !strcmp(path,"hdd0:/") || !strcmp(path,"MISC/")){
					}
					else{
						if(nfiles>1){
							sort(&files[1], 0, nfiles-2);
							//sel=0;
							//top=0;
							nmarks = 0;
							memset(marks, 0, MAX_ENTRY);
						}
					}
				}
				else if(new_pad & PAD_R2){	//GETSIZE
					if(path[0]==0 || !strcmp(path,"hdd0:/") || !strcmp(path,"MISC/")){
					}
					else if(nmarks==0 && !strcmp(files[sel].name, "..")){
					}
					else{
						if(nmarks==0){
							drawMsg("SIZE =");
							size=getFileSize(path, &files[sel]);
						}
						else{
							drawMsg(lang->filer_checkingsize);
							for(i=size=0; i<nfiles; i++){
								if(marks[i])
									size+=getFileSize(path, &files[i]);
								if(size<0) size=-1;
							}
						}
						//
						if(size<0){
							strcpy(msg0, lang->filer_getsizefailed);
						}
						else{
							if(size >= 1024*1024)
								sprintf(msg0, "SIZE = %.1f MByte", (double)size/1024/1024);
							else if(size >= 1024)
								sprintf(msg0, "SIZE = %.1f KByte", (double)size/1024);
							else
								sprintf(msg0, "SIZE = %d Byte", size);
							//mc�̂Ƃ������\��
							if(!strncmp(path, "mc", 2) && nmarks==0){
								sprintf(tmp, " ATTR = %04X", files[sel].attr);
								strcat(msg0, tmp);
							}
						}
						pushed = FALSE;
					}
				}
				//ELF_FILE ELF�I����
				if(cnfmode==ELF_FILE){
					if(new_pad & PAD_CIRCLE) {	//�t�@�C��������
						if(!(files[sel].attr & MC_ATTR_SUBDIR)){
							char fullpath[MAX_PATH];
							int ret;
							sprintf(fullpath, "%s%s", path, files[sel].name);
							if(!strncmp(path, "MISC/", 5)){
								strcpy(out, fullpath);
								strcpy(LastDir, path);
								break;
							}
							ret = checkELFheader(fullpath);
							if(!strncmp(fullpath, "hdd", 3))
								mountedParty[0][0]=0;
							if(ret==1){
								//ELF�t�@�C���I��
								strcpy(out, fullpath);
								strcpy(LastDir, path);
								break;
							}
							else{
								//ELF�t�@�C���ł͂Ȃ��Ƃ�
								pushed=FALSE;
								sprintf(msg0, lang->filer_not_elf);
#ifdef ENABLE_PSB
								{
									char *extension;
									extension = getExtension(files[sel].name);
									if(extension!=NULL){
										if(!stricmp(extension, ".psb")){
											strcpy(out, fullpath);
											strcpy(LastDir, path);
											break;
										}
									}
								}
#endif
							}
						}
					}
					else if(new_pad & PAD_SQUARE) {	// �t�@�C���}�X�N�؂�ւ�
						if(!strcmp(ext,"*")) strcpy(ext, "elf");
						else				 strcpy(ext, "*");
						cd=TRUE;
					}
					else if(new_pad & PAD_CROSS){	//�߂�
						break;
					}
				}
				//FNT_FILE FNT�I����
				else if(cnfmode==FNT_FILE){
					if(new_pad & PAD_CIRCLE) {//FNT�t�@�C��������
						if(!(files[sel].attr & MC_ATTR_SUBDIR)){
							sprintf(out, "%s%s", path, files[sel].name);
							//�w�b�_�`�F�b�N
							if(checkFONTX2header(out)<0){
								pushed=FALSE;
								sprintf(msg0, lang->filer_not_fnt);
								out[0] = 0;
							}
							else{
								strcpy(LastDir, path);
								break;
							}
						}
					}
					else if(new_pad & PAD_SQUARE) {	// �t�@�C���}�X�N�؂�ւ�
						if(!strcmp(ext,"*")) strcpy(ext, "fnt");
						else				 strcpy(ext, "*");
						cd=TRUE;
					}
					else if(new_pad & PAD_CROSS){	//�߂�
						break;
					}
				}
				//DIR �f�B���N�g���I����
				else if(cnfmode==DIR){
					if(new_pad & PAD_START) {	//�f�B���N�g��������
						if( path[0]!=0 && strcmp(path, "hdd0:/")!=0 && strncmp(path, "cdfs", 4)!=0 ){
							strcpy(out, path);
							break;
						}
					}
					else if(new_pad & PAD_CROSS){	//�߂�
						break;
					}
				}
				//ANY_FILE	�t�@�C���[���[�h	���ׂẴt�@�C�����Ώ�
				else if(cnfmode==ANY_FILE){
					if(new_pad & PAD_CIRCLE) {
						if(!(files[sel].attr & MC_ATTR_SUBDIR)){	//�t�@�C��
							char fullpath[MAX_PATH];
							int ret;
							sprintf(fullpath, "%s%s", path, files[sel].name);
							ret = checkELFheader(fullpath);
							if(!strncmp(fullpath, "hdd", 3))
								mountedParty[0][0]=0;
							if(ret==1){
								//ELF�t�@�C���I��
								strcpy(out, fullpath);
								strcpy(LastDir, path);
								break;
							}
							else{
								//ELF�t�@�C���ł͂Ȃ��Ƃ�
								pushed=FALSE;
								sprintf(msg0, lang->filer_not_elf);
#ifdef ENABLE_PSB
								{
									char *extension;
									extension = getExtension(fullpath);
									if(extension!=NULL){
										if(!stricmp(extension, ".psb")){	//psb�t�@�C�������s
											int ynret;
											int psbret;
											ynret = MessageBox(lang->filer_execute_psb, LBF_VER, MB_YESNO);
											if(ynret==IDYES){
												psbret = psb(fullpath);
												if(psbret==0){
													pushed=TRUE;
													cd=TRUE;	//�󂫃X�y�[�X�Čv�Z
												}
												else if(psbret>0){
													sprintf(msg0, "error line no = %d", psbret);
												}
												else if(psbret<0){
													strcpy(msg0, "psb open error");
												}
											}
										}
									}
								}
#endif
							}
						}
					}
					else if(new_pad & PAD_R1){	// ���j���[
						drawDark();
						itoGsFinish();
						itoSwitchFrameBuffers();
						drawDark();

						//���j���[
						ret = menu(path, files[sel].name);

						if(ret==COPY || ret==CUT){	// �N���b�v�{�[�h�ɃR�s�[
							strcpy(clipPath, path);
							if(nmarks>0){
								for(i=nclipFiles=0; i<nfiles; i++)
									if(marks[i])
										clipFiles[nclipFiles++]=files[i];
							}
							else{
								clipFiles[0]=files[sel];
								nclipFiles = 1;
							}
							sprintf(msg0, lang->filer_copy_to_clip);
							pushed=FALSE;
							if(ret==CUT)	cut=TRUE;
							else			cut=FALSE;
						}
						else if(ret==DELETE){	// �f���[�g
							drawDark();
							itoGsFinish();
							itoSwitchFrameBuffers();
							drawDark();
							if(nmarks==0){
								if(title && files[sel].title[0])
									sprintf(tmp,"%s",files[sel].title);
								else{
									sprintf(tmp,"%s",files[sel].name);
									if(files[sel].attr & MC_ATTR_SUBDIR)
										strcat(tmp,"/");
								}
								strcat(tmp, "\n");
								strcat(tmp, lang->filer_delete);
							}
							else{
								strcpy(tmp, lang->filer_deletemarkfiles);
							}
							ret = MessageBox(tmp, LBF_VER, MB_YESNO);
							if(ret==IDYES){
								if(nmarks==0){
									strcpy(tmp, files[sel].name);
									if(files[sel].attr & MC_ATTR_SUBDIR) strcat(tmp,"/");
									strcat(tmp, " ");
									strcat(tmp, lang->filer_deleting);
									drawMsg(tmp);
									ret=delete(path, &files[sel]);
								}
								else{
									for(i=0; i<nfiles; i++){
										if(marks[i]){
											strcpy(tmp, files[i].name);
											if(files[i].attr & MC_ATTR_SUBDIR) strcat(tmp,"/");
											strcat(tmp, " ");
											strcat(tmp, lang->filer_deleting);
											drawMsg(tmp);
											ret=delete(path, &files[i]);
											if(ret<0) break;
										}
									}
								}
								if(ret>=0){
									cd=TRUE;	//�󂫃X�y�[�X�Čv�Z
								}
								else{
									strcpy(msg0, lang->filer_deletefailed);
									pushed = FALSE;
								}
							}
						}
						else if(ret==RENAME){	// ���l�[��
							drawDark();
							itoGsFinish();
							itoSwitchFrameBuffers();
							drawDark();
							strcpy(tmp, files[sel].name);
							if(keyboard(tmp, 36)>=0){
								if(Rename(path, &files[sel], tmp)<0){
									pushed=FALSE;
									strcpy(msg0, lang->filer_renamefailed);
								}
								else
									cd=TRUE;
							}
						}
						else if(ret==PASTE){	// �N���b�v�{�[�h����y�[�X�g
							drawMsg(lang->filer_pasting);
							ret=paste(path);
							if(ret < 0){
								strcpy(msg0, lang->filer_pastefailed);
								pushed = FALSE;
							}
							else{
								if(cut) nclipFiles=0;
							}
							cd=TRUE;
						}
						else if(ret==NEWDIR){	// �V�K�t�H���_�쐬
							tmp[0]=0;
							drawDark();
							itoGsFinish();
							itoSwitchFrameBuffers();
							drawDark();
							if(keyboard(tmp, 36)>=0){
								ret = newdir(path, tmp);
								if(ret == -17){
									strcpy(msg0, lang->filer_direxists);
									pushed=FALSE;
								}
								else if(ret < 0){
									strcpy(msg0, lang->filer_newdirfailed);
									pushed=FALSE;
								}
								else{
									strcat(path, tmp);
									strcat(path, "/");
									cd=TRUE;
								}
							}
						}
						else if(ret==GETSIZE){	// �T�C�Y�\��
							if(nmarks==0){
								drawMsg("SIZE =");
								size=getFileSize(path, &files[sel]);
							}
							else{
								drawMsg(lang->filer_checkingsize);
								for(i=size=0; i<nfiles; i++){
									if(marks[i])
										size+=getFileSize(path, &files[i]);
									if(size<0) size=-1;
								}
							}
							//
							if(size<0){
								strcpy(msg0, lang->filer_getsizefailed);
							}
							else{
								if(size >= 1024*1024)
									sprintf(msg0, "SIZE = %.1f MByte", (double)size/1024/1024);
								else if(size >= 1024)
									sprintf(msg0, "SIZE = %.1f KByte", (double)size/1024);
								else
									sprintf(msg0, "SIZE = %d Byte", size);
								//mc�̂Ƃ������\��
								if(!strncmp(path, "mc", 2) && nmarks==0){
									sprintf(tmp, " ATTR = %04X", files[sel].attr);
									strcat(msg0, tmp);
								}
							}
							pushed = FALSE;
						}
						else if(ret==EXPORT){	// psu�t�@�C���ɃG�N�X�|�[�g
							int sjisout = FALSE;
							drawDark();
							itoGsFinish();
							itoSwitchFrameBuffers();
							drawDark();

							if(nmarks==0){
								ret = MessageBox(lang->filer_export, LBF_VER, MB_YESNO|MB_USETRIANGLE);
							}
							else{
								ret = MessageBox(lang->filer_exportmarkfiles, LBF_VER, MB_YESNO|MB_USETRIANGLE);
							}
							if(ret==IDYES||ret==(IDYES|IDTRIANGLE)){//��������YES��I�������Ƃ�
								if(ret&IDTRIANGLE) sjisout = TRUE;	//����YES��I�������Ƃ��t�@�C������sjis�ŏo�͂���
								ret=0;
								if(nmarks==0){
									ret = psuExport(path, &files[sel], sjisout);
								}
								else{
									for(i=0; i<nfiles; i++){
										if(marks[i]){
											if(files[i].attr & MC_ATTR_SUBDIR){	//�t�H���_�̂Ƃ�
												ret = psuExport(path, &files[i], sjisout);
												if(ret<0) break;	//���f����
											}
										}
									}
								}
								//���U���g
								if(ret<0){
									sprintf(msg0, "%s %d", lang->filer_exportfailed, ret);
									pushed = FALSE;
								}
								else{
									if(setting->Exportdir[0])
										strcpy(tmp,setting->Exportdir);
									else
										strcpy(tmp,path);
									sprintf(msg0, "%s %s", lang->filer_exportto, tmp);
									pushed = FALSE;
									cd = TRUE;
								}
							}
						}
						else if(ret==IMPORT){	// psu�t�@�C������C���|�[�g
							drawDark();
							itoGsFinish();
							itoSwitchFrameBuffers();
							drawDark();
	
							if(nmarks==0){
								ret = MessageBox(lang->filer_import, LBF_VER, MB_MC0MC1CANCEL);
							}
							else{
								ret = MessageBox(lang->filer_importmarkfiles, LBF_VER, MB_MC0MC1CANCEL);
							}
							if(ret==IDMC0||ret==IDMC1){
								int outmc=0;
								if(ret==IDMC0) outmc=0;
								if(ret==IDMC1) outmc=1;
								if(nmarks==0){
									ret = psuImport(path, &files[sel], outmc);
								}
								else{
									for(i=0; i<nfiles; i++){
										if(marks[i]){
											if(!(files[i].attr & MC_ATTR_SUBDIR)){	//�t�@�C���̂Ƃ�
												ret = psuImport(path, &files[i], outmc);
												if(ret<0) break;	//���f����
											}
										}
									}
								}
								//���U���g
								if(ret<0){
									sprintf(msg0, "%s %d", lang->filer_importfailed, ret);
									pushed = FALSE;
								}
								else{
									if(outmc==0) strcpy(tmp,"mc0:/");
									if(outmc==1) strcpy(tmp,"mc1:/");
									sprintf(msg0, "%s %s", lang->filer_importto, tmp);
									pushed = FALSE;
									cd = TRUE;
								}
							}
						}
					}
					else if(new_pad & PAD_CROSS) {	// �}�[�N
						if(sel!=0 && path[0]!=0 && strcmp(path,"hdd0:/")){
							if(marks[sel]){
								marks[sel]=FALSE;
								nmarks--;
							}
							else{
								marks[sel]=TRUE;
								nmarks++;
							}
						}
						sel++;
					}
					else if(new_pad & PAD_SQUARE) {	// �}�[�N���]
						if(path[0]!=0 && strcmp(path,"hdd0:/")){
							for(i=1; i<nfiles; i++){
								if(marks[i]){
									marks[i]=FALSE;
									nmarks--;
								}
								else{
									marks[i]=TRUE;
									nmarks++;
								}
							}
						}
					}
				}
			}
		}
		// ��ʃt�H���_�ړ�
		if(up){
			if((p=strrchr(path, '/'))!=NULL)
				*p = 0;
			if((p=strrchr(path, '/'))!=NULL){
				p++;
				strcpy(oldFolder, p);
				*p = 0;
			}
			else{
				strcpy(oldFolder, path);
				path[0] = 0;
			}
			cd=TRUE;
		}
		//�t�@�C�����X�g���擾
		if(cd){
			nfiles = setFileList(path, ext, files, cnfmode);
			// �󂫗e�ʎ擾
			vfreeSpace=FALSE;	//�󂫗e�ʕ\���t���O
			if(cnfmode==ANY_FILE){
				if(!strncmp(path, "mc", 2)){
					int type;
					mcGetInfo(path[2]-'0', 0, &type, &mcfreeSpace, NULL);
					mcSync(MC_WAIT, NULL, NULL);
					if(type==MC_TYPE_PS2)	//ps2 mc
						freeSpace = mcfreeSpace*1024;
					else if(type==MC_TYPE_PSX||type==MC_TYPE_POCKET)	//ps1 mc
						freeSpace = mcfreeSpace*8192;
					else
						freeSpace = 0;
					vfreeSpace=TRUE;
				}
				else if(!strncmp(path,"hdd",3)&&strcmp(path,"hdd0:/")){
					freeSpace = fileXioDevctl("pfs0:",PFSCTL_GET_ZONE_FREE,NULL,0,NULL,0)*fileXioDevctl("pfs0:",PFSCTL_GET_ZONE_SIZE,NULL,0,NULL,0);
					vfreeSpace=TRUE;
				}
			}
			// �ϐ�������
			sel=0;
			top=0;
			if(up){
				for(i=0; i<nfiles; i++){
					if(!strcmp(oldFolder, files[i].name)){
						sel=i;
						top=sel-3;
						break;
					}
				}
			}
			nmarks = 0;
			memset(marks, 0, MAX_ENTRY);
			cd=FALSE;
			up=FALSE;
			showdirsize=FALSE;
		}

		//disc stop
		if(strncmp(path,"cdfs",4) && setting->discControl)
			CDVD_Stop();

		// �t�@�C�����X�g�\���p�ϐ��̐��K��
		if(top > nfiles-MAX_ROWS)	top=nfiles-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nfiles)		sel=nfiles-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// ��ʕ`��J�n
		clrScr(setting->color[COLOR_BACKGROUND]);
		// �t�@�C�����X�g
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nfiles)
				break;
			//�F�ƃJ�[�\���\��
			if(top+i == sel){
				color = setting->color[COLOR_HIGHLIGHTTEXT];
				printXY(">", x, y, color, TRUE);
			}
			else
				color = setting->color[COLOR_TEXT];
			//�}�[�N�\��
			if(marks[top+i]){
				printXY("*", x+FONT_WIDTH, y, setting->color[COLOR_TEXT], TRUE);
			}
			//�t�@�C�����X�g�\��
			if(title){
				if(files[top+i].title[0]!=0)
					strcpy(tmp,files[top+i].title);	//�Q�[���^�C�g��
				else
					strcpy(tmp,files[top+i].name);	//�t�@�C����
			}
			else
				strcpy(tmp,files[top+i].name);	//�t�@�C����

			//�t�H���_�̂Ƃ��X���b�V������
			if((files[top+i].attr & MC_ATTR_SUBDIR)&&(strcmp(files[top+i].name,"..")))
				strcat(tmp,"/");

			//�t�@�C�����������Ƃ��́A�Z������
			if(strlen(tmp)>MAX_ROWS_X&&MAX_ROWS_X>3){
				tmp[MAX_ROWS_X-3]=0;
				code=tmp[MAX_ROWS_X-4];
				if( (code>=0x81)&&(code<=0x9F) ) tmp[MAX_ROWS_X-4] = 0;
				if( (code>=0xE0)&&(code<=0xFF) ) tmp[MAX_ROWS_X-4] = 0;
				strcat(tmp, "...");
			}

			//�t�@�C������\��
			if(setting->fileicon){
#ifdef ENABLE_ICON
				if(files[top+i].type!=TYPE_OTHER){
					drawIcon(
						x+FONT_WIDTH*2, y,
						FONT_WIDTH*2, FONT_HEIGHT - GetFontMargin(LINE_MARGIN),
						files[top+i].type);
				}
#else
				if(files[top+i].type>=TYPE_FILE && files[top+i].type<TYPE_OTHER){
					uint64 iconcolor=0;
					//�A�C�R���̐F
					if(files[top+i].type==TYPE_FILE)
						iconcolor=setting->color[COLOR_FILE];
					else if(files[top+i].type==TYPE_ELF)
						iconcolor=setting->color[COLOR_ELF];
					else if(files[top+i].type==TYPE_DIR)
						iconcolor=setting->color[COLOR_DIR];
					else if(files[top+i].type==TYPE_PS2SAVE)
						iconcolor=setting->color[COLOR_PS2SAVE];
					else if(files[top+i].type==TYPE_PS1SAVE)
						iconcolor=setting->color[COLOR_PS1SAVE];
					else if(files[top+i].type==TYPE_PSU)
						iconcolor=setting->color[COLOR_PSU];
					//�A�C�R����\��
					itoSprite(iconcolor,
						x+FONT_WIDTH*2, y,
						x+FONT_WIDTH*2+FONT_WIDTH, y+(FONT_HEIGHT - GetFontMargin(LINE_MARGIN)), 0);
				}
#endif
				//�t�@�C�����\��
				printXY(tmp, x+FONT_WIDTH*4, y, color, TRUE);
			}
			else{
				//�t�@�C�����̂ݕ\��
				printXY(tmp, x+FONT_WIDTH*2, y, color, TRUE);
			}

			//�ڍו\��
			if(path[0]==0 || !strcmp(path,"hdd0:/") || !strcmp(path,"MISC/")){
				//�������Ȃ�
			}
			else{
				if(detail==1){	//�t�@�C���T�C�Y�\��
					int len;
					if(files[top+i].attr & MC_ATTR_SUBDIR && showdirsize==FALSE){
						sprintf(tmp,"<DIR>");
					}
					else{
						if(files[top+i].fileSizeByte >= 1024*1024)
							sprintf(tmp, "%.1f MB", (double)files[top+i].fileSizeByte/1024/1024);
						else if(files[top+i].fileSizeByte >= 1024)
							sprintf(tmp, "%.1f KB", (double)files[top+i].fileSizeByte/1024);
						else
							sprintf(tmp,"%d B ",files[top+i].fileSizeByte);
					}
					len=strlen(tmp);
					if(strcmp(files[top+i].name,"..")){
						itoSprite(setting->color[COLOR_BACKGROUND],
							(MAX_ROWS_X-3)*FONT_WIDTH, y,
							(MAX_ROWS_X+8)*FONT_WIDTH, y+FONT_HEIGHT, 0);
						itoLine(setting->color[COLOR_FRAME], (MAX_ROWS_X-2.5)*FONT_WIDTH, y, 0,
							setting->color[COLOR_FRAME], (MAX_ROWS_X-2.5)*FONT_WIDTH, y+FONT_HEIGHT, 0);	
						printXY(tmp, FONT_WIDTH*(MAX_ROWS_X+7-len), y, color, TRUE);
					}
				}
				else if(detail==2){	//�X�V�����\��
					int len;
					//cdfs�́A�X�V�������擾�ł��Ȃ�
					if(!strncmp(path,"cdfs",4)){
						strcpy(tmp,"----/--/-- --:--:--");
					}
					else{
						sprintf(tmp,"%04d/%02d/%02d %02d:%02d:%02d",
							files[top+i].modifytime.year,
							files[top+i].modifytime.month,
							files[top+i].modifytime.day,
							files[top+i].modifytime.hour,
							files[top+i].modifytime.min,
							files[top+i].modifytime.sec);
					}
					len=strlen(tmp);
					if(strcmp(files[top+i].name,"..")){
						itoSprite(setting->color[COLOR_BACKGROUND],
							(MAX_ROWS_X-13)*FONT_WIDTH, y,
							(MAX_ROWS_X+8)*FONT_WIDTH, y+FONT_HEIGHT, 0);
						itoLine(setting->color[COLOR_FRAME], (MAX_ROWS_X-12.5)*FONT_WIDTH, y, 0,
							setting->color[COLOR_FRAME], (MAX_ROWS_X-12.5)*FONT_WIDTH, y+FONT_HEIGHT, 0);	
						printXY(tmp, FONT_WIDTH*(MAX_ROWS_X+7-len), y, color, TRUE);
					}
				}
			}
			y += FONT_HEIGHT;
		}
		// �X�N���[���o�[
		if(nfiles > MAX_ROWS){
			drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
				(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nfiles);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nfiles);
			itoSprite(setting->color[COLOR_FRAME],
				(MAX_ROWS_X+8)*FONT_WIDTH,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				(MAX_ROWS_X+9)*FONT_WIDTH,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		//
		if(l2button){
			//
			int dialog_x;		//�_�C�A���Ox�ʒu
			int dialog_y;		//�_�C�A���Oy�ʒu
			int dialog_width;	//�_�C�A���O��
			int dialog_height;	//�_�C�A���O����

			dialog_width = FONT_WIDTH*28;
			dialog_height = FONT_HEIGHT*5;
			dialog_x = (SCREEN_WIDTH-dialog_width)/2;
			dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
			// �`��J�n
			drawDark();
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
			//
			x = dialog_x+FONT_WIDTH*1;
			y = dialog_y+FONT_HEIGHT*0.5;
			//
			sprintf(tmp, "��:%s", lang->filer_l2popup_detail);
			printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
			sprintf(tmp, "��:%s", lang->filer_l2popup_icon);
			printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
			sprintf(tmp, "�~:%s", lang->filer_l2popup_flicker);
			printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
			sprintf(tmp, "��:%s", lang->filer_l2popup_dirsize);
			if(!strncmp(path, "mc", 2)){
				printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
			}
			else{
				printXY(tmp, x, y, setting->color[COLOR_GRAYTEXT], TRUE); y+=FONT_HEIGHT;
			}
		}
		// ���b�Z�[�W
		if(pushed) sprintf(msg0, "Path: %s", path);
		// �������
		if(l2button){
			//sprintf(msg1, "");
			msg1[0]='\0';
		}
		else{
			if(cnfmode==ANY_FILE){
				if(title)
					sprintf(msg1, lang->filer_anyfile_hint1);
				else
					sprintf(msg1, lang->filer_anyfile_hint2);
			}
			else if(cnfmode==ELF_FILE){
				if(!strcmp(ext, "*"))
					sprintf(msg1, lang->filer_elffile_hint1);
				else
					sprintf(msg1, lang->filer_elffile_hint2);
			}
			else if(cnfmode==FNT_FILE){
				if(!strcmp(ext, "*"))
					sprintf(msg1, lang->filer_fntfile_hint1);
				else
					sprintf(msg1, lang->filer_fntfile_hint2);
			}
			else if(cnfmode==DIR){
				sprintf(msg1, lang->filer_dir_hint);
			}
		}
		setScrTmp(msg0, msg1);

		// �t���[�X�y�[�X�\��
		if(vfreeSpace){
			if(freeSpace >= 1024*1024)
				sprintf(tmp, "%.1fMB free", (double)freeSpace/1024/1024);
			else if(freeSpace >= 1024)
				sprintf(tmp, "%.1fKB free", (double)freeSpace/1024);
			else
				sprintf(tmp, "%dB free", freeSpace);
			ret=strlen(tmp);
			//
			printXY(tmp,
				(MAX_ROWS_X+10-ret)*FONT_WIDTH, SCREEN_MARGIN,
				setting->color[COLOR_TEXT], TRUE);
		}
		drawScr();
	}
	
	if(mountedParty[0][0]!=0){
		fileXioUmount("pfs0:");
		mountedParty[0][0]=0;
	}
	if(mountedParty[1][0]!=0){
		fileXioUmount("pfs1:");
		mountedParty[1][0]=0;
	}
	return;
}
