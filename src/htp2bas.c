#include <stdio.h>

#define STDERR stderr
#define peekw(a)( * (unsigned short * )(a))
#define hl_addr(a)(mem + a - 0x4016)
#define wrap_pos 71

int wrap;

void w2printf( char * s, int v ) {
  wrap++;
  printf(s, v);
  switch (s[0]) {
  case ' ':
    dowrap:
      if (wrap > wrap_pos) wrap = 0; // , printf("\n");
    return;
  case '<':
    if (s[1] == '%') {
      wrap++;
      goto countdigits;
    }
    break;
  case '%':
    if (s[1] == 'u') {
      wrap += 2;
      countdigits:
        while (v > 9) wrap++, v /= 10;
      goto dowrap;
    } else if (s[1] == 'c' && (v == 32 || v == 44 || v == 58)) goto dowrap;
  }
}

//const 
unsigned char tokens[] = {
      0xde, 0xab, 0xad, 0xaa, 0xaf, 0xbe, 0xbc, 0xbd, 0x44, 0xcc, 0x47, 0xcc, 0x43, 0xd2, 0x48, 0xcd, 0x46, 0xce, 0x41, 0x4e, 0xc4, 0x4f, 0xd2, 0x43, 0x4c, 0xd2, 0x44, 0x49, 0xcd, 0x45, 0x4e,
0xc4, 0x46, 0x4f, 0xd2, 0x47, 0x4f, 0x54, 0xcf, 0x47, 0x4f, 0x53, 0x55, 0xc2, 0x49, 0xc6, 0x43, 0x4f, 0x4e, 0xd4, 0x44, 0x41, 0x54, 0xc1, 0x4c, 0x49, 0x53, 0xd4, 0x52, 0x45, 0x41, 0xc4, 0x4e,
0x45, 0x58, 0xd4, 0x4f, 0xce, 0x50, 0x4f, 0x4b, 0xc5, 0x44, 0x45, 0x46, 0x46, 0xce, 0x52, 0x45, 0x54, 0x55, 0x52, 0xce, 0x53, 0x41, 0x56, 0xc5, 0x54, 0x48, 0x45, 0xce, 0x4c, 0x4f, 0x41, 0xc4,
0x52, 0x45, 0x53, 0x54, 0x4f, 0x52, 0xc5, 0x53, 0x54, 0x45, 0xd0, 0x50, 0x4f, 0xd0, 0x4e, 0x45, 0xd7, 0x54, 0xcf, 0x20, 0x20, 0x0a, 0x8a, 0x50, 0x52, 0x49, 0x4e, 0xd4, 0x52, 0x55, 0xce, 0x54,
0x52, 0xc3, 0x4e, 0x4f, 0xd4, 0x50, 0x4c, 0x4f, 0xd4, 0x43, 0x41, 0x4c, 0xcc, 0x49, 0x4e, 0x50, 0x55, 0xd4, 0x53, 0x54, 0x52, 0xa4, 0x4c, 0x46, 0x54, 0xa4, 0x52, 0x47, 0x48, 0xa4, 0x43, 0x48,
0x52, 0xa4, 0x4d, 0x49, 0x44, 0xa4, 0x49, 0x4e, 0xd4, 0x41, 0x53, 0xc3, 0x4c, 0x45, 0xce, 0x41, 0x42, 0xd3, 0x53, 0x47, 0xce, 0x43, 0x4f, 0xd3, 0x50, 0x45, 0x45, 0xcb, 0x53, 0x51, 0xd2, 0x52,
0x4e, 0xc4, 0x53, 0x49, 0xce, 0x54, 0x41, 0xce, 0x55, 0x53, 0xd2, 0x56, 0x41, 0xcc, 0x41, 0x54, 0xce, 0x50, 0x4f, 0x49, 0x4e, 0xd4, 0x45, 0x58, 0xd0, 0x4c, 0x4f, 0xc7, 0x8d, 0x8d, 0x8d, 0x46,
0x52, 0xc5
// , 0x44, 0xc4, 0x43, 0xce, 0x50, 0xd0, 0x4f, 0xc4, 0x55, 0xc6, 0x53, 0xce, 0x54, 0xcd, 0x55, 0xd3, 0x2f, 0xb0, 0x49, 0xd1, 0x4f, 0xcd, 0x4f, 0xd6, 0x53, 0xcc, 0x42, 0xd3, 0x4f, 0xcb, 0x45, 0x52, 0x52, 0x4f, 0
};

