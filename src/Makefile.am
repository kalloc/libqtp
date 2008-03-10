## Process this file with automake to produce Makefile.in

COMPATNUM = @LIBTOOL_VERSION_INFO@

INCLUDES = 

EXTRA_DIST = main.l parse.ggo parse.c parse.h qtl_mysql.h qtl_tailq.h

main.c: main.l
	flex -Pqtp -omain.c main.l

parse.c: parse.ggo
	gengetopt -fparse -Fparse -iparse.ggo

bin_PROGRAMS = qtp qtp-make

lib_LTLIBRARIES = libqtp.la

libqtp_la_SOURCES = \
	qcm.c qcm.h qcm_internal.h queue.h \
	idb.c idb.h qsm.c qsm.h md5.c md5.h \
	gensession.c gensession.h \
	garbage.c garbage.h \
	cgix.c cgix.h qtp.h \
	qstr.c qstr.h \
	zdb.c zdb.h

libqtp_la_ldflags = -version-info $(compatnum)

qtp_SOURCES = \
	utils.c utils.h \
	parse.c parse.h \
	qstr_no_lt.c \
	main.c 

qtp_make_SOURCES = \
	qtp-make.c

install-data-hook:
	test -d $(DESTDIR)$(includedir) || mkdir -p $(DESTDIR)$(includedir)
	$(INSTALL_DATA) qcm.h $(DESTDIR)$(includedir)/qcm.h
	$(INSTALL_DATA) qtp.h $(DESTDIR)$(includedir)/qtp.h
	$(INSTALL_DATA) qsm.h $(DESTDIR)$(includedir)/qsm.h
	$(INSTALL_DATA) cgix.h $(DESTDIR)$(includedir)/cgix.h
	$(INSTALL_DATA) qcm_internal.h $(DESTDIR)$(includedir)/qcm_internal.h
	$(INSTALL_DATA) qtl_mysql.h $(DESTDIR)$(includedir)/qtl_mysql.h
	$(INSTALL_DATA) qtl_tailq.h $(DESTDIR)$(includedir)/qtl_tailq.h
