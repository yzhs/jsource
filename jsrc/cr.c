/* Copyright 1990-2009, Jsoftware Inc.  All rights reserved.               */
/* Licensed use only. Any other use is in violation of copyright.          */
/*                                                                         */
/* Conjunctions: Rank Associates                                           */

#include "j.h"

#define DR(r)           (((UI)(I)(r)>RMAX)?RMAX:r)   // return RMAX if input is negative or > RMAX

#define ZZDEFN
#include "result.h"

// make sure these don't overlap with bits in result.h
#define STATEOUTERREPEATAX 12
#define STATEOUTERREPEATA (1LL<<STATEOUTERREPEATAX)
#define STATEINNERREPEATWX 13
#define STATEINNERREPEATW (1LL<<STATEINNERREPEATWX)
#define STATEINNERREPEATAX 14
#define STATEINNERREPEATA (1LL<<STATEINNERREPEATAX)
// There must be NO higher bits than STATEINNERREPEATA, because we shift down and OR into flags

// General setup for verbs that do not go through jtirs[12].  Some of these are marked as IRS verbs.  General
// verbs derived from u"n also come through here, via jtrank2.
// A verb u["n] using this function checks to see whether it has multiple cells; if so,
// it calls here, giving a callback; we split the arguments into cells and call the callback,
// which is often the same original function that called here.
// rr is the rank at which the verb will be applied: in u"n, the smaller of rank-of-u and n
A jtrank1ex(J jt,AD * RESTRICT w,A fs,I rr,AF f1){F1PREFIP;PROLOG(0041);A z,virtw;
   I mn,n=1,wcn,wf,wk,wr,*ws,wt;
 RZ(w);
 wt=AT(w);
 if(wt&SPARSE)R sprank1(w,fs,rr,f1);
#define ZZFLAGWORD state
 I state=0;  // init flags, including zz flags
 // wr=rank, ws->shape, wcr=effective rank, wf=#frame (inner+outer)
 // if inner rank is > outer rank, set it equal to outer rank
 wr=AR(w); ws=AS(w); efr(rr,wr,rr);  // get rank at which to apply the verb
 // RANKONLY verbs were handled in the caller to this routine, but fs might be RANKATOP.  In that case we could include its rank in the loop here,
 // if its rank is not less than the outer rank (we would simply ignore it), but we don't bother.  If its rank is smaller we can't ignore it because assembly might affect
 // the order of fill.  But if f is BOXATOP, there will be no fill, and we can safely use the smaller rank
 if(fs&&FAV(fs)->flag2&VF2BOXATOP1){
  I mr=FAV(fs)->mr; efr(rr,mr,rr);
  state = (FAV(fs)->flag2&VF2BOXATOP1)>>(VF2BOXATOP1X-ZZFLAGBOXATOPX);  // If this is BOXATOP, set so for loop.  Don't touch fs yet, since we might not loop
  state &= ~((FAV(fs)->flag2&VF2ATOPOPEN1)>>(VF2ATOPOPEN1X-ZZFLAGBOXATOPX));  // We don't handle &.> here; ignore it
  // if we are using the BOXATOP from f, we can also use the raze flags.  Set these only if BOXATOP to prevent us from incorrectly
  // marking the result block as having uniform items if we didn't go through the assembly loop here
  state |= (-state) & FAV(fs)->flag2 & (VF2WILLBEOPENED|VF2COUNTITEMS);
 }
 wf=wr-rr; // obsolete state |= STATEWREL&~ARELATIVES(w);   // relies on STATEWREL>BOX
// obsolete if(ARELATIVE(w))state|=STATEWREL;
 if(!wf){R CALL1(f1,w,fs);}  // if there's only one cell and no frame, run on it, that's the result.
 // multiple cells.  Loop through them.
// obsolete  I wn=AN(w);  // empty-operand indicator
 // Get size of each argument cell in atoms.  If this overflows, there must be a 0 in the frame, & we will have
 // gone through the fill path (& caught the overflow)
 RE(mn=prod(wf,ws)); PROD(wcn,rr,ws+wf);   // number of cells, number of atoms in a cell
 // ?cn=number of atoms in a cell, ?k=#bytes in a cell
 wk=wcn*bp(wt);

 A zz=0;  // place where we will build up the homogeneous result cells
 if(mn){I i0;
  // Normal case where there are cells.
  // allocate the virtual blocks that we will use for the arguments, and fill in the shape of a cell of each
  // The base pointer AK advances through the source argument. 
  //
  // Self-virtual blocks modify the shape of a block, but that code notifies
  // us through a flag bit.
  jtinplace = (J)((I)jtinplace & ((((wt&TYPEVIPOK)!=0)&(AC(w)>>(BW-1)))*JTINPLACEW-(JTINPLACEW<<1)));  // turn off inplacing unless DIRECT and w is inplaceable, and #atoms in cell > 1
// obsolete  RZ(virtw = virtual(w,0,rr)); {I * virtws = AS(virtw); DO(rr, virtws[i] = ws[wf+i];)} AN(virtw)=wcn;  AFLAG(virtw)|=AFUNINCORPABLE;
  RZ(virtw = virtual(w,0,rr)); MCIS(AS(virtw),ws+wf,rr); AN(virtw)=wcn; AFLAG(virtw)|=AFUNINCORPABLE;
  // if the original block was direct inplaceable, make the virtual block inplaceable.  (We can't do this for indirect blocks because a virtual block is not marked recursive - rather it increments
  // the usecount of the entire backing block - and modifying the virtual contents would leave the usecounts invalid if the backing block is recursive.  Maybe could do this if it isn't?)
  // We will leave jtinplace set as it was coming into this routine.  It will be set only if the called function can handle inplacing; and <@f is inplaceable only if f is.
  // The final test for inplaceability lies with the function, and it will not detect reassignments of the cell.  Pity.
  // To save tests later we turn off inplacing if we can't use it here
  // loop over the frame
  I virtwk=AK(virtw);  // save virtual-operand pointer in case modified
  AC(virtw)=ACUC1|ACINPLACE; // mark the virtual block inplaceable; this will be ineffective unless the original w was direct inplaceable, and inplacing is allowed by u
#define ZZDECL
#include "result.h"
// obsolete   ZZPARMS(0,0,ws,wf,mn,1)
  ZZPARMS(wf,mn,1)
#define ZZINSTALLFRAME(optr) MCISd(optr,ws,wf)
  for(i0=mn;i0;--i0){
   RZ(z=CALL1IP(f1,virtw,fs));

#define ZZBODY  // assemble results
#include "result.h"

   // advance input pointer for next cell.  We keep the same virtual block because it can't be incorporated into anything; but the virtual block was inplaceable the
   // AK, AN, AR, AS, AT fields may have been modified.  We restore them
// obsolete   if(!((I)jtinplace&JTINPLACEW)){
   AK(virtw) = virtwk += wk;
   if(AFLAG(virtw)&AFVIRTUALINPLACE){
        // The block was self-virtualed.  Restore its original shape
     AR(virtw)=(RANKT)rr; AT(virtw)=wt; MCIS(AS(virtw),ws+wf,rr); AN(virtw)=wcn; AFLAG(virtw) &= ~AFVIRTUALINPLACE;  // restore all fields that might have been modified.  Pity there are so many
   }
  }

#define ZZEXIT
#include "result.h"

 }else{UC d; I *zzs;
  // no cells - execute on a cell of fills
  RZ(virtw=reshape(vec(INT,rr,ws+wf),filler(w)));  // The cell of fills
  // Do this quietly, because
  // if there is an error, we just want to use a value of 0 for the result; thus debug
  // mode off and RESETERR on failure.
  // However, if the error is a non-computational error, like out of memory, it
  // would be wrong to ignore it, because the verb might execute erroneously with no
  // indication that anything unusual happened.  So fail then
  d=jt->db; jt->db=0; z=CALL1(f1,virtw,fs); jt->db=d;
  if(jt->jerr){if(EMSK(jt->jerr)&EXIGENTERROR)RZ(z); z=zero; RESETERR;}  // use 0 as result if error encountered
  GA(zz,AT(z),0L,wf+AR(z),0L); zzs=AS(zz); MCISds(zzs,ws,wf); MCIS(zzs,AS(z),AR(z));
// obsolete   is = ws; DO(wf, *zzs++=*is++;);  // copy frame
// obsolete   is = AS(z); DO(AR(z), *zzs++=*is++;);    // copy result shape
 }

// result is now in zz

 AFLAG(zz)|=AFNOSMREL;  // obsolete.  We used to check state
 EPILOG(zz);
}

