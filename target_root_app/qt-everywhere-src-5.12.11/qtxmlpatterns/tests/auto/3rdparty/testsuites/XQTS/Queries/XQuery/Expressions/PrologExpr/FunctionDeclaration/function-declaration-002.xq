(:********************************************************************:)
(: Test: function-declaration-002.xq                                  :)
(: Written By: Pulkita Tyagi                                          :)
(: Date: Thu Jun  2 00:24:56 2005                                     :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

declare function local:foo($n as xs:integer)
{
   <tr> {$n} </tr>
};
local:foo(4)
