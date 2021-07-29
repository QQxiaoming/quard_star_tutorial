#include <QMetaType>

namespace NS
{
    Q_NAMESPACE
    enum NamespaceEnum { Alpha=1, Bravo, Charlie };
    Q_ENUM_NS(NamespaceEnum)
}

namespace NS2
{
    Q_NAMESPACE
    enum class NamespaceEnum : quint8 { Alpha=1, Bravo, Charlie };
    Q_ENUM_NS(NamespaceEnum)
}