// Streamlined version when rank is 0.  In this version we look for ATOPOPEN (i. e. each and every)
// f1 is the function to use if there are no flags, OR if there is just 1 cell with no frame or a cell of fill 
A jtrank1ex0(J jt,AD * RESTRICT w,A fs,AF f1){F1PREFIP;PROLOG(0041);A z,virtw;
   I mn,n=1,wk,wr,*ws,wt;
 RZ(w);
 wr=AR(w);   // rank of w
// obsolete if(ARELATIVE(w))state|=STATEWREL;
 if(!wr){R CALL1IP(f1,w,fs);}  // if there's only one cell and no frame, run on it, that's the result.  Make this as fast as possible.
 // Switch to sparse code if argument is sparse
 wt=AT(w);
 if(wt&SPARSE)R sprank1(w,fs,0,f1);
#define ZZFLAGWORD state
 // wr=rank, ws->shape
 // Each cell is an atom.  Get # atoms (=#result cells)
 // ?k=#bytes in a cell, ?s->shape
 ws=AS(w); mn=AN(w); wk=bp(wt);

 A zz=0;  // place where we will build up the homogeneous result cells

 // Look for the forms we handle specially: <@:f (not here where rank=0)  <@f  f@>   and their combinations  <@(f@>) f&.> (<@:f)@>  but not as combinations  (<@f)@> (unless f has rank _) <@:(f@>)   also using &
 I state=0;
 I razeflags=FAV(fs)->flag2 & (VF2WILLBEOPENED|VF2COUNTITEMS);  // remember the raze flags from the outer level.  If we take a BOXATOP we will need them

 if(mn){  // if no cells, go handle fill before we advance over flags
  // Here there are cells to execute on.  Collect ATOP flags

  // RANKONLY verbs contain an invalid f1 pointer (it was used to get to a call to here).  We have to step over the RANKONLY to get to what we can execute
  while(FAV(fs)->flag2&VF2RANKONLY1){fs=FAV(fs)->f; f1=FAV(fs)->f1;}

  while(1){  // loop collecting ATOPs
   I fstate=(FAV(fs)->flag2&(VF2BOXATOP1|VF2ATOPOPEN1))>>(VF2BOXATOP1X-ZZFLAGBOXATOPX);  // extract <@ and @> status bits from f
   if(fstate&state||!fstate)break;  // If this f overlaps with old, or it's not just a flag node, we have to stop
// obsolete    // Skip over u"0 forms at the beginning, so that u"r can leave fs pointing there and thus pick up razeflags attached to it
// obsolete    if(!fstate){if(state||!(FAV(fs)->flag2&VF2RANKONLY1))break;  continue;}  // If no <> flags, it's a processing node, and we have to stop.  Exception: u"0.  Here, all u"r must be u"0
   if(fstate&ZZFLAGATOPOPEN1){
    // @> &> &.>
    //  Advance to the f of f@>
    fs=FAV(fs)->f; f1=FAV(fs)->f1;
   }else{
    // <@: <@ <& <&:
    // Because the outermost rank is 0, <@f by itself is OK; but later, as in (<@f)@>, it is not.  <@:f is.  So check for infinite rank
    if(state&ZZFLAGATOPOPEN1 && FAV(fs)->mr<RMAX)break;  // not first, and not infinite rank: ignore
    // Advance fs to the g of <@g
    fs=(FAV(fs)->flag2&VF2ISCCAP)?FAV(fs)->h:FAV(fs)->g; f1=FAV(fs)->f1;
   }
   state|=fstate;  // We accepted the new f, so take its flags
  }

  A *wav;   // virtwk is offset of virtual block/pointer to next box
  // Normal case where there are cells.
  // RANKONLY verbs were handled in the caller to this routine, but fs might be RANKATOP.  In that case we could include its rank in the loop here,
  // if its rank is not less than the outer rank (we would simply ignore it), but we don't bother.  If its rank is smaller we can't ignore it because assembly might affect
  // the order of fill.  But if f is BOXATOP, there will be no fill, and we can safely use the smaller rank
#define ZZDECL
#include "result.h"
// obsolete   ZZPARMS(0,0,ws,wf,mn,1)
  ZZPARMSNOFS(wr,mn)
  // if we are using the BOXATOP from f, we can also use the raze flags.  Set these only if BOXATOP to prevent us from incorrectly
  // marking the result block as having uniform items if we didn't go through the assembly loop here
  state |= (-(state&ZZFLAGBOXATOP)) & razeflags;  // These flags appear on ;@:(<@f)   (not on u"r)

  // Now that we have handled the structural requirements of ATOPOPEN, clear it if w is not open
  // Allocate a non-in-place virtual block unless this is ATOPOPEN and w is boxed, in which case we will just use the value of the A block
// obsolete  RZ(virtw = virtual(w,0,rr)); {I * virtws = AS(virtw); DO(rr, virtws[i] = ws[wf+i];)} AN(virtw)=wcn;  AFLAG(virtw)|=AFUNINCORPABLE;
  if(!(state&ZZFLAGATOPOPEN1)||!(wt&BOX)){
   RZ(virtw = virtual(w,0,0)); AN(virtw)=1; AFLAG(virtw) |= AFUNINCORPABLE; state&=~ZZFLAGATOPOPEN1;
  }else{wav=AAV(w); virtw=*wav++;}
#define ZZINSTALLFRAME(optr) MCISd(optr,ws,wr)
  do{
   RZ(z=CALL1(f1,virtw,fs));

#define ZZBODY  // assemble results
#include "result.h"

   if(--mn==0)break;  // exit loop before last fetch to avoid fetching out of bounds
   // advance input pointer for next cell.  We keep the same virtual block because it can't be incorporated into anything
// obsolete   if(!((I)jtinplace&JTINPLACEW)){
   if(!(state&ZZFLAGATOPOPEN1)){AK(virtw) += wk;}else{virtw=*wav++;}
  }while(1);

#define ZZEXIT
#include "result.h"

 }else{UC d; I *zzs;
  // no cells - execute on a cell of fills
  RZ(virtw=filler(w));  // The cell of fills
  // Do this quietly, because
  // if there is an error, we just want to use a value of 0 for the result; thus debug
  // mode off and RESETERR on failure.
  // However, if the error is a non-computational error, like out of memory, it
  // would be wrong to ignore it, because the verb might execute erroneously with no
  // indication that anything unusual happened.  So fail then
// obsolete  if(!(state&ZZFLAGBOXATOP)){
  if(!(FAV(fs)->flag2&VF2BOXATOP1)){
   d=jt->db; jt->db=0; z=CALL1(f1,virtw,fs); jt->db=d;   // normal execution on fill-cell
   if(jt->jerr){if(EMSK(jt->jerr)&EXIGENTERROR)RZ(z); z=zero; RESETERR;}  // use 0 as result if error encountered
  }else{
   // If we are executing a BOXATOP on a single cell, we know the result is going to be an atomic box.  We don't bother executing the verb at all then.
   // jmf.ijs unknowingly takes advantage of this fact, and would crash if executed on an empty cell
   z=ace;  // cell 'returned' a:
  }
  GA(zz,AT(z),0L,wr+AR(z),0L); zzs=AS(zz); MCISds(zzs,ws,wr); MCIS(zzs,AS(z),AR(z));
// obsolete   is = ws; DO(wr, *zzs++=*is++;);  // copy frame
// obsolete   is = AS(z); DO(AR(z), *zzs++=*is++;);    // copy result shape
 }

// result is now in zz

 AFLAG(zz)|=AFNOSMREL;  // obsolete.  We used to check state
 EPILOG(zz);
}

