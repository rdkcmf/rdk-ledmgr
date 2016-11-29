##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2016 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
bin_PROGRAMS = ledmgr
ledmgr_SOURCES = extended/ledmgr.cpp ledmgrbase.cpp ledmgrmain.cpp indicator.cpp fp_profile.hpp indicator.hpp ledmgrbase.hpp extended/ledmgr.hpp ledmgr_types.hpp
ledmgr_CPPFLAGS = -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/iarmbus -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/iarmmgrs/sysmgr \
	-I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/iarmmgrs/ir -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/iarmmgrs-hal \
	-I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/ds -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/ds-hal \
	-I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/glib-2.0 -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/glib-2.0/include \
	-I${RDK_FSROOT_PATH}/usr/lib/glib-2.0/include \
	-I${RDK_FSROOT_PATH}/usr/include \
	-I$(srcdir)/extended
ledmgr_LDADD = -lpthread -lglib-2.0 -L${RDK_FSROOT_PATH}/usr/local/lib -L${RDK_FSROOT_PATH}/usr/lib -lIARMBus -lds -ldshalcli
