file(REMOVE_RECURSE
  "bin2header.1.gz"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/uninstall.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
