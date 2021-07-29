%{Cpp:LicenseTemplate}\

#ifndef %{GUARD}
#define %{GUARD}

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <Qt3DCore/qbackendnode.h>

QT_BEGIN_NAMESPACE
%{JS: Cpp.openNamespaces('%{Class}')}
class %{CN} : public Qt3DCore::%{Base}
{
public:
    %{CN}();

    void sceneChangeEvent(const Qt3DCore::QSceneChangePtr &e) override;

private:
    void initializeFromPeer(const Qt3DCore::QNodeCreatedChangeBasePtr &change) final;
};
%{JS: Cpp.closeNamespaces('%{Class}')}

QT_END_NAMESPACE

#endif // %{GUARD}\
