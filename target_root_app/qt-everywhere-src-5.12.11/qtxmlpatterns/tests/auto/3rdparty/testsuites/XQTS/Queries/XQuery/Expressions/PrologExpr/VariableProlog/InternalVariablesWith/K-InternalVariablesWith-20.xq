(:*******************************************************:)
(: Test: K-InternalVariablesWith-20                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A prolog variable having a circular dependency, stretching through functions and variables(#2). :)
(:*******************************************************:)

declare variable $var := local:func1();
declare function local:func1()
{
	local:func2()
};
declare function local:func2()
{
	local:func3()
};

declare variable $var2 := local:func2();
declare function local:func3()
{
	$var2
};
boolean($var)
