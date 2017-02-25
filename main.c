#include <stdio.h>
#include <stdlib.h>

/*
File format for .bin3:
First 8 bytes is the number of points as an unsigned integer.
Points follow, 10 bytes each:
	x, y, z as 16-bit unsigned integers
	r, g, b as 8-bit unsigned integers
	1 padding byte
*/

struct Info {
	long long m;
	unsigned int c;
};

int cnt;
struct Info in[75232797];

int memoctn;
struct Info memoct[0xfffff];

struct Info2 {
	long long m;
	unsigned int c;
	int from;
	int size;
};

int cnt2;
struct Info2 in2[2000];

int level[256];
int nlevel;

char n[256];
FILE *fi,*fa;

// VERSION WITH MAGIC BITS
// -----------------------
long long splitBy3(int a){
	 long long x = a & 0x1fffff;
	x = (x | x << 32) & 0x1f00000000ffff;
	x = (x | x << 16) & 0x1f0000ff0000ff;
	x = (x | x << 8) & 0x100f00f00f00f00f;
	x = (x | x << 4) & 0x10c30c30c30c30c3;
	x = (x | x << 2) & 0x1249249249249249;
	return x;
}

long long morton3d( int x,  int y,  int z){
return splitBy3(x) | (splitBy3(y) << 1) | (splitBy3(z) << 2);
}

inline int popcnt(register  int u)
{
    u = (u & 0x55555555) + ((u >> 1) & 0x55555555);
    u = (u & 0x33333333) + ((u >> 2) & 0x33333333);
    u = (u & 0x0f0f0f0f) + ((u >> 4) & 0x0f0f0f0f);
    u = (u & 0x00ff00ff) + ((u >> 8) & 0x00ff00ff);
    u = (u & 0x0000ffff) + ((u >>16) & 0x0000ffff);
    return u;
}

inline int place(int n)
{
return 1<<n;
}

//---------------------------------------------
int comparar(const void *arg1, const void *arg2)
{
 if(((struct Info *)arg1)->m < ((struct Info *)arg2)->m) return -1;
   else if(((struct Info *)arg1)->m > ((struct Info *)arg2)->m) return 1;
   else return 0;
}

void sortnodos()
{
 qsort(in, cnt, sizeof(struct Info), comparar);	
}

//---------------------------------------------
int colores[8];
int ncol;

int promediocol(void)
{
//	if (ncol>8) printf("NCOLLLL %i",ncol);
int sumr=0,sumg=0,sumb=0;
int i;
for(i=0;i<ncol;i++) {
  sumb+=(colores[i]&0xff)<<16;
  sumg+=(colores[i]&0xff00)<<8;
  sumr+=colores[i]&0xff0000;
  }
return (((sumb/ncol)>>16)&0xff)|(((sumg/ncol)>>8)&0xff00)|(((sumb/ncol))&0xff0000);
}

void newnode(long long p, int b,int col)
{
memoct[memoctn].m=p;
memoct[memoctn].c=(col<<8)|(b&0xff);
memoctn++;
}


void teste()
{
	int i;
for(i=0;i<cnt;i++) {
	
	if (in[i].m==in[i+1].m) printf("*");
	//printf("%llx ",in[i].m);
     }
}

///////////////////////////////////////////////////////////////////
int ultimocolor;

