MSG ?= "chore: push without commit message"

push:
	@-git add .
	@-git commit -m "$(MSG)"
	@-git push