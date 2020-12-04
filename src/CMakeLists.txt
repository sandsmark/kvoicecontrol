qt1_wrap_moc(MOC_FILES
    buffer.h
    docking.h
    kvoicecontrol.h
    optionsdata.h
    options.h
    preprocessing.h
    reference_editor.h
    reference.h
    score.h
    soundcard.h
    speakermodel.h
    train_dialog.h
    utterance.h
)

add_executable(kvoicecontrol
    kvoicecontrol.cpp speakermodel.cpp reference.cpp utterance.cpp
    reference_editor.cpp score.cpp preprocessing.cpp realfftf.c soundcard.cpp
    buffer.cpp options.cpp optionsdata.cpp docking.cpp train_dialog.cpp

    ${MOC_FILES}
)

target_compile_definitions(kvoicecontrol PRIVATE VERSION=\"${PROJECT_VERSION}\")

target_link_libraries(kvoicecontrol
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    Qt::Qt1
    pulse-simple ${PULSEAUDIO_LIBRARY}
)

install(TARGETS kvoicecontrol RUNTIME DESTINATION ${KDE1_BINDIR})

