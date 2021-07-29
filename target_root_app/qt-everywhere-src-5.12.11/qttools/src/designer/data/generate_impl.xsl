<!DOCTYPE xsl:stylesheet [
     <!ENTITY endl "&#10;">
]>
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xs="http://www.w3.org/2001/XMLSchema">

    <xsl:output method="text"/>

    <xsl:include href="generate_shared.xsl"/>

<!-- Implementation: constructor -->

    <xsl:template name="ctor-init-child-elements">
        <xsl:param name="node"/>
        <xsl:for-each select="$node/xs:element">
            <xsl:variable name="array" select="@maxOccurs='unbounded'"/>
            <xsl:if test="not($array)">
                <xsl:variable name="camel-case-name">
                    <xsl:call-template name="camel-case">
                         <xsl:with-param name="text" select="@name"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:choose>
                    <xsl:when test="@type = 'xs:integer' or @type = 'xs:unsignedInt' or @type = 'xs:long' or @type = 'xs:unsignedLong'">
                        <xsl:text>    m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text> = 0;&endl;</xsl:text>
                    </xsl:when>
                    <xsl:when test="@type = 'xs:double' or @type = 'xs:float'">
                         <xsl:text>    m_</xsl:text>
                         <xsl:value-of select="$camel-case-name"/>
                         <xsl:text> = 0.0;&endl;</xsl:text>
                    </xsl:when>
                    <xsl:when test="@type = 'xs:boolean'">
                        <xsl:text>    m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text> = false;&endl;</xsl:text>
                    </xsl:when>
                    <xsl:when test="@type = 'xs:string'">
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:text>    m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text> = nullptr;&endl;</xsl:text>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

<!-- Implementation: destructor -->

    <xsl:template name="dtor-delete-members">
        <xsl:param name="node"/>

        <xsl:for-each select="$node/xs:element[not(@use) or (@use!='deprecated')]">
            <xsl:variable name="camel-case-name">
                <xsl:call-template name="camel-case">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:choose>
                <xsl:when test="@maxOccurs='unbounded'">
                    <xsl:if test="$xs-type-cat = 'pointer'">
                        <xsl:text>    qDeleteAll(m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text>);&endl;</xsl:text>
                    </xsl:if>
                    <xsl:text>    m_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text>.clear();&endl;</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:if test="$xs-type-cat = 'pointer'">
                        <xsl:text>    delete m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text>;&endl;</xsl:text>
                    </xsl:if>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="dtor-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:value-of select="$name"/>
        <xsl:text>::~</xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>()</xsl:text>

        <!-- Collect the delete statements for the pointer members in a variable.
             If there are any, write a destructor body, else use "= default;" -->
        <xsl:variable name="dtor-body">
            <xsl:for-each select="$node//xs:sequence | $node//xs:choice | $node//xs:all">
                <xsl:call-template name="dtor-delete-members">
                    <xsl:with-param name="node" select="."/>
                </xsl:call-template>
            </xsl:for-each>
        </xsl:variable>
        <xsl:choose>
            <xsl:when test="$dtor-body != ''">
                <xsl:text>&endl;{&endl;</xsl:text>
                <xsl:value-of select="$dtor-body"/>
                <xsl:text>}&endl;&endl;</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text> = default;&endl;&endl;</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

<!-- Implementation: clear() -->

    <xsl:template name="clear-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:if test="boolean($node/xs:choice)">
            <xsl:text>void </xsl:text><xsl:value-of select="$name"/>
            <xsl:text>::clear()&endl;</xsl:text>
            <xsl:text>{&endl;</xsl:text>

            <xsl:variable name="set" select="$node//xs:sequence | $node//xs:choice | $node//xs:all"/>
            <xsl:variable name="count" select="count($set)"/>
            <xsl:for-each select="$set">
                <xsl:call-template name="dtor-delete-members">
                    <xsl:with-param name="node" select="."/>
                </xsl:call-template>
            </xsl:for-each>

            <xsl:if test="$count &gt; 0">
                <xsl:text>&endl;</xsl:text>
            </xsl:if>

            <xsl:if test="boolean($node/xs:choice)">
                <xsl:text>    m_kind = Unknown;&endl;&endl;</xsl:text>
            </xsl:if>

            <xsl:if test="not($node/xs:choice)">
                <xsl:if test="$count &gt; 0">
                    <xsl:text>    m_children = 0;&endl;</xsl:text>
                </xsl:if>
            </xsl:if>

            <xsl:for-each select="$set">
                <xsl:call-template name="ctor-init-child-elements">
                    <xsl:with-param name="node" select="."/>
                </xsl:call-template>
            </xsl:for-each>

            <xsl:text>}&endl;&endl;</xsl:text>
        </xsl:if>

    </xsl:template>

    <!-- Format a string constant for comparison as QLatin1String("foo") - they're all ascii-only -->
    <xsl:template name="string-constant-for-comparison">
        <xsl:param name="literal"/>
        <xsl:text>QLatin1String("</xsl:text>
        <xsl:value-of select="$literal"/>
        <xsl:text>")</xsl:text>
    </xsl:template>

    <!-- Format a string constant for storage as QString(QLatin1Char('X')) or QLatin1String("foo"), respectively -->
    <xsl:template name="string-constant-for-storage">
    <xsl:param name="literal"/>
        <xsl:choose>
            <xsl:when test="string-length($literal) &lt; 2">
                  <xsl:text>QString(QLatin1Char('</xsl:text>
                <xsl:value-of select="$literal"/>
                <xsl:text>'))</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>QStringLiteral("</xsl:text>
                    <xsl:value-of select="$literal"/>
                <xsl:text>")</xsl:text>
           </xsl:otherwise>
       </xsl:choose>
    </xsl:template>

<!-- Implementation: read(QXmlStreamReader) -->

    <xsl:template name="read-impl-load-attributes">
        <xsl:param name="node"/>

        <xsl:if test="$node/xs:attribute">
            <xsl:text>    const QXmlStreamAttributes &amp;attributes = reader.attributes();&endl;</xsl:text>
            <xsl:text>    for (const QXmlStreamAttribute &amp;attribute : attributes) {&endl;</xsl:text>
            <xsl:text>        const QStringRef name = attribute.name();&endl;</xsl:text>

            <xsl:for-each select="$node/xs:attribute">
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
                <xsl:variable name="qstring-func">
                    <xsl:call-template name="xs-type-from-qstringref-func">
                        <xsl:with-param name="xs-type" select="@type"/>
                        <xsl:with-param name="val">
                           <xsl:text>attribute.value()</xsl:text>
                        </xsl:with-param>
                    </xsl:call-template>
                </xsl:variable>

                <xsl:text>        if (name == </xsl:text>
                <xsl:call-template name="string-constant-for-comparison">
                    <xsl:with-param name="literal" select="@name"/>
                </xsl:call-template>
                <xsl:text>) {&endl;</xsl:text>
                <xsl:text>            setAttribute</xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>(</xsl:text>
                <xsl:value-of select="$qstring-func"/>
                <xsl:text>);&endl;</xsl:text>
                <xsl:text>            continue;&endl;</xsl:text>
                <xsl:text>        }&endl;</xsl:text>
            </xsl:for-each>

            <xsl:text>        reader.raiseError(QLatin1String("Unexpected attribute ") + name);&endl;</xsl:text>
            <xsl:text>    }&endl;</xsl:text>
            <xsl:text>&endl;</xsl:text>
        </xsl:if>
    </xsl:template>

    <xsl:template name="read-impl-load-child-element">
        <xsl:param name="node"/>

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
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="array" select="@maxOccurs = 'unbounded'"/>

            <xsl:text>            if (!tag.compare(</xsl:text>
            <xsl:call-template name="string-constant-for-comparison">
                <xsl:with-param name="literal" select="$lower-name"/>
            </xsl:call-template>
            <xsl:text>, Qt::CaseInsensitive)) {&endl;</xsl:text>

            <xsl:choose>
                <xsl:when test="@use='deprecated'">
                    <xsl:text>                qWarning("Omitting deprecated element &lt;</xsl:text>
                    <xsl:value-of select="$lower-name"/>
                    <xsl:text>&gt;.");&endl;</xsl:text>
                    <xsl:text>                reader.skipCurrentElement();&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="not($array) and $xs-type-cat = 'value'">
                    <xsl:variable name="qstring-func">
                        <xsl:call-template name="xs-type-from-qstring-func">
                            <xsl:with-param name="xs-type" select="@type"/>
                            <xsl:with-param name="val" select="'reader.readElementText()'"/>
                        </xsl:call-template>
                    </xsl:variable>

                    <xsl:text>                setElement</xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>(</xsl:text>
                    <xsl:value-of select="$qstring-func"/>
                    <xsl:text>);&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="@maxOccurs='unbounded' and $xs-type-cat = 'value'">
                    <xsl:variable name="qstring-func">
                        <xsl:call-template name="xs-type-from-qstring-func">
                            <xsl:with-param name="xs-type" select="@type"/>
                            <xsl:with-param name="val" select="'reader.readElementText()'"/>
                        </xsl:call-template>
                    </xsl:variable>

                    <xsl:text>                m_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text>.append(</xsl:text>
                    <xsl:value-of select="$qstring-func"/>
                    <xsl:text>);&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="not(@maxOccurs='unbounded') and $xs-type-cat = 'pointer'">
                    <xsl:text>                auto</xsl:text>
                    <xsl:text> *v = new Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text>();&endl;</xsl:text>
                    <xsl:text>                v->read(reader);&endl;</xsl:text>
                    <xsl:text>                setElement</xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>(v);&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="@maxOccurs='unbounded' and $xs-type-cat = 'pointer'">
                    <xsl:text>                auto</xsl:text>
                    <xsl:text> *v = new Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text>();&endl;</xsl:text>
                    <xsl:text>                v->read(reader);&endl;</xsl:text>
                    <xsl:text>                m_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text>.append(v);&endl;</xsl:text>
                </xsl:when>
            </xsl:choose>
            <xsl:text>                continue;&endl;</xsl:text>
            <xsl:text>            }&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="read-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:text>void </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>::read(QXmlStreamReader &amp;reader)&endl;</xsl:text>

        <xsl:text>{&endl;</xsl:text>

        <xsl:call-template name="read-impl-load-attributes">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:text>    while (!reader.hasError()) {&endl;</xsl:text>
        <xsl:text>        switch (reader.readNext()) {&endl;</xsl:text>
        <xsl:text>        case QXmlStreamReader::StartElement : {&endl;</xsl:text>
        <xsl:text>            const QStringRef tag = reader.name();&endl;</xsl:text>

        <xsl:for-each select="$node//xs:sequence | $node//xs:choice | $node//xs:all">
            <xsl:call-template name="read-impl-load-child-element">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>            reader.raiseError(QLatin1String("Unexpected element ") + tag);&endl;</xsl:text>
        <xsl:text>        }&endl;</xsl:text>
        <xsl:text>            break;&endl;</xsl:text>
        <xsl:text>        case QXmlStreamReader::EndElement :&endl;</xsl:text>
        <xsl:text>            return;&endl;</xsl:text>

        <xsl:variable name="hasText" select="$node[@mixed='true']"/>
        <xsl:if test="$hasText">
            <xsl:text>        case QXmlStreamReader::Characters :&endl;</xsl:text>
            <xsl:text>            if (!reader.isWhitespace())&endl;</xsl:text>
            <xsl:text>                m_text.append(reader.text().toString());&endl;</xsl:text>
            <xsl:text>            break;&endl;</xsl:text>
        </xsl:if>

        <xsl:text>        default :&endl;</xsl:text>
        <xsl:text>            break;&endl;</xsl:text>
        <xsl:text>        }&endl;</xsl:text>
        <xsl:text>    }&endl;</xsl:text>
        <xsl:text>}&endl;&endl;</xsl:text>
    </xsl:template>

<!-- Implementation: write() -->

    <xsl:template name="write-impl-save-attributes">
        <xsl:param name="node"/>

        <xsl:for-each select="$node/xs:attribute">
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
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>    if (hasAttribute</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>())&endl;</xsl:text>
            <xsl:text>        writer.writeAttribute(</xsl:text>
            <xsl:call-template name="string-constant-for-storage">
                <xsl:with-param name="literal" select="$lower-name"/>
            </xsl:call-template>

            <xsl:text>, </xsl:text>

            <xsl:call-template name="xs-type-to-qstring-func">
                <xsl:with-param name="xs-type" select="@type"/>
                <xsl:with-param name="val" select="concat('attribute', $cap-name, '()')"/>
            </xsl:call-template>

            <xsl:text>);&endl;&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="write-impl-save-choice-child-element">
        <xsl:param name="node"/>
        <xsl:variable name="have-kind" select="name($node) = 'xs:choice'"/>

        <xsl:text>    switch (kind()) {&endl;</xsl:text>

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
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>    case </xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>:&endl;</xsl:text>
                <xsl:choose>
                    <xsl:when test="$xs-type-cat = 'value'">
                        <xsl:variable name="qstring-func">
                            <xsl:call-template name="xs-type-to-qstring-func">
                                <xsl:with-param name="xs-type" select="@type"/>
                                <xsl:with-param name="val" select="concat('element', $cap-name, '()')"/>
                            </xsl:call-template>
                        </xsl:variable>

                        <xsl:text>        writer.writeTextElement(</xsl:text>
                        <xsl:call-template name="string-constant-for-storage">
                            <xsl:with-param name="literal" select="$camel-case-name"/>
                        </xsl:call-template>
                        <xsl:text>, </xsl:text>
                        <xsl:value-of select="$qstring-func"/>
                        <xsl:text>);&endl;</xsl:text>
                    </xsl:when>
                    <xsl:when test="$xs-type-cat = 'pointer'">
                        <xsl:variable name="cpp-return-type">
                            <xsl:call-template name="xs-type-to-cpp-return-type">
                                <xsl:with-param name="xs-type" select="@type"/>
                            </xsl:call-template>
                        </xsl:variable>

                        <xsl:text>        if (m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text> != nullptr)&endl;</xsl:text>
                        <xsl:text>            m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text>->write(writer, </xsl:text>
                        <xsl:call-template name="string-constant-for-storage">
                            <xsl:with-param name="literal" select="$lower-name"/>
                        </xsl:call-template>
                        <xsl:text>);&endl;</xsl:text>
                    </xsl:when>
                </xsl:choose>
            <xsl:text>        break;&endl;</xsl:text>
            <xsl:text>&endl;</xsl:text>
        </xsl:for-each>

        <xsl:text>    default:&endl;</xsl:text>
        <xsl:text>        break;&endl;</xsl:text>
        <xsl:text>    }&endl;</xsl:text>
    </xsl:template>

    <xsl:template name="write-impl-save-sequence-child-element">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>
        <xsl:for-each select="$node/xs:element[not(@use) or (@use!='deprecated')]">
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
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cpp-argument-type">
                <xsl:call-template name="xs-type-to-cpp-argument-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:choose>
                <xsl:when test="@maxOccurs='unbounded'">
                    <xsl:text>    for (</xsl:text>
                    <xsl:value-of select="$cpp-argument-type"/>
                    <xsl:text>v : m_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text>)&endl;</xsl:text>
                    <xsl:choose>
                        <xsl:when test="$xs-type-cat = 'pointer'">
                            <xsl:text>        v->write(writer, </xsl:text>
                            <xsl:call-template name="string-constant-for-storage">
                                <xsl:with-param name="literal" select="$lower-name"/>
                            </xsl:call-template>
                            <xsl:text>);&endl;</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:variable name="qstring-func">
                                <xsl:call-template name="xs-type-to-qstring-func">
                                    <xsl:with-param name="xs-type" select="@type"/>
                                    <xsl:with-param name="val" select="'v'"/>
                                </xsl:call-template>
                            </xsl:variable>

                            <xsl:text>        writer.writeTextElement(</xsl:text>
                            <xsl:call-template name="string-constant-for-storage">
                                <xsl:with-param name="literal" select="$lower-name"/>
                            </xsl:call-template>
                            <xsl:text>, </xsl:text>
                            <xsl:value-of select="$qstring-func"/>
                            <xsl:text>);&endl;</xsl:text>
                        </xsl:otherwise>
                    </xsl:choose>
                    <xsl:text>&endl;</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>    if (m_children &amp; </xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>)&endl;</xsl:text>
                    <xsl:choose>
                        <xsl:when test="$xs-type-cat = 'pointer'">
                            <xsl:text>        m_</xsl:text>
                            <xsl:value-of select="$camel-case-name"/>
                            <xsl:text>->write(writer, </xsl:text>
                            <xsl:call-template name="string-constant-for-storage">
                                <xsl:with-param name="literal" select="$lower-name"/>
                            </xsl:call-template>
                            <xsl:text>);&endl;</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:variable name="qstring-func">
                                <xsl:call-template name="xs-type-to-qstring-func">
                                    <xsl:with-param name="xs-type" select="@type"/>
                                    <xsl:with-param name="val" select="concat('m_', $camel-case-name)"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:text>        writer.writeTextElement(</xsl:text>
                            <xsl:call-template name="string-constant-for-storage">
                                <xsl:with-param name="literal" select="$lower-name"/>
                            </xsl:call-template>
                            <xsl:text>, </xsl:text>
                            <xsl:value-of select="$qstring-func"/>
                            <xsl:text>);&endl;</xsl:text>
                        </xsl:otherwise>
                    </xsl:choose>
                    <xsl:text>&endl;</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="write-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>
        <xsl:variable name="lower-name">
            <xsl:call-template name="lower-text">
                <xsl:with-param name="text" select="@name"/>
            </xsl:call-template>
        </xsl:variable>

        <xsl:text>void </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>::write(QXmlStreamWriter &amp;writer, const QString &amp;tagName) const&endl;</xsl:text>
        <xsl:text>{&endl;</xsl:text>

        <xsl:text>    writer.writeStartElement(tagName.isEmpty() ? QStringLiteral("</xsl:text>
        <xsl:value-of select="$lower-name"/>
        <xsl:text>") : tagName.toLower());&endl;&endl;</xsl:text>

        <xsl:call-template name="write-impl-save-attributes">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:for-each select="$node//xs:choice">
            <xsl:call-template name="write-impl-save-choice-child-element">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:for-each select="$node//xs:sequence | $node//xs:all">
            <xsl:call-template name="write-impl-save-sequence-child-element">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:variable name="hasText" select="$node[@mixed='true']"/>
        <xsl:if test="$hasText">
            <xsl:text>    if (!m_text.isEmpty())&endl;</xsl:text>
            <xsl:text>        writer.writeCharacters(m_text);&endl;&endl;</xsl:text>
        </xsl:if>

        <xsl:text>    writer.writeEndElement();&endl;</xsl:text>
        <xsl:text>}&endl;&endl;</xsl:text>
    </xsl:template>

<!-- Implementation: child element setters -->

    <xsl:template name="child-setter-impl-helper">
        <xsl:param name="node"/>
        <xsl:param name="name"/>
        <xsl:variable name="isChoice" select="name($node)='xs:choice'"/>

        <xsl:for-each select="$node/xs:element[not(@use) or (@use!='deprecated')]">
            <xsl:variable name="array" select="@maxOccurs = 'unbounded'"/>
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

            <xsl:if test="$xs-type-cat = 'pointer'">
                <xsl:value-of select="$return-cpp-type"/>
                <xsl:value-of select="$name"/>
                <xsl:text>::takeElement</xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>()&endl;{&endl;</xsl:text>
                <xsl:text>    </xsl:text>
                <xsl:value-of select="$return-cpp-type"/>
                <xsl:text>a = m_</xsl:text>
                <xsl:value-of select="$camel-case-name"/>
                <xsl:text>;&endl;</xsl:text>
                <xsl:text>    m_</xsl:text>
                <xsl:value-of select="$camel-case-name"/>
                <xsl:text> = nullptr;&endl;</xsl:text>
                <xsl:if test="not($isChoice)">
                    <xsl:text>    m_children ^= </xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>;&endl;</xsl:text>
                </xsl:if>
                <xsl:text>    return a;&endl;</xsl:text>
                <xsl:text>}&endl;&endl;</xsl:text>
            </xsl:if>

            <xsl:text>void </xsl:text>
            <xsl:value-of select="$name"/>
            <xsl:text>::setElement</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>(</xsl:text>
            <xsl:value-of select="$argument-cpp-type"/>
            <xsl:text>a)&endl;</xsl:text>
            <xsl:text>{&endl;</xsl:text>
            <xsl:choose>
                <xsl:when test="$isChoice">
                    <xsl:text>    clear();&endl;</xsl:text>
                    <xsl:text>    m_kind = </xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>;&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="$xs-type-cat = 'pointer'">
                    <xsl:text>    delete </xsl:text>
                    <xsl:text>m_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text>;&endl;</xsl:text>
                </xsl:when>
            </xsl:choose>
            <xsl:if test="not($isChoice)">
                <xsl:text>    m_children |= </xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>;&endl;</xsl:text>
            </xsl:if>
            <xsl:text>    m_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text> = a;&endl;</xsl:text>
            <xsl:text>}&endl;&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="child-setter-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:for-each select="$node/xs:sequence | $node/xs:choice | $node/xs:all">
            <xsl:call-template name="child-setter-impl-helper">
                <xsl:with-param name="node" select="."/>
                <xsl:with-param name="name" select="$name"/>
            </xsl:call-template>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="child-clear-impl">
        <xsl:param name="node"/>

       <xsl:variable name="name" select="concat('Dom', @name)"/>
       <xsl:for-each select="$node/xs:sequence | $node/xs:choice | $node/xs:all">
            <xsl:variable name="isChoice" select="name()='xs:choice'"/>
            <xsl:variable name="make-child-enum" select="boolean(xs:sequence) and not(@maxOccurs='unbounded')"/>

            <xsl:for-each select="xs:element[not(@use) or (@use!='deprecated')]">
                <xsl:if test="not($isChoice) and not(@maxOccurs='unbounded')">
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
                    <xsl:variable name="xs-type-cat">
                        <xsl:call-template name="xs-type-category">
                            <xsl:with-param name="xs-type" select="@type"/>
                            <xsl:with-param name="array" select="@maxOccurs='unbounded'"/>
                        </xsl:call-template>
                    </xsl:variable>

                    <xsl:text>void </xsl:text>
                    <xsl:value-of select="$name"/>
                    <xsl:text>::clearElement</xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>()&endl;</xsl:text>
                    <xsl:text>{&endl;</xsl:text>
                    <xsl:if test="$xs-type-cat = 'pointer'">
                        <xsl:text>    delete m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text>;&endl;</xsl:text>
                        <xsl:text>    m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text> = nullptr;&endl;</xsl:text>
                    </xsl:if>
                    <xsl:text>    m_children &amp;= ~</xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>;&endl;</xsl:text>
                    <xsl:text>}&endl;&endl;</xsl:text>
                </xsl:if>
            </xsl:for-each>
        </xsl:for-each>
    </xsl:template>


<!-- Implementation -->

    <xsl:template name="class-implementation">
        <xsl:param name="node"/>

        <xsl:call-template name="dtor-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="clear-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="read-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="write-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="child-setter-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="child-clear-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

    </xsl:template>

<!-- Root -->

    <xsl:template match="xs:schema">

<xsl:text>@LICENSE@
// THIS FILE IS AUTOMATICALLY GENERATED. DO NOT EDIT!

</xsl:text>
        <xsl:text>#include "@HEADER@"&endl;</xsl:text>
        <xsl:text>&endl;</xsl:text>
        <xsl:text>&endl;</xsl:text>
        <xsl:text>QT_BEGIN_NAMESPACE&endl;</xsl:text>

        <xsl:text>#ifdef QFORMINTERNAL_NAMESPACE&endl;</xsl:text>
        <xsl:text>using namespace QFormInternal;&endl;</xsl:text>
        <xsl:text>#endif&endl;</xsl:text>
        <xsl:text>&endl;</xsl:text>

        <xsl:text>/*******************************************************************************&endl;</xsl:text>
        <xsl:text>** Implementations&endl;</xsl:text>
        <xsl:text>*/&endl;&endl;</xsl:text>

        <xsl:for-each select="xs:complexType">
            <xsl:call-template name="class-implementation">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>
        <xsl:text>QT_END_NAMESPACE&endl;</xsl:text>

        <xsl:text>&endl;</xsl:text>
    </xsl:template>

</xsl:stylesheet>
