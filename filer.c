#include "launchelf.h"

typedef struct{
	char name[256];
	char title[16*4+1];
	unsigned short attr;
	int type;
} FILEINFO;

// psu�t�@�C���w�b�_�\����
typedef struct { // 512 bytes
	unsigned int  attr;
	unsigned int  size;	//file size, 0 for directory
	unsigned char createtime[8];	//0x00:sec:min:hour:day:month:year
	unsigned int unknown1;
	unsigned int unknown2;
	unsigned char modifytime[8];	//0x00:sec:min:hour:day:month:year
	unsigned char unknown3[32];
	unsigned char name[32];
	unsigned char unknown4[416];
} PSU_HEADER;

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

enum
{
	TYPE_OTHER,
	TYPE_DIR,
	TYPE_FILE,
	TYPE_PS2SAVE,
	TYPE_ELF
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

unsigned char *elisaFnt=NULL;
size_t freeSpace;
int mcfreeSpace;
int vfreeSpace;
int cut;
int nclipFiles, nmarks, nparties;
int title;
char mountedParty[2][MAX_NAME];
char parties[MAX_PARTITIONS][MAX_NAME];
char clipPath[MAX_PATH], LastDir[MAX_NAME], marks[MAX_ENTRY];
FILEINFO clipFiles[MAX_ENTRY];
int fileMode =  FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH;

/*===============================================================
// HDD�̃p�[�e�B�V�����ƃp�X���擾
//===============================================================*/
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

/*===============================================================
// �p�[�e�B�V�����̃}�E���g
//===============================================================*/
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

/*===============================================================
// �m�F�_�C�A���O
//===============================================================*/
int ynDialog(const char *message)
{
	char msg[512];
	int dh, dw, dx, dy;
	int sel=0, n, tw;//, a=6, b=4, c=2;
	int i, x, len, ret;
	int x_margin;
	int y_margin;
	char tmp[MAX_PATH];

	strcpy(msg, message);

	//\n��؂��\0��؂�ɕϊ� ���s�̐� n
	for(i=0,n=1; msg[i]!=0; i++){
		if(msg[i]=='\n'){
			msg[i]=0;
			n++;
		}
	}
	//�\�����镶����̍ő�̕��𒲂ׂ� tw
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
			drawChar('>', dx+x_margin+x, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3]);
		else
			drawChar('>',dx+x_margin+x+FONT_WIDTH*11, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3]);
		drawScr();
	}
	//
	x=(tw-FONT_WIDTH*12)/2;
	drawChar(' ', dx+x_margin+x, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3]);
	drawChar(' ',dx+x_margin+x+FONT_WIDTH*11, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3]);
	return ret;
}

/*===============================================================
// �m�F�_�C�A���O
//===============================================================*/
void MessageDialog(const char *message)
{
	char msg[512];
	int dh, dw, dx, dy;
	int n, tw;
	int i, x, len, ret;
	int x_margin;
	int y_margin;
	char tmp[MAX_PATH];

	strcpy(msg, message);

	//\n��؂��\0��؂�ɕϊ� ���s�̐� n
	for(i=0,n=1; msg[i]!=0; i++){
		if(msg[i]=='\n'){
			msg[i]=0;
			n++;
		}
	}
	//�\�����镶����̍ő�̕��𒲂ׂ� tw
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

/*===============================================================
// �N�C�b�N�\�[�g
//===============================================================*/
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
void sort(FILEINFO *a, int left, int right) {
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

/*===============================================================
// �������[�J�[�h�ǂݍ���
//===============================================================*/
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
		(!strcmp(mcDir[i].name,".") || !strcmp(mcDir[i].name,"..")))
			continue;
		strcpy(info[j].name, mcDir[i].name);
		if(mcDir[i].attrFile & MC_ATTR_SUBDIR)
			info[j].attr = FIO_S_IFDIR;
		else
			info[j].attr = FIO_S_IFREG;
		j++;
	}
	
	return j;
}

/*===============================================================
// CD�ǂݍ���
//===============================================================*/
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
		 (!strcmp(TocEntryList[i].filename,".") ||
		  !strcmp(TocEntryList[i].filename,"..")))
			continue;
		strcpy(info[j].name, TocEntryList[i].filename);
		if(TocEntryList[i].fileProperties & 0x02)
			info[j].attr = FIO_S_IFDIR;
		else
			info[j].attr = FIO_S_IFREG;
		j++;
	}
	
	return j;
}

/*===============================================================
// �p�[�e�B�V�������X�g�ݒ�
//===============================================================*/
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

