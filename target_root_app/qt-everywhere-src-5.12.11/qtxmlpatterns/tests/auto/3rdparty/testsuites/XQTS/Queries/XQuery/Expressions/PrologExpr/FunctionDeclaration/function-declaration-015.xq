(:*******************************************************:)
(: Test: function-declaration-015.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Jun  2 00:24:56 2005                        :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

declare function local:odd($x as xs:integer) as xs:boolean
{if ($x = 0) then false() else local:even($x - 1)};
declare function local:even($x as xs:integer) as xs:boolean
{if ($x = 0) then true() else local:odd($x - 1)};
local:even(3)
