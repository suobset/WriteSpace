# ==============================================================================
#  WriteSpace Master Makefile
#  Run from within a project directory (e.g., 01_drafts/my-project/)
# ==============================================================================

# Default command is 'help'
.DEFAULT_GOAL := help

# --- Project Variables (Dynamically set) ---
# Sets PROJECT_DIR to the name of the current directory (e.g., 'my-project')
PROJECT_DIR := $(notdir $(CURDIR))

# Finds the first .md file in the current directory. Errors if none is found.
SRC_MD := $(firstword $(wildcard *.md))
ifeq ($(SRC_MD),)
    $(error No Markdown file (.md) found in this directory. Please ensure your main project file (e.g., $(PROJECT_DIR).md) has a .md extension.)
endif

# --- Global Paths (Relative to the project directory from which make is run) ---
# ROOT_DIR points to the repository root (two levels up from a typical project dir like 01_drafts/project_name)
ROOT_DIR := ../..
TEMPLATES_DIR := $(ROOT_DIR)/02_templates
# ASSETS_DIR points to a project-specific subdirectory within 03_assets
# e.g., if in 01_drafts/my-paper, assets are in 03_assets/my-paper/
ASSETS_DIR := $(ROOT_DIR)/03_assets/$(PROJECT_DIR)
BIB_FILE := $(ROOT_DIR)/library.bib

# --- Output Configuration ---
OUTPUT_DIR := ./output
HTML_OUT := $(OUTPUT_DIR)/$(PROJECT_DIR).html
PDF_OUT := $(OUTPUT_DIR)/$(PROJECT_DIR).pdf
TEX_OUT := $(OUTPUT_DIR)/$(PROJECT_DIR).tex # For the 'tex' target

# --- Pandoc Base Command ---
# This is the core command used by most build recipes.
# It ensures the output directory exists, sets the source markdown file,
# and configures resource paths for assets (project-specific and current dir),
# enables citation processing, and specifies the bibliography file.
PANDOC_BASE = @mkdir -p $(OUTPUT_DIR) && pandoc "$(SRC_MD)" --resource-path=. --resource-path=$(ASSETS_DIR) --citeproc --bibliography=$(BIB_FILE)

# --- Recipes ---

# Declare phony targets to prevent conflicts with files of the same name
# and to ensure they run even if files with those names exist.
.PHONY: all html html-plain pdf pdf-ieee pdf-mla tex clean help

help: ## Show this help message.
	@awk 'BEGIN {FS = ":.*?## "; printf "Usage: make [target]\n\nTargets:\n"} /^[a-zA-Z0-9_-]+:.*?## / {printf "  \033[36m%-18s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

all: html pdf ## Build all standard formats (HTML and PDF).

html: html-styled ## Build styled HTML output (alias for html-styled for clarity).

html-styled: ## Build styled HTML output using style.css.
	@echo "--> Building styled HTML for $(PROJECT_DIR) -> $(HTML_OUT)"
	$(PANDOC_BASE) --standalone --toc --css=$(TEMPLATES_DIR)/style.css -o "$(HTML_OUT)"

html-plain: ## Build plain HTML output with no styling.
	@echo "--> Building plain HTML for $(PROJECT_DIR) -> $(HTML_OUT)"
	$(PANDOC_BASE) --standalone --toc -o "$(HTML_OUT)"

pdf: pdf-paper ## Build PDF using the 'paper' LaTeX template (alias for pdf-paper).

pdf-paper: ## Build PDF using the 'paper' LaTeX template.
	@echo "--> Building PDF (paper template) for $(PROJECT_DIR) -> $(PDF_OUT)"
	$(PANDOC_BASE) --pdf-engine=pdflatex --template=$(TEMPLATES_DIR)/paper.tex -o "$(PDF_OUT)"

pdf-ieee: ## Build PDF using the IEEEtran LaTeX template.
	@echo "--> Building PDF (IEEEtran template) for $(PROJECT_DIR) -> $(PDF_OUT)"
	$(PANDOC_BASE) --pdf-engine=pdflatex --template=$(TEMPLATES_DIR)/ieee.tex -o "$(PDF_OUT)"

pdf-mla: ## Build PDF using the MLA LaTeX template.
	@echo "--> Building PDF (MLA template) for $(PROJECT_DIR) -> $(PDF_OUT)"
	$(PANDOC_BASE) --pdf-engine=pdflatex --template=$(TEMPLATES_DIR)/mla.tex -o "$(PDF_OUT)"

tex: ## Generate a standalone .tex file for debugging (using paper template).
	@echo "--> Generating LaTeX source for $(PROJECT_DIR) -> $(TEX_OUT)"
	$(PANDOC_BASE) --template=$(TEMPLATES_DIR)/paper.tex -o "$(TEX_OUT)"

clean: ## Remove all generated files from the output directory.
	@echo "--> Cleaning output directory: $(OUTPUT_DIR)"
	@rm -rf $(OUTPUT_DIR)
# Ensure there's a newline at the end of the file
