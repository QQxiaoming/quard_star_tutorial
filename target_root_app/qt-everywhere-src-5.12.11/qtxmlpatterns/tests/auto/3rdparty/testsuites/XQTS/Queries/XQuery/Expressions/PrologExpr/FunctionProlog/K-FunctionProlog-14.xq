(:*******************************************************:)
(: Test: K-FunctionProlog-14                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A user function which when run doesn't match the declared returned type. :)
(:*******************************************************:)

declare function local:myFunction() as xs:integer
{
	subsequence((1, 2, "a string"), 3 ,1)
};
fn:boolean(local:myFunction())
