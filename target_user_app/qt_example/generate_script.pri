target.path = ../output

generate_script.commands = echo \'export QT_HOME=/opt/Qt-5.12.11\' > $${target.path}/$${TARGET}.sh && \
           echo \'export QT_QPA_FB_DRM=1\' >> $${target.path}/$${TARGET}.sh && \
           echo \'export QT_PLUGIN_PATH=\$\$QT_HOME/plugins\' >> $${target.path}/$${TARGET}.sh && \
           echo \'export LD_LIBRARY_PATH=\"\$\$LD_LIBRARY_PATH:\$\$QT_HOME/lib\"\'  >> $${target.path}/$${TARGET}.sh && \
           echo \'./$${TARGET}\' >> $${target.path}/$${TARGET}.sh
generate_script.target = run_script
generate_script.path = $${target.path}

QMAKE_EXTRA_TARGETS += generate_script

INSTALLS += generate_script
