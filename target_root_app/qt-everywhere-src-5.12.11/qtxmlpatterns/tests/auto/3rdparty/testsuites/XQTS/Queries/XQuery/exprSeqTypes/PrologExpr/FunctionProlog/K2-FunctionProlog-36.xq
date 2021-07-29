(:*******************************************************:)
(: Test: K2-FunctionProlog-36                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Pass an untyped value(different cardinality). :)
(:*******************************************************:)
declare function local:foo($arg) as xs:boolean
{
    $arg
};
local:foo(xs:untypedAtomic("false"))