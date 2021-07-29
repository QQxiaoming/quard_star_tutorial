(:**************************************************************:)
(: Test: functx-functx-replace-multi-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The first argument if it is not empty, otherwise the second argument 
 :
 : @author  W3C XML Query WG 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_if-absent.html 
 : @param   $arg the item(s) that may be absent 
 : @param   $value the item(s) to use if the item is absent 
 :) 
declare function functx:if-absent 
  ( $arg as item()* ,
    $value as item()* )  as item()* {
       
    if (exists($arg))
    then $arg
    else $value
 } ;

(:~
 : Performs multiple replacements, using pairs of replace parameters 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_replace-multi.html 
 : @param   $arg the string to manipulate 
 : @param   $changeFrom the sequence of strings or patterns to change from 
 : @param   $changeTo the sequence of strings to change to 
 :) 
declare function functx:replace-multi 
  ( $arg as xs:string? ,
    $changeFrom as xs:string* ,
    $changeTo as xs:string* )  as xs:string? {
       
   if (count($changeFrom) > 0)
   then functx:replace-multi(
          replace($arg, $changeFrom[1],
                     functx:if-absent($changeTo[1],'')),
          $changeFrom[position() > 1],
          $changeTo[position() > 1])
   else $arg
 } ;

let $fr := ('[a-c]', 'def', '\d+')
return 
let $to := ('x', 'y', '0')
return (functx:replace-multi('abcdef123',$fr,$to))