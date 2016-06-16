#PROJDIR = ${CURDIR}
PROJDIR = .
#PROJDIR = /home/developer/other/android_deps/OpenCV-2.4.10-android-sdk/samples/optical-flow

include test/Makefile

FILES_TO_COMPILE = common.cpp main.cpp blinkmeasure.cpp templatebased.cpp optflow.cpp
# -Werror -Wall -Wextra
COMPILER_FLAGS = -std=c++11 -Wextra
OUTPUT_BIN =
MACROS =


clonegoogletest :
	git clone git@github.com:predkambrij/googletest.git test/googletest/
cloneEyeLike :
	git clone git@github.com:predkambrij/eyeLike.git jni/eyeLike/

ndkb :
	/home/developer/other/android_deps/android-ndk-r10d/ndk-build

_compile :
	cd jni; \
	g++ $(MACROS) $(COMPILER_FLAGS) $(FILES_TO_COMPILE) -o ../$(OUTPUT_BIN) -I. $$(pkg-config --cflags --libs opencv);

_setEyelikeFiles :
	$(eval FILES_TO_COMPILE += eyeLike/src/findEyeCorner.cpp eyeLike/src/findEyeCenter.cpp eyeLike/src/helpers.cpp)

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

dco : _setDesktopSettings _setEyelikeFiles _compile
d : _setDesktopSettings _setEyelikeFiles _compile run/d
dt : _setTestSettings _setEyelikeFiles _compile run/t
annot : _setAnnotVars _compile run/annot

# tests (it doesn't work, see Makefile in test directory)
#_setTestTempl : $(TB)/gtest_main.a
#	$(eval FILES_TO_COMPILE = common.cpp blinkmeasure.cpp templatebased.cpp common_settings_test.cpp ../test/jnitests/templatebased_unittest.cpp ../bins/tests/gtest_main.a)
#	$(eval OUTPUT_BIN = ./bins/tests/templatebased_unittest)
#	$(eval COMPILER_FLAGS += -lpthread)
#testTempl : _setTestTempl _compile
#	$(TB)/templatebased_unittest

testTempl : testclean $(TB)/templatebased_unittest
	$(TB)/templatebased_unittest

te:
	echo $(PROJDIR)