A jtrank2ex(J jt,AD * RESTRICT a,AD * RESTRICT w,A fs,I lr,I rr,I lcr,I rcr,AF f2){F2PREFIP;PROLOG(0042);A virta,virtw,z;
   I acn,af,ak,ar,*as,at,mn,n=1,wcn,wf,wk,wr,*ws,wt;
 I outerframect, outerrptct, innerframect, innerrptct, aof, wof, sof, lof, sif, lif, *lis, *los;
 RZ(a&&w);
 at=AT(a); wt=AT(w);
 if(at&SPARSE||wt&SPARSE)R sprank2(a,w,fs,lcr,rcr,f2);  // this needs to be updated to handle multiple ranks
// lr,rr are the ranks of the underlying verb.  lcr,rcr are the cell-ranks given by u"lcr rcr.
// If " was not used, lcr,rcr=lr,rr
// When processing v"l r the shapes look like:
// a frame   x x O  | x x x
//                   <---l-->
// w frame   x x    | x x x I
//                   <---r-->
// the outer frame is to the left of the |, inner frame to the right.
// the rank of v is not included; the frames shown above pick up after that.  There are two
// possible repetitions required: if there is mismatched frame BELOW the rank (l r), as shown by letter I above,
// the individual cells of the shorter-frame argument must be repeated.  innerrptct gives the
// number of times the cell should be repeated.  If there is mismatched frame ABOVE the rank (l r), as
// shown by letter O above, the rank-(l/r) cell of the short-frame argument must be repeated.  outerrptct
// tells how many times the cell should be repeated; outerrestartpt gives the address of the rank-(l/r) cell
// being repeated; outercellct gives the number of (below lr) cells that are processed before an outer repetition.
// The two repeats can be for either argument independently, depending on which frame is shorter.

 // ?r=rank, ?s->shape, ?cr=effective rank, ?f=#total frame (inner+outer), for each argument
 // if inner rank is > outer rank, set it equal to outer rank
#define ZZFLAGWORD state
 I ZZFLAGWORD=0;  // init flags, including zz flags
// obsolete  I state=STATEFIRST|AFNOSMREL;  // initial state: working on first item, OK to pop stack, no relative contents, etc
 ar=AR(a); as=AS(a); efr(lcr,ar,lcr); efr(lr,lcr,lr);// obsolete  if(lr>lcr)lr=lcr;
// obsolete  state |= STATEAREL&~ARELATIVES(a);   // relies on STATEAREL>BOX
// obsolete if(ARELATIVE(a))state|=STATEAREL;
 wr=AR(w); ws=AS(w); efr(rcr,wr,rcr); efr(rr,rcr,rr);// obsolete  if(rr>rcr)rr=rcr;

 // RANKONLY verbs were handled in the caller to this routine, but fs might be RANKATOP.  In that case we can include its rank in the loop here, which will save loop setups
 if(fs&&(I)(((FAV(fs)->flag2&(VF2RANKATOP2|VF2BOXATOP2))-1)|(-(rr^rcr))|(-(lr^lcr)))>=0){I lrn, rrn;  // prospective new ranks to include
  efr(lrn,lr,(I)FAV(fs)->lr); efr(rrn,rr,(I)FAV(fs)->rr);  // get the ranks if we accept the new cell
// obsolete   if((((lrn-lr)&(lr-lcr))|((rrn-rr)&(rr-rcr)))>=0){  //  if either side has 3 different ranks, stop, no room
  lr=lrn; rr=rrn;   // We can include the @ in the loop.  That means we can honor its BOXATOP too...
  state = (FAV(fs)->flag2&VF2BOXATOP2)>>(VF2BOXATOP2X-ZZFLAGBOXATOPX);  // If this is BOXATOP, set so for loop.  Don't touch fs yet, since we might not loop
  state &= ~((FAV(fs)->flag2&VF2ATOPOPEN2W)>>(VF2ATOPOPEN2WX-ZZFLAGBOXATOPX));  // We don't handle &.> here; ignore it
  // if we are using the BOXATOP from f, we can also use the raze flags.  Set these only if BOXATOP to prevent us from incorrectly
  // marking the result block as having uniform items if we didn't go through the assembly loop here
  state |= (-state) & FAV(fs)->flag2 & (VF2WILLBEOPENED|VF2COUNTITEMS);
// obsolete  }
 }

 af=ar-lr; wf=wr-rr;   // frames wrt innermost cell
// obsolete  state |= STATEWREL&~ARELATIVES(w);   // relies on STATEWREL>BOX
 if(!(af|wf)){R CALL2IP(f2,a,w,fs);}  // if there's only one cell and no frame, run on it, that's the result.
 // multiple cells.  Loop through them.

 // Get the length of the outer frames, which are needed only if either "-rank is greater than the verb rank,
 // either argument has frame with respect to the "-ranks, and those frames are not the same length
 aof=ar-lcr; wof=wr-rcr;   // ?of = outer frame
// obsolete  if(!((lcr>lr||rcr>rr)&&((aof>0)||(wof>0))&&aof!=wof)){los=0; lof=aof=wof=0; outerframect=outerrptct=1;  // no outer frame unless it's needed
 if(0<=(((lr-lcr)|(rr-rcr))&(-(aof^wof)))){los=0; lof=aof=wof=0; outerframect=outerrptct=1;  // no outer frame unless it's needed
 }else{
  // outerframect is the number of cells in the shorter frame; outerrptct is the number of cells in the residual frame
  if(aof>=wof){wof=wof<0?0:wof; lof=aof; sof=wof; los=as;}else{aof=aof<0?0:aof;  lof=wof; sof=aof; los=ws; state|=STATEOUTERREPEATA;}  // clamp smaller frame at min=0
// obsolete   ASSERT(!ICMP(as,ws,sof),EVLENGTH);
  DO(sof, ASSERT(as[i]==ws[i], EVLENGTH);)  // prefixes must agree
  RE(outerframect=prod(sof,los)); RE(outerrptct=prod(lof-sof,los+sof));  // get # cells in frame, and in unmatched frame
 }

 // Now work on inner frames.  Compare frame lengths after discarding outer frames
 // set lif=length of longer inner frame, sif=length of shorter inner frame, lis->longer inner shape
 if((af-aof)==(wf-wof)){
  // inner frames are equal.  No repeats
  lif=wf-wof; sif=af-aof; lis=ws+wof;
 } else if((af-aof)<(wf-wof)){
  // w has the longer inner frame.  Repeat cells of a
  lif=wf-wof; sif=af-aof; lis=ws+wof;
  state |= STATEINNERREPEATA;
 } else{
  // a has the longer inner frame.  Repeat cells of w
  lif=af-aof; sif=wf-wof; lis=as+aof;
  state |= STATEINNERREPEATW;
 }
// obsolete  ASSERT(!ICMP(as+aof,ws+wof,sif),EVLENGTH);  // error if frames are not same as prefix
 DO(sif, ASSERT(as[aof+i]==ws[wof+i],EVLENGTH);)  // error if frames are not same as prefix
 RE(innerrptct=prod(lif-sif,lis+sif));  // number of repetitions per matched-frame cell
 RE(innerframect=prod(sif,lis));   // number of cells in matched frame

 I an=AN(a), wn=AN(w);  // empty-operand indicators
 // Migrate loops with count=1 toward the inner to reduce overhead.  We choose not to promote the outer to the inner if both
 // innerframect & innerrptct are 1, on grounds of rarity
 if(innerrptct==1){innerrptct=innerframect; innerframect=1; state &=~(STATEINNERREPEATW|STATEINNERREPEATA);}  // if only one loop needed, make it the inner, with no repeats

 // Get size of each argument cell in atoms.  If this overflows, there must be a 0 in the frame, & we will have
 // gone through the fill path (& caught the overflow)
 PROD(acn,lr,as+af); PROD(wcn,rr,ws+wf);
 // Allocate workarea y? to hold one cell of ?, with uu,vv pointing to the data area y?
 // ?cn=number of atoms in a cell, ?k=#bytes in a cell
 ak=acn*bp(at);    // reshape below will catch any overflow
 wk=wcn*bp(wt);

 // See how many cells are going to be in the result
 RE(mn=mult(mult(outerframect,outerrptct),mult(innerframect,innerrptct)));

 // See which arguments we can inplace.  The key is that they have to be not repeated.  This means outerrptct=1, and the specified argument not repeated in the inner loop.  Also,
 // a and w mustn't be the same block (one cannot be a virtual of the other unless the backer's usecount disables inplacing)
 jtinplace = (J)((I)jtinplace & ~(((a==w)|(outerrptct!=1))*(JTINPLACEA+JTINPLACEW)|(state>>STATEINNERREPEATWX)));  // turn off inplacing if variable is inner-repeated, or any outer repeat, or identical args

 // allocate the virtual blocks that we will use for the arguments, and fill in the shape of a cell of each
 // The base pointer AK advances through the source argument.  But if an operand is empty (meaning that there are no output cells),
 // replace any empty operand with a cell of fills.  (Note that operands can have no atoms and yet the result can have nonempty cells,
 // if the cells are empty but the frame does not contain 0)
 //
 // Self-virtual blocks modify the shape of a block, but that code notifies
 // us through a flag bit.
 fauxblock(virtwfaux); fauxblock(virtafaux); 
 if(mn|an){
  jtinplace = (J)((I)jtinplace & ((((at&TYPEVIPOK)!=0)&(AC(a)>>(BW-1)))*JTINPLACEA+~JTINPLACEA));  // turn off inplacing unless DIRECT and a is inplaceable.
// obsolete  RZ(virta = virtual(a,0,lr)); {I * virtas = AS(virta); DO(lr, virtas[i] = as[af+i];)} AN(virta)=acn; AFLAG(virta)|=AFUNINCORPABLE;
// obsolete    RZ(virta = virtual(a,0,lr));
  fauxvirtual(virta,virtafaux,a,lr) MCIS(AS(virta),as+af,lr); AN(virta)=acn;
// obsolete AFLAG(virta)|=AFUNINCORPABLE;
  AC(virta)=ACUC1|ACINPLACE; // mark the virtual block inplaceable; this will be ineffective unless the original a was direct inplaceable, and inplacing is allowed by u
 }else{RZ(virta=reshape(vec(INT,lr,as+af),filler(a)));}

 if(mn|wn){  // repeat for w
  jtinplace = (J)((I)jtinplace & ((((wt&TYPEVIPOK)!=0)&(AC(w)>>(BW-1)))*JTINPLACEW+~JTINPLACEW));  // turn off inplacing unless DIRECT and w is inplaceable.
// obsolete   RZ(virtw = virtual(w,0,rr)); 
  fauxvirtual(virtw,virtwfaux,w,rr) MCIS(AS(virtw),ws+wf,rr); AN(virtw)=wcn;
// obsolete  AFLAG(virtw)|=AFUNINCORPABLE;
  AC(virtw)=ACUC1|ACINPLACE; // mark the virtual block inplaceable; this will be ineffective unless the original w was direct inplaceable, and inplacing is allowed by u
 }else{RZ(virtw=reshape(vec(INT,rr,ws+wf),filler(w)));}

 A zz=0;  // place where we will build up the homogeneous result cells
 if(mn){I i0, i1, i2, i3;
  // Normal case where there are cells.
  I virtak=AK(virta);  // save virtual-operand pointer in case modified
  I virtwk=AK(virtw);  // save virtual-operand pointer in case modified
  
  // loop over the matched part of the outer frame

#define ZZDECL
#include "result.h"
// obsolete   ZZPARMS(los,lof,lis,lif,mn,2)
  ZZPARMS(lof+lif,mn,2)
#define ZZINSTALLFRAME(optr) MCISd(optr,los,lof) MCISd(optr,lis,lif)

  for(i0=outerframect;i0;--i0){
   I outerrptstart=state&STATEOUTERREPEATA?virtak:virtwk;
   // loop over the unmatched part of the outer frame, repeating the shorter argument
   for(i1=outerrptct;i1;--i1){  // make MOVEY? post-increment
    if(state&STATEOUTERREPEATA){AK(virta) = virtak = outerrptstart;}else{AK(virtw) = virtwk = outerrptstart;}
    // loop over the matched part of the inner frame
    for(i2=innerframect;i2;--i2){
     // loop over the unmatched part of the inner frame, repeating the shorter argument
     for(i3=innerrptct;i3;--i3){
      // invoke the function, get the result for one cell
      RZ(z=CALL2IP(f2,virta,virtw,fs));

#define ZZBODY  // assemble results
#include "result.h"

#if 0
      // if the result is boxed, accumulate the SMREL info
      if(state&AFNOSMREL)state&=AFLAG(y)|~AFNOSMREL;  // if we ever get an SMREL (or a non-boxed result), stop looking

      // process according to state
      if(state&STATENORM){
       // Normal case: not first time, no error found yet.  Move verb result to its resting place.  zv points to the next output location
       if(TYPESNE(yt,AT(y))||yr!=AR(y)||yr&&ICMP(AS(y),ys,yr)||ARELATIVE(y)){state^=(STATENORM|STATEERR0);}  //switch to ERR0 state if there is a change of cell type/rank/shape, or result is relative
       else{
        // Normal path.
        MC(zv,AV(y),k); zv+=k;  // move the result-cell to the output, advance to next output spot
          // If the result-cells are pointers to boxes, we are adding a nonrecursive reference, which does not require any adjustment to usecounts.
          // If we anticipate making the result recursive, we will have to increment the usecount and also worry about backing out errors and wrecks.
        if(!(state&STATENOPOP))tpop(old);  // Now that we have copied to the output area, free what the verb allocated
       }
      } else if(state&STATEFIRST){I *is, zn;
       // Processing the first cell.  Allocate the result area now that we know the shape/type of the result.  If an argument is memory-mapped,
       // we have to go through the box/unbox drill, because the blocks coming out will have different relocations that must be preserved.
       //  In that case, we switch this allocation to be a single box per result-cell.
       //  We also have to do this for sparse results, so that they will be collected into a single result at the end
       yt=AT(y);  // type of the first result
       if(!( ((AFLAG(a)|AFLAG(w))&(AFNJA|AFSMM|AFREL)) || (yt&SPARSE) ) ){
        yr=AR(y); yn=AN(y);
        RE(zn=mult(mn,yn));   // zn=number of atoms in all result cells (if they stay homogeneous)
        state^=(STATEFIRST|STATENORM);  // advance to STATENORM
        // If the results are not going to be DIRECT, they will be allocated up the stack, and we mustn't pop the stack between results
        if(!(yt&DIRECT))state |= STATENOPOP;
       }else{
        yt=BOX; yr=0; zn=mn; state^=(STATEFIRST|STATEERR);
       }
       GA(z,yt,zn,lof+lif+yr,0L); I *zs=AS(z); zv=CAV(z);
       is = los; DO(lof, *zs++=*is++;);  // copy outer frame
       is = lis; DO(lif, *zs++=*is++;);  // copy inner frame
       if(!(state&STATEERR)){
        ys=AS(y); k=yn*bp(yt);   // save info about the first cell for later use
        is = AS(y); DO(yr, *zs++=*is++;);    // copy result shape
        MC(zv,AV(y),k); zv+=k;   // If there was a first cell, copy it in & advance to next output spot
        old=jt->tnextpushx;  // pop back to AFTER where we allocated our result and argument blocks
       }
      }

      if(state&(STATEERR0|STATEERR)){
       if(state&STATEERR0){
        // We had a wreck.  Either the first cell was not direct/boxed, or there was a change of type.  We cope by boxing
        // each individual result, so that we can open them at the end to produce a single result (which might fail when opened)
        // If the result is boxed, it means we detected the wreck before the initial allocation.  The initial allocation
        // is the boxed area where we build <"0 result, and zv points to the first box pointer.  We have nothing to adjust.
        C *zv1=CAV(z);   // pointer to cell data
        A zsav=z; GATV(z,BOX,mn,lof+lif,AS(zsav)); A *x=AAV(z);   // allocate place for boxed result; copy frame part of result-shape.  Note GATV reassigns z early, need zsav
        // For each previous result, put it into a box and store the address in the result area
        // We have to calculate the number of cells, rather than using the output address, because the length of a cell may be 0
        // wrecki does not include the cell that caused the wreck
        I wrecki = (innerrptct-i3) + innerrptct * ( (innerframect-i2) + innerframect * ( (outerrptct-i1) + outerrptct * (outerframect-i0) ) );
        DQ(wrecki , A q; GA(q,yt,yn,yr,ys); MC(AV(q),zv1,k); zv1+=k; *x++=q;)  // We know the type/shape/rank of the first result matches them all
        // from now on the main output pointer, zv, points into the result area for z
        zv = (C*)x;
        state^=(STATEERR0|STATEERR);  // advance to STATEERR
       }
       // Here for all errors, including the first after it has cleaned up the mess, and for sparse result the very first time with no mess
       // we are incorporating y into the boxed z, so we have to mark it as such (and possibly reallocate it)
       INCORP(y);
       *(A*)zv=y; zv+=sizeof(A*);   // move in the most recent result, advance pointer to next one
      }
#endif
      // advance input pointers for next cell.  We keep the same virtual block because it can't be incorporated into anything; but if the virtual block was inplaceable the
   // AK, AN, AR, AS, AT fields may have been modified.  We restore them
      if(!(state&STATEINNERREPEATA)){
       AK(virta) = virtak += ak;
       if(AFLAG(virta)&AFVIRTUALINPLACE){
        // The block was self-virtualed.  Restore its original shape
        AR(virta)=(RANKT)lr; AT(virta)=at; MCIS(AS(virta),as+af,lr); AN(virta)=acn; AFLAG(virta) &= ~AFVIRTUALINPLACE;  // restore all fields that might have been modified.  Pity there are so many
       }
      }

      if(!(state&STATEINNERREPEATW)){
       AK(virtw) = virtwk += wk;
       if(AFLAG(virtw)&AFVIRTUALINPLACE){
        AR(virtw)=(RANKT)rr; AT(virtw)=wt; MCIS(AS(virtw),ws+wf,rr); AN(virtw)=wcn; AFLAG(virtw) &= ~AFVIRTUALINPLACE;  // restore all fields that might have been modified.  Pity there are so many
       }
      }
     }
      // advance input pointers for next cell.  We increment any block that was being held constant in the inner loop.  There can be only one such.  Such an arg is never inplaced
     if(state&STATEINNERREPEATA)AK(virta) = virtak += ak;
     else if(state&STATEINNERREPEATW)AK(virtw) = virtwk += wk;
    }
   }
  }

#define ZZEXIT
#include "result.h"

 }else{UC d; I *zzs;
  // if there are no cells, execute on a cell of fills.
  // Do this quietly, because
  // if there is an error, we just want to use a value of 0 for the result; thus debug
  // mode off and RESETERR on failure.
  // However, if the error is a non-computational error, like out of memory, it
  // would be wrong to ignore it, because the verb might execute erroneously with no
  // indication that anything unusual happened.  So fail then
  d=jt->db; jt->db=0; z=CALL2(f2,virta,virtw,fs); jt->db=d;
  if(jt->jerr){if(EMSK(jt->jerr)&EXIGENTERROR)RZ(z); z=zero; RESETERR;}  // use 0 as result if error encountered
  GA(zz,AT(z),0L,lof+lif+AR(z),0L); zzs=AS(zz);
  MCISds(zzs,los,lof); MCISds(zzs,lis,lif); MCIS(zzs,AS(z),AR(z));
// obsolete   is = los; DO(lof, *zzs++=*is++;);  // copy outer frame
// obsolete   is = lis; DO(lif, *zzs++=*is++;);  // copy inner frame
// obsolete   is = AS(z); DO(AR(z), *zzs++=*is++;);    // copy result shape
 }

// obsolete  if(state&STATEERR){z=ope(z);  // If we went to error state, we have created x <@f y; this creates > x <@f y which is the final result
// obsolete  }else{AFLAG(z)|=state&AFNOSMREL;}  // if not error, we saw all the subcells, so if they're all non-rel we know.  This may set NOSMREL in a non-boxed result, but that's OK
// result is now in zz

 AFLAG(zz)|=AFNOSMREL;  // obsolete.  We used to check state
 EPILOG(zz);
}

