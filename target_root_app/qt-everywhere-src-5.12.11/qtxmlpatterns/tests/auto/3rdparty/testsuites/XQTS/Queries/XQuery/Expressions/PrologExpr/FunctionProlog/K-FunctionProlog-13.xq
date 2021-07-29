(:*******************************************************:)
(: Test: K-FunctionProlog-13                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Namespaces declarations appearing after a function declaration are not in scope inside the function. :)
(:*******************************************************:)

declare function local:computeSum()
{
	$prefix:myVariable
};
declare namespaces prefix = "example.com/Anamespace";
1
