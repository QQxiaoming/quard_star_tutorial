(:*******************************************************:)
(: Test: K-FunctionProlog-23                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: User functions where the last argument of three arguments is unused. :)
(:*******************************************************:)

declare function local:func($a as xs:integer, $b as xs:integer, $unused)
{
	$a + $b
};
local:func(1, 2, 3)