// version for rank 0.  We look at ATOPOPEN too.  f2 is the function to use if there is no frame
A jtrank2ex0(J jt,AD * RESTRICT a,AD * RESTRICT w,A fs,AF f2){F2PREFIP;PROLOG(0042);A virta,virtw,z;
   I ak,ar,*as,at,ict,oct,mn,wk,wr,*ws,wt;
 RZ(a&&w); ar=AR(a); wr=AR(w); if(!(ar|wr))R CALL2IP(f2,a,w,fs);   // if no frame, make just 1 call
 at=AT(a); wt=AT(w);
 if(at&SPARSE||wt&SPARSE)R sprank2(a,w,fs,0,0,f2);  // this needs to be updated to handle multiple ranks
#define ZZFLAGWORD state

 // Verify agreement
 as=AS(a); ws=AS(w); DO(MIN(ar,wr), ASSERT(as[i]==ws[i], EVLENGTH););

 // Calculate strides for inner and outer loop.  Cell-size is known to be 1 atom.  The stride of the inner loop is 1 atom, except for a
 // repeated value, of which there can be at most 1.  For a repeated value, we set the stride to 0 and remember the repetition count and stride
 ak=bp(at); wk=bp(wt);  // stride for 1 atom
 if(ar>=wr) { // a will be the long frame
  mn=AN(a);  // result has shape of longer frame, & same # atoms
  if(ar==wr){  // no surplus frame: common case
   ict=mn; oct=0; // leave frames as is, set loop counters
  }else{
   PROD(oct,wr,as); PROD(ict,ar-wr,as+wr); wr=wk; wk=0;  // set repeat counts, outer/inner strides
  }
 }else{
   mn=AN(w);
   PROD(oct,ar,ws); PROD(ict,wr-ar,ws+ar); as=ws; ar=wr; wr=ak; ak=0;  // set repeat counts, outer/inner strides
 }
 // Now as/ar are the addr/length of the long frame
 // wr is the atomsize*repeatct for the repeated arg
 // ak/wk are strides, 0 for a repeated arg
 // ict is the inner repeat count: length of surplus frame if any, else length of args
 // oct is the outer repeat count: length of common frame if there is a surplus frame, else 0



 // Look for the forms we handle specially: <@:f (not here where rank=0)  <@f  f@>   and their combinations  <@(f@>) f&.> (<@:f)@>  but not as combinations  (<@f)@> (unless f has rank _) <@:(f@>)   also using &
 // For the nonce, we assume that VF2ATOPOPEN2A and VF2ATOPOPEN2W are always the same
 I state=0;
 I razeflags=FAV(fs)->flag2 & (VF2WILLBEOPENED|VF2COUNTITEMS);  // remember the raze flags from the outer level.  If we take a BOXATOP we will need them

 A zz=0;  // place where we will build up the homogeneous result cells
 if(mn){
  // Collect flags <@ and @> from the nodes.  It would be nice to do this even on empty arguments, but that would complicate our job in coming up with a fill-cell or argument cell, because
  // we would have to keep track of whether we passed an ATOPOPEN.  But then we could avoid executing the fill cell any time the is a BOXATOP, even down the stack.  As it is, the only time we
  // elide the execution is when BOXATOP occurs at the first node, i.e. for an each that is not boxed

  // RANKONLY verbs contain an invalid f1 pointer (it was used to get to a call to here).  We have to step over the RANKONLY to get to what we can execute
  while(FAV(fs)->flag2&VF2RANKONLY2){fs=FAV(fs)->f; f2=FAV(fs)->f2;}

  while(1){  // loop collecting ATOPs
   I fstate=(FAV(fs)->flag2&(VF2BOXATOP2|VF2ATOPOPEN2A|VF2ATOPOPEN2W))>>(VF2BOXATOP2X-ZZFLAGBOXATOPX);  // extract <@ and @> status bits from f
   if(fstate&state||!fstate)break;  // If this f overlaps with old, or it's not a flag-only node, we have to stop
// obsolete    // Skip over u"r forms at the beginning, so that u"r can leave fs pointing there and thus pick up razeflags attached to it
// obsolete    if(!fstate){if(state|(~FAV(fs)->flag2&VF2RANKONLY2))break; fs=FAV(fs)->f; f2=FAV(fs)->f2; continue;}  // If no <> flags, it's a processing node, and we have to stop.  Exception: u"0 at beginning.  Here, all u"r must be u"0
   if(fstate&ZZFLAGATOPOPEN2W){
    // @> &> &.>
    //  Advance to the f of f@>
    fs=FAV(fs)->f; f2=FAV(fs)->f2;
   }else{
    // <@: <@ <& <&:
    // Because the outermost rank is 0, <@f by itself is OK; but later, as in (<@f)@>, it is not.  <@:f is.  So check for infinite rank
    if(state&ZZFLAGATOPOPEN2W && FAV(fs)->mr<RMAX)break;  // not first, and not infinite rank: ignore
    // Advance fs to the g of <@g
    fs=(FAV(fs)->flag2&VF2ISCCAP)?FAV(fs)->h:FAV(fs)->g; f2=FAV(fs)->f2;
   }
   state|=fstate;  // We accepted the new f, so take its flags
  }

  // allocate the virtual blocks that we will use for the arguments, and fill in the shape of a cell of each
  // The base pointer AK advances through the source argument.  But if an operand is empty (meaning that there are no output cells),
  // replace any empty operand with a cell of fills.  (Note that operands can have no atoms and yet the result can have nonempty cells,
  // if the cells are empty but the frame does not contain 0)
  //
  // Self-virtual blocks modify the shape of a block, but that code notifies
  // us through a flag bit.
  A *aav, *wav;
  // Normal case where there are cells.

  // if we are using the BOXATOP from f, we can also use the raze flags.  Set these only if BOXATOP to prevent us from incorrectly
  // marking the result block as having uniform items if we didn't go through the assembly loop here
  state |= (-(state&ZZFLAGBOXATOP)) & razeflags;  // These flags appear on ;@:(<@f)   (not on u"r)

  // Now that we have handled the structural requirements of ATOPOPEN, clear it if the argument is not boxed
  // Allocate a non-in-place virtual block unless this is ATOPOPEN and w is boxed, in which case we will just use the value of the A block
// obsolete  RZ(virtw = virtual(w,0,rr)); {I * virtws = AS(virtw); DO(rr, virtws[i] = ws[wf+i];)} AN(virtw)=wcn;  AFLAG(virtw)|=AFUNINCORPABLE;
  if(!(state&ZZFLAGATOPOPEN2W)||!(wt&BOX)){
   RZ(virtw = virtual(w,0,0)); AN(virtw)=1; AFLAG(virtw) |= AFUNINCORPABLE; state&=~ZZFLAGATOPOPEN2W;
  }else{wav=AAV(w); virtw=*wav;}
  if(!(state&ZZFLAGATOPOPEN2A)||!(at&BOX)){
   RZ(virta = virtual(a,0,0)); AN(virta)=1; AFLAG(virta) |= AFUNINCORPABLE; state&=~ZZFLAGATOPOPEN2A;
  }else{aav=AAV(a); virta=*aav;}
  
  // loop over the matched part of the outer frame

#define ZZDECL
#include "result.h"
// obsolete   ZZPARMS(los,lof,lis,lif,mn,2)
  ZZPARMSNOFS(ar,mn)
#define ZZINSTALLFRAME(optr) MCISd(optr,as,ar)

  do{I i0=ict;
   do{
    RZ(z=CALL2IP(f2,virta,virtw,fs));

#define ZZBODY  // assemble results
#include "result.h"

    if(--i0==0)break;  // stop before we load the last+1 item
    if(!(state&ZZFLAGATOPOPEN2A)){AK(virta) += ak;}else{aav=(A*)((I)aav+ak); virta=*aav;}
    if(!(state&ZZFLAGATOPOPEN2W)){AK(virtw) += wk;}else{wav=(A*)((I)wav+wk); virtw=*wav; }
   }while(1);
   // we have stopped with the pointers pointing to the last item read.  Advance them both to the next atom
   if(--oct<=0)break;  // if no more cells, avoid fetching out of bounds
   if(!(state&ZZFLAGATOPOPEN2A)){AK(virta) += ak?ak:wr;}else{virta=*++aav;}
   if(!(state&ZZFLAGATOPOPEN2W)){AK(virtw) += wk?wk:wr;}else{virtw=*++wav;}
  }while(1);

#define ZZEXIT
#include "result.h"

 }else{UC d; I *zzs;
  // if there are no cells, execute on a cell of fills.
  // Do this quietly, because
  // if there is an error, we just want to use a value of 0 for the result; thus debug
  // mode off and RESETERR on failure.
  // However, if the error is a non-computational error, like out of memory, it
  // would be wrong to ignore it, because the verb might execute erroneously with no
  // indication that anything unusual happened.  So fail then

  if(!(FAV(fs)->flag2&VF2BOXATOP2)){
   if(!AN(a)){RZ(virta=filler(a));}else{virta = virtual(a,0,0); AN(virta)=1;}  // if there are cells, use first atom; else fill atom
   if(!AN(w)){RZ(virtw=filler(w));}else{virtw = virtual(w,0,0); AN(virtw)=1;}
   d=jt->db; jt->db=0; z=CALL2(f2,virta,virtw,fs); jt->db=d;   // normal execution on fill-cell
   if(jt->jerr){if(EMSK(jt->jerr)&EXIGENTERROR)RZ(z); z=zero; RESETERR;}  // use 0 as result if error encountered
  }else{
   // If we are executing a BOXATOP on a single cell, we know the result is going to be an atomic box.  We don't bother executing the verb at all then.
   z=ace;  // cell 'returned' a:
  }
  GA(zz,AT(z),0L,ar+AR(z),0L); zzs=AS(zz); MCISds(zzs,as,ar); MCIS(zzs,AS(z),AR(z));  // allocate result, copy frame and shape
 }

// obsolete  if(state&STATEERR){z=ope(z);  // If we went to error state, we have created x <@f y; this creates > x <@f y which is the final result
// obsolete  }else{AFLAG(z)|=state&AFNOSMREL;}  // if not error, we saw all the subcells, so if they're all non-rel we know.  This may set NOSMREL in a non-boxed result, but that's OK
// result is now in zz

 AFLAG(zz)|=AFNOSMREL;  // obsolete.  We used to check state
 EPILOG(zz);
}

