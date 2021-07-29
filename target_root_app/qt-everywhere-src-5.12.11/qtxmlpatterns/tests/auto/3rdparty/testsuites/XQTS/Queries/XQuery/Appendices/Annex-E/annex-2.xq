(: Name: annex-2 :)
(: Description: User defined function # 2 from annex E of F& O Specs. :)

declare namespace eg = "http://example.org";

declare function eg:if-absent (
  $node as node()?,
  $value as xs:anyAtomicType) as xs:anyAtomicType* 
{
  if ($node)
    then fn:data($node)
    else $value
};

let $arg1 := <element1>some data</element1>
let $arg2 as xs:anyAtomicType := 1
return
 eg:if-absent($arg1,$arg2)