#set(VCPKG_USE_HEAD_VERSION ON)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO NoahBoddie/lexicon
    REF a7da7a0a4724925c076bb0c87d44c62ff4aa6e5f
    SHA512 c75430e933ac0115818f125fd7c0035b04183d79c23cdf9642fc9039ef5c93e036b977afe8be078d38f109bbdee3ccb067a0cf6963b17c975746326b26d465c2
    HEAD_REF main
)
#could be master for HEAD_REF?
vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
)

vcpkg_install_cmake()

vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake/lexicon)

file(REMOVE_RECURSE
    ${CURRENT_PACKAGES_DIR}/debug/include
    #${CURRENT_PACKAGES_DIR}/lib
)

file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)

