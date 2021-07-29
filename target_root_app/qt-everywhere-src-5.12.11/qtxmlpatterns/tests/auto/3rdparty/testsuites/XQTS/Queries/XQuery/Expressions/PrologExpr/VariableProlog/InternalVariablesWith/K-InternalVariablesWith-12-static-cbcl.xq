(:*******************************************************:)
(: Test: K-InternalVariablesWith-12                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-06-13T10:58:41+01:00                       :)
(: Purpose: One prolog variable depending on a user function appearing after it. :)
(:*******************************************************:)
declare variable $var1 := local:myFunc();
declare function local:myFunc() as xs:integer
{
	1
};
$var1 eq 1
