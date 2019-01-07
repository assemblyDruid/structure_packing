// Notes from: http://www.catb.org/esr/structure-packing/

//
//
//
// SCALAR PADDING
//
//
//

// EX1) ASSUMED LAYOUT
// TOTAL BYTES: 9 or 13 bytes
char* p; // 4 bytes on x86, 8 bytes on x64
char  c; // always 1 byte
int   x; // usually (compiler dependent) 4 bytes (at least 16 bits IEEE)


// NOTE:
// Pointer alignment is the most strict. It will always line up on a factor of 4 on 32 bit systems and
// a factor of 8 on 64 bit systems. *Alignmnet* refers to where a member *starts*. *Padding* refers to how
// many "slop" bytes are used to enforce alignment.

// EX1) ACTUAL LAYOUT
// TOTAL BYTES: 12 or 16 bytes
char*    p; // 4 or 8 bytes.
char     c; // 1 byte
char[3]  IMPLICIT_PADDING; // 3 bytes of slop
int      x; // 4 bytes


// EX2) ASSUMED LAYOUT (x86)
// TOTAL BYTES: 7
char* p; // 4
char  c; // 1 byte
short x; // 2 bytes


// EX2) ACTUAL LAYOUT (x86)
// TOTAL BYTES: 8
char*   p; // 4
char    c; // 1 byte
char[1] IMPLICIT_PADDING; // 1 byte of slop
short   x; // 2 bytes


// EX3) ASSUMED LAYOUT (x64)
// TOTAL BYTES: 17
char* p; // 8 bytes
char  c; // 1 byte
long  x; // 8 bytes


// EX3) ACTUAL LAYOUT (x64)
// TOTAL BYYES: 24
char*   p; // 8 bytes
char    c; // 1 byte
char[7] IMPLICIT_ADDING; // 7 bytes of slop
long    x; // 8 bytes


// EX4) ASSUMED LAYOUT (non-pointer first member, x86)
// TOTAL BYTES: 9
char  c; // 1 byte
char* p; // 4 bytes
int   x; // 4 bytes


// EX4) ACTUAL LAYOUT (non-pointer first member, x86)
// TOTAL BYTES: Any value between 9 and 12 (most likely 12)
char    c;                 // 1 byte
char[M] IMPLICIT_PADDING_1 // Any value between 0 and 3 (most likely 3)
char*   p;                 // 4 bytes
char[N] IMPLICIT_PADDING_2 // 0 bytes | In this case we have two 4 byte values consecutively
int     x;                 // 4 bytes

// NOTE: The value of M is less predictable. If the compiler happened to map c to the last byte
// of a machine word, the next byte (the first of p) would be the first byte of the next one
// and properly pointer-aligned. M would be zero. It is more likely that c will be mapped to
// the first byte of a machine word. In that case M will be whatever padding is needed to
// ensure that p has pointer alignment - 3 on a 32-bit machine, 7 on a 64-bit machine.
// Intermediate cases are possible. M can be anything from 0 to 7 (0 to 3 on 32-bit) because a
// char can start on any byte boundary in a machine word.
//
// To reduce space, switch the positions of `p` and `c`

//
//
//
// STRUCTURE ALIGNMENT AND PADDING
//
//
//

// NOTE: In general, structs apply the alignment of the widest/largest *scalar*  member.
// In C and C-like languages, the address of the struct is the address of the first member.
// C++ may break this rule (having leading slop/padding space).

// EX5) ASSUMED LAYOUT (x64)
// TOTAL BYTES: 9
struct foo
{
    char* p; // 8 bytes
    char  c; // 1 byte
    long  x; // 8 bytes <-- sets struct width (largest)
};

// EX5) ACTUAL LAYOUT (x64)
// TOTAL BYTES: 24
struct foo
{
    char*   p;                // 8 bytes
    char    c;                // 1 byte
    char[7] IMPLICIT_PADDING; // 7 bytes of slop
    long    x;                // 8 bytes
};

// This is unsuprising considering what we've seen for scalar alignment. What is suprising is
// that if we had put `c` first, we do not see the same behavior. Instead of variable [M] and [N]
// padding on either side of the pointer, c *must start* on an 8 bit (widest) alignment.

// EX6) ASSUMED LAYOUT (x64)
// TOTAL BYTES: 9
struct foo
{
    char  c; // 1 byte
    char* p; // 8 bytes
    long  x; // 8 bytes <-- sets struct width (largest)
};

