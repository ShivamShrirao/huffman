#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

typedef struct nodes
{
	float freq;
	int val;
	struct nodes *left, *right;
}node;

typedef struct link_lists
{
	node *var;
	struct link_lists *next;
}ll;

#define LEN_ALPHA 256
unsigned char alpha[LEN_ALPHA]={'\0'};
size_t freq[LEN_ALPHA]={0};
size_t codeTable[LEN_ALPHA]={0};
size_t revCodeTable[LEN_ALPHA]={0};
size_t fileSize;
ll* freeMe;
// TODO => MAKE ARRAY FOR STORING CODES
void insertion_sort();
void make_codes(node *tree, size_t code);
void show_codeTable();
node* build_huff_tree();
void compressFile(FILE *inp, FILE *out);
void decompressFile(FILE *inp, FILE *out, node *tree);
ll* create_list(int len);
ll* show_elem(ll *leaf, int ind);
ll* rem_el(ll *leaf , int ind);
void ll_insert(ll *tree, int ind, node *val);
void push_end(ll *leaf, node *var);
void free_list(ll *leaf);

void count_freq(FILE *inp){
	fseek(inp,0,SEEK_END);
	size_t fsz = ftell(inp);
	fileSize = fsz;
	rewind(inp);
	int fd = fileno(inp);
	char *dat = mmap(0,fsz,PROT_READ|PROT_WRITE,MAP_PRIVATE,fd,0);
	for(int i=0; i<fsz; i++){
		int ind = (unsigned char)(*(dat+i));
		freq[ind]++;
	}
	int unmap_result = munmap(dat, fsz);
	insertion_sort();
}

int main(){
	for(int i = 0; i < LEN_ALPHA; ++i)
	{
		alpha[i]=i;
	}
	FILE *inp, *out, *dec;
	printf("Enter file to process: ");
	char filename[30];//="input.txt";
	scanf("%s",filename);
	// ADD CHECK FOR FILE EXIST
	inp = fopen(filename,"r");
	count_freq(inp);
	node *tree = build_huff_tree();
	make_codes(tree,1);
	show_codeTable();
	printf("1.) Compress.\n2.) Decompress.\n[?] Choice > ");
	int choi;
	scanf("%d",&choi);
	if(choi==1){
		out = fopen("output.txt","wb");
		compressFile(inp,out);
		fclose(inp);
	}
	else{
		out = fopen("output.txt","rb");
		dec = fopen("decomp.txt","wb");
		decompressFile(out,dec,tree);
		fclose(dec);
	}
	free_list(freeMe);
	fclose(out);
	return 0;
}

node* build_huff_tree(){
	ll* leaf = create_list(LEN_ALPHA);
	int k=LEN_ALPHA;
	while(k>1){
		node *n = malloc(sizeof(node));
		n->left=show_elem(leaf,0)->var;
		n->right=show_elem(leaf,1)->var;
		n->freq = n->left->freq + n->right->freq;
		n->val = -1;
		int flg=1;
		for (int i = 1; i < k; ++i)
		{
			if(n->freq <= show_elem(leaf,i)->var->freq){
				ll_insert(leaf,i,n);
				leaf=rem_el(leaf,0);
				leaf=rem_el(leaf,0);
				flg=0;
				break;
			}
		}
		if(flg){
			push_end(leaf, n);
			leaf=rem_el(leaf,0);
			leaf=rem_el(leaf,0);
		}
		k--;
	}
	freeMe=leaf;
	return leaf->var;
}

ll* create_list(int len){
	ll *st = malloc(sizeof(ll)), *lit=st, *tmp;
	for (int i = 0; i < len; ++i)
	{
		lit->var=malloc(sizeof(node));
		lit->var->left='\0';
		lit->var->right='\0';
		lit->var->freq = freq[i];
		lit->var->val = alpha[i];
		ll *nx = malloc(sizeof(ll));
		lit->next=nx;
		tmp = lit;
		lit = nx;
	}
	tmp->next = '\0';
	free(lit);
	return st;
}

