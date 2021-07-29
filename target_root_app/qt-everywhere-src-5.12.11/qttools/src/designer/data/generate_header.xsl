<!DOCTYPE xsl:stylesheet [
     <!ENTITY endl "&#10;">
]>
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xs="http://www.w3.org/2001/XMLSchema">

    <xsl:output method="text"/>

    <xsl:include href="generate_shared.xsl"/>

<!-- Forward declaration -->

    <xsl:template name="class-forward-declaration">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/attribute::name)"/>

        <xsl:text>class </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>;&endl;</xsl:text>
    </xsl:template>

<!-- Class declaration: child element accessors -->

    <xsl:template name="child-element-accessors">
        <xsl:param name="node"/>

        <xsl:variable name="set" select="$node/xs:sequence | $node/xs:choice | $node/xs:all"/>
        <xsl:variable name="count" select="count($set)"/>

        <xsl:if test="$count &gt; 0">
            <xsl:text>    // child element accessors&endl;</xsl:text>
        </xsl:if>

        <xsl:for-each select="$set">
            <xsl:call-template name="child-element-accessor">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

    </xsl:template>

<!-- Class declaration: child element accessor -->

    <xsl:template name="child-element-accessor">
        <xsl:param name="node"/>

        <xsl:variable name="isChoice" select="name($node)='xs:choice'"/>

        <xsl:if test="$isChoice">
            <xsl:text>    enum Kind { Unknown = 0</xsl:text>
            <xsl:for-each select="$node/xs:element">
                <xsl:variable name="camel-case-name">
                    <xsl:call-template name="camel-case">
                        <xsl:with-param name="text" select="@name"/>
                    </xsl:call-template>
                </xsl:variable>

                <xsl:variable name="cap-name">
                    <xsl:call-template name="cap-first-char">
                        <xsl:with-param name="text" select="$camel-case-name"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:text>, </xsl:text>
                <xsl:value-of select="$cap-name"/>
            </xsl:for-each>
            <xsl:text> };&endl;</xsl:text>
            <xsl:text>    inline Kind kind() const { return m_kind; }&endl;&endl;</xsl:text>
        </xsl:if>

        <xsl:for-each select="$node/xs:element[not(@use) or (@use!='deprecated')]">
            <xsl:variable name="array" select="@maxOccurs='unbounded'"/>
            <xsl:variable name="camel-case-name">
                <xsl:call-template name="camel-case">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cap-name">
                <xsl:call-template name="cap-first-char">
                    <xsl:with-param name="text" select="$camel-case-name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="return-cpp-type">
                <xsl:call-template name="xs-type-to-cpp-return-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                    <xsl:with-param name="array" select="$array"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="argument-cpp-type">
                <xsl:call-template name="xs-type-to-cpp-argument-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                    <xsl:with-param name="array" select="$array"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                    <xsl:with-param name="array" select="$array"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>    inline </xsl:text>
            <xsl:value-of select="$return-cpp-type"/>
            <xsl:text>element</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>() const { return m_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text>; }&endl;</xsl:text>

            <xsl:if test="$xs-type-cat = 'pointer'">
                <xsl:text>    </xsl:text>
                <xsl:value-of select="$return-cpp-type"/>
                <xsl:text>takeElement</xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>();&endl;</xsl:text>
            </xsl:if>

            <xsl:text>    void setElement</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>(</xsl:text>
            <xsl:value-of select="$argument-cpp-type"/>
            <xsl:text>a);&endl;</xsl:text>

            <xsl:if test="not($isChoice) and not(@maxOccurs='unbounded')">
                <xsl:text>    inline bool hasElement</xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>() const { return m_children &amp; </xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>; }&endl;</xsl:text>
                <xsl:text>    void clearElement</xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>();&endl;</xsl:text>
            </xsl:if>
            <xsl:text>&endl;</xsl:text>

        </xsl:for-each>
    </xsl:template>

<!-- Class declaration: child elements data -->

    <xsl:template name="child-elements-data">
        <xsl:param name="node"/>

        <xsl:variable name="set" select="$node/xs:sequence | $node/xs:choice | $node/xs:all"/>
        <xsl:variable name="count" select="count($set)"/>

        <xsl:if test="boolean($node/xs:choice)">
            <xsl:text>&endl;    // child element data&endl;</xsl:text>
            <xsl:text>    Kind m_kind = Unknown;&endl;</xsl:text>
        </xsl:if>
        <xsl:if test="not($node/xs:choice) and $count &gt; 0">
            <xsl:text>&endl;    // child element data&endl;</xsl:text>
            <xsl:text>    uint m_children = 0;&endl;</xsl:text>
        </xsl:if>

        <xsl:for-each select="$set">
            <xsl:call-template name="child-element-data">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

    </xsl:template>

