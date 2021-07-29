(:*******************************************************:)
(: Test: function-declaration-012.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Jun  2 00:24:56 2005                        :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

declare function local:fn1 ($n as xs:integer) as xs:integer
{
   local:fn2($n)
};
declare function local:fn2 ($n as xs:integer) as xs:integer
{
   if ($n = 1)
        then 1
        else $n + local:fn1($n - 1)
};
local:fn1(4)