int makeoctree8(int from,int to)
{
int filesize;
long long padre;
int i,j;
int bith;
int vv;

//printf("oct %i\n",memoctn);
memoctn=0;
nlevel=0;
level[nlevel++]=memoctn;

//printf("%i..%i\n",from,to);
//printf("%llx ..",in[from].m);
//printf("%llx \t",in[to-1].m);

padre=in[from].m>>3;
bith=place(in[from].m&0x7);
ncol=0;
colores[ncol++]=in[from].c;

for(i=from+1;i<to;i++) {
//	printf("%llx %i\n",in[i].m,memoctn);	
     if ((in[i].m>>3)==padre) {
       bith|=place(in[i].m&0x7);
       colores[ncol++]=in[i].c;
     } else {
       newnode(padre,bith,promediocol());
       padre=in[i].m>>3;
       bith=place(in[i].m&0x7);
       ncol=0;
       colores[ncol++]=in[i].c;
       }
     }
newnode(padre,bith,promediocol());
level[nlevel++]=memoctn;

//while (level[nlevel-2]<memoctn-1) {
while (nlevel<9) {

	i=level[nlevel-2];

	padre=memoct[i].m>>3;
	bith=place(memoct[i].m&0x7);
	ncol=0;
	colores[ncol++]=memoct[i].c>>8;
	i++;
	for(;i<level[nlevel-1];i++) {
     if ((memoct[i].m>>3)==padre) {
       bith|=place(memoct[i].m&0x7);
       colores[ncol++]=memoct[i].c>>8;
     } else {
       newnode(padre,bith,promediocol());
       padre=memoct[i].m>>3;
       bith=place(memoct[i].m&0x7);
       ncol=0;
       colores[ncol++]=memoct[i].c>>8;
       }
     }
	newnode(padre,bith,promediocol());
	level[nlevel++]=memoctn;
	}

//printf("niveles:%d\n",nlevel);
sprintf(n,"sib-%x.3do",((in[from].m>>24)&0xffff));
fi=fopen(n,"w+b");

int nodos=0;
for (i=nlevel;i>1;i--) {
	for (j=level[i-2];j<level[i-1];j++) { nodos++; }
	}
//nodos=(nodos+7)*4;
nodos=nodos*4;

vv=0x3d000008;fwrite(&vv,sizeof(int),1,fi); // magic vv=0x3d000100
fwrite(&nodos,sizeof(int),1,fi); // cantidad de nodos
vv=0x0;fwrite(&vv,sizeof(int),1,fi);
vv=0x0;fwrite(&vv,sizeof(int),1,fi);
vv=0x0;fwrite(&vv,sizeof(int),1,fi);
vv=0x0;fwrite(&vv,sizeof(int),1,fi);
vv=0x0;fwrite(&vv,sizeof(int),1,fi);

filesize=7*4;

 int sumadir=0;
 int actdir=0;

for (i=nlevel;i>1;i--) {
	for (j=level[i-2];j<level[i-1];j++) {
		vv=((sumadir-actdir)<<8)|(memoct[j].c&0xff);fwrite(&vv,sizeof(int),1,fi); // 24bits direcciones
		filesize+=4;
		sumadir+=popcnt((memoct[j].c&0xff));
		actdir++;
		}
	}

ultimocolor=memoct[level[nlevel-2]].c>>8;

for (i=nlevel;i>1;i--) {
	for (j=level[i-2];j<level[i-1];j++) {
		vv=memoct[j].c>>8;fwrite(&vv,sizeof(int),1,fi);
		filesize+=4;
		}
	}

for(i=from;i<to;i++) {
    vv=in[i].c;fwrite(&vv,sizeof(int),1,fi);
    filesize+=4;
     }
fclose(fi);
return filesize+4;
}

