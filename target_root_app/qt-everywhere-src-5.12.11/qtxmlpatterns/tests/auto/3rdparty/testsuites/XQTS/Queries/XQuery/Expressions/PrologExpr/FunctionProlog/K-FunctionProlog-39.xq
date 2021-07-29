(:*******************************************************:)
(: Test: K-FunctionProlog-39                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Function arguments shadows global variables. :)
(:*******************************************************:)

declare variable $local:myVar := 1;
declare function local:myFunction($local:myVar)
{
	$local:myVar
};
$local:myVar, local:myFunction(2)
