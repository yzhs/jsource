/* Copyright 1990-2004, Jsoftware Inc.  All rights reserved.               */
/* Licensed use only. Any other use is in violation of copyright.          */
/*                                                                         */
/* Adverbs: Macros and Defined-Constants (for Adverbs and Conjunctions)    */


#define GAPPEND         (I)0
#define GINSERT         (I)3
#define GTRAIN          (I)6

#define COMPOSE(c)  ((c)==CAT||(c)==CATCO||(c)==CAMP||(c)==CAMPCO)
#define COMPOSECO(c)  ((c)==CATCO||(c)==CAMPCO)

#define CONJCASE(a,w)   (2*!(VERB&AT(a))+!(VERB&AT(w)))
#define NN              3               /* NOUN NOUN                       */
#define NV              2               /* NOUN VERB                       */
#define VN              1               /* VERB NOUN                       */
#define VV              0               /* VERB VERB                       */

#define DECLF            V* RESTRICT sv=FAV(self); AD * RESTRICT fs=sv->f;  \
                        AF f1=fs?FAV(fs)->f1:0,f2=fs?FAV(fs)->f2:0

#define DECLG           V* RESTRICT sv=FAV(self);AD * RESTRICT gs=sv->g;  \
                        AF g1=gs?FAV(gs)->f1:0,g2=gs?FAV(gs)->f2:0

#define DECLFG          DECLF;         AD * RESTRICT gs=sv->g;  \
                        AF g1=gs?FAV(gs)->f1:0,g2=gs?FAV(gs)->f2:0

#define DECLFGH         DECLFG;        AD * RESTRICT hs=sv->h;  \
                        AF h1=hs?FAV(hs)->f1:0,h2=hs?FAV(hs)->f2:0

// If there are multiple cells, loop over them & call back; otherwise fall through to handle to single cell
#define PREF1(f)        {I m=mr(self);            F1RANK(  m,f,self);}
#define PREF2(f)        {I l=lr(self),r=rr(self); F2RANK(l,r,f,self);}

#define AS1(f,exp,x)      DF1(f){F1PREFIP;PROLOG(x);DECLF ;A z; PREF1(f); z=(exp); EPILOG(z);}
#define AS2(f,exp,x)      DF2(f){F2PREFIP;PROLOG(x);DECLF ;A z; PREF2(f); z=(exp); EPILOG(z);}
#define CS1(f,exp,x)      DF1(f){PROLOG(x);DECLFG;A z; PREF1(f); z=(exp); EPILOG(z);}
#define CS2(f,exp,x)      DF2(f){PROLOG(x);DECLFG;A z; PREF2(f); z=(exp); EPILOG(z);}
// obsolete #define CS1IP(f,exp,x) DF1(f){F1PREFIP;PROLOG(x);DECLFG;A z;PREF1(f); exp; EPILOG(z);}
// implied rank loops.  If the arg has frame, loop over the cells.  f##cell operates on a single cell with no rank check
// inplaceability is passed through
#define CS1IP(class,f,exp,x) static DF1(f##cell){F1PREFIP;DECLFG;A z;PROLOG(x); exp; EPILOG(z);} class DF1(f){PREF1(f##cell); R f##cell(jt,w,self);}
// obsolete #define CS2IP(f,exp,x)  DF2(f){F2PREFIP;PROLOG(x);DECLFG;A z;PREF2(f); exp; EPILOG(z);}
#define CS2IP(class,f,exp,x) static DF2(f##cell){F2PREFIP;DECLFG;A z;PROLOG(x); exp; EPILOG(z);} class DF2(f){PREF2(f##cell); R f##cell(jt,a,w,self);}



#define ADERIV(id,f1,f2,flag,m,l,r)  fdef(0,id,VERB,(AF)(f1),(AF)(f2),w,0L,0L,(flag),(I)(m),(I)(l),(I)(r))
#define CDERIV(id,f1,f2,flag,m,l,r)  fdef(0,id,VERB,(AF)(f1),(AF)(f2),a,w ,0L,(flag),(I)(m),(I)(l),(I)(r))

#define ASSERTVV(a,w)   RZ(a&&w); ASSERT(VERB&AT(a)&&VERB&AT(w),EVDOMAIN)
#define ASSERTVVn(a,w)  RZ(a&&w); ASSERT(VERB&AT(a)&&(VERB|NOUN)&AT(w),EVDOMAIN)
#define ASSERTVN(a,w)   RZ(a&&w); ASSERT(VERB&AT(a)&&NOUN&AT(w),EVDOMAIN)
#define ASSERTNN(a,w)   RZ(a&&w); ASSERT(NOUN&AT(a)&&NOUN&AT(w),EVDOMAIN)

#define SCALARFN(id,w)  (id==ID(w)&&!lr(w)&&!rr(w))

#define FIT0(c,v)       (CFIT==v->id&&c==ID(v->f)&&equ(zero,v->g))
