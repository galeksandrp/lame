/*
 *	lame utility library include file
 *
 *	Copyright (c) 1999 Albert L Faber
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef LAME_UTIL_H
#define LAME_UTIL_H

/***********************************************************************
*
*  Global Include Files
*
***********************************************************************/
#include "machine.h"
#include "encoder.h"
#include "lame.h"
#include "lame-analysis.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
*
*  Global Type Definitions
*
***********************************************************************/
/* bitstream handling structure */
typedef struct {
    int totbyte;        /* byte counter of bit stream */
    int bitidx;         /* pointer to top byte in buffer */
    char buf[LAME_MAXMP3BUFFER];         /* bit stream buffer */
    /* mpeg1: buffer=511 bytes  smallest frame: 96-38(sideinfo)=58
     * max number of frames in reservoir:  8 
     * mpeg2: buffer=255 bytes.  smallest frame: 24-23bytes=1
     * with VBR, if you are encoding all silence, it is possible to
     * have 8kbs/24khz frames with 1byte of data each, which means we need
     * to buffer up to 255 headers! */
    /* also, max_header_buf has to be a power of two */
#define MAX_HEADER_BUF 256
#define MAX_HEADER_LEN 40 /* max size of header is 38 */
    struct {
	int write_timing;
	char buf[MAX_HEADER_LEN];
    } header[MAX_HEADER_BUF];
    int h_ptr;
    int w_ptr;
} bit_stream_t;

/* max scalefactor band, max(SBMAX_l, SBMAX_s*3, (SBMAX_s-3)*3+8) */
#define SFBMAX (SBMAX_s*3)

/* scalefactor band start/end positions */
typedef struct 
{
    int l[1+SBMAX_l];
    int s[1+SBMAX_s];
} scalefac_struct;


typedef struct {
    FLOAT	l[SBMAX_l];
    FLOAT	s[SBMAX_s][3];
} III_psy;

typedef struct {
    III_psy thm;
    III_psy en;
    FLOAT	pe;
} III_psy_ratio;

typedef struct {
    FLOAT xr[576];
    int l3_enc[576];
    int scalefac[SFBMAX];
    FLOAT maxXR[SFBMAX];
    FLOAT ATHadjust;

    int part2_length;
    int part2_3_length;
    int big_values;
    int count1;
    int global_gain;
    int scalefac_compress;
    int block_type;
    int mixed_block_flag;
    int table_select[3];
    int subblock_gain[3+1];
    int region0_count;
    int region1_count;
    int preflag;
    int scalefac_scale;
    int count1table_select;

    int sfb_lmax;
    int sfb_smin;
    int psy_lmax;
    int sfbmax;
    int psymax;
    int sfbdivide;
    int xrNumMax;
    int width[SFBMAX];
    int window[SFBMAX];
    int count1bits;
    /* added for LSF */
    int slen[4];
    int dummy_for_padding[3];
} gr_info;

/* Layer III side information. */
typedef struct {
    gr_info tt[2][2];
    int main_data_begin;  /* in bytes */
    int ResvSize; /* in bits */
    int ResvMax;  /* in bits */
    int maxmp3buf; /* in bytes */
    int sideinfo_len;
    int scfsi[2][4];
} III_side_info_t;

typedef struct 
{
    int sum;    /* what we have seen so far */
    int seen;   /* how many frames we have seen in this chunk */
    int want;   /* how many frames we want to collect into one chunk */
    int pos;    /* actual position in our bag */
    int size;   /* size of our bag */
    int *bag;   /* pointer to our bag */
} VBR_seek_info_t;


/* Guest structure, only temporarly here */

#define MAX_CHANNELS  2
#ifdef HAVE_MPGLIB
#include "../mpglib/interface.h"
#endif

/********************************************************************
 * internal variables NOT set by calling program, and should not be *
 * modified by the calling program                                  *
 ********************************************************************/
struct lame_internal_flags {
    /* most used variables */
    FLOAT xrwork[2][576];  /* xr^(3/4) and fabs(xr) */

    /* side information */
    III_side_info_t l3_side;

