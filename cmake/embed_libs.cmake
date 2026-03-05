file(WRITE "${OUTPUT_FILE}" "")

math(EXPR LAST "${COUNT} - 1")

foreach(I RANGE 0 ${LAST})
	set(VAR_NAME "${VAR_${I}}")
	set(FILE_PATH "${PATH_${I}}")
	file(READ "${FILE_PATH}" HEX_CONTENT HEX)
	string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," HEX_BYTES "${HEX_CONTENT}")
	file(APPEND "${OUTPUT_FILE}" "static const char ${VAR_NAME}[] = {${HEX_BYTES}0x00};\n")
endforeach()
