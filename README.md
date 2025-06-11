# WriteSpace

WriteSpace is a personal, localized, and version-controlled writing environment built on plain text, Markdown, and Git. The goal is to create a standardized and powerful workflow for all writing projects, from blog posts to research papers, freeing them from proprietary software and ensuring they are always accessible and portable.

The main branch exists for templating and

## Core Philosophy

* **Plain Text First**: All core content is in Markdown, a simple and readable format. This ensures longevity and compatibility.
* **Version Control Everything**: Git tracks every change, allowing for easy rollback, branching for different drafts, and a complete history of the work.
* **Automate Conversion**: Scripts handle the transformation from the Markdown source to final formats like HTML, PDF, and LaTeX, eliminating manual copy-pasting and reformatting.
* **One Source, Many Outputs**: A single Markdown document can be rendered beautifully for the web, a formal academic paper, or an ebook, without altering the source content.

## Key Components

* **`templates/`**: A directory of starter templates for different writing projects (blog post, research paper, etc.).
* **`scripts/`**: A collection of conversion scripts, likely powered by a `Makefile` and `pandoc`.
* **`library.bib`**: A central BibTeX file for managing all citations and references.
* **`assets/`**: A global directory for images, diagrams, and other media to be used across projects.

## Dependencies

To use this system, the following tools are required:

* [Git](https://git-scm.com/)
* [Pandoc](https://pandoc.org/): A universal document converter.
* A LaTeX Distribution (e.g., [MiKTeX](https://miktex.org/) for Windows, [MacTeX](https://www.tug.org/mactex/) for macOS, or `texlive` for Linux).
* A good text editor. Don't overthink this.

## Workflow

1.  **Idea**: Create a new file in the `01_ideas/` directory.
2.  **Draft**: When ready, copy a template from `templates/` to `02_drafts/new-project-name/`.
3.  **Write**: Write the content in Markdown. Add images to `assets/` and citations from `library.bib`.
4.  **Build**: Run `make` in the project directory to generate all desired output formats (e.g., `make pdf`, `make html`).
5.  **Commit**: Use `git commit` to save your progress.
6.  **Publish**: Copy the generated HTML to Blogger or submit the generated PDF.

## License

The Unlicense. This is free and unencumbered software released into the public domain.

Primary maintainer: [@suobset](https://github.com/suobset)

## Example Workflows

These examples demonstrate how to use the `writespace` command for common tasks.

### Workflow 1: Blog Post (Plain HTML)

1.  **Create Project**: `writespace new my-first-post --template blog`
2.  **Navigate**: `cd 01_drafts/my-first-post`
3.  **Write**: Edit `my-first-post.md` with your content.
4.  **Build**: `writespace build html-plain`
5.  **Result**: A file named `output/my-first-post.html` is created, ready to be copied to Blogger or another platform.

### Workflow 2: Blog Post (Styled HTML)

1.  **Create Project**: `writespace new my-styled-post --template blog`
2.  **Navigate**: `cd 01_drafts/my-styled-post`
3.  **Write**: Edit `my-styled-post.md` and add images to `03_assets/my-styled-post/`.
4.  **Build**: `writespace build html`
5.  **Result**: `output/my-styled-post.html` is created, linked to `style.css` for viewing in a browser.

### Workflow 3: Academic Paper (IEEE PDF)

1.  **Create Project**: `writespace new my-conference-paper --template blog`
    *   (Note: The template is `blog` for this example as it's a generic markdown starting point. Users can adapt any .md file.)
2.  **Navigate**: `cd 01_drafts/my-conference-paper`
3.  **Add Citations**: Open `../../library.bib` and add your references (e.g., `@article{knuth1984, ...}`).
4.  **Write**: Edit `my-conference-paper.md`. Use Pandoc citation syntax (`[@knuth1984]`).
5.  **Build**: `writespace build pdf-ieee`
6.  **Result**: `output/my-conference-paper.pdf` is created, formatted to IEEE conference standards with a bibliography.
