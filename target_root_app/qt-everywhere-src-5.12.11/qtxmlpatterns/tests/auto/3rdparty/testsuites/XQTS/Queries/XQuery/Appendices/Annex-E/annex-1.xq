(: Name: annex-1 :)
(: Description: User defined function # 1 from annex E of F& O Specs. :)

declare namespace eg = "http://example.org";

declare function eg:if-empty (
  $node as node()?,
  $value as xs:anyAtomicType) as xs:anyAtomicType* 
{
  if ($node and $node/child::node())
            then fn:data($node)
            else $value
};

let $arg1 := <element1>some data</element1>
let $arg2 as xs:anyAtomicType := 1
return
 eg:if-empty($arg1,$arg2)