<!-- Class declaration: child element data -->

    <xsl:template name="child-element-data">
        <xsl:param name="node"/>

        <xsl:variable name="isChoice" select="$node[name()='xs:choice']"/>

        <xsl:for-each select="$node/xs:element[not(@use) or (@use!='deprecated')]">
            <xsl:variable name="camel-case-name">
                <xsl:call-template name="camel-case">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cpp-type">
                <xsl:call-template name="xs-type-to-cpp-return-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                    <xsl:with-param name="array" select="@maxOccurs='unbounded'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$cpp-type"/>
            <xsl:text>m_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>

            <xsl:variable name="array" select="@maxOccurs='unbounded'"/>
            <xsl:if test="not($array)">
                <xsl:choose>
                    <xsl:when test="@type = 'xs:integer' or @type = 'xs:unsignedInt' or @type = 'xs:long' or @type = 'xs:unsignedLong'">
                        <xsl:text> = 0</xsl:text>
                    </xsl:when>
                    <xsl:when test="@type = 'xs:double' or @type = 'xs:float'">
                        <xsl:text> = 0.0</xsl:text>
                    </xsl:when>
                    <xsl:when test="@type = 'xs:boolean'">
                        <xsl:text> = false</xsl:text>
                    </xsl:when>
                    <xsl:when test="@type = 'xs:string'">
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:text> = nullptr</xsl:text>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:if>

            <xsl:text>;&endl;</xsl:text>
        </xsl:for-each>

        <xsl:if test="not($isChoice) and not(@macOccurs='unbounded')">
            <xsl:text>&endl;    enum Child {&endl;</xsl:text>
            <xsl:for-each select="$node/xs:element[not(@use) or (@use!='deprecated')]">
                <xsl:variable name="camel-case-name">
                    <xsl:call-template name="camel-case">
                        <xsl:with-param name="text" select="@name"/>
                    </xsl:call-template>
                </xsl:variable>

                <xsl:text>        </xsl:text>
                <xsl:call-template name="cap-first-char">
                    <xsl:with-param name="text" select="$camel-case-name"/>
                </xsl:call-template>
                <xsl:text> = </xsl:text>
                <xsl:call-template name="powers-of-two">
                    <xsl:with-param name="num" select="position() - 1"/>
                </xsl:call-template>
                <xsl:if test="position()!=last()">
                    <xsl:text>,</xsl:text>
                </xsl:if>
                <xsl:text>&endl;</xsl:text>

            </xsl:for-each>
            <xsl:text>    };&endl;</xsl:text>
        </xsl:if>
    </xsl:template>

<!-- Class declaration: attribute accessors -->

    <xsl:template name="attribute-accessors">
        <xsl:param name="node"/>

        <xsl:variable name="set" select="$node//xs:attribute"/>
        <xsl:variable name="count" select="count($set)"/>

        <xsl:if test="$count &gt; 0">
            <xsl:text>    // attribute accessors&endl;</xsl:text>
        </xsl:if>

        <xsl:for-each select="$set">
            <xsl:variable name="camel-case-name">
                <xsl:call-template name="camel-case">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cap-name">
                <xsl:call-template name="cap-first-char">
                    <xsl:with-param name="text" select="$camel-case-name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cpp-return-type">
                <xsl:call-template name="xs-type-to-cpp-return-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cpp-argument-type">
                <xsl:call-template name="xs-type-to-cpp-argument-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>    inline bool hasAttribute</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>() const { return m_has_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text>; }&endl;</xsl:text>

            <xsl:text>    inline </xsl:text>
            <xsl:value-of select="$cpp-return-type"/>
            <xsl:text>attribute</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>() const { return m_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text>; }&endl;</xsl:text>

            <xsl:text>    inline void setAttribute</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>(</xsl:text>
            <xsl:value-of select="$cpp-argument-type"/>
            <xsl:text>a) { m_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text> = a; m_has_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text> = true; }&endl;</xsl:text>

            <xsl:text>    inline void clearAttribute</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>() { m_has_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text> = false; }&endl;&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

