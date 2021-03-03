---
layout: page
title: Running libTAS using WSL 2
permalink: /guides/wsl/
---

*This guide was written by @SchneeheideWW*

# Introduction

So you have learnt about the existence of libTAS and you are really excited to
TAS some of your favourite PC games but you don't have a computer running a
Linux-based operating system or you simply want to stick with your Windows OS.
In that case, using the Windows Subsystem for Linux is probably the most
efficient approach.

This guide is meant for people who have very little or no experience with Linux
so I will take things slowly.

By the way, Linux is a family of operating systems - the one we are going to
use specifically is Ubuntu (20.04 LTS as I am writing this guide). First off,
let's take a look at the entire setup


libTAS can't run natively on Windows. Thus, we use WSL 2 to create an
environment that lets us execute Linux-based code. Effectively, this will give
us a terminal in which we can do a lot of things given we know the commands.
For many Windows users this is usually an uncomfortable experience - they'd
rather have a nice graphical user interface (GUI) with lots of things to click on.
As it happens, libTAS does come with a GUI - and currently, here's the problem: 
We only have the terminal and no way to display something fancy as a GUI.
So we need the "X Window System" to bridge the gap. In our case, we will use vcxsrv.
Once our X-Server has been set up, we can open libTAS and then (hopefully) start TASing!

Summed up, we have a Windows 10 running WSL 2 running libTAS through an X-Server.
A bit convoluted, but more efficient than using a VM! And once everything is
installed and set up, it's only a few clicks and you are ready to TAS.

Things you will need to download:

* [Ubuntu](https://www.microsoft.com/store/apps/9n6svws3rx71)
* [vcxsrv](https://sourceforge.net/projects/vcxsrv/)
* [libTAS](../)

And of course, you will need a game that runs on Linux. Otherwise you need to
use wine, which, aside from adding yet another layer in our approach, will
potentially not work.

# Step 1: Installing & Setting Up WSL 2 

I think [Microsoft's guide](https://docs.microsoft.com/en-us/windows/wsl/install-win10#manual-installation-steps) did a decent job explaining the installation process so I just refer to that here. If however you do happen to run into trouble, feel free to join the [libTAS discord server](https://discord.gg/3MBVAzU) and ask for help. Optionally, you can also find tutorials on YouTube.

You can check that you have the version 2 installed by running the following command in a Windows terminal: `wsl --list --verbose`. If it prints `1` at `VERSION`, then you need to upgrade to version 2.

Once WSL 2 has been installed, grab Ubuntu from the store (this is all mentioned in Microsoft's guide but I just
want to point it out here again). You should have a new Windows-App called Ubuntu 20.04 LTS.

Start it and a terminal will open. For the first time only you'll be asked to
create a [user account](https://docs.microsoft.com/en-us/windows/wsl/user-support) which will be your default user.

If you like to adjust your terminal, right-click on the top bar to open a menu and select properties at the bottom.
Here you can adjust the copy&paste behaviour, font size, colours and other things. Generally, if you want to paste something, you can just right-click anywhere on the console.

Now it's time to check for updates. Paste this into your terminal and hit enter:

    sudo apt update
    sudo apt upgrade

`sudo` stands for "super user do" which is basically admin powers. `sudo apt update`
checks for updates and `sudo apt upgrade` is for actually updating. Confirm and
wait for the updates to finish.

# Step 2: Installing libTAS

Next we are going to install libTAS. You can access files outside of Ubuntu
by mounting your harddrive which is very convenient. This means you can just
download libTAS on your Windows environment, then access it in the terminal.
It is recommended that you place the .deb file somewhere convenient because
you will have to fill in its path in the terminal using the cd command.

For example:

    cd /mnt/d/libTAS
    
would change the current directory to D:\libTAS on Windows (do pay attention to
the different pathing style).

Adjust the command to your path where you've put the .deb file and change your
current directory to it, then run

    sudo apt install ./libtas_*_amd64.deb

The `*` makes it so that it uses our .deb file regardless of its version, e.g.
it may be named `libtas_1.4.0_d086878_amd64.deb` and it will still work. Very convenient.

Potentially you will see error messages saying that there are unmet dependencies.
The easiest way to fix that is to simply run

    sudo apt --fix-broken install

and it will install a lot of stuff. Hopefully that should be it and libTAS has been installed.

However, as mentioned at the beginning, we now need vcxsrv to be able to display
it (or anything that uses a GUI for that matter).

# Step 3: Installing & Setting Up VcXsrv

Install VcXsrv on Windows, then run `xlaunch.exe` located in the installation folder. 
Your firewall will most likely want to block it. Permit access both on private
and public networks. You may want to go into your firewall's settings and double check.

Now we are going to configure the X-Server. On the first screen, change "Display number" to 0. 
"Multiple windows" is picked by default and useful for our needs because you
will have multiple windows (libTAS, your game, RAM search/watch, Input Editor etc).

On the second screen, select "Start no client". We start our programs in due time.

On the third screen, uncheck "Native opengl" and check "Disable access control".

On the fourth screen you can save these settings so you can launch them similarly
to a normal shortcut which is very convenient for later.

Confirm and start your X-Server. 

There will be an icon in your taskbar's corner and a process called 
"VcXsrv windows xserver" in your Task-Manager shall you need to kill it.

# Step 4: The Final Stretch & Displaying libTAS

Now to connect WSL with our server, paste

    export DISPLAY=$(awk '/nameserver / {print $2; exit}' /etc/resolv.conf 2>/dev/null):0

into your console and hit enter. Technically we should be done now. If you run

    libTAS
    
a window should pop and you should see libTAS in all its glory!

However the export thing is necessary every time you start Ubuntu. There is a
convenient way to have it run automatically by typing said export line into a file called `.bashrc`.

Because this tutorial was made for Windows users, in our next step we will install
a nice file explorer with a graphical interface as well as a text editor. Run

    apt install nautilus

in your console to install said explorer. For the text editor I went with `gedit`.
You can install it by running

    apt install gedit

Once it's finished, simply run

    nautilus

and a new window should pop up, given that you've established a connection first.

You should now see a file explorer. Navigate to `Home` and look for a file
called `.bashrc` (yes, it starts with a dot). Right-click on it and open it with
gedit (our new text editor), then scroll to the file's end (don't touch anything
on your way please) and add

    export DISPLAY=$(awk '/nameserver / {print $2; exit}' /etc/resolv.conf 2>/dev/null):0

as a new line. Save and close your terminal.

Now if you've done everything correctly, the next time you open a terminal and
run libTAS, it should pop up right away without having to run the export
command first (make sure that your X-Server is running of course).

# Step 5: Installing Steam (optional)

If your game has a Linux version and exists on Steam, then the easiest way to
get the files of the game you want to TAS would be to simply install Steam and download it.
There are some ways to directly access Steam repositories but I am not familiar with that. So run

    sudo dpkg --add-architecture i386 # allows WSL to access 32 bit repos
    sudo add-apt-repository multiverse
    sudo apt update
    sudo apt install steam

and it should install Steam. Download your game, then launch it through libTAS.