// EX6) ACTUAL LAYOUT (x64)
// TOTAL BYTES: 16
struct foo
{
    char    c;                // 1 byte
    char[7] IMPLICIT_PADDING; // 7 bytes of slop
    char*   p;                // 8 bytes
    long    x;                // 8 bytes
};


// Note: The compiler will behave as though a structure has trailing padding out to its stride address. The
// stride address is the memory address after the struct that coincides with the same *alignment* of the struct.

// EX7) ASSUMED LAYOUT (x64)
// TOTAL BYTES: 9
struct fooStride
{
    char* p; // 8 bytes <-- sets struct width (largest)
    char  c; // 1 byte
};

// EX7) ACTUAL LAYOUT (x64)
// TOTAL BYTES: 16
struct fooStride
{
    char*   p;                // 8 bytes
    char    c;                // 1 byte
    char[7] IMPLICIT_PADDING; // 7 bytes of slop
};

struct fooStride singleton; // 16 bytes
struct fooStride quad[4];   // 64 bytes


// EX8) ASSUMED LAYOUT (x64)
// TOTAL BYTES: 3
struct fooStride
{
    short s; // 2 bytes <-- sets struct width (largest)
    char  c; // 1 byte
};


// EX8) ACTUAL LAYOUT (x64)
// TOTAL BYTES: 4
struct fooStride
{
    short   s;               // 2 bytes
    char    c;               // 1 byte
    char[1] IMPLICIT_PADDING // 1 byte of slop
};


// EX9) ASSUMED LAYOUT
// TOTAL BYTES: 9
struct foo
{
    char c;           // 1 byte
    struct inner_foo
    {
        char* p       // 8 bytes <-- sets struct width (largest)
        short x       // 2 bytes
    };
};

// EX9) ACTUAL LAYOUT
// TOTAL BYTES: 24
struct foo
{
    char c;                      // 1 byte
    char[7] IMPLICIT_PADDING;    // 7 bytes of slop
    struct inner_foo
    {
        char* p                  // 8 bytes
        short x                  // 2 bytes
        char[6] IMPLICIT_PADDING // 6 bytes of slop
    };
};

//
//
//
// BITFIELDS
//
//
//

// Bitfields allow you to declare structure members on less than character width, down to a single bit.

// EX9) ASSUMED LAYOUT
// TOTAL BYTES: 2.5
struct bitfield
{
    short s;        // 2 bytes
    char  c;        // 1 byte
    int   flip:1;   // 1 bit <-- sets struct width (largest)
    int   nybble:4; // 4 bits
    int   septet:7; // 7 bits
};

// EX9) ACTUAL LAYOUT (1 of 2)
// TOTAL BYTES: 4 bytes
struct bitfield
{
    short s;                  // 2 bytes
    char  c;                  // 1 byte
    int   flip:1;             // 1 bit
    int   nybble:4;           // 4 bits
    int   IMPLICIT_PADDING:3  // 3 bits of slop
    int   septet:7;           // 7 bits
    int   IMPLICIT_PADDING:25 // 25 bit of slop (struct stride set by `int` @ 32 bits)
};

// EX9) ACTUAL LAYOUT (2 of 2)
// TOTAL BYTES: 4 bytes
struct bitfield
{
    short s;                 // 2 bytes
    char  c;                 // 1 byte
    int   IMPLICIT_PADDING:3 // 3 bits of slop
    int   flip:1;            // 1 bit
    int   nybble:4;          // 4 bits
    int   septet:7;          // 7 bits
    int   IMPLICIT_PADDING:1 // 1 bit of slop
};

// Essentially, the padding could occur before or after the payload bits. It is also worth noting
// that the padding bits do not have to be initialized (to anything).

// EX10) This struct takes up a single 32 bit word (C99)
struct word32
{
    int bigField:31;
    int smallField:1;
};

// EX11) This struct takes up two 32 bit words (C99)
struct word64
{
    int bigField:31;
    int smallField:1;
    int bigField_2:31;
    int smallField_2: 1;
};

// EX12) This struct takes up three 32 bit words (C99) with *31 slop bits*
struct word96
{
    int bigField:31;
    int smallField:1;
    int bigField_2:32;
    int smallField_2:1;
    int smallField_3:1; // <-- Offending line. 31 bits of slop follow to enforce boundaries.
};


//
//
//
// Structure Reordering
//
//
//

// Slop only occurs in two situations:
//    1: Where small storage follows large storage with more strict alignment rules.
//    2: A struct naturally ends beofre the stride address.
//
// The easiest way to eliminate slop is to order members by decreasing alignment.
// Pointers first, then 4 byte types, then 2 bytes types, then 1.
