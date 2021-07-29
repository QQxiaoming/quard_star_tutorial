(:*******************************************************:)
(: Test: K-FunctionProlog-12                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Variables appearing after a function declaration is not in scope inside the function. :)
(:*******************************************************:)

declare function local:computeSum()
{
	$myVariable
};
declare variable $myVariable := 1;
1
