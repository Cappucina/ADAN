#!/bin/bash

git add .
git commit -m """
${1:-"Co-commit via Live Share"}

${2:-""}

Co-authored-by: Hanna Skairipa <HANNA@HANNASKAIRIPA.COM>
Co-authored-by: Kauht <89326272+KAUHT@USERS.NOREPLY.GITHUB.COM>
Co-authored-by: Lily <TRANSICLE@DUCK.COM>
"""

git push