<!-- Class declaration -->

    <xsl:template name="class-declaration">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>
        <xsl:variable name="hasText" select="$node[@mixed='true']"/>

        <xsl:text>class QDESIGNER_UILIB_EXPORT </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text> {&endl;    Q_DISABLE_COPY(</xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>)&endl;</xsl:text>
        <xsl:text>public:&endl;</xsl:text>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>() = default;&endl;</xsl:text>
        <xsl:text>    ~</xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>();&endl;&endl;</xsl:text>

        <xsl:text>    void read(QXmlStreamReader &amp;reader);&endl;</xsl:text>
        <xsl:text>    void write(QXmlStreamWriter &amp;writer, const QString &amp;tagName = QString()) const;&endl;&endl;</xsl:text>

        <xsl:if test="$hasText">
            <xsl:text>    inline QString text() const { return m_text; }&endl;</xsl:text>
            <xsl:text>    inline void setText(const QString &amp;s) { m_text = s; }&endl;&endl;</xsl:text>
        </xsl:if>

        <xsl:call-template name="attribute-accessors">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="child-element-accessors">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:text>private:&endl;</xsl:text>

        <xsl:if test="$hasText">
            <xsl:text>    QString m_text;&endl;&endl;</xsl:text>
        </xsl:if>

        <xsl:if test="boolean($node/xs:choice)">
            <xsl:text>    void clear();&endl;&endl;</xsl:text>
        </xsl:if>

        <xsl:variable name="set" select="$node//xs:attribute"/>
        <xsl:variable name="count" select="count($set)"/>

        <xsl:if test="$count &gt; 0">
            <xsl:text>    // attribute data&endl;</xsl:text>
        </xsl:if>
        <xsl:for-each select="$set">
            <xsl:variable name="camel-case-name">
                <xsl:call-template name="camel-case">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cpp-type">
                <xsl:call-template name="xs-type-to-cpp-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$cpp-type"/>
            <xsl:text> m_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>

            <xsl:choose>
                <xsl:when test="@type = 'xs:integer'">
                    <xsl:text> = 0</xsl:text>
                </xsl:when>
                <xsl:when test="@type = 'xs:double' or @type = 'xs:float'">
                    <xsl:text> = 0.0</xsl:text>
                </xsl:when>
                <xsl:when test="@type = 'xs:boolean'">
                    <xsl:text> = false</xsl:text>
                </xsl:when>
            </xsl:choose>

            <xsl:text>;&endl;</xsl:text>
            <xsl:text>    bool m_has_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text> = false;&endl;</xsl:text>
            <xsl:if test="position()!=last()">
                <xsl:text>&endl;</xsl:text>
            </xsl:if>
        </xsl:for-each>

        <xsl:call-template name="child-elements-data">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:text>};&endl;&endl;</xsl:text>
    </xsl:template>

<!-- Root -->

    <xsl:template match="xs:schema">

<xsl:text>@LICENSE@
//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

// THIS FILE IS AUTOMATICALLY GENERATED. DO NOT EDIT!

#ifndef UI4_H
#define UI4_H

#include &lt;qlist.h&gt;
#include &lt;qstring.h&gt;
#include &lt;qstringlist.h&gt;
#include &lt;qvector.h&gt;
#include &lt;qxmlstream.h&gt;
#include &lt;qglobal.h&gt;

QT_BEGIN_NAMESPACE

#define QDESIGNER_UILIB_EXTERN Q_DECL_EXPORT
#define QDESIGNER_UILIB_IMPORT Q_DECL_IMPORT

#if defined(QT_DESIGNER_STATIC) || defined(QT_UIC) || defined(QT_UIC3)
#  define QDESIGNER_UILIB_EXPORT
#elif defined(QDESIGNER_UILIB_LIBRARY)
#  define QDESIGNER_UILIB_EXPORT QDESIGNER_UILIB_EXTERN
#else
#  define QDESIGNER_UILIB_EXPORT QDESIGNER_UILIB_IMPORT
#endif

#ifndef QDESIGNER_UILIB_EXPORT
#    define QDESIGNER_UILIB_EXPORT
#endif

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

</xsl:text>

        <xsl:text>&endl;</xsl:text>
        <xsl:text>/*******************************************************************************&endl;</xsl:text>
        <xsl:text>** Forward declarations&endl;</xsl:text>
        <xsl:text>*/&endl;&endl;</xsl:text>

        <xsl:for-each select="xs:complexType">
            <xsl:call-template name="class-forward-declaration">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>&endl;</xsl:text>
        <xsl:text>/*******************************************************************************&endl;</xsl:text>
        <xsl:text>** Declarations&endl;</xsl:text>
        <xsl:text>*/&endl;&endl;</xsl:text>

        <xsl:for-each select="xs:complexType">
            <xsl:call-template name="class-declaration">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>
        <xsl:text>
#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

QT_END_NAMESPACE

#endif // UI4_H
</xsl:text>
    </xsl:template>
</xsl:stylesheet>
