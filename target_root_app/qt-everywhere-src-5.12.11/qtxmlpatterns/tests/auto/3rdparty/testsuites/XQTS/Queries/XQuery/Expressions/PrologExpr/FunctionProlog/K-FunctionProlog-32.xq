(:*******************************************************:)
(: Test: K-FunctionProlog-32                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: One cannot declare a user function as xs:gYear. :)
(:*******************************************************:)

declare function xs:gYear($arg as xs:anyAtomicType?) as xs:gYear?
{
	xs:gYear($arg)
};
1
