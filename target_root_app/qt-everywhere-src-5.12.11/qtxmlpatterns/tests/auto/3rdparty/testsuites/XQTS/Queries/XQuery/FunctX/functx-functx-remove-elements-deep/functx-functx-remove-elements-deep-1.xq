(:**************************************************************:)
(: Test: functx-functx-remove-elements-deep-1                                  :)
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
 : Removes descendant elements from an XML node, based on name 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_remove-elements-deep.html 
 : @param   $nodes root(s) to start from 
 : @param   $names the names of the elements to remove 
 :) 
declare function functx:remove-elements-deep 
  ( $nodes as node()* ,
    $names as xs:string* )  as node()* {
       
   for $node in $nodes
   return
     if ($node instance of element())
     then if (functx:name-test(name($node),$names))
          then ()
          else element { node-name($node)}
                { $node/@*,
                  functx:remove-elements-deep($node/node(), $names)}
     else if ($node instance of document-node())
     then functx:remove-elements-deep($node/node(), $names)
     else $node
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
   <a>
      <b>b1</b>
      <c>c1</c>
   </a>
   <c>Mixed <b>content</b></c>
</in-xml>
return 
let $in-xml-2 := <in-xml xmlns:x="http://x">
   <a>
      <x:b>b1</x:b>
      <c>c1</c>
   </a>
   <c>Mixed <b>content</b></c>
</in-xml>
return (functx:remove-elements-deep(
     $in-xml-1,
     'b'))