    /* variables for subband filter and MDCT */
    FLOAT sb_sample[2][3][18][SBLIMIT];
    FLOAT amp_filter[SBLIMIT];
    int xrNumMax_longblock;

/*  
 * Some remarks to the Class_ID field:
 * The Class ID is an Identifier for a pointer to this struct.
 * It is very unlikely that a pointer to lame_internal_flags has the same 32 bits
 * in it's structure (large and other special properties, for instance prime).
 *
 * To test that the structure is right and initialized, use:
 *   if ( gfc -> Class_ID == LAME_ID ) ...
 * Other remark:
 *   If you set a flag to 0 for uninit data and 1 for init data, the right test
 *   should be "if (flag == 1)" and NOT "if (flag)". Unintended modification
 *   of this element will be otherwise misinterpreted as an init.
 */

#ifndef  MFSIZE
# define MFSIZE  ( 3*1152 + ENCDELAY - MDCTDELAY )
#endif
    sample_t     mfbuf [2] [MFSIZE];

#  define  LAME_ID   0xFFF88E3B
    unsigned long Class_ID;
    int alignment;

    int lame_encode_frame_init;
    int iteration_init_init;
    int fill_buffer_resample_init;

    int padding;        /* padding for the current frame? */
    int mode_gr;        /* granules per frame */
    int channels_in;	/* number of channels in the input data stream (PCM or decoded PCM) */
    int channels_out;   /* number of channels in the output data stream (not used for decoding) */
    FLOAT samplefreq_in;
    FLOAT samplefreq_out;
    FLOAT resample_ratio;      /* input_samp_rate/output_samp_rate */

    int mf_samples_to_encode;
    int mf_size;
    int VBR_min_bitrate;            /* min bitrate index */
    int VBR_max_bitrate;            /* max bitrate index */
    int bitrate_index;
    int samplerate_index;
    int mode_ext;


    /* lowpass and highpass filter control */
    FLOAT lowpass1,lowpass2;   /* normalized frequency bounds of passband */
    FLOAT highpass1,highpass2; /* normalized frequency bounds of passband */

    FLOAT narrowStereo;        /* stereo image narrowen factor */
    FLOAT reduce_side;         /* side channel PE reduce factor */

    int filter_type;          /* 0=polyphase filter, 1= FIR filter 2=MDCT filter(bad)*/
    int use_scalefac_scale;   /* 0 = not use  1 = use */
    int use_subblock_gain;    /* 0 = not use  1 = use */
    int noise_shaping_amp;    /* 0 = ISO model: amplify all distorted bands
				 1 = amplify within 50% of max (on db scale)
				 2 = amplify only most distorted band
				 3 = half amplify only most distorted band
				 4 = after noise shaping, try to reduce noise
				     by decreasing scalefactor
				 5 = reduce "total noise" if possible
			      */
    int substep_shaping;  /* 0 = no substep
			     1 = use substep only long
			     2 = use substep only short
			     3 = use substep all block type.
			  */
    int psymodel;         /* 2 = use psychomodel and noise shaping.
			     1 = use psychomodel but no noise shaping.
			     0 = don't use psychomodel
			  */
    int use_best_huffman;   /* 0 = no.  1=outside loop  2=inside loop(slow) */

    /* used for padding */
    int frac_SpF;
    int slot_lag;

    /* intensity stereo threshold */
    FLOAT istereo_ratio;

    /* psymodel constants */
    FLOAT *s3_ss;
    FLOAT *s3_ll;

    FLOAT mld_l[SBMAX_l],mld_s[SBMAX_s];
    int	bo_l[SBMAX_l], bo_s[SBMAX_s], bo_l2s[SBMAX_s];
    int	npart_l,npart_s;

    int	s3ind[CBANDS][2];
    int	s3ind_s[CBANDS][2];

    int	endlines_s[CBANDS];
    int	numlines_l[CBANDS];
    FLOAT rnumlines_l[CBANDS];
    FLOAT rnumlines_ls[CBANDS];
    FLOAT masking_lower;
    FLOAT masking_lower_short;
    FLOAT interChRatio;

    /* psymodel work, (for next frame data) */
    III_psy_ratio masking_next[2][MAX_CHANNELS*2];
    int blocktype_next[2][MAX_CHANNELS*2]; /* for block type */
    int mode_ext_next;
    int is_start_sfb_l_next[2];
    int is_start_sfb_s_next[2];
    int is_start_sfb_l[2];
    int is_start_sfb_s[2];

    struct {
	/* short block tuning */
	FLOAT	subbk_ene[MAX_CHANNELS*2][12];
	FLOAT	attackthre;
	FLOAT	decay;
	int	switching_band;

	/* adjustment of Mid/Side maskings from LR maskings */
	FLOAT msfix;
	int tune;
    } nsPsy;

