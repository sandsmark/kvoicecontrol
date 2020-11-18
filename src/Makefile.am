# This file you have to edit. Change the name here
SUBDIRS = doc

# set the include path for X, qt and KDE, and versit library for vCalendar
INCLUDES= $(all_includes)

# this gives better compile-time warnings.
#CXXFLAGS += -pedantic

DISTCLEANFILES = $(BUILTSOURCES)
bin_PROGRAMS = 	kvoicecontrol

kvoicecontrol_LDADD   = \
		$(all_libraries) \
		-lkdeui -lkdecore -lqt -lXext -lX11 \
		$(KDE_RPATH)

# set up some extra paths
APPSDIR = $(kde_appsdir)/Multimedia
DATADIR = $(kde_datadir)/kvoicecontrol

########## FILES

# Which sources should be compiled for korganizer.
kvoicecontrol_SOURCES	= kvoicecontrol.cpp speakermodel.cpp reference.cpp utterance.cpp\
	reference_editor.cpp score.cpp preprocessing.cpp realfftf.c soundcard.cpp\
	buffer.cpp options.cpp optionsdata.cpp docking.cpp train_dialog.cpp

BUILT_SOURCES =	kvoicecontrol.moc speakermodel.moc reference.moc utterance.moc\
	reference_editor.moc score.moc preprocessing.moc soundcard.moc\
	buffer.moc options.moc optionsdata.moc docking.moc train_dialog.moc

# just install datas here. Use install-exec-local for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) kvoicecontrol.kdelnk $(APPSDIR)

	$(mkinstalldirs) $(DATADIR)

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(APPSDIR)/kvoicecontrol.kdelnk

# add a dependency for every moc file to be full portable

# for the alarm daemon
kvoicecontrol.moc: kvoicecontrol.h
	$(MOC) kvoicecontrol.h -o kvoicecontrol.moc

speakermodel.moc: speakermodel.h
	$(MOC) speakermodel.h -o speakermodel.moc

reference.moc: reference.h
	$(MOC) reference.h -o reference.moc

utterance.moc: utterance.h
	$(MOC) utterance.h -o utterance.moc

reference_editor.moc: reference_editor.h
	$(MOC) reference_editor.h -o reference_editor.moc

score.moc: score.h
	$(MOC) score.h -o score.moc

preprocessing.moc: preprocessing.h
	$(MOC) preprocessing.h -o preprocessing.moc

soundcard.moc: soundcard.h
	$(MOC) soundcard.h -o soundcard.moc

buffer.moc: buffer.h
	$(MOC) buffer.h -o buffer.moc

options.moc: options.h
	$(MOC) options.h -o options.moc

optionsdata.moc: optionsdata.h
	$(MOC) optionsdata.h -o optionsdata.moc

docking.moc: docking.h
	$(MOC) docking.h -o docking.moc

train_dialog.moc: train_dialog.h
	$(MOC) train_dialog.h -o train_dialog.moc

kvoicecontrol.cpp: kvoicecontrol.moc
speakermodel.cpp: speakermodel.moc
reference.cpp: reference.moc
utterance.cpp: utterance.moc
reference_editor.cpp: reference_editor.moc
score.cpp: score.moc
preprocessing.cpp: preprocessing.moc
soundcard.cpp: soundcard.moc
buffer.cpp: buffer.moc
options.cpp: options.moc optionsdata.moc
optionsdata.cpp: optionsdata.moc
docking.cpp: docking.moc
train_dialog.cpp: train_dialog.moc


