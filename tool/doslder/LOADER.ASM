	ifndef	??version
?debug	macro
	endm
publicdll macro	name
	public	name
	endm
$comm	macro	name,dist,size,count
	comm	dist name:BYTE:count*size
	endm
	else
$comm	macro	name,dist,size,count
	comm	dist name[size]:BYTE:count
	endm
	endif
	?debug	V 300h
	?debug	S "loader.c"
	?debug	C E9C0B93944086C6F616465722E63
	?debug	C E94019CA181B433A5C424F524C414E44435C494E434C5544455C73+
	?debug	C 7464696F2E68
	?debug	C E94019CA181B433A5C424F524C414E44435C494E434C5544455C5F+
	?debug	C 646566732E68
	?debug	C E94019CA181C433A5C424F524C414E44435C494E434C5544455C5F+
	?debug	C 6E66696C652E68
	?debug	C E94019CA181B433A5C424F524C414E44435C494E434C5544455C5F+
	?debug	C 6E756C6C2E68
_TEXT	segment byte public 'CODE'
_TEXT	ends
DGROUP	group	_DATA,_BSS
	assume	cs:_TEXT,ds:DGROUP
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
_BSS	ends
_TEXT	segment byte public 'CODE'
   ;	
   ;	int         Load_lenix(FILE * lenix)
   ;	
	assume	cs:_TEXT
_Load_lenix	proc	near
	push	bp
	mov	bp,sp
	mov	dx,word ptr [bp+4]
   ;	
   ;	{
   ;	    lenix = lenix;
   ;	
   ;	
   ;	    return -1;
   ;	
	mov	ax,-1
	jmp	short @1@58
@1@58:
   ;	
   ;	}
   ;	
	pop	bp
	ret	
_Load_lenix	endp
   ;	
   ;	int main(int argc,char ** argv)
   ;	
	assume	cs:_TEXT
_main	proc	near
	push	bp
	mov	bp,sp
	sub	sp,2
	push	si
	push	di
	mov	di,word ptr [bp+6]
   ;	
   ;	{
   ;	    FILE * lenix = (void *)0;
   ;	
	mov	word ptr [bp-2],0
   ;	
   ;	    char * name = "lenix";
   ;	
	mov	si,offset DGROUP:s@
   ;	
   ;	
   ;	    if( argc > 1 )
   ;	
	cmp	word ptr [bp+4],1
	jle	short @2@86
   ;	
   ;	        name = argv[1];
   ;	
	mov	si,word ptr [di+2]
@2@86:
   ;	
   ;	    if( NULL == (lenix = fopen(name,"rb") ) )
   ;	
	mov	ax,offset DGROUP:s@+6
	push	ax
	push	si
	call	near ptr _fopen
	pop	cx
	pop	cx
	mov	word ptr [bp-2],ax
	or	ax,ax
	jne	short @2@170
   ;	
   ;	    {
   ;	        printf("can not found file: %s\n",name);
   ;	
	push	si
	mov	ax,offset DGROUP:s@+9
	push	ax
	call	near ptr _printf
	pop	cx
	pop	cx
   ;	
   ;	        return 1;
   ;	
	mov	ax,1
@2@142:
	jmp	short @2@198
@2@170:
   ;	
   ;	    }
   ;	    Load_lenix(lenix);
   ;	
	push	word ptr [bp-2]
	call	near ptr _Load_lenix
	pop	cx
   ;	
   ;	    Pm_initial();
   ;	
	call	near ptr _Pm_initial
   ;	
   ;	    return 0;
   ;	
	xor	ax,ax
	jmp	short @2@142
@2@198:
   ;	
   ;	}
   ;	
	pop	di
	pop	si
	mov	sp,bp
	pop	bp
	ret	
_main	endp
	?debug	C E9
	?debug	C FA00000000
_TEXT	ends
_DATA	segment word public 'DATA'
s@	label	byte
	db	'lenix'
	db	0
	db	'rb'
	db	0
	db	'can not found file: %s'
	db	10
	db	0
_DATA	ends
_TEXT	segment byte public 'CODE'
_TEXT	ends
	extrn	_Pm_initial:near
	extrn	__setargv__:far
	public	_main
	public	_Load_lenix
	extrn	_printf:near
	extrn	_fopen:near
_s@	equ	s@
	end
