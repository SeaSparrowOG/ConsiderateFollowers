install(
	FILES
		"${DATA_DIR}/SKSE/Plugins/ConsiderateFollowers.ini"
	DESTINATION "SKSE/Plugins"
	COMPONENT components
	EXCLUDE_FROM_ALL
)

install(
	FILES
		"${DATA_DIR}/ConsiderateFollowers.esp"
	DESTINATION "."
	COMPONENT components
	EXCLUDE_FROM_ALL
)

install(
	FILES
		"${DATA_DIR}/ConsiderateFollowers.bsa"
	DESTINATION "."
	COMPONENT components
	EXCLUDE_FROM_ALL
)