////////////////////////////////////////////////////////////////////////
void split8(void)
{
long long padre;
int j,i=0;
int fs;
int vv;

cnt2=0;
while (i<cnt) {
	padre=in[i].m>>24;
	j=i;
	while ((in[i].m>>24)==padre && i<cnt) i++;
	fs=makeoctree8(j,i);

	in2[cnt2].m=padre;
	in2[cnt2].c=ultimocolor;
	in2[cnt2].from=j;
	in2[cnt2].size=fs;
	cnt2++;
	}

int bith;

memoctn=0;
nlevel=0;
level[nlevel++]=memoctn;

padre=in2[0].m>>3;
bith=place(in2[0].m&0x7);
ncol=0;
colores[ncol++]=in2[0].c;
for( i=1;i<cnt2;i++) {
     if ((in2[i].m>>3)==padre) {
       bith|=place(in2[i].m&0x7);
       colores[ncol++]=in2[i].c;
     } else {
       newnode(padre,bith,promediocol());
       padre=in2[i].m>>3;
       bith=place(in2[i].m&0x7);
       ncol=0;
       colores[ncol++]=in2[i].c;
       }
     }
newnode(padre,bith,promediocol());
level[nlevel++]=memoctn;

while (level[nlevel-2]<memoctn-1) {

	i=level[nlevel-2];

	padre=memoct[i].m>>3;
	bith=place(memoct[i].m&0x7);
	ncol=0;
	colores[ncol++]=memoct[i].c>>8;
	i++;
	for(;i<level[nlevel-1];i++) {
     if ((memoct[i].m>>3)==padre) {
       bith|=place(memoct[i].m&0x7);
       colores[ncol++]=memoct[i].c>>8;
     } else {
       newnode(padre,bith,promediocol());
       padre=memoct[i].m>>3;
       bith=place(memoct[i].m&0x7);
       ncol=0;
       colores[ncol++]=memoct[i].c>>8;
       }
     }
	newnode(padre,bith,promediocol());
	level[nlevel++]=memoctn;
	}


printf("niveles:%d\n",nlevel);

sprintf(n,"sibenikm.3do");
fi=fopen(n,"w+b");

int nodos=0;
for (i=nlevel;i>1;i--) {
	for (j=level[i-2];j<level[i-1];j++) { nodos++; }
	}
nodos=nodos*4;

vv=0x3d00010f;fwrite(&vv,sizeof(int),1,fi); // magic vv=0x3d000100
fwrite(&nodos,sizeof(int),1,fi); // cantidad de nodos
vv=0x0;fwrite(&vv,sizeof(int),1,fi);
vv=0x0;fwrite(&vv,sizeof(int),1,fi);
vv=0x0;fwrite(&vv,sizeof(int),1,fi);
vv=0x0;fwrite(&vv,sizeof(int),1,fi);
vv=0x0;fwrite(&vv,sizeof(int),1,fi);

 int sumadir=0;
 int actdir=0;

for (i=nlevel;i>1;i--) {
	for (j=level[i-2];j<level[i-1];j++) {

		vv=((sumadir-actdir)<<8)|(memoct[j].c&0xff);fwrite(&vv,sizeof(int),1,fi); // 24bits direcciones
		sumadir+=popcnt((memoct[j].c&0xff));
		actdir++;
		}
//	printf("%d  sumadir:%x\n",level[i-1]-level[i-2],sumadir-actdir);
	}

ultimocolor=memoct[level[nlevel-2]].c>>8;

for (i=nlevel;i>1;i--) {
	for (j=level[i-2];j<level[i-1];j++) {
		vv=memoct[j].c>>8;fwrite(&vv,sizeof(int),1,fi);
		nodos+=4;
		}
	}
//printf("nodos1:%x ",nodos);
nodos=nodos+(7*4)+cnt2*4;
//printf("nodos2:%x cnt2:%d \n",nodos,cnt2);
//return;

for(i=0;i<cnt2;i++) {
    vv=nodos>>2;
    //printf("%d size:%x -",i,vv);
	fwrite(&vv,sizeof(int),1,fi);
	nodos=nodos+in2[i].size;
//    printf("%d size:%d -",i,in2[i].size);
     }

for(i=0;i<cnt2;i++) {
	sprintf(n,"sib-%x.3do",//in2[i].m
		((in2[i].m>>24)&0xffff));
	fa=fopen(n,"r+b");
	while(feof(fa)==0) {
		fread(&vv,sizeof(int),1,fa);
		fwrite(&vv,sizeof(int),1,fi);
		}
	fclose(fa);
    }
fclose(fi);
}

//---------------------------------------------

int main(int argc, char *argv[])
{
	fi=fopen("D:/work/r4/r4dataextern/voxel/sibenik.bin3","rb");
	long long ncnt;
	fread(&ncnt,sizeof(ncnt),1,fi);
	cnt=ncnt;
	printf("Cantidad :%i\n",cnt);

	int i;
	unsigned short  x,y,z;
	int c;
	printf("cargando...\n");
	for (i=0;i<cnt;i++) {
		fread(&x,sizeof(x),1,fi);
		fread(&y,sizeof(y),1,fi);
		fread(&z,sizeof(z),1,fi);
		fread(&c,sizeof(c),1,fi);
		in[i].m=morton3d(x,y,z);
		in[i].c=c&0xffffff;
		//printf("%i %i %i %i %llx\n",i,x,y,z,in[i].m);
		if ((i&0xffff)==0) printf(".");
	}
	fclose(fi);
	//teste();
	printf("\nordenando...\n");
	sortnodos();
	//teste();
	printf("generando...\n");
	split8();
	printf("fin...\n");

	return 0;
}
