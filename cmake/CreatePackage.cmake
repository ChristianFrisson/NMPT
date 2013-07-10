# FUBI packaging system.
#
# at the end of any application that needs packaging,
# add after: INCLUDE (${CMAKE_SOURCE_DIR}/cmake/CreatePackage.cmake)
#
# to create the package (with app-name the CMake target name of the application):
# Apple *.app bundle: make app-name install
# Linux *.deb package: cpack --config CPackConfig-app-name.cmake

# PACKAGING WITH CPack
# For more see http://www.cmake.org/Wiki/CMake:Packaging_With_CPack
IF(NOT USE_DEBUG) # mandatory for packaging release versions

IF(APPLE)
	SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/release")
ENDIF()

IF(UNIX OR APPLE) # not yet tested with Windows 
	INCLUDE(InstallRequiredSystemLibraries)
	set(CPACK_PACKAGE_NAME "${EXECUTABLE_NAME}")
	set(CPACK_BUNDLE_NAME "${EXECUTABLE_NAME}")
    	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Melon: user-friendly 3D positions to pointers with OpenNI")
    	set(CPACK_PACKAGE_DESCRIPTION "Melon is a tool that allows easy conversion from 3D positions to pointers usable in an interactive environment. It is based on OpenNI & NITE and has been developed at the numediart Institute - http://www.numediart.org.")
    	SET(CPACK_PACKAGE_VENDOR "numediart")
        set(CPACK_PACKAGE_CONTACT "http://numediart.org/")
        set(CPACK_PACKAGE_VERSION "${${EXECUTABLE_NAME}_VERSION}")
	set(CPACK_SOURCE_IGNORE_FILES
		"^${PROJECT_SOURCE_DIR}/Builds/"
	)
    	#Unused so far, since we make single application packages instead of a single framework distribution
	#set(CPACK_PACKAGE_EXECUTABLES "FUBIOSC" "FUBI.icns") #should contain pairs of <executable> and <icon name>
	IF (APPLE)
		set(CPACK_GENERATOR "DragNDrop")#to test: set(CPACK_GENERATOR "PackageMaker;OSXX11")
	ELSE()
		set(CPACK_GENERATOR "TBZ2")
	ENDIF()
ENDIF ( UNIX OR APPLE )