    /**
     *  ATH related stuff, if something new ATH related has to be added,
     *  please plugg it here into the ATH_t struct
     */
    struct {
	FLOAT l[SBMAX_l];     /* ATH for sfbs in long blocks */
	FLOAT s[SBMAX_s];     /* ATH for sfbs in short blocks */
	FLOAT cb[CBANDS];     /* ATH for convolution bands */
	FLOAT l_avg[SBMAX_l];
	FLOAT s_avg[SBMAX_s];

	/* factor for tuning the (sample power) point below which adaptive
	 * threshold of hearing adjustment occurs */
	FLOAT aa_decay;
	FLOAT adjust[MAX_CHANNELS*2]; /* ATH lowering factor based on peak volume */
	FLOAT eql_w[CBANDS];  /* equal loudness weights (based on ATH) */
    } ATH;

    /* quantization */
    int OldValue[2];
    int CurrentStep[2];
    scalefac_struct scalefac_band;

    char bv_scf[576];
    int pseudohalf[SFBMAX];

    /* default cutoff(the max sfb where we should do noise-shaping) */
    int cutoff_sfb_l;
    int cutoff_sfb_s;

    /* variables for bitstream.c */
    bit_stream_t bs;

    /* optional ID3 tags, used in id3tag.c  */
    struct id3tag_spec {
	/* private data members */
	int flags;
	const char *title;
	const char *artist;
	const char *album;
	int year;
	const char *comment;
	int track;
	int totaltrack;
	int genre;
    }  tag_spec;
    uint16_t nMusicCRC;

    unsigned int crcvalue;
    VBR_seek_info_t VBR_seek_table; /* used for Xing VBR header */

    int nogap_total;
    int nogap_current;  

    struct {
	void (*msgf)  (const char *format, va_list ap);
	void (*debugf)(const char *format, va_list ap);
	void (*errorf)(const char *format, va_list ap);
    } report;

  /* variables used by util.c */
  /* BPC = maximum number of filter convolution windows to precompute */
#define BPC 320
    sample_t *inbuf_old [2];
    sample_t *blackfilt [2*BPC+1];
    FLOAT itime[2];

    int (*scale_bitcounter)(gr_info * const gi);
#if HAVE_NASM
    /* functions to replace with CPU feature optimized one in takehiro.c */
    int (*choose_table)(const int *ix, const int *end, int *s);
    void (*fft_fht)(FLOAT *, int);

    /* CPU features */
    struct {
	unsigned int  MMX       : 1; /* Pentium MMX, Pentium II...IV, K6, K6-2,
					K6-III, Athlon */
	unsigned int  AMD_3DNow : 1; /* K6-2, K6-III, Athlon, K8 */
	unsigned int  AMD_E3DNow: 1; /* Athlon, K8               */
	unsigned int  SSE      : 1; /* Pentium III, Pentium 4    */
	unsigned int  SSE2     : 1; /* Pentium 4, K8             */
    } CPU_features;
#endif

#ifdef BRHIST
    /* simple statistics */
    int bitrate_stereoMode_Hist[16][4];
    int	bitrate_blockType_Hist [16][4+1];/*norm/start/stop/short/mixed(short)*/
#endif
#ifndef NOANALYSIS
    /* used by the frame analyzer */
    plotting_data *pinfo;
#endif


    /* input description */
    unsigned long num_samples; /* number of samples. default=2^32-1 */
    int num_channels;          /* input number of channels. default=2 */
    int in_samplerate;         /* input_samp_rate in Hz. default=44.1 kHz */
    int out_samplerate;        /* output_samp_rate.
				  default: LAME picks best value 
				  at least not used for MP3 decoding:
				  Remember 44.1 kHz MP3s and AC97 */
    float scale;               /* scale input by this amount before encoding
				  at least not used for MP3 decoding */
    float scale_left;          /* scale input of channel 0 (left) by this
				  amount before encoding */
    float scale_right;         /* scale input of channel 1 (right) by this
				  amount before encoding */

    /* general control params */
    int bWriteVbrTag;          /* add Xing VBR tag? */

    int quality;               /* quality setting 0=best, 9=worst  default=5 */
    MPEG_mode mode;            /* see enum in lame.h
				  default = LAME picks best value */
    int force_ms;              /* force M/S mode. */
    int use_istereo;           /* use intensity stereo */
    int free_format;           /* use free format? default=0 */

