#include "launchelf.h"

//PS2TIME LaunchELF 3.80
typedef struct
{
	unsigned char unknown;
	unsigned char sec;      // date/time (second)
	unsigned char min;      // date/time (minute)
	unsigned char hour;     // date/time (hour)
	unsigned char day;      // date/time (day)
	unsigned char month;    // date/time (month)
	unsigned short year;    // date/time (year)
} PS2TIME __attribute__((aligned (2)));

typedef struct{
	PS2TIME modifyTime;
    unsigned fileSizeByte;
	unsigned short attr;
	char title[16*4+1];
	char name[256];
	int type;
} FILEINFO;

// psu�t�@�C���w�b�_�\����
typedef struct { // 512 bytes
	unsigned short  attr;
	unsigned short  unknown1;
	unsigned int  size;	//file size, 0 for directory
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
	TYPE_OTHER,
	TYPE_DIR,
	TYPE_FILE,
	TYPE_PS2SAVE,
	TYPE_ELF
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
  ' ', ',', '.', ',', '.', 0xFF,':', ';', '?', '!', 0xFF,0xFF,'�', '`', 0xFF,'^',   // 0x40
  0xFF,'_', 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,'0', 0xFF,'-', '-', 0xFF,0xFF,  // 0x50
  0xFF,0xFF,0xFF,0xFF,0xFF,'\'','\'','"', '"', '(', ')', 0xFF,0xFF,'[', ']', '{',   // 0x60
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,'+', '-', 0xFF,'*', 0xFF,  // 0x70
  '/', '=', 0xFF,'<', '>', 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,'�', 0xFF,0xFF,'�', 0xFF,  // 0x80
  '$', 0xFF,0xFF,'%', '#', '&', '*', '@', 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x90
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xA0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xB0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,'&', '|', '!', 0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xC0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xD0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xE0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xF0
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
int fileMode =  FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH;

#define MAX_ARGC 3
int psb_argc;
char psb_argv[MAX_ARGC][MAX_PATH+2];

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
	
	strcpy(fullpath, path);
	if(file!=NULL){
		strcat(fullpath, file->name);
		if(file->attr & FIO_S_IFDIR) strcat(fullpath,"/");
	}
	if((p=strchr(&fullpath[6], '/'))==NULL) return -1;
	if(dir!=NULL) sprintf(dir, "pfs0:%s", p);
	*p=0;
	if(party!=NULL) sprintf(party, "hdd0:%s", &fullpath[6]);
	
	return 0;
}

//-------------------------------------------------
// �p�[�e�B�V�����̃}�E���g
int mountParty(const char *party)
{
	if(!strcmp(party, mountedParty[0]))
		return 0;
	else if(!strcmp(party, mountedParty[1]))
		return 1;
	
	fileXioUmount("pfs0:"); mountedParty[0][0]=0;
	if(fileXioMount("pfs0:", party, FIO_MT_RDWR) < 0) return -1;
	strcpy(mountedParty[0], party);
	return 0;
}

//-------------------------------------------------
// �m�F�_�C�A���O
int ynDialog(const char *message, int defaultsel)
{
	char msg[2048];
	int dh, dw, dx, dy;
	int sel=0, n, tw;//, a=6, b=4, c=2;
	int i, x, len, ret;
	int x_margin;
	int y_margin;
	char tmp[MAX_PATH];

	sel=defaultsel;
	strcpy(msg, message);

	//\n��؂��\0��؂�ɕϊ� n:���s�̐�
	for(i=0,n=1; msg[i]!=0; i++){
		if(msg[i]=='\n'){
			msg[i]=0;
			n++;
		}
	}
	//�\�����镶����̍ő�̕��𒲂ׂ� tw:��
	for(i=len=tw=0; i<n; i++){
		ret = printXY(&msg[len], 0, 0, 0, FALSE);	//�\�����镶����̕��𒲂ׂ�
		if(ret>tw) tw=ret;
		len=strlen(&msg[len])+1;
	}

	if(tw<FONT_WIDTH*22) tw=FONT_WIDTH*22;	//���̍ŏ��l

	x_margin = FONT_WIDTH;				//���E�̃}�[�W��
	y_margin = FONT_HEIGHT/2;			//�㉺�̃}�[�W��
	dw = tw+x_margin*2;				//�_�C�A���O�̕�
	dh = FONT_HEIGHT*(n+2)+y_margin*2;	//�_�C�A���O�̍���
	dx = (SCREEN_WIDTH-dw)/2;			//�_�C�A���O��x
	dy = (SCREEN_HEIGHT-dh)/2;			//�_�C�A���O��y
	
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad & PAD_LEFT){
				sel=0;	//OK
			}else if(new_pad & PAD_RIGHT){
				sel=1;	//CANCEL
			}else if(new_pad & PAD_CROSS){
				ret=-1;
				break;
			}else if(new_pad & PAD_CIRCLE){
				if(sel==0) ret=1;
				else	   ret=-1;
				break;
			}
			else if(new_pad & PAD_SELECT){
				sel=1;	//CANCEL
			}
			else if(new_pad & PAD_START){
				sel=0;	//OK
			}
		}
		//�`��J�n
		//���b�Z�[�W����
		itoSprite(setting->color[0],
			0, SCREEN_MARGIN+FONT_HEIGHT,
			SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2, 0);
		//�w�i
		drawDialogTmp(dx, dy,
			dx+dw, dy+dh,
			setting->color[0], setting->color[1]);
		//���b�Z�[�W
		for(i=len=0; i<n; i++){
			printXY(&msg[len], dx+x_margin, (dy+y_margin+i*FONT_HEIGHT), setting->color[3],TRUE);
			len=strlen(&msg[len])+1;
		}
		//OK��CANCEL
		x=(tw-FONT_WIDTH*22)/2;
		sprintf(tmp, " %-10s %-10s", lang->gen_ok, lang->gen_cancel);
		printXY(tmp, dx+x_margin+x, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3],TRUE);
		//�J�[�\��
		if(sel==0)
			printXY(">", dx+x_margin+x, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3], TRUE);
		else
			printXY(">",dx+x_margin+x+FONT_WIDTH*11, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3], TRUE);
		drawScr();
	}
/*
	//
	x=(tw-FONT_WIDTH*12)/2;
	drawChar(' ', dx+x_margin+x, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3]);
	drawChar(' ',dx+x_margin+x+FONT_WIDTH*11, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3]);
*/
	return ret;
}

//-------------------------------------------------
// ���b�Z�[�W�_�C�A���O
void MessageDialog(const char *message)
{
	char msg[2048];
	int dh, dw, dx, dy;
	int n, tw;
	int i, x, len, ret;
	int x_margin;
	int y_margin;
	char tmp[MAX_PATH];

	strcpy(msg, message);

	//\n��؂��\0��؂�ɕϊ� n:���s�̐�
	for(i=0,n=1; msg[i]!=0; i++){
		if(msg[i]=='\n'){
			msg[i]=0;
			n++;
		}
	}
	//�\�����镶����̍ő�̕��𒲂ׂ� tw:��
	for(i=len=tw=0; i<n; i++){
		ret = printXY(&msg[len], 0, 0, 0, FALSE);	//�\�����镶����̕��𒲂ׂ�
		if(ret>tw) tw=ret;
		len=strlen(&msg[len])+1;
	}

	if(tw<FONT_WIDTH*10) tw=FONT_WIDTH*10;	//���̍ŏ��l

	x_margin = FONT_WIDTH;				//���E�̃}�[�W��
	y_margin = FONT_HEIGHT/2;			//�㉺�̃}�[�W��
	dw = tw+x_margin*2;				//�_�C�A���O�̕�
	dh = FONT_HEIGHT*(n+2)+y_margin*2;	//�_�C�A���O�̍���
	dx = (SCREEN_WIDTH-dw)/2;			//�_�C�A���O��x
	dy = (SCREEN_HEIGHT-dh)/2;			//�_�C�A���O��y
	
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad & PAD_CIRCLE){
				break;
			}
		}
		//�`��J�n
		//�w�i
		drawDialogTmp(dx, dy,
			dx+dw, dy+dh,
			setting->color[0], setting->color[1]);
		//���b�Z�[�W
		for(i=len=0; i<n; i++){
			printXY(&msg[len], dx+x_margin, (dy+y_margin+i*FONT_HEIGHT), setting->color[3],TRUE);
			len=strlen(&msg[len])+1;
		}
		x=(tw-FONT_WIDTH*10)/2;
		//OK
		sprintf(tmp, "��: %s", lang->gen_ok);
		printXY(tmp, dx+x_margin+x, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3],TRUE);
		drawScr();
	}
	return;
}

