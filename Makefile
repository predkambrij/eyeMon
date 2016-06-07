include test/Makefile

PROJDIR = ${CURDIR}/

FILES_TO_COMPILE = common.cpp main.cpp blinkmeasure.cpp templatebased.cpp
OUTPUT_BIN =
MACROS =


clonegoogletest :
	git clone git@github.com:predkambrij/googletest.git test/googletest/

ndkb :
	/home/developer/other/android_deps/android-ndk-r10d/ndk-build

# -Werror -Wall -Wextra
_compile :
	cd jni; \
	g++ $(MACROS) -std=c++11 -Wall -Wextra $(FILES_TO_COMPILE) -o ../$(OUTPUT_BIN) -I. $$(pkg-config --cflags --libs opencv);

_setEyelikeFiles :
	$(eval FILES_TO_COMPILE += eyelike/findEyeCorner.cpp eyelike/findEyeCenter.cpp eyelike/helpers.cpp)

# setEyelikeFiles
_setDesktopSettings :
	$(eval FILES_TO_COMPILE += common_settings_comp.cpp main_settings.cpp)
	$(eval OUTPUT_BIN = bins/d)

# setEyelikeFiles
_setTestSettings :
	$(eval FILES_TO_COMPILE += common_settings_test.cpp main_settings_test.cpp)
	$(eval MACROS = -DIS_TEST)
	$(eval OUTPUT_BIN = bins/t)

_setAnnotVars :
	$(eval FILES_TO_COMPILE = annot.cpp)
	$(eval OUTPUT_BIN = bins/annot)

run/% :
	bins/$*

dco : _setDesktopSettings _compile
d : _setDesktopSettings _compile run/d
dt : _setTestSettings _compile run/t
annot : _setAnnotVars _compile run/annot


