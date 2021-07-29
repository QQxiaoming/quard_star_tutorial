(:*******************************************************:)
(: Test: function-declaration-013.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Jun  2 00:24:56 2005                        :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

declare function local:foo2($i as xs:string) as xs:string
{local:foo($i)};
declare function local:foo($i as xs:string) as xs:string
{$i};
local:foo2("abc")
