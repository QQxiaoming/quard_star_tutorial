(: Name: annex-3 :)
(: Description: User defined function # 3 from annex E of F& O Specs. :)

declare namespace eg = "http://example.org";

declare function eg:value-union (
  $arg1 as xs:anyAtomicType*,
  $arg2 as xs:anyAtomicType*) as xs:anyAtomicType* 
{
  fn:distinct-values(($arg1, $arg2))
};

let $arg1 as xs:anyAtomicType := 1
let $arg2 as xs:anyAtomicType := 2
return
 eg:value-union($arg1,$arg2)