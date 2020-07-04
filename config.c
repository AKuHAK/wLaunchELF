#include "launchelf.h"

enum
{
	DEF_TIMEOUT = 10,
	DEF_FILENAME = TRUE,
	DEF_COLOR1 = ITO_RGBA(30,30,50,0),		//�w�i
	DEF_COLOR2 = ITO_RGBA(64,64,80,0),		//�g
	DEF_COLOR3 = ITO_RGBA(192,192,192,0),	//�����̕����F
	DEF_COLOR4 = ITO_RGBA(128,128,128,0),	//�ʏ�̕����F
	DEF_SCREEN_X = 160,
	DEF_SCREEN_Y = 55,
	DEF_DISCCONTROL = TRUE,
	DEF_INTERLACE = TRUE,	//�t���b�J�[�R���g���[��
	
	DEFAULT=0,
	TIMEOUT=0,
	DISCCONTROL,
	FILENAME,
	SCREEN,
	COLOR1=0,
	OK=14,
	CANCEL
};

SETTING *setting, *tmpsetting;

////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////
// LAUNCHELF.CNF �ɐݒ��ۑ�����
void saveConfig(char *mainMsg)
{
	int i, ret, fd, size;
	const char LF[3]={0x0D, 0x0A, 0};
	char c[MAX_PATH], tmp[22][MAX_PATH];
	char* p;
	
	// �ݒ���������Ɋi�[
	for(i=0; i<12; i++)
		strcpy(tmp[i], setting->dirElf[i]);
	sprintf(tmp[12], "%d", setting->timeout);
	sprintf(tmp[13], "%d", setting->filename);
	for(i=0; i<4; i++)
		sprintf(tmp[14+i], "%lu", setting->color[i]);
	sprintf(tmp[18], "%d", setting->screen_x);
	sprintf(tmp[19], "%d", setting->screen_y);
	sprintf(tmp[20], "%d", setting->discControl);
	sprintf(tmp[21], "%d", setting->interlace);
	
	p = (char*)malloc(sizeof(SETTING));
	p[0]=0;
	size=0;
	for(i=0; i<22; i++){
		strcpy(c, tmp[i]);
		strcat(c, LF);
		strcpy(p+size, c);
		size += strlen(c);
	}
	// LaunchELF�̃f�B���N�g����CNF����������LaunchELF�̃f�B���N�g���ɃZ�[�u
	strcpy(c, LaunchElfDir);
	strcat(c, "LbF.CNF");
	if((fd=fioOpen(c, O_RDONLY)) >= 0)
		fioClose(fd);
	else{
		if(!strncmp(LaunchElfDir, "mc", 2))
			sprintf(c, "mc%d:/SYS-CONF", LaunchElfDir[2]-'0');
		else
			sprintf(c, "mc%d:/SYS-CONF", CheckMC());
		// SYS-CONF����������SYS-CONF�ɃZ�[�u
		if((fd=fioDopen(c)) >= 0){
			fioDclose(fd);
			strcat(c, "/LbF.CNF");
		// SYS-CONF���Ȃ�������LaunchELF�̃f�B���N�g���ɃZ�[�u
		}else{
			strcpy(c, LaunchElfDir);
			strcat(c, "LbF.CNF");
		}
	}
	strcpy(mainMsg,"Save Failed");
	// ��������
	if((fd=fioOpen(c,O_CREAT|O_WRONLY|O_TRUNC)) < 0){
		return;
	}
	ret = fioWrite(fd,p,size);
	if(ret==size) sprintf(mainMsg, "Save Config (%s)", c);
	fioClose(fd);
	free(p);
}

