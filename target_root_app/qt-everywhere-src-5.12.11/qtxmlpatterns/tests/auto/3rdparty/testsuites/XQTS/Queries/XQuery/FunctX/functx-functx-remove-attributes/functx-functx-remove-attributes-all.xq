(:**************************************************************:)
(: Test: functx-functx-remove-attributes-all                                  :)
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
 : Removes attributes from an XML element, based on name 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_remove-attributes.html 
 : @param   $elements the element(s) from which to remove the attributes 
 : @param   $names the names of the attributes to remove, or * for all attributes 
 :) 
declare function functx:remove-attributes 
  ( $elements as element()* ,
    $names as xs:string* )  as element() {
       
   for $element in $elements
   return element
     {node-name($element)}
     {$element/@*[not(functx:name-test(name(),$names))],
      $element/node() }
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

let $in-xml-1 := <a attr1="123" attr2="456">abc</a>
return 
let $in-xml-2 := <a xmlns:a="http://a"
    a:attr1="123" attr1="456">abc</a>
return (functx:remove-attributes(
     $in-xml-1, ('attr1','attr2')), functx:remove-attributes(
     $in-xml-1, ('attr1','attr3')), functx:remove-attributes($in-xml-1, '*'), functx:remove-attributes(
     $in-xml-2, ('a:attr1')))