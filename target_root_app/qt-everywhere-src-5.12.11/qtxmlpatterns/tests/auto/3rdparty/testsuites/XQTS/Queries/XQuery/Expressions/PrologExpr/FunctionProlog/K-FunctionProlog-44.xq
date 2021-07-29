(:*******************************************************:)
(: Test: K-FunctionProlog-44                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cardinality error in argument value.         :)
(:*******************************************************:)

declare function local:myFunction($arg as item(), $arg2 as xs:integer)
{
	$arg, $arg2
};
local:myFunction("3", ())
