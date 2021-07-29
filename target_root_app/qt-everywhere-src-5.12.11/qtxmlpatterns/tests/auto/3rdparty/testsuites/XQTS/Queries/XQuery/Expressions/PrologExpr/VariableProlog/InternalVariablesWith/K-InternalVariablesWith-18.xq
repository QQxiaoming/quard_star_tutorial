(:*******************************************************:)
(: Test: K-InternalVariablesWith-18                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A prolog variable having a circular dependency, by having a variable reference in a call site argument. :)
(:*******************************************************:)
declare variable $var := local:func1();
declare function local:func1()
{
	local:func2($var)
};
declare function local:func2($arg2)
{
	$arg2
};
$var
