all: subdirs
clean: subdirs
install: subdirs

.PHONY: subdirs

subdirs:
	@$(MAKE) --no-print-directory -C src $(MAKECMDGOALS)
