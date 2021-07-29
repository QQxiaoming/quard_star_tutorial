(:*******************************************************:)
(: Test: K-FunctionProlog-20                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: User functions which in some implementations causes constant propagation combined with function versioning. :)
(:*******************************************************:)

declare function local:func($choose, $whenTrue, $whenFalse)
{
	if($choose)
	then $whenTrue
	else $whenFalse
};
local:func(true(), current-time(), current-date()) instance of xs:time
and
local:func(false(), current-time(), current-date()) instance of xs:date
