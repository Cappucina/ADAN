# Homebrew Formula for ADAN
class Adan < Formula
  desc "Modern programming language compiler with native code generation"
  homepage "https://github.com/placeholder/ADAN"
  url "https://github.com/placeholder/ADAN/archive/v0.1.0.tar.gz"
  sha256 "SKIP"
  license "MIT"

  depends_on "rust" => :build
  depends_on "nasm"

  def install
    system "cargo", "build", "--release", "--locked"
    bin.install "target/release/ADAN" => "adan"
    bin.install "build_asm.sh" => "adan-build-asm"

    doc.install "README.md"
    (share/"adan/examples").install Dir["examples/*"]
  end

  test do
    system "#{bin}/adan", "--help"
  end
end
