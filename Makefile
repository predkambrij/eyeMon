include test/Makefile

PROJDIR = ${CURDIR}/

FILES_TO_COMPILE = common.cpp main.cpp blinkmeasure.cpp
OUTPUT_BIN =
MACROS =


clonegoogletest :
	git clone https://predkambrij@bitbucket.org/predkambrij/googletest.git test/googletest/

ndk-build :
	/home/developer/other/android_deps/android-ndk-r10d/ndk-build

# -Werror -Wall -Wextra
compile :
	cd jni; \
	g++ $(MACROS) -std=c++11 -Werror -Wextra $(FILES_TO_COMPILE) -o ../$(OUTPUT_BIN) -I. $$(pkg-config --cflags --libs opencv);

setEyelikeFiles :
	$(eval FILES_TO_COMPILE += eyelike/findEyeCorner.cpp eyelike/findEyeCenter.cpp eyelike/helpers.cpp)

# setEyelikeFiles
setDesktopSettings :
	$(eval FILES_TO_COMPILE += common_settings_comp.cpp main_settings.cpp)
	$(eval OUTPUT_BIN = bins/d)

# setEyelikeFiles
setTestSettings :
	$(eval FILES_TO_COMPILE += common_settings_test.cpp main_settings_test.cpp)
	$(eval MACROS = -DIS_TEST)
	$(eval OUTPUT_BIN = bins/t)

setAnnotVars :
	$(eval FILES_TO_COMPILE = annot.cpp)
	$(eval OUTPUT_BIN = bins/annot)

run/% :
	bins/$*

d : setDesktopSettings compile run/d
dt : setTestSettings compile run/t
annot : setAnnotVars compile run/annot


