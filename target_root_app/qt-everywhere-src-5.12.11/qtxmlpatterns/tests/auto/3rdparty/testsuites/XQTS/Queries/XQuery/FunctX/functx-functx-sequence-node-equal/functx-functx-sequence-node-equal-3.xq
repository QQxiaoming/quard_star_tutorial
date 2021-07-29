(:**************************************************************:)
(: Test: functx-functx-sequence-node-equal-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether two sequences contain the same XML nodes, in the same order 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_sequence-node-equal.html 
 : @param   $seq1 the first sequence of nodes 
 : @param   $seq2 the second sequence of nodes 
 :) 
declare function functx:sequence-node-equal 
  ( $seq1 as node()* ,
    $seq2 as node()* )  as xs:boolean {
       
  every $i in 1 to max((count($seq1),count($seq2)))
  satisfies $seq1[$i] is $seq2[$i]
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
   <fName>Kate</fName>
   <lName>Jones</lName>
</author>
return (functx:sequence-node-equal(
     $in-xml/author[1],$anAuthor))
