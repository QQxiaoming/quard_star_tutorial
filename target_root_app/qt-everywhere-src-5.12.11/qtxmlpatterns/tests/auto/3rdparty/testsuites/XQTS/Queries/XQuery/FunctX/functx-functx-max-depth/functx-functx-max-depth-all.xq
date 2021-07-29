(:**************************************************************:)
(: Test: functx-functx-max-depth-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The maximum depth of elements in an XML tree 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_max-depth.html 
 : @param   $root the root to start from 
 :) 
declare function functx:max-depth 
  ( $root as node()? )  as xs:integer? {
       
   if ($root/*)
   then max($root/*/functx:max-depth(.)) + 1
   else 1
 } ;

let $in-xml := <authors>
   <author>
      <fName>Kate</fName>
      <lName>Jones</lName>
   </author>
   <author>
      <fName>John</fName>
      <lName>Doe</lName>
   </author>
</authors>
return (functx:max-depth($in-xml), functx:max-depth($in-xml/author[1]))