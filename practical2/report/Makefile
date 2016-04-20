LATEX:=pdflatex
LATEXFLAGS:=
DOCUMENT:=p2

.PHONY: all clean
.SECONDARY: $(DOCUMENT).fmt

all: $(DOCUMENT).pdf

%.pdf: %.tex %.fmt .FORCE
	$(LATEX) $(LATEXFLAGS) $(basename $@)
	$(LATEX) $(LATEXFLAGS) $(basename $@)

.PHONY: .FORCE

%.fmt: pre.tex
	$(LATEX) $(LATEXFLAGS) -ini -jobname="$(basename $@)" "&$(LATEX) $<\dump"

clean:
	$(RM) -v $(addprefix $(DOCUMENT).,aux fmt log nav out snm toc vrb)
