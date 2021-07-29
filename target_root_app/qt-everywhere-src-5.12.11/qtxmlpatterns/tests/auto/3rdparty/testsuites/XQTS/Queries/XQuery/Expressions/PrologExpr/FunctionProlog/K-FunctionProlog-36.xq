(:*******************************************************:)
(: Test: K-FunctionProlog-36                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: The variable '$myArg' is in scope inside the function, but not in the query body. :)
(:*******************************************************:)

declare function local:MyFunction($myArg)
{
	0
};
$myArg
