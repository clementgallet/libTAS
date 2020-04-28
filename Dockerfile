FROM archlinux/base
# libtas
  # dependencies
    # main
      RUN pacman -Sy --noconfirm git
      RUN pacman -Sy --noconfirm base-devel
      RUN pacman -Sy --noconfirm automake
      RUN pacman -Sy --noconfirm pkgconf
      RUN pacman -Sy --noconfirm qt5-base
      RUN pacman -Sy --noconfirm xcb-util-cursor
      RUN pacman -Sy --noconfirm alsa-lib
      RUN pacman -Sy --noconfirm ffmpeg
      RUN pacman -Sy --noconfirm libffi
      RUN pacman -Sy --noconfirm mesa

    # HUD
      RUN pacman -Sy --noconfirm fontconfig
      RUN pacman -Sy --noconfirm freetype2

    # fonts
      RUN pacman -Sy --noconfirm gnu-free-fonts
      RUN pacman -Sy --noconfirm ttf-ubuntu-font-family

    # wine
      RUN printf '%s\n' $'[multilib]\n\
Include = /etc/pacman.d/mirrorlist' >> /etc/pacman.conf
      RUN pacman -Sy --noconfirm wine

    # i386
      # lib32-alsa-lib
        RUN pacman -Sy --noconfirm lib32-alsa-lib

  # install
    RUN mkdir /root/src
    RUN cd /root/src && git clone https://github.com/clementgallet/libTAS.git
    RUN cd /root/src/libTAS && ./build.sh --with-i386 --disable-dependency-tracking
    RUN cd /root/src/libTAS && make install

  # run
    CMD bash
