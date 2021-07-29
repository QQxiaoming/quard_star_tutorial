(:*******************************************************:)
(: Test: K2-FunctionProlog-31                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Pass a sequence of mixed atomic values and nodes to an argument which has no type declared. :)
(:*******************************************************:)
declare function local:foo($arg) as xs:boolean*
{
    $arg
};
local:foo((<e>true</e>, true(), xs:untypedAtomic("false"), false(), <e>  true  </e>))