# Borrowed from Performous performous-packaging.cmake CMakeModule
IF(UNIX)
	# Try to find architecture
	execute_process(COMMAND uname -m OUTPUT_VARIABLE CPACK_PACKAGE_ARCHITECTURE)
	string(STRIP "${CPACK_PACKAGE_ARCHITECTURE}" CPACK_PACKAGE_ARCHITECTURE)
	# Try to find distro name and distro-specific arch
	execute_process(COMMAND lsb_release -is OUTPUT_VARIABLE LSB_ID)
	execute_process(COMMAND lsb_release -rs OUTPUT_VARIABLE LSB_RELEASE)
	string(STRIP "${LSB_ID}" LSB_ID)
	string(STRIP "${LSB_RELEASE}" LSB_RELEASE)
	set(LSB_DISTRIB "${LSB_ID}${LSB_RELEASE}")
	IF(NOT LSB_DISTRIB)
		set(LSB_DISTRIB "unix")
	ENDIF(NOT LSB_DISTRIB)

	IF(NOT APPLE)
		SET(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/CPackConfig-${EXECUTABLE_NAME}.cmake")
		SET(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${EXECUTABLE_NAME};/")
		#EXECUTE_PROCESS(COMMAND rm "${CMAKE_BINARY_DIR}/CPackConfig.cmake")
	ENDIF()

	# For Debian-based distros we want to create DEB packages.
	IF("${LSB_DISTRIB}" MATCHES "Ubuntu|Debian")
		SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "numediart.org")
		set(CPACK_GENERATOR "DEB")
		set(CPACK_DEBIAN_PACKAGE_PRIORITY "extra")
		set(CPACK_DEBIAN_PACKAGE_SECTION "universe/multimedia")
		#set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "..., ...")
		SET(CPACK_DEBIAN_PACKAGE_VERSION "${${EXECUTABLE_NAME}_VERSION}")
		SET(CPACK_DEBIAN_PACKAGE_NAME "${EXECUTABLE_NAME}")

		# We need to alter the architecture names as per distro rules
		IF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "i[3-6]86")
			set(CPACK_PACKAGE_ARCHITECTURE i386)
		ENDIF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "i[3-6]86")
		IF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "x86_64")
			set(CPACK_PACKAGE_ARCHITECTURE amd64)
		ENDIF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "x86_64")

		# Set the dependencies based on the distro version

		# Ubuntu
        	IF("${LSB_DISTRIB}" MATCHES "Ubuntu")
            		set(CPACK_DEBIAN_PACKAGE_MAINTAINER "numediart")
        	ENDIF()

		IF("${LSB_DISTRIB}" MATCHES "Ubuntu1") # for our interest, 10.04, 10.10, 11.04...

			# OpenNI https://launchpad.net/~v-launchpad-jochen-sprickerhof-de/
            		IF(USE_OPENNI1)
				list(APPEND UBUNTU_DEPS "openni") #openni-sensor-primesense
			ENDIF()

			# OpenCV
			IF(USE_OPENCV)
				IF(OpenCV_VERSION VERSION_GREATER 2.3.0)
                    			IF(("${LSB_DISTRIB}" MATCHES "Ubuntu10.04") OR ("${LSB_DISTRIB}" MATCHES "Ubuntu11.04") OR ("${LSB_DISTRIB}" MATCHES "Ubuntu11.10"))
                    				#MESSAGE("\n\nWARNING!\nRepackage an up-to-date OpenCV 2.3.1 package from https://launchpad.net/~gijzelaar/+archive/opencv2.3 against recent FFmpeg > 0.8.x packages from https://launchpad.net/~jon-severinsson/+archive/ffmpeg") # no we need to repackage them against more uptodate ffmpeg packages rebuilt from Jon Severinsson's ppa source packages
                    				list(APPEND UBUNTU_DEPS "libopencv-dev")
                    			ELSE()
                    				MESSAGE(FATAL_ERROR "OpenCV >= 2.3.0 not available as package for your distribution")
                    			ENDIF()
				ELSE()	
                    			IF("${LSB_DISTRIB}" MATCHES "Ubuntu10.04")
                        			list(APPEND UBUNTU_DEPS "libcv4" "libcvaux4" "libhighgui4")
                    			ELSE()
                        			list(APPEND UBUNTU_DEPS "libcv2.1" "libcvaux2.1" "libhighgui2.1")
                    			ENDIF()
                		ENDIF()
			ENDIF()
 
			IF(NOT CPACK_DEBIAN_PACKAGE_DEPENDS)
				message("WARNING: ${LSB_DISTRIB} not supported yet.\nPlease set deps in cmake/CreatePackage.cmake before packaging.")
			ENDIF(NOT CPACK_DEBIAN_PACKAGE_DEPENDS)
			string(TOLOWER "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${LSB_DISTRIB}_${CPACK_PACKAGE_ARCHITECTURE}" CPACK_PACKAGE_FILE_NAME)

			# Install the icon file
			INSTALL(FILES ${CMAKE_SOURCE_DIR}/resources/img/melon.png DESTINATION share/pixmaps COMPONENT ${EXECUTABLE_NAME} RENAME ${EXECUTABLE_NAME}.png)
			#INSTALL(FILES ${CMAKE_SOURCE_DIR}/resources/icons/melon.xpm DESTINATION share/pixmaps COMPONENT ${EXECUTABLE_NAME} RENAME ${EXECUTABLE_NAME}.xpm)

			# Install the .desktop description
			file(WRITE ${CMAKE_BINARY_DIR}/${EXECUTABLE_NAME}.desktop [Desktop\ Entry]\nType=Application\nExec=${EXECUTABLE_NAME}\nMimeType=application/x-${EXECUTABLE_NAME};\nIcon=${EXECUTABLE_NAME}\nName=${EXECUTABLE_NAME}\nGenericName=${DESCRIPTION}\nComment=${DESCRIPTION})
			INSTALL(FILES ${CMAKE_BINARY_DIR}/${EXECUTABLE_NAME}.desktop DESTINATION share/applications COMPONENT ${EXECUTABLE_NAME})

		ENDIF("${LSB_DISTRIB}" MATCHES "Ubuntu1")

	ENDIF("${LSB_DISTRIB}" MATCHES "Ubuntu|Debian")
	# For Fedora-based distros we want to create RPM packages.
	# IF("${LSB_DISTRIB}" MATCHES "Fedora")
	# 	set(CPACK_GENERATOR "RPM")
	# 	set(CPACK_RPM_PACKAGE_NAME "${CMAKE_EXECUTABLE_NAME}")
	# 	set(CPACK_RPM_PACKAGE_VERSION "${PROJECT_VERSION}")
	# 	set(CPACK_RPM_PACKAGE_RELEASE "1")
	# 	set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games")
	# 	set(CPACK_RPM_PACKAGE_LICENSE "LGPL?")
	# 	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Melon: user-friendly 3D positions to pointers with OpenNI.")
	# 	set(CPACK_RPM_PACKAGE_DESCRIPTION "Melon is a tool that allows easy conversion from 3D positions to pointers usable in an interactive environment. It is based on OpenNI & NITE and has been developed at the numediart Institute - http://www.numediart.org.")
		# We need to alter the architecture names as per distro rules
	# 	IF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "i[3-6]86")
	# 		set(CPACK_PACKAGE_ARCHITECTURE i386)
	# 	ENDIF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "i[3-6]86")
	# 	IF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "x86_64")
	# 		set(CPACK_PACKAGE_ARCHITECTURE amd64)
	# 	ENDIF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "x86_64")
	# 	# Set the dependencies based on the distro version
	# 	IF("${LSB_DISTRIB}" MATCHES "Fedora14")
	# 		set(CPACK_RPM_PACKAGE_REQUIRES "..., ...")
	# 	ENDIF("${LSB_DISTRIB}" MATCHES "Fedora14")
	# 	IF("${LSB_DISTRIB}" MATCHES "Fedora13")
	# 		set(CPACK_RPM_PACKAGE_REQUIRES "..., ...")
	# 	ENDIF("${LSB_DISTRIB}" MATCHES "Fedora13")
	# 	IF(NOT CPACK_RPM_PACKAGE_REQUIRES)
	# 		message("WARNING: ${LSB_DISTRIB} is not supported.\nPlease set deps in cmake/performous-packaging.cmake before packaging.")
	# 	ENDIF(NOT CPACK_RPM_PACKAGE_REQUIRES)
	#  ENDIF("${LSB_DISTRIB}" MATCHES "Fedora")
	set(CPACK_SYSTEM_NAME "${LSB_DISTRIB}-${CPACK_PACKAGE_ARCHITECTURE}")