/*===============================================================
// HDD�ǂݍ���
//===============================================================*/
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
		}
		return nparties;
	}
	
	getHddParty(path,NULL,party,dir);
	ret = mountParty(party);
	if(ret<0) return 0;
	dir[3] = ret+'0';
	
	if((fd=fileXioDopen(dir)) < 0) return 0;
	
	while(fileXioDread(fd, &dirbuf)){
		if(dirbuf.stat.mode & FIO_S_IFDIR &&
		(!strcmp(dirbuf.name,".") || !strcmp(dirbuf.name,"..")))
			continue;
		
		info[i].attr = dirbuf.stat.mode;
		strcpy(info[i].name, dirbuf.name);
		i++;
		if(i==max) break;
	}
	
	fileXioDclose(fd);
	
	return i;
}

/*===============================================================
// USB�}�X�X�g���[�W�ǂݍ���
//===============================================================*/
int readMASS(const char *path, FILEINFO *info, int max)
{
	fat_dir_record record;
	int ret, n=0;
	
	loadUsbModules();
	
	ret = usb_mass_getFirstDirentry((char*)path+5, &record);
	while(ret > 0){
		if(record.attr & 0x10 &&
		(!strcmp(record.name,".") || !strcmp(record.name,".."))){
			ret = usb_mass_getNextDirentry(&record);
			continue;
		}
		
		strcpy(info[n].name, record.name);
		if(record.attr & 0x10)
			info[n].attr = FIO_S_IFDIR;
		else
			info[n].attr = FIO_S_IFREG;
		n++;
		ret = usb_mass_getNextDirentry(&record);
	}
	
	return n;
}

/*===============================================================
// �t�@�C�����X�g�擾
//===============================================================*/
int getDir(const char *path, FILEINFO *info)
{
	int max=MAX_ENTRY-2;
	int n;
	
	if(!strncmp(path, "mc", 2))			n=readMC(path, info, max);
	else if(!strncmp(path, "hdd", 3))	n=readHDD(path, info, max);
	else if(!strncmp(path, "mass", 4))	n=readMASS(path, info, max);
	else if(!strncmp(path, "cdfs", 4))	n=readCD(path, info, max);
	else return 0;
	
	return n;
}

/*===============================================================
// �Z�[�u�f�[�^�^�C�g���̎擾
//===============================================================*/
int getGameTitle(const char *path, const FILEINFO *file, char *out)
{
	iox_dirent_t dirEnt;
	char party[MAX_NAME], dir[MAX_PATH];
	int fd=-1, dirfd=-1, size, hddin=FALSE, ret;
	
	if(file->attr & FIO_S_IFREG) return -1;
	if(path[0]==0 || !strcmp(path,"hdd0:/")) return -1;
	
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
			 !strcmp(dirEnt.name,"icon.sys")){
				strcat(dir, "icon.sys");
				if((fd=fileXioOpen(dir, O_RDONLY, fileMode)) < 0)
					goto error;
				if((size=fileXioLseek(fd,0,SEEK_END)) <= 0x100)
					goto error;
				fileXioLseek(fd,0xC0,SEEK_SET);
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
		if((size=fioLseek(fd,0,SEEK_END)) <= 0x100) goto error;
		fioLseek(fd,0xC0,SEEK_SET);
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

/*===============================================================
// ���j���[
//===============================================================*/
int menu(const char *path, const char *file)
{
	uint64 color;
	char enable[NUM_MENU], tmp[MAX_PATH];
	int x, y, i, sel;
	
	int menu_x = FONT_WIDTH*45;
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
	if(!strncmp(path, "hdd", 3)){
		enable[EXPORT] = FALSE;
	}
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
		//R1�{�^�����������Ƃ��̃J�[�\����".."�̂Ƃ�
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
		}
		
		// �`��J�n
		drawDialogTmp(menu_x, menu_y, menu_x+menu_w, menu_y+menu_h, setting->color[0], setting->color[1]);
		for(i=0,y=74; i<NUM_MENU; i++){
			if(i==COPY)			strcpy(tmp, lang->filer_menu_copy);
			else if(i==CUT)		strcpy(tmp, lang->filer_menu_cut);
			else if(i==PASTE)		strcpy(tmp, lang->filer_menu_paste);
			else if(i==DELETE)	strcpy(tmp, lang->filer_menu_delete);
			else if(i==RENAME)	strcpy(tmp, lang->filer_menu_rename);
			else if(i==NEWDIR)	strcpy(tmp, lang->filer_menu_newdir);
			else if(i==GETSIZE)	strcpy(tmp, lang->filer_menu_getsize);
			else if(i==EXPORT)	strcpy(tmp, lang->filer_menu_exportpsu);
			else if(i==IMPORT)	strcpy(tmp, lang->filer_menu_importpsu);

			if(enable[i])
				color = setting->color[2];
			else
				color = setting->color[3];
			
			printXY(tmp, menu_x+FONT_WIDTH*2, menu_y+FONT_HEIGHT/2+i*FONT_HEIGHT, color, TRUE);
			y+=FONT_HEIGHT;
		}
		if(sel<NUM_MENU)
			drawChar('>', menu_x+FONT_WIDTH, menu_y+FONT_HEIGHT/2+sel*FONT_HEIGHT, setting->color[2]);
		
		// �������
		x = FONT_WIDTH*2;
		y = SCREEN_MARGIN+FONT_HEIGHT*20;
		itoSprite(setting->color[0],
			0, y,
			SCREEN_WIDTH, y+FONT_HEIGHT, 0);
		sprintf(tmp,"��:%s �~:%s", lang->gen_ok, lang->gen_cancel);
		printXY(tmp, x, y, setting->color[3], TRUE);
		drawScr();
	}
	
	return sel;
}

