(:*******************************************************:)
(: Test: K2-FunctionProlog-33                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Mix in an atomic value which matches the type it's being converted to. :)
(:*******************************************************:)
declare function local:foo($arg (: $arg doesn't have a type declared. :)) as xs:boolean*
{
    $arg
};
local:foo((<e>true</e>, true(), xs:untypedAtomic("false"), false(), <e>  true  </e>))