#include "launchelf.h"
#define	MAX_CNFLINE	4096

typedef struct
{
	char type;	// ͭ���ե饰
	char mem;	// free();
	short len;	// �ԤΥХ��ȥ�����(���Ԥϴޤޤ�)
	short ses;	// ��°���å����
	short keylen;	// ������Ĺ��
	short strlen;	// �ǡ�����Ĺ��
	char *ptr;	// ��Ƭ�ؤΥݥ���
	char *key;	// �����ؤΥݥ���
	char *str;	// �ǡ����ؤΥݥ���
} /*__attribute__((packed))*/ CNF;
// packed: 90112	//=���������®�٤������ˤʤ롩���¹ԥХ��ʥ�Υ�������1KB(���̻�512B)��������
// nonpack: 98304
int cnf_lines;

CNF *cnf = NULL;
char *ses = NULL;
char *buf = NULL;
char *bsm = NULL;
int init = FALSE;
int	sesrw = -1;
int comp = FALSE;
int mode = 0;

enum {
	TYPE_DELETED,
	TYPE_HEADLINE,
	TYPE_COMMENT,
	TYPE_SESSTART,
	TYPE_SESEND,
	TYPE_DATA,
};

static int alloced=0;
void *X_malloc(size_t mallocsize)
{
	void *ret;
	ret = malloc(mallocsize);
	if (ret == NULL)
		printf("cnf: malloc failed (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	else
		printf("cnf: malloc vaild (ofs: %08X, size: %d) [%d]\n", (unsigned int) ret, mallocsize, ++alloced);
	return ret;
}
void X_free(void *mallocdata)
{
	if (mallocdata != NULL) {
		printf("cnf: free vaild (ofs: %08X) [%d]\n", (unsigned int) mallocdata, --alloced);
		free(mallocdata);
	} else 
		printf("cnf: free failed (ofs: %08X)\n", (unsigned int) mallocdata);
	mallocdata = NULL;
}
#define	malloc	X_malloc
#define free	X_free

//----------------------------------------------
// �����
int cnf_init(void)
{
	cnf_free();
	
	cnf = (CNF*)malloc(MAX_CNFLINE*sizeof(CNF));
	if (cnf == NULL) return FALSE;
	memset(cnf, 0, MAX_CNFLINE*sizeof(CNF));
	
	init = TRUE;
	mode = 0;
	
	return TRUE;
}

//----------------------------------------------
// ����
void cnf_free(void)
{
	int line;
	if (bsm != NULL) free(bsm);
	if (buf != NULL) free(buf);
	if (ses != NULL) free(ses);
	if (cnf != NULL) {
		for (line=0;line<cnf_lines;line++)
			if (cnf[line].mem == TRUE)
				free(cnf[line].ptr);
		free(cnf);
	}
	buf = NULL;
	cnf = NULL;
	ses = NULL;
	comp = FALSE;
	sesrw = -1;
	cnf_lines = 0;
	init = FALSE;
	mode = 0;
	return;
}

//----------------------------------------------
// �ɤ߹���
// �����:
//	 0:	�ɤ߹�������
//	-1:	�ɤ߹��߼���
int cnf_load(char *cnfpath)
{
	FILE *fp=NULL;
	int ofs, line, top, i;
	int equp, sesp, nowses;
	char temp[32], *tmp;
	int dsize;
	size_t size;
	
	if (init==FALSE) return -1;
	memset(temp, 0, 32);
	
	//printf("cnf: path: %s\n", cnfpath);
	// �ɤ߹���
	fp = fopen(cnfpath, "rb");
	if (fp == NULL) return -1;
	fread(temp, 1, 32, fp);
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if ((dsize = tek_getsize(temp)) >= 0) {
		comp = TRUE;
		buf = (char*)malloc(dsize);
		if (buf != NULL) {
			tmp = (char*)malloc(size);
			if (tmp != NULL){
				fread(tmp, 1, size, fp);
				if (tek_decomp(tmp, buf, size)<0) {
					free(buf);
					buf = NULL;
				}
				size = dsize;
				free(tmp);
			}
		}
		fclose(fp);
	} else {
		comp = FALSE;
		buf = (char*)malloc(size);
		if (buf != NULL)
			fread(buf, 1, size, fp);
		fclose(fp);
	}
	if (buf == NULL)
		return -1;
	//printf("cnf: size: %d\n", size);
	
	// ����
	// ���Կ��Υ������
	// ����°���å����δ���
	// ��������ǡ����Υݥ��󥿤�Ĺ��
	// ��2�ѥ��ˤ��ƹԿ����¤�ű��(�Ȥ���ʬ����ɬ�פʥ��������)����٤�������(���ξ��ο�����¸����ܤ��ɵ��Ϥɤ�����٤�����)������
	for (ofs=line=top=equp=sesp=0,nowses=-1; ofs<=size; ofs++){
		if ((ofs < size) && (buf[ofs] == 0x3D) && (ofs != top) && (equp == 0))
			equp = ofs;
		if ((ofs < size) && ((sesp < top) || (top == 0)) && (buf[ofs] == 0x5D))
			sesp = ofs;
		if (((ofs < size) && ((buf[ofs] == 13) || (buf[ofs] == 10))) || ((ofs == size) && (ofs > top))) {
			cnf[line].type = TYPE_COMMENT;
			cnf[line].ptr = buf + top;
			cnf[line].len = ofs - top;
			cnf[line].ses = nowses;
			if (ofs == top) {
				cnf[line].type = TYPE_SESEND;
				nowses = -1;
			} else if ((buf[top] == 0x20) || (buf[top] == 0x23) || ((buf[top] == 0x2F) && (buf[top] == 0x2F)) || (buf[top] == 0x3A) || (buf[top] == 0x3B)) {
				cnf[line].type = TYPE_COMMENT;
			} else if ((buf[top] == 0x5B) && (sesp > top)) {
				cnf[line].type = TYPE_SESSTART;
				cnf[line].key = buf + top +1;
				cnf[line].keylen = sesp - top -1;
				nowses = line;
				sesp = 0;
				for(i=line-1; i>=0; i--) {
					if (cnf[i].type == TYPE_COMMENT) {
						cnf[i].type = TYPE_HEADLINE;
						cnf[i].ses = line;
					} else if (cnf[i].type == TYPE_SESEND) {
						break;
					}
				}
			} else if (equp > top) {
				cnf[line].type = TYPE_DATA;
				cnf[line].str = buf + equp +1;
				//printf("[%4d] str=%08X\n", line+1, (int) cnf[line].str);
				cnf[line].strlen = ofs - equp -1;
				cnf[line].key = buf + top;
				cnf[line].keylen = equp - top;
				if (buf[equp-1] == 32) 
					cnf[line].keylen--;
				if (buf[equp+1] == 32) {
					cnf[line].str++;
					cnf[line].strlen--;
				}
				//equp = 0;
			//} else {
			//	printf("cnf: line error! top:%08X, ofs:%08X, len:%d\n", top, ofs, ofs-top);
			}
			if ((ofs < size-1) && (((buf[ofs] == 13) && (buf[ofs+1] == 10)) || ((buf[ofs] == 10) && (buf[ofs+1] == 13))))
				ofs++;
			line++;
			if (line == MAX_CNFLINE) break;
			top = ofs+1;
			equp = 0;
		}
	}
	cnf_lines = line;
	/*
	char *tmp;
	printf("cnf: total %d lines\n", cnf_lines);
	if (cnf_lines > 0) {
		printf("cnf: all lines:\n");
		for(line=0;line<cnf_lines;line++){
			if (cnf[line].type == TYPE_DELETED) {
				printf("%4d: DELETED\n", line+1);
			} else if (cnf[line].type == TYPE_COMMENT) {
				tmp = (char*)malloc(cnf[line].len+1);
				if (tmp != NULL) {
					strncpy(tmp, cnf[line].ptr, cnf[line].len);
					tmp[cnf[line].len] = 0;
					printf("%4d: COMMENT: %s\n", line+1, tmp);
					free(tmp);
				}
			} else if (cnf[line].type == TYPE_SESSTART) {
				tmp = (char*)malloc(cnf[line].keylen+1);
				if (tmp != NULL) {
					strncpy(tmp, cnf[line].key, cnf[line].keylen);
					tmp[cnf[line].keylen] = 0;
					printf("%4d: SESSION: %s\n", line+1, tmp);
					free(tmp);
				}
			} else if (cnf[line].type == TYPE_DATA) {
				tmp = (char*)malloc(cnf[line].keylen+1);
				if (tmp != NULL) {
					strncpy(tmp, cnf[line].key, cnf[line].keylen);
					tmp[cnf[line].keylen] = 0;
					printf("%4d: DATA [=%4d] [%s] = ", line+1, cnf[line].ses+1, tmp);
					free(tmp);
				}
				tmp = (char*)malloc(cnf[line].strlen+1);
				if (tmp != NULL) {
					strncpy(tmp, cnf[line].str, cnf[line].strlen);
					tmp[cnf[line].strlen] = 0;
					printf("[%s]", tmp);
					free(tmp);
				}
				printf("\n");
			}
		}
	}
	//*/
	return 0;
}

//----------------------------------------------
// ��¸
// �����:
//	 0: ��¸����
//	-1:	��¸����
int cnf_save(char *cnfpath)
{
	FILE *fp = NULL;
	int sesw, line;
	char crlf[2] = {13,10};
	if (!init) return -1;
	
	fp = fopen(cnfpath, "wb");
	if (fp == NULL) return -1;
	
	for(sesw=-1;sesw<cnf_lines;sesw++){
		if ((sesw == -1) || ((sesw >= 0) && (cnf[sesw].type == TYPE_SESSTART))) {
			if (sesw >= 0) {
				for(line=0;line<sesw;line++) {
					if ((cnf[line].ses == sesw) && (cnf[line].type == TYPE_HEADLINE)) {
						fwrite(cnf[line].ptr, 1, cnf[line].len, fp);
						fwrite(crlf, 1, 2, fp);
					}
				}
				fwrite(cnf[sesw].ptr, 1, cnf[sesw].len, fp);
				fwrite(crlf, 1, 2, fp);
			}
			for(line=0;line<cnf_lines;line++){
				if ((cnf[line].ses == sesw) && ((cnf[line].type == TYPE_COMMENT) || (cnf[line].type == TYPE_DATA))) {
					fwrite(cnf[line].ptr, 1, cnf[line].len, fp);
					fwrite(crlf, 1, 2, fp);
				}
			}
			fwrite(crlf, 1, 2, fp);
		}
	}
	fclose(fp);
	return 0;
}

//----------------------------------------------
// Ǥ�դΥХåե��ؤ���¸
// �����:
// 0��	�ե����륵����(����)
// ��-1	����(���ӥåȤ�ȿž(NOT)����ȥե����륵����)
size_t cnf_bsave(char *buff, int limit)
{
	size_t cnt=0;
	int sesw, line, i, err=0;
	if (!init) return -1;
	for(sesw=-1;sesw<cnf_lines;sesw++){
		if ((sesw == -1) || ((sesw >= 0) && (cnf[sesw].type == TYPE_SESSTART))) {
			if (sesw >= 0) {
				for(line=0;line<sesw;line++) {
					if ((cnf[line].ses == sesw) && (cnf[line].type == TYPE_HEADLINE)) {
						if (cnt+cnf[line].len+2 > limit) {
							err = 1;
							cnt+= cnf[line].len+2;
						} else {
							for(i=0;i<cnf[line].len;i++)
								buff[cnt++] = cnf[line].ptr[i];
							buff[cnt++] = 13;
							buff[cnt++] = 10;
						}
					}
				}
				if (cnt+cnf[sesw].len+2 > limit) {
					err = 1;
					cnt+= cnf[sesw].len+2;
				} else {
					for(i=0;i<cnf[sesw].len;i++)
						buff[cnt++] = cnf[sesw].ptr[i];
					buff[cnt++] = 13;
					buff[cnt++] = 10;
				}
			}
			for(line=0;line<cnf_lines;line++){
				if ((cnf[line].ses == sesw) && ((cnf[line].type == TYPE_COMMENT) || (cnf[line].type == TYPE_DATA))) {
					if (cnt+cnf[line].len+2 > limit) {
						err = 1;
						cnt+= cnf[line].len+2;
					} else {
						for(i=0;i<cnf[line].len;i++)
							buff[cnt++] = cnf[line].ptr[i];
						buff[cnt++] = 13;
						buff[cnt++] = 10;
					}
				}
			}
			if (cnt+2 > limit) {
				err = 1;
				cnt+= 2;
			} else {
				buff[cnt++] = 13;
				buff[cnt++] = 10;
			}
		}
	}
	if (err && limit!=0) return -(cnt+1);
	return cnt;
}

//----------------------------------------------
// ʸ����ǡ����μ���
// �����:
//	 0:	��������
//	-1:	���������Ƥ��ʤ�
//	-2:	������Ĺ��
//	-3:	�������ʤ�
int cnf_getstr(const char *key, char *str, const char *def)
{
	int line, keylen, ret;
	
	if (!init) return -1;
	
	keylen = strlen(key);
	ret = -3;
	strcpy(str, def);
	for (line=0;line<cnf_lines;line++){
		if ((cnf[line].ses == sesrw) && (cnf[line].type == TYPE_DATA) && (cnf[line].keylen == keylen) && (!strncmp(cnf[line].key, key, keylen))) {
			strncpy(str, cnf[line].str, cnf[line].strlen);
			str[cnf[line].strlen] = 0;
			ret = 0;
			break;
		}
	}
	return ret;
}

//----------------------------------------------
// ʸ����ǡ������ѹ�
// �����:
//	 1:	�����ɲ�����
//	 0:	�ǡ����ѹ�����
//	-1:	���������Ƥ��ʤ�
//	-2:	������Ĺ��
//	-3:	ʸ����Ĺ��
//	-4:	�����ɲü���
int cnf_setstr(const char *key, char *str)
{
	int line, keylen, srclen;
	char *tmp;
	if (!init) return -1;
	
	keylen = strlen(key);
	srclen = strlen(str);
	for (line=0;line<cnf_lines;line++){
		if ((cnf[line].ses == sesrw) && (cnf[line].type == TYPE_DATA) && (cnf[line].keylen == keylen) && (!strncmp(cnf[line].key, key, keylen))) {
			if (srclen <= cnf[line].strlen) {
				strcpy(cnf[line].str, str);
				cnf[line].len-= cnf[line].strlen - srclen;
				cnf[line].strlen = srclen;
				return 0;
			} else {
				tmp = (char*)malloc(keylen + srclen +3);
				if (tmp != NULL) {
					if (cnf[line].mem == TRUE) free(cnf[line].ptr);
					cnf[line].ptr = tmp;
					cnf[line].mem = TRUE;
					if (cnf[line].key+keylen+1 == cnf[line].str) {
						cnf[line].len = keylen + srclen +1;
						cnf[line].str = tmp + keylen +1;
						tmp[keylen] = 0x3D;
					} else {
						cnf[line].len = keylen + srclen +3;
						cnf[line].str = tmp + keylen +3;
						strncpy(tmp + keylen, " = ", 3);
					}
					cnf[line].key = tmp;
					cnf[line].strlen = srclen;
					strncpy(cnf[line].key, key, keylen);
					strncpy(cnf[line].str, str, srclen);
					return 0;
				}
				return -3;
			}
		}
	}
	if (cnf_lines < MAX_CNFLINE) {
		if (mode & 1)
			tmp = (char*)malloc(keylen + srclen +3);
		else
			tmp = (char*)malloc(keylen + srclen +1);
		if (tmp != NULL) {
			line = cnf_lines++;
			cnf[line].type = TYPE_DATA;
			cnf[line].ses = sesrw;
			cnf[line].mem = TRUE;
			cnf[line].ptr = tmp;
			cnf[line].key = tmp;
			cnf[line].keylen = keylen;
			cnf[line].strlen = srclen;
			strncpy(tmp, key, keylen);
			if (mode & 1) {
				cnf[line].len = keylen + srclen +3;
				cnf[line].str = tmp + keylen +3;
				strncpy(tmp + keylen, " = ", 3);
				strncpy(tmp + keylen +3, str, srclen);
			} else {
				cnf[line].len = keylen + srclen +1;
				cnf[line].str = tmp + keylen +1;
				tmp[keylen] = 0x3D;
				strncpy(tmp + keylen +1, str, srclen);
			}
			return 1;
		}
		return -3;
	}
	return -4;
}

//----------------------------------------------
// ������¸�ߤ���
int cnf_delkey(const char *key)
{
	int line, keylen;
	if (!init) return -1;
	
	keylen = strlen(key);
	for (line=0;line<cnf_lines;line++){
		if ((cnf[line].ses == sesrw) && (cnf[line].type == TYPE_DATA) && (cnf[line].keylen == keylen) && (!strncmp(cnf[line].key, key, keylen))) {
			if (cnf[line].mem == TRUE) {
				free(cnf[line].ptr);
				cnf[line].mem = FALSE;
			}
			cnf[line].type = TYPE_DELETED;
			return 1;
		}
	}
	return -3;
}

//----------------------------------------------
// �����������륻�å������ѹ�
int cnf_session(const char *name)
{
	int line, keylen;
	char *tmp;
	if (!init) return -1;
	if (name == NULL) {
		sesrw = -1;
		return 0;
	}
	keylen = strlen(name);
	sesrw = -1;
	for (line=0;line<cnf_lines;line++) {
		if ((cnf[line].type == TYPE_SESSTART) && (cnf[line].keylen == keylen) && (!strncmp(cnf[line].key, name, keylen))) {
			sesrw = line;
			return 0;
		}
	}
	if (cnf_lines < MAX_CNFLINE) {
		tmp = (char*)malloc(keylen +2);
		if (tmp != NULL) {
			line = cnf_lines++;
			cnf[line].type = TYPE_SESSTART;
			cnf[line].ses = -1;
			cnf[line].mem = TRUE;
			cnf[line].ptr = tmp;
			cnf[line].len = keylen +2;
			cnf[line].key = tmp+1;
			cnf[line].keylen = keylen;
			strncpy(tmp +1, name, keylen);
			tmp[0] = 0x5B; tmp[keylen +1] = 0x5D;
			sesrw = line;
			return 1;
		}
		return -3;
	}
	return -4;
}

//----------------------------------------------
// cnf�⡼������
// mode.b0	���������ζ��ڤ�ʸ�������Ⱦ�ѥ��ڡ���(=0:�ʤ�,=1:����)
//		b1	����ե����붯��������¸�ե饰(=0:��ư,=1:����)
//		b2-	̤����(0�ˤ���)
int cnf_mode(int flag)
{
	mode = flag;
	return 0;
}

