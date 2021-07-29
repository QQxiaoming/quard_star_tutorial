(:*******************************************************:)
(: Test: K2-FunctionProlog-32                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Pass a value which cannot be converted to the expected type. :)
(:*******************************************************:)

declare function local:foo($arg (: $arg doesn't have a type declared. :)) as xs:boolean*
{
$arg
};
local:foo(current-date())
