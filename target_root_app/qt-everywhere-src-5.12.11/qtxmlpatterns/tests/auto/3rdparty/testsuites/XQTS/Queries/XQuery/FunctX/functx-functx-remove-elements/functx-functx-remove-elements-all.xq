(:**************************************************************:)
(: Test: functx-functx-remove-elements-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether a name matches a list of names or name wildcards 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_name-test.html 
 : @param   $testname the name to test 
 : @param   $names the list of names or name wildcards 
 :) 
declare function functx:name-test 
  ( $testname as xs:string? ,
    $names as xs:string* )  as xs:boolean {
       
$testname = $names
or
$names = '*'
or
functx:substring-after-if-contains($testname,':') =
   (for $name in $names
   return substring-after($name,'*:'))
or
substring-before($testname,':') =
   (for $name in $names[contains(.,':*')]
   return substring-before($name,':*'))
 } ;

(:~
 : Removes child elements from an XML node, based on name 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_remove-elements.html 
 : @param   $elements the element(s) from which you wish to remove the children 
 : @param   $names the names of the child elements to remove 
 :) 
declare function functx:remove-elements 
  ( $elements as element()* ,
    $names as xs:string* )  as element()* {
       
   for $element in $elements
   return element
     {node-name($element)}
     {$element/@*,
      $element/node()[not(functx:name-test(name(),$names))] }
 } ;

(:~
 : Performs substring-after, returning the entire string if it does not contain the delimiter 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_substring-after-if-contains.html 
 : @param   $arg the string to substring 
 : @param   $delim the delimiter 
 :) 
declare function functx:substring-after-if-contains 
  ( $arg as xs:string? ,
    $delim as xs:string )  as xs:string? {
       
   if (contains($arg,$delim))
   then substring-after($arg,$delim)
   else $arg
 } ;

let $in-xml-1 := <in-xml>
   <a>123</a>
   <a>456</a>
   <c>Mixed <b>content</b></c>
</in-xml>
return 
let $in-xml-2 := <in-xml xmlns:x="http://x">
   <a>123</a>
   <x:a>456</x:a>
   <c>Mixed <x:a>content</x:a></c>
</in-xml>
return (functx:remove-elements(
     $in-xml-1,
     'c'), functx:remove-elements(
     $in-xml-1,
     ('a','b')), functx:remove-elements(
     $in-xml-2,
     'x:a'))