(:*******************************************************:)
(: Test: K-FunctionProlog-62                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Check that a global variable is in scope despite an unused function being declared. :)
(:*******************************************************:)

declare variable $my := 3;
declare function local:myFunction($my, $arg2, $arg4)
{
	1
};
$my eq 3
