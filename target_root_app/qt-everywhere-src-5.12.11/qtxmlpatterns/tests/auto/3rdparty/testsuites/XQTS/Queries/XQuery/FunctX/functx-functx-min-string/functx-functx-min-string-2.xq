(:**************************************************************:)
(: Test: functx-functx-min-string-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The minimum of a sequence of values, treating them like strings 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_min-string.html 
 : @param   $strings the sequence of strings 
 :) 
declare function functx:min-string 
  ( $strings as xs:anyAtomicType* )  as xs:string? {
       
   min(for $string in $strings return string($string))
 } ;

let $in-xml := <in-xml>
  <x>a</x>
  <y>c</y>
  <z>b</z>
</in-xml>
return (functx:min-string( (100,25,3) ))
