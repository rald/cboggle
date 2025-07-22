#include "trie.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

char *dice[]={
	"AAEEGN",
	"ABBJOO",
	"ACHOPS",
	"AFFKPS",
	"AOOTTW",
	"CIMOTU",
	"DEILRX",
	"DELRVY",
	"DISTTY",
	"EEGHNW",
	"EEINSU",
	"EHRTVW",
	"EIOSST",
	"ELRTTY",
	"HIMNUQ",
	"HLNNRZ",
};

char board[17];
bool graph[16];
char **words=NULL;
size_t nwords=0;

#include <ctype.h>
#include <string.h>

static void trim(char *s) {
  char *e;
  for(e=s+strlen(s);e>s && isspace((unsigned char)*(e-1)); e--);
  *e = '\0';
}

void Boggle_WordAdd(char ***words,size_t *nwords,char *word) {
	(*words)=realloc(*words,sizeof(*words)*(*nwords+1));
	(*words)[(*nwords)++]=strdup(word);
}

void Boggle_WordFree(char ***words,size_t *nwords) {
	for(int i=0;i<(*nwords);i++) {
		free((*words)[i]);
		(*words)[i]=NULL;
	}
	free(*words);
	(*words)=NULL;
	(*nwords)=0;
}

ssize_t Boggle_WordFind(char **words,size_t nwords,char *word) {
	int j=-1;
	for(int i=0;i<nwords;i++) {
		if(!strcasecmp(word,words[i])) {
			j=i;
			break;
		}
	}
	return j;
}

void shuffle(char *dice[]) {
	for(int i=15;i>0;i--) {
		int j=rand()%(i+1);
		char *tmp=dice[i];
		dice[i]=dice[j];
		dice[j]=tmp;
	}
}

void initBoard(char board[],char *dice[]) {
	int k=0;
	for(int j=0;j<4;j++) {
		for(int i=0;i<4;i++) {
			board[k]=dice[k][rand()%6];
			k++;
		}
	}
}

void printBoard(char board[]) {
	int k=0;
	for(int j=0;j<4;j++) {
		for(int i=0;i<4;i++) {
			printf("%c",board[k++]);
		}
		printf("\n");
	}
	printf("\n");
}

void dfs(Trie *trie,int x,int y,int d) {
	static char word[17];

	if(x<0 || x>3 || y<0 || y>3) return;

	int k=y*4+x;
	int let=toupper(board[k]);
	int idx=board[k]-'A';

	if(graph[k]) return;

	trie=trie->next[idx];

	if(trie==NULL) return;

	word[d++]=let;

	if(trie->mark && d>=3) {
		word[d]='\0';
		if(Boggle_WordFind(words,nwords,word)==-1) {
			Boggle_WordAdd(&words,&nwords,word);
		}
	}

	graph[k]=true;

	for(int j=-1;j<=1;j++)
		for(int i=-1;i<=1;i++)
	  	if(i || j)
				dfs(trie,x+i,y+j,d);

	graph[k]=false;

	if(let=='Q') {
		trie=trie->next['U'-'A'];
		if(trie!=NULL) {
			word[d++]='U';
			if(trie->mark && d>=3) {
				word[d]='\0';
				if(Boggle_WordFind(words,nwords,word)==-1) {
					Boggle_WordAdd(&words,&nwords,word);
				}
			}
			graph[k]=true;
			for(int j=-1;j<=1;j++)
				for(int i=-1;i<=1;i++)
			  	if(i || j)
						dfs(trie,x+i,y+j,d);
			graph[k]=false;
		}
	}

}


int main(void) {

	srand(time(NULL));

	shuffle(dice);
	initBoard(board,dice);

	Trie *trie=Trie_New();

	char *line=NULL;
	size_t llen=0;
	ssize_t rlen=0;

	FILE *fin=fopen("csw.txt","r");
	while((rlen=getline(&line,&llen,fin))!=-1) {
		char *p=strchr(line,'\n');
		if(p) *p='\0';
		Trie_AddWord(trie,line);
	}
	fclose(fin);

	for(int i=0;i<16;i++)
		graph[i]=false;

	for(int j=0;j<4;j++)
		for(int i=0;i<4;i++)
			dfs(trie,i,j,0);

	size_t nflags=nwords;
	bool flags[nflags];
	size_t left=nwords;

	for(int i=0;i<nflags;i++)
		flags[i]=false;

	printBoard(board);

	ssize_t pos;
	int score=0;
	int points=0;

	printf("> ");
	while((rlen=getline(&line,&llen,stdin))!=-1) {
		char f[256],w[256],d[1024];
		char *p=strchr(line,'\n');
		if(p) *p='\0';
		trim(line);

		if(line[0]=='.') {
			if(!strcasecmp(line,".board")) {
				printBoard(board);
			} else if(!strcasecmp(line,".left")) {
				printf("%zd of %zd words left.\n",left,nwords);
			} else if(!strcasecmp(line,".list")) {
				for(size_t i=0;i<nwords;i++) {
					if(flags[i]) printf("%s ",words[i]);
				}
				printf("\n");
			} else if(!strcasecmp(line,".score")) {
				printf("your score is %d.\n",score);
			} else if(sscanf(line,".define %255s",f)) {
				FILE *fin=fopen("cswd.txt","r");
				trim(f);
				while((fscanf(fin,"%255[^\t]\t%1023[^\n]\n",w,d)==2)) {
					if(!strcasecmp(f,w)) {
						printf("%s\n",d);
						break;
					}
				}
				fclose(fin);
			} else if(!strcasecmp(line,".quit")) {
				break;
			}
		} else {
			pos=Boggle_WordFind(words,nwords,line);
			if(pos!=-1) {
				if(flags[pos]) {
					printf("'%s' is already guessed.\n",words[pos]);
				} else {
					flags[pos]=true;
					left--;

					size_t len=strlen(words[pos]);
					if(len==3 || len==4) points=1;
					else if(len==5) points=2;
					else if(len==6) points=3;
					else if(len==7) points=5;
					else if(len>=8) points=11;

					score+=points;

					printf("you guessed '%s' plus %d points. your score is %d.\n",words[pos],points,score);
				}
			}
		}
		printf("> ");

		free(line);
		line=NULL;
		llen=0;
		rlen=0;
	}

	Boggle_WordFree(&words,&nwords);
	Trie_Free(&trie);

	printf("bye.\n");

	return 0;
}
