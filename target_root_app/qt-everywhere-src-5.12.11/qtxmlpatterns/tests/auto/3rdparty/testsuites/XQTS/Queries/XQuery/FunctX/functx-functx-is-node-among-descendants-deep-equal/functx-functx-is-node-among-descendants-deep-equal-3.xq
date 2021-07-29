(:**************************************************************:)
(: Test: functx-functx-is-node-among-descendants-deep-equal-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether an XML node is among the descendants of a sequence, based on contents and attributes 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_is-node-among-descendants-deep-equal.html 
 : @param   $node the node to test 
 : @param   $seq the sequence of nodes to search 
 :) 
declare function functx:is-node-among-descendants-deep-equal 
  ( $node as node()? ,
    $seq as node()* )  as xs:boolean {
       
   some $nodeInSeq in $seq/descendant-or-self::*/(.|@*)
   satisfies deep-equal($nodeInSeq,$node)
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
return 
let $anAuthor := <author>
  <fName>John</fName>
  <lName>Doe</lName>
</author>
return 
let $anotherAuthor := <author>
  <lName>Doe</lName>
  <fName>John</fName>
</author>
return (functx:is-node-among-descendants-deep-equal(
     $anotherAuthor,$in-xml))