ENDIF(UNIX)

#From: http://www.cmake.org/Wiki/BundleUtilitiesExample
SET(plugin_dest_dir bin)
SET(qtconf_dest_dir bin)
SET(qtframeworks_dest_dir bin)
SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${EXECUTABLE_NAME}")
IF(APPLE)
  SET(plugin_dest_dir ${EXECUTABLE_NAME}.app/Contents/PlugIns)
  SET(qtconf_dest_dir ${EXECUTABLE_NAME}.app/Contents/Resources)
  SET(qtframeworks_dest_dir ${EXECUTABLE_NAME}.app/Contents/Resources)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_NAME}.app")
ENDIF(APPLE)
IF(WIN32)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${EXECUTABLE_NAME}.exe")
ENDIF(WIN32)

#--------------------------------------------------------------------------------
# Install the QtTest application, on Apple, the bundle is at the root of the
# install tree, and on other platforms it'll go into the bin directory.
INSTALL(TARGETS ${EXECUTABLE_NAME}
   BUNDLE DESTINATION . COMPONENT ${EXECUTABLE_NAME}
   RUNTIME DESTINATION bin COMPONENT ${EXECUTABLE_NAME}
)

#--------------------------------------------------------------------------------
# Use BundleUtilities to get all other dependencies for the application to work.
# It takes a bundle or executable along with possible plugins and inspects it
# for dependencies.  If they are not system dependencies, they are copied.

# Now the work of copying dependencies into the bundle/package
# The quotes are escaped and variables to use at install time have their $ escaped
# An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
# Note that the image plugins depend on QtSvg and QtXml, and it got those copied
# over.
IF(APPLE)
LIST(APPEND PLUGINS ${EXTRA_APPS})
INSTALL(CODE "
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"${PLUGINS}\" \"${LINKED_DIRECTORIES}\")
    " COMPONENT ${EXECUTABLE_NAME})
ENDIF()

#IF(APPLE)
#	INSTALL(CODE "hdiutil create -format UDBZ -srcfolder \"${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_NAME}.app\" \"${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_NAME}.dmg\"")
#ENDIF()

IF(APPLE)
	# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
	# where CPackConfig.cmake is created by including CPack
	# And then there's ways to customize this as well
	set(CPACK_PACKAGE_NAME "${EXECUTABLE_NAME}")
	set(CPACK_BUNDLE_NAME "${EXECUTABLE_NAME}")
	set(CPACK_BINARY_DRAGNDROP ON)
	set(CPACK_PACKAGE_EXECUTABLES "${EXECUTABLE_NAME}" "FUBI.icns") #should contain pairs of <executable> and <icon name>
	set(CPACK_GENERATOR "PackageMaker;OSXX11")
	#include(CPack)
	#EXECUTE_PROCESS(COMMAND cp "${CMAKE_BINARY_DIR}/CPackConfig.cmake" "${CMAKE_BINARY_DIR}/CPackConfig${EXECUTABLE_NAME}.cmake")
ENDIF()

IF(NOT APPLE)
	include(CPack)
	#EXECUTE_PROCESS(COMMAND rm "${CMAKE_BINARY_DIR}/CPackSourceConfig.cmake")
ENDIF()

ENDIF(NOT USE_DEBUG) # mandatory for packaging release versions
