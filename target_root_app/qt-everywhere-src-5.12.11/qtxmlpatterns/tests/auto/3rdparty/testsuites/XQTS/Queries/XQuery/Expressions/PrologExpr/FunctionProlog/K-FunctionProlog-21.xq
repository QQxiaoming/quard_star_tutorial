(:*******************************************************:)
(: Test: K-FunctionProlog-21                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: User functions where the middle argument of three arguments is unused. :)
(:*******************************************************:)

declare function local:func($a as xs:integer, $unused, $c as xs:integer)
{
	$a + $c
};
local:func(1, 2, 3)
