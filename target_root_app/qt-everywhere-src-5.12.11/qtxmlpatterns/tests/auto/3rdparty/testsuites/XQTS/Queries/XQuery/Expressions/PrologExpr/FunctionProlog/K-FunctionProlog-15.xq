(:*******************************************************:)
(: Test: K-FunctionProlog-15                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A user function whose return type doesn't match the body, which can be statically inferred. :)
(:*******************************************************:)

declare function local:myFunction() as xs:anyURI
{
	1
};
true()
