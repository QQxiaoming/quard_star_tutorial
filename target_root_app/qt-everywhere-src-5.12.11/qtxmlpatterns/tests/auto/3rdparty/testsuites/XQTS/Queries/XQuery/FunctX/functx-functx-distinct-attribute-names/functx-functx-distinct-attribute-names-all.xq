(:**************************************************************:)
(: Test: functx-functx-distinct-attribute-names-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The distinct names of all attributes in an XML fragment 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_distinct-attribute-names.html 
 : @param   $nodes the root to start from 
 :) 
declare function functx:distinct-attribute-names 
  ( $nodes as node()* )  as xs:string* {
       
   distinct-values($nodes//@*/name(.))
 } ;

let $in-xml := <authors a1="xyz">
   <author a2="abc">
      <fName a3="def">Kate</fName>
      <lName>Jones</lName>
   </author>
   <author>
      <fName a3="def">John</fName>
      <lName>Doe</lName>
   </author>
</authors>
return (functx:distinct-attribute-names($in-xml))