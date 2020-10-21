TEMPLATE = subdirs

SUBDIRS += \
    jsonipc \
    service \

service.depends = jsonipc

DISTFILES += \
    rpm/situations-sonar.changes \
    rpm/situations-sonar.spec \
    rpm/situations-sonar.yaml \

OTHER_FILES += \
    .qmake.conf \
    AUTHORS \
    LICENSE \
    README.md \
