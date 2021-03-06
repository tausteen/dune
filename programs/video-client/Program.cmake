if(QT_LIBRARIES)
  dune_qt4_wrap_cpp(MOC_SRCS
    programs/video-client/VideoClient.hpp
    programs/video-client/GraphicsScene.hpp
    )

  add_executable(video-client
    programs/video-client/VideoClient.cpp
    programs/video-client/GraphicsScene.cpp
    programs/video-client/Main.cpp
    ${MOC_SRCS})

  target_link_libraries(video-client dune-core ${DUNE_SYS_LIBS} ${QT_LIBRARIES})
  set(DUNE_EXTRA_EXE ${DUNE_EXTRA_EXE} video-client)
  set(DUNE_EXTRA_EXE_NAMES ${DUNE_EXTRA_EXE_NAMES} "video-client;Video Client")
endif(QT_LIBRARIES)
