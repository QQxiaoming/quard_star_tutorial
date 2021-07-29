(:**************************************************************:)
(: Test: functx-functx-sequence-deep-equal-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether two sequences have the same XML node content and/or values 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_sequence-deep-equal.html 
 : @param   $seq1 the first sequence 
 : @param   $seq2 the second sequence 
 :) 
declare function functx:sequence-deep-equal 
  ( $seq1 as item()* ,
    $seq2 as item()* )  as xs:boolean {
       
  every $i in 1 to max((count($seq1),count($seq2)))
  satisfies deep-equal($seq1[$i],$seq2[$i])
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
return (functx:sequence-deep-equal(
     (1,2,3), (1.0,2.0,3.0)))
