<?xml version="1.0"?> 

<!--                                                                         -->
<!-- Generate a report for one or more XQTS result reports                   -->
<!--                                                                         -->
<!-- Author: Andrew Eisenberg                                                -->
<!--                                                                         -->
<!-- History:                                                                -->
<!--                                                                         -->
<!--   2005-06-29    Initial release                                         -->
<!--                                                                         -->
<!--   2005-10-28    Add XQueryX result columns                              -->
<!--                 Improved formatting of tables                           -->
<!--                                                                         -->
<!--   2005-11-28    Identify implementations that use older versions of     -->
<!--                 the test suite                                          -->
<!--                 Allow report title to be annotated                      -->
<!--                                                                         -->
<!--                                                                         -->
<!--   2005-12-15    Change summary table entries to "pass/fail/total"       -->
<!--                 Fill in some missing "not specified" in the report      -->
<!--                                                                         -->
<!--   2006-09-22    Add links from test cases to catalog entries and        -->
<!--                 query text                                              -->
<!--                                                                         -->
<!--   2006-10-24    Generate % passed for Minimal Conformance line of       -->
<!--                 summary                                                 -->
<!--                                                                         -->
<!--   2006-10-26    Adjust total tests for XQueryX implementations          -->
<!--                 (XQueryX isn't generated for parse errors)              -->
<!--                                                                         -->
<!--   2006-11-08    Add light green coloring for "almost perfect".          -->
<!--                 Add a legend.                                           -->
<!--                                                                         -->
<!--   2006-12-19    Accept XQTS version of "".                              -->
<!--                                                                         -->
<!--   2007-11-08    Accept common HTML tags in an xqtsr:p element.          -->
<!--                                                                         -->
<!--   2010-03-19    Remove test for "K-" and "K2-" test cases.              -->
<!--                                                                         -->


<!-- Columns and organizatons can be marked with anonymous true or false.    -->
<!-- If absent, then false is assumed.                                       -->


<xsl:stylesheet
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   version="1.0"
   xmlns:xqts="http://www.w3.org/2005/02/query-test-XQTSCatalog"
   xmlns:xqtsr="http://www.w3.org/2005/02/query-test-XQTSResult">
   
   <xsl:output
      method="html"
      indent="no"
      />
   
   <xsl:strip-space elements="*"/>
   
   
   <!--                                                                      -->
   <!-- Parameters                                                           -->
   <!--                                                                      -->
   
   <!-- Place document author at end of report -->
   <xsl:param name="documentAuthor" select="''" />
   
   <!-- Place creation date at end of report -->
   <xsl:param name="creationDate" select="''" />
   
   <!-- Document that contains URLs of result reports -->
   <!-- <results><result>url</result>...</results> -->
   <xsl:param name="resultFiles" select="'Results.xml'" />
   
   <!-- Generate Summarized Results section -->
   <xsl:param name="summary" select="'true'" />
   
   <!-- Generate Implementation-defined items, Static Context Properties,   -->
   <!-- and Dynamic Context Properties                                      -->
   <xsl:param name="impdef" select="'true'" />
   
   <!-- Generate Detailed Results section -->
   <xsl:param name="details" select="'true'" />
   
   <!-- Generate failures only -->
   <xsl:param name="failures" select="'false'" />
   
   <!-- Generate Test Run section -->
   <xsl:param name="test-run-details" select="'true'" />
   
   <!-- Add to the report title -->
   <xsl:param name="title-annotation" select="''" />
   
   
   
   <!-- Some global variables -->
   <xsl:variable name="results" select='document(document($resultFiles)/results/*)'/>
   <!--   
   <xsl:variable name="files" select='document($resultFiles)/results/*'/>
   <xsl:variable name="OResultFiles">
      <xsl:for-each select="$files">
         <xsl:sort select="document(.)/xqtsr:test-suite-result/xqtsr:syntax"/>
         <xsl:value-of select="."/>
         <xsl:text> </xsl:text>
      </xsl:for-each>
   </xsl:variable>
   <xsl:variable name="results" select='document($OResultFiles)'/>
   -->
   <xsl:variable name='summaryColumns'>
      <xsl:choose>
         <xsl:when test='count($results) &gt; 1'>1</xsl:when>
         <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>
   </xsl:variable>
   
   <xsl:variable name="XQTSversion" select="xqts:test-suite/@version"/>
   
   
   <xsl:variable name="TargetLanguage" select="xqts:test-suite/@targetLanguage" />
   <xsl:variable name="XQueryQueryOffsetPath" select="xqts:test-suite/@XQueryQueryOffsetPath" />
   <xsl:variable name="XQueryXQueryOffsetPath" select="xqts:test-suite/@XQueryXQueryOffsetPath" />
   <xsl:variable name="ResultOffsetPath" select="xqts:test-suite/@ResultOffsetPath" />
   <xsl:variable name="XQueryFileExtension" select="xqts:test-suite/@XQueryFileExtension" />
   <xsl:variable name="XQueryXFileExtension" select="xqts:test-suite/@XQueryXFileExtension" />
   <xsl:variable name="SourceOffsetPath" select="xqts:test-suite/@SourceOffsetPath" />
   
   <xsl:variable name="xquery" select="count($results//xqtsr:syntax[text()='XQuery'])"/>
   <xsl:variable name="xqueryx" select="count($results//xqtsr:syntax[text()='XQueryX'])"/>
   
   <!-- colors -->
   
   <xsl:variable name="perfectcolor" select='"mediumseagreen"'/>
   <xsl:variable name="passcolor" select='"palegreen"'/>
   <xsl:variable name="failcolor" select='"coral"'/>
   <xsl:variable name="untestedcolor" select='"white"'/>
   <xsl:variable name="backgroundcolor" select='"lightcyan"'/>
   <xsl:variable name="groupcolor" select='"paleturquoise"'/>
   
   <xsl:key name='byname' match='xqtsr:test-case' use='@name'/>
   
   <xsl:key name='byscenario' match='xqts:test-case' use='@scenario'/>
   
   
   
   <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
   <!-- /                                                                    -->
   <!--                                                                      -->
   <!-- Generate:                                                            -->
   <!--    1) TOC, if necessary                                              -->
   <!--    2) Each of the sections                                           -->
   <!--    3) Footer, with author and creation date                          -->
   <!--                                                                      -->
   <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
   
   <xsl:template match="/">
      
      <html xmlns="http://www.w3.org/1999/xhtml">
         <head>
            <title>
               <xsl:text>XQuery Test Suite Result Summary</xsl:text>
               <xsl:if test="$title-annotation != ''">
                  <xsl:text> - </xsl:text>
                  <xsl:value-of select="$title-annotation" />
               </xsl:if>
            </title>            
         </head>
         
         
         <body>
            
            <center>
               <h1>
                  <xsl:text>XQuery Test Suite Result Summary</xsl:text>
                  <xsl:if test="$title-annotation != ''">
                     <xsl:text> - </xsl:text>
                     <xsl:value-of select="$title-annotation" />
                  </xsl:if>
               </h1>
            </center>
            
            <!-- Generate summary -->
            
            <p/>
            <blockquote>
               <p>
                  This document contains the results of running the
                  <a href="http://www.w3.org/XML/Query/test-suite/">XML Query Test Suite</a>
                  on one or more implementaions of XQuery and XQueryX.
               </p>
               
               <p>
                  <xsl:text>The latest version of this test suite is XQTS </xsl:text>
                  <xsl:value-of select="$XQTSversion"/>
                  <xsl:text>. Implementations that have used older versions of the test suite are noted.</xsl:text>
               </p>
               
               <p>
                  When results are listed as number/number/number, then indicate passed/failed/total.
                  Passed and failed together may not equal total, due to tests not run or not reported.
               </p>
               
               <p>The latest version of our files is available at
                  <a href="http://dev.w3.org/cvsweb/2006/xquery-test-suite/">http://dev.w3.org/cvsweb/2006/xquery-test-suite/</a>.
                  <xsl:if test='$details="true"'>
                     The "catalog" and "query" links found with each test case are links to this version.                     
                  </xsl:if>
               </p>
               
               <xsl:apply-templates>
                  <xsl:with-param name='summary' select="'yes'"/>
               </xsl:apply-templates>
            </blockquote>
            <p/>
            
            
            
            <!-- Generate TOC if the report contains multiple sections -->
            
            <xsl:variable name='impdefp'>
               <xsl:choose>
                  <xsl:when test='$impdef = "true"'>1</xsl:when>
                  <xsl:otherwise>0</xsl:otherwise>
               </xsl:choose>
            </xsl:variable>
            
            <xsl:variable name='testrunp'>
               <xsl:choose>
                  <xsl:when test='$test-run-details = "true"'>1</xsl:when>
                  <xsl:otherwise>0</xsl:otherwise>
               </xsl:choose>
            </xsl:variable>
            
            <xsl:variable name='summaryp'>
               <xsl:choose>
                  <xsl:when test='$summary = "true"'>1</xsl:when>
                  <xsl:otherwise>0</xsl:otherwise>
               </xsl:choose>
            </xsl:variable>
            
            <xsl:variable name='detailsp'>
               <xsl:choose>
                  <xsl:when test='$details = "true"'>1</xsl:when>
                  <xsl:otherwise>0</xsl:otherwise>
               </xsl:choose>
            </xsl:variable>
            
            <xsl:if test='($detailsp + $impdefp + $summaryp) > 1'>
               
               <hr/>            
               
               <h3>Contents</h3>
               <ul>
                  <li><a href='#implementations'>Implementations</a></li>
                  <xsl:if test='$test-run-details = "true"'>
                     <li><a href='#testRun'>Test Run</a></li>
                  </xsl:if>
                  <xsl:if test='$impdef = "true"'>
                     <li><a href='#features'>Features</a></li>
                     <li><a href='#impdef'>Implementation-defined Items</a></li>
                     <li><a href='#staticcontext'>Static Context Properties</a></li>
                     <li><a href='#dynamiccontext'>Dynamic Context Properties</a></li>
                  </xsl:if>
                  <xsl:if test='$summary = "true"'>
                     <li><a href='#summary'>Summarized Results</a></li>
                  </xsl:if>
                  <xsl:if test='$details = "true"'>
                     <li><a href='#details'>Detailed Results</a></li>
                  </xsl:if>
               </ul>
               
               <hr/>            
            </xsl:if>
            
            
            <!-- Generate a list of implementations in this report -->
            
            <h2><a name='implementations'/>Implementations:</h2>
            
            <blockquote>
               
               <xsl:for-each select="$results">
                  <xsl:sort select="./xqtsr:test-suite-result/xqtsr:syntax"/>
                  <xsl:sort select="./xqtsr:test-suite-result/xqtsr:implementation/@name"/>
                  <xsl:variable
                     name="result"
                     select="./xqtsr:test-suite-result/xqtsr:implementation"
                     />
                  
                  <!-- Skip implementations that wish to be anonymous -->
                  
                  <xsl:if test="not($result/xqtsr:organization/@anonymous = 'true')">
                     <h3>
                        <xsl:value-of select="$result/@name"/>		  
                     </h3>
                     <blockquote>
                        <xsl:apply-templates select="$result"/>
                     </blockquote>
                  </xsl:if>
               </xsl:for-each>
               
            </blockquote>
            
            
            <!-- Generate information about each test run -->
            
            <xsl:if test='$test-run-details = "true"'>
               
               <h2><a name='testRun'/>Test Run:</h2>
               
               <blockquote>
                  
                  <xsl:for-each select="$results">
                     <xsl:sort select="./xqtsr:test-suite-result/xqtsr:syntax"/>
                     <xsl:sort select="./xqtsr:test-suite-result/xqtsr:implementation/@name"/>
                     <xsl:variable
                        name="test-run"
                        select="./xqtsr:test-suite-result/xqtsr:test-run"
                        />
                     <xsl:variable
                        name="result"
                        select="./xqtsr:test-suite-result/xqtsr:implementation"
                        />
                     <h3>
                        <xsl:choose>
                           <xsl:when test="./xqtsr:test-suite-result/xqtsr:implementation/@anonymous-result-column = 'true'">
                              <xsl:text>Anonymous</xsl:text>
                           </xsl:when>
                           <xsl:otherwise>
                              <xsl:value-of select="$result/@name"/>		  
                           </xsl:otherwise>
                        </xsl:choose>
                     </h3>
                     <blockquote>
                        <xsl:apply-templates select="$test-run"/>
                     </blockquote>
                  </xsl:for-each>
                  
               </blockquote>
               
            </xsl:if>
            
            
            <!-- Generate information about features, implementation-defined items, -->
            <!-- and static context                                                 -->
            
            <xsl:if test='$impdef="true"'>
               <h2><a name='features'/>Features:</h2>
               
               <blockquote>
                  
                  <table>
                     <xsl:apply-templates select='//xqts:feature'/>
                  </table>
                  
               </blockquote>                  
               
               <h2><a name='impdef'/>Implementation-defined Items:</h2>
               
               <blockquote>
                  
                  <table>
                     <xsl:apply-templates select='//xqts:implementation-defined-item'/>
                  </table>
                  
               </blockquote>                  
               
               <h2><a name='staticcontext'/>Static Context Properties:</h2>
               
               <blockquote>
                  
                  <table>
                     <xsl:apply-templates select='//xqts:context-property[@context-type="static"]'/>
                  </table>
                  
               </blockquote>                  
               
               <h2><a name='dynamiccontext'/>Dynamic Context Properties:</h2>
               
               <blockquote>
                  
                  <table>
                     <xsl:apply-templates select='//xqts:context-property[@context-type="dynamic"]'/>
                  </table>
                  
               </blockquote>                  
               
            </xsl:if>
            
            
            <!-- Generate the summarized results -->            
            
            <xsl:if test='$summary = "true"'>
               <h2><a name='summary'/>Summarized Results:</h2>
               
               <blockquote>
                  
                  <xsl:apply-templates>
                     <xsl:with-param name='detail'>false</xsl:with-param>
                  </xsl:apply-templates>
                  
               </blockquote>
            </xsl:if>            
            
            
            <!-- Generate the detailed results -->
            
            <xsl:if test='$details = "true"'>            
               <h2><a name='details'/>Detailed Results:</h2>
               
               <xsl:apply-templates>
                  <xsl:with-param name='detail'>true</xsl:with-param>
               </xsl:apply-templates>
               
            </xsl:if>            
            
            
            <!-- Generate the document footer with author and creation date -->
            
            <xsl:if test="$documentAuthor != '' or $creationDate != ''">
               
               <p/>
               <p align="right">
                  <small>
                     <xsl:if test="$documentAuthor != ''">
                        <xsl:text>Generated: </xsl:text>
                        <xsl:value-of select="$documentAuthor"/>
                        <br/>
                     </xsl:if>
                     <xsl:if test="$creationDate != ''">
                        <xsl:value-of select="$creationDate"/>
                        <br/>
                     </xsl:if>
                  </small>
               </p>
            </xsl:if>
            
         </body>
         
      </html>               
      
   </xsl:template>
   
   
   
   
   <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
   <!-- xqts:test-suite                                                      -->
   <!--                                                                      -->
   <!-- Generate a table with columns:                                       -->
   <!--    1) Feature - the feature name                                     -->
   <!--    2) a column for each result report, indicating success or failure -->
   <!--    3) a summary for all reports, if more than one                    -->
   <!--                                                                      -->
   <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
   
   <xsl:template match='xqts:test-suite'>
      
      <xsl:param name='detail'/>
      <xsl:param name='summary' select="'no'"/>
      
      <!-- Generate the legent -->
      
      <table border="0" width="100%">
         <tr>
            <td>
               <table align="right">
                  <tr>
                     <td>Legend:&#xA0;&#xA0;&#xA0;&#xA0;</td>
                     <td>
                        <table frame="border">
                           <tr>
                              <td bgcolor="{$perfectcolor}">&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;</td>
                           </tr>
                        </table>
                     </td>
                     <td>passed</td>
                     <td>&#xA0;&#xA0;&#xA0;&#xA0;</td>
                     <td>
                        <table frame="border">
                           <tr>
                              <td bgcolor="{$passcolor}">&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;</td>
                           </tr>
                        </table>
                     </td>
                     <td>almost passed<br/>(&#x2265; 98%)</td>
                     <td>&#xA0;&#xA0;&#xA0;&#xA0;</td>
                     <td>
                        <table frame="border">
                           <tr>
                              <td bgcolor="{$failcolor}">&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;</td>
                           </tr>
                        </table>
                     </td>
                     <td>failed</td>
                     <td>&#xA0;&#xA0;&#xA0;&#xA0;</td>
                     <td>
                        <table frame="border">
                           <tr>
                              <td bgcolor="{$untestedcolor}">&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;</td>
                           </tr>
                        </table>
                     </td>
                     <td>untested</td>
                  </tr>
               </table>
            </td>
         </tr>
      </table>
      
      <blockquote>      
         <table
            frame="hsides"
            rules="groups"
            border="1"
            bordercolor="black"
            bgcolor="{$backgroundcolor}"
            cellpadding="2">
            
            <colgroup align="left"/>
            <colgroup align="center" span="{$xquery}"/>
            <colgroup align="center"  span="{$xqueryx}"/>
            <xsl:if test='$summaryColumns = 1'>            
               <colgroup align="left"/>
            </xsl:if>
            
            <thead>
               <!-- Generate column heads for XQuery and XQueryX groups -->
               
               <tr>
                  <th></th> <!-- empty column over "features" -->
                  <xsl:if test="$xquery != 0 and ($xquery + $xqueryx) != 1">
                     <th colspan="{$xquery}">XQuery<br/><hr/></th>
                  </xsl:if>
                  <xsl:if test="$xqueryx != 0 and ($xquery + $xqueryx) != 1">
                     <th colspan="{$xqueryx}">XQueryX<br/><hr/></th>
                  </xsl:if>
                  <xsl:if test='$summaryColumns = 1'>
                     <th></th>
                  </xsl:if>
               </tr>
               
               <tr>
                  
                  <th>Feature</th>
                  
                  <!-- Generate a column head for each result report -->
                  
                  <xsl:for-each select="$results">
                     <xsl:sort select="./xqtsr:test-suite-result/xqtsr:syntax"/>
                     <xsl:sort select="./xqtsr:test-suite-result/xqtsr:implementation/@name"/>
                     <th valign="top">
                        <xsl:choose>
                           <xsl:when test="./xqtsr:test-suite-result/xqtsr:implementation/@anonymous-result-column = 'true'">
                              <xsl:text>Anonymous</xsl:text>
                           </xsl:when>
                           <xsl:otherwise>
                              <xsl:value-of select="./xqtsr:test-suite-result/xqtsr:implementation/@name"/>
                           </xsl:otherwise>
                        </xsl:choose>
                        <xsl:if test="./xqtsr:test-suite-result/xqtsr:test-run/xqtsr:test-suite/@version != $XQTSversion">
                           <br/>
                           <font size="-1">
                              <xsl:choose>
                                 <xsl:when test="./xqtsr:test-suite-result/xqtsr:test-run/xqtsr:test-suite/@version = ''">
                                    <xsl:text>(unspecified)</xsl:text>
                                 </xsl:when>
                                 <xsl:otherwise>
                                    <xsl:text>(XQTS </xsl:text>
                                    <xsl:value-of select="./xqtsr:test-suite-result/xqtsr:test-run/xqtsr:test-suite/@version" />
                                    <xsl:text>)</xsl:text>
                                 </xsl:otherwise>
                              </xsl:choose>
                           </font>
                        </xsl:if>
                     </th>
                  </xsl:for-each>
                  
                  <xsl:if test='$summaryColumns = 1'>
                     <th>Summary</th>
                  </xsl:if>
               </tr>
               
            </thead>
            
            <tbody>         
               <xsl:choose> 
                  <xsl:when test="$summary='yes'">
                     <xsl:apply-templates
                        select='xqts:test-group[xqts:GroupInfo/xqts:title="Minimal Conformance"
                        or xqts:GroupInfo/xqts:title="Optional Features"]'>
                        <xsl:with-param name='levels' select="1"/>
                        <xsl:with-param name='grandSummary' select="'true'"/>
                     </xsl:apply-templates>
                  </xsl:when>
                  <xsl:otherwise>        
                     <xsl:apply-templates select='xqts:test-group'>
                        <xsl:with-param name='detail'><xsl:value-of select="$detail"/></xsl:with-param>
                     </xsl:apply-templates>
                     
                  </xsl:otherwise> 
               </xsl:choose>        
               
            </tbody>
         </table>
      </blockquote>
      
   </xsl:template>
   
   
   
   
   <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
   <!-- xqts:test-group                                                      -->
   <!--                                                                      -->
   <!-- Generate the test group hierarchy in the left-most column            -->
   <!--                                                                      -->
   <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
   
   <xsl:template match="xqts:test-group">
      <xsl:param name='detail'/>
      <xsl:param name='levels' select="100"/>
      <xsl:param name='grandSummary' select="'false'"/>
      
      <xsl:variable name="immediate-tests" select="./xqts:test-case"/>
      <xsl:variable name="tests" select=".//xqts:test-case"/>
      <xsl:variable name="rawtotal" select="count($tests)"/>
      <xsl:variable name="title" select="xqts:GroupInfo/xqts:title"/>
      <xsl:variable
         name='parseErrors'
         select="key('byscenario', 'parse-error')"/>
      <xsl:variable name='totalNotPE' select="count($tests[.=$parseErrors])"/>
      
      
      <!-- If the feature group does not immediately contain any tests, then -->
      <!-- span then entire table.                                           -->
      
      <xsl:choose>
         <xsl:when test="(count($immediate-tests) = 0 and $grandSummary='false')
            or $grandSummary='true' and xqts:GroupInfo/xqts:title = 'Optional Features'">
            <tr bgcolor="{$groupcolor}">
               <td>
                  <xsl:for-each select="ancestor::xqts:test-group">&#xA0;&#xA0;&#xA0;&#xA0;</xsl:for-each>
                  <xsl:value-of select="xqts:GroupInfo/xqts:title"/>
               </td>
               
               <xsl:for-each select="$results">
                  <td/>
               </xsl:for-each>
               
               <xsl:if test='$summaryColumns = 1'>
                  <td/>
               </xsl:if>
               
            </tr>            
         </xsl:when>
         
         <xsl:otherwise>            
            <tr>
               <td valign='top'>
                  <xsl:for-each select="ancestor::xqts:test-group">&#xA0;&#xA0;&#xA0;&#xA0;</xsl:for-each>
                  <xsl:value-of select="xqts:GroupInfo/xqts:title"/>
               </td>
               
               <!-- summary for each group -->
               
               <xsl:for-each select="$results">
                  <xsl:sort select="./xqtsr:test-suite-result/xqtsr:syntax"/>
                  <xsl:sort select="./xqtsr:test-suite-result/xqtsr:implementation/@name"/>
                  <xsl:variable name='syntax' select='./xqtsr:test-suite-result/xqtsr:syntax'/>
                  <td align="center">
                     <xsl:variable
                        name='results'
                        select="key('byname', $tests/@name)"/>
                     <xsl:variable
                        name='passed'
                        select="count($results[@result='pass'])"
                        />
                     <xsl:variable
                        name='failed'
                        select="count($results[@result='fail'])"
                        />
                     <xsl:variable name="total">
                        <xsl:choose>
                           <xsl:when test="$syntax='XQueryX'">
                              <xsl:value-of select='$rawtotal - $totalNotPE'/>
                           </xsl:when>
                           <xsl:otherwise>
                              <xsl:value-of select='$rawtotal'/>
                           </xsl:otherwise>
                        </xsl:choose>
                     </xsl:variable>
                     
                     <xsl:attribute name="bgcolor">
                        <xsl:choose>
                           <xsl:when test='$passed=$total and $passed != 0'>
                              <xsl:value-of select="$perfectcolor"/>
                           </xsl:when>
                           <xsl:when test='(100 * $passed) &gt; (98 * $total)'>
                              <xsl:value-of select="$passcolor"/>
                           </xsl:when>
                           <xsl:when test='$passed = 0'>
                              <xsl:value-of select="$untestedcolor"/>
                           </xsl:when>
                           <xsl:otherwise>
                              <xsl:value-of select="$failcolor"/>
                           </xsl:otherwise>
                        </xsl:choose>
                     </xsl:attribute>
                     <xsl:choose>                     
                        <xsl:when test="$grandSummary='true'">
                           <xsl:text>&#xA0;</xsl:text>
                           <xsl:value-of select="$passed"/>
                           <xsl:text>&#xA0;/&#xA0;</xsl:text>
                           <xsl:value-of select="$failed"/>
                           <xsl:text>&#xA0;/&#xA0;</xsl:text>
                           <xsl:value-of select="$total"/>                           
                           <xsl:text>&#xA0;</xsl:text>
                           <xsl:if test="$title='Minimal Conformance'">
                              <br/>
                              <xsl:value-of select="concat(round((1000*$passed) div $total) div 10, '%')"/>
                           </xsl:if>
                        </xsl:when>
                        <xsl:otherwise>
                           <xsl:value-of select="$passed"/>
                           <xsl:text>/</xsl:text>
                           <xsl:value-of select="$total"/>                           
                        </xsl:otherwise>                        
                     </xsl:choose>
                  </td>
               </xsl:for-each>
               
               <xsl:if test='$summaryColumns = 1'>
                  <td align="center">
                     <xsl:variable name="totalresults" select="count($results)"/>
                     <xsl:variable name="passresults">
                        <xsl:for-each select="$results">
                           <xsl:variable
                              name='results'
                              select="key('byname', $tests/@name)"/>                        
                           <xsl:variable name='syntax' select='./xqtsr:test-suite-result/xqtsr:syntax'/>
                           <xsl:variable name="total">
                              <xsl:choose>
                                 <xsl:when test="$syntax='XQueryX'">
                                    <xsl:value-of select='$rawtotal - $totalNotPE'/>
                                 </xsl:when>
                                 <xsl:otherwise>
                                    <xsl:value-of select='$rawtotal'/>
                                 </xsl:otherwise>
                              </xsl:choose>
                           </xsl:variable>
                           <xsl:if test="$total = count($results[@result='pass']) and $total != 0">
                              <xsl:value-of select="1"/>
                           </xsl:if>
                        </xsl:for-each>
                     </xsl:variable>
                     
                     <xsl:variable
                        name='passed'
                        select="string-length($passresults)"
                        />
                     <xsl:attribute name="bgcolor">
                        <xsl:choose>
                           <xsl:when test='($totalresults="1" and $passed="1") or $passed >= 2'>
                              <xsl:value-of select="$perfectcolor"/>
                           </xsl:when>
                           <xsl:otherwise>
                              <xsl:value-of select="$failcolor"/>
                           </xsl:otherwise>
                        </xsl:choose>
                     </xsl:attribute>
                     
                     <xsl:value-of select="$passed"/>
                     <xsl:text>/</xsl:text>
                     <xsl:value-of select="$totalresults"/>
                  </td>
               </xsl:if>               
               
            </tr>
            
         </xsl:otherwise>               
      </xsl:choose>
      
      <!-- Generate detail rows, if requested  -->
      
      <xsl:if test='$detail="true"'>
         <xsl:apply-templates select="xqts:test-case">
            <xsl:with-param name='detail'>true</xsl:with-param>
         </xsl:apply-templates>
      </xsl:if>
      
      <!-- Generate rows for the test groups that are contained -->
      
      <xsl:if test="not($levels = 0
         or ($grandSummary = 'true' and xqts:GroupInfo/xqts:title = 'Minimal Conformance'))">
         <xsl:apply-templates select="xqts:test-group">
            <xsl:with-param name='detail' select='$detail'/>
            <xsl:with-param name='levels' select='$levels - 1'/>
            <xsl:with-param name='grandSummary' select='$grandSummary'/>
         </xsl:apply-templates>
         
      </xsl:if>      
   </xsl:template>
   
   
   
   
   <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
   <!-- xqts:test-case                                                       -->
   <!--                                                                      -->
   <!-- Generate the test group hierarchy for the left-most column           -->
   <!--                                                                      -->
   <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
   
   <xsl:template match="xqts:test-case">
      <xsl:variable name='test-name' select="@name" />
      <xsl:variable name='creator' select="@Creator" />
      <xsl:variable name="FilePath" select="@FilePath" />
      
      <xsl:variable name="failedresults">
         <xsl:for-each select="$results">
            <xsl:variable
               name='results'
               select="key('byname', $test-name)"/>                        
            <xsl:if test="count($results[@result='fail']) > 0">
               <xsl:value-of select="1"/>
            </xsl:if>
         </xsl:for-each>
      </xsl:variable>
      
      <xsl:variable
         name='failed'
         select="string-length($failedresults)"
         />
      
      <xsl:if test='$failures != "true" or $failed > 0'>
         <tr>
            <td valign="top">
               <xsl:for-each select="ancestor::xqts:test-group">&#xA0;&#xA0;&#xA0;&#xA0;</xsl:for-each>
               <xsl:value-of select="$test-name"/>
               
               <!-- generate links to catalog and query -->
               
                  <br>
                     <xsl:for-each select="ancestor::xqts:test-group">&#xA0;&#xA0;&#xA0;&#xA0;</xsl:for-each>
                     <font size="-2">
                        <xsl:text>&#xA0;&#xA0;&#xA0;&#xA0;(</xsl:text>
                        <a href='{concat("http://dev.w3.org/cvsweb/~checkout~/2006/xquery-test-suite/TestSuiteStagingArea/XQTSCatalog.xml#", $test-name)}'>catalog</a>
                        <xsl:text>, </xsl:text>
                        <a href='{concat("http://dev.w3.org/cvsweb/~checkout~/2006/xquery-test-suite/TestSuiteStagingArea/",$XQueryQueryOffsetPath, $FilePath, @name, $XQueryFileExtension)}'>query</a>
                        <xsl:text>)</xsl:text>
                     </font>
                  </br>
               
               <xsl:if test='$failures = "true"'>
                  <br>
                     <xsl:for-each select="ancestor::xqts:test-group">&#xA0;&#xA0;&#xA0;&#xA0;</xsl:for-each>
                     <xsl:text>(Created by </xsl:text>
                     <xsl:value-of select="$creator"/>
                     <xsl:text>)</xsl:text>
                  </br>
               </xsl:if>
            </td>
            
            
            <xsl:for-each select="$results">
               <xsl:sort select="./xqtsr:test-suite-result/xqtsr:syntax"/>
               <xsl:sort select="./xqtsr:test-suite-result/xqtsr:implementation/@name"/>
               
               <xsl:variable
                  name='test'
                  select="key('byname', $test-name)"/>
               
               <!-- Long text causes horizaontal scrolling                          -->
               <!-- IE solution style="word-break:break-all; word-wrap:break-word;" -->
               <!-- Another solution is to add <wbr> tags inside the text           -->
                  <!-- Some suggest  style="overflow-x:hidden;"                        -->
                  
                  <td valign="top">
                     <xsl:attribute name="bgcolor">
                        <xsl:choose>
                           <xsl:when test='$test/@result="pass"'>
                              <xsl:value-of select="$perfectcolor"/>
                           </xsl:when>
                           <xsl:when test='$test/@result="fail"'>
                              <xsl:value-of select="$failcolor"/>
                           </xsl:when>
                        </xsl:choose>
                     </xsl:attribute>
                     <xsl:choose>
                        <xsl:when test="$test">
                           <xsl:value-of select='$test/@result'/>
                           <xsl:if test="$test/@comment">
                              <div  align="left"  width="20">
                                 <font size="1">
                                    <p><xsl:value-of select="$test/@comment"/></p>
                                 </font>
                              </div>
                           </xsl:if>
                           
                        </xsl:when>
                        <xsl:otherwise>
                           <xsl:text>no result</xsl:text>
                        </xsl:otherwise>
                     </xsl:choose>
                  </td>
               </xsl:for-each>
               
            </tr>
         </xsl:if>
      </xsl:template>
      
      
      
      
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      <!-- xqts:implementation-defined-item                                     -->
      <!--                                                                      -->
      <!-- Generate a row for each implementation-define item in the catalog.   -->
      <!-- For each item, generate a table with information provided by each    -->
      <!-- of the test reports.                                                 -->
      <!--                                                                      -->
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      
      <xsl:template match='xqts:implementation-defined-item'>
         <xsl:variable name='name' select='@name'/>
         <xsl:variable name='spec' select='@spec'/>
         <tr>
            <td valign='top'>
               <xsl:text>[</xsl:text>
               <xsl:value-of select='@spec'/>
               <xsl:text>] </xsl:text>
               <xsl:value-of select='xqts:description/text()'/>
               <blockquote>
                  
                  <table>
                     <xsl:for-each select="$results">
                        <xsl:variable
                           name="result"
                           select="./xqtsr:test-suite-result/xqtsr:implementation"
                           />
                        <xsl:variable
                           name="feature"
                           select="./xqtsr:test-suite-result/xqtsr:implementation/xqtsr:implementation-defined-items/xqtsr:implementation-defined-item[@name=$name]"
                           />
                        <tr>
                           <td valign='top'>
                              <xsl:choose>
                                 <xsl:when test="./xqtsr:test-suite-result/xqtsr:implementation/@anonymous-result-column = 'true'">
                                    <xsl:text>Anonymous:&#xA0;&#xA0;&#xA0;</xsl:text>
                                 </xsl:when>
                                 <xsl:otherwise>
                                    <xsl:value-of select="$result/@name"/>
                                    <xsl:text>:&#xA0;&#xA0;&#xA0;</xsl:text>
                                 </xsl:otherwise>
                              </xsl:choose>
                           </td>
                           <td valign='top'>
                              <xsl:choose>
                                 <xsl:when test="$feature/@value">
                                    <xsl:value-of select="$feature/@value"/>		  
                                 </xsl:when>
                                 <xsl:otherwise>
                                    <xsl:text>not specified</xsl:text>
                                 </xsl:otherwise>
                              </xsl:choose>
                           </td>
                        </tr>
                     </xsl:for-each>
                  </table>
                  
               </blockquote>
               <p/>
            </td>
         </tr>
      </xsl:template>
      
      
      
      
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      <!-- xqts:feature                                                         -->
      <!--                                                                      -->
      <!-- Generate a row for each feature in the catalog. For each item,       -->
      <!-- generate a table with information provided by each of the test       -->
      <!-- reports.                                                             -->
      <!--                                                                      -->
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      
      <xsl:template match='xqts:feature'>
         <xsl:variable name='name' select='@name'/>
         <tr>
            <td valign='top'>
               <xsl:value-of select='$name'/>
               <blockquote>
                  
                  <table>
                     <xsl:for-each select="$results">
                        <xsl:variable
                           name="result"
                           select="./xqtsr:test-suite-result/xqtsr:implementation"
                           />
                        <xsl:variable
                           name="feature"
                           select="./xqtsr:test-suite-result/xqtsr:implementation/xqtsr:features/xqtsr:feature[@name=$name]"
                           />
                        <tr>
                           <td valign='top'>
                              <xsl:choose>
                                 <xsl:when test="./xqtsr:test-suite-result/xqtsr:implementation/@anonymous-result-column = 'true'">
                                    <xsl:text>Anonymous:&#xA0;&#xA0;&#xA0;</xsl:text>
                                 </xsl:when>
                                 <xsl:otherwise>
                                    <xsl:value-of select="$result/@name"/>
                                    <xsl:text>:&#xA0;&#xA0;&#xA0;</xsl:text>
                                 </xsl:otherwise>
                              </xsl:choose>
                           </td>
                           <td valign='top'>
                              <xsl:choose>
                                 <xsl:when test="$feature/@supported">
                                    <xsl:value-of select="$feature/@supported"/>
                                 </xsl:when>
                                 <xsl:otherwise>
                                    <xsl:text>not specified</xsl:text>
                                 </xsl:otherwise>
                              </xsl:choose>
                           </td>
                        </tr>
                     </xsl:for-each>
                  </table>
                  
               </blockquote>
               <p/>
            </td>
         </tr>
      </xsl:template>
      
      
      
      
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      <!-- xqts:context-property                                                -->
      <!--                                                                      -->
      <!-- Generate a row for each context property in the catalog. For each    -->
      <!-- property, generate a table with information provided by each of the  -->
      <!-- test reports.                                                        -->
      <!--                                                                      -->
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      
      <xsl:template match='xqts:context-property'>
         
         <xsl:variable name='name' select='@name'/>
         
         <tr>
            <td valign='top'>
               <xsl:value-of select='$name'/>
               <blockquote>
                  
                  <table>
                     <xsl:for-each select="$results">
                        <xsl:variable
                           name="result"
                           select="./xqtsr:test-suite-result/xqtsr:implementation"
                           />
                        <xsl:variable
                           name="property"
                           select="./xqtsr:test-suite-result/xqtsr:implementation/xqtsr:context-properties/xqtsr:context-property[@name=$name]"
                           />
                        <tr>
                           <td valign='top'>
                              <xsl:choose>
                                 <xsl:when test="./xqtsr:test-suite-result/xqtsr:implementation/@anonymous-result-column = 'true'">
                                    <xsl:text>Anonymous:&#xA0;&#xA0;&#xA0;</xsl:text>
                                 </xsl:when>
                                 <xsl:otherwise>
                                    <xsl:value-of select="$result/@name"/>
                                    <xsl:text>:&#xA0;&#xA0;&#xA0;</xsl:text>
                                 </xsl:otherwise>
                              </xsl:choose>
                           </td>
                           <td valign='top'>
                              <xsl:choose>
                                 <xsl:when test="$property/@value">
                                    <xsl:value-of select="$property/@value"/>
                                 </xsl:when>
                                 <xsl:otherwise>
                                    <xsl:text>not specified</xsl:text>
                                 </xsl:otherwise>
                              </xsl:choose>
                           </td>
                        </tr>
                     </xsl:for-each>
                  </table>
                  
               </blockquote>
               <p/>
            </td>
         </tr>
         
      </xsl:template>
      
      
      
      
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      <!-- xqtsr:implementation                                                 -->
      <!--                                                                      -->
      <!-- Generate rows that provide information about the implementation      -->
      <!--                                                                      -->
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      
      <xsl:template match="xqtsr:implementation">
         <table width="100%">
            <tr>
               <td width="25%" valign="top">Organization:</td>
               <td width="75%" valign="top">
                  <xsl:value-of select="xqtsr:organization/@name"/>
                  <br/>
                  <xsl:if test='xqtsr:organization/@website'>
                     <a href="{xqtsr:organization/@website}">
                        <xsl:value-of select="xqtsr:organization/@website"/>
                     </a>
                  </xsl:if>
               </td>
            </tr>
            <tr>
               <td valign="top">Implementation:</td>
               <td valign="top"><xsl:value-of select="@name"/></td>
            </tr>
            <tr>
               <td valign="top">Version:</td>
               <td valign="top"><xsl:value-of select="@version"/></td>
            </tr>
            
            
         </table>
      </xsl:template>
      
      
      
      
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      <!-- xqtsr:test-run                                                       -->
      <!--                                                                      -->
      <!-- Generate rows that provide information about the running of the test -->
      <!-- suite.                                                               -->
      <!--                                                                      -->
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      
      <xsl:template match="xqtsr:test-run">
         <table width="100%">
            <tr>
               <td width="25%" valign="top">Date:</td>
               <td width="75%" valign="top">
                  <xsl:value-of select="@dateRun"/>
               </td>
            </tr>         
            <tr>
               <td width="25%" valign="top">Test Suite Version:</td>
               <td width="75%" valign="top">
                  <xsl:value-of select="xqtsr:test-suite/@version"/>
               </td>
            </tr>         
            <tr>
               <td valign="top">Syntax:</td>
               <td valign="top"><xsl:value-of select="../xqtsr:syntax"/></td>
            </tr>
            
            <xsl:if test='xqtsr:transformation'>
               <tr>
                  <td valign="top" colspan="2">
                     Transformation:
                     <blockquote>
                        <xsl:apply-templates select='xqtsr:transformation/*'/>
                     </blockquote>
                  </td>
               </tr>         
            </xsl:if>
            <xsl:if test='xqtsr:comparison'>
               <tr>
                  <td valign="top" colspan="2">
                     Comparison:
                     <blockquote>
                        <xsl:apply-templates select='xqtsr:comparison/*'/>
                     </blockquote>
                  </td>
               </tr>         
            </xsl:if>
            <xsl:if test='xqtsr:otherComments'>
               <tr>
                  <td valign="top" colspan="2">
                     Other Comments:
                     <blockquote>
                        <xsl:apply-templates select='xqtsr:otherComments/*'/>
                     </blockquote>
                  </td>
               </tr>         
            </xsl:if>
         </table>
      </xsl:template>
      
      
      
      
      
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      <!-- xqtsr:p                                                              -->
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      
      <xsl:template match='xqtsr:p'>
         <p><xsl:apply-templates/></p>
      </xsl:template>
      
      
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      <!-- common HTML tags                                                     -->
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      
      <xsl:template match='p | table | tr | th | td | ol | ul | li | br'>
         <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
         </xsl:copy>
      </xsl:template>
      
      
      
      
      
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      <!-- xqtsr:implementation-defined                                         -->
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      
      <xsl:template match="xqtsr:implementation-defined">
         <table border="0">
            <xsl:apply-templates/>
         </table>
      </xsl:template>
      
      
      
      
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      <!-- xqtsr:feature                                                        -->
      <!-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -->
      
      <xsl:template match="xqtsr:feature">
         <tr>
            <td width="30%" valign="top">
               <xsl:value-of select="@name"/>
               <xsl:text>:&#xA0;</xsl:text>
            </td>
            <td width="70%" valign="top">
               <xsl:value-of select="@value"/>
            </td>
         </tr>
      </xsl:template>
      
      
   </xsl:stylesheet>
