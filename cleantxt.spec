Name: cleantxt
Version: 1.0.1
Release: 1
Group: Applications/Text
Summary: Cleans up tab, space and end-of-line formatting in text files.
License: GPL
URL: https://github.com/rodgersb/cleantxt
Packager: Bryan Rodgers <rodgersb@it.net.au>

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Source: cleantxt-%{version}.tar.gz

%description
CleanTxt is a command-line utility that cleans up formatting issues in
text files. In particular it was designed to:

* Address conversion between different line ending sequences
  e.g. LF (UNIX-like), CR+LF (DOS/MS-Windows) or CR (Apple MacOS classic)
* Change indenting between using hard-tab characters or spaces, and adjust
  the distance between tab stops.
* Removes excess trailing whitespace and blank lines, making minor
  storage savings and reducing noise in the output of diff-like utilities.
* Ensures the final line ends in a newline, which avoids problems with
  some UNIX utilities and triggering warnings in ANSI C compilers.
* Addresses the ctrl-Z character, which on some platforms signifies
  end-of-file.

%prep
%setup -q

%build
%configure
%__make

%install
%make_install

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc Changelog COPYING README TODO

%{_bindir}/cleantxt
%{_mandir}/man1/cleantxt.1*

%changelog
* Sat Nov 30 2013 Bryan Rodgers <rodgersb@it.net.au>
- Packaged up v1.0.1
