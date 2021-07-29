(:*******************************************************:)
(: Test: K-InternalVariablesWith-19                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A prolog variable having a circular dependency, stretching through functions and variables. :)
(:*******************************************************:)

declare variable $var2 := local:func1();
declare variable $var := ($var2 treat as xs:integer) + 1;
declare function local:func1()
{
	local:func2()
};
declare function local:func2()
{
	local:func3()
};
declare function local:func3()
{
	local:func4()
};
declare function local:func4()
{
	$var
};
boolean($var)
