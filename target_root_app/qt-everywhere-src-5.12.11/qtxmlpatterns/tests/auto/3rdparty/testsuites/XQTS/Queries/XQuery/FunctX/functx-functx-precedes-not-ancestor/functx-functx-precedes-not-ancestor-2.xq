(:**************************************************************:)
(: Test: functx-functx-precedes-not-ancestor-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether an XML node precedes another without being its ancestor 
 :
 : @author  W3C XML Query Working Group 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_precedes-not-ancestor.html 
 : @param   $a the first node 
 : @param   $b the second node 
 :) 
declare function functx:precedes-not-ancestor 
  ( $a as node()? ,
    $b as node()? )  as xs:boolean {
       
   $a << $b and empty($a intersect $b/ancestor::node())
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
return (functx:precedes-not-ancestor(
     $in-xml//author[1],
     $in-xml//author[1]/fName))
