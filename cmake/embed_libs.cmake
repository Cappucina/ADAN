
# Function to embed a file as a C string macro
function(embed_file_as_c_string INPUT OUTPUT MACRO)
	file(READ "${INPUT}" CONTENTS)
	string(REPLACE "\\" "\\\\" CONTENTS "${CONTENTS}")
	string(REPLACE "\"" "\\\"" CONTENTS "${CONTENTS}")
	string(REPLACE "\n" "\\n\" \\\n\"" CONTENTS "${CONTENTS}")
	file(APPEND "${OUTPUT}" "#define ${MACRO} \"${CONTENTS}\"\n\n")
endfunction()
