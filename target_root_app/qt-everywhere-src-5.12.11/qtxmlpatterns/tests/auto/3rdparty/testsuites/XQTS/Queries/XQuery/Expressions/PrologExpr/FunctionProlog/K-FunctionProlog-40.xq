(:*******************************************************:)
(: Test: K-FunctionProlog-40                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Variables declared inside functions shadow function arguments. :)
(:*******************************************************:)

declare function local:myFunction($local:myVar as xs:integer)
{
	for $local:myVar in ($local:myVar, 3) return $local:myVar
};
deep-equal(local:myFunction(1), (1, 3))
