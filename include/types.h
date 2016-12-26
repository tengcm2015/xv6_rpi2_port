#ifndef TYPES_H
#define TYPES_H

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef unsigned long long uint64;

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

typedef uint pde_t;
typedef uint pte_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

/* trap vectors layout at virtual
address HVECTORS (and KZERO(0x80000000), doubled mapped).*/
typedef struct Vpage0 {
        void    (*vectors[8])(void);
        uint     vtable[8];
} Vpage0;

typedef struct framebufdescription {
	uint width; //width
	uint height; // height
	uint v_width; // virtual width
	uint v_height; // virtual height
	uint pitch; // GPU pitch
	uint depth; // bit depth
	uint x;
	uint y;
	uint fbp; //GPU framebuffer pointer
	uint fbs; // GPU framebuffer size
} FBI;

#endif
