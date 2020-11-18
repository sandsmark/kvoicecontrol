# This file you have to edit. Change the name here
SUBDIRS = src

# not a GNU package. You can remove this line, if 
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = no-dependencies

VERSION=$(shell awk '/^Version:/ { print $$2 }' kvoicecontrol.lsm)
CVSTAG = v$(subst .,-,$(VERSION))

acinclude.m4: acinclude.m4.in libtool.m4.in
	cat acinclude.m4.in libtool.m4.in > acinclude.m4

distclean-local:
	-find . -name .deps | xargs rm -rf
