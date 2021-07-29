(:*******************************************************:)
(: Test: K2-FunctionCallExpr-1                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Two user functions with many arguments.      :)
(:*******************************************************:)
declare function local:bar($c, $d, $e, $f, $g, $h, $i, $j, $a, $b)
{
    1
};

declare function local:moo($k)
{
    $k
};
local:moo(1)