int main(int argc, char ** argv) {
  // char tokens[] = "Ţ«�ŞŻľĽ˝PéReíCňHíInkeůanäoňcuňDiíEnäFoňGotďGosuâIćConôDatáLisôReaäNexôOîPokĺExôReturîSavĺtheîLoaäRestorĺsteđPođNe÷tďBeeđPrinôRuîMoînoôPloôCalěInpuôstr¤lft¤rgh¤chr¤mid¤inôasăleîabósgîcoópeeësqňrnäsiîtaîusňvaěatîpoinôexđloçŤŤŤfrĺKeůNe÷ExôEsavĺEloaäMergĺVerifůEdiôRepeaôUntiěDeletĺGosubŁGotoŁReturîform$¨hex$¨dec¨min¨max¨fsw¨mod¨Prçfra¨string$¨Ą¤var¨round¨Ujra m{retezett t|mâNem folytathatţStack-hibáT|bb READ, mint DATÁ [rtelmetleîRossz adattipuóHivatkoz s neml{tez| sorrá0-val val~ oszt óNem {rtelmezhet| sz íTul kicsi a t ňTul nagy sz íTul hosszu sz|veçRossz t|mbindeřV ltoz~k t|r|lveˇˇ% %Č(%D%L%Ž%_\x22Q!Ř";
  char accents[] = "{é|ö}ü~ó\x7fá©íµúĽő˝ű@Á[É\\Ö]Ü_Ó\x95Ú\x9dŰ";
  int i;
  fprintf(stderr, "HTP2BAS: Homelab fájlból Basic listát készít.\nÍrta Papp László, 2008.\n\n");
  if (argc == 1) {
    fprintf(stderr, "Nem tudom mit konvertáljak, nem adta meg!\n");
    return (1);
  }
  for (i = 1; i < argc; i++) {
    FILE * htp = fopen(argv[i], "rb");
    unsigned char buf[65536];
    unsigned char * b = buf, * mem, * endofprg, * s1, * s2;
    int editnum = 0;
    if (htp) {
      fprintf(STDERR, "%s\n", argv[i]);
      fseek(htp, 257, SEEK_CUR);
      fread(buf, 65536, 1, htp);
      while ( * (b++));
      if (peekw(b) != 0x4016) {
        fprintf(STDERR, "A programot nem Basic-ben írták!\n");
        continue;
      }
      mem = b + 4;
      if ( * (hl_addr(0x404c)) == 0) fprintf(STDERR, "A program autostartos!\n");
      s2 = hl_addr(peekw(hl_addr(0x4045)));
      s1 = hl_addr(peekw(hl_addr(0x4030)));
      s2 = s2 < mem ? s1 : s2;
      b = s1 < s2 ? s1 : s2;
      if (b - mem > 0x40a0 - 0x4016) fprintf(stderr, "A program beágyazott gépi kódú részt, vagy adatterületet tartalmaz!\n");
      endofprg = hl_addr(peekw(hl_addr(0x4018)));
      for (; b < endofprg;) {
        int remark = 0, quoted = 0, stringflag = 0;
        int linenum;
        wrap = 0;
        if ( * b == 0x80) printf("\n%d. lap\n\n", ++editnum), b++;
        linenum = b[1] + (b[0] << 8);
        b += 2;
        if (linenum < 32767) {
          w2printf("%u  ", linenum);
          while (1) {
            unsigned char * a = accents;
            if (stringflag && ( * b >= 'A' + 31 || * b < 'A'))
              w2printf("$", 0), stringflag = 0;
            if ( * b == 34) quoted ^= 1;
            if ( * b == 96) {
              printf("\n");
              b++;
              break;
            }
            if ( * b > 127 && !remark && !quoted) {
              unsigned char * t = tokens;
              int n = 128;
              while ( * t && n < * b) {
                while ( * (t++) < 128);
                n++;
              }
              if ( * t == 0) goto nottoken;
              switch ( * b) {
              case 0x8d ... 0xae:
              case 0xc9 ... 0xd6:
                w2printf(" ", 0);
              }
              do {
                w2printf("%c", * t & 0x7f);
              } while ( * (t++) < 128);
              switch ( * b) {
              case 0x8d ... 0xae:
              case 0xc9 ... 0xd6:
                w2printf(" ", 0);
                break;
              case 0xaf ... 0xc8:
                w2printf("(", 0);
              }
            } else {
              nottoken: if (!quoted && !remark && ( * b >= 'a' && * b < 'a' + 31))
                ( * b) ^= 0x20, stringflag = 1;
              while ( * a && * a != * b) a += 2; // accents : speciális változók?
              if ( * a) w2printf("%c", *(a + 1));
              else switch ( * b) {
              case 1 ... 31:
                if (!quoted) {
                  switch ( * b) {
                  case 1 ... 27:
                    w2printf("%c", 'a' + * b - 1);
                  case 31:
                    w2printf("(", 0);
                  }
                  break;
                }
              case 127 ... 255:
              case 0:
                w2printf("<%d>", * b);
                break;
              default:
                w2printf("%c", * b);
              }
            }
            if ( * b == 137) remark = 1;
            b++;
          } // while(1)
        } else {
          if (linenum > 32767) {
            fprintf(stderr, "A program hibás!\n");
            return (2);
          }
          while ( * (b++) != 96);
        }
      } // for b<endofprg
      printf("\n");
      if (argc > 2) printf("--------------------\n\n");
      fclose(htp);
    } // if htp
  } // for argc
} // vége
