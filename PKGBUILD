# Maintainer: Noah Bieler <noah.bieler[at]gmx[dot]ch>
pkgname=logbook
pkgver=0.1
pkgrel=1
pkgdesc="Logs the date, the current directory and a note."
url="http://www.foo.tld"
arch=('x86_64' 'i686')
license=('GPLv3')
depends=()
optdepends=()
makedepends=()
conflicts=()
replaces=()
backup=()
source=("http://www.server.tld/${pkgname}-${pkgver}.tar.gz"
"foo.desktop")

build() {
    make
    install -d -m755 l "${pkgdir}/usr/bin/"
    install -m755 l "${pkgdir}/usr/bin/l"
}

# vim:set ts=2 sw=2 et:
