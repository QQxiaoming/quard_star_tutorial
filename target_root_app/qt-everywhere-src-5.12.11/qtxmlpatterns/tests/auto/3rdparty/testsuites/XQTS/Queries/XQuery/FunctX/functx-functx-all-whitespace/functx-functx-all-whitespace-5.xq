(:**************************************************************:)
(: Test: functx-functx-all-whitespace-5                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether a value is all whitespace or a zero-length string 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_all-whitespace.html 
 : @param   $arg the string (or node) to test 
 :) 
declare function functx:all-whitespace 
  ( $arg as xs:string? )  as xs:boolean {
       
   normalize-space($arg) = ''
 } ;

let $in-xml := <in-xml>
   <a>    </a>
   <b>x    </b>
   <c>  <x>x</x>    </c>
</in-xml>
return (functx:all-whitespace($in-xml/c))
