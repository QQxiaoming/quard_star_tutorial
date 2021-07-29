(:*******************************************************:)
(: Test: K-FunctionProlog-45                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Item type error in argument value. xs:decimal doesn't match xs:integer. :)
(:*******************************************************:)

declare function local:myFunction($arg as item()?, $arg2 as xs:integer)
{
	$arg, $arg2
};
local:myFunction((), 4.1)
