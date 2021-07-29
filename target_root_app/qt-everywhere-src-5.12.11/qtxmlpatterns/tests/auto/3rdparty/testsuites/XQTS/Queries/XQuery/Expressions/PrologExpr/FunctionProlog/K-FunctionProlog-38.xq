(:*******************************************************:)
(: Test: K-FunctionProlog-38                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: The variable '$myArg2' is in scope inside one function, but not the other function. :)
(:*******************************************************:)

declare function local:MyFunction($myArg)
{
	$myArg2
};
declare function local:MyFunction2($myArg2)
{
	0
};
1