//-------------------------------------------------
// �N�C�b�N�\�[�g
int cmpFile(FILEINFO *a, FILEINFO *b)
{
	unsigned char *p, ca, cb;
	int i, n, ret, aElf=FALSE, bElf=FALSE, t=title;
	
	if(a->attr==b->attr){
		if(a->attr & FIO_S_IFREG){
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
		else  n=strlen(a->name);
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
	
	if(a->attr & FIO_S_IFDIR)	return -1;
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
	
	mcSync(0,NULL,NULL);
	
	strcpy(dir, &path[4]); strcat(dir, "*");
	mcGetDir(path[2]-'0', 0, dir, 0, MAX_ENTRY-2, mcDir);
	mcSync(0, NULL, &ret);
	
	for(i=j=0; i<ret; i++)
	{
		if(mcDir[i].attrFile & MC_ATTR_SUBDIR &&
		(!strcmp(mcDir[i].name, ".") || !strcmp(mcDir[i].name, "..")))
			continue;
		strcpy(info[j].name, mcDir[i].name);
		if(mcDir[i].attrFile & MC_ATTR_SUBDIR)
			info[j].attr = FIO_S_IFDIR;
		else
			info[j].attr = FIO_S_IFREG;
		info[j].fileSizeByte = mcDir[i].fileSizeByte;
		info[j].modifyTime.unknown = mcDir[i]._modify.unknown2;
		info[j].modifyTime.sec = mcDir[i]._modify.sec;
		info[j].modifyTime.min = mcDir[i]._modify.min;
		info[j].modifyTime.hour = mcDir[i]._modify.hour;
		info[j].modifyTime.day = mcDir[i]._modify.day;
		info[j].modifyTime.month = mcDir[i]._modify.month;
		info[j].modifyTime.year = mcDir[i]._modify.year;
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
		if(TocEntryList[i].fileProperties & 0x02 &&
		 (!strcmp(TocEntryList[i].filename, ".") ||
		  !strcmp(TocEntryList[i].filename, "..")))
			continue;
		strcpy(info[j].name, TocEntryList[i].filename);
		if(TocEntryList[i].fileProperties & 0x02)
			info[j].attr = FIO_S_IFDIR;
		else
			info[j].attr = FIO_S_IFREG;
		info[j].fileSizeByte = TocEntryList[i].fileSize;
		memset(&info[j].modifyTime, 0, sizeof(PS2TIME)); //�擾�ł��Ȃ�
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
			info[i].attr = FIO_S_IFDIR;
			info[i].fileSizeByte = 0;
			memset(&info[i].modifyTime, 0, sizeof(PS2TIME));
		}
		return nparties;
	}
	
	getHddParty(path, NULL, party, dir);
	ret = mountParty(party);
	if(ret<0) return 0;
	dir[3] = ret+'0';
	
	if((fd=fileXioDopen(dir)) < 0) return 0;
	
	while(fileXioDread(fd, &dirbuf)){
		if(dirbuf.stat.mode & FIO_S_IFDIR &&
		(!strcmp(dirbuf.name, ".") || !strcmp(dirbuf.name, "..")))
			continue;
		
		info[i].attr = dirbuf.stat.mode;
		strcpy(info[i].name, dirbuf.name);
		info[i].fileSizeByte = dirbuf.stat.size;
		info[i].modifyTime.unknown = dirbuf.stat.mtime[0];
		info[i].modifyTime.sec = dirbuf.stat.mtime[1];
		info[i].modifyTime.min = dirbuf.stat.mtime[2];
		info[i].modifyTime.hour = dirbuf.stat.mtime[3];
		info[i].modifyTime.day = dirbuf.stat.mtime[4];
		info[i].modifyTime.month = dirbuf.stat.mtime[5];
		info[i].modifyTime.year = dirbuf.stat.mtime[6]+dirbuf.stat.mtime[7]*256;
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
		if((FIO_SO_ISDIR(record.stat.mode))
			&& (!strcmp(record.name,".") || !strcmp(record.name,".."))
		) continue;

		strcpy(info[n].name, record.name);
		if(FIO_SO_ISDIR(record.stat.mode)){
			info[n].attr = FIO_S_IFDIR;
		}
		else if(FIO_SO_ISREG(record.stat.mode)){
			info[n].attr = FIO_S_IFREG;
			info[n].fileSizeByte = record.stat.size;
		}
		else
			continue;
		strncpy(info[n].name, info[n].name, 32);
		info[n].modifyTime.unknown = 0;
		info[n].modifyTime.sec = record.stat.mtime[1];
		info[n].modifyTime.min = record.stat.mtime[2];
		info[n].modifyTime.hour = record.stat.mtime[3];
		info[n].modifyTime.day = record.stat.mtime[4];
		info[n].modifyTime.month = record.stat.mtime[5];
		info[n].modifyTime.year = record.stat.mtime[6] + record.stat.mtime[7]*256;
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
// �Z�[�u�f�[�^�^�C�g���̎擾
int getGameTitle(const char *path, const FILEINFO *file, char *out)
{
	iox_dirent_t dirEnt;
	char party[MAX_NAME], dir[MAX_PATH];
	int fd=-1, dirfd=-1, size, hddin=FALSE, ret;
	
	if(file->attr & FIO_S_IFREG) return -1;
	if(path[0]==0 || !strcmp(path, "hdd0:/")) return -1;
	
	if(!strncmp(path, "hdd", 3)){
		getHddParty(path, file, party, dir);
		ret = mountParty(party);
		if(ret<0) return -1;
		dir[3]=ret+'0';
		hddin=TRUE;
	}else
		sprintf(dir, "%s%s/", path, file->name);
	
	ret = -1;
	if(hddin){
		if((dirfd=fileXioDopen(dir)) < 0) goto error;
		while(fileXioDread(dirfd, &dirEnt)){
			if(dirEnt.stat.mode & FIO_S_IFREG &&
			 !strcmp(dirEnt.name, "icon.sys")){
				strcat(dir, "icon.sys");
				if((fd=fileXioOpen(dir, O_RDONLY, fileMode)) < 0)
					goto error;
				if((size=fileXioLseek(fd, 0, SEEK_END)) <= 0x100)
					goto error;
				fileXioLseek(fd, 0xC0, SEEK_SET);
				fileXioRead(fd, out, 16*4);
				out[16*4] = 0;
				fileXioClose(fd); fd=-1;
				ret=0;
				break;
			}
		}
		fileXioDclose(dirfd); dirfd=-1;
	}
	else{
		strcat(dir, "icon.sys");
		if((fd=fioOpen(dir, O_RDONLY)) < 0) goto error;
		if((size=fioLseek(fd, 0, SEEK_END)) <= 0x100) goto error;
		fioLseek(fd, 0xC0, SEEK_SET);
		fioRead(fd, out, 16*4);
		out[16*4] = 0;
		fioClose(fd); fd=-1;
		ret=0;
	}
error:
	if(fd>=0){
		if(hddin) fileXioClose(fd);
		else	  fioClose(fd);
	}
	if(dirfd>=0) fileXioDclose(dirfd);
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
		enable[EXPORT] = FALSE;
		enable[IMPORT] = FALSE;
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
			else if(new_pad & PAD_SQUARE){
				if(enable[COPY]){
					sel=COPY;
					break;
				}
			}
			else if(new_pad & PAD_TRIANGLE){
				if(enable[PASTE]){
					sel=PASTE;
					break;
				}
			}
		}
		
		// �`��J�n
		drawDialogTmp(menu_x, menu_y, menu_x+menu_w, menu_y+menu_h, setting->color[0], setting->color[1]);
		for(i=0,y=74; i<NUM_MENU; i++){
			if(i==COPY)          sprintf(tmp, "%s(��)", lang->filer_menu_copy);
			else if(i==CUT)     strcpy(tmp, lang->filer_menu_cut);
			else if(i==PASTE)   sprintf(tmp, "%s(��)", lang->filer_menu_paste);
			else if(i==DELETE)  strcpy(tmp, lang->filer_menu_delete);
			else if(i==RENAME)  strcpy(tmp, lang->filer_menu_rename);
			else if(i==NEWDIR)  strcpy(tmp, lang->filer_menu_newdir);
			else if(i==GETSIZE) strcpy(tmp, lang->filer_menu_getsize);
			else if(i==EXPORT)  strcpy(tmp, lang->filer_menu_exportpsu);
			else if(i==IMPORT)  strcpy(tmp, lang->filer_menu_importpsu);

			if(enable[i])
				color = setting->color[2];
			else
				color = setting->color[3];
			
			printXY(tmp, menu_x+FONT_WIDTH*2, menu_y+FONT_HEIGHT/2+i*FONT_HEIGHT, color, TRUE);
			y+=FONT_HEIGHT;
		}
		if(sel<NUM_MENU)
			printXY(">", menu_x+FONT_WIDTH, menu_y+FONT_HEIGHT/2+sel*FONT_HEIGHT, setting->color[2], TRUE);

		// �������
		x = FONT_WIDTH*2;
		y = SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT;
		itoSprite(setting->color[0],
			0, y,
			SCREEN_WIDTH, y+FONT_HEIGHT, 0);
		sprintf(tmp,"��:%s �~:%s", lang->gen_ok, lang->gen_cancel);
		printXY(tmp, x, y, setting->color[3], TRUE);
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
	
	if(file->attr & FIO_S_IFDIR){
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
	
	if(file->attr & FIO_S_IFDIR){
		strcat(dir,"/");
		// �Ώۃt�H���_���̑S�t�@�C���E�t�H���_���폜
		nfiles = getDir(dir, files);
		for(i=0; i<nfiles; i++){
			ret=delete(dir, &files[i]);
			if(ret < 0) return -1;
		}
		// �Ώۃt�H���_���폜
		if(!strncmp(dir, "mc", 2)){
			mcSync(0,NULL,NULL);
			mcDelete(dir[2]-'0', 0, &dir[4]);
			mcSync(0, NULL, &ret);
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
			mcSync(0,NULL,NULL);
			mcDelete(dir[2]-'0', 0, &dir[4]);
			mcSync(0, NULL, &ret);
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
		mcSync(0,NULL,NULL);
		mcMkDir(path[2]-'0', 0, dir);
		mcSync(0, NULL, &ret);
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
	
	// ���̓p�X�̐ݒ�ƃp�[�e�B�V�����̃}�E���g�B
	if(!strncmp(inPath, "hdd", 3)){
		hddin = TRUE;
		getHddParty(inPath, &file, inParty, in);
		if(!strcmp(inParty, mountedParty[0]))
			pfsin=0;
		else if(!strcmp(inParty, mountedParty[1]))
			pfsin=1;
		else
			pfsin=-1;
	}
	// �o�̓p�X�̐ݒ�ƃp�[�e�B�V�����̃}�E���g�B
	if(!strncmp(outPath, "hdd", 3)){
		hddout = TRUE;
		getHddParty(outPath, &file, outParty, out);
		if(!strcmp(outParty, mountedParty[0]))
			pfsout=0;
		else if(!strcmp(outParty, mountedParty[1]))
			pfsout=1;
		else
			pfsout=-1;
	}
	//���̓p�X��HDD�̂Ƃ��}�E���g
	if(hddin){
		if(pfsin<0){
			if(pfsout==0) pfsin=1;
			else		  pfsin=0;
			sprintf(tmp, "pfs%d:", pfsin);
			if(mountedParty[pfsin][0]!=0)
				fileXioUmount(tmp); mountedParty[pfsin][0]=0;
			printf("%s mounting\n", inParty);
			if(fileXioMount(tmp, inParty, FIO_MT_RDWR) < 0) return -1;
			strcpy(mountedParty[pfsin], inParty);
		}
		in[3]=pfsin+'0';
	}
	else
		sprintf(in, "%s%s", inPath, file.name);
	//�o�̓p�X��HDD�̂Ƃ��}�E���g
	if(hddout){
		if(pfsout<0){
			if(pfsin==0) pfsout=1;
			else		 pfsout=0;
			sprintf(tmp, "pfs%d:", pfsout);
			if(mountedParty[pfsout][0]!=0)
				fileXioUmount(tmp); mountedParty[pfsout][0]=0;
			if(fileXioMount(tmp, outParty, FIO_MT_RDWR) < 0) return -1;
			printf("%s mounting\n", outParty);
			strcpy(mountedParty[pfsout], outParty);
		}
		out[3]=pfsout+'0';
	}
	else
		sprintf(out, "%s%s", outPath, file.name);
	
	// �t�H���_�̏ꍇ
	if(file.attr & FIO_S_IFDIR){
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
			if(ynDialog(tmp,0)<0) return -1;
			drawMsg(lang->filer_pasting);
		}
		else if(ret < 0)
			return -1;
		// �t�H���_�̒��g��S�R�s�[
		sprintf(out, "%s%s/", outPath, file.name);
		sprintf(in, "%s%s/", inPath, file.name);
		nfiles = getDir(in, files);
		for(i=0; i<nfiles; i++)
			if(copy(out, in, files[i], n+1) < 0) return -1;
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
		else	  buffSize = fioRead(in_fd, buff, buffSize);
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
				mcSync(0,NULL,NULL);
				mcDelete(out[2]-'0', 0, &out[4]);
				mcSync(0, NULL, NULL);
				goto error;
			}
		}
		size -= buffSize;
	}
	ret=0;
error:
	free(buff);
	if(in_fd>0){
		if(hddin) fileXioClose(in_fd);
		else	  fioClose(in_fd);
	}
	if(out_fd>0){	//�C������
		if(hddout) fileXioClose(out_fd);
		else	  fioClose(out_fd);
	}
	return ret;
}

//-------------------------------------------------
// �y�[�X�g
int paste(const char *path)
{
	char tmp[MAX_PATH];
	int i, ret=-1;
	
	if(!strcmp(path,clipPath)) return -1;
	
	for(i=0; i<nclipFiles; i++){
		strcpy(tmp, clipFiles[i].name);
		if(clipFiles[i].attr & FIO_S_IFDIR) strcat(tmp,"/");
		strcat(tmp, " ");
		strcat(tmp, lang->filer_pasting);
		drawMsg(tmp);
		ret=copy(path, clipPath, clipFiles[i], 0);
		if(ret < 0) break;
		if(cut){
			ret=delete(clipPath, &clipFiles[i]);
			if(ret<0) break;
		}
	}
	
	if(mountedParty[0][0]!=0){
		fileXioUmount("pfs0:"); mountedParty[0][0]=0;
	}
	
	if(mountedParty[1][0]!=0){
		fileXioUmount("pfs1:"); mountedParty[1][0]=0;
	}
	
	return ret;
}

//-------------------------------------------------
//psbCommand psb�R�}���h���s
int psbCommand(void)
{
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

	strcpy(path[0], psb_argv[1]);
	strcpy(path[1], psb_argv[2]);

	//�R�����g
	if(!strnicmp(psb_argv[0], "rem",3))
		return 0;

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

	//psb_argv[1]�̍Ōオ�X���b�V���̂Ƃ��폜
	len = strlen(path[0]);
	if(len>0){
		if(path[0][len-1]=='/')
			path[0][len-1]='\0';
	}
	if(psb_argc>1){
		//psb_argv[2]�̍Ō�ɃX���b�V�������Ƃ�����
		len = strlen(path[1]);
		if(len>0){
			if(path[1][len-1]!='/')
				strcat(path[1], "/");
		}
	}
	//
	strcpy(dir, path[0]);
	if((p=strrchr(dir, '/'))){
		p++;
		*p=0;
	}

	//psb_argv[1]����FILEINFO���쐬����
	//file.name
	if((p = strrchr(path[0], '/')))
		strcpy(file.name, p+1);
	//file.attr
	if(!strncmp(path[0], "mc", 2)){
		mcTable mcDir __attribute__((aligned(64)));
		int mcret;
		//�t�H���_�Ƃ��ăI�[�v�����Ă݂�
		strcpy(pathtmp, path[0]+4); strcat(pathtmp,"/*");
		mcGetDir(path[0][2]-'0', 0, pathtmp, 0, 1, &mcDir);
		mcSync(0, NULL, &mcret);
		if(mcret<0){
			//���s������t�@�C���Ƃ��ăI�[�v�����Ă݂�
			fd = fioOpen(path[0], O_RDONLY);
			if(fd<0)
				file.attr=0;	//���s
			else{
				fioClose(fd);
				file.attr=FIO_S_IFREG;	//�t�@�C��
			}
		}
		else{
		
			file.attr=FIO_S_IFDIR;	//�t�H���_
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
				file.attr=0;	//���s
			else{
				fioClose(fd);
				file.attr=FIO_S_IFREG;	//�t�@�C��
			}
		}
		else{
		
			file.attr=FIO_S_IFDIR;	//�t�H���_
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
				file.attr=0;	//���s
			}
			else{
				fileXioClose(fd);
				file.attr=FIO_S_IFREG;	//�t�@�C��
			}
		}
		else{
			fileXioDclose(fd);
			file.attr=FIO_S_IFDIR;
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
				file.attr=0;	//���s
			else{
				fioClose(fd);
				file.attr=FIO_S_IFREG;	//�t�@�C��
			}
		}
		else{
		
			file.attr=FIO_S_IFDIR;	//�t�H���_
			fioDclose(fd);
		}
	}

	//�R�}���h
	if(!stricmp(psb_argv[0], "copy")){	//�R�s�[
		if(psb_argc<3){
			ret=-1;
		}
		else{
			if(file.attr==0){
				//�R�s�[������̂���������
				ret=-1;
			}
			else{
				//�o�b�N�A�b�v
				clipFilesBackup = clipFiles[0];
				nclipFilesBackup = nclipFiles;
				//
				clipFiles[0]=file;
				nclipFiles = 1;
				//�R�s�[���̃f�B���N�g����(clipPath)
				strcpy(clipPath, dir);
				//
				cut=FALSE;	//�R�s�[
				//�y�[�X�g�J�n
				ret=paste(path[1]);
				//���ɖ߂�
				clipFiles[0] = clipFilesBackup;
				nclipFiles = nclipFilesBackup;
			}
		}
		if(ret) MessageDialog("copy Failed");
	}
	else if(!stricmp(psb_argv[0], "move")){	//�ړ�
		if(psb_argc<3){
			ret=-1;
		}
		else{
			if(file.attr==0){
				//�R�s�[������̂���������
				ret=-1;
			}
			else{
				//�o�b�N�A�b�v
				clipFilesBackup = clipFiles[0];
				nclipFilesBackup = nclipFiles;
				//
				clipFiles[0]=file;
				nclipFiles = 1;
				//�R�s�[���̃f�B���N�g����(clipPath)
				strcpy(clipPath, dir);
				//
				cut=TRUE;	//�ړ�
				//�y�[�X�g�J�n
				ret=paste(path[1]);
				//���ɖ߂�
				clipFiles[0] = clipFilesBackup;
				nclipFiles = nclipFilesBackup;
			}
		}
		if(ret) MessageDialog("move Failed");
	}
	else if(!stricmp(psb_argv[0], "del")){	//�폜
		if(file.attr==0){
			//�폜������̂���������
			ret=-1;
		}
		else{
			int ynret;
			sprintf(message, "%s", file.name);
			if(file.attr & FIO_S_IFDIR) strcat(message, "/");
			strcat(message, "\n");
			strcat(message, lang->filer_delete);
			ynret = ynDialog(message,0);
			if(ynret>0){
				//�폜�J�n
				ret=delete(dir, &file);
			}
		}
		if(ret) MessageDialog("del Failed");
	}
	else if(!stricmp(psb_argv[0], "mkdir")){	//�t�H���_�쐬
		//�쐬�J�n
		ret=newdir(dir, file.name);
		if(ret) MessageDialog("mkdir Failed");
	}
	else if(!stricmp(psb_argv[0], "rmdir")){	//�t�H���_�폜
		if(file.attr==0){
			//�폜������̂���������
			ret=-1;
		}
		else{
			int ynret;
			//�f�B���N�g���폜
			if(file.attr==FIO_S_IFDIR){
				sprintf(message, "%s", file.name);
				if(file.attr & FIO_S_IFDIR) strcat(message, "/");
				strcat(message, "\n");
				strcat(message, lang->filer_delete);
				ynret = ynDialog(message,0);
				if(ynret>0){
					//�폜�J�n
					ret=delete(dir, &file);
				}
			}
			else{
				ret=-1;
			}
		}
		if(ret) MessageDialog("rmdir Failed");
	}
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
	if(!strncmp(psbpath, "hdd", 3)){
		if(nparties==0){
			loadHddModules();
			setPartyList();
		}
	}
	else if(!strncmp(psbpath, "cdfs", 4))
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

//-------------------------------------------------
//�Q�[���^�C�g�����t�@�C�����ɕϊ�
void title2filename(const unsigned char *in, unsigned char *out)
{
	int i=0;
	int code;

	strcpy(out,in);
	code=in[i];
	while(in[i]){
		code=in[i];
		//windows�Ńt�@�C�����Ɏg���Ȃ������́u_�v�ɕϊ�
		if(code==0x22) out[i]='_';	// '"'
		if(code==0x2A) out[i]='_';	// '*'
		if(code==0x2C) out[i]='_';	// ','
		if(code==0x2F) out[i]='_';	// '/'
		if(code==0x3A) out[i]='_';	// ':'
		if(code==0x3B) out[i]='_';	// ';'
		if(code==0x3C) out[i]='_';	// '<'
		if(code==0x3E) out[i]='_';	// '>'
		if(code==0x3F) out[i]='_';	// '?'
		if(code==0x5c) out[i]='_';	// '\'
		if(code==0x7C) out[i]='_';	// '|'
		i++;
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
// psu�t�@�C������C���|�[�g
// �߂�l
// 0�ȉ� :���s
// 0     :mc0�ɃC���|�[�g
// 1     :mc1�ɃC���|�[�g
int psuImport(const char *path, const FILEINFO *file)
{
	//
	int ret = -1;	//�߂�l
	int n = 0;
	PSU_HEADER psu_header[MAX_ENTRY];
	char outdir[MAX_PATH];		//�Z�[�u�f�[�^�̃t�H���_��
	char title[16*4+1]="";
	char *buff=NULL;
	int outmc=0;	//�C���|�[�g���mc�ԍ�

	int in_fd = -1, out_fd = -1;
	int hddin = FALSE;
	int i;

	int dialog_x;		//�_�C�A���Ox�ʒu
	int dialog_y;		//�_�C�A���Oy�ʒu
	int dialog_width;	//�_�C�A���O�� 
	int dialog_height;	//�_�C�A���O����  

	//�t�H���_�̂Ƃ��́Apsu����C���|�[�g�ł��Ȃ�
	if(file->attr & FIO_S_IFDIR){
		ret=-1;
		return ret;
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
			if(r<0) return 0;
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
				ret=-2;
				goto error;
			}
			psuSize = fileXioLseek(in_fd, 0, SEEK_END);
			fileXioLseek(in_fd, 0, SEEK_SET);
		}
		else{
			in_fd = fioOpen(inpath, O_RDONLY);
			if(in_fd<0){
				ret=-2;
				goto error;
			}
			psuSize = fioLseek(in_fd, 0, SEEK_END);
			fioLseek(in_fd, 0, SEEK_SET);
		}
	
		//psu�w�b�_�ǂݍ���
		if(psuSize<sizeof(PSU_HEADER)){
			ret=-3;
			goto error;
		}
		//psu�w�b�_��ǂݍ��ނ̂�psu_header[0]���ꎞ�I�Ɏg��
		memset(&psu_header[0], 0, sizeof(PSU_HEADER));
		if(hddin) fileXioRead(in_fd, (char*)&psu_header[0], sizeof(PSU_HEADER));
		else fioRead(in_fd, &psu_header[0], sizeof(PSU_HEADER));
		n = psu_header[0].size;	//�t�@�C����
		strcpy(outdir, psu_header[0].name);	//�o�͂���t�H���_��
		seek = sizeof(PSU_HEADER);	//�t�@�C���̃V�[�N
	
		//psu_header[0]����ǂݍ���
		for(i=0;i<n;i++){
			//�t�@�C���w�b�_�ǂݍ���
			if(psuSize<seek+sizeof(PSU_HEADER)){
				ret=-4;
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
			//
			if(psu_header[i].size>0){
				fileSize = (((psu_header[i].size-1)/0x400)+1)*0x400;
				if(psuSize<seek + fileSize){
					ret=-5;
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

	//step2 ���̕\�� 
	{
		char tmp[2048];
		int x, y, scroll;
		char fullpath[MAX_PATH];	//�I�����ꂽ�t�H���_�܂��̓t�@�C���̃t���p�X

		//psu�t�@�C���̃t���p�X
		sprintf(fullpath, "%s%s", path, file->name);

		dialog_width = FONT_WIDTH*50;
		dialog_height = FONT_HEIGHT*16;
		dialog_x = (SCREEN_WIDTH-dialog_width)/2;
		dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
		scroll = 0;
		while(1){
			waitPadReady(0, 0);
			if(readpad()){
				if(new_pad & PAD_UP){
					scroll -= 8;
					if(scroll<0) scroll += MAX_ENTRY;
				}
				else if(new_pad & PAD_DOWN){
					scroll += 8;
					if(scroll>=MAX_ENTRY) scroll -= MAX_ENTRY;
				}
				else if(new_pad & PAD_LEFT){
					outmc --;
					if(outmc<0) outmc = 2;
				}
				else if(new_pad & PAD_RIGHT){
					outmc ++;
					if(outmc>2) outmc = 0;
				}
				else if(new_pad & PAD_CROSS){
					ret=-6;
					return ret;
				}
				else if(new_pad & PAD_CIRCLE){
					if(outmc==2){
						ret=-6;
						return ret;
					}
					break;
				}
				else if(new_pad & PAD_SELECT){
					//�L�����Z��
					outmc=2;
				}
			}

			// �`��J�n
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[0], setting->color[1]);
			drawFrame(dialog_x+FONT_WIDTH, dialog_y+FONT_HEIGHT*4,
				dialog_x+dialog_width-FONT_WIDTH, dialog_y+FONT_HEIGHT*14, setting->color[1]);
			// psu�t�@�C���̏���\��
			x = dialog_x+FONT_WIDTH*2;
			y = dialog_y+FONT_HEIGHT*0.5;
			strcpy(tmp, fullpath);
			if(strlen(tmp)>46){
				tmp[42]=0;
				strcat(tmp,"...");
			}
			printXY(tmp, x, y, setting->color[3], TRUE);
			y +=FONT_HEIGHT;
			strcpy(tmp, title);
			if(strlen(tmp)>46){	//title�������Ƃ��ɒZ������
				tmp[42] = 0;
				strcat(tmp,"...");

			}
			printXY(tmp, x, y, setting->color[3], TRUE);
			y +=FONT_HEIGHT;
			sprintf(tmp, "%2d %s", n, lang->filer_import_files);
			printXY(tmp, x, y, setting->color[3], TRUE);
			y +=FONT_HEIGHT*2;
			printXY(lang->filer_import_header, x, y, setting->color[3], TRUE);
			y +=FONT_HEIGHT;
			for(i=0;i<8;i++){
				sprintf(tmp, "%4d:", i+scroll);
				if(i+scroll<n)
					sprintf(tmp, "%4d: %4X: %8d: %s", i+scroll, psu_header[i+scroll].attr, psu_header[i+scroll].size, psu_header[i+scroll].name);
					if(strlen(tmp)>46){
						tmp[42]=0;
						strcat(tmp,"...");
					}
				printXY(tmp, x, y, setting->color[3], TRUE);
				y +=FONT_HEIGHT;
			}
			y += FONT_HEIGHT;
			sprintf(tmp,"   mc0:/    mc1:/    %s", lang->gen_cancel);
			printXY(tmp, x, y, setting->color[3], TRUE);
			printXY(">", x+FONT_WIDTH+FONT_WIDTH*9*outmc, y, setting->color[3], TRUE);
			// �������
			x = FONT_WIDTH*2;
			y = SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT;
			itoSprite(setting->color[0],
				0, y,
				SCREEN_WIDTH, y+FONT_HEIGHT, 0);
			sprintf(tmp,"��:%s �~:%s", lang->gen_ok, lang->gen_cancel);
			printXY(tmp, x, y, setting->color[3], TRUE);
			drawScr();
		}
	}

	//step3 �C���|�[�g�J�n
	{
		char inpath[MAX_PATH];	//psu�t�@�C���̃t���p�X
		char outpath[MAX_PATH];//�Z�[�u�f�[�^�̃t�H���_���o�͂���t�H���_�̃t���p�X
		int seek;
		char tmp[2048];		//�G�p �\���p
		char out[MAX_PATH];	//�Z�[�u�f�[�^�̃t�H���_�̒��̃t�@�C���̃t���p�X
		size_t outsize;
		char party[MAX_NAME];
		int r;

		//�Z�[�u�f�[�^�̃t�H���_���o�͂���t�H���_�̃t���p�X
		if(!outmc)
			strcpy(outpath, "mc0:/");
		else
			strcpy(outpath, "mc1:/");

		//psu�t�@�C��
		if(!strncmp(path, "hdd", 3)){
			hddin = TRUE;
			getHddParty(path, NULL, party, inpath);
			//pfs0�Ƀ}�E���g
			r = mountParty(party);
			if(r<0) return 0;
			inpath[3] = r+'0';
			//psu�t�@�C���̃t���p�X
			strcat(inpath, file->name);
		}
		else{
			//psu�t�@�C���̃t���p�X
			sprintf(inpath, "%s%s", path, file->name);
		}

		//�Z�[�u�f�[�^�̃t�H���_�쐬
		r = newdir(outpath, outdir);
		if(r == -17){	//�t�H���_�����łɂ���Ƃ��㏑�����m�F����
			drawDark();
			itoGsFinish();
			itoSwitchFrameBuffers();
			drawDark();
			sprintf(tmp, "%s%s/\n%s", outpath, outdir, lang->filer_overwrite);
			if(ynDialog(tmp,0)<0){	//�L�����Z��
				ret = -7;
				goto error;
			}
		}
		else if(r < 0){//�t�H���_�쐬���s
			ret = -8;
			goto error;
		}

		//psu�t�@�C���I�[�v��
		if(hddin){
			in_fd = fileXioOpen(inpath, O_RDONLY, fileMode);
			if(in_fd<0){
				ret=-2;
				goto error;
			}
		}
		else{
			in_fd = fioOpen(inpath, O_RDONLY);
			if(in_fd<0){
				ret=-9;
				goto error;
			}
		}

		// �`��J�n
		dialog_width = FONT_WIDTH*32;
		dialog_height = FONT_HEIGHT*2;
		dialog_x = (SCREEN_WIDTH-dialog_width)/2;
		dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
		drawDark();
		itoGsFinish();
		itoSwitchFrameBuffers();
		drawDark();
		seek = sizeof(PSU_HEADER);
		for(i=0;i<n;i++){
			// �v���O���X�o�[
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[0], setting->color[1]);
			itoSprite(setting->color[1],
				dialog_x+FONT_HEIGHT/2, dialog_y+FONT_WIDTH/2,
				dialog_x+FONT_HEIGHT/2+(dialog_width-FONT_WIDTH)*(i*100/n)/100, dialog_y+dialog_height-FONT_WIDTH/2, 0);
			//
			sprintf(tmp, "%2d / %2d", i, n);
			printXY(tmp, dialog_x+120, dialog_y+FONT_HEIGHT/2, setting->color[3], TRUE);
			drawScr();
			//
			seek += sizeof(PSU_HEADER);
			if(psu_header[i].size>0){
				//�������ރf�[�^�̃��������m��
				buff = (char*)malloc(psu_header[i].size);
				if(buff==NULL){
					ret=-10;
					goto error;
				}
				//�o�͂���t�@�C���I�[�v��
				sprintf(out, "%s%s/%s", outpath, outdir, psu_header[i].name);
				out_fd = fioOpen(out, O_WRONLY | O_TRUNC | O_CREAT);
				if(out_fd<0){
					ret=-11;
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
					ret=-12;
					goto error;
				}
				//�N���[�Y
				fioClose(out_fd); out_fd=-1;
				free(buff);
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
int psuExport(const char *path, const FILEINFO *file)
{
	int ret = -1;	//�߂�l
	int n = 0;

	mcTable mcDir[MAX_ENTRY] __attribute__((aligned(64)));
	int mcret;
	int r;

	int sjisout = FALSE;		//psu�t�@�C����sjis�ŏo��
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
	if(file->attr & FIO_S_IFREG){	//�t�@�C��
		ret = -1;
		return ret;
	}

	//step1 �G�N�X�|�[�g����Z�[�u�f�[�^�𒲂ׂ�
	{
		char inpath[MAX_PATH];		//�I�����ꂽ�t�H���_�̃t���p�X
		char Pattern[MAX_PATH];	//�񋓗p�p�^�[��

		//�I�����ꂽ�t�H���_�̃t���p�X
		sprintf(inpath, "%s%s", path, file->name);

		//���X�g�ǂݍ���
		sprintf(Pattern, "%s/*", &inpath[4]);
		mcSync(0, NULL, &mcret);
		mcGetDir(inpath[2]-'0', 0, Pattern, 0, MAX_ENTRY-2, mcDir);
		mcSync(0, NULL, &n);	//�t�@�C����
		//mcDir[0]�̏��
		mcDir[0].fileSizeByte=0;
		mcDir[0].attrFile=0x8427;
		strcpy(mcDir[0].name,".");
	}

	//step2 ���̕\��
	{
		int x,y,scroll;
		char inpath[MAX_PATH];	//�I�����ꂽ�t�H���_�̃t���p�X
		char tmp[2048];		//�\���p
		int i;

		//�I�����ꂽ�t�H���_�̃t���p�X
		sprintf(inpath, "%s%s", path, file->name);

		dialog_width = FONT_WIDTH*50;
		dialog_height = FONT_HEIGHT*16;
		dialog_x = (SCREEN_WIDTH-dialog_width)/2;
		dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
		scroll = 0;
		while(1){
			waitPadReady(0, 0);
			if(readpad()){
				if(new_pad & PAD_UP){
					scroll -= 8;
					if(scroll<0) scroll += MAX_ENTRY;
				}
				else if(new_pad & PAD_DOWN){
					scroll += 8;
					if(scroll>=MAX_ENTRY) scroll -= MAX_ENTRY;
				}
				else if(new_pad & PAD_TRIANGLE){
					sjisout=TRUE;
					break;
				}
				else if(new_pad & PAD_CROSS){
					ret=-201;
					return ret;
				}
				else if(new_pad & PAD_CIRCLE){
					break;
				}
			}
	
			// �`��J�n
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[0], setting->color[1]);
			drawFrame(dialog_x+FONT_WIDTH, dialog_y+FONT_HEIGHT*4,
				dialog_x+dialog_width-FONT_WIDTH, dialog_y+FONT_HEIGHT*14, setting->color[1]);
			//
			x = dialog_x+FONT_WIDTH*2;
			y = dialog_y+FONT_HEIGHT*0.5;
			sprintf(tmp, "%s/", inpath);
			if(strlen(tmp)>46){
				tmp[42]=0;
				strcat(tmp,"...");
			}
			printXY(tmp, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			sprintf(tmp, "%s", file->title);
			if(strlen(tmp)>46){	//title�������Ƃ��ɒZ������
				tmp[42] = 0;
				strcat(tmp,"...");
			}
			printXY(tmp, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			sprintf(tmp, "%d %s", n, lang->filer_export_files);
			printXY(tmp, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT*2;
			printXY(lang->filer_export_header, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			for(i=0;i<8;i++){
				sprintf(tmp, "%4d:", i+scroll);
				if(i+scroll<n)
					sprintf(tmp, "%4d: %4X: %8d: %s", i+scroll, mcDir[i+scroll].attrFile, mcDir[i+scroll].fileSizeByte, mcDir[i+scroll].name);
				if(strlen(tmp)>46){
					tmp[42]=0;
					strcat(tmp,"...");
				}
				printXY(tmp, x, y, setting->color[3], TRUE);
				y += FONT_HEIGHT;
			}
			y += FONT_HEIGHT;
			sprintf(tmp,"��:%s �~:%s", lang->gen_ok, lang->gen_cancel);
			printXY(tmp, x, y, setting->color[3], TRUE);
			// �������
			x = FONT_WIDTH*2;
			y = SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT;
			itoSprite(setting->color[0],
				0, y,
				SCREEN_WIDTH, y+FONT_HEIGHT, 0);
			sprintf(tmp,"��:%s �~:%s", lang->gen_ok, lang->gen_cancel);
			printXY(tmp, x, y, setting->color[3], TRUE);
	
			drawScr();
		}
	}

	//step3
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
		strcpy(tmp,file->name);
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
				//2byte������1byte�ڂ����������
				if( (code>=0x81)&&(code<=0x9F) ) tmp[27] = 0;
				if( (code>=0xE0)&&(code<=0xFF) ) tmp[27] = 0;
			}
		}

		//�o�͂���psu�t�@�C���̃t���p�X
		strcat(outpath, tmp);
		strcat(outpath, ".psu");

		//�o�͂���psu�t�@�C����hdd�̂Ƃ��p�X��ύX
		if(!strncmp(outpath, "hdd", 3)){
			if(nparties==0){
				loadHddModules();
				setPartyList();
			}
			hddout = TRUE;
			getHddParty(outpath, NULL, party, tmp);
			//pfs0�Ƀ}�E���g
			r = mountParty(party);
			if(r<0) return -301;
			strcpy(outpath, tmp);
			outpath[3] = r+'0';
		}
		else if(!strncmp(outpath, "cdfs", 2)){
			ret=-302;
			goto error;
		}

		//psu�t�@�C���I�[�v�� �V�K�쐬
		if(hddout){
			// O_TRUNC �������Ȃ����߁A�I�[�v���O�Ƀt�@�C���폜
			fileXioRemove(outpath);
			out_fd = fileXioOpen(outpath, O_WRONLY|O_TRUNC|O_CREAT, fileMode);
			if(out_fd<0){
				ret=-303;
				goto error;
			}
		}
		else{	//mc mass
			out_fd = fioOpen(outpath, O_WRONLY | O_TRUNC | O_CREAT);
			if(out_fd<0){
				ret=-304;
				goto error;
			}
		}

		//psu�w�b�_��������
		memset(&psu_header, 0, sizeof(PSU_HEADER));
		psu_header.attr = 0x8427;
		psu_header.size = n;
		strcpy(psu_header.name, file->name);
		if(hddout){
			outsize = fileXioWrite(out_fd, (char*)&psu_header, sizeof(PSU_HEADER));
			if(outsize!=sizeof(PSU_HEADER)){
				ret=-305;
				goto error;
			}
		}
		else{
			outsize = fioWrite(out_fd, &psu_header, sizeof(PSU_HEADER));
			if(outsize!=sizeof(PSU_HEADER)){
				ret=-306;
				goto error;
			}
		}
	
		//�t�@�C���w�b�_�ƃt�@�C����������
		dialog_width = FONT_WIDTH*32;
		dialog_height = FONT_HEIGHT*2;
		dialog_x = (SCREEN_WIDTH-dialog_width)/2;
		dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
		drawDark();
		itoGsFinish();
		itoSwitchFrameBuffers();
		drawDark();
		for(i=0;i<n;i++){
			// �`��J�n
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[0], setting->color[1]);
			// �v���O���X�o�[
			itoSprite(setting->color[1],
				dialog_x+FONT_HEIGHT/2, dialog_y+FONT_WIDTH/2,
				dialog_x+FONT_HEIGHT/2+(dialog_width-FONT_WIDTH)*(i*100/n)/100, dialog_y+dialog_height-FONT_WIDTH/2, 0);
			sprintf(tmp, "%2d / %2d", i, n);
			printXY(tmp, dialog_x+120, dialog_y+FONT_HEIGHT/2, setting->color[3], TRUE);
			drawScr();
			//�t�@�C���w�b�_���쐬
			memset(&psu_header, 0, sizeof(PSU_HEADER));
			psu_header.attr = mcDir[i].attrFile;	//�t�@�C�������̓������[�J�[�h�Ɠ����ɂ���
			psu_header.size = mcDir[i].fileSizeByte;
			strncpy(psu_header.name, mcDir[i].name,32);
			//�t�@�C���w�b�_��������
			if(hddout){
				outsize = fileXioWrite(out_fd, (char*)&psu_header, sizeof(PSU_HEADER));
				if(outsize!=sizeof(PSU_HEADER)){
					ret=-307;
					goto error;
				}
			}
			else{
				outsize = fioWrite(out_fd, &psu_header, sizeof(PSU_HEADER));
				if(outsize!=sizeof(PSU_HEADER)){
					ret=-308;
					goto error;
				}
			}
			//�t�@�C����������
			if(mcDir[i].fileSizeByte>0){
				sprintf(tmppath, "%s/%s", inpath, mcDir[i].name);
				writeSize = (((mcDir[i].fileSizeByte-1)/0x400)+1)*0x400;
				buff = (char*)malloc(writeSize);
				if(buff==NULL){
					ret=-309;
					goto error;
				}
				memset(buff, 0, writeSize);
				//�t�@�C���I�[�v��
				in_fd = fioOpen(tmppath, O_RDONLY);
				if(in_fd<0){
					ret=-310;
					goto error;
				}
				//�ǂݍ���
				readSize = fioRead(in_fd, buff, mcDir[i].fileSizeByte);
				if(readSize!=mcDir[i].fileSizeByte){
					ret=-311;
					goto error;
				}
				//�N���[�Y
				fioClose(in_fd); in_fd=-1;
				//psu�t�@�C���ɏ�������
				if(hddout){
					outsize = fileXioWrite(out_fd, buff, writeSize);
					if(outsize!=writeSize){
						ret=-312;
						goto error;
					}
				}
				else{
					outsize = fioWrite(out_fd, buff, writeSize);
					if(outsize!=writeSize){
						ret=-313;
						goto error;
					}
				}
				free(buff);
			}
		}
	}
	//psu�t�@�C���N���[�Y
	if(hddout)fileXioClose(out_fd);
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
			mcSync(0,NULL,NULL);
			mcDelete(outpath[2]-'0', 0, &outpath[4]);
			mcSync(0, NULL, &r);
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
		drawDialogTmp(KEY_X, KEY_Y, KEY_X+KEY_W, KEY_Y+KEY_H, setting->color[0], setting->color[1]);
		//�L�[�{�[�h�����̘g
		drawFrame(KEY_X+FONT_WIDTH, KEY_Y+FONT_HEIGHT*1.5,
			KEY_X+KEY_W-FONT_WIDTH, KEY_Y+FONT_HEIGHT*9.5, setting->color[1]);
		//���͒��̕�����̕\��
		printXY(out,
			KEY_X+FONT_WIDTH*2, KEY_Y+FONT_HEIGHT*0.5,
			setting->color[3], TRUE);
		t++;
		//�L�����b�g
		if(t<SCANRATE/2){
			printXY("|",
				KEY_X+FONT_WIDTH*0.5+(cur+1)*FONT_WIDTH,
				KEY_Y+FONT_HEIGHT*0.5,
				setting->color[3], TRUE);
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
			itoSprite(setting->color[2]|0x10000000,
				x, y,
				x+FONT_WIDTH*3, y+FONT_HEIGHT, 0);
		}
		else{
			if(sel==WFONTS*HFONTS)
				x = KEY_X+KEY_W/4;	//OK
			else
				x = KEY_X+KEY_W/2;	//CANCEL
			y = KEY_Y+FONT_HEIGHT*10;
			itoSprite(setting->color[2]|0x10000000,
				x, y,
				x+KEY_W/4, y+FONT_HEIGHT, 0);
		}
		//�A���t�@�u�����h����
		itoPrimAlphaBlending(FALSE);

		//�L�[�{�[�h�\��
		for(i=0; i<KEY_LEN; i++){
			sprintf(tmp,"%c",KEY[i]);
			printXY(tmp,
				KEY_X+FONT_WIDTH*3 + (i%WFONTS)*FONT_WIDTH*3,
				KEY_Y+FONT_HEIGHT*2 + (i/WFONTS)*FONT_HEIGHT,
				setting->color[3], TRUE);
		}
		//OK�\��
		x=((KEY_W/4)-FONT_WIDTH*strlen(lang->gen_ok))/2;
		sprintf(tmp, "%s",lang->gen_ok);
		printXY(tmp, KEY_X+KEY_W/4+x, KEY_Y+FONT_HEIGHT*10, setting->color[3], TRUE);
		//CANCEL�\��
		x=((KEY_W/4)-FONT_WIDTH*strlen(lang->gen_cancel))/2;
		sprintf(tmp, "%s",lang->gen_cancel);
		printXY(tmp, KEY_X+KEY_W/2+x, KEY_Y+FONT_HEIGHT*10, setting->color[3], TRUE);
		// �������
		x = FONT_WIDTH*2;
		y = SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT;
		itoSprite(setting->color[0],
			0, y,
			SCREEN_WIDTH, y+FONT_HEIGHT, 0);
		printXY(lang->filer_keyboard_hint, x, y, setting->color[3], TRUE);
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
	FILEINFO file;
	char party[MAX_NAME], dir[MAX_PATH];

	char tmp[16*4+1];

	// �t�@�C�����X�g�ݒ�
	if(path[0]==0){
		for(i=0;i<5;i++){
			memset(&files[i].modifyTime,0,sizeof(PS2TIME));
			files[i].fileSizeByte = 0;
			files[i].attr = FIO_S_IFDIR;
			if(i==0) strcpy(files[i].name, "mc0:");
			if(i==1) strcpy(files[i].name, "mc1:");
			if(i==2) strcpy(files[i].name, "hdd0:");
			if(i==3) strcpy(files[i].name, "cdfs:");
			if(i==4) strcpy(files[i].name, "mass:");
			files[i].title[0] = 0;
			files[i].type = TYPE_OTHER;
		}
		nfiles = 5;
		if(cnfmode==ELF_FILE){
			memset(&files[5].modifyTime,0,sizeof(PS2TIME));
			files[5].fileSizeByte = 0;
			files[5].attr = FIO_S_IFDIR;
			strcpy(files[5].name, "MISC");
			files[5].type = TYPE_OTHER;
			files[5].title[0] = 0;
			nfiles = 6;
		}
	}
	else if(!strcmp(path, "MISC/")){
		for(i=0;i<7;i++){
			memset(&files[i].modifyTime,0,sizeof(PS2TIME));
			files[i].fileSizeByte = 0;
			if(i==0)
				files[i].attr = FIO_S_IFDIR;
			else
				files[i].attr = FIO_S_IFREG;
			if(i==0) strcpy(files[i].name, "..");
			if(i==1) strcpy(files[i].name, "FileBrowser");
			if(i==2) strcpy(files[i].name, "PS2Browser");
			if(i==3) strcpy(files[i].name, "PS2Disc");
			if(i==4) strcpy(files[i].name, "PS2Net");	//PS2Net uLaunchELF3.60
			if(i==5) strcpy(files[i].name, "INFO");
			if(i==6) strcpy(files[i].name, "CONFIG");
			files[i].title[0] = 0;
			files[i].type = TYPE_OTHER;
		}
		nfiles = 7;
	}
	else{
		//files[0]��������
		memset(&files[0].modifyTime, 0, sizeof(PS2TIME));
		files[0].fileSizeByte = 0;
		files[0].attr = FIO_S_IFDIR;
		strcpy(files[0].name, "..");
		files[0].title[0] = 0;
		files[0].type=TYPE_OTHER;

		//�t�@�C�����X�g�ƃt�@�C�������擾
		nfiles = getDir(path, &files[1]) + 1;
		if(strcmp(ext, "*")){	//�t�@�C���}�X�N
			for(i=j=1; i<nfiles; i++){
				if(files[i].attr & FIO_S_IFDIR)
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
			//�Q�[���^�C�g���擾
			if( !strncmp(path, "cdfs", 4)){
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
			if(files[i].attr & FIO_S_IFDIR){	//�t�H���_
				if(ret<0)
					files[i].type=TYPE_DIR;
				else
					files[i].type=TYPE_PS2SAVE;	//PS2SAVE
			}
			else if(files[i].attr & FIO_S_IFREG){	//�t�@�C��
				sprintf(fullpath, "%s%s", path, files[i].name);
				//ELF�w�b�_�𒲂ׂ�
				if(!strncmp(path, "mc", 2) || !strncmp(path, "mass", 4)){
					checkELFret = checkELFheader(fullpath); 	//checkELFheader
					if(checkELFret<0)
						files[i].type=TYPE_FILE;
					else
						files[i].type=TYPE_ELF;
				}
				else if( !strncmp(path, "hdd", 3)&&strcmp(path, "hdd0:/") ){
					checkELFret = checkELFheader(fullpath); 	//checkELFheader
					mountedParty[0][0]=0;
					if(checkELFret<0)
						files[i].type=TYPE_FILE;
					else
						files[i].type=TYPE_ELF;
					//HDD�̂Ƃ��ă}�E���g
					strcpy(file.name, files[i].name);
					strcpy(file.title, files[i].title);
					file.attr=files[i].attr;
					file.type=files[i].type;
					//
					getHddParty(path, &file, party, dir);
					mountParty(party);
				}
				else if( !strncmp(path, "cdfs", 4)){
					if(setting->discELFCheck){
						checkELFret = checkELFheader(fullpath); 	//checkELFheader
						if(checkELFret<0)
							files[i].type=TYPE_FILE;
						else
							files[i].type=TYPE_ELF;
					}
					else{
						files[i].type=TYPE_FILE;
					}
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
	uint64 color,iconcolor=0;
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
				up=TRUE;
			else if(new_pad & PAD_CIRCLE){	//change dir
				if(files[sel].attr & FIO_S_IFDIR){
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
				cd=TRUE;
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
					}
					pushed = FALSE;
				}
			}
			else if(new_pad & PAD_R3){	//FILEICON
				setting->fileicon = !setting->fileicon;
				if(setting->fileicon) cd=TRUE;
			}
			else if(new_pad & PAD_L3){	//FLICKERCONTROL
				setting->flickerControl = !setting->flickerControl;
			}
			else if(new_pad & PAD_L2) {	//�ڍו\��
				detail++;
				if(detail==3) detail=0;
			}
			//ELF_FILE ELF�I����
			if(cnfmode==ELF_FILE){
				if(new_pad & PAD_CIRCLE) {	//�t�@�C��������
					if(files[sel].attr & FIO_S_IFREG){
						int ret;
						sprintf(out, "%s%s", path, files[sel].name);
						ret = checkELFheader(out);	//�w�b�_�`�F�b�N
						mountedParty[0][0]=0;
						if(ret==1){
							//ELF�t�@�C���I��
							strcpy(LastDir, path);
							break;
						}
						else{
							//ELF�t�@�C���ł͂Ȃ��Ƃ�
							char *extension;
							extension = getExtension(files[sel].name);
							if(extension!=NULL){
								if(!stricmp(extension, ".psb")){
									strcpy(LastDir, path);
									break;
								}
							}
						}
						pushed=FALSE;
						sprintf(msg0, lang->filer_not_elf);
						out[0] = 0;
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
					if(files[sel].attr & FIO_S_IFREG){
						sprintf(out, "%s%s", path, files[sel].name);
						//�w�b�_�`�F�b�N
						if(checkFONTX2header(out)<0){
							mountedParty[0][0]=0;
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
					if(files[sel].attr & FIO_S_IFREG){	//�t�@�C��
						char fullpath[MAX_PATH];
						int ret;
						sprintf(fullpath, "%s%s", path, files[sel].name);
						ret = checkELFheader(fullpath);
						mountedParty[0][0]=0;
						if(ret==1){
							//ELF�t�@�C���I��
							strcpy(out, fullpath);
							strcpy(LastDir, path);
							break;
						}
						else{
							//ELF�t�@�C���ł͂Ȃ��Ƃ�
							char *extension;
							pushed=FALSE;
							sprintf(msg0, lang->filer_not_elf);
							extension = getExtension(fullpath);
							if(extension!=NULL){
								if(!stricmp(extension, ".psb")){	//psb�t�@�C�������s
									int ynret;
									int psbret;
									ynret = ynDialog(lang->filer_execute_psb, 0);
									if(ynret>0){
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
								if(files[sel].attr & FIO_S_IFDIR)
									strcat(tmp,"/");
							}
							strcat(tmp, "\n");
							strcat(tmp, lang->filer_delete);
							ret = ynDialog(tmp,0);
						}
						else
							ret = ynDialog(lang->filer_deletemarkfiles,0);

						if(ret>0){
							if(nmarks==0){
								strcpy(tmp, files[sel].name);
								if(files[sel].attr & FIO_S_IFDIR) strcat(tmp,"/");
								strcat(tmp, " ");
								strcat(tmp, lang->filer_deleting);
								drawMsg(tmp);
								ret=delete(path, &files[sel]);
							}
							else{
								for(i=0; i<nfiles; i++){
									if(marks[i]){
										strcpy(tmp, files[i].name);
										if(files[i].attr & FIO_S_IFDIR) strcat(tmp,"/");
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
						}
						pushed = FALSE;
					}
					else if(ret==EXPORT){	// psu�t�@�C���ɃG�N�X�|�[�g
						drawDark();
						itoGsFinish();
						itoSwitchFrameBuffers();
						drawDark();

						ret = psuExport(path, &files[sel]);
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
					else if(ret==IMPORT){	// psu�t�@�C������C���|�[�g
						drawDark();
						itoGsFinish();
						itoSwitchFrameBuffers();
						drawDark();

						ret = psuImport(path, &files[sel]);
						if(ret<0){
							sprintf(msg0, "%s %d", lang->filer_importfailed, ret);
							pushed = FALSE;
						}
						else{
							if(ret==0) strcpy(tmp,"mc0:/");
							else strcpy(tmp,"mc1:/");
							sprintf(msg0, "%s %s", lang->filer_importto, tmp);
							pushed = FALSE;
							cd = TRUE;
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
		//�t�H���_�ړ��i�ړ���̃t�H���_�����݂̃t�H���_�Ɠ����Ƃ��̓t�@�C�����X�g���X�V�j
		if(cd){
			nfiles = setFileList(path, ext, files, cnfmode);
			// �󂫗e�ʎ擾
			vfreeSpace=FALSE;	//�󂫗e�ʕ\���t���O
			if(cnfmode==ANY_FILE){
				if(!strncmp(path, "mc", 2)){
					mcGetInfo(path[2]-'0', 0, NULL, &mcfreeSpace, NULL);
					mcSync(0,NULL,NULL);
					freeSpace = mcfreeSpace*1024;
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
		}
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
		clrScr(setting->color[0]);
		// �t�@�C�����X�g
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nfiles)
				break;
			if(top+i == sel){
				color = setting->color[2];
				printXY(">", x, y, color, TRUE);	//�J�[�\���\��
			}
			else
				color = setting->color[3];

			//�}�[�N�\��
			if(marks[top+i]){
				printXY("*", x+FONT_WIDTH, y, setting->color[3], TRUE);
			}

			//�t�@�C�����X�g�\��
			if(files[top+i].attr & FIO_S_IFDIR){	//�t�H���_�̂Ƃ�
				if(!strcmp(files[top+i].name,".."))
					strcpy(tmp,"..");
				else if(title && files[top+i].title[0]!=0)
					strcpy(tmp,files[top+i].title);	//�Q�[���^�C�g��
				else
					sprintf(tmp, "%s/", files[top+i].name);	//�t�H���_��
			}
			else
				strcpy(tmp,files[top+i].name);	//�t�@�C����

			//�t�@�C�����������Ƃ��́A�Z������
			if(strlen(tmp)>52){
				tmp[53]=0;
				code=tmp[52];
				if( (code>=0x81)&&(code<=0x9F) ) tmp[52] = 0;
				if( (code>=0xE0)&&(code<=0xFF) ) tmp[52] = 0;
				strcat(tmp,"...");
			}

			//
			if(setting->fileicon){
				//�t�@�C�����ƃA�C�R����\��
				if(files[top+i].type!=TYPE_OTHER){
					if(files[top+i].type==TYPE_DIR) iconcolor=setting->color[4];
					else if(files[top+i].type==TYPE_FILE) iconcolor=setting->color[5];
					else if(files[top+i].type==TYPE_PS2SAVE) iconcolor=setting->color[6];
					else if(files[top+i].type==TYPE_ELF) iconcolor=setting->color[7];
					//�A�C�R��
					itoSprite(iconcolor,
						x+FONT_WIDTH*2, y,
						x+FONT_WIDTH*2+FONT_WIDTH, y+(FONT_HEIGHT - GetFontMargin(LINE_MARGIN)), 0);
				}
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
					if(files[top+i].attr & FIO_S_IFDIR)
						sprintf(tmp,"<DIR>");
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
						itoSprite(setting->color[0],
							SCREEN_WIDTH-FONT_WIDTH*14, y,
							SCREEN_WIDTH-FONT_WIDTH*4, y+FONT_HEIGHT, 0);
						itoLine(setting->color[1], SCREEN_WIDTH-FONT_WIDTH*13.5, y, 0,
							setting->color[1], SCREEN_WIDTH-FONT_WIDTH*13.5, y+FONT_HEIGHT, 0);	
						printXY(tmp, SCREEN_WIDTH-FONT_WIDTH*(4+len), y, color, TRUE);
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
							files[top+i].modifyTime.year,
							files[top+i].modifyTime.month,
							files[top+i].modifyTime.day,
							files[top+i].modifyTime.hour,
							files[top+i].modifyTime.min,
							files[top+i].modifyTime.sec);
					}
					len=strlen(tmp);
					if(strcmp(files[top+i].name,"..")){
						itoSprite(setting->color[0],
							SCREEN_WIDTH-FONT_WIDTH*24, y,
							SCREEN_WIDTH-FONT_WIDTH*4, y+FONT_HEIGHT, 0);
						itoLine(setting->color[1], SCREEN_WIDTH-FONT_WIDTH*23.5, y, 0,
							setting->color[1], SCREEN_WIDTH-FONT_WIDTH*23.5, y+FONT_HEIGHT, 0);	
						printXY(tmp, SCREEN_WIDTH-FONT_WIDTH*(4+len), y, color, TRUE);
					}
				}
			}
			y += FONT_HEIGHT;
		}
		// �X�N���[���o�[
		if(nfiles > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nfiles);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nfiles);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// ���b�Z�[�W
		if(pushed) sprintf(msg0, "Path: %s", path);
		// �������
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
		setScrTmp(msg0, msg1);

		// �t���[�X�y�[�X�\��
		if(vfreeSpace){
			if(freeSpace >= 1024*1024)
				sprintf(tmp, "[%.1fMB free]", (double)freeSpace/1024/1024);
			else if(freeSpace >= 1024)
				sprintf(tmp, "[%.1fKB free]", (double)freeSpace/1024);
			else
				sprintf(tmp, "[%dB free]", freeSpace);
			ret=strlen(tmp);
			//
			printXY(tmp,
				SCREEN_WIDTH-FONT_WIDTH*(ret+2), SCREEN_MARGIN,
				setting->color[3], TRUE);
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
