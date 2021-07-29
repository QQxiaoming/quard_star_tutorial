(:*******************************************************:)
(: Test: K2-FunctionProlog-34                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Call fn:true() and fn:false() and pass to a user function. :)
(:*******************************************************:)
declare function local:foo($arg) as xs:boolean*
{
    $arg
};
local:foo((true(), xs:untypedAtomic("false"))),
local:foo((false(), xs:untypedAtomic("false")))