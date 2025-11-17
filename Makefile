MSG ?= "chore: push without commit message"

push:
	@git add .
	@git commit -m "$(MSG)" || echo "No changes to commit"
	@git push
	@echo "Pushed changes to remote repository."