////////////////////////////////////////////////////////////////////////
// LAUNCHELF.CNF ����ݒ��ǂݍ���
void loadConfig(char *mainMsg)
{
	int i, j, k, fd, len, mcport;
	size_t size;
	char path[MAX_PATH], tmp[22][MAX_PATH], *p;
	
	setting = (SETTING*)malloc(sizeof(SETTING));
	// LaunchELF�����s���ꂽ�p�X����ݒ�t�@�C�����J��
	sprintf(path, "%s%s", LaunchElfDir, "LbF.CNF");
	if(!strncmp(path, "cdrom", 5)) strcat(path, ";1");
	fd = fioOpen(path, O_RDONLY);
	// �J���Ȃ�������ASYS-CONF�̐ݒ�t�@�C�����J��
	if(fd<0) {
		if(!strncmp(LaunchElfDir, "mc", 2))
			mcport = LaunchElfDir[2]-'0';
		else
			mcport = CheckMC();
		if(mcport==1 || mcport==0){
			sprintf(path, "mc%d:/SYS-CONF/LbF.CNF", mcport);
			fd = fioOpen(path, O_RDONLY);
		}
	}
	// �ǂ̃t�@�C�����J���Ȃ������ꍇ�A�ݒ������������
	if(fd<0) {
		for(i=0; i<12; i++)
			setting->dirElf[i][0] = 0;
		setting->timeout = DEF_TIMEOUT;
		setting->filename = DEF_FILENAME;
		setting->color[0] = DEF_COLOR1;
		setting->color[1] = DEF_COLOR2;
		setting->color[2] = DEF_COLOR3;
		setting->color[3] = DEF_COLOR4;
		setting->screen_x = DEF_SCREEN_X;
		setting->screen_y = DEF_SCREEN_Y;
		setting->discControl = DEF_DISCCONTROL;
		setting->interlace = DEF_INTERLACE;
		mainMsg[0] = 0;
	} else {
		// �ݒ�t�@�C�����������ɓǂݍ���
		size = fioLseek(fd, 0, SEEK_END);
		printf("size=%d\n", size);
		fioLseek(fd, 0, SEEK_SET);
		p = (char*)malloc(sizeof(size));
		fioRead(fd, p, size);
		fioClose(fd);
		
		// �v22�s�̃e�L�X�g��ǂݍ���
		// 12�s�ڂ܂ł̓{�^���Z�b�e�B���O
		// 13�s�ڂ� TIMEOUT �l
		// 14�s�ڂ� PRINT ONLY FILENAME �̐ݒ�l
		// 15�s�ڂ̓J���[1
		// 16�s�ڂ̓J���[2
		// 17�s�ڂ̓J���[3
		// 18�s�ڂ̓J���[4
		// 19�s�ڂ̓X�N���[��X
		// 20�s�ڂ̓X�N���[��Y
		// 21�s�ڂ̓f�B�X�N�R���g���[��
		// 22�s�ڂ̓C���^�[���[�X
		for(i=j=k=0; i<size; i++) {
			if(p[i]==0x0D && p[i+1]==0x0A) {
				if(i-k<MAX_PATH) {
					p[i]=0;
					strcpy(tmp[j++], &p[k]);
				} else
					break;
				if(j>=22)
					break;
				k=i+2;
			}
		}
		while(j<22)
			tmp[j++][0] = 0;
		// �{�^���Z�b�e�B���O
		for(i=0; i<12; i++) {
			// v3.01�ȑO�̃o�[�W�����Ƃ̏�ʌ݊�
			if(tmp[i][0] == '/')
				sprintf(setting->dirElf[i], "mc:%s", tmp[i]);
			else
				strcpy(setting->dirElf[i], tmp[i]);
		}
		// TIMEOUT�l�̐ݒ�
		if(tmp[12][0]) {
			setting->timeout = 0;
			len = strlen(tmp[12]);
			i = 1;
			while(len-- != 0) {
				setting->timeout += (tmp[12][len]-'0') * i;
				i *= 10;
			}
		} else
			setting->timeout = DEF_TIMEOUT;
		// PRINT ONLY FILENAME �̐ݒ�
		if(tmp[13][0])
			setting->filename = tmp[13][0]-'0';
		else
			setting->filename = DEF_FILENAME;
		// �J���[1����4�̐ݒ�
		if(tmp[14][0]) {
			for(i=0; i<4; i++) {
				setting->color[i] = 0;
				len = strlen(tmp[14+i]);
				j = 1;
				while(len-- != 0) {
					setting->color[i] += (tmp[14+i][len]-'0') * j;
					j *= 10;
				}
			}
		} else {
			setting->color[0] = DEF_COLOR1;
			setting->color[1] = DEF_COLOR2;
			setting->color[2] = DEF_COLOR3;
			setting->color[3] = DEF_COLOR4;
		}
		// �X�N���[��X�̐ݒ�
		if(tmp[18][0]) {
			setting->screen_x = 0;
			j = strlen(tmp[18]);
			for(i=1; j; i*=10)
				setting->screen_x += (tmp[18][--j]-'0')*i;
		} else
			setting->screen_x = DEF_SCREEN_X;
		// �X�N���[��Y�̐ݒ�
		if(tmp[19][0]) {
			setting->screen_y = 0;
			j = strlen(tmp[19]);
			for(i=1; j; i*=10)
				setting->screen_y += (tmp[19][--j]-'0')*i;
		} else
			setting->screen_y = DEF_SCREEN_Y;
		// �f�B�X�N�R���g���[���̐ݒ�
		if(tmp[20][0])
			setting->discControl = tmp[20][0]-'0';
		else
			setting->discControl = DEF_DISCCONTROL;
		// �C���^�[���[�X�̐ݒ�
		if(tmp[21][0])
			setting->interlace = tmp[21][0]-'0';
		else
			setting->interlace = DEF_INTERLACE;
		
		free(p);
		sprintf(mainMsg, "Load Config (%s)", path);
	}
	return;
}