ll* show_elem(ll *leaf, int ind){
	ll *nx=leaf;
	for (int i = 0; i < ind; ++i)
	{
		if (!nx->next)
		{
			return '\0';
		}
		else
			nx = nx->next;
	}
	return nx;
}

void ll_insert(ll *leaf, int ind, node *var){
	ll *cu=leaf, *nx=cu;
	for (int i = 0; i < ind; ++i)
	{
		cu=nx;
		nx=cu->next;
	}
	cu->next = malloc(sizeof(ll));
	cu->next->next=nx;
	cu->next->var=var;
}

void push_end(ll *leaf, node *var){
	ll *nx=leaf;
	for (int i = 0; nx->next; ++i)
	{
		nx=nx->next;
	}
	nx->next = malloc(sizeof(ll));
	nx->next->var = var;
}

ll* rem_el(ll *leaf, int ind){				// haven't freed mem. I don't remember why
	ll *lit=leaf, *px=lit;
	for (int i = 0; i < ind; ++i)
	{
		px=lit;
		lit=lit->next;
	}
	if(ind==0){
		leaf=lit->next;
	}
	px->next=lit->next;
	return leaf;
}

void free_list(ll *leaf){
	for (int i = 0; leaf->next; ++i)
	{
		ll *lit=leaf->next;
		free(leaf);
		leaf=lit;
	}
}

void make_codes(node *tree, size_t code){
	if(tree){
		if (tree->val != -1)
		{
			codeTable[(tree->val)]=code;
		}
		else{
			make_codes(tree->left,code*10+1);
			make_codes(tree->right,code*10+2);
		}
	}
}

void show_codeTable(){
	for (int i = 0; i < LEN_ALPHA; ++i)
	{
		printf("%d : %zu : %zu\n", alpha[i], freq[i], codeTable[alpha[i]]);
		// printf("%c: %zu\n", i, codeTable[i]);
	}
}

void compressFile(FILE *inp, FILE *out){
	for (int i = 0; i < LEN_ALPHA; ++i)
	{
		size_t code=codeTable[i], revC=0;
		while(code != 0)
		{
		size_t remainder = code%10;
		revC = revC*10 + remainder;
		code/=10;
		}
		revCodeTable[i] = revC;
	}
	rewind(inp);
	char bit, op='\0';
	int ch=0, make_byte=8;
	while((ch=fgetc(inp))!=EOF)
	{
		int code=revCodeTable[ch];
		while(code>=1){
			bit = code%10 - 1;
			code /= 10;
			op=op<<1;
			make_byte--;
			op= op | bit;
			if(!make_byte){
				putc(op,out);
				make_byte=8;
			}
		}
	}
	if(make_byte!=8)
	{
		op = op << make_byte;
		putc(op,out);
	}
}

void decompressFile(FILE *inp, FILE *out, node *tree){
	rewind(inp);
	node *lit=tree;
	char bit, filter= 1<<7;
	int ch=0;
	while((ch=fgetc(inp))!=EOF)
	{
		int make_byte=8;
		while(make_byte>0){
			bit=ch & filter;
			ch=ch<<1;
			make_byte--;
			if(bit==0){
				lit=lit->left;
			}
			else{
				lit=lit->right;
			}
			if(lit->val != -1)
			{
				fputc(lit->val,out);
				lit=tree;
			}
		}
	}
	if(ftell(out)!=fileSize){
		fseeko(out,-1,SEEK_END);
		ftruncate(fileno(out),ftell(out));
	}
}

void insertion_sort(){
	float temp;
	char tmpc;
	for (int i = 1; i < LEN_ALPHA; ++i)
	{
		temp=freq[i];
		tmpc=alpha[i];
		int j;
		for (j = i-1; (j>=0)&&(temp<freq[j]); j--)
		{
			freq[j+1]=freq[j];
			alpha[j+1]=alpha[j];
		}
		freq[j+1]=temp;
		alpha[j+1]=tmpc;
	}
}