/*===============================================================
// �t�@�C���T�C�Y�擾
//===============================================================*/
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
			fioClose(fd);;
		}
	}
	return size;
}

/*===============================================================
// �t�@�C���E�t�H���_�폜
//===============================================================*/
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

/*===============================================================
// �t�@�C���E�t�H���_���l�[��
//===============================================================*/
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

/*===============================================================
// �V�K�t�H���_�쐬
//===============================================================*/
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

/*===============================================================
// �t�@�C���R�s�[
//===============================================================*/
int copy(const char *outPath, const char *inPath, FILEINFO file, int n)
{
	FILEINFO files[MAX_ENTRY];
	char out[MAX_PATH], in[MAX_PATH], tmp[MAX_PATH],
		*buff=NULL, inParty[MAX_NAME], outParty[MAX_NAME];
	int hddout=FALSE, hddin=FALSE, nfiles, i;
	size_t size, outsize;
	int ret=-1, pfsout=-1, pfsin=-1, in_fd=-1, out_fd=-1, buffSize;
	
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
	//���̓p�X��HDD�̂Ƃ��}�E���g
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
			itoSwitchFrameBuffers();
			drawDark();
			ret=-1;
			if(title) ret=getGameTitle(outPath, &file, tmp);
			if(ret<0) sprintf(tmp, "%s%s/", outPath, file.name);
			strcat(tmp, "\n");
			strcat(tmp, lang->filer_overwrite);
			if(ynDialog(tmp)<0) return -1;
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

/*===============================================================
// �y�[�X�g
//===============================================================*/
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

/*===============================================================
//�Q�[���^�C�g�����t�@�C�����ɕϊ�
//===============================================================*/
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

/*===============================================================
//�Q�[���^�C�g����sjis�̉p�����ƋL����ASCII�ɕϊ�
//===============================================================*/
void sjis2ascii(const unsigned char *in, unsigned char *out)
{
	int i=0;
	int code;
	unsigned char ascii;
	int n=0;

	code=in[i];
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

/*===============================================================
// psu�t�@�C������C���|�[�g
// �߂�l
// 0�ȉ� :���s
// 0     :mc0�ɃC���|�[�g
// 1     :mc1�ɃC���|�[�g
//===============================================================*/
int psuImport(const char *path, const FILEINFO *file)
{
	//
	int ret = -1;	//�߂�l
	int n = 0;
	PSU_HEADER psu_header[MAX_ENTRY];
	char outdir[MAX_PATH];		//�Z�[�u�f�[�^�̃t�H���_��
	char title[16*4+1];
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
				tmp[42]='.';
				tmp[43]='.';
				tmp[44]='.';
				tmp[45]=0;
			}
			printXY(tmp, x, y, setting->color[3], TRUE);
			y +=FONT_HEIGHT;
			strcpy(tmp, title);
			if(strlen(tmp)>46){	//title�������Ƃ��ɒZ������
				tmp[42] = '.';
				tmp[43] = '.';
				tmp[44] = '.';
				tmp[45] = 0;
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
						tmp[42]='.';
						tmp[43]='.';
						tmp[44]='.';
						tmp[45]=0;
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
			y = SCREEN_MARGIN+FONT_HEIGHT*20;
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
			itoSwitchFrameBuffers();
			drawDark();
			sprintf(tmp, "%s%s/\n%s", outpath, outdir, lang->filer_overwrite);
			if(ynDialog(tmp)<0){	//�L�����Z��
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
		dialog_height = FONT_HEIGHT*1.5;
		dialog_x = (SCREEN_WIDTH-dialog_width)/2;
		dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
		drawDark();
		itoSwitchFrameBuffers();
		drawDark();
		seek = sizeof(PSU_HEADER);
		for(i=0;i<n;i++){
			// �v���O���X�o�[
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[0], setting->color[1]);
			itoSprite(setting->color[1],
				dialog_x+6, dialog_y+6,
				dialog_x+6+(dialog_width-12)*(i*100/n)/100, dialog_y+dialog_height-5, 0);
			//
			sprintf(tmp, "%2d / %2d", i, n);
			printXY(tmp, dialog_x+120, dialog_y+7, setting->color[3], TRUE);
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
				memset(buff, 0, psu_header[i].size);

				//�o�͂���t�@�C���I�[�v��
				sprintf(out, "%s%s/%s", outpath, outdir, psu_header[i].name);
				out_fd = fioOpen(out, O_WRONLY | O_TRUNC | O_CREAT);
				if(out_fd<0){
					ret=-11;
					goto error;
				}
				//�ǂݍ���
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
				fioClose(out_fd);
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
	}
	//
	ret=outmc;
error:
	free(buff);
	if(in_fd>0){
		if(hddin) fileXioClose(in_fd);
		else fioClose(in_fd);
	}
	if(out_fd>0){
		fioClose(out_fd);
	}

	return ret;
}

/*===============================================================
// psu�t�@�C���ɃG�N�X�|�[�g
//===============================================================*/
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
					ret=-1;
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
				tmp[42]='.';
				tmp[43]='.';
				tmp[44]='.';
				tmp[45]=0;
			}
			printXY(tmp, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			sprintf(tmp, "%s", file->title);
			if(strlen(tmp)>46){	//title�������Ƃ��ɒZ������
				tmp[42] = '.';
				tmp[43] = '.';
				tmp[44] = '.';
				tmp[45] = 0;
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
					tmp[42]='.';
					tmp[43]='.';
					tmp[44]='.';
					tmp[45]=0;
				}
				printXY(tmp, x, y, setting->color[3], TRUE);
				y += FONT_HEIGHT;
			}
			y += FONT_HEIGHT;
			sprintf(tmp,"��:%s �~:%s", lang->gen_ok, lang->gen_cancel);
			printXY(tmp, x, y, setting->color[3], TRUE);
			// �������
			x = FONT_WIDTH*2;
			y = SCREEN_MARGIN+FONT_HEIGHT*20;
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
		int writeSize;
		unsigned char tmp[2048]="";	//�\���p
		int code;
		char tmppath[MAX_PATH];
		PSU_HEADER psu_header;

		//�I�����ꂽ�t�H���_�̃t���p�X
		sprintf(inpath, "%s%s", path, file->name);

		//�o�͂���psu�t�@�C����
		if(sjisout){
			if(file->title[0]){
				//�t�@�C�����Ɏg���Ȃ�������ϊ�
				title2filename(file->title, tmp);
				//mc�̃t�@�C�����̍ő��32byte
				if(strlen(tmp)>28){	//title�������Ƃ��ɒZ������
					tmp[28] = 0;
					code=tmp[27];
					//2byte������1byte�ڂ����������
					if( (code>=0x81)&&(code<=0x9F) ) tmp[27] = 0;
					if( (code>=0xE0)&&(code<=0xFF) ) tmp[27] = 0;
				}
			}
			else
				strcpy(tmp,file->name);
		}
		else
			strcpy(tmp,file->name);
		//
		if(setting->Exportdir[0])
			sprintf(outpath, "%s%s.psu", setting->Exportdir, tmp);
		else
			sprintf(outpath, "%s%s.psu", path, tmp);
	
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
			if(r<0) return -2;
			strcpy(outpath, tmp);
			outpath[3] = r+'0';
		}
		else if(!strncmp(outpath, "cdfs", 2)){
			ret=-3;
			goto error;
		}

		//psu�t�@�C���I�[�v�� �V�K�쐬
		if(hddout){
			// O_TRUNC �������Ȃ����߁A�I�[�v���O�Ƀt�@�C���폜
			fileXioRemove(outpath);
			out_fd = fileXioOpen(outpath, O_WRONLY|O_TRUNC|O_CREAT, fileMode);
			if(out_fd<0){
				ret=-4;
				goto error;
			}
		}
		else{	//mc mass
			out_fd = fioOpen(outpath, O_WRONLY | O_TRUNC | O_CREAT);
			if(out_fd<0){
				ret=-4;
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
				ret=-5;
				goto error;
			}
		}
		else{
			outsize = fioWrite(out_fd, &psu_header, sizeof(PSU_HEADER));
			if(outsize!=sizeof(PSU_HEADER)){
				ret=-5;
				goto error;
			}
		}
	
		//�t�@�C���w�b�_�ƃt�@�C����������
		dialog_width = FONT_WIDTH*32;
		dialog_height = FONT_HEIGHT*1.5;
		dialog_x = (SCREEN_WIDTH-dialog_width)/2;
		dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
		drawDark();
		itoSwitchFrameBuffers();
		drawDark();
		for(i=0;i<n;i++){
			// �`��J�n
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[0], setting->color[1]);
			// �v���O���X�o�[
			itoSprite(setting->color[1],
				dialog_x+6, dialog_y+6,
				dialog_x+6+(dialog_width-12)*(i*100/n)/100, dialog_y+dialog_height-5, 0);
			sprintf(tmp, "%2d / %2d", i, n);
			printXY(tmp, dialog_x+120, dialog_y+7, setting->color[3], TRUE);
			drawScr();
			//�t�@�C���w�b�_���쐬
			memset(&psu_header, 0, sizeof(PSU_HEADER));
			psu_header.attr = mcDir[i].attrFile;	//�t�@�C�������̓������[�J�[�h�Ɠ����ɂ���
			psu_header.size = mcDir[i].fileSizeByte;
			strcpy(psu_header.name, mcDir[i].name);
			//�t�@�C���w�b�_��������
			if(hddout){
				outsize = fileXioWrite(out_fd, (char*)&psu_header, sizeof(PSU_HEADER));
				if(outsize!=sizeof(PSU_HEADER)){
					ret=-6;
					goto error;
				}
			}
			else{
				outsize = fioWrite(out_fd, &psu_header, sizeof(PSU_HEADER));
				if(outsize!=sizeof(PSU_HEADER)){
					ret=-6;
					goto error;
				}
			}
			//�t�@�C����������
			if(mcDir[i].fileSizeByte>0){
				sprintf(tmppath, "%s/%s", inpath, mcDir[i].name);
				writeSize = (((mcDir[i].fileSizeByte-1)/0x400)+1)*0x400;
				buff = (char*)malloc(writeSize);
				if(buff==NULL){
					ret=-7;
					goto error;
				}
				memset(buff, 0, writeSize);
				//�t�@�C���I�[�v��
				in_fd = fioOpen(tmppath, O_RDONLY);
				if(in_fd<0){
					ret=-7;
					goto error;
				}
				//�ǂݍ���
				fioRead(in_fd, buff, mcDir[i].fileSizeByte);
				//�N���[�Y
				fioClose(in_fd);
				//psu�t�@�C���ɏ�������
				if(hddout){
					outsize = fileXioWrite(out_fd, buff, writeSize);
					if(outsize!=writeSize){
						ret=-8;
						goto error;
					}
				}
				else{
					outsize = fioWrite(out_fd, buff, writeSize);
					if(outsize!=writeSize){
						ret=-8;
						goto error;
					}
				}
				free(buff);
			}
		}
	}
	ret=0;
