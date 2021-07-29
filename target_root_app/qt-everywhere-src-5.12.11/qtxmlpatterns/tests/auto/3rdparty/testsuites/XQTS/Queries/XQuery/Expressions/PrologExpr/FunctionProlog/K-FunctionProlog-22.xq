(:*******************************************************:)
(: Test: K-FunctionProlog-22                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: User functions where the first argument of three arguments is unused. :)
(:*******************************************************:)

declare function local:func($unused, $b as xs:integer, $c as xs:integer)
{
	$b + $c
};
local:func(1, 2, 3)
