

LIBS = $(top_builddir)/src/libplanescape_impl.la -lm  $(top_builddir)/libdreamland/libdreamland.la -lcrypt -lz


data_DATA = $(lib_LTLIBRARIES:.la=.xml)

libdir=$(libexecdir)/plugins
datadir=$(libexecdir)/plugins

.la.xml:
	@echo "Generating $@ from $<"
	@echo '<?xml version="1.0" encoding="KOI8-R"?>' > $@
	@echo '<so>' >> $@
	@echo '    <depends>' >> $@
	@. $<; for i in $$dependency_libs; do \
	    case $$i in \
		*/libplanescape_impl.la) ;;\
		*/libdreamland.la) ;;\
		*/libstdc++.la) ;;\
		*.la) basename $$i | sed -e 's!^lib!        <node>!' \
					 -e 's!\.la$$!</node>!' >> $@ ;; \
	    esac; \
	done
	@echo '    </depends>' >> $@
	@echo '</so>' >> $@

