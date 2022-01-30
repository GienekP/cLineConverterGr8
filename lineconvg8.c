/*--------------------------------------------------------------------*/
#include <stdio.h>
/*--------------------------------------------------------------------*/
#define ALCWIDTH 320
#define ALCHEIGHT 240
#define RCSIZE (120*14)
#define PICSIZE ((ALCWIDTH)*(ALCHEIGHT))
/*--------------------------------------------------------------------*/
typedef unsigned char byte;
/*--------------------------------------------------------------------*/
byte load(const char *fn, byte *b, unsigned int s)
{
	byte ret=0;
	unsigned int i=0;
    FILE *pf;
    pf=fopen(fn,"rb");
    if (pf)
    {
		i=fread(b,sizeof(byte),s,pf);
		fclose(pf);
	};	
	if (i==s) {ret=1;} else {printf("%i Wrong input file %s\n",i,fn);};
	return ret;
}
/*--------------------------------------------------------------------*/
byte averageChroma(const byte *dta, unsigned int s)
{
	byte hist[16],max=0,p=0;
	unsigned int i;
	for (i=0; i<16; i++) {hist[i]=0;};
	for (i=0; i<s; i++)
	{
		hist[((dta[i]>>4)&0x0F)]++;
	};
	for (i=0; i<16; i++)
	{
		if (hist[i]>max) {max=hist[i];p=i;};
	};
	return (p<<4);
}
/*--------------------------------------------------------------------*/
byte maxLuma(const byte *dta, unsigned int s, byte lvl)
{
	byte max=lvl+2;
	unsigned int i;
	if (max>0x0E) {max=0x0E;};
	for (i=0; i<s; i++)
	{
		unsigned int c=(((dta[i]>>5)&0x07)<<1);
		if (c>max) {max=c;};
	};
	return max;
}
/*--------------------------------------------------------------------*/
void buildRasta(const byte *chroma, const byte *luma, byte *rp, byte lvl)
{
	const unsigned int ct[14]={14,64,80,48,48,48,18,46,64,64,48,48,40,10};
	unsigned int i,j,h=0;
	for (i=0; i<(ALCHEIGHT/2); i++)
	{
		for (j=0; j<14; j++)
		{
			unsigned int cyc=ct[j];
			byte a=averageChroma(&chroma[h],cyc);
			byte b=maxLuma(&luma[h],cyc,lvl);
			h+=cyc;
			rp[i*14+j]=(a|b);
		};
	};
}
/*--------------------------------------------------------------------*/
extern void saveASM(const char *, const byte *, const byte *, byte lvl);
/*--------------------------------------------------------------------*/
void lineconvg8(const char *fn1, const char *fn2, const char *fn3, byte lvl)
{
	byte chroma[PICSIZE];
	byte luma[PICSIZE];
	byte rp[RCSIZE];
	if ((load(fn1,chroma,PICSIZE)) && (load(fn2,luma,PICSIZE)))
	{
		buildRasta(chroma,luma,rp,lvl);
		saveASM(fn3,luma,rp,lvl);
	};
}
/*--------------------------------------------------------------------*/
unsigned char graylvl(const char *txt)
{
	unsigned char a;
	a=txt[0];
	if (a==0) {a=0x2;};
	if ((a>='0') && (a<='9')) {a-='0';};
	if ((a>='A') && (a<='F')) {a-=('A'-10);};
	if ((a>='a') && (a<='f')) {a-=('a'-10);};
	a&=0x0F;
	return a;
}
/*--------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	switch (argc)
	{
		case 4:
		{
			lineconvg8(argv[1],argv[2],argv[3],0x02);
		} break;
		case 5:
		{
			lineconvg8(argv[1],argv[2],argv[3],graylvl(argv[4]));
		} break;
		default:
		{
			printf("LineConverter Gr.8 - (c)GienekP\n");
			printf("use:\n");
			printf("   lineconvg8 ataripal.raw dither.raw picture.asm [2]\n");
			printf("   ataripal.raw - 320x240 8-bit ATARI pallete format\n");		
			printf("   dither.raw - 320x240 8-bit (grayscale) pixels\n");		
			printf("   2 - background brightness\n");	
		} break;
	};
	return 0;
}
/*--------------------------------------------------------------------*/
unsigned int rh(const byte *data, unsigned int *pc)
{
	unsigned int ret;
	ret=data[*pc];
	*pc=*pc+1;
	return ret;
}
/*--------------------------------------------------------------------*/
void saveASM(const char *fn, const byte *mono, const byte *rp, byte lvl)
{
    unsigned int i,j,k,m=0,counter=0;
    unsigned int *pc;
    pc=&counter;
    FILE *pf;
    pf=fopen(fn,"w");
    if (pf)
    {    
		fprintf(pf,";\n");
		fprintf(pf,"; LineConverter Gr.8\n");
		fprintf(pf,"; (c) GienekP\n");
		fprintf(pf,";\n\n");
		fprintf(pf,"DMACTLS = $022F\n");
		fprintf(pf,"DLPTRS  = $0230\n");
		fprintf(pf,"GTICTLS = $026F\n");
		fprintf(pf,"COLPM0S = $02C0\n");
		fprintf(pf,"COLPM1S = $02C1\n");
		fprintf(pf,"COLPM2S = $02C2\n");
		fprintf(pf,"COLPM3S = $02C3\n");
		fprintf(pf,"COLPF0S = $02C4\n");
		fprintf(pf,"COLPF1S = $02C5\n");
		fprintf(pf,"COLPF2S = $02C6\n");
		fprintf(pf,"COLPF3S = $02C7\n");
		fprintf(pf,"COLBAKS = $02C8\n");
		fprintf(pf,"COLPF1  = $D017\n");
		fprintf(pf,"COLPF2  = $D018\n");
		fprintf(pf,"TRIG0   = $D010\n");
		fprintf(pf,"TRIG1   = $D011\n");
		fprintf(pf,"CONSOL  = $D01F\n");
		fprintf(pf,"SKCTL   = $D20F\n");
		fprintf(pf,"WSYNC   = $D40A\n");
		fprintf(pf,"VCOUNT  = $D40B\n\n");
		fprintf(pf,".define poke mva #%%%%2 %%%%1\n\n");
		fprintf(pf,"    org $2000\n");
		fprintf(pf,"    run MAIN\n\n");
		fprintf(pf,"    :16 .byte 0\n");
		fprintf(pf,"PICTURE\n");
		
		for (i=0; i<ALCHEIGHT; i++)
		{
			if (i==204) {fprintf(pf,"    :16 .byte 0\n");};
			fprintf(pf,"    dta ");
			for (j=0; j<(ALCWIDTH/8); j++)
			{
				unsigned int d=0;
				for (k=0; k<8; k++)
				{
					unsigned int c;
					if (mono[m]) {c=0;} else {c=1;};
					d<<=1;
					d|=c;
					m++;
				}
				fprintf(pf,"$%02X",d);
				if (j<((ALCWIDTH/8)-1)) {fprintf(pf,", ");};
			};
			fprintf(pf,"\n");
		};
		fprintf(pf,".ALIGN $0400\n");
		fprintf(pf,"ANT ANTIC_PROGRAM PICTURE,ANT\n\n");
		fprintf(pf,".MACRO ANTIC_PROGRAM\n");
		fprintf(pf,"    :+204 dta $4F,a(:1+$0000+#*40)\n");
		fprintf(pf,"    :+36 dta $4F,a(:1+$1FF0+#*40)\n");
		fprintf(pf,"    dta $41,a(:2)\n");
		fprintf(pf,".ENDM\n\n");
		fprintf(pf,"MAIN\n");
		fprintf(pf,"    poke(DMACTLS,0)\n");
		fprintf(pf,"    poke(GTICTLS,$00)\n");
		fprintf(pf,"    mva <ANT DLPTRS\n");
		fprintf(pf,"    mva >ANT DLPTRS+1\n");
		fprintf(pf,"    poke(COLPM0S,$00)\n");
		fprintf(pf,"    poke(COLPM1S,$00)\n");
		fprintf(pf,"    poke(COLPM2S,$00)\n");
		fprintf(pf,"    poke(COLPM3S,$00)\n");
		fprintf(pf,"    poke(COLPF0S,$00)\n");
		fprintf(pf,"    poke(COLPF1S,$0%01X)\n",(lvl&0x0F));
		fprintf(pf,"    poke(COLPF2S,$00)\n");
		fprintf(pf,"    poke(COLPF3S,$00)\n");
		fprintf(pf,"    poke(COLBAKS,$00)\n");
		fprintf(pf,"    poke(DMACTLS,$22)\n");
		fprintf(pf,"LOOP\n");
		fprintf(pf,"    lda VCOUNT\n");
		fprintf(pf,"    cmp #$02\n");
		fprintf(pf,"    bne LOOP\n");
		fprintf(pf,"    lda #$00\n");
		fprintf(pf,"    sta COLPF2\n");
		fprintf(pf,"    sta WSYNC\n");
		fprintf(pf,"    sta WSYNC\n");
		fprintf(pf,"    sta WSYNC\n");
		fprintf(pf,"    sta WSYNC\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		// Start count
		fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
		fprintf(pf,"    ldx #$%02X\n",rh(rp,pc));
		fprintf(pf,"    ldy #$%02X\n",rh(rp,pc));
		fprintf(pf,"    sta COLPF2\n");
		fprintf(pf,"    stx COLPF2\n");
		fprintf(pf,"    sty COLPF2\n");
		fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
		fprintf(pf,"    sta COLPF2\n");
		fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
		fprintf(pf,"    sta COLPF2\n");
		fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
		fprintf(pf,"    sta COLPF2\n");
		fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
		fprintf(pf,"    sta COLPF2\n");
		fprintf(pf,"    lda #$00;\n");
		fprintf(pf,"    sta COLPF2\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    nop\n");
		fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
		fprintf(pf,"    ldx #$%02X\n",rh(rp,pc));
		fprintf(pf,"    ldy #$%02X\n",rh(rp,pc));
		fprintf(pf,"    sta COLPF2\n");
		fprintf(pf,"    stx COLPF2\n");
		fprintf(pf,"    sty COLPF2\n");
		fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
		fprintf(pf,"    sta COLPF2\n");
		fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
		fprintf(pf,"    sta COLPF2\n");
		fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
		fprintf(pf,"    sta COLPF2\n");
		fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
		fprintf(pf,"    sta COLPF2\n");
		fprintf(pf,"    lda #$00\n");
		fprintf(pf,"    sta COLPF2\n");
		for (i=0; i<119; i++)
		{
			fprintf(pf,"    nop\n");
			fprintf(pf,"    nop\n");
			fprintf(pf,"    nop\n");
			fprintf(pf,"    nop\n");
			fprintf(pf,"    nop\n");
			fprintf(pf,"    nop\n");
			fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
			fprintf(pf,"    ldx #$%02X\n",rh(rp,pc));
			fprintf(pf,"    ldy #$%02X\n",rh(rp,pc));
			fprintf(pf,"    sta COLPF2\n");
			fprintf(pf,"    stx COLPF2\n");
			fprintf(pf,"    sty COLPF2\n");
			fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
			fprintf(pf,"    sta COLPF2\n");
			fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
			fprintf(pf,"    sta COLPF2\n");
			fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
			fprintf(pf,"    sta COLPF2\n");
			fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
			fprintf(pf,"    sta COLPF2\n");
			fprintf(pf,"    lda #$00;\n");
			fprintf(pf,"    sta COLPF2\n");
			fprintf(pf,"    nop\n");
			fprintf(pf,"    nop\n");
			fprintf(pf,"    nop\n");
			fprintf(pf,"    nop\n");
			fprintf(pf,"    nop\n");
			fprintf(pf,"    nop\n");
			fprintf(pf,"    nop\n");
			fprintf(pf,"    nop\n");
			fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
			fprintf(pf,"    ldx #$%02X\n",rh(rp,pc));
			fprintf(pf,"    ldy #$%02X\n",rh(rp,pc));
			fprintf(pf,"    sta COLPF2\n");
			fprintf(pf,"    stx COLPF2\n");
			fprintf(pf,"    sty COLPF2\n");
			fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
			fprintf(pf,"    sta COLPF2\n");
			fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
			fprintf(pf,"    sta COLPF2\n");
			fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
			fprintf(pf,"    sta COLPF2\n");
			fprintf(pf,"    lda #$%02X\n",rh(rp,pc));
			fprintf(pf,"    sta COLPF2\n");
			fprintf(pf,"    lda #$00\n");
			fprintf(pf,"    sta COLPF2\n");
		};
		fprintf(pf,"    lda TRIG0\n");
		fprintf(pf,"    beq STOP\n");
		fprintf(pf,"    lda TRIG1\n");
		fprintf(pf,"    beq STOP\n");
		fprintf(pf,"    lda CONSOL\n");
		fprintf(pf,"    and #1\n");
		fprintf(pf,"    beq STOP\n");
		fprintf(pf,"    lda SKCTL\n");
		fprintf(pf,"    and #$04\n");
		fprintf(pf,"    beq STOP\n");
		fprintf(pf,"    jmp LOOP\n");
		fprintf(pf,"STOP\n");
		fprintf(pf,"    rts\n");
    	fclose(pf);
	};
}
/*--------------------------------------------------------------------*/