////////////////////////////////////////////////////////////////////////
// Config���
void config(char *mainMsg)
{
	char c[MAX_PATH];
	int i;
	int s;	//select_y
	int x, y;
	int page=0;
	int r,g,b;
	int s_x=0;	//select_x
	
	tmpsetting = setting;
	setting = (SETTING*)malloc(sizeof(SETTING));
	*setting = *tmpsetting;
	
	s=0;
	while(1)
	{
		// ����
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad & PAD_UP){	//��
				s--;
				if(s<0) s=15;
				if(page==0){
					if(s==13) s=11;
				}
				else if(page==1){
					if(s==13) s=2;
				}
				else if(page==2){
					if(s==13) s=7;
				}
			}
			else if(new_pad & PAD_DOWN){	//��
				s++;
				if(s>15) s=0;
				if(page==0){
					if(s==12) s=OK;	//OK�܂ňړ�
				}
				else if(page==1){
					if(s==3) s=OK;		//OK�܂ňړ�
				}
				else if(page==2){
					if(s==8) s=OK;		//OK�܂ňړ�
				}
			}
			else if(new_pad & PAD_LEFT){	//��
				if(page==0){
					s=DEFAULT;	//DEFAULT�܂ňړ�
				}
				else if(page==1){
					s=TIMEOUT;		//TIMEOUT�܂ňړ�
				}
				else if(page==2){
					if(s<4){
						s_x--;
						if(s_x<0) s_x=2;
					}
					else
						s=COLOR1;		//COLOR1�܂ňړ�
				}
			}
			else if(new_pad & PAD_RIGHT){	//�E
				if(page==2){
					if(s<4){
						s_x++;
						if(s_x>2) s_x=0;
					}
					else
						s=OK;	//OK
				}
				else
					s=OK;	//OK
			}
			else if(new_pad & PAD_CROSS){	//�~
				if(page==0){
					if(s<12)
					setting->dirElf[s][0]=0;
				}
				if(page==1){
					if(s==0){
						if(setting->timeout > 0) setting->timeout--;
					}
				}
				if(page==2){
					if(s<4){
						r = setting->color[s] & 0xFF;
						g = setting->color[s] >> 8 & 0xFF;
						b = setting->color[s] >> 16 & 0xFF;
						if(s_x==0){
							if(r>0) r--;
						}
						if(s_x==1){
							if(g>0) g--;
						}
						if(s_x==2){
							if(b>0) b--;
						}
						setting->color[s] = ITO_RGBA(r, g, b, 0);
						if(s == 0) itoSetBgColor(setting->color[0]);
					}
					if(s==4){	//SCREEN X
						if(setting->screen_x > 0) {
							setting->screen_x--;
							screen_env.screen.x = setting->screen_x;
							itoSetScreenPos(setting->screen_x, setting->screen_y);
						}
					}
					if(s==5){	//SCREEN Y
						if(setting->screen_y > 0) {
							setting->screen_y--;
							screen_env.screen.y = setting->screen_y;
							itoSetScreenPos(setting->screen_x, setting->screen_y);
						}
					}
				}
			}
			else if(new_pad & PAD_CIRCLE){	//��
				if(page==0){
					if(s<OK){
						getFilePath(setting->dirElf[s], TRUE);
						if(!strncmp(setting->dirElf[s], "mc", 2)){
							sprintf(c, "mc%s", &setting->dirElf[s][3]);
							strcpy(setting->dirElf[s], c);
						}
					}
				}
				if(page==1){
					if(s==TIMEOUT)
						setting->timeout++;
					else if(s==FILENAME)
						setting->filename = !setting->filename;
					else if(s==DISCCONTROL)
						setting->discControl = !setting->discControl;
				}
				if(page==2){
					if(s<4){
						r = setting->color[s] & 0xFF;
						g = setting->color[s] >> 8 & 0xFF;
						b = setting->color[s] >> 16 & 0xFF;
						if(s_x==0){
							if(r<255) r++;
						}
						if(s_x==1){
							if(g<255) g++;
						}
						if(s_x==2){
							if(b<255) b++;
						}
						setting->color[s] = ITO_RGBA(r, g, b, 0);
						if(s == 0) itoSetBgColor(setting->color[0]);
					}
					else if(s==4) {	//SCREEN X
						setting->screen_x++;
						screen_env.screen.x = setting->screen_x;
						itoSetScreenPos(setting->screen_x, setting->screen_y);
					}
					else if(s==5) {	//SCREEN Y
						setting->screen_y++;
						screen_env.screen.y = setting->screen_y;
						itoSetScreenPos(setting->screen_x, setting->screen_y);
					}
					else if(s==6) {	//�t���b�J�[�R���g���[��
						setting->interlace = !setting->interlace;
					}
					else if(s==7) {	//INIT
						setting->color[0] = DEF_COLOR1;
						setting->color[1] = DEF_COLOR2;
						setting->color[2] = DEF_COLOR3;
						setting->color[3] = DEF_COLOR4;
						setting->screen_x = DEF_SCREEN_X;
						setting->screen_y = DEF_SCREEN_Y;
						setting->interlace = DEF_INTERLACE;
						//
						screen_env.screen.x = setting->screen_x;
						screen_env.screen.y = setting->screen_y;
						screen_env.interlace = ITO_INTERLACE;
						itoGsReset();
						itoGsEnvSubmit(&screen_env);
						itoSetBgColor(setting->color[0]);
					}
				}
				if(s==OK){
					free(tmpsetting);
					saveConfig(mainMsg);
					break;
				}
				if(s==CANCEL){
					free(setting);
					setting = tmpsetting;
					screen_env.screen.x = setting->screen_x;
					screen_env.screen.y = setting->screen_y;
					screen_env.interlace = ITO_INTERLACE;	//setting->interlace;
					itoGsReset();
					itoGsEnvSubmit(&screen_env);
					itoSetBgColor(setting->color[0]);
					mainMsg[0] = 0;
					break;
				}
			}
			else if(new_pad & PAD_L1){
				page--;
				s_x=0;
				if(page<0) page=2;
				if(page==0){
					s=DEFAULT;	//DEFAULT�܂ňړ�
				}
				else if(page==1){
					s=TIMEOUT;		//TIMEOUT�܂ňړ�
				}
				else if(page==2){
					s=COLOR1;		//COLOR1�܂ňړ�
				}
			}
			else if(new_pad & PAD_R1){
				page++;
				s_x=0;
				if(page>2) page=0;
				if(page==0){
					s=DEFAULT;	//DEFAULT�܂ňړ�
				}
				else if(page==1){
					s=TIMEOUT;		//TIMEOUT�܂ňړ�
				}
				else if(page==2){
					s=COLOR1;		//COLOR1�܂ňړ�
				}
			}
			else if(new_pad & PAD_SELECT){
				s=CANCEL;	//CANCEL
			}
			else if(new_pad & PAD_START){
				s=OK;	//OK
			}
		}
		
		// ��ʕ`��J�n
		clrScr(setting->color[0]);

		//�y�[�W�J�[�\���̘g
		drawFrame(FONT_WIDTH*1.5, SCREEN_MARGIN+FONT_HEIGHT,
			FONT_WIDTH*62.5, SCREEN_MARGIN+FONT_HEIGHT*2,
			setting->color[1]);

		//�y�[�W�J�[�\��
		itoSprite(setting->color[1],
			FONT_WIDTH*8+FONT_WIDTH*16*page, SCREEN_MARGIN+FONT_HEIGHT,
			FONT_WIDTH*8+FONT_WIDTH*16+FONT_WIDTH*16*page, SCREEN_MARGIN+FONT_HEIGHT*2, 0);
		printXY("<L1  BUTTON SETTING       MISC       SCREEN SETTING  R1>",
			FONT_WIDTH*4, SCREEN_MARGIN+FONT_HEIGHT+2, setting->color[3], TRUE);

		// �g�̒�
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;

		//�y�[�W1
		if(page==0){
			for(i=0; i<12; i++){
				switch(i){
				case 0:
					strcpy(c,"  DEFAULT: ");
					break;
				case 1:
					strcpy(c,"  ��     : ");
					break;
				case 2:
					strcpy(c,"  �~     : ");
					break;
				case 3:
					strcpy(c,"  ��     : ");
					break;
				case 4:
					strcpy(c,"  ��     : ");
					break;
				case 5:
					strcpy(c,"  L1     : ");
					break;
				case 6:
					strcpy(c,"  R1     : ");
					break;
				case 7:
					strcpy(c,"  L2     : ");
					break;
				case 8:
					strcpy(c,"  R2     : ");
					break;
				case 9:
					strcpy(c,"  L3     : ");
					break;
				case 10:
					strcpy(c,"  R3     : ");
					break;
				case 11:
					strcpy(c,"  START  : ");
					break;
				}
				strcat(c, setting->dirElf[i]);
				printXY(c, x, y, setting->color[3], TRUE);
				y += FONT_HEIGHT;
			}
		}
		//�y�[�W2
		if(page==1){
			sprintf(c, "  TIMEOUT: %d", setting->timeout);
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			
			if(setting->discControl)
				sprintf(c, "  DISC CONTROL: ON");
			else
				sprintf(c, "  DISC CONTROL: OFF");
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			
			if(setting->filename)
				sprintf(c, "  PRINT ONLY FILENAME: ON");
			else
				sprintf(c, "  PRINT ONLY FILENAME: OFF");
			printXY(c, x, y, setting->color[3], TRUE);
		}
		//�y�[�W3
		if(page==2){
			for(i=0;i<4;i++){
				r = setting->color[i] & 0xFF;
				g = setting->color[i] >> 8 & 0xFF;
				b = setting->color[i] >> 16 & 0xFF;
				sprintf(c, "��");	//�F�̃v���r���[
				printXY(c, x+FONT_WIDTH*32, y, setting->color[i], TRUE);
				sprintf(c, "  COLOR%d:  R:%3d  G:%3d  B:%3d", i+1, r, g, b);
				printXY(c, x, y, setting->color[3], TRUE);
				y += FONT_HEIGHT;
			}
			sprintf(c, "  SCREEN X: %3d", setting->screen_x );
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			sprintf(c, "  SCREEN Y: %3d", setting->screen_y );
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			if(setting->interlace)
				sprintf(c, "  FLICKER CONTROL: ON");
			else
				sprintf(c, "  FLICKER CONTROL: OFF");
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			printXY("  INIT", x, y, setting->color[3], TRUE);
		}

		//OK��CANCEL
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*17;
		printXY("  OK", x, y, setting->color[3], TRUE);
		y += FONT_HEIGHT;
		printXY("  CANCEL", x, y, setting->color[3], TRUE);

		//�J�[�\��
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3+s*FONT_HEIGHT;
		if(page==2)
			if(s<4) x = FONT_WIDTH*13 + FONT_WIDTH*s_x*7;
		drawChar('>', x, y, setting->color[3]);

		//�������
		if(page==0){
			if (s < OK)
				sprintf(c, "��:Edit �~:Clear");
			else
				sprintf(c, "��:OK");
		}
		if(page==1){
			if(s==TIMEOUT)
				sprintf(c, "��:Add �~:Away");
			else if(s==FILENAME)
				sprintf(c, "��:Change");
			else if(s==DISCCONTROL)
				sprintf(c, "��:Change");
			else
				sprintf(c, "��:OK");
		}
		if(page==2){
			if(s<6)
				sprintf(c, "��:Add �~:Away");
			else if(s==6)
				sprintf(c, "��:Change");
			else
				sprintf(c, "��:OK");
		}
		setScrTmp("", c);
		drawScr();
	}
	
	return;
}
