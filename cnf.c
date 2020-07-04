#include "launchelf.h"

typedef struct
{
	char cnf_str[MAX_CNF_LINE][MAX_CNF_DATASTR];
} CNF;

int cnf_lines;

CNF *cnf = NULL;
int init = FALSE;

//-------------------------------------------------
//������
int cnf_init(void)
{
	//
	cnf_free();
	cnf = (CNF*)malloc(sizeof(CNF));
	if(cnf==NULL) return FALSE;

	memset(cnf, 0, sizeof(CNF));
	cnf_lines = 0;
	init = TRUE;

	return TRUE;
}

//-------------------------------------------------
//�J��
void cnf_free(void)
{
	//
	if(init==TRUE) free(cnf);
	init = FALSE;

	return;
}

//-------------------------------------------------
//���[�h
//�߂�l
//  0:���[�h����
// -1:���[�h���s
int cnf_load(char* cnfpath)
{
	FILE *fp=NULL;
	int i, j, k;
	size_t size;
	char *cnftmp;

	//
	if(init==FALSE) return -1;

	//�I�[�v��
	fp = fopen(cnfpath, "rb");
	if(fp==NULL) return -1;

	//
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	cnftmp = (char*)malloc(size);
	fread(cnftmp, sizeof(char), size, fp);

	//�N���[�Y
	fclose(fp);

	for(i=j=k=0; i<size; i++){
		if(cnftmp[i]==0x0D && cnftmp[i+1]==0x0A){
			if(i-k<MAX_CNF_DATASTR){
				cnftmp[i]=0;
				strcpy(cnf->cnf_str[j++], &cnftmp[k]);
			}
			else
				break;
			if(j>=MAX_CNF_LINE)
				break;
			k = i+2;
		}
	}
	cnf_lines = j;

	free(cnftmp);
	return 0;
}

//-------------------------------------------------
//�Z�[�u
//�߂�l
//  0:�Z�[�u����
// -1:�Z�[�u���s
int cnf_save(char* cnfpath)
{
	FILE *fp = NULL;
	int i;
	char tmp[MAX_CNF_DATASTR+3];

	//
	if(init==FALSE) return -1;

	fp = fopen(cnfpath, "wb");
	if(fp==NULL) return -1;

	for(i=0;i<cnf_lines;i++){
		sprintf(tmp, "%s\r\n", cnf->cnf_str[i]);
		fwrite(tmp, sizeof(char), strlen(tmp), fp);
	}

	fclose(fp);
	return 0;
}

//-------------------------------------------------
//������f�[�^���擾
//�߂�l
//  0:���� �f�[�^�擾����
// -1:���s ����������Ă��Ȃ�
// -2:���s �L�[������
// -3:���s �L�[���Ȃ�
int cnf_getstr(const char* key, char *str, const char* Default)
{
	int i,ret;
	char keyname[MAX_CNF_KEYSTR+1];

	if(init==FALSE) return -1;

	//�L�[������
	if(strlen(Default)>MAX_CNF_KEYSTR) return -2;

	strcpy(str, Default);

	strcpy(keyname, key);
	strcat(keyname, "=");
	ret = 0;
	for(i=0;i<cnf_lines;i++){
		if(!strncmp(cnf->cnf_str[i], keyname, strlen(keyname))){
			strcpy(str, cnf->cnf_str[i]+strlen(keyname));
			ret=1;
			break;
		}
	}
	if(ret==1)
		return 0;
	else
		return -3;
}

//-------------------------------------------------
//������f�[�^��ύX
//�߂�l
//  1:���� �L�[�ǉ�����
//  0:���� �f�[�^�ύX����
// -1:���s ����������Ă��Ȃ�
// -2:���s �L�[������
// -3:���s �����񂪒���
// -4:���s �L�[�ǉ����s
int cnf_setstr(const char* key, char *str)
{
	int i;
	char keyname[MAX_CNF_KEYSTR+1];

	if(init==FALSE) return -1;

	//�L�[������
	if(strlen(key)>MAX_CNF_KEYSTR) return -2;
	//�����񂪒���
	if(strlen(key)+strlen(str)+1>MAX_CNF_DATASTR) return -3;

	strcpy(keyname, key);
	strcat(keyname, "=");
	for(i=0;i<MAX_CNF_LINE;i++){
		//�L�[������������ύX
		if(!strncmp(cnf->cnf_str[i], keyname, strlen(keyname))){
			sprintf(cnf->cnf_str[i], "%s=%s", key, str);
			return 0;
		}
	}

	//�L�[��������Ȃ�������ǉ�
	if(cnf_lines>=MAX_CNF_LINE)
		//�ǉ��ł��Ȃ�
		return -4;

	sprintf(cnf->cnf_str[cnf_lines], "%s=%s", key, str);
	cnf_lines++;

	return 1;
}

/*
//-------------------------------------------------
int cnf_debug(int id, char *data)
{
	if(init==FALSE) return -1;

	if(data!=NULL){
		if(id>=0 && id<cnf_lines)
			strcpy(data, cnf->cnf_str[id]);
	}
	return cnf_lines;
}
*/
