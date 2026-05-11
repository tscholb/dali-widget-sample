Name:       com.samsung.dali.widget-viewer-interactive
Summary:    DALi native widget viewer interactive
Version:    1.0.0
Release:    1
Group:      System/Libraries
License:    Apache-2.0
URL:        https://github.com/dalihub/dali-widget-sample
Source0:    %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(dali2-core)
BuildRequires:  pkgconfig(dali2-adaptor)
BuildRequires:  pkgconfig(dali2-toolkit)
BuildRequires:  dali2-integration-devel
BuildRequires:  dali2-adaptor-integration-devel
BuildRequires:  dali2-toolkit-integration-devel
BuildRequires:  pkgconfig(widget_viewer_dali)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(message-port)

%description
DALi native widget viewer interactive.
Demonstrates interactive widget viewing with touch and wheel event
forwarding using WidgetViewManager and WidgetView from widget-viewer-dali.

##############################
# Preparation
##############################
%prep
%setup -q

%define app_root_dir    widget-viewer-interactive/
%define app_ro_dir      /usr/apps/%{name}
%define xml_file_dir    /usr/share/packages
%define app_res_dir     %{app_ro_dir}/res
%define app_exe_dir     %{app_ro_dir}/bin

##############################
# Build
##############################
%build
PREFIX="/usr"
CXXFLAGS+=" -Wall -g -O2"
LDFLAGS+=" -Wl,--rpath=$PREFIX/lib -Wl,--as-needed"

%ifarch %{arm}
CXXFLAGS+=" -D_ARCH_ARM_"
%endif

cd %{_builddir}/%{name}-%{version}/%{app_root_dir}

cmake -DCMAKE_INSTALL_PREFIX=%{app_ro_dir} \
      -DTIZEN:BOOL=ON \
%if 0%{?enable_debug}
      -DCMAKE_BUILD_TYPE=Debug \
%endif
      .

make %{?jobs:-j%jobs}

##############################
# Installation
##############################
%install
rm -rf %{buildroot}
cd %{app_root_dir}
%make_install

mkdir -p %{buildroot}%{xml_file_dir}
cp -f %{_builddir}/%{name}-%{version}/%{app_root_dir}/%{name}.xml %{buildroot}%{xml_file_dir}

##############################
# Post Install
##############################
%post
/sbin/ldconfig
exit 0

##############################
# Post Uninstall
##############################
%postun
/sbin/ldconfig
exit 0

##############################
# Files
##############################
%files
%manifest %{app_root_dir}/%{name}.manifest
%defattr(-,root,root,-)
%{app_exe_dir}/widget-viewer-interactive
%{xml_file_dir}/%{name}.xml
