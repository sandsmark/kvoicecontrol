
# add here all files
data_DATA = kvoicecontrol.html

# this is the directory, where all datas are installed
# you have given the datas in data_DATA
datadir = $(kde_htmldir)/en/kvoicecontrol

install-data-local:
	-rm -f $(datadir)/index.html
	$(LN_S) kvoicecontrol.html $(datadir)/index.html

uninstall-local:
	rm $(datadir)/kvoicecontrol.html