error:
	free(buff);
	if(in_fd>0) fioClose(in_fd);
	if(out_fd>0){
		if(hddout) fileXioClose(out_fd);
		else fioClose(out_fd);
	}

	if(ret<0){
		// �G�N�X�|�[�g���s�����Ƃ�psu�t�@�C�����폜
		if(!strncmp(outpath, "mc", 2)){
			mcSync(0,NULL,NULL);
			mcDelete(outpath[2]-'0', 0, &outpath[4]);
			mcSync(0, NULL, &r);
		}else if(!strncmp(outpath, "pfs", 3)){
			r = fileXioRemove(outpath);
		}else if(!strncmp(outpath, "mass", 4)){
			r = fioRemove(outpath);
		}
	}
	return ret;
}

/*===============================================================
// �X�N���[���L�[�{�[�h
//===============================================================*/
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
		for(i=0; i<KEY_LEN; i++)
			drawChar(KEY[i],
				KEY_X+FONT_WIDTH*3 + (i%WFONTS)*FONT_WIDTH*3,
				KEY_Y+FONT_HEIGHT*2 + (i/WFONTS)*FONT_HEIGHT,
				setting->color[3]);
		//OK�\��
		x=((KEY_W/4)-FONT_WIDTH*strlen(lang->gen_ok))/2;
		sprintf(tmp, "%s",lang->gen_ok);
		printXY(tmp, KEY_X+KEY_W/4+x, KEY_Y+FONT_HEIGHT*10, setting->color[3], TRUE);
		//CANCEL�\��
		x=((KEY_W/4)-FONT_WIDTH*strlen(lang->gen_cancel))/2;
		sprintf(tmp, "%s",lang->gen_cancel);
		printXY(tmp, KEY_X+KEY_W/2+x, KEY_Y+FONT_HEIGHT*10, setting->color[3], TRUE);
