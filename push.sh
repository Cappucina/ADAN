find samples -type f ! -name "*.adn" -delete

git add .
git commit -m "upd: $(date +%Y-%m-%d-%H-%M)"
git push --force