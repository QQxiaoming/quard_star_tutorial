<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:test="http://www.w3.org/2005/02/query-test-XQTSCatalog">
   <xsl:key name="citation-spec" match="/test-suite/citations/citation-spec" use="@name"/>

      <xsl:variable name="TargetLanguage" select="/test:test-suite/@targetLanguage" />
      <xsl:variable name="XQueryQueryOffsetPath" select="/test:test-suite/@XQueryQueryOffsetPath" />
      <xsl:variable name="XQueryXQueryOffsetPath" select="/test:test-suite/@XQueryXQueryOffsetPath" />
      <xsl:variable name="ResultOffsetPath" select="/test:test-suite/@ResultOffsetPath" />
      <xsl:variable name="XQueryFileExtension" select="/test:test-suite/@XQueryFileExtension" />
      <xsl:variable name="XQueryXFileExtension" select="/test:test-suite/@XQueryXFileExtension" />
      <xsl:variable name="SourceOffsetPath" select="/test:test-suite/@SourceOffsetPath" />

   <xsl:template match="/">
      <html>
         <head>
            <title>XQuery Interoperability Test Suite</title>
         </head>
         <body>
            <xsl:if test="test:test-suite/@targetLanguage">
               <h1><xsl:value-of select="test:test-suite/@targetLanguage" /> Interoperability Test Suite</h1>
            </xsl:if>
            <xsl:if test="not( test:test-suite/@targetLanguage )">
               <h1>XQuery/XQueryX Interoperability Test Suite</h1>
            </xsl:if>
            <h6>[Version: <xsl:value-of select="test:test-suite/@version" /> Catalog Design Date: <xsl:value-of select="test:test-suite/@CatalogDesignDate" />]</h6>
            <h2>Introduction</h2>
            <P>
               The following document is an XSLT transform of the XQuery Testing Task Force (XQTTF) test catalog. The test 
               catalog is an XML file containing meta-data about the XQuery interoperability 
               tests published by the XQTTF. The meta-data serves to link test queries with 
               their required input sources and results as well as providing further 
               information about the tests (e.g. who the author was, what the purpose of the 
               test is) and categorizing the various tests into their relevant sections. For 
               further information and details about this catalog file, please visit the XQTTF 
               homepage at <A HREF="http://www.w3.org/XML/Query/test-suite/">http://www.w3.org/XML/Query/test-suite/</A>.
            </P>
            <h2>Citations</h2>
            <table>
               <tr>
                  <th>Name</th>
                  <th>Description</th>
                  <th>Note</th>
               </tr>
               <xsl:apply-templates select="test:test-suite/test:citations/test:citation-spec" />
            </table>
            <h2>Comparators</h2>
            <table>
               <tr>
                  <th>Name</th>
                  <th>Description</th>
               </tr>
               <xsl:for-each select="test:test-suite/test:comparisons/test:comparison">
                  <tr>
                     <td>
                        <A>
                           <xsl:attribute name="name">
                              <xsl:value-of select="@name"/>
                           </xsl:attribute>
                           <b>
                              <xsl:value-of select="@name"/>
                           </b>
                        </A>
                     </td>
                     <td>
                        <xsl:value-of select="test:description"/>
                     </td>
                  </tr>
               </xsl:for-each>
            </table>
            <h2>Scenarios</h2>
            <table>
               <tr>
                  <th>Name</th>
                  <th>Description</th>
               </tr>
               <xsl:for-each select="test:test-suite/test:scenarios/test:scenario">
                  <tr>
                     <td>
                        <A>
                           <xsl:attribute name="name">
                              <xsl:value-of select="@name"/>
                           </xsl:attribute>
                           <b>
                              <xsl:value-of select="@name"/>
                           </b>
                        </A>
                     </td>
                     <td>
                        <xsl:value-of select="test:description"/>
                     </td>
                  </tr>
               </xsl:for-each>
            </table>
            <h2>Roles</h2>
            <table>
               <tr>
                  <th>Name</th>
                  <th>Description</th>
               </tr>
               <xsl:for-each select="test:test-suite/test:roles/test:role">
                  <tr>
                     <td>
                        <A>
                           <xsl:attribute name="name">
                              <xsl:value-of select="@name"/>
                           </xsl:attribute>
                           <b>
                              <xsl:value-of select="@name"/>
                           </b>
                        </A>
                     </td>
                     <td>
                        <xsl:value-of select="test:description"/>
                     </td>
                  </tr>
               </xsl:for-each>
            </table>
            <h2>Sources</h2>
            <table>
               <tr>
                  <th>ID</th>
                  <th>Creator</th>
                  <th>Schema</th>
                  <th>File</th>
                  <th>Description</th>
               </tr>
               <xsl:for-each select="test:test-suite/test:sources/test:source">
                  <tr>
                     <td>
                        <A>
                           <xsl:attribute name="name">
                              <xsl:value-of select="@ID"/>
                           </xsl:attribute>
                           <b>
                              <xsl:value-of select="@ID"/>
                           </b>
                        </A>
                     </td>
                     <td>
                        <xsl:value-of select="@Creator"/>
                     </td>
                     <td>
                        <A>
                           <xsl:attribute name="href">#<xsl:value-of select="@schema"/></xsl:attribute>
                           <xsl:value-of select="@schema"/>								
                        </A>
                     </td>
                     <td>
                        <A>
                           <xsl:attribute name="href">
                              <xsl:value-of select="$SourceOffsetPath"/><xsl:value-of select="@FileName"/></xsl:attribute>
                           <xsl:value-of select="@FileName"/>
                        </A>
                     </td>
                     <td>
                        <xsl:value-of select="test:description"/>
                     </td>
                  </tr>
               </xsl:for-each>
            </table>
            <h2>Schemas</h2>
            <table>
               <tr>
                  <th>ID</th>
                  <th>URI</th>
                  <th>File Name</th>
                  <th>Description</th>
               </tr>
               <xsl:for-each select="test:test-suite/test:sources/test:schema">
                  <tr>
                     <td>
                        <A>
                           <xsl:attribute name="name">
                              <xsl:value-of select="@ID"/>
                           </xsl:attribute>
                           <b>
                              <xsl:value-of select="@ID"/>
                           </b>
                        </A>
                     </td>
                     <td>
                        <xsl:value-of select="@uri"/>
                     </td>
                     <td>
                        <A>
                           <xsl:attribute name="href"><xsl:value-of select="$SourceOffsetPath"/><xsl:value-of select="@FileName"/></xsl:attribute>
                           <xsl:value-of select="@FileName"/>
                        </A>
                     </td>
                     <td>
                        <xsl:value-of select="test:description"/>
                     </td>
                  </tr>
               </xsl:for-each>
            </table>
            <h2>Collections</h2>
            <table>
               <tr>
                  <th>ID</th>
                  <th>Creator</th>
                  <th>Contents</th>
                  <th>Description</th>
               </tr>
               <xsl:for-each select="test:test-suite/test:sources/test:collection">
                  <tr>
                     <td>
                        <A>
                           <xsl:attribute name="name">
                              <xsl:value-of select="@ID"/>
                           </xsl:attribute>
                           <b>
                              <xsl:value-of select="@ID"/>
                           </b>
                        </A>
                     </td>
                     <td>
                        <xsl:value-of select="@Creator"/>
                     </td>
                     <td>
                        <xsl:for-each select="test:input-document">
                           <A>
                              <xsl:attribute name="href">#<xsl:value-of select="."/></xsl:attribute>
                              <xsl:value-of select="."/>
                           </A>
			   <xsl:if test="position() != last()">
			      <xsl:text>, </xsl:text>
			   </xsl:if>
                        </xsl:for-each>
                     </td>
                     <td>
                        <xsl:value-of select="test:description"/>
                     </td>
                  </tr>
               </xsl:for-each>
            </table>
            <h2>Modules</h2>
            <table>
               <tr>
                  <th>ID</th>
                  <th>Creator</th>
                  <th>File</th>
                  <th>Description</th>
               </tr>
               <xsl:for-each select="test:test-suite/test:sources/test:module">
                  <tr>
                     <td>
                        <A>
                           <xsl:attribute name="name">
                              <xsl:value-of select="@ID"/>
                           </xsl:attribute>
                           <b>
                              <xsl:value-of select="@ID"/>
                           </b>
                        </A>
                     </td>
                     <td>
                        <xsl:value-of select="@Creator"/>
                     </td>
                     <td>
                        <A>
                           <xsl:attribute name="href">
                              <xsl:value-of select="$SourceOffsetPath"/><xsl:value-of select="@FileName"/><xsl:value-of select="$XQueryFileExtension"/>
                           </xsl:attribute>
                           <xsl:value-of select="@FileName"/>
                        </A>
                     </td>
                     <td>
                        <xsl:value-of select="test:description"/>
                     </td>
                  </tr>
               </xsl:for-each>
            </table>
            <h2>Test Groups</h2>
            <ul>
               <xsl:for-each select="test:test-suite/test:test-group">
                  <li>
                     <A>
                        <xsl:attribute name="href">#<xsl:value-of select="@name"/></xsl:attribute>
                        <xsl:number level="multiple" format="1.1" count="test:test-group"/>&#x20;<xsl:value-of select="test:GroupInfo/test:title"/></A>
                     <xsl:if test="@featureOwner">
                        &#x20;[<xsl:value-of select="@featureOwner" />]
                     </xsl:if>
                     <xsl:if test="test:test-group">
                        <ul>
                           <xsl:for-each select="test:test-group">
                              <li>
                                 <A>
                                    <xsl:attribute name="href">#<xsl:value-of select="@name"/></xsl:attribute>
                                    <xsl:number level="multiple" format="1.1" count="test:test-group"/>&#x20;<xsl:value-of select="test:GroupInfo/test:title"/></A>
                                 <xsl:if test="@featureOwner">
                                    &#x20;[<xsl:value-of select="@featureOwner" />]
                                 </xsl:if>
                              </li>
                           </xsl:for-each>
                        </ul>
                     </xsl:if>
                  </li>
               </xsl:for-each>
            </ul>
            <xsl:apply-templates select="test:test-suite/test:test-group" />
            <h2>Appendices</h2>
            <h3>Implementation Defined Items</h3>
            <blockquote>
               <table border="1">
                  <xsl:apply-templates select="test:test-suite/test:implementation-defined-items/test:implementation-defined-item" />
               </table>
            </blockquote>
            <h3>Context Properties</h3>
            <xsl:apply-templates select="test:test-suite/test:context-properties" />
            <h3>Features</h3>
            <table>
               <xsl:apply-templates select="test:test-suite/test:features/test:feature" />
            </table>
         </body>
      </html>
   </xsl:template>
   <xsl:template match="test:test-group">
      <A>
         <xsl:attribute name="name">
            <xsl:value-of select="@name"/>
         </xsl:attribute>
      </A>
      <h3>
         <xsl:number level="multiple" format="1.1" count="test:test-group"/>&#x20;<xsl:value-of select="test:GroupInfo/test:title" />
         <xsl:if test="@featureOwner">
            &#x20;[<xsl:value-of select="@featureOwner" />]
         </xsl:if>
      </h3>
      <p/>
      <xsl:value-of select="test:GroupInfo/test:description" />
      <p/>
      <ul>
         <xsl:for-each select="test:test-group">
            <li>
               <A>
                  <xsl:attribute name="href">#<xsl:value-of select="@name"/></xsl:attribute>
                  <xsl:number level="multiple" format="1.1" count="test:test-group"/>&#x20;<xsl:value-of select="test:GroupInfo/test:title"/></A>
               <xsl:if test="@featureOwner">
                  &#x20;[<xsl:value-of select="@featureOwner" />]
               </xsl:if>
               <xsl:if test="test:test-group">
                  <ul>
                     <xsl:for-each select="test:test-group">
                        <li>
                           <A>
                              <xsl:attribute name="href">#<xsl:value-of select="@name"/></xsl:attribute>
                              <xsl:number level="multiple" format="1.1" count="test:test-group"/>&#x20;<xsl:value-of select="test:GroupInfo/test:title"/></A>
                           <xsl:if test="@featureOwner">
                              &#x20;[<xsl:value-of select="@featureOwner" />]
                           </xsl:if>
                        </li>
                     </xsl:for-each>
                  </ul>
               </xsl:if>
            </li>
         </xsl:for-each>
      </ul>
      <xsl:apply-templates select="test:test-case" />
      <xsl:apply-templates select="test:test-group" />
   </xsl:template>

   <xsl:template match="test:test-case">
      <xsl:variable name="FilePath" select="@FilePath" />
      <table width="80%" bgcolor="tan">
         <a name="{@name}"/>
         <tr>
            <td>
               <b>
                  <xsl:value-of select="@name"/>
               </b>
            </td>
            <td>
               Scenario:
               <A>
                  <xsl:attribute name="href">#<xsl:value-of select="@scenario"/></xsl:attribute>
                  <xsl:value-of select="@scenario"/>
               </A>
            </td>
            <td>
               Creator:
               <xsl:value-of select="@Creator"/>
            </td>
         </tr>
         <tr>
            <td colspan="3">
               <B>Description:</B>
            </td>
         </tr>
         <tr>
            <td colspan="3">
               <xsl:value-of select="test:description"/>
            </td>
         </tr>
         <tr>
            <td colspan="3">
               <xsl:value-of select="test:query/test:description"/>
            </td>
         </tr>
         <tr>
            <td colspan="3">
               <B>Spec Citations:</B>
            </td>
         </tr>
         <xsl:for-each select="test:spec-citation">
            <xsl:variable name="spec-URI" select="key( 'citation-spec', @spec )/test:spec-URI" />
            <tr>
               <td colspan="3">
                  <A>
                     <xsl:attribute name="href">
                        <xsl:value-of select="$spec-URI"/>#<xsl:value-of select="@section-pointer"/>
                     </xsl:attribute>
                     <xsl:value-of select="$spec-URI"/>#<xsl:value-of select="@section-pointer"/>
                  </A>
               </td>
            </tr>
         </xsl:for-each>
         
         <tr>
            <td colspan="3">
               <B>Reflects Resolution:</B>
            </td>
         </tr>
         <xsl:for-each select="test:reflects-resolution">
            <tr>
               <td colspan="3">
                  <A>
                     <xsl:attribute name="href">
                        <xsl:value-of select="."/>
                     </xsl:attribute>
                     <xsl:value-of select="substring-after(., '?id=')"/>
                  </A>
               </td>
            </tr>
         </xsl:for-each>
         
         <xsl:if test="test:input-query">
            <tr>
               <td colspan="3">
                  <B>External Variables:</B>
               </td>
            </tr>
            <xsl:for-each select="test:input-query">
               <tr>
                  <td>
                     <xsl:text>Variable: </xsl:text>
                     <xsl:value-of select="@variable"/>
                  </td>
                  <td colspan="2">
                     <A>
                        <xsl:choose>
                           <xsl:when test="$TargetLanguage='XQuery'">
                              <xsl:attribute name="href">
                                 <xsl:value-of select="$XQueryQueryOffsetPath"/><xsl:value-of select="$FilePath"/><xsl:value-of select="@name"/><xsl:value-of select="$XQueryFileExtension"/>
                              </xsl:attribute>
                              <xsl:value-of select="$XQueryQueryOffsetPath"/><xsl:value-of select="$FilePath"/><xsl:value-of select="@name"/><xsl:value-of select="$XQueryFileExtension"/>
                           </xsl:when>
                           <xsl:when test="$TargetLanguage='XQueryX'">
                              <xsl:attribute name="href">
                                 <xsl:value-of select="$XQueryXQueryOffsetPath"/><xsl:value-of select="$FilePath"/><xsl:value-of select="@name"/><xsl:value-of select="$XQueryXFileExtension"/>
                              </xsl:attribute>
                              <xsl:value-of select="$XQueryXQueryOffsetPath"/><xsl:value-of select="$FilePath"/><xsl:value-of select="@name"/><xsl:value-of select="$XQueryXFileExtension"/>
                           </xsl:when>
                           <xsl:otherwise>
                              <xsl:attribute name="href">
                                 <xsl:value-of select="$XQueryQueryOffsetPath"/><xsl:value-of select="$FilePath"/><xsl:value-of select="@name"/><xsl:value-of select="$XQueryFileExtension"/>
                              </xsl:attribute>
                              <xsl:value-of select="$XQueryQueryOffsetPath"/><xsl:value-of select="$FilePath"/><xsl:value-of select="@name"/><xsl:value-of select="$XQueryFileExtension"/>
                           </xsl:otherwise>
                        </xsl:choose>
                     </A>
                  </td>
               </tr>
            </xsl:for-each>
         </xsl:if>
         
         <tr>
            <td colspan="2">
               <B>Query:</B>
            </td>
            <td>
               <xsl:if test="@is-XPath2='true'">
                  (XPath 2.0)
               </xsl:if>
            </td>
         </tr>
         <tr>
            <td colspan="3">
               <A>
                  <xsl:choose>
                     <xsl:when test="$TargetLanguage='XQuery'">
                        <xsl:attribute name="href">
                           <xsl:value-of select="$XQueryQueryOffsetPath"/><xsl:value-of select="$FilePath"/><xsl:value-of select="test:query/@name"/><xsl:value-of select="$XQueryFileExtension"/>
                        </xsl:attribute>
                        <xsl:value-of select="$XQueryQueryOffsetPath"/><xsl:value-of select="$FilePath"/><xsl:value-of select="test:query/@name"/><xsl:value-of select="$XQueryFileExtension"/>
                     </xsl:when>
                     <xsl:when test="$TargetLanguage='XQueryX'">
                        <xsl:attribute name="href">
                           <xsl:value-of select="$XQueryXQueryOffsetPath"/><xsl:value-of select="$FilePath"/><xsl:value-of select="test:query/@name"/><xsl:value-of select="$XQueryXFileExtension"/>
                        </xsl:attribute>
                        <xsl:value-of select="$XQueryXQueryOffsetPath"/><xsl:value-of select="$FilePath"/><xsl:value-of select="test:query/@name"/><xsl:value-of select="$XQueryXFileExtension"/>
                     </xsl:when>
                     <xsl:otherwise>
                        <xsl:attribute name="href">
                           <xsl:value-of select="$XQueryQueryOffsetPath"/><xsl:value-of select="$FilePath"/><xsl:value-of select="test:query/@name"/><xsl:value-of select="$XQueryFileExtension"/>
                        </xsl:attribute>
                        <xsl:value-of select="$XQueryQueryOffsetPath"/><xsl:value-of select="$FilePath"/><xsl:value-of select="test:query/@name"/><xsl:value-of select="$XQueryFileExtension"/>
                     </xsl:otherwise>
                  </xsl:choose>
               </A>
            </td>
         </tr>
         <xsl:if test="test:module">
            <tr>
               <td colspan="3">
                  <B>Modules:</B>
               </td>
            </tr>
            <xsl:for-each select="test:module">
               <tr>
                  <td colspan="2">
                     Module ID:
                     <A>
                        <xsl:attribute name="href">#<xsl:value-of select="."/></xsl:attribute>
                        <xsl:value-of select="."/>
                     </A>
                  </td>
                  <td>
                     Namespace:
                     <A>
                        <xsl:attribute name="href">#<xsl:value-of select="@namespace"/></xsl:attribute>
                        <xsl:value-of select="@namespace"/>
                     </A>
                  </td>
               </tr>
            </xsl:for-each>
         </xsl:if>
         <tr>
            <td colspan="3">
               <B>Inputs:</B>
            </td>
         </tr>
         <xsl:for-each select="test:input-file">
            <tr>
               <td>
                  Name:
                  <xsl:value-of select="@variable"/>
               </td>
               <td>
                  Role:
                  <A>
                     <xsl:attribute name="href">#<xsl:value-of select="@role"/></xsl:attribute>
                     <xsl:value-of select="@role"/>
                  </A>
               </td>
               <td>
                  Source ID:
                  <A>
                     <xsl:attribute name="href">#<xsl:value-of select="."/></xsl:attribute>
                     <xsl:value-of select="."/>
                  </A>
               </td>
            </tr>
         </xsl:for-each>
         <xsl:for-each select="test:input-URI">
            <tr>
               <td>
                  Role:
                  <A>
                     <xsl:attribute name="href">#<xsl:value-of select="@role"/></xsl:attribute>
                     <xsl:value-of select="@role"/>
                  </A>
               </td>
               <td>
                  Source ID:
                  <A>
                     <xsl:attribute name="href">#<xsl:value-of select="."/></xsl:attribute>
                     <xsl:value-of select="."/>
                  </A>
               </td>
               <td>
	          <xsl:text>(as input URI)</xsl:text>
               </td>
            </tr>
         </xsl:for-each>
         <xsl:for-each select="test:defaultCollection">
            <tr>
               <td>
                  Role:
                  <A>
                     <xsl:attribute name="href">#<xsl:value-of select="@role"/></xsl:attribute>
                     <xsl:value-of select="@role"/>
                  </A>
               </td>
               <td>
                  Source ID:
                  <A>
                     <xsl:attribute name="href">#<xsl:value-of select="."/></xsl:attribute>
                     <xsl:value-of select="."/>
                  </A>
               </td>
               <td>
	          <xsl:text>(as default collection)</xsl:text>
               </td>
            </tr>
         </xsl:for-each>
         <xsl:for-each select="test:contextItem">
            <tr>
               <td>
                  Role:
                  <A>
                     <xsl:attribute name="href">#<xsl:value-of select="@role"/></xsl:attribute>
                     <xsl:value-of select="@role"/>
                  </A>
               </td>
               <td>
                  Source ID:
                  <A>
                     <xsl:attribute name="href">#<xsl:value-of select="."/></xsl:attribute>
                     <xsl:value-of select="."/>
                  </A>
               </td>
               <td>
	          <xsl:text>(as context item)</xsl:text>
               </td>
            </tr>
         </xsl:for-each>
         <tr>
            <td colspan="3">
               <B>Outputs:</B>
            </td>
         </tr>
         <xsl:for-each select="test:output-file">
            <tr>
               <td>
                  Role:
                  <A>
                     <xsl:attribute name="href">#<xsl:value-of select="@role"/></xsl:attribute>
                     <xsl:value-of select="@role"/>
                  </A>
                  <xsl:if test="@spec-version">
                     <xsl:value-of select="concat(' (Version ', @spec-version, ')')" />
                  </xsl:if>
               </td>
               <td>
                  Comparator:
                  <A>
                     <xsl:attribute name="href">#<xsl:value-of select="@compare"/></xsl:attribute>
                     <xsl:value-of select="@compare"/>
                  </A>
               </td>
               <td>
                  <A>
                     <xsl:attribute name="href">
                        <xsl:value-of select="$ResultOffsetPath"/><xsl:value-of select="$FilePath"/><xsl:value-of select="."/></xsl:attribute>
                     Results/<xsl:value-of select="$FilePath"/><xsl:value-of select="."/>
                  </A>
               </td>
            </tr>
         </xsl:for-each>
         <xsl:for-each select="test:expected-error">
            <tr>
               <td colspan="3">
                  Error:
                  <A>
                     <xsl:attribute name="href">http://www.w3.org/TR/xquery/#ERR<xsl:value-of select="."/></xsl:attribute>
                     err:<xsl:value-of select="."/>
                  </A>
                  <xsl:if test="@spec-version">
                     <xsl:value-of select="concat(' (Version ', @spec-version, ')')" />
                  </xsl:if>
               </td>
            </tr>
         </xsl:for-each>
      </table>
      <p/>
   </xsl:template>
   <xsl:template match="test:citation-spec">
      <tr>
         <td>
            <A>
               <xsl:attribute name="name">
                  <xsl:value-of select="@name"/>
               </xsl:attribute>
               <xsl:attribute name="href">
                  <xsl:value-of select="test:spec-URI"/>
               </xsl:attribute>
               <b>
                  <xsl:value-of select="@name"/>
               </b>
            </A>
         </td>
         <td>
            <xsl:value-of select="test:description"/>
         </td>
         <td>
            <I>
               <xsl:value-of select="test:note"/>
            </I>
         </td>
      </tr>
   </xsl:template>
   <xsl:template match="test:implementation-defined-item">
      <tr>
         <td valign="top">
            <xsl:text>[</xsl:text>
            <xsl:value-of select="@spec" />
            <xsl:text>]</xsl:text>
            <br />
            <xsl:value-of select="@name" />
         </td>
         <td valign="top">
            <xsl:value-of select="test:description/text()" />
         </td>
      </tr>
   </xsl:template>
   <xsl:template match="test:context-properties">
      <h4>Static Context</h4>
      <blockquote>
         <table>
            <xsl:apply-templates select="test:context-property[@context-type='static']" />
         </table>
      </blockquote>
      <h4>Dynamic Context</h4>
      <blockquote>
         <table>
            <xsl:apply-templates select="test:context-property[@context-type='dynamic']" />
         </table>
      </blockquote>
   </xsl:template>
   <xsl:template match="test:context-property">
      <tr>
         <td>
            <xsl:value-of select="@name" />
         </td>
         <td />
      </tr>
   </xsl:template>
   <xsl:template match="test:feature">
      <tr>
         <td>
            <xsl:value-of select="@name" />
         </td>
         <td />
      </tr>
   </xsl:template>
</xsl:stylesheet>