// Call a function that has Integrated Rank Support
// The function may leave the rank set on exit; we clear it
/* f knows how to compute f"r                           */
// jt->ranks is rank of monad or leftrank<<16 + rightrank
// jt->ranks is ~0 if the call is not through IRS
  // every call to IRS resets jt->ranks at the end
/* frame agreement is verified before invoking f        */
/* frames either match, or one is empty                 */
/* (i.e. prefix agreement invokes general case)         */
// If the action verb handles inplacing, we pass that through

// irs1() and irs2() are simply calls to the IRS-savvy function f[12] with the specified rank

A jtirs1(J jt,A w,A fs,I m,AF f1){A z;I wr; 
 F1PREFIP; RZ(w);
// obsolete  wr=AR(w); rv[1]=efr(m,wr,m);
// Get the rank of w; if the requested rank m is > wr, use ~0 because some verbs test for that as an expedient
// If m is negative, use wr+m but never < 0
 wr=AR(w); m=m>=wr?(RANKT)~0:m; wr+=m; wr=wr<0?0:wr; wr=m>=0?m:wr;   // requested rank, after negative resolution, or ~0
// obsolete  if(fs&&!(VAV(fs)->flag&VINPLACEOK1))jtinplace=jt;  // pass inplaceability only if routine supports it
// obsolete  jt->ranks=(RANK2T)m;   // set rank to use.  May be > actual arg rank.  Set to ~0 if possible
 jt->ranks=(RANK2T)wr;  // install rank for called routine
// obsolete   if(m>=wr)z = CALL1IP(f1,w,fs);  // just 1 cell
// obsolete   else{
// obsolete    rv[0]=0;  // why?
// obsolete   old=jt->rank; jt->rank=rv; z=CALL1IP(f1,w,fs); jt->rank=old;
 z=CALL1IP(f1,w,fs);
// obsolete   }
 jt->ranks=(RANK2T)~0;  // reset rank to infinite
 RETF(z);
}

