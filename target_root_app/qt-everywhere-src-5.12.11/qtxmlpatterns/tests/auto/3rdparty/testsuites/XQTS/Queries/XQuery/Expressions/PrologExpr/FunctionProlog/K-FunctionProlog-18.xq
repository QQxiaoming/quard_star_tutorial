(:*******************************************************:)
(: Test: K-FunctionProlog-18                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Two user functions using global variables.   :)
(:*******************************************************:)

declare variable $var1 := 1;
declare function local:func1() as xs:integer
{
	$var1
};

declare variable $var2 := 2;
declare function local:func2() as xs:integer
{
	$var2
};
1 eq local:func1() and 2 eq local:func2()