    /*
     * set either mean_bitrate_kbps>0  or compression_ratio>0,
     * LAME will compute the value of the variable not set.
     * Default is compression_ratio = 11.025
     */
    int mean_bitrate_kbps;      /* bitrate */
    float compression_ratio;    /* sizeof(wav file)/sizeof(mp3 file) */

    /* frame params */
    int copyright;              /* mark as copyright. default=0           */
    int original;               /* mark as original. default=1            */
    int error_protection;       /* use 2 bytes per frame for a CRC
				   checksum. default=0                    */
    int extension;              /* the MP3 'private extension' bit.
				   Meaningless                            */
    int strict_ISO;             /* enforce ISO spec as much as possible   */
    int forbid_diff_type;	/* forbid different block type */

    /* quantization/noise shaping */
    int disable_reservoir;      /* use bit reservoir? */

    /* VBR control */
    vbr_mode VBR;
    int VBR_q;
    int VBR_min_bitrate_kbps;
    int VBR_max_bitrate_kbps;

    /* resampling and filtering */
    int lowpassfreq;                /* freq in Hz. 0=lame choses.
				       -1=no filter                          */
    int highpassfreq;               /* freq in Hz. 0=lame choses.
				       -1=no filter                          */
    int lowpasswidth;               /* freq width of filter, in Hz
				       (default=15%)                         */
    int highpasswidth;              /* freq width of filter, in Hz
				       (default=15%)                         */
    /*
     * psycho acoustics and other arguments which you should not change 
     * unless you know what you are doing
     */
    int ATHonly;                    /* only use ATH                         */
    int ATHshort;                   /* only use ATH for short blocks        */
    int noATH;                      /* disable ATH                          */
    float ATHcurve;                 /* change ATH formula 4 shape           */
    float ATHlower;                 /* lower ATH by this many db            */
    int mixed_blocks;
    int emphasis;                   /* Input PCM is emphased PCM (for
				       instance from one of the rarely
				       emphased CDs), it is STRONGLY not
				       recommended to use this, because
				       psycho does not take it into account,
				       and last but not least many decoders
				       don't care about these bits          */

    /************************************************************************/
    /* internal variables, do not set...                                    */
    /* provided because they may be of use to calling application           */
    /************************************************************************/

    int version;                    /* 0=MPEG-2/2.5  1=MPEG-1               */
    int encoder_delay;
    int encoder_padding;  /* number of samples of padding appended to input */
    int framesize;                  
    int frameNum;                   /* number of frames encoded             */

    /* VBR tags. */
    int TotalFrameSize;
    int nVbrNumFrames;

    struct {
	int mmx;
	int amd3dnow;
	int sse;
    } asm_optimizations;
#ifdef HAVE_MPGLIB
    PMPSTR pmp;
    PMPSTR pmp_replaygain;
#endif
};





/***********************************************************************
*
*  Global Function Prototype Declarations
*
***********************************************************************/
int	BitrateIndex(int, int);
void	disable_FPE(void);

int	lame_encode_buffer_sample_t(
    lame_t gfc,
    sample_t buffer_lr[],
    int nsamples,
    unsigned char *mp3buf,
    const int mp3buf_size
    );
/* same as lame_decode1 (look in lame.h), but returns 
   unclipped raw floating-point samples. It is declared
   here, not in lame.h, because it returns LAME's 
   internal type sample_t. No more than 1152 samples 
   per channel are allowed. */
int lame_decode1_unclipped(
    lame_t,
    unsigned char*  mp3buf,
    int             len,
    sample_t        pcm_l[],
    sample_t        pcm_r[] );



#ifdef HAVE_NASM
extern int  has_MMX   ( void );
extern int  has_3DNow ( void );
extern int  has_E3DNow( void );
extern int  has_SSE   ( void );
extern int  has_SSE2  ( void );
#else
# define has_MMX()	0
# define has_3DNow()	0
# define has_E3DNow()	0
# define has_SSE()	0
# define has_SSE2()	0
#endif

/***********************************************************************
*
*  Macros about Message Printing and Exit
*
***********************************************************************/
extern void lame_errorf(lame_t gfc, const char *, ...);
extern void lame_debugf(lame_t gfc, const char *, ...);
extern void lame_msgf  (lame_t gfc, const char *, ...);
#define DEBUGF  lame_debugf
#define ERRORF	lame_errorf
#define MSGF	lame_msgf


#ifdef __cplusplus
}
#endif

#endif /* LAME_UTIL_H */
