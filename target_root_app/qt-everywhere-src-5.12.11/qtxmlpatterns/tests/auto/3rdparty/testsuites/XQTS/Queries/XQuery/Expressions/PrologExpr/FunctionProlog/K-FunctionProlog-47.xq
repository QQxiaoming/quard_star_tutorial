(:*******************************************************:)
(: Test: K-FunctionProlog-47                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: 'empty-sequence()+' is syntactically invalid. :)
(:*******************************************************:)

declare function local:myFunction($arg as empty-sequence()+)
{
	$arg
};
local:myFunction(())
