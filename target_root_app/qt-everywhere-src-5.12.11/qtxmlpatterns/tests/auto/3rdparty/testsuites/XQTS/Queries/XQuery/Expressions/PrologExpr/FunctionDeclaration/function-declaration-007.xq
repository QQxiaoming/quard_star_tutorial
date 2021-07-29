(:*******************************************************:)
(: Test: function-declaration-007.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Jun  2 00:24:56 2005                        :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

declare variable $a := 1;
declare function local:foo($a as xs:integer)
{
  if ($a > 100)
  then $a
  else
    let $a := $a + 1
    return local:foo($a)
};
local:foo($a)
