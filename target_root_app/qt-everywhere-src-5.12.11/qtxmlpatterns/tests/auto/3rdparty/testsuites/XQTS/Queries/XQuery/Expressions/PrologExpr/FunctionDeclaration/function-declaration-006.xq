(:*******************************************************:)
(: Test: function-declaration-006.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Jun  2 00:24:56 2005                        :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

declare function local:mysum($i as xs:integer, $j as xs:integer)
{
   let $j := $i + $j
   return $j
};
declare function local:invoke_mysum($st as xs:integer)
{
  for $d in (1,2,3,4,5)
  let $st := local:mysum($d, $st)
   return $st
};
local:invoke_mysum(0)
