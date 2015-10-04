/* GLOBAL.H - RSAREF types and constants */

/* PROTOTYPES should be set to one if and only if the compiler
 * supports function argument prototyping.
 * The following makes PROTOTYPES default to 0 if it has not already
 * been defined with C compiler flags.
 */
#ifndef PROTOTYPES
#define PROTOTYPES 0
#endif

/*Modified by MMoore http://mikestechspot.blogspot.com
Changed typedefs to be fully compatible w/ Arduino 08/09/2010 */

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long UINT4;

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
 * If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
 * returns an empty list.
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif

