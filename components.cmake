install(
	FILES
		"${DATA_DIR}/SKSE/Plugins/ConsiderateFollowers.ini"
	DESTINATION "ReleaseBuild/SKSE/Plugins"
	COMPONENT components
	EXCLUDE_FROM_ALL
)

install(
	FILES
		"${DATA_DIR}/ConsiderateFollowers.esp"
	DESTINATION "ReleaseBuild"
	COMPONENT components
	EXCLUDE_FROM_ALL
)

install(
	FILES
		"${DATA_DIR}/ConsiderateFollowers.bsa"
	DESTINATION "ReleaseBuild"
	COMPONENT components
	EXCLUDE_FROM_ALL
)