/*		//�J�[�\���\��
		if(sel<=WFONTS*HFONTS)
			x = KEY_X+FONT_WIDTH*2 + (sel%WFONTS)*FONT_WIDTH*3;	//CANCEL�ȊO
		else
			x = KEY_X+FONT_WIDTH*11;	//CANCEL
		y = KEY_Y+FONT_HEIGHT*2 + (sel/WFONTS)*FONT_HEIGHT;
		drawChar('>', x, y, setting->color[3]);
*/
		// �������
		x = FONT_WIDTH*2;
		y = SCREEN_MARGIN+FONT_HEIGHT*20;
		itoSprite(setting->color[0],
			0, y,
			SCREEN_WIDTH, y+FONT_HEIGHT, 0);
		printXY(lang->filer_keyboard_hint, x, y, setting->color[3], TRUE);
		drawScr();
	}
	return 0;
}

/*===============================================================
// �t�@�C�����X�g�ݒ�
//===============================================================*/
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
		strcpy(files[0].name, "mc0:");
		files[0].attr = FIO_S_IFDIR;
		files[0].type=TYPE_OTHER;
		strcpy(files[1].name, "mc1:");
		files[1].attr = FIO_S_IFDIR;
		files[1].type=TYPE_OTHER;
		strcpy(files[2].name, "hdd0:");
		files[2].attr = FIO_S_IFDIR;
		files[2].type=TYPE_OTHER;
		strcpy(files[3].name, "cdfs:");
		files[3].attr = FIO_S_IFDIR;
		files[3].type=TYPE_OTHER;
		strcpy(files[4].name, "mass:");
		files[4].attr = FIO_S_IFDIR;
		files[4].type=TYPE_OTHER;
		nfiles = 5;
		for(i=0; i<nfiles; i++)
			files[i].title[0]=0;
		if(cnfmode==ELF_FILE){
			strcpy(files[nfiles].name, "MISC");
			files[nfiles].attr = FIO_S_IFDIR;
			files[nfiles].type=TYPE_OTHER;
			nfiles++;
		}
		vfreeSpace=FALSE;
	}
	else if(!strcmp(path, "MISC/")){
		strcpy(files[0].name, "..");
		files[0].attr = FIO_S_IFDIR;
		files[0].type=TYPE_OTHER;
		strcpy(files[1].name, "FileBrowser");
		files[1].attr = FIO_S_IFREG;
		files[1].type=TYPE_OTHER;
		strcpy(files[2].name, "PS2Browser");
		files[2].attr = FIO_S_IFREG;
		files[2].type=TYPE_OTHER;
		strcpy(files[3].name, "PS2Disc");
		files[3].attr = FIO_S_IFREG;
		files[3].type=TYPE_OTHER;
		strcpy(files[4].name, "PS2Net");	//PS2Net uLaunchELF3.60
		files[4].attr = FIO_S_IFREG;
		files[4].type=TYPE_OTHER;
		nfiles = 5;
		for(i=0; i<nfiles; i++)
			files[i].title[0]=0;
	}
	else{
		strcpy(files[0].name, "..");
		files[0].attr = FIO_S_IFDIR;
		files[0].type=TYPE_OTHER;
		nfiles = getDir(path, &files[1]) + 1;
		if(strcmp(ext,"*")){	//�t�@�C���}�X�N
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
		//�Q�[���^�C�g���ƃt�@�C���^�C�v�擾
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
			memset(files[i].title,0,65);
			files[i].title[0]=0;
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
				else if( !strncmp(path,"hdd",3)&&strcmp(path,"hdd0:/") ){
					checkELFret = checkELFheader(fullpath); 	//checkELFheader
					mountedParty[0][0]=0;
					if(checkELFret<0)
						files[i].type=TYPE_FILE;
					else
						files[i].type=TYPE_ELF;
					//HDD�̂Ƃ��ă}�E���g
					strcpy(file.name,files[i].name);
					strcpy(file.title,files[i].title);
					file.attr=files[i].attr;
					file.type=files[i].type;
					//
					getHddParty(path,&file,party,dir);
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
		if(!strcmp(path, "hdd0:/"))
			vfreeSpace=FALSE;
		else if(nfiles>1)
			sort(&files[1], 0, nfiles-2);
	}
	
	return nfiles;
}

/*===============================================================
// �C�ӂ̃t�@�C���p�X��Ԃ�
//===============================================================*/
void getFilePath(char *out, int cnfmode)
{
	char path[MAX_PATH], oldFolder[MAX_PATH],
		msg0[MAX_PATH], msg1[MAX_PATH],
		tmp[MAX_PATH], ext[8], *p;
	uint64 color,iconcolor=0;
	FILEINFO files[MAX_ENTRY];
	int nfiles=0, sel=0, top=0, rows=MAX_ROWS;
	int cd=TRUE, up=FALSE, pushed=TRUE;
	int nofnt=FALSE;
	int x, y, y0, y1;
	int i, ret;//, fd;
	size_t size;
	int code;

	if(cnfmode==ELF_FILE)
		strcpy(ext, "elf");
	else if(cnfmode==DIR)
		strcpy(ext, "");
	else if(cnfmode==ANY_FILE)
		strcpy(ext, "*");

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
				sel-=rows/2;
			else if(new_pad & PAD_RIGHT)
				sel+=rows/2;
			else if(new_pad & PAD_TRIANGLE)
				up=TRUE;
			else if(new_pad & PAD_CIRCLE){
				if(files[sel].attr & FIO_S_IFDIR){
					if(!strcmp(files[sel].name,".."))
						up=TRUE;
					else{
						strcat(path, files[sel].name);
						strcat(path, "/");
						cd=TRUE;
					}
				}
				else{
					sprintf(out, "%s%s", path, files[sel].name);
					ret=checkELFheader(out);
					mountedParty[0][0]=0;
					if(ret<0){
						pushed=FALSE;
						sprintf(msg0, lang->filer_not_elf);
						out[0] = 0;
					}
					else{
						strcpy(LastDir, path);
						break;
					}
				}
			}
			//ELF_FILE ELF�I����
			if(cnfmode==ELF_FILE){
				if(new_pad & PAD_SQUARE) {	// �t�@�C���}�X�N�؂�ւ�
					if(!strcmp(ext,"*")) strcpy(ext, "elf");
					else				 strcpy(ext, "*");
					cd=TRUE;
				}
				else if(new_pad & PAD_L1) {	// �^�C�g���\���؂�ւ�
					title = !title;
					cd=TRUE;
				}
				else if(new_pad & PAD_CROSS){	// ���C�����j���[�ɖ߂�
					break;
				}
				else if(new_pad & PAD_SELECT){	// ���C�����j���[�ɖ߂�
					break;
				}
			}
			//DIR �f�B���N�g���I����
			else if(cnfmode==DIR){
				if(new_pad & PAD_START) {
					if( path[0]!=0 && strcmp(path, "hdd0:/")!=0 && strncmp(path, "cdfs", 4)!=0 ){
						strcpy(out, path);
						break;
					}
				}
				else if(new_pad & PAD_L1) {	// �^�C�g���\���؂�ւ�
					title = !title;
					cd=TRUE;
				}
				else if(new_pad & PAD_CROSS){	// ���C�����j���[�ɖ߂�
					break;
				}
				else if(new_pad & PAD_SELECT){	// ���C�����j���[�ɖ߂�
					break;
				}
			}
			//ANY_FILE	�t�@�C���[���[�h	���ׂẴt�@�C�����Ώ�
			else{
				if(new_pad & PAD_R1){	// ���j���[
					drawDark();
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
							ret = ynDialog(tmp);
						}
						else
							ret = ynDialog(lang->filer_deletemarkfiles);
						
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
								vfreeSpace=FALSE;	//�󂫃X�y�[�X�Čv�Z
								cd=TRUE;
							}
							else{
								strcpy(msg0, lang->filer_deletefailed);
								pushed = FALSE;
							}
						}
					}
					else if(ret==RENAME){	// ���l�[��
						drawDark();
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
							vfreeSpace=FALSE;	//�󂫃X�y�[�X�Čv�Z
						}
						cd=TRUE;
					}
					else if(ret==NEWDIR){	// �V�K�t�H���_�쐬
						tmp[0]=0;
						drawDark();
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
						drawMsg(lang->filer_checkingsize);
						if(nmarks==0){
							size=getFileSize(path, &files[sel]);
						}
						else{
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
							vfreeSpace=FALSE;	//�󂫃X�y�[�X�Čv�Z
						}
					}
					else if(ret==IMPORT){	// psu�t�@�C������C���|�[�g
						drawDark();
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
							vfreeSpace=FALSE;	//�󂫃X�y�[�X�Čv�Z
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
				else if(new_pad & PAD_L1) {	// �^�C�g���\���؂�ւ�
					title = !title;
					nofnt = TRUE;
					cd=TRUE;
				}
				else if(new_pad & PAD_SELECT){	// ���C�����j���[�ɖ߂�
					break;
				}
				else if(new_pad & PAD_R2){	//�R���t�B�O
					config(msg0);
					pushed = FALSE;
					if(setting->discControl)
						loadCdModules();
					if(setting->fileicon)
						cd=TRUE;
				}
				else if(new_pad & PAD_R3){
					//FILEICON
					setting->fileicon = !setting->fileicon;
					if(setting->fileicon)
						cd=TRUE;
				}
				else if(new_pad & PAD_L3){
					//FILEICON
					setting->flickerControl = !setting->flickerControl;
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
		// �t�H���_�ړ�
		if(cd){
			nfiles = setFileList(path, ext, files, cnfmode);
			// �󂫗e�ʎ擾
			if(cnfmode==ANY_FILE){
				if(!strncmp(path, "mc", 2)){
					mcGetInfo(path[2]-'0', 0, NULL, &mcfreeSpace, NULL);
				}
				else if(!strncmp(path,"hdd",3)&&strcmp(path,"hdd0:/")){
					freeSpace = 
					fileXioDevctl("pfs0:",PFSCTL_GET_ZONE_FREE,NULL,0,NULL,0)*fileXioDevctl("pfs0:",PFSCTL_GET_ZONE_SIZE,NULL,0,NULL,0);
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
		if(top > nfiles-rows)	top=nfiles-rows;
		if(top < 0)			top=0;
		if(sel >= nfiles)		sel=nfiles-1;
		if(sel < 0)			sel=0;
		if(sel >= top+rows)	top=sel-rows+1;
		if(sel < top)			top=sel;
		
		// ��ʕ`��J�n
		clrScr(setting->color[0]);
		// �t�@�C�����X�g
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<rows; i++){
			if(top+i >= nfiles)
				break;
			if(top+i == sel){
				color = setting->color[2];
				drawChar('>', x, y, color);	//�J�[�\���\��
			}
			else
				color = setting->color[3];

			//�}�[�N�\��
			if(marks[top+i]){
				drawChar('*', x+FONT_WIDTH, y, setting->color[3]);
/*
				//�A���t�@�u�����h�L��
				itoPrimAlphaBlending( TRUE );
				itoSprite(setting->color[2]|0x10000000,
					x+FONT_WIDTH, y,
					x+FONT_WIDTH*57, y+FONT_HEIGHT-2, 0);
				//�A���t�@�u�����h����
				itoPrimAlphaBlending(FALSE);
*/
			}

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
			if(!setting->fileicon)
				//�t�@�C�����̂ݕ\��
				printXY(tmp, x+FONT_WIDTH*2, y, color, TRUE);
			else{
				//�t�@�C�����ƃA�C�R����\��
				if(files[top+i].type!=TYPE_OTHER){
					if(files[top+i].type==TYPE_DIR) iconcolor=setting->color[4];
					else if(files[top+i].type==TYPE_FILE) iconcolor=setting->color[5];
					else if(files[top+i].type==TYPE_PS2SAVE) iconcolor=setting->color[6];
					else if(files[top+i].type==TYPE_ELF) iconcolor=setting->color[7];
					//�A�C�R��
					//printXY("��", x+FONT_WIDTH*2, y, iconcolor, TRUE);
					itoSprite(iconcolor,
						x+FONT_WIDTH*2, y,
						x+FONT_WIDTH*2+FONT_WIDTH, y+16, 0);
				}
				//�t�@�C�����\��
				printXY(tmp, x+FONT_WIDTH*4, y, color, TRUE);
			}
			y += FONT_HEIGHT;
		}
		// �X�N���[���o�[
		if(nfiles > rows){
			drawFrame(FONT_WIDTH*61, SCREEN_MARGIN+FONT_HEIGHT*3,
				FONT_WIDTH*62, SCREEN_MARGIN+FONT_HEIGHT*19,setting->color[1]);
			y0=FONT_HEIGHT*16*((double)top/nfiles);
			y1=FONT_HEIGHT*16*((double)(top+rows)/nfiles);
			itoSprite(setting->color[1],
				FONT_WIDTH*61,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				FONT_WIDTH*62,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// ���b�Z�[�W
		if(pushed) sprintf(msg0, "Path: %s", path);
		// �������
		if(cnfmode==ANY_FILE){
			if(title)
				sprintf(msg1, lang->filer_hint1);
			else
				sprintf(msg1, lang->filer_hint2);
		}
		else if(cnfmode==ELF_FILE){
			if(!strcmp(ext, "*"))
				sprintf(msg1, lang->filer_hint3);
			else
				sprintf(msg1, lang->filer_hint4);
		}
		else if(cnfmode==DIR){
			sprintf(msg1, lang->filer_hint5);
		}
		setScrTmp(msg0, msg1);

		// �t���[�X�y�[�X�\��
		if(!strncmp(path, "mc", 2) && !vfreeSpace && !cnfmode){
			if(mcSync(1,NULL,NULL)!=0){
				freeSpace = mcfreeSpace*1024;
				vfreeSpace=TRUE;
			}
		}
		if(vfreeSpace){
			if(freeSpace >= 1024*1024)
				sprintf(tmp, "[%.1fMB free]", (double)freeSpace/1024/1024);
			else if(freeSpace >= 1024)
				sprintf(tmp, "[%.1fKB free]", (double)freeSpace/1024);
			else
				sprintf(tmp, "[%dB free]", freeSpace);
			ret=strlen(tmp);
			itoSprite(setting->color[0],
				FONT_WIDTH*62-FONT_WIDTH*ret,
				SCREEN_MARGIN+FONT_HEIGHT,
				FONT_WIDTH*62,
				SCREEN_MARGIN+FONT_HEIGHT*2, 0);
			printXY(tmp,
				FONT_WIDTH*62-FONT_WIDTH*ret,
				SCREEN_MARGIN+FONT_HEIGHT,
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