// IRS setup for dyads x op y.  This routine sets jt->rank and calls the verb, which loops if it needs to
// a is x, w is y
// fs is the f field of the verb (the verb to be applied repeatedly) - or 0 if none (if we are called internally)
//  if inplacing is enabled in jt, fs must be given
// l, r are nominal ranks of fs
// f2 is the verb that does the work (jtover, jtreshape, etc).  Normally it will loop using rank?ex if it needs to
// IRS verbs are those that look at jt->rank.  This is where we set up jt->rank.  Once
// we have it, we call the setup verb, which will go on to do its internal looping and (optionally) call
// the verb f2 to finish operation on a cell
A jtirs2(J jt,A a,A w,A fs,I l,I r,AF f2){A z;I ar,wr;
 F2PREFIP; RZ(a&&w);
// obsolete  ar=AR(a); rv[0]=efr(l,ar,l); af=ar-l;  // get rank, effective rank of u"n, length of frame...
// obsolete  wr=AR(w); rv[1]=efr(r,wr,r); wf=wr-r;     // ...for both args
 wr=AR(w); r=r>=wr?(RANKT)~0:r; wr+=r; wr=wr<0?0:wr; wr=r>=0?r:wr; r=AR(w)-wr;   // wr=requested rank, after negative resolution, or ~0; r=frame of w, possibly negative if no frame
 ar=AR(a); l=l>=ar?(RANKT)~0:l; ar+=l; ar=ar<0?0:ar; ar=l>=0?l:ar; l=AR(a)-ar;   // ar=requested rank, after negative resolution, or ~0; l=frame of a, possibly negative if no frame
 DO(MIN(r,l), ASSERT(AS(a)[i]==AS(w)[i],EVLENGTH);)  // verify agreement before we modify jt->ranks
// obsolete  ASSERT(!ICMP(AS(a),AS(w),MIN(af,wf)),EVLENGTH);
// obsolete  jt->ranks=(RANK2T)((l<<16)+r);  // install as parm to the function.  Set to ~0 if possible
 jt->ranks=(RANK2T)((ar<<RANKTX)+wr);  // install as parm to the function.  Set to ~0 if possible
// obsolete  if(fs&&!(VAV(fs)->flag&VINPLACEOK2))jtinplace=jt;  // pass inplaceability only if routine supports it
// obsolete  if(!(af|wf))z = CALL2IP(f2,a,w,fs);   // if no frame, call setup verb and return result
// obsolete  else{
// obsolete   old=jt->rank; jt->rank=rv; z=CALL2IP(f2,a,w,fs); jt->rank=old;   // save ranks, call setup verb, pop rank stack
 z=CALL2IP(f2,a,w,fs);   // save ranks, call setup verb, pop rank stack
   // Not all verbs (*f2)() use the fs argument.
// obsolete  }
 jt->ranks=(RANK2T)~0;  // reset rank to infinite
 RETF(z);
}


