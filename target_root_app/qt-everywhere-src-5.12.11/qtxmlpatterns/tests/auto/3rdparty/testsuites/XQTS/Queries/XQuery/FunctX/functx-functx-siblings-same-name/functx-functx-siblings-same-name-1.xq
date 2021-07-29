(:**************************************************************:)
(: Test: functx-functx-siblings-same-name-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The siblings of an XML element that have the same name 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_siblings-same-name.html 
 : @param   $element the node 
 :) 
declare function functx:siblings-same-name 
  ( $element as element()? )  as element()* {
       
   $element/../*[node-name(.) = node-name($element)]
   except $element
 } ;

let $in-xml := <authors>
   <author>
      <fName>Kate</fName>
      <fName>Jane</fName>
      <lName>Jones</lName>
   </author>
   <author>
      <fName>John</fName>
      <lName>Doe</lName>
   </author>
</authors>
return (functx:siblings-same-name(
     $in-xml/author[1]))
