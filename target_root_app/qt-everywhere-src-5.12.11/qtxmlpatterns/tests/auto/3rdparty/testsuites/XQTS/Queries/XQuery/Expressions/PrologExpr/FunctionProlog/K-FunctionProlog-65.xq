(:*******************************************************:)
(: Test: K-FunctionProlog-65                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Since the return type is empty-sequence() it is a type error(XPTY0004) to pass its return value to anything which requires cardinality exactly-one. That the function's body is of type 'none', doesn't affect that, it only adapts to the declared return type. However, there's no constraints on what is reported first, so FOER0000 can also be issued. :)
(:*******************************************************:)
declare function local:myFunction() as empty-sequence()
{
	fn:error()
};
QName("http://example.com/ANamespace", local:myFunction())
