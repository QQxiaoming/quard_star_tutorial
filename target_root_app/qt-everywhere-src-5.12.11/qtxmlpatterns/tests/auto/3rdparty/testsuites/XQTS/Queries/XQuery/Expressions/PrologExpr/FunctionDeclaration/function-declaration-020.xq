(:*******************************************************:)
(: Test: function-declaration-020.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Jun  2 00:24:56 2005                        :)
(: Purpose: Demonstrate function declaration - overloading      :)
(:********************************************************************:)

declare namespace foo = "http://www..oracle.com/xquery/test";
declare function foo:price () as xs:integer+
{
  100
};
declare function foo:price ($z as xs:integer) as xs:integer+
{
  $z
};
declare function foo:price ($x as xs:integer, $y as xs:integer) as xs:integer+
{
  $x, $y
};
declare function foo:price ($x as xs:integer, $y as xs:integer, $z as xs:integer) as xs:integer+
{
  $x+$y+$z
};
foo:price(), foo:price(1), foo:price(2,3), foo:price(4,5,6)
