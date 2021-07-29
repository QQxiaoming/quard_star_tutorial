(:*******************************************************:)
(: Test: function-declaration-017.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Jun  2 00:24:56 2005                        :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

declare default element namespace "http://www.example.com/filesystem";
declare variable $v as xs:integer := 100;
declare function local:udf1 ($CUSTNO as xs:integer)
{
  <empty>
  {$CUSTNO*$v}
  </empty>
};
local:udf1(10)
