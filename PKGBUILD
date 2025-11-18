# Maintainer: Your Name <your.email@example.com>
pkgname=adan
pkgver=0.1.0
pkgrel=1
pkgdesc="ADAN - A modern programming language compiler with native code generation"
arch=('x86_64')
url="https://github.com/Cappucina/ADAN"
license=('MIT')
depends=('gcc' 'nasm')
makedepends=('rust' 'cargo')
source=("$pkgname-$pkgver.tar.gz::https://github.com/Cappucina/ADAN/archive/v$pkgver.tar.gz")
sha256sums=('SKIP')

build() {
    cd "$pkgname-$pkgver"
    cargo build --release --locked
}

check() {
    cd "$pkgname-$pkgver"
    cargo test --release
}

package() {
    cd "$pkgname-$pkgver"

    # Install binary
    install -Dm755 "target/release/ADAN" "$pkgdir/usr/bin/adan"
    install -Dm755 "build_asm.sh" "$pkgdir/usr/bin/adan-build-asm"

    # Install documentation
    install -Dm644 README.md "$pkgdir/usr/share/doc/$pkgname/README.md"

    # Install examples
    mkdir -p "$pkgdir/usr/share/$pkgname/examples"
    cp -r examples/* "$pkgdir/usr/share/$pkgname/examples/" 2>/dev/null || true

    # Install license if exists
    if [ -f LICENSE ]; then
        install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
    fi
}