static DF1(cons1a){R FAV(self)->f;}
static DF2(cons2a){R FAV(self)->f;}

// Constant verbs do not inplace because we loop over cells.  We could speed this up if it were worthwhile.
static DF1(cons1){V*sv=FAV(self);
 RZ(w);
 I mr; efr(mr,AR(w),*AV(sv->h));
 R rank1ex(w,self,mr,cons1a);
}
static DF2(cons2){V*sv=FAV(self);I*v=AV(sv->h);
 RZ(a&&w);
 I lr2,rr2; efr(lr2,AR(a),v[1]); efr(rr2,AR(w),v[2]);
 R rank2ex(a,w,self,AR(a),AR(w),lr2,rr2,cons2a);
}

// Handle u"n y where u supports irs.  Since the verb may support inplacing even with rank (,"n for example), pass that through.
// If inplacing is allowed here, pass that on to irs.  It will see whether the action verb can support inplacing.
// THIS SUPPORTS INPLACING: NOTHING HERE MAY DEREFERENCE jt!!
static DF1(rank1i){DECLF;A h=sv->h;I*v=AV(h); R irs1(w,fs,*v,f1);}
static DF2(rank2i){DECLF;A h=sv->h;I*v=AV(h); R irs2(a,w,fs,v[1],v[2],f2);}

