load(configure)
qtCompileTest(flite)
qtCompileTest(flite_alsa)
!packagesExist(speech-dispatcher) {
    qtCompileTest(speechd)
}
load(qt_parts)
