## Process this file with automake to produce Makefile.in
bin_PROGRAMS = ledset


# sources
ledset_SOURCES = \
	ledset.c



# compile for debugging ?
if DEBUG
    OPT_CFLAGS = -O0 -g
else
    OPT_CFLAGS = -O2
endif

# cflags
ledset_CFLAGS = \
	-Wall \
	-I$(top_srcdir)/include \
	-I../../include \
	$(OPT_CFLAGS) \
	$(niftyprefs_CFLAGS) \
	$(niftylog_CFLAGS) \
	$(babl_CFLAGS) 
	
# ldflags
ledset_LDFLAGS = \
	$(niftyprefs_LIBS) \
	$(niftylog_LIBS) \
	$(babl_LIBS) \
	../../src/libniftyled.la \
	-Wall -no-undefined