// u"n y when u does not support irs. We loop over cells, and as we do there is no reason to enable inplacing
// THIS SUPPORTS INPLACING: NOTHING HERE MAY DEREFERENCE jt!!
static DF1(rank1){DECLF;A h=sv->h;I m,*v=AV(h),wr;
 RZ(w);
 wr=AR(w); efr(m,wr,v[0]);
 // We know that the first call is RANKONLY, and we consume any other RANKONLYs in the chain until we get to something else.  The something else becomes the
 // fs/f1 to rank1ex.  Until we can handle multiple fill neighborhoods, we mustn't consume a verb of lower rank
 while(FAV(fs)->flag2&VF2RANKONLY1){
  h=FAV(fs)->h; I hm=AV(h)[0]; efr(hm,m,hm); if(hm<m)break;  // if new rank smaller than old, abort
  m=hm; fs=FAV(fs)->f; f1=FAV(fs)->f1;
 }
 R m<wr?rank1ex(w,fs,m,f1):CALL1(f1,w,fs);
}
// Version for rank 0.  Call rank1ex0, pointing to the u"r so that rank1ex0 gets to look at any razeflags attached to u"r
static DF1(jtrank10atom){ A fs=FAV(self)->f; R (FAV(fs)->f1)(jt,w,fs);}  // will be used only for no-frame executions.  Otherwise will be replaced by the flags loop
static DF1(jtrank10){R jtrank1ex0(jt,w,self,jtrank10atom);}  // pass inplaceability through.


// For the dyads, rank2ex does a quadruply-nested loop over two rank-pairs, which are the n in u"n (stored in h) and the rank of u itself (fetched from u).
// THIS SUPPORTS INPLACING: NOTHING HERE MAY DEREFERENCE jt!!
static DF2(rank2){DECLF;A h=sv->h;I ar,l=AV(h)[1],r=AV(h)[2],wr;
 RZ(a&&w);
 ar=AR(a); efr(l,ar,l);
 wr=AR(w); efr(r,wr,r);
 if(((l-ar)|(r-wr))<0) {I llr=l, lrr=r;  // inner ranks, if any
 // We know that the first call is RANKONLY, and we consume any other RANKONLYs in the chain until we get to something else.  The something else becomes the
 // fs/f1 to rank1ex.  We have to stop if the new ranks will not fit in the two slots allotted to them.
 // This may lead to error until we support multiple fill neighborhoods
  while(FAV(fs)->flag2&VF2RANKONLY2){
   h=FAV(fs)->h; I hlr=AV(h)[1]; I hrr=AV(h)[2]; efr(hlr,llr,hlr); efr(hrr,lrr,hrr);  // fetch ranks of new verb, resolve negative, clamp against old inner rank
   if((hlr^llr)|(hrr^lrr)){  // if there is a new rank to insert...
    if((l^llr)|(r^lrr))break;  // if lower slot full, exit, we can't add a new one
    llr=hlr; lrr=hrr;  // install new inner ranks, where they are new lows
   }
   // either we can ignore the new rank or we can consume it.  In either case pass on to the next one
   fs=FAV(fs)->f; f2=FAV(fs)->f2;   // advance to the new function
  }
// obsolete   I llr=VAV(fs)->lr, lrr=VAV(fs)->rr;  // fetch ranks of verb we are going to call
// obsolete   // if the verb we are calling is another u"n, we can skip coming through here a second time & just go to the f2 for the nested rank
// obsolete   // should move this to before runtime
// obsolete   if(f2==rank2&&!(AT(a)&SPARSE||AT(w)&SPARSE)){fs = VAV(fs)->f; f2=VAV(fs)->f2;}
  R rank2ex(a,w,fs,llr,lrr,l,r,f2);
 }else R CALL2(f2,a,w,fs);  // pass in verb ranks to save a level of rank processing if not infinite.  Preserves inplacing
}
// Version for rank 0.  Call rank1ex0, pointing to the u"r so that rank1ex0 gets to look at any razeflags attached to u"r
static DF2(jtrank20atom){ A fs=FAV(self)->f; R (FAV(fs)->f2)(jt,a,w,fs);}  // will be used only for no-frame executions.  Otherwise will be replaced by the flags loop
static DF2(jtrank20){R jtrank2ex0(jt,a,w,self,jtrank20atom);}  // pass inplaceability through.


// a"w; result is a verb
F2(jtqq){A h,t;AF f1,f2;D*d;I *hv,n,r[3],vf,flag2=0,*v;
 RZ(a&&w);
 // The h value in the function will hold the ranks from w.  Allocate it
 GAT(h,INT,3,1,0); hv=AV(h);  // hv->rank[0]
 if(VERB&AT(w)){
  // verb v.  Extract the ranks into a floating-point list
  GAT(t,FL,3,1,0); d=DAV(t);
  n=r[0]=hv[0]=mr(w); d[0]=n<=-RMAX?-inf:RMAX<=n?inf:n;
  n=r[1]=hv[1]=lr(w); d[1]=n<=-RMAX?-inf:RMAX<=n?inf:n;
  n=r[2]=hv[2]=rr(w); d[2]=n<=-RMAX?-inf:RMAX<=n?inf:n;
  // The floating-list is what we will call the v operand into rank?ex.  It holds the nominal verb ranks which may be negative
  // h is the integer version
  w=t;
 }else{
  // Noun v. Extract and turn into 3 values, stored in h
  n=AN(w);
  ASSERT(1>=AR(w),EVRANK);
  ASSERT(0<n&&n<4,EVLENGTH);
  RZ(t=vib(w)); v=AV(t);
  hv[0]=v[2==n]; r[0]=DR(hv[0]);
  hv[1]=v[3==n]; r[1]=DR(hv[1]);
  hv[2]=v[n-1];  r[2]=DR(hv[2]);
 }
 // r is the actual verb ranks, never negative.

 // Get the action routines and flags to use for the derived verb
 if(NOUN&AT(a)){f1=cons1; f2=cons2; ACIPNO(a);// use the constant routines for nouns; mark the constant non-inplaceable since it may be reused;
  // Mark the noun as non-inplaceable.  If the derived verb is used in another sentence, it must first be
  // assigned to a name, which will protects values inside it.
  ACIPNO(a);
  vf=VASGSAFE;    // the noun can't mess up assignment, and does not support IRS
 }else{
  V* av=FAV(a);   // point to verb info
  // The flags for u indicate its IRS and atomic status.  If atomic (for monads only), ignore the rank, just point to
  // the action routine for the verb.  Otherwise, choose the appropriate rank routine, depending on whether the verb
  // supports IRS.  The IRS verbs may profitably support inplacing, so we enable it for them.
  vf=av->flag&(VASGSAFE|VINPLACEOK1|VINPLACEOK2);  // inherit ASGSAFE from u, and inplacing
  if(av->flag&VISATOMIC1){f1=av->f1;}else{if(av->flag&VIRS1){f1=rank1i;}else{f1=r[0]?rank1:jtrank10; flag2|=VF2RANKONLY1;}}
  if(av->flag&VIRS2){f2=rank2i;}else{f2=(r[1]|r[2])?rank2:jtrank20;flag2|=VF2RANKONLY2;}
  // Test for special cases
  if(av->f2==jtfslashatg && r[1]==1 && r[2]==1){  // f@:g"1 1 where f and g are known atomic
   I isfork=av->id==CFORK;
   if(FAV(FAV(isfork?av->g:av->f)->f)->id==CPLUS && FAV(isfork?av->h:av->g)->id==CSTAR) {
    // +/@:*"1 1 or ([: +/ *)"1 1 .  Use special rank-1 routine.  It supports IRS, but not inplacing (fslashatg didn't inplace either)
    f2=jtsumattymes1; vf |= VIRS2; flag2 &= ~VF2RANKONLY2;  // switch to new routine, which supports IRS
   }
  }
 }

 // Create the derived verb.  The derived verb (u"n) NEVER supports IRS; it inplaces if the action verb u supports inplacing
 R fdef(flag2,CQQ,VERB, f1,f2, a,w,h, vf, r[0],r[1],r[2]);
}
