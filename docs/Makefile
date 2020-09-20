SOURCES=index.txt faq.txt profile.txt library.txt filter.txt filters.txt
PAGES=$(subst .txt,.html,$(SOURCES))

all: $(PAGES)

$(PAGES): %.html: %.txt
	asciidoc --conf-file asciidoc.conf $<

clean:
	rm -f $(PAGES)
