target_sources(${TARGET_NAME} PRIVATE
    CommandManagerTests.cpp
    ComponentSnapshotTests.cpp
    ScriptBindingCommandTests.cpp
    pch.cpp
    pch.h
    )

# The editor sources under test, compiled into this binary directly (MatchaEditor is an
# executable, so there's no library to link). Only Qt-free files can go here: CommandManager.cpp
# is the Command subsystem's one non-header-only piece, and every command class is either
# header-only (PropertyEditCommand, AddComponentCommand, RemoveComponentCommand, ComponentSnapshot)
# or would need its own .cpp added below (CreateEntityCommand, DeleteEntitiesCommand,
# RenameEntitiesCommand, ReparentEntitiesCommand, PasteEntitiesCommand) before a test could
# exercise it.
target_sources(${TARGET_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/MatchaEditor/Code/Core/CommandManager.